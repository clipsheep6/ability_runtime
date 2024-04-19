/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_dialog_session.h"

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using AAFwk::AbilityManagerClient;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;

class JsDialogSession {
public:
    JsDialogSession() = default;
    ~JsDialogSession() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::DIALOG, "JsDialogSession::Finalizer is called");
        std::unique_ptr<JsDialogSession>(static_cast<JsDialogSession*>(data));
    }

    static napi_value GetDialogSessionInfo(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsDialogSession, OnGetDialogSessionInfo);
    }

    static napi_value SendDialogResult(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsDialogSession, OnSendDialogResult);
    }

private:
    napi_value OnGetDialogSessionInfo(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DIALOG, "enter, argc = %{public}d", static_cast<int32_t>(info.argc));
        if (info.argc < 1) {
            TAG_LOGE(AAFwkTag::DIALOG, "not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string dialogSessionId = "";
        if (!ConvertFromJsValue(env, info.argv[0], dialogSessionId)) {
            TAG_LOGE(AAFwkTag::DIALOG, "Failed to unwrap dialogSessionId");
            ThrowInvalidParamError(env, "Parameter error: dialogSessionId must be a valid string.");
            return CreateJsUndefined(env);
        }

        sptr<AAFwk::DialogSessionInfo> dialogSessionInfo;
        TAG_LOGD(AAFwkTag::DIALOG, "GetDialogSessionInfo begin");
        auto errcode = AbilityManagerClient::GetInstance()->GetDialogSessionInfo(dialogSessionId, dialogSessionInfo);
        if (errcode || dialogSessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "GetDialogSessionInfo error");
            return CreateJsUndefined(env);
        }
        return OHOS::AppExecFwk::WrapDialogSessionInfo(env, *dialogSessionInfo);
    }

    napi_value OnSendDialogResult(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::DIALOG, "enter, argc = %{public}d", static_cast<int32_t>(info.argc));
        if (info.argc < ARGC_THREE) {
            TAG_LOGE(AAFwkTag::DIALOG, "not enough params");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string dialogSessionId = "";
        if (!ConvertFromJsValue(env, info.argv[0], dialogSessionId)) {
            TAG_LOGE(AAFwkTag::DIALOG, "Failed to unwrap dialogSessionId");
            ThrowInvalidParamError(env, "Parameter error: dialogSessionId must be a valid string.");
            return CreateJsUndefined(env);
        }
        AAFwk::Want want;
        if (!AppExecFwk::UnwrapWant(env, info.argv[1], want)) {
            TAG_LOGE(AAFwkTag::DIALOG, "Failed to unwrap want");
            ThrowInvalidParamError(env, "Parameter error: want must be a Want.");
            return CreateJsUndefined(env);
        }
        bool isAllow = false;
        if (!ConvertFromJsValue(env, info.argv[ARGC_TWO], isAllow)) {
            TAG_LOGE(AAFwkTag::DIALOG, "Failed to unwrap isAllow");
            ThrowInvalidParamError(env, "Parameter error: isAllow must be a Boolean.");
            return CreateJsUndefined(env);
        }
        NapiAsyncTask::CompleteCallback complete =
            [dialogSessionId, want, isAllow](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto errorcode = AbilityManagerClient::GetInstance()->SendDialogResult(want, dialogSessionId, isAllow);
            if (errorcode) {
                task.Reject(env, CreateJsError(env, errorcode, "Send dialog result failed"));
            } else {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            }
        };
        napi_value lastParam = (info.argc > ARGC_THREE) ? info.argv[ARGC_THREE] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsDialogSession::OnSendDialogResult",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};

napi_value JsDialogSessionInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::DIALOG, "JsDialogSessionInit is called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGI(AAFwkTag::DIALOG, "Invalid input parameters");
        return nullptr;
    }

    std::unique_ptr<JsDialogSession> jsDialogSession = std::make_unique<JsDialogSession>();
    napi_wrap(env, exportObj, jsDialogSession.release(), JsDialogSession::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsDialogSession";
    BindNativeFunction(env, exportObj, "getDialogSessionInfo", moduleName, JsDialogSession::GetDialogSessionInfo);
    BindNativeFunction(env, exportObj, "sendDialogResult", moduleName, JsDialogSession::SendDialogResult);
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // nampspace OHOS
