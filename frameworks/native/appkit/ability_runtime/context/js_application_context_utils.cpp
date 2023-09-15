/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "js_application_context_utils.h"

#include <map>

#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "application_context_manager.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr int32_t ERROR_CODE_ONE = 1;
const char* MD_NAME = "JsApplicationContextUtils";
}  // namespace

NativeValue *JsApplicationContextUtils::CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateBundleContext(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnCreateBundleContext(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (!CheckCallerIsSystemApp()) {
        HILOG_ERROR("This application is not system-app, can not use system-api");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return engine.CreateUndefined();
    }

    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string bundleName;
    if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
        HILOG_ERROR("Parse bundleName failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    auto bundleContext = applicationContext->CreateBundleContext(bundleName);
    if (!bundleContext) {
        HILOG_ERROR("bundleContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsBaseContext(engine, bundleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue *JsApplicationContextUtils::SwitchArea(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::SwitchArea is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnSwitchArea(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }

    int mode = 0;
    if (!ConvertFromJsValue(engine, info.argv[0], mode)) {
        HILOG_ERROR("Parse mode failed");
        return engine.CreateUndefined();
    }

    applicationContext->SwitchArea(mode);

    NativeValue *thisVar = info.thisVar;
    NativeObject *object = ConvertNativeValueTo<NativeObject>(thisVar);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return engine.CreateUndefined();
    }
    BindNativeProperty(*object, "cacheDir", GetCacheDir);
    BindNativeProperty(*object, "tempDir", GetTempDir);
    BindNativeProperty(*object, "filesDir", GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", GetBundleCodeDir);
    return engine.CreateUndefined();
}


NativeValue* JsApplicationContextUtils::CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateModuleContext(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    if (!ConvertFromJsValue(engine, info.argv[1], moduleName)) {
        HILOG_INFO("Parse inner module name.");
        if (!ConvertFromJsValue(engine, info.argv[0], moduleName)) {
            HILOG_ERROR("Parse moduleName failed");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return engine.CreateUndefined();
        }
        moduleContext = applicationContext->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(engine, info.argv[0], bundleName)) {
            HILOG_ERROR("Parse bundleName failed");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return engine.CreateUndefined();
        }
        if (!CheckCallerIsSystemApp()) {
            HILOG_ERROR("This application is not system-app, can not use system-api");
            AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
            return engine.CreateUndefined();
        }
        HILOG_INFO("Parse outer module name.");
        moduleContext = applicationContext->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        HILOG_ERROR("failed to create module context.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsBaseContext(engine, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnCreateModuleContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* JsApplicationContextUtils::GetArea(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetArea is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetArea(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetArea(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    int area = applicationContext->GetArea();
    return engine.CreateNumber(area);
}

NativeValue *JsApplicationContextUtils::GetCacheDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetCacheDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetCacheDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetCacheDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetTempDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetTempDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetTempDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetTempDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetFilesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetFilesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetDistributedFilesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDistributedFilesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDistributedFilesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetDistributedFilesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDistributedFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetDatabaseDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDatabaseDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDatabaseDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetDatabaseDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDatabaseDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::GetPreferencesDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetPreferencesDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetPreferencesDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::GetGroupDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetGroupDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetPreferencesDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::OnGetGroupDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string groupId;
    if (!ConvertFromJsValue(engine, info.argv[0], groupId)) {
        HILOG_ERROR("Parse groupId failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    HILOG_DEBUG("Get Group Dir");
    auto complete = [applicationContext = applicationContext_, groupId]
        (NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        std::string path = context->GetGroupDir(groupId);
        task.ResolveWithNoError(engine, CreateJsValue(engine, path));
    };

    NativeValue* lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnGetGroupDir",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::GetBundleCodeDir(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("JsApplicationContextUtils::GetBundleCodeDir is called");
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetBundleCodeDir(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetBundleCodeDir(NativeEngine &engine, NativeCallbackInfo &info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue *JsApplicationContextUtils::KillProcessBySelf(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnKillProcessBySelf(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnKillProcessBySelf(NativeEngine &engine, NativeCallbackInfo &info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    HILOG_DEBUG("kill self process");
    AsyncTask::CompleteCallback complete =
        [applicationContext = applicationContext_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = applicationContext.lock();
            if (!context) {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                    "applicationContext if already released."));
                return;
            }
            context->KillProcessBySelf();
            task.ResolveWithNoError(engine, engine.CreateUndefined());
        };
    NativeValue* lastParam = (info.argc = ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnkillProcessBySelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::SetColorMode(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnSetColorMode(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnSetColorMode(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called");
    // only support one params
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }

    int32_t colorMode = 0;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], colorMode)) {
        HILOG_ERROR("Parse colorMode failed");
        return engine.CreateUndefined();
    }
    applicationContext->SetColorMode(colorMode);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::SetLanguage(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnSetLanguage(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnSetLanguage(NativeEngine &engine, NativeCallbackInfo &info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string language;
    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], language)) {
        HILOG_ERROR("Parse language failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    applicationContext->SetLanguage(language);
    return engine.CreateUndefined();
}


NativeValue *JsApplicationContextUtils::GetRunningProcessInformation(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetRunningProcessInformation(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnGetRunningProcessInformation(NativeEngine &engine, NativeCallbackInfo &info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    HILOG_DEBUG("Get Process Info");
    auto complete = [applicationContext = applicationContext_](NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        AppExecFwk::RunningProcessInfo processInfo;
        auto ret = context->GetProcessRunningInformation(processInfo);
        if (ret == 0) {
            NativeValue* objValue = engine.CreateObject();
            NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
            object->SetProperty("processName", CreateJsValue(engine, processInfo.processName_));
            object->SetProperty("pid", CreateJsValue(engine, processInfo.pid_));
            object->SetProperty("uid", CreateJsValue(engine, processInfo.uid_));
            object->SetProperty("bundleNames", CreateNativeArray(engine, processInfo.bundleNames));
            object->SetProperty(
                "state", CreateJsValue(engine, ConvertToJsAppProcessState(processInfo.state_, processInfo.isFocused)));
            object->SetProperty("isContinuousTask", CreateJsValue(engine, processInfo.isContinuousTask));
            object->SetProperty("isKeepAlive", CreateJsValue(engine, processInfo.isKeepAlive));
            object->SetProperty("isFocused", CreateJsValue(engine, processInfo.isFocused));
            NativeValue* arrayValue = engine.CreateArray(1);
            NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
            if (array == nullptr) {
                HILOG_ERROR("Initiate array failed.");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                    "Initiate array failed."));
            } else {
                array->SetElement(0, objValue);
                task.ResolveWithNoError(engine, arrayValue);
            }
        } else {
            task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Get process infos failed."));
        }
    };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnGetRunningProcessInformation",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

void JsApplicationContextUtils::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("JsApplicationContextUtils::Finalizer is called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

NativeValue *JsApplicationContextUtils::RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnRegisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnUnregisterAbilityLifecycleCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnRegisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return engine.CreateUndefined();
    }
    if (callback_ != nullptr) {
        HILOG_DEBUG("callback_ is not nullptr.");
        return engine.CreateNumber(callback_->Register(info.argv[0]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(&engine);
    int32_t callbackId = callback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnRegisterAbilityLifecycleCallback is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnUnregisterAbilityLifecycleCallback is called");
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("OnUnregisterAbilityLifecycleCallback, Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    AsyncTask::CompleteCallback complete = [callbackWeak, callbackId, errCode](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                HILOG_ERROR("callback is nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnUnregisterAbilityLifecycleCallback begin");
            if (!callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::RegisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnRegisterEnvironmentCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::UnregisterEnvironmentCallback(
    NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnUnregisterEnvironmentCallback(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnRegisterEnvironmentCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("OnRegisterEnvironmentCallback is called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return engine.CreateUndefined();
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return engine.CreateUndefined();
    }
    if (envCallback_ != nullptr) {
        HILOG_DEBUG("envCallback_ is not nullptr.");
        return engine.CreateNumber(envCallback_->Register(info.argv[0]));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(&engine);
    int32_t callbackId = envCallback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    HILOG_DEBUG("OnRegisterEnvironmentCallback is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnUnregisterEnvironmentCallback(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("OnUnregisterEnvironmentCallback is called");
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("OnUnregisterEnvironmentCallback, Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    AsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId, errCode](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (errCode != 0) {
                task.Reject(engine, CreateJsError(engine, errCode, "Invalidate params."));
                return;
            }
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                HILOG_ERROR("env_callback is nullptr");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "env_callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnUnregisterEnvironmentCallback begin");
            if (!env_callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterEnvironmentCallback", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::On(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnOn(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::Off(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnOff(*engine, *info) : nullptr;
}

NativeValue *JsApplicationContextUtils::OnOn(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("OnOn is called");

    if (info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (type == "abilityLifecycle") {
        return OnOnAbilityLifecycle(engine, info, false);
    }
    if (type == "abilityLifecycleEvent") {
        return OnOnAbilityLifecycle(engine, info, true);
    }
    if (type == "environment") {
        return OnOnEnvironment(engine, info, false);
    }
    if (type == "environmentEvent") {
        return OnOnEnvironment(engine, info, true);
    }
    if (type == "applicationStateChange") {
        return OnOnApplicationStateChange(engine, info);
    }
    HILOG_ERROR("on function type not match.");
    AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOff(NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_INFO("OnOff is called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (type == "applicationStateChange") {
        return OnOffApplicationStateChange(engine, info);
    }

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    int32_t callbackId = -1;
    if (info.argv[1]->TypeOf() == NATIVE_NUMBER) {
        napi_get_value_int32(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[1]), &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }

    if (type == "abilityLifecycle") {
        return OnOffAbilityLifecycle(engine, info, callbackId);
    }
    if (type == "abilityLifecycleEvent") {
        return OnOffAbilityLifecycleEventSync(engine, info, callbackId);
    }
    if (type == "environment") {
        return OnOffEnvironment(engine, info, callbackId);
    }
    if (type == "environmentEvent") {
        return OnOffEnvironmentEventSync(engine, info, callbackId);
    }
    HILOG_ERROR("off function type not match.");
    AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOnAbilityLifecycle(
    NativeEngine &engine, NativeCallbackInfo &info, bool isSync)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (callback_ != nullptr) {
        HILOG_DEBUG("callback_ is not nullptr.");
        return engine.CreateNumber(callback_->Register(info.argv[1], isSync));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(&engine);
    int32_t callbackId = callback_->Register(info.argv[1], isSync);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnOnAbilityLifecycle is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnOffAbilityLifecycle(
    NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    AsyncTask::CompleteCallback complete = [callbackWeak, callbackId](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                HILOG_ERROR("callback is nullptr");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnOffAbilityLifecycle begin");
            if (!callback->UnRegister(callbackId, false)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnOffAbilityLifecycle", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::OnOffAbilityLifecycleEventSync(
    NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }
    if (callback_ == nullptr) {
        HILOG_ERROR("callback is nullptr");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }
    if (!callback_->UnRegister(callbackId, true)) {
        HILOG_ERROR("call UnRegister failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOnEnvironment(
    NativeEngine &engine, NativeCallbackInfo &info, bool isSync)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }

    if (envCallback_ != nullptr) {
        HILOG_DEBUG("envCallback_ is not nullptr.");
        return engine.CreateNumber(envCallback_->Register(info.argv[1], isSync));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(&engine);
    int32_t callbackId = envCallback_->Register(info.argv[1], isSync);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    HILOG_DEBUG("OnOnEnvironment is end");
    return engine.CreateNumber(callbackId);
}

NativeValue *JsApplicationContextUtils::OnOffEnvironment(
    NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    AsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                HILOG_ERROR("env_callback is nullptr");
                task.Reject(engine,
                    CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                        "env_callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnOffEnvironment begin");
            if (!env_callback->UnRegister(callbackId, false)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(engine, engine.CreateUndefined());
        };
    NativeValue *lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnOffEnvironment", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsApplicationContextUtils::OnOffEnvironmentEventSync(
    NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }
    if (envCallback_ == nullptr) {
        HILOG_ERROR("env_callback is nullptr");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }
    if (!envCallback_->UnRegister(callbackId, true)) {
        HILOG_ERROR("call UnRegister failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return engine.CreateUndefined();
    }
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOnApplicationStateChange(
    NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called.");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ != nullptr) {
        applicationStateCallback_->Register(info.argv[INDEX_ONE]);
        return engine.CreateUndefined();
    }

    applicationStateCallback_ = std::make_shared<JsApplicationStateChangeCallback>(&engine);
    applicationStateCallback_->Register(info.argv[INDEX_ONE]);
    applicationContext->RegisterApplicationStateChangeCallback(applicationStateCallback_);
    return engine.CreateUndefined();
}

NativeValue *JsApplicationContextUtils::OnOffApplicationStateChange(
    NativeEngine &engine, const NativeCallbackInfo &info)
{
    HILOG_DEBUG("called.");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ == nullptr) {
        HILOG_ERROR("ApplicationStateCallback_ is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argc == ARGC_ONE || info.argv[INDEX_ONE]->TypeOf() != NATIVE_OBJECT) {
        applicationStateCallback_->UnRegister();
    } else if (!applicationStateCallback_->UnRegister(info.argv[INDEX_ONE])) {
        HILOG_ERROR("call UnRegister failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (applicationStateCallback_->IsEmpty()) {
        applicationStateCallback_.reset();
    }
    return engine.CreateUndefined();
}

NativeValue* JsApplicationContextUtils::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("GetApplicationContext start");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsApplicationContext(engine);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnGetApplicationContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachApplicationContext,
        workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr);
    return contextObj;
}

bool JsApplicationContextUtils::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}

NativeValue* JsApplicationContextUtils::CreateJsApplicationContext(NativeEngine &engine)
{
    HILOG_DEBUG("CreateJsApplicationContext start");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        return objValue;
    }

    std::shared_ptr<ApplicationContext> applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        return objValue;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    SetNamedNativePointer(engine, *object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    std::shared_ptr<Context> context = std::dynamic_pointer_cast<Context>(applicationContext);
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager, context));
    }

    BindNativeApplicationContext(engine, object);
    return objValue;
}

void JsApplicationContextUtils::BindNativeApplicationContext(NativeEngine &engine, NativeObject* object)
{
    BindNativeProperty(*object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(*object, "distributedFilesDir", JsApplicationContextUtils::GetDistributedFilesDir);
    BindNativeProperty(*object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(engine, *object, "registerAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "unregisterAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    BindNativeFunction(engine, *object, "registerEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::RegisterEnvironmentCallback);
    BindNativeFunction(engine, *object, "unregisterEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterEnvironmentCallback);
    BindNativeFunction(engine, *object, "createBundleContext", MD_NAME, JsApplicationContextUtils::CreateBundleContext);
    BindNativeFunction(engine, *object, "switchArea", MD_NAME, JsApplicationContextUtils::SwitchArea);
    BindNativeFunction(engine, *object, "getArea", MD_NAME, JsApplicationContextUtils::GetArea);
    BindNativeFunction(engine, *object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
    BindNativeFunction(engine, *object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(engine, *object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(engine, *object, "getApplicationContext", MD_NAME,
        JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(engine, *object, "killAllProcesses", MD_NAME, JsApplicationContextUtils::KillProcessBySelf);
    BindNativeFunction(engine, *object, "setColorMode", MD_NAME, JsApplicationContextUtils::SetColorMode);
    BindNativeFunction(engine, *object, "setLanguage", MD_NAME, JsApplicationContextUtils::SetLanguage);
    BindNativeFunction(engine, *object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(engine, *object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(engine, *object, "getGroupDir", MD_NAME,
        JsApplicationContextUtils::GetGroupDir);
}

JsAppProcessState JsApplicationContextUtils::ConvertToJsAppProcessState(
    const AppExecFwk::AppProcessState &appProcessState, const bool &isFocused)
{
    JsAppProcessState processState;
    switch (appProcessState) {
        case AppExecFwk::AppProcessState::APP_STATE_CREATE:
        case AppExecFwk::AppProcessState::APP_STATE_READY:
            processState = STATE_CREATE;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_FOREGROUND:
            processState = isFocused ? STATE_ACTIVE : STATE_FOREGROUND;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_BACKGROUND:
            processState = STATE_BACKGROUND;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_TERMINATED:
        case AppExecFwk::AppProcessState::APP_STATE_END:
            processState = STATE_DESTROY;
            break;
        default:
            HILOG_ERROR("Process state is invalid.");
            processState = STATE_DESTROY;
            break;
    }
    return processState;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
