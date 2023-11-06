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

#include "js_fill_request_callback.h"

#include "accesstoken_kit.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "native_engine.h"
#include "native_value.h"
#include "ui_content.h"
#include "want.h"
#include "window.h"

#include "ability_manager_client.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "js_ui_extension_context.h"
#include "string_wrapper.h"
#include "napi_common_start_options.h"
#include "napi_common_want.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
const std::string WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
} // namespace

JsFillRequestCallback::JsFillRequestCallback(
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow) {}

void JsFillRequestCallback::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_DEBUG("JsFillRequestCallback Finalizer is called");
    std::unique_ptr<JsFillRequestCallback>(static_cast<JsFillRequestCallback*>(data));
}

napi_value JsFillRequestCallback::FillRequestSuccess(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestSuccess);
}

napi_value JsFillRequestCallback::FillRequestFailed(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestFailed);
}

napi_value JsFillRequestCallback::FillRequestCanceled(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsFillRequestCallback, OnFillRequestCanceled);
}

napi_value JsFillRequestCallback::OnFillRequestSuccess(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    std::string viewDataJsonString;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(env, info.argv[INDEX_ZERO], viewDataJsonString)) {
        HILOG_ERROR("Failed to parse viewData JsonString!");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    SendResultCodeAndViewData(env, info, ResultCode::FILL_SUCESS, viewDataJsonString);

    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestFailed(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    SendResultCodeAndViewData(env, info, ResultCode::FILL_FAILED, nullptr);
    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestCanceled(napi_env env, NapiCallbackInfo &info)
{
    HILOG_DEBUG("Called.");
    SendResultCodeAndViewData(env, info, ResultCode::FILL_CANCEL, nullptr);
    return CreateJsUndefined(env);
}

void JsFillRequestCallback::SendResultCodeAndViewData(napi_env env, NapiCallbackInfo &info, ResultCode resultCode, std::string jsString)
{
    AAFwk::Want want;
    if (resultCode == ERR_OK ) {
        want.SetParam(WANT_PARAMS_VIEW_DATA, jsString);
    }

    NapiAsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, sessionInfo = sessionInfo_, want, resultCode](napi_env env,
            NapiAsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is nullptr");
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return CreateJsUndefined(env);
            }
            auto ret = uiWindow->TransferAbilityResult(resultCode, want);
            if (ret != Rosen::WMError::WM_OK) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
                return CreateJsUndefined(env);
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, errorCode));
            }
            return CreateJsUndefined(env);
        };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsFillRequestCallback::OnFillRequestSuccess",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));    
}

napi_value JsFillRequestCallback::CreateJsFillRequestCallback(napi_env env,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
{
    HILOG_DEBUG("Caller.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("object is null");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsFillRequestCallback> jsSession =
        std::make_unique<JsFillRequestCallback>(sessionInfo, uiWindow);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsFillRequestCallback";
    BindNativeFunction(env, object, "onFillRequestSuccess", moduleName, FillRequestSuccess);
    BindNativeFunction(env, object, "onFillRequestFailed", moduleName, FillRequestFailed);
    BindNativeFunction(env, object, "onFillRequestCanceled", moduleName, FillRequestCanceled);
    return object;
}
}  // namespace AbilityRuntime
}  // namespace OHOS