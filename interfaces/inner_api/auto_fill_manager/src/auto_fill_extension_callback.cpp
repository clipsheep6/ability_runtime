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
#include "auto_fill_extension_callback.h"

#include "auto_fill_error.h"
#include "auto_fill_manager.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
constexpr static char WANT_PARAMS_CUSTOM_DATA_KEY[] = "ohos.ability.params.customData";
constexpr static char WANT_PARAMS_AUTO_FILL_CMD_KEY[] = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_AUTO_FILL_EVENT_KEY[] = "ability.want.params.AutoFillEvent";
} // namespace
void AutoFillExtensionCallback::OnResult(int32_t errCode, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called, result code is %{public}d.", errCode);
    AutoFillManager::GetInstance().RemoveEvent(eventId_);
    CloseModalUIExtension();

    if (errCode == AutoFill::AUTO_FILL_SUCCESS) {
        SendAutoFillSucess(want);
    } else {
        auto resultCode = (errCode == AutoFill::AUTO_FILL_CANCEL) ?
            AutoFill::AUTO_FILL_CANCEL : AutoFill::AUTO_FILL_FAILED;
        SendAutoFillFailed(resultCode);
    }
}

void AutoFillExtensionCallback::OnRelease(int32_t errCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called, result code is %{public}d.", errCode);
    AutoFillManager::GetInstance().RemoveEvent(eventId_);
    CloseModalUIExtension();

    if (errCode != 0) {
        SendAutoFillFailed(AutoFill::AUTO_FILL_RELEASE_FAILED);
    }
}

void AutoFillExtensionCallback::OnError(int32_t errCode, const std::string &name, const std::string &message)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called, errcode is %{public}d, name is %{public}s, message is %{public}s",
        errCode, name.c_str(), message.c_str());
    AutoFillManager::GetInstance().RemoveEvent(eventId_);
    CloseModalUIExtension();

    if (errCode != 0) {
        SendAutoFillFailed(AutoFill::AUTO_FILL_ON_ERROR);
    }
}

void AutoFillExtensionCallback::HandleReloadInModal(const AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("Called.");
    isReloadInModal_ = true;
    auto customDataString(wantParams.GetStringParam(WANT_PARAMS_CUSTOM_DATA_KEY));
    AutoFill::ReloadInModalRequest request = {
        .uiContent = uiContent_,
        .nodeId = sessionId_,
        .customData = customDataString,
        .autoFillWindowType = autoFillWindowType_,
        .extensionCallback = shared_from_this(),
    };
    CloseModalUIExtension();
    int32_t resultCode = AutoFillManager::GetInstance().ReloadInModal(request);
    if (resultCode != AutoFill::AUTO_FILL_SUCCESS) {
        SendAutoFillFailed(resultCode);
    }
}

void AutoFillExtensionCallback::OnReceive(const AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    int32_t cmdValue = wantParams.GetIntParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, 0);
    if (cmdValue == static_cast<int32_t>(AutoFill::AutoFillCommand::RELOAD_IN_MODAL)) {
        HandleReloadInModal(wantParams);
        return;
    }
    if (wantParams.GetIntParam(WANT_PARAMS_AUTO_FILL_EVENT_KEY, 0) == AutoFill::AUTO_FILL_CANCEL_TIME_OUT) {
        AutoFillManager::GetInstance().RemoveEvent(eventId_);
    }
}

void AutoFillExtensionCallback::onRemoteReady(const std::shared_ptr<Ace::ModalUIExtensionProxy> &modalUIExtensionProxy)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (modalUIExtensionProxy == nullptr || uiContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Proxy or uiContent_ is nullptr.");
        return;
    }
    AutoFillManager::GetInstance().SetAutoFillExtensionProxy(uiContent_, modalUIExtensionProxy);
}

void AutoFillExtensionCallback::onDestroy()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (isReloadInModal_) {
        isReloadInModal_ = false;
        return;
    }
    AutoFillManager::GetInstance().RemoveEvent(eventId_);
    if (uiContent_ != nullptr && autoFillWindowType_ == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        AutoFillManager::GetInstance().RemoveAutoFillExtensionProxy(uiContent_);
    }
    uiContent_ = nullptr;
    SendAutoFillFailed(AutoFill::AUTO_FILL_CANCEL);
}

void AutoFillExtensionCallback::SetFillRequestCallback(const std::shared_ptr<IFillRequestCallback> &callback)
{
    fillCallback_ = callback;
}

void AutoFillExtensionCallback::SetSaveRequestCallback(const std::shared_ptr<ISaveRequestCallback> &callback)
{
    saveCallback_ = callback;
}

void AutoFillExtensionCallback::SetSessionId(int32_t sessionId)
{
    sessionId_= sessionId;
}

void AutoFillExtensionCallback::SetUIContent(Ace::UIContent *uiContent)
{
    uiContent_ = uiContent;
}

void AutoFillExtensionCallback::SetEventId(uint32_t eventId)
{
    eventId_ = eventId;
}

void AutoFillExtensionCallback::SetWindowType(const AutoFill::AutoFillWindowType &autoFillWindowType)
{
    autoFillWindowType_ = autoFillWindowType;
}

void AutoFillExtensionCallback::SetViewData(const AbilityBase::ViewData &viewData)
{
    viewData_ = viewData;
}

AbilityBase::ViewData AutoFillExtensionCallback::GetViewData()
{
    return viewData_;
}

void AutoFillExtensionCallback::HandleTimeOut()
{
    CloseModalUIExtension();
    SendAutoFillFailed(AutoFill::AUTO_FILL_REQUEST_TIME_OUT);
}

void AutoFillExtensionCallback::SendAutoFillSucess(const AAFwk::Want &want)
{
    if (fillCallback_ != nullptr) {
        std::string dataStr = want.GetStringParam(WANT_PARAMS_VIEW_DATA_KEY);
        AbilityBase::ViewData viewData;
        viewData.FromJsonString(dataStr.c_str());
        fillCallback_->OnFillRequestSuccess(viewData);
        fillCallback_ = nullptr;
    }

    if (saveCallback_ != nullptr) {
        saveCallback_->OnSaveRequestSuccess();
        saveCallback_ = nullptr;
    }
}

void AutoFillExtensionCallback::SendAutoFillFailed(int32_t errCode)
{
    if (fillCallback_ != nullptr) {
        fillCallback_->OnFillRequestFailed(errCode);
        fillCallback_ = nullptr;
    }

    if (saveCallback_ != nullptr) {
        saveCallback_->OnSaveRequestFailed();
        saveCallback_ = nullptr;
    }
}

void AutoFillExtensionCallback::CloseModalUIExtension()
{
    if (uiContent_ == nullptr) {
        TAG_LOGD(AAFwkTag::AUTOFILLMGR, "uiContent_ is nullptr.");
        return;
    }

    if (autoFillWindowType_ == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        AutoFillManager::GetInstance().RemoveAutoFillExtensionProxy(uiContent_);
        uiContent_->DestroyCustomPopupUIExtension(sessionId_);
    } else if (autoFillWindowType_ == AutoFill::AutoFillWindowType::MODAL_WINDOW) {
        uiContent_->CloseModalUIExtension(sessionId_);
    }
    if (!isReloadInModal_) {
        uiContent_ = nullptr;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS