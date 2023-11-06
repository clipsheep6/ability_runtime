/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "js_ability_auto_startup_manager.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "auto_startup_info.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "permission_constants.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr const char *ON_OFF_TYPE_SYSTEM = "systemAutoStartup";
} // namespace

void JsAbilityAutoStartupManager::Finalizer(napi_env env, void *data, void *hint)
{
    HILOG_DEBUG("Called.");
    std::unique_ptr<JsAbilityAutoStartupManager>(static_cast<JsAbilityAutoStartupManager *>(data));
}

napi_value JsAbilityAutoStartupManager::RegisterAutoStartupCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnRegisterAutoStartupCallback);
}

napi_value JsAbilityAutoStartupManager::UnregisterAutoStartupCallback(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnUnregisterAutoStartupCallback);
}

napi_value JsAbilityAutoStartupManager::SetApplicationAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnSetApplicationAutoStartup);
}

napi_value JsAbilityAutoStartupManager::CancelApplicationAutoStartup(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnCancelApplicationAutoStartup);
}

napi_value JsAbilityAutoStartupManager::QueryAllAutoStartupApplications(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityAutoStartupManager, OnQueryAllAutoStartupApplications);
}

bool JsAbilityAutoStartupManager::CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        HILOG_ERROR("Current app is not system app, not allow.");
        return false;
    }
    return true;
}

napi_value JsAbilityAutoStartupManager::OnRegisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_SYSTEM) {
        HILOG_ERROR("The param is invalid.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        HILOG_ERROR("Current app is not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (jsAutoStartupCallback_ == nullptr) {
        jsAutoStartupCallback_ = new (std::nothrow) JsAbilityAutoStartupCallBack(env);
        if (jsAutoStartupCallback_ == nullptr) {
            HILOG_ERROR("JsAutoStartupCallback_ is nullptr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        auto ret =
            AbilityManagerClient::GetInstance()->RegisterAutoStartupSystemCallback(jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            jsAutoStartupCallback_ = nullptr;
            HILOG_ERROR("Register auto start up listener wrong[%{public}d].", ret);
            ThrowError(env, GetJsErrorCodeByNativeError(ret));
            return CreateJsUndefined(env);
        }
    }

    jsAutoStartupCallback_->Register(info.argv[INDEX_ONE]);
    return CreateJsUndefined(env);
}

napi_value JsAbilityAutoStartupManager::OnUnregisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("OnUnregisterAutoStartupCallback Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The argument is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string type;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE_SYSTEM) {
        HILOG_ERROR("Failed to parse type.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    if (jsAutoStartupCallback_ == nullptr) {
        HILOG_ERROR("JsAutoStartupCallback_ is nullptr.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    auto callback = info.argc > ARGC_ONE ? info.argv[INDEX_ONE] : CreateJsUndefined(env);
    jsAutoStartupCallback_->UnRegister(callback);
    if (jsAutoStartupCallback_->IsCallbacksEmpty()) {
        auto ret = AbilityManagerClient::GetInstance()->UnregisterAutoStartupSystemCallback(
            jsAutoStartupCallback_->AsObject());
        if (ret != ERR_OK) {
            ThrowError(env, GetJsErrorCodeByNativeError(ret));
        }
        jsAutoStartupCallback_ = nullptr;
    }

    return CreateJsUndefined(env);
}

napi_value JsAbilityAutoStartupManager::OnSetApplicationAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("OnSetApplicationAutoStartup Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The argument is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        HILOG_ERROR("Current app is not system app");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
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
        *ret = AbilityManagerClient::GetInstance()->SetApplicationAutoStartup(autoStartupInfo);
    };
    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (ret == nullptr) {
            HILOG_ERROR("The argument is invalid.");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            HILOG_ERROR("Wrong error:%{public}d.", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value lastParam = (info.argc >= ARGC_TWO) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityAutoStartupManager::OnSetApplicationAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("OnCancelApplicationAutoStartup Called.");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("The param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    if (!CheckCallerIsSystemApp()) {
        HILOG_ERROR("Current app is not system app.");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
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
        *ret = AbilityManagerClient::GetInstance()->CancelApplicationAutoStartup(autoStartupInfo);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (ret == nullptr) {
            HILOG_ERROR("The parameter is invalid.");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
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
    NapiAsyncTask::Schedule("JsAbilityAutoStartupManager::OnCancelApplicationAutoStartup", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAbilityAutoStartupManager::OnQueryAllAutoStartupApplications(napi_env env, const NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    if (!CheckCallerIsSystemApp()) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    auto infoList = std::make_shared<std::vector<AutoStartupInfo>>();
    NapiAsyncTask::ExecuteCallback execute = [infos = infoList, ret = retVal] () {
        if (ret == nullptr || infos == nullptr) {
            HILOG_ERROR("The param is invalid.");
            return;
        }
        *ret = AbilityManagerClient::GetInstance()->QueryAllAutoStartupApplications(*infos);
    };

    NapiAsyncTask::CompleteCallback complete = [infos = infoList, ret = retVal](
                                                   napi_env env, NapiAsyncTask &task, int32_t status) {
        if (ret == nullptr || infos == nullptr) {
            HILOG_ERROR("The param is invalid.");
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(INNER_ERR)));
            return;
        }
        if (*ret != ERR_OK) {
            HILOG_ERROR("Failed error:%{public}d.", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsAutoStartupInfoArray(env, *infos));
    };

    napi_value lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityAutoStartupManager::OnQueryAllAutoStartupApplications", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAbilityAutoStartupManagerInit(napi_env env, napi_value exportObj)
{
    HILOG_DEBUG("Called.");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Env or exportObj nullptr.");
        return nullptr;
    }

    auto jsAbilityAutoStartupManager = std::make_unique<JsAbilityAutoStartupManager>();
    napi_wrap(env, exportObj, jsAbilityAutoStartupManager.release(),
        JsAbilityAutoStartupManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAbilityAutoStartupManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsAbilityAutoStartupManager::RegisterAutoStartupCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsAbilityAutoStartupManager::UnregisterAutoStartupCallback);
    BindNativeFunction(env, exportObj, "setApplicationAutoStartup", moduleName,
        JsAbilityAutoStartupManager::SetApplicationAutoStartup);
    BindNativeFunction(env, exportObj, "cancelApplicationAutoStartup", moduleName,
        JsAbilityAutoStartupManager::CancelApplicationAutoStartup);
    BindNativeFunction(env, exportObj, "queryAllAutoStartupApplications", moduleName,
        JsAbilityAutoStartupManager::QueryAllAutoStartupApplications);
    HILOG_DEBUG("End.");
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS