/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "js_runtime_utils.h"

#include <fstream>

#include "hilog_wrapper.h"
#include "js_runtime.h"

#ifdef WINDOWS_PLATFORM
#include <io.h>

namespace {
char* realpath(const char* path, char* resolvedPath)
{
    if (_access(path, 0) < 0) {
        return nullptr;
    }
    if (strcpy_s(resolvedPath, PATH_MAX, path) != 0) {
        return nullptr;
    }
    return resolvedPath;
}
}
#endif
namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char EXT_NAME_ABC[] = ".abc";
constexpr char EXT_NAME_ETS[] = ".ets";
constexpr char EXT_NAME_TS[] = ".ts";
constexpr char EXT_NAME_JS[] = ".js";
constexpr char PREFIX_BUNDLE[] = "@bundle:";
constexpr char PREFIX_MODULE[] = "@module:";
constexpr char PREFIX_LOCAL[] = "@local:";
constexpr char NPM_PATH_SEGMENT[] = "node_modules";
constexpr char NPM_ENTRY_FILE[] = "index.abc";
constexpr char NPM_ENTRY_LINK[] = "entry.txt";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr char OTHER_BUNDLE_INSTALL_PATH[] = "/data/bundles/";

constexpr size_t MAX_NPM_LEVEL = 1;
constexpr size_t SEGMENTS_LIMIT_TWO = 2;
constexpr size_t SEGMENTS_LIMIT_THREE = 3;

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    std::unique_ptr<AsyncTask::ExecuteCallback>&& execute, std::unique_ptr<AsyncTask::CompleteCallback>&& complete,
    NativeValue** result)
{
    if (lastParam == nullptr || lastParam->TypeOf() != NATIVE_FUNCTION) {
        NativeDeferred* nativeDeferred = nullptr;
        *result = engine.CreatePromise(&nativeDeferred);
        return std::make_unique<AsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        *result = engine.CreateUndefined();
        NativeReference* callbackRef = engine.CreateReference(lastParam, 1);
        return std::make_unique<AsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}

inline bool StringStartWith(const std::string& str, const char* startStr, size_t startStrLen)
{
    return ((str.length() >= startStrLen) && (str.compare(0, startStrLen, startStr) == 0));
}

inline bool StringEndWith(const std::string& str, const char* endStr, size_t endStrLen)
{
    size_t len = str.length();
    return ((len >= endStrLen) && (str.compare(len - endStrLen, endStrLen, endStr) == 0));
}

void SplitString(const std::string& str, std::vector<std::string>& out, size_t pos = 0, const char* seps = "\\/")
{
    if (str.empty() || pos >= str.length()) {
        return;
    }

    size_t startPos = pos;
    size_t endPos = 0;
    while ((endPos = str.find_first_of(seps, startPos)) != std::string::npos) {
        if (endPos > startPos) {
            out.emplace_back(str.substr(startPos, endPos - startPos));
        }
        startPos = endPos + 1;
    }

    if (startPos < str.length()) {
        out.emplace_back(str.substr(startPos));
    }
}

std::string JoinString(const std::vector<std::string>& strs, char sep, size_t startIndex = 0)
{
    std::string out;
    for (size_t index = startIndex; index < strs.size(); ++index) {
        if (!strs[index].empty()) {
            out.append(strs[index]) += sep;
        }
    }
    if (!out.empty()) {
        out.pop_back();
    }
    return out;
}

inline std::string StripString(const std::string& str, const char* charSet = " \t\n\r")
{
    size_t startPos = str.find_first_not_of(charSet);
    if (startPos == std::string::npos) {
        return std::string();
    }

    return str.substr(startPos, str.find_last_not_of(charSet) - startPos + 1);
}
} // namespace

// Help Functions
NativeValue* CreateJsError(NativeEngine& engine, int32_t errCode, const std::string& message)
{
    return engine.CreateError(CreateJsValue(engine, errCode), CreateJsValue(engine, message));
}

void BindNativeFunction(NativeEngine& engine, NativeObject& object, const char* name,
    const char* moduleName, NativeCallback func)
{
    std::string fullName(moduleName);
    fullName += ".";
    fullName += name;
    object.SetProperty(name, engine.CreateFunction(fullName.c_str(), fullName.length(), func, nullptr));
}

void BindNativeProperty(NativeObject& object, const char* name, NativeCallback getter)
{
    NativePropertyDescriptor property;
    property.utf8name = name;
    property.name = nullptr;
    property.method = nullptr;
    property.getter = getter;
    property.setter = nullptr;
    property.value = nullptr;
    property.attributes = napi_default;
    property.data = nullptr;
    object.DefineProperty(property);
}

void* GetNativePointerFromCallbackInfo(NativeEngine* engine, NativeCallbackInfo* info, const char* name)
{
    if (engine == nullptr || info == nullptr) {
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(info->thisVar);
    if (object != nullptr && name != nullptr) {
        object = ConvertNativeValueTo<NativeObject>(object->GetProperty(name));
    }
    return (object != nullptr) ? object->GetNativePointer() : nullptr;
}

void SetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name, void* ptr, NativeFinalize func)
{
    NativeValue* value = engine.CreateObject();
    NativeObject* newObject = ConvertNativeValueTo<NativeObject>(value);
    if (newObject == nullptr) {
        return;
    }
    newObject->SetNativePointer(ptr, func, nullptr);
    object.SetProperty(name, value);
}

void* GetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name)
{
    NativeObject* namedObj = ConvertNativeValueTo<NativeObject>(object.GetProperty(name));
    return (namedObj != nullptr) ? namedObj->GetNativePointer() : nullptr;
}

// Handle Scope
HandleScope::HandleScope(JsRuntime& jsRuntime)
{
    scopeManager_ = jsRuntime.GetNativeEngine().GetScopeManager();
    if (scopeManager_ != nullptr) {
        nativeScope_ = scopeManager_->OpenEscape();
    }
}

HandleScope::HandleScope(NativeEngine& engine)
{
    scopeManager_ = engine.GetScopeManager();
    if (scopeManager_ != nullptr) {
        nativeScope_ = scopeManager_->OpenEscape();
    }
}

HandleScope::~HandleScope()
{
    if (nativeScope_ != nullptr) {
        scopeManager_->CloseEscape(nativeScope_);
        nativeScope_ = nullptr;
    }
    scopeManager_ = nullptr;
}

NativeValue* HandleScope::Escape(NativeValue* value)
{
    if (nativeScope_ != nullptr) {
        scopeManager_->Escape(nativeScope_, value);
    }
    return value;
}

// Async Task
AsyncTask::AsyncTask(NativeDeferred* deferred, std::unique_ptr<AsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<AsyncTask::CompleteCallback>&& complete)
    : deferred_(deferred), execute_(std::move(execute)), complete_(std::move(complete))
{}

AsyncTask::AsyncTask(NativeReference* callbackRef, std::unique_ptr<AsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<AsyncTask::CompleteCallback>&& complete)
    : callbackRef_(callbackRef), execute_(std::move(execute)), complete_(std::move(complete))
{}

AsyncTask::~AsyncTask() = default;

void AsyncTask::Schedule(const std::string &name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task)
{
    if (task && task->Start(name, engine)) {
        task.release();
    }
}

void AsyncTask::Resolve(NativeEngine& engine, NativeValue* value)
{
    HILOG_DEBUG("AsyncTask::Resolve is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            CreateJsError(engine, 0),
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_DEBUG("AsyncTask::Resolve is called end.");
}

void AsyncTask::Reject(NativeEngine& engine, NativeValue* error)
{
    if (deferred_) {
        deferred_->Reject(error);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            engine.CreateUndefined(),
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
}

void AsyncTask::Execute(NativeEngine* engine, void* data)
{
    if (engine == nullptr || data == nullptr) {
        return;
    }
    auto me = static_cast<AsyncTask*>(data);
    if (me->execute_ && *(me->execute_)) {
        (*me->execute_)();
    }
}

void AsyncTask::Complete(NativeEngine* engine, int32_t status, void* data)
{
    if (engine == nullptr || data == nullptr) {
        return;
    }
    std::unique_ptr<AsyncTask> me(static_cast<AsyncTask*>(data));
    if (me->complete_ && *(me->complete_)) {
        HandleScope handleScope(*engine);
        (*me->complete_)(*engine, *me, status);
    }
}

bool AsyncTask::Start(const std::string &name, NativeEngine& engine)
{
    work_.reset(engine.CreateAsyncWork(name, Execute, Complete, this));
    return work_->Queue();
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, AsyncTask::CompleteCallback&& complete, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(engine, lastParam,
        std::make_unique<AsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<AsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, nullptr_t, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(
        engine, lastParam, std::make_unique<AsyncTask::ExecuteCallback>(std::move(execute)), nullptr, result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    nullptr_t, AsyncTask::CompleteCallback&& complete, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(
        engine, lastParam, nullptr, std::make_unique<AsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    nullptr_t, nullptr_t, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(engine, lastParam, std::unique_ptr<AsyncTask::ExecuteCallback>(),
        std::unique_ptr<AsyncTask::CompleteCallback>(), result);
}

void FixExtName(std::string& path)
{
    if (path.empty()) {
        return;
    }

    if (StringEndWith(path, EXT_NAME_ABC, sizeof(EXT_NAME_ABC) - 1)) {
        return;
    }

    if (StringEndWith(path, EXT_NAME_ETS, sizeof(EXT_NAME_ETS) - 1)) {
        path.erase(path.length() - (sizeof(EXT_NAME_ETS) - 1), sizeof(EXT_NAME_ETS) - 1);
    } else if (StringEndWith(path, EXT_NAME_TS, sizeof(EXT_NAME_TS) - 1)) {
        path.erase(path.length() - (sizeof(EXT_NAME_TS) - 1), sizeof(EXT_NAME_TS) - 1);
    } else if (StringEndWith(path, EXT_NAME_JS, sizeof(EXT_NAME_JS) - 1)) {
        path.erase(path.length() - (sizeof(EXT_NAME_JS) - 1), sizeof(EXT_NAME_JS) - 1);
    }

    path.append(EXT_NAME_ABC);
}

std::string GetInstallPath(const std::string& curJsModulePath, bool module)
{
    size_t pos = std::string::npos;
    if (StringStartWith(curJsModulePath, BUNDLE_INSTALL_PATH, std::string(BUNDLE_INSTALL_PATH).length())) {
        pos = std::string(BUNDLE_INSTALL_PATH).length() - 1;
    } else {
        if (!StringStartWith(curJsModulePath, OTHER_BUNDLE_INSTALL_PATH,
            std::string(OTHER_BUNDLE_INSTALL_PATH).length())) {
            return std::string();
        }

        pos = curJsModulePath.find('/', std::string(OTHER_BUNDLE_INSTALL_PATH).length());
        if (pos == std::string::npos) {
            return std::string();
        }
    }

    if (module) {
        pos = curJsModulePath.find('/', pos + 1);
        if (pos == std::string::npos) {
            return std::string();
        }
    }

    return curJsModulePath.substr(0, pos + 1);
}

std::string MakeNewJsModulePath(
    const std::string& curJsModulePath, const std::string& newJsModuleUri)
{
    std::string moduleInstallPath = GetInstallPath(curJsModulePath, true);
    if (moduleInstallPath.empty()) {
        return std::string();
    }

    std::vector<std::string> pathVector;
    SplitString(curJsModulePath, pathVector, moduleInstallPath.length());

    if (pathVector.empty()) {
        return std::string();
    }

    // Remove file name, reserve only dir name
    pathVector.pop_back();

    std::vector<std::string> relativePathVector;
    SplitString(newJsModuleUri, relativePathVector);

    for (auto& value : relativePathVector) {
        if (value == ".") {
            continue;
        } else if (value == "..") {
            if (pathVector.empty()) {
                return std::string();
            }
            pathVector.pop_back();
        } else {
            pathVector.emplace_back(std::move(value));
        }
    }

    std::string jsModulePath = moduleInstallPath + JoinString(pathVector, '/');
    FixExtName(jsModulePath);
    if (jsModulePath.size() >= PATH_MAX) {
        return std::string();
    }

    char path[PATH_MAX];
    if (realpath(jsModulePath.c_str(), path) != nullptr) {
        return std::string(path);
    }
    return std::string();
}

std::string FindNpmPackageInPath(const std::string& npmPath)
{
    std::string fileName = npmPath + "/" + NPM_ENTRY_FILE;

    char path[PATH_MAX];
    if (fileName.size() >= PATH_MAX) {
        return std::string();
    }
    if (realpath(fileName.c_str(), path) != nullptr) {
        return path;
    }

    fileName = npmPath + "/" + NPM_ENTRY_LINK;
    if (fileName.size() >= PATH_MAX) {
        return std::string();
    }
    if (realpath(fileName.c_str(), path) == nullptr) {
        return std::string();
    }

    std::ifstream stream(path, std::ios::ate);
    if (!stream.is_open()) {
        return std::string();
    }

    auto fileLen = stream.tellg();
    if (fileLen >= PATH_MAX) {
        return std::string();
    }

    stream.seekg(0);
    stream.read(path, fileLen);
    path[fileLen] = '\0';
    stream.close();

    std::string npmPackagePath = npmPath + '/' + StripString(path);
    if (npmPackagePath.size() >= PATH_MAX) {
        return std::string();
    }
    if (realpath(npmPackagePath.c_str(), path) == nullptr) {
        return std::string();
    }
    return path;
}

std::string FindNpmPackageInTopLevel(
    const std::string& moduleInstallPath, const std::string& npmPackage, size_t start)
{
    for (size_t level = start; level <= MAX_NPM_LEVEL; ++level) {
        std::string path = moduleInstallPath + NPM_PATH_SEGMENT + '/' + std::to_string(level) + '/' + npmPackage;
        path = FindNpmPackageInPath(path);
        if (!path.empty()) {
            return path;
        }
    }

    return std::string();
}

std::string FindNpmPackage(const std::string& curJsModulePath, const std::string& npmPackage)
{
    std::string newJsModulePath = MakeNewJsModulePath(curJsModulePath, npmPackage);
    if (!newJsModulePath.empty()) {
        return newJsModulePath;
    }
    std::string moduleInstallPath = GetInstallPath(curJsModulePath);
    if (moduleInstallPath.empty()) {
        return std::string();
    }
    std::vector<std::string> pathVector;
    SplitString(curJsModulePath, pathVector, moduleInstallPath.length());
    if (pathVector.empty()) {
        return std::string();
    }

    if (pathVector[0] != NPM_PATH_SEGMENT) {
        return FindNpmPackageInTopLevel(moduleInstallPath, npmPackage);
    }

    // Remove file name, reserve only dir name
    pathVector.pop_back();

    // Find npm package until reach top level npm path such as 'node_modules/0',
    // so there must be 2 element in vector
    while (pathVector.size() > 2) {
        std::string path =
            moduleInstallPath + JoinString(pathVector, '/') + '/' + NPM_PATH_SEGMENT + '/' + npmPackage;
        path = FindNpmPackageInPath(path);
        if (!path.empty()) {
            return path;
        }

        pathVector.pop_back();
    }

    char* p = nullptr;
    size_t index = std::strtoul(pathVector.back().c_str(), &p, 10);
    if (p == nullptr || *p != '\0') {
        return std::string();
    }

    return FindNpmPackageInTopLevel(moduleInstallPath, npmPackage, index);
}

std::string ParseOhmUri(
    const std::string& originBundleName, const std::string& curJsModulePath, const std::string& newJsModuleUri)
{
    std::string moduleInstallPath;
    std::vector<std::string> pathVector;
    size_t index = 0;

    if (StringStartWith(newJsModuleUri, PREFIX_BUNDLE, sizeof(PREFIX_BUNDLE) - 1)) {
        SplitString(newJsModuleUri, pathVector, sizeof(PREFIX_BUNDLE) - 1);

        // Uri should have atleast 3 segments
        if (pathVector.size() < SEGMENTS_LIMIT_THREE) {
            return std::string();
        }

        const auto& bundleName = pathVector[index++];
        if (bundleName == originBundleName) {
            moduleInstallPath = std::string(BUNDLE_INSTALL_PATH);
        } else {
            moduleInstallPath = std::string(OTHER_BUNDLE_INSTALL_PATH);
            moduleInstallPath.append(bundleName).append("/");
        }
        moduleInstallPath.append(pathVector[index++]).append("/");
    } else if (StringStartWith(newJsModuleUri, PREFIX_MODULE, sizeof(PREFIX_MODULE) - 1)) {
        SplitString(newJsModuleUri, pathVector, sizeof(PREFIX_MODULE) - 1);

        // Uri should have atleast 2 segments
        if (pathVector.size() < SEGMENTS_LIMIT_TWO) {
            return std::string();
        }

        moduleInstallPath = GetInstallPath(curJsModulePath, false);
        if (moduleInstallPath.empty()) {
            return std::string();
        }
        moduleInstallPath.append(pathVector[index++]).append("/");
    } else if (StringStartWith(newJsModuleUri, PREFIX_LOCAL, sizeof(PREFIX_LOCAL) - 1)) {
        SplitString(newJsModuleUri, pathVector, sizeof(PREFIX_LOCAL) - 1);

        if (pathVector.empty()) {
            return std::string();
        }

        moduleInstallPath = GetInstallPath(curJsModulePath);
        if (moduleInstallPath.empty()) {
            return std::string();
        }
    } else {
        return std::string();
    }

    if (pathVector[index] != NPM_PATH_SEGMENT) {
        return moduleInstallPath + JoinString(pathVector, '/', index);
    }

    return FindNpmPackageInTopLevel(moduleInstallPath, JoinString(pathVector, '/', index + 1));
}

bool MakeFilePath(const std::string& codePath, const std::string& modulePath, std::string& fileName)
{
    std::string path(codePath);
    path.append("/").append(modulePath);
    if (path.length() > PATH_MAX) {
        HILOG_ERROR("Path length(%{public}zu) longer than MAX(%{public}d)", path.length(), PATH_MAX);
        return false;
    }
    char resolvedPath[PATH_MAX + 1] = { 0 };
    if (realpath(path.c_str(), resolvedPath) != nullptr) {
        fileName = resolvedPath;
        return true;
    }

    auto start = path.find_last_of('/');
    auto end = path.find_last_of('.');
    if (end == std::string::npos || end == 0) {
        HILOG_ERROR("No secondary file path");
        return false;
    }

    auto pos = path.find_last_of('.', end - 1);
    if (pos == std::string::npos) {
        HILOG_ERROR("No secondary file path");
        return false;
    }

    path.erase(start + 1, pos - start);
    HILOG_DEBUG("Try using secondary file path: %{private}s", path.c_str());

    if (realpath(path.c_str(), resolvedPath) == nullptr) {
        HILOG_ERROR("Failed to call realpath, errno = %{public}d", errno);
        return false;
    }

    fileName = resolvedPath;
    return true;
}

std::string NormalizeUri(
    const std::string& bundleName, const std::string& curJsModulePath, const std::string& newJsModuleUri)
{
    std::string newJsModulePath;
    if (curJsModulePath.empty() || newJsModuleUri.empty()) {
        return newJsModulePath;
    }

    std::string normalizeUri = newJsModuleUri;
    std::replace(normalizeUri.begin(), normalizeUri.end(), '\\', '/');

    switch (normalizeUri[0]) {
        case '.': {
            newJsModulePath = MakeNewJsModulePath(curJsModulePath, normalizeUri);
            break;
        }
        case '@': {
            newJsModulePath = ParseOhmUri(bundleName, curJsModulePath, normalizeUri);
            if (newJsModulePath.empty()) {
                newJsModulePath = FindNpmPackage(curJsModulePath, normalizeUri);
            }
            break;
        }
        default: {
            newJsModulePath = FindNpmPackage(curJsModulePath, normalizeUri);
            break;
        }
    }

    FixExtName(newJsModulePath);
    return newJsModulePath;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
