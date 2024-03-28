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

#include "js_fill_request_callback.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "native_engine.h"
#include "native_value.h"
#include "ui_content.h"
#include "view_data.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr const char *WANT_PARAMS_VIEW_DATA = "ohos.ability.params.viewData";
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD = "ohos.ability.params.autoFillCmd";
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD_AUTOFILL = "autofill";
} // namespace

JsFillRequestCallback::JsFillRequestCallback(
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow)
{}

void JsFillRequestCallback::Finalizer(napi_env env, void* data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "JsFillRequestCallback Finalizer is called");
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
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    if (info.argc < ARGC_ONE || !IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to parse viewData JsonString!");
        SendResultCodeAndViewData(
            JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return CreateJsUndefined(env);
    }

    FillResponse response;
    JsAutoFillExtensionUtil::UnwrapFillResponse(env, info.argv[INDEX_ZERO], response);
    std::string jsonString = response.viewData.ToJsonString();
    if (jsonString.empty()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "JsonString is empty");
        SendResultCodeAndViewData(
            JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED_INVALID_PARAM, "");
        return CreateJsUndefined(env);
    }

    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS, jsonString);
    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestFailed(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED, "");
    return CreateJsUndefined(env);
}

napi_value JsFillRequestCallback::OnFillRequestCanceled(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_CANCEL, "");
    return CreateJsUndefined(env);
}

void JsFillRequestCallback::SendResultCodeAndViewData(
    const JsAutoFillExtensionUtil::AutoFillResultCode &resultCode, const std::string &jsString)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "UiWindow is nullptr.");
        return;
    }

    AAFwk::Want want;
    if (resultCode == JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS) {
        want.SetParam(WANT_PARAMS_VIEW_DATA, jsString);
        want.SetParam(WANT_PARAMS_AUTO_FILL_CMD, WANT_PARAMS_AUTO_FILL_CMD_AUTOFILL);
    }

    auto ret = uiWindow_->TransferAbilityResult(resultCode, want);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Transfer ability result failed.");
        return;
    }

    auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo_);
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Terminate ui extension ability failed, errorCode: %{public}d", errorCode);
    }
}

napi_value JsFillRequestCallback::CreateJsFillRequestCallback(napi_env env,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object is null");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsFillRequestCallback> jsSession =
        std::make_unique<JsFillRequestCallback>(sessionInfo, uiWindow);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsFillRequestCallback";
    BindNativeFunction(env, object, "onSuccess", moduleName, FillRequestSuccess);
    BindNativeFunction(env, object, "onFailure", moduleName, FillRequestFailed);
    BindNativeFunction(env, object, "onCancel", moduleName, FillRequestCanceled);
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS