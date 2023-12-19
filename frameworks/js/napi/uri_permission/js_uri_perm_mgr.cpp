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

#include "js_uri_perm_mgr.h"

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ability_runtime_error_util.h"
#include "app_utils.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "parameters.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t argCountFour = 4;
constexpr int32_t argCountThree = 3;
constexpr int32_t argCountTwo = 2;
}
class JsUriPermMgr {
public:
    JsUriPermMgr() = default;
    ~JsUriPermMgr() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_INFO("JsUriPermMgr::Finalizer is called");
        std::unique_ptr<JsUriPermMgr>(static_cast<JsUriPermMgr*>(data));
    }

    static napi_value GrantUriPermission(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsUriPermMgr, OnGrantUriPermission);
    }

    static napi_value RevokeUriPermission(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsUriPermMgr, OnRevokeUriPermission);
    }

private:
    napi_value OnGrantUriPermission(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_DEBUG("Grant Uri Permission start");
        if (info.argc != argCountThree && info.argc != argCountFour) {
            HILOG_ERROR("The number of parameter is invalid.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string uriStr;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[0], uriStr)) {
            HILOG_ERROR("The uriStr is invalid.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int flag = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(env, info.argv[1], flag)) {
            HILOG_ERROR("The flag is invalid.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        std::string targetBundleName;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[argCountTwo], targetBundleName)) {
            HILOG_ERROR("The targetBundleName is invalid.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            HILOG_ERROR("This application is not system-app, can not use system-api.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        NapiAsyncTask::CompleteCallback complete =
        [uriStr, flag, targetBundleName](napi_env env, NapiAsyncTask& task, int32_t status) {
            Uri uri(uriStr);
            int errCode;
            if (AAFwk::AppUtils::GetInstance().JudgePCDevice()) {
                errCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionFor2In1(
                    uri, flag, targetBundleName);
            } else {
                errCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermission(
                    uri, flag, targetBundleName, 0);
            }
            if (errCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else if (errCode ==  AAFwk::CHECK_PERMISSION_FAILED) {
                task.Reject(env, CreateNoPermissionError(env, "ohos.permission.PROXY_AUTHORIZATION_URI"));
            } else if (errCode == AAFwk::ERR_CODE_INVALID_URI_FLAG) {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_FLAG,
                "Invalid URI flag."));
            } else if (errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE) {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_TYPE,
                "Only support file URI."));
            } else if (errCode == AAFwk::ERR_CODE_GRANT_URI_PERMISSION) {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_GRANT_URI_PERMISSION,
                "Sandbox application can not grant URI permission."));
            } else {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Internal Error."));
            }
        };
        napi_value lastParam = (info.argc == argCountFour) ? info.argv[argCountThree] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsUriPermMgr::OnGrantUriPermission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnRevokeUriPermission(napi_env env, NapiCallbackInfo& info)
    {
        // only support 2 or 3 params (2 parameter and 1 optional callback)
        if (info.argc != argCountThree && info.argc != argCountTwo) {
            HILOG_ERROR("Invalid arguments");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string uriStr;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[0], uriStr)) {
            HILOG_ERROR("invalid of the uriStr.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(env, info.argv[1], bundleName)) {
            HILOG_ERROR("The bundleName is invalid.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            HILOG_ERROR("can not use system-api, this application is not system-app.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return CreateJsUndefined(env);
        }
        NapiAsyncTask::CompleteCallback complete =
        [uriStr, bundleName](napi_env env, NapiAsyncTask& task, int32_t status) {
            Uri uri(uriStr);
            auto errCode = AAFwk::UriPermissionManagerClient::GetInstance().RevokeUriPermissionManually(uri,
                bundleName);
            if (errCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else if (errCode == AAFwk::CHECK_PERMISSION_FAILED) {
                task.Reject(env, CreateNoPermissionError(env,
                    "Do not have permission ohos.permission.PROXY_AUTHORIZATION_URI"));
            } else if (errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE) {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_TYPE,
                "Only support file URI."));
            } else {
                task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Internal Error."));
            }
        };
        napi_value lastParam = (info.argc == argCountThree) ? info.argv[argCountTwo] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsUriPermMgr::OnRevokeUriPermission",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};

napi_value CreateJsUriPermMgr(napi_env env, napi_value exportObj)
{
    HILOG_INFO("CreateJsUriPermMgr is called");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_INFO("Invalid input parameters");
        return nullptr;
    }

    std::unique_ptr<JsUriPermMgr> jsUriPermMgr = std::make_unique<JsUriPermMgr>();
    napi_wrap(env, exportObj, jsUriPermMgr.release(), JsUriPermMgr::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsUriPermMgr";
    BindNativeFunction(env, exportObj, "grantUriPermission", moduleName, JsUriPermMgr::GrantUriPermission);
    BindNativeFunction(env, exportObj, "revokeUriPermission", moduleName, JsUriPermMgr::RevokeUriPermission);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
