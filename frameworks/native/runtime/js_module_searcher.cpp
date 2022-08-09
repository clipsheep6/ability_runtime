/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_module_searcher.h"

#include <algorithm>
#include <fstream>
#include <vector>

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DEFAULT_BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";
constexpr char BUNDLES_INSTALL_PATH[] = "/data/bundles/";

constexpr char PREFIX_BUNDLE[] = "@bundle:";
constexpr char PREFIX_MODULE[] = "@module:";
constexpr char PREFIX_LOCAL[] = "@local:";

constexpr char NPM_PATH_SEGMENT[] = "node_modules";

constexpr char NPM_ENTRY_FILE[] = "index.abc";
constexpr char NPM_ENTRY_LINK[] = "entry.txt";

constexpr char EXT_NAME_ABC[] = ".abc";
constexpr char EXT_NAME_ETS[] = ".ets";
constexpr char EXT_NAME_TS[] = ".ts";
constexpr char EXT_NAME_JS[] = ".js";

constexpr size_t MAX_NPM_LEVEL = 1;

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
}

std::string JsModuleSearcher::operator()(const std::string& curJsModulePath, const std::string& newJsModuleUri) const
{
    return ParseJsModuleUri(curJsModulePath, newJsModuleUri);
}

void JsModuleSearcher::FixExtName(std::string& path)
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

std::string JsModuleSearcher::GetInstallPath(const std::string& curJsModulePath, bool module)
{
    size_t pos = std::string::npos;
    if (StringStartWith(curJsModulePath, DEFAULT_BUNDLE_INSTALL_PATH, sizeof(DEFAULT_BUNDLE_INSTALL_PATH) - 1)) {
        pos = sizeof(DEFAULT_BUNDLE_INSTALL_PATH) - 1 - 1;
    } else {
        if (!StringStartWith(curJsModulePath, BUNDLES_INSTALL_PATH, sizeof(BUNDLES_INSTALL_PATH) - 1)) {
            return std::string();
        }

        pos = curJsModulePath.find('/', sizeof(BUNDLES_INSTALL_PATH) - 1);
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

std::string JsModuleSearcher::MakeNewJsModulePath(
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
    char path[PATH_MAX];
    std::string jsModulePath = moduleInstallPath + JoinString(pathVector, '/');
    if (jsModulePath.size() >= PATH_MAX) {
        return std::string();
    }
    if (realpath(jsModulePath.c_str(), path) != nullptr) {
        return std::string(path);
    }
    return std::string();
}

std::string JsModuleSearcher::FindNpmPackageInPath(const std::string& npmPath)
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
    return npmPath + '/' + StripString(path);
}

std::string JsModuleSearcher::FindNpmPackageInTopLevel(
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

std::string JsModuleSearcher::FindNpmPackage(const std::string& curJsModulePath, const std::string& npmPackage)
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

std::string JsModuleSearcher::ParseOhmUri(const std::string& curJsModulePath, const std::string& newJsModuleUri) const
{
    std::string moduleInstallPath;
    std::vector<std::string> pathVector;
    size_t index = 0;

    if (StringStartWith(newJsModuleUri, PREFIX_BUNDLE, sizeof(PREFIX_BUNDLE) - 1)) {
        SplitString(newJsModuleUri, pathVector, sizeof(PREFIX_BUNDLE) - 1);

        // Uri should have atleast 3 segments
        if (pathVector.size() < 3) {
            return std::string();
        }

        const auto& bundleName = pathVector[index++];
        if (bundleName == bundleName_) {
            moduleInstallPath = DEFAULT_BUNDLE_INSTALL_PATH;
        } else {
            moduleInstallPath = BUNDLES_INSTALL_PATH;
            moduleInstallPath.append(bundleName).append("/");
        }
        moduleInstallPath.append(pathVector[index++]).append("/");
    } else if (StringStartWith(newJsModuleUri, PREFIX_MODULE, sizeof(PREFIX_MODULE) - 1)) {
        SplitString(newJsModuleUri, pathVector, sizeof(PREFIX_MODULE) - 1);

        // Uri should have atleast 2 segments
        if (pathVector.size() < 2) {
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

std::string JsModuleSearcher::ParseJsModuleUri(const std::string& curJsModulePath, const std::string& newJsModuleUri) const
{
    HILOG_DEBUG("Search JS module ParseJsModuleUri (%{public}s, %{public}s) begin", curJsModulePath.c_str(),
        newJsModuleUri.c_str());

    std::string newJsModulePath;
    if (curJsModulePath.empty() || newJsModuleUri.empty()) {
        return newJsModulePath;
    }
    std::string normalizeUri = newJsModuleUri;
    replace(normalizeUri.begin(), normalizeUri.end(), '\\', '/');

    switch (normalizeUri[0]) {
        case '.': {
            newJsModulePath = MakeNewJsModulePath(curJsModulePath, normalizeUri);
            break;
        }
        case '@': {
            newJsModulePath = ParseOhmUri(curJsModulePath, normalizeUri);
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
    HILOG_DEBUG("Search JS module ParseJsModuleUri (%{public}s, %{public}s) => %{public}s end",
        curJsModulePath.c_str(), normalizeUri.c_str(), newJsModulePath.c_str());
    return newJsModulePath;
}

bool JsModuleSearcher::GetABCFileBuffer(
    const std::string& curJsModulePath, const std::string& newJsModuleUri, std::ostream &dest) const
{
    std::string newJsModulePath = ParseJsModuleUri(curJsModulePath, newJsModuleUri);

    if (!GetABCFile(hapPath_, newJsModulePath, dest)) {
        HILOG_ERROR("Get abc file failed");
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS