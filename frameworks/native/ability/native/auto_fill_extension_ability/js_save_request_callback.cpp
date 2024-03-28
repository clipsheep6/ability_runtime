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

#include "js_save_request_callback.h"

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_auto_fill_extension_util.h"
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
JsSaveRequestCallback::JsSaveRequestCallback(
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow)
{}

void JsSaveRequestCallback::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Finalizer is called.");
    std::unique_ptr<JsSaveRequestCallback>(static_cast<JsSaveRequestCallback*>(data));
}

napi_value JsSaveRequestCallback::SaveRequestSuccess(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsSaveRequestCallback, OnSaveRequestSuccess);
}

napi_value JsSaveRequestCallback::SaveRequestFailed(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsSaveRequestCallback, OnSaveRequestFailed);
}

napi_value JsSaveRequestCallback::OnSaveRequestSuccess(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_SUCESS);
    return CreateJsUndefined(env);
}

napi_value JsSaveRequestCallback::OnSaveRequestFailed(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    SendResultCodeAndViewData(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_FAILED);
    return CreateJsUndefined(env);
}

void JsSaveRequestCallback::SendResultCodeAndViewData(const JsAutoFillExtensionUtil::AutoFillResultCode &resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    if (uiWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "UI window is nullptr.");
        return;
    }

    AAFwk::Want want;
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

napi_value JsSaveRequestCallback::CreateJsSaveRequestCallback(napi_env env,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object is null");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsSaveRequestCallback> jsSession =
        std::make_unique<JsSaveRequestCallback>(sessionInfo, uiWindow);
    napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsSaveRequestCallback";
    BindNativeFunction(env, object, "onSuccess", moduleName, SaveRequestSuccess);
    BindNativeFunction(env, object, "onFailure", moduleName, SaveRequestFailed);
    return object;
}
}  // namespace AbilityRuntime
}  // namespace OHOS