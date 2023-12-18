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

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "application_context_manager.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_ability_auto_startup_callback.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
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

#define THROW_RETURN(env, errorMsg, errorCode) \
    do {                                                  \
        HILOG_ERROR(errorMsg);                            \
        AbilityRuntimeErrorUtil::Throw(env, errorCode);   \
        return CreateJsUndefined(env);                    \
    } while (0)
}  // namespace

napi_value JsApplicationContextUtils::CreateBundleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateBundleContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateBundleContext(napi_env env, NapiCallbackInfo& info)
{
    if (!CheckCallerIsSystemApp()) {
        THROW_RETURN(env, "Not system app", ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
    }

    if (info.argc == 0) {
        THROW_RETURN(env, "Not enough arguments", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        THROW_RETURN(env, "Invalid context", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        THROW_RETURN(env, "Parse bundleName failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    auto bundleContext = applicationContext->CreateBundleContext(bundleName);
    if (!bundleContext) {
        THROW_RETURN(env, "BundleContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    napi_value value = CreateJsBaseContext(env, bundleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        THROW_RETURN(env, "Invalid systemModule", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        THROW_RETURN(env, "Get context native object failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        }, nullptr, nullptr);
    return contextObj;
}

napi_value JsApplicationContextUtils::SwitchArea(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSwitchArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSwitchArea(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int mode = 0;
    if (!ConvertFromJsValue(env, info.argv[0], mode)) {
        HILOG_ERROR("Parse mode failed");
        return CreateJsUndefined(env);
    }

    applicationContext->SwitchArea(mode);

    napi_value object = info.thisVar;
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("Check type failed");
        return CreateJsUndefined(env);
    }
    BindNativeProperty(env, object, "cacheDir", GetCacheDir);
    BindNativeProperty(env, object, "tempDir", GetTempDir);
    BindNativeProperty(env, object, "resourceDir", GetResourceDir);
    BindNativeProperty(env, object, "filesDir", GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", GetBundleCodeDir);
    return CreateJsUndefined(env);
}


napi_value JsApplicationContextUtils::CreateModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateModuleContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateModuleContext(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        HILOG_DEBUG("Parse inner module name.");
        if (!ConvertFromJsValue(env, info.argv[0], moduleName)) {
            THROW_RETURN(env, "Parse moduleName failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        }
        moduleContext = applicationContext->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
            THROW_RETURN(env, "Parse moduleName failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        }
        if (!CheckCallerIsSystemApp()) {
            THROW_RETURN(env, "Not system app", ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        }
        HILOG_INFO("Parse outer module name.");
        moduleContext = applicationContext->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        THROW_RETURN(env, "Failed to create module context", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    napi_value value = CreateJsBaseContext(env, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        THROW_RETURN(env, "Invalid systemModule", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        THROW_RETURN(env, "Failed to get context native object", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        }, nullptr, nullptr);
    return contextObj;
}

napi_value JsApplicationContextUtils::CreateModuleResourceManager(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateModuleResourceManager, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateModuleResourceManager(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        THROW_RETURN(env, "Invalid applicationContext", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        THROW_RETURN(env, "Parse bundleName failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        THROW_RETURN(env, "Parse moduleName failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    if (!CheckCallerIsSystemApp()) {
        THROW_RETURN(env, "Not system app", ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
    }
    auto resourceManager = applicationContext->CreateModuleResourceManager(bundleName, moduleName);
    if (resourceManager == nullptr) {
        THROW_RETURN(env, "Failed to create resourceManager", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    auto jsResourceManager = CreateJsResourceManager(env, resourceManager, nullptr);
    return jsResourceManager;
}

napi_value JsApplicationContextUtils::GetArea(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetArea(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    int area = applicationContext->GetArea();
    return CreateJsValue(env, area);
}

napi_value JsApplicationContextUtils::GetCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetCacheDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCacheDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetTempDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetTempDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetTempDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetResourceDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetResourceDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetResourceDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetResourceDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDistributedFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetDistributedFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDistributedFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetDatabaseDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetPreferencesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::GetGroupDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetGroupDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::OnGetGroupDir(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::string groupId;
    if (!ConvertFromJsValue(env, info.argv[0], groupId)) {
        THROW_RETURN(env, "Parse groupId failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    HILOG_DEBUG("Get Group Dir");
    auto complete = [applicationContext = applicationContext_, groupId]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        std::string path = context->GetGroupDir(groupId);
        task.ResolveWithNoError(env, CreateJsValue(env, path));
    };

    napi_value lastParam = (info.argc == ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnGetGroupDir",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetBundleCodeDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::KillProcessBySelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnKillProcessBySelf, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnKillProcessBySelf(napi_env env, NapiCallbackInfo& info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    HILOG_DEBUG("kill self process");
    NapiAsyncTask::CompleteCallback complete =
        [applicationContext = applicationContext_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = applicationContext.lock();
            if (!context) {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                    "applicationContext if already released."));
                return;
            }
            context->KillProcessBySelf();
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc = ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnkillProcessBySelf",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::SetColorMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetColorMode, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetColorMode(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    // only support one params
    if (info.argc == ARGC_ZERO) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int32_t colorMode = 0;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], colorMode)) {
        HILOG_ERROR("Parse colorMode failed");
        return CreateJsUndefined(env);
    }
    applicationContext->SetColorMode(colorMode);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetLanguage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetLanguage, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetLanguage(napi_env env, NapiCallbackInfo& info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string language;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], language)) {
        THROW_RETURN(env, "Parse language failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    applicationContext->SetLanguage(language);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::ClearUpApplicationData(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnClearUpApplicationData, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnClearUpApplicationData(napi_env env, NapiCallbackInfo &info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    NapiAsyncTask::CompleteCallback complete =
        [applicationContext = applicationContext_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto context = applicationContext.lock();
            if (!context) {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                    "applicationContext if already released."));
                return;
            }
            context->ClearUpApplicationData();
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsApplicationContextUtils::OnClearUpApplicationData",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::GetRunningProcessInformation(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetRunningProcessInformation, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetRunningProcessInformation(napi_env env, NapiCallbackInfo& info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    HILOG_DEBUG("Get Process Info");
    auto complete = [applicationContext = applicationContext_](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        AppExecFwk::RunningProcessInfo processInfo;
        auto ret = context->GetProcessRunningInformation(processInfo);
        if (ret == 0) {
            napi_value object = nullptr;
            napi_create_object(env, &object);
            napi_set_named_property(env, object, "processName", CreateJsValue(env, processInfo.processName_));
            napi_set_named_property(env, object, "pid", CreateJsValue(env, processInfo.pid_));
            napi_set_named_property(env, object, "uid", CreateJsValue(env, processInfo.uid_));
            napi_set_named_property(env, object, "bundleNames", CreateNativeArray(env, processInfo.bundleNames));
            napi_set_named_property(env, object,
                "state", CreateJsValue(env, ConvertToJsAppProcessState(processInfo.state_, processInfo.isFocused)));
            napi_value array = nullptr;
            napi_create_array_with_length(env, 1, &array);
            if (array == nullptr) {
                HILOG_ERROR("Initiate array failed.");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                    "Initiate array failed."));
            } else {
                napi_set_element(env, array, 0, object);
                task.ResolveWithNoError(env, array);
            }
        } else {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Get process infos failed."));
        }
    };

    napi_value lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnGetRunningProcessInformation",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

void JsApplicationContextUtils::Finalizer(napi_env env, void *data, void *hint)
{
    HILOG_INFO("called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils *>(data));
}

napi_value JsApplicationContextUtils::RegisterAbilityLifecycleCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnRegisterAbilityLifecycleCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::UnregisterAbilityLifecycleCallback(
    napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnUnregisterAbilityLifecycleCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRegisterAbilityLifecycleCallback(
    napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return CreateJsUndefined(env);
    }
    if (callback_ != nullptr) {
        HILOG_DEBUG("callback_ is not nullptr.");
        return CreateJsValue(env, callback_->Register(info.argv[0]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    int32_t callbackId = callback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
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
        napi_get_value_int32(env, info.argv[INDEX_ZERO], &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    NapiAsyncTask::CompleteCallback complete = [callbackWeak, callbackId, errCode](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            if (errCode != 0) {
                task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                HILOG_ERROR("callback is nullptr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnUnregisterAbilityLifecycleCallback begin");
            if (!callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::RegisterEnvironmentCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnRegisterEnvironmentCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::UnregisterEnvironmentCallback(
    napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnUnregisterEnvironmentCallback, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRegisterEnvironmentCallback(
    napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        return CreateJsUndefined(env);
    }
    if (envCallback_ != nullptr) {
        HILOG_DEBUG("envCallback_ is not nullptr.");
        return CreateJsValue(env, envCallback_->Register(info.argv[0]));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(env);
    int32_t callbackId = envCallback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    HILOG_DEBUG("end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnUnregisterEnvironmentCallback(
    napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(env, info.argv[INDEX_ZERO], &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    NapiAsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId, errCode](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            if (errCode != 0) {
                task.Reject(env, CreateJsError(env, errCode, "Invalidate params."));
                return;
            }
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                HILOG_ERROR("env_callback is nullptr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "env_callback is nullptr"));
                return;
            }

            HILOG_DEBUG("begin");
            if (!env_callback->UnRegister(callbackId)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "call UnRegister failed!"));
                return;
            }

            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnUnregisterEnvironmentCallback", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::On(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnOn, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::Off(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnOff, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnOn(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("called");

    if (info.argc != ARGC_TWO) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        THROW_RETURN(env, "Param0 is invalid", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        THROW_RETURN(env, "Convert type failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (type == "abilityLifecycle") {
        return OnOnAbilityLifecycle(env, info, false);
    }
    if (type == "abilityLifecycleEvent") {
        return OnOnAbilityLifecycle(env, info, true);
    }
    if (type == "environment") {
        return OnOnEnvironment(env, info, false);
    }
    if (type == "environmentEvent") {
        return OnOnEnvironment(env, info, true);
    }
    if (type == "applicationStateChange") {
        return OnOnApplicationStateChange(env, info);
    }
    if (type == "abilityAutoStartup") {
        return OnRegisterAutoStartupCallback(env, info);
    }
    THROW_RETURN(env, "On function type not match", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

napi_value JsApplicationContextUtils::OnOff(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("called");
    if (info.argc < ARGC_ONE) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        THROW_RETURN(env, "Param0 is invalid", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        THROW_RETURN(env, "Convert type failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (type == "applicationStateChange") {
        return OnOffApplicationStateChange(env, info);
    }

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        THROW_RETURN(env, "Not enough params", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    int32_t callbackId = -1;
    if (CheckTypeForNapiValue(env, info.argv[1], napi_number)) {
        napi_get_value_int32(env, info.argv[1], &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }

    if (type == "abilityLifecycle") {
        return OnOffAbilityLifecycle(env, info, callbackId);
    }
    if (type == "abilityLifecycleEvent") {
        return OnOffAbilityLifecycleEventSync(env, info, callbackId);
    }
    if (type == "environment") {
        return OnOffEnvironment(env, info, callbackId);
    }
    if (type == "environmentEvent") {
        return OnOffEnvironmentEventSync(env, info, callbackId);
    }
    if (type == "abilityAutoStartup") {
        return OnUnregisterAutoStartupCallback(env, info);
    }
    THROW_RETURN(env, "Off function type not match", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

napi_value JsApplicationContextUtils::OnOnAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info, bool isSync)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (callback_ != nullptr) {
        HILOG_DEBUG("callback_ is not nullptr.");
        return CreateJsValue(env, callback_->Register(info.argv[1], isSync));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    int32_t callbackId = callback_->Register(info.argv[1], isSync);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnOffAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    NapiAsyncTask::CompleteCallback complete = [callbackWeak, callbackId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                HILOG_ERROR("callback is nullptr");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnOffAbilityLifecycle begin");
            if (!callback->UnRegister(callbackId, false)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnOffAbilityLifecycle", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnOffAbilityLifecycleEventSync(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    if (callback_ == nullptr) {
        THROW_RETURN(env, "Callback is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    if (!callback_->UnRegister(callbackId, true)) {
        THROW_RETURN(env, "Call UnRegister failed", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnEnvironment(
    napi_env env, NapiCallbackInfo& info, bool isSync)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }

    if (envCallback_ != nullptr) {
        HILOG_DEBUG("envCallback_ is not nullptr.");
        return CreateJsValue(env, envCallback_->Register(info.argv[1], isSync));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(env);
    int32_t callbackId = envCallback_->Register(info.argv[1], isSync);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    HILOG_DEBUG("OnOnEnvironment is end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnOffEnvironment(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    NapiAsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                HILOG_ERROR("env_callback is nullptr");
                task.Reject(env,
                    CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                        "env_callback is nullptr"));
                return;
            }

            HILOG_DEBUG("OnOffEnvironment begin");
            if (!env_callback->UnRegister(callbackId, false)) {
                HILOG_ERROR("call UnRegister failed!");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "call UnRegister failed!"));
                return;
            }

            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
    napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnOffEnvironment", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnOffEnvironmentEventSync(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    HILOG_DEBUG("called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    if (envCallback_ == nullptr) {
        THROW_RETURN(env, "Callback is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    if (!envCallback_->UnRegister(callbackId, true)) {
        THROW_RETURN(env, "Call UnRegister failed", ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnApplicationStateChange(
    napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called.");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ != nullptr) {
        applicationStateCallback_->Register(info.argv[INDEX_ONE]);
        return CreateJsUndefined(env);
    }

    applicationStateCallback_ = std::make_shared<JsApplicationStateChangeCallback>(env);
    applicationStateCallback_->Register(info.argv[INDEX_ONE]);
    applicationContext->RegisterApplicationStateChangeCallback(applicationStateCallback_);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOffApplicationStateChange(
    napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called.");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ == nullptr) {
        THROW_RETURN(env, "ApplicationStateCallback_ is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (info.argc == ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        applicationStateCallback_->UnRegister();
    } else if (!applicationStateCallback_->UnRegister(info.argv[INDEX_ONE])) {
        THROW_RETURN(env, "Call UnRegister failed", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    if (applicationStateCallback_->IsEmpty()) {
        applicationStateCallback_.reset();
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetApplicationContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetApplicationContext(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("called");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        THROW_RETURN(env, "ApplicationContext is nullptr", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }

    napi_value value = CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        THROW_RETURN(env, "Invalid systemModule", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        THROW_RETURN(env, "Failed to get context native object", ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr, nullptr);
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

napi_value JsApplicationContextUtils::CreateJsApplicationContext(napi_env env)
{
    HILOG_DEBUG("start");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        return nullptr;
    }

    std::shared_ptr<ApplicationContext> applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        return object;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    SetNamedNativePointer(env, object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", CreateJsApplicationInfo(env, *appInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    std::shared_ptr<Context> context = std::dynamic_pointer_cast<Context>(applicationContext);
    if (resourceManager != nullptr) {
        napi_set_named_property(env, object, "resourceManager", CreateJsResourceManager(env, resourceManager, context));
    }

    BindNativeApplicationContext(env, object);
    return object;
}

void JsApplicationContextUtils::BindNativeApplicationContext(napi_env env, napi_value object)
{
    BindNativeProperty(env, object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(env, object, "resourceDir", JsApplicationContextUtils::GetResourceDir);
    BindNativeProperty(env, object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(env, object, "distributedFilesDir", JsApplicationContextUtils::GetDistributedFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(env, object, "registerAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::RegisterAbilityLifecycleCallback);
    BindNativeFunction(env, object, "unregisterAbilityLifecycleCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterAbilityLifecycleCallback);
    BindNativeFunction(env, object, "registerEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::RegisterEnvironmentCallback);
    BindNativeFunction(env, object, "unregisterEnvironmentCallback", MD_NAME,
        JsApplicationContextUtils::UnregisterEnvironmentCallback);
    BindNativeFunction(env, object, "createBundleContext", MD_NAME, JsApplicationContextUtils::CreateBundleContext);
    BindNativeFunction(env, object, "switchArea", MD_NAME, JsApplicationContextUtils::SwitchArea);
    BindNativeFunction(env, object, "getArea", MD_NAME, JsApplicationContextUtils::GetArea);
    BindNativeFunction(env, object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
    BindNativeFunction(env, object, "createModuleResourceManager", MD_NAME,
        JsApplicationContextUtils::CreateModuleResourceManager);
    BindNativeFunction(env, object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(env, object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(env, object, "getApplicationContext", MD_NAME,
        JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(env, object, "killAllProcesses", MD_NAME, JsApplicationContextUtils::KillProcessBySelf);
    BindNativeFunction(env, object, "setColorMode", MD_NAME, JsApplicationContextUtils::SetColorMode);
    BindNativeFunction(env, object, "setLanguage", MD_NAME, JsApplicationContextUtils::SetLanguage);
    BindNativeFunction(env, object, "clearUpApplicationData", MD_NAME,
        JsApplicationContextUtils::ClearUpApplicationData);
    BindNativeFunction(env, object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getGroupDir", MD_NAME,
        JsApplicationContextUtils::GetGroupDir);
    BindNativeFunction(env, object, "setAutoStartup", MD_NAME, JsApplicationContextUtils::SetAutoStartup);
    BindNativeFunction(env, object, "cancelAutoStartup", MD_NAME, JsApplicationContextUtils::CancelAutoStartup);
    BindNativeFunction(env, object, "isAutoStartup", MD_NAME, JsApplicationContextUtils::IsAutoStartup);
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

napi_value JsApplicationContextUtils::SetAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetAutoStartup, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::CancelAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnCancelAutoStartup, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::IsAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnIsAutoStartup, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRegisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("called.");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != "abilityAutoStartup") {
        HILOG_ERROR("Parse type failed.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (jsAutoStartupCallback_ == nullptr) {
        jsAutoStartupCallback_ = new (std::nothrow) JsAbilityAutoStartupCallBack(env);
        if (jsAutoStartupCallback_ == nullptr) {
            HILOG_ERROR("jsAutoStartupCallback_ is nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        auto ret = AAFwk::AbilityManagerClient::GetInstance()->RegisterAutoStartupCallback(
            jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            jsAutoStartupCallback_ = nullptr;
            HILOG_ERROR("Register auto start up listener error[%{public}d].", ret);
            ThrowError(env, GetJsErrorCodeByNativeError(ret));
            return CreateJsUndefined(env);
        }
    }

    jsAutoStartupCallback_->Register(info.argv[INDEX_ONE]);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnUnregisterAutoStartupCallback(
    napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != "abilityAutoStartup") {
        HILOG_ERROR("Failed to parse type.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (jsAutoStartupCallback_ == nullptr) {
        HILOG_ERROR("jsAutoStartupCallback_ is nullptr.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    auto callback = info.argc > ARGC_ONE ? info.argv[INDEX_ONE] : CreateJsUndefined(env);
    jsAutoStartupCallback_->UnRegister(callback);
    if (jsAutoStartupCallback_->IsCallbacksEmpty()) {
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->UnregisterAutoStartupCallback(
            jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            ThrowError(env, GetJsErrorCodeByNativeError(ret));
        }
        jsAutoStartupCallback_ = nullptr;
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnSetAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info.argv[INDEX_ZERO], autoStartupInfo)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [autoStartupInfo, ret = retVal] () {
        if (ret == nullptr) {
            HILOG_ERROR("The param is invalid.");
            return;
        }
        *ret = AAFwk::AbilityManagerClient::GetInstance()->SetAutoStartup(autoStartupInfo);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (ret == nullptr) {
            HILOG_ERROR("The param is invalid.");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(AAFwk::INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            HILOG_ERROR("Failed error:%{public}d.", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnSetAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnCancelAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info.argv[INDEX_ZERO], autoStartupInfo)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [autoStartupInfo, ret = retVal] () {
        if (ret == nullptr) {
            HILOG_ERROR("The param is invalid.");
            return;
        }
        *ret = AAFwk::AbilityManagerClient::GetInstance()->CancelAutoStartup(autoStartupInfo);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (ret == nullptr) {
            HILOG_ERROR("The param is invalid.");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(AAFwk::INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            HILOG_ERROR("Failed error:%{public}d.", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnCancelAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnIsAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AutoStartupInfo autoStartupInfo;
    if (!UnwrapAutoStartupInfo(env, info.argv[INDEX_ZERO], autoStartupInfo)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    auto isAutoStartup = std::make_shared<bool>(false);
    NapiAsyncTask::ExecuteCallback execute = [autoStartupInfo, ret = retVal, isFlag = isAutoStartup] () {
        if (ret == nullptr || isFlag == nullptr) {
            HILOG_ERROR("The param is invalid.");
            return;
        }
        *ret = AAFwk::AbilityManagerClient::GetInstance()->IsAutoStartup(autoStartupInfo, *isFlag);
    };

    NapiAsyncTask::CompleteCallback complete =
        [ret = retVal, isFlag = isAutoStartup](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (ret == nullptr || isFlag == nullptr) {
            HILOG_ERROR("The param is invalid.");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(AAFwk::INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            HILOG_ERROR("Failed error:%{public}d.", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, *isFlag));
    };

    napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnIsAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
