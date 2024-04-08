/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_ability_auto_startup_callback.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
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

napi_value JsApplicationContextUtils::CreateBundleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateBundleContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateBundleContext(napi_env env, NapiCallbackInfo& info)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "This application is not system-app, can not use system-api.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough arguments");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto bundleContext = applicationContext->CreateBundleContext(bundleName);
    if (!bundleContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    napi_value value = CreateJsBaseContext(env, bundleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(bundleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr bundle context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value JsApplicationContextUtils::SwitchArea(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSwitchArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSwitchArea(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int mode = 0;
    if (!ConvertFromJsValue(env, info.argv[0], mode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse mode failed");
        return CreateJsUndefined(env);
    }

    applicationContext->SwitchArea(mode);

    napi_value object = info.thisVar;
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Check type failed");
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
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGD(AAFwkTag::APPKIT, "Parse inner module name.");
        if (!ConvertFromJsValue(env, info.argv[0], moduleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return CreateJsUndefined(env);
        }
        moduleContext = applicationContext->CreateModuleContext(moduleName);
    } else {
        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return CreateJsUndefined(env);
        }
        if (!CheckCallerIsSystemApp()) {
            TAG_LOGE(AAFwkTag::APPKIT, "This application is not system-app, can not use system-api");
            AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::APPKIT, "Parse outer module name.");
        moduleContext = applicationContext->CreateModuleContext(bundleName, moduleName);
    }

    if (!moduleContext) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create module context.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    napi_value value = CreateJsBaseContext(env, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value JsApplicationContextUtils::CreateSystemHspModuleResourceManager(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateSystemHspModuleResourceManager, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateSystemHspModuleResourceManager(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string bundleName = "";
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string moduleName = "";
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGD(AAFwkTag::APPKIT, "Parse module name failed.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    int32_t retCode = applicationContext->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    if (resourceManager == nullptr && retCode == ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP);
        return CreateJsUndefined(env);
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    return CreateJsResourceManager(env, resourceManager, nullptr);
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
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    if (!ConvertFromJsValue(env, info.argv[0], bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, info.argv[1], moduleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "This application is not system-app, can not use system-api");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    auto resourceManager = applicationContext->CreateModuleResourceManager(bundleName, moduleName);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create resourceManager");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto jsResourceManager = CreateJsResourceManager(env, resourceManager, nullptr);
    return jsResourceManager;
}

napi_value JsApplicationContextUtils::GetArea(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetArea, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetArea(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    int area = applicationContext->GetArea();
    return CreateJsValue(env, area);
}

napi_value JsApplicationContextUtils::GetCacheDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetCacheDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCacheDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetTempDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetTempDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetTempDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetResourceDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetResourceDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetResourceDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetResourceDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetFilesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDistributedFilesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetDistributedFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDistributedFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetDatabaseDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetPreferencesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::GetGroupDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetGroupDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::OnGetGroupDir(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string groupId;
    if (!ConvertFromJsValue(env, info.argv[0], groupId)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse groupId failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::APPKIT, "Get Group Dir");
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

napi_value JsApplicationContextUtils::RestartApp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnRestartApp, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnRestartApp(napi_env env, NapiCallbackInfo& info)
{
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto errCode = applicationContext->RestartApp(want);
    if (errCode == ERR_OK) {
        return CreateJsUndefined(env);
    }
    if (errCode == ERR_INVALID_VALUE) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    } else if (errCode == AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_RESTART_APP_INCORRECT_ABILITY);
    } else if (errCode == AAFwk::ERR_RESTART_APP_FREQUENT) {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_RESTART_APP_FREQUENT);
    } else {
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
    }
    TAG_LOGE(AAFwkTag::APPKIT, "errCode is %{public}d.", errCode);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetBundleCodeDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
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
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "kill self process");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // only support one params
    if (info.argc == ARGC_ZERO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int32_t colorMode = 0;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], colorMode)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse colorMode failed");
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
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string language;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], language)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse language failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    applicationContext->SetLanguage(language);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::PreloadUIExtensionAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnPreloadUIExtensionAbility, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnPreloadUIExtensionAbility(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("OnPreloadUIExtensionAbility params error!");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Parse want failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(want);
    [applicationContext = applicationContext_](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        if (errcode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            HILOG_ERROR("JsApplicationContextUtils::OnPreloadUIExtensionAbility is failed %{public}d", errcode);
            task.Reject(env, CreateJsErrorByNativeErr(env, errcode));
        }
    };
    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSServiceExtensionContext::OnPreloadUIExtensionAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
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
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
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
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "Get Process Info");
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
                TAG_LOGE(AAFwkTag::APPKIT, "Initiate array failed.");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        return CreateJsUndefined(env);
    }
    if (callback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "callback_ is not nullptr.");
        return CreateJsValue(env, callback_->Register(info.argv[0]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    int32_t callbackId = callback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    TAG_LOGI(AAFwkTag::APPKIT, "end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnUnregisterAbilityLifecycleCallback(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "OnUnregisterAbilityLifecycleCallback, Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(env, info.argv[INDEX_ZERO], &callbackId);
        TAG_LOGD(AAFwkTag::APPKIT, "callbackId is %{public}d.", callbackId);
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
                TAG_LOGE(AAFwkTag::APPKIT, "callback is nullptr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "callback is nullptr"));
                return;
            }

            TAG_LOGD(AAFwkTag::APPKIT, "OnUnregisterAbilityLifecycleCallback begin");
            if (!callback->UnRegister(callbackId)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    // only support one params
    if (info.argc != ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params.");
        return CreateJsUndefined(env);
    }

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        return CreateJsUndefined(env);
    }
    if (envCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "envCallback_ is not nullptr.");
        return CreateJsValue(env, envCallback_->Register(info.argv[0]));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(env);
    int32_t callbackId = envCallback_->Register(info.argv[INDEX_ZERO]);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    TAG_LOGD(AAFwkTag::APPKIT, "end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnUnregisterEnvironmentCallback(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    int32_t errCode = 0;
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        errCode = ERROR_CODE_ONE;
    }
    int32_t callbackId = -1;
    if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        errCode = ERROR_CODE_ONE;
    } else {
        napi_get_value_int32(env, info.argv[INDEX_ZERO], &callbackId);
        TAG_LOGD(AAFwkTag::APPKIT, "callbackId is %{public}d.", callbackId);
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
                TAG_LOGE(AAFwkTag::APPKIT, "env_callback is nullptr");
                task.Reject(env, CreateJsError(env, ERROR_CODE_ONE, "env_callback is nullptr"));
                return;
            }

            TAG_LOGD(AAFwkTag::APPKIT, "begin");
            if (!env_callback->UnRegister(callbackId)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    if (info.argc != ARGC_TWO) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        TAG_LOGE(AAFwkTag::APPKIT, "convert type failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
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
    TAG_LOGE(AAFwkTag::APPKIT, "on function type not match.");
    AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOff(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        TAG_LOGE(AAFwkTag::APPKIT, "param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        TAG_LOGE(AAFwkTag::APPKIT, "convert type failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (type == "applicationStateChange") {
        return OnOffApplicationStateChange(env, info);
    }

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    int32_t callbackId = -1;
    if (CheckTypeForNapiValue(env, info.argv[1], napi_number)) {
        napi_get_value_int32(env, info.argv[1], &callbackId);
        TAG_LOGD(AAFwkTag::APPKIT, "callbackId is %{public}d.", callbackId);
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
    TAG_LOGE(AAFwkTag::APPKIT, "off function type not match.");
    AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info, bool isSync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (callback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "callback_ is not nullptr.");
        return CreateJsValue(env, callback_->Register(info.argv[1], isSync));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    int32_t callbackId = callback_->Register(info.argv[1], isSync);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    TAG_LOGI(AAFwkTag::APPKIT, "end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnOffAbilityLifecycle(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    NapiAsyncTask::CompleteCallback complete = [callbackWeak, callbackId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            auto callback = callbackWeak.lock();
            if (callback == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "callback is nullptr");
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                    "callback is nullptr"));
                return;
            }

            TAG_LOGD(AAFwkTag::APPKIT, "OnOffAbilityLifecycle begin");
            if (!callback->UnRegister(callbackId, false)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callback is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (!callback_->UnRegister(callbackId, true)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnEnvironment(
    napi_env env, NapiCallbackInfo& info, bool isSync)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }

    if (envCallback_ != nullptr) {
        TAG_LOGD(AAFwkTag::APPKIT, "envCallback_ is not nullptr.");
        return CreateJsValue(env, envCallback_->Register(info.argv[1], isSync));
    }
    envCallback_ = std::make_shared<JsEnvironmentCallback>(env);
    int32_t callbackId = envCallback_->Register(info.argv[1], isSync);
    applicationContext->RegisterEnvironmentCallback(envCallback_);
    TAG_LOGD(AAFwkTag::APPKIT, "OnOnEnvironment is end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::OnOffEnvironment(
    napi_env env, NapiCallbackInfo& info, int32_t callbackId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::weak_ptr<JsEnvironmentCallback> envCallbackWeak(envCallback_);
    NapiAsyncTask::CompleteCallback complete = [envCallbackWeak, callbackId](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            auto env_callback = envCallbackWeak.lock();
            if (env_callback == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "env_callback is nullptr");
                task.Reject(env,
                    CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                        "env_callback is nullptr"));
                return;
            }

            TAG_LOGD(AAFwkTag::APPKIT, "OnOffEnvironment begin");
            if (!env_callback->UnRegister(callbackId, false)) {
                TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (envCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env_callback is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    if (!envCallback_->UnRegister(callbackId, true)) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnApplicationStateChange(
    napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "called.");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
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
    TAG_LOGD(AAFwkTag::APPKIT, "called.");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ApplicationStateCallback_ is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (info.argc == ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        applicationStateCallback_->UnRegister();
    } else if (!applicationStateCallback_->UnRegister(info.argv[INDEX_ONE])) {
        TAG_LOGE(AAFwkTag::APPKIT, "call UnRegister failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
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
    TAG_LOGD(AAFwkTag::APPKIT, "called");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        TAG_LOGW(AAFwkTag::APPKIT, "applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    napi_value value = CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::APPKIT, "Finalizer for weak_ptr application context is called");
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
    TAG_LOGD(AAFwkTag::APPKIT, "start");
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
    BindNativeFunction(env, object, "createSystemHspModuleResourceManager", MD_NAME,
        JsApplicationContextUtils::CreateSystemHspModuleResourceManager);
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
    BindNativeFunction(env, object, "preloadUIExtensionAbility", MD_NAME,
        JsApplicationContextUtils::PreloadUIExtensionAbility);
    BindNativeFunction(env, object, "getProcessRunningInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "getGroupDir", MD_NAME,
        JsApplicationContextUtils::GetGroupDir);
    BindNativeFunction(env, object, "restartApp", MD_NAME,
        JsApplicationContextUtils::RestartApp);
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
            TAG_LOGE(AAFwkTag::APPKIT, "Process state is invalid.");
            processState = STATE_DESTROY;
            break;
    }
    return processState;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
