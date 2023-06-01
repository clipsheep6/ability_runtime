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

#include "js_uri_perm_mgr.h"

#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ability_runtime_error_util.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
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

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsUriPermMgr::Finalizer is called");
        std::unique_ptr<JsUriPermMgr>(static_cast<JsUriPermMgr*>(data));
    }

    static NativeValue* GrantUriPermission(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsUriPermMgr* me = CheckParamsAndGetThis<JsUriPermMgr>(engine, info);
        return (me != nullptr) ? me->OnGrantUriPermission(*engine, *info) : nullptr;
    }

    static NativeValue* RevokeUriPermission(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsUriPermMgr* me = CheckParamsAndGetThis<JsUriPermMgr>(engine, info);
        return (me != nullptr) ? me->OnRevokeUriPermission(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnGrantUriPermission(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("Grant Uri Permission start");
        if (info.argc != argCountThree && info.argc != argCountFour) {
            HILOG_ERROR("The number of parameter is invalid.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        std::string uriStr;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), uriStr)) {
            HILOG_ERROR("The uriStr is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        int flag = 0;
        if (!OHOS::AppExecFwk::UnwrapInt32FromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), flag)) {
            HILOG_ERROR("The flag is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        std::string targetBundleName;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[argCountTwo]), targetBundleName)) {
            HILOG_ERROR("The targetBundleName is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        AsyncTask::CompleteCallback complete =
        [uriStr, flag, targetBundleName](NativeEngine& engine, AsyncTask& task, int32_t status) {
            Uri uri(uriStr);
            auto errCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermission(uri, flag,
                targetBundleName, 0);
            if (errCode == ERR_OK) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else if (errCode ==  AAFwk::CHECK_PERMISSION_FAILED) {
                task.Reject(engine, CreateNoPermissionError(engine, "ohos.permission.PROXY_AUTHORIZATION_URI"));
            } else if (errCode == AAFwk::ERR_CODE_INVALID_URI_FLAG) {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_FLAG,
                "Invalid URI flag."));
            } else if (errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE) {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_TYPE,
                "Only support file URI."));
            } else if (errCode == AAFwk::ERR_CODE_GRANT_URI_PERMISSION) {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_GRANT_URI_PERMISSION,
                "Sandbox application can not grant URI permission."));
            } else {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Internal Error."));
            }
        };
        NativeValue* lastParam = (info.argc == argCountFour) ? info.argv[argCountThree] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsUriPermMgr::OnGrantUriPermission",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnRevokeUriPermission(NativeEngine& engine, NativeCallbackInfo& info)
    {
        // only support 2 or 3 params (2 parameter and 1 optional callback)
        if (info.argc != argCountThree && info.argc != argCountTwo) {
            HILOG_ERROR("Invalid arguments");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }
        std::string uriStr;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), uriStr)) {
            HILOG_ERROR("The uriStr is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        std::string bundleName;
        if (!OHOS::AppExecFwk::UnwrapStringFromJS2(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), bundleName)) {
            HILOG_ERROR("The bundleName is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        auto selfToken = IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            HILOG_ERROR("This application is not system-app, can not use system-api");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
            return engine.CreateUndefined();
        }
        AsyncTask::CompleteCallback complete =
        [uriStr, bundleName](NativeEngine& engine, AsyncTask& task, int32_t status) {
            Uri uri(uriStr);
            auto errCode = AAFwk::UriPermissionManagerClient::GetInstance().RevokeUriPermissionManually(uri,
                bundleName);
            if (errCode == ERR_OK) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else if (errCode == AAFwk::CHECK_PERMISSION_FAILED) {
                task.Reject(engine, CreateNoPermissionError(engine,
                    "Do not have permission ohos.permission.PROXY_AUTHORIZATION_URI"));
            } else if (errCode == AAFwk::ERR_CODE_INVALID_URI_TYPE) {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_URI_TYPE,
                "Only support file URI."));
            } else {
                task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR,
                "Internal Error."));
            }
        };
        NativeValue* lastParam = (info.argc == argCountThree) ? info.argv[argCountTwo] : nullptr;
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JsUriPermMgr::OnRevokeUriPermission",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};

NativeValue* CreateJsUriPermMgr(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("CreateJsUriPermMgr is called");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("Invalid input parameters");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsUriPermMgr> jsUriPermMgr = std::make_unique<JsUriPermMgr>();
    object->SetNativePointer(jsUriPermMgr.release(), JsUriPermMgr::Finalizer, nullptr);

    const char *moduleName = "JsUriPermMgr";
    BindNativeFunction(*engine, *object, "grantUriPermission", moduleName, JsUriPermMgr::GrantUriPermission);
    BindNativeFunction(*engine, *object, "revokeUriPermission", moduleName, JsUriPermMgr::RevokeUriPermission);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
