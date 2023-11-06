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

#include "js_dialog_session.h"

#include "ability_manager_client.h"
#include "dialog_session_record.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using AAFwk::AbilityManagerClient;
constexpr size_t ARGC_TWO = 2;

napi_value JsDialogSession::GetDialogSessionInfo(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsDialogSession, OnGetDialogSessionInfo);
}

napi_value JsDialogSession::SendDialogResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsDialogSession, OnSendDialogResult);
}

napi_value JsDialogSession::OnGetDialogSessionInfo(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("GetDialogSessionInfo start");
    if (info.argc == 0) {
        HILOG_ERROR("not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string dialogSessionId = "";
    if (!ConvertFromJsValue(env, info.argv[0], dialogSessionId)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    sptr<AAFwk::DialogSessionInfo> dialogSessionInfo;
    HILOG_DEBUG("GetDialogSessionInfo begin");
    auto errcode = AbilityManagerClient::GetInstance()->GetDialogSessionInfo(dialogSessionId, dialogSessionInfo);
    if (errcode) {
        HILOG_ERROR("GetDialogSessionInfo error");
        return CreateJsUndefined(env);
    }
    return OHOS::AppExecFwk::WrapDialogSessionInfo(env, *dialogSessionInfo);
}

napi_value JsDialogSession::OnSendDialogResult(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("OnSendDialogResult start");
    if (info.argc == 0) {
        HILOG_ERROR("not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string dialogSessionId = "";
    if (!ConvertFromJsValue(env, info.argv[0], dialogSessionId)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[1], want)) {
        HILOG_ERROR("Failed to unwrap want");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    int32_t dialogResultCode = -1;
    if (!ConvertFromJsValue(env, info.argv[ARGC_TWO], dialogResultCode)) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }
    bool isAllow = dialogResultCode == 0 ? true : false;
    NapiAsyncTask::CompleteCallback complete =
        [dialogSessionId, want, isAllow](napi_env env, NapiAsyncTask &task, int32_t status) {
        auto errorcode = AbilityManagerClient::GetInstance()->SendDialogResult(want, dialogSessionId, isAllow);
        if (errorcode) {
            task.Reject(env, CreateJsError(env, errorcode, "Send dialog result failed"));
        } else {
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsDialogSession::OnSendDialogResult",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}
} // namespace AbilityRuntime
} // nampspace OHOS
