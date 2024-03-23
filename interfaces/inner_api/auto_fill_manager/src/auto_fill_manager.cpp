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

#include "auto_fill_manager.h"

#include "auto_fill_error.h"
#include "auto_fill_manager_util.h"
#include "extension_ability_info.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "int_wrapper.h"
#include "parameters.h"
#include "string_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string WANT_PARAMS_EXTENSION_TYPE = "autoFill/password";
const std::string WANT_PARAMS_SMART_EXTENSION_TYPE = "autoFill/smart";
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
constexpr static char WANT_PARAMS_AUTO_FILL_CMD_KEY[] = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY[] = "ohos.ability.params.popupWindow";
constexpr static char WANT_PARAMS_EXTENSION_TYPE_KEY[] = "ability.want.params.uiExtensionType";
constexpr static char WANT_PARAMS_AUTO_FILL_TYPE_KEY[] = "ability.want.params.AutoFillType";
constexpr static char AUTO_FILL_MANAGER_THREAD[] = "AutoFillManager";
constexpr static uint32_t AUTO_FILL_REQUEST_TIME_OUT_VALUE = 1000;
constexpr static uint32_t AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID = 0;
} // namespace
AutoFillManager &AutoFillManager::GetInstance()
{
    static AutoFillManager instance;
    return instance;
}

AutoFillManager::~AutoFillManager()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (eventHandler_ != nullptr) {
        eventHandler_.reset();
    }
}

int32_t AutoFillManager::RequestAutoFill(
    Ace::UIContent *uiContent,
    const AutoFill::AutoFillRequest &request,
    const std::shared_ptr<IFillRequestCallback> &fillCallback, bool &isPopup)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (uiContent == nullptr || fillCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent or fillCallback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }

    if (request.autoFillType == AbilityBase::AutoFillType::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Auto fill type is invalid.");
        return AutoFill::AUTO_FILL_TYPE_INVALID;
    }
    return HandleRequestExecuteInner(uiContent, request, fillCallback, nullptr, isPopup);
}

int32_t AutoFillManager::RequestAutoSave(
    Ace::UIContent *uiContent,
    const AutoFill::AutoFillRequest &request,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (uiContent == nullptr || saveCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent or save callback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    bool isPopup = false;
    return HandleRequestExecuteInner(uiContent, request, nullptr, saveCallback, isPopup);
}

int32_t AutoFillManager::HandleRequestExecuteInner(
    Ace::UIContent *uiContent,
    const AutoFill::AutoFillRequest &request,
    const std::shared_ptr<IFillRequestCallback> &fillCallback,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback, bool &isPopup)
{
    if (uiContent == nullptr || (fillCallback == nullptr && saveCallback == nullptr)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent or fillCallback&saveCallback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    {
        std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
        SetTimeOutEvent(++eventId_);
    }

    auto extensionCallback = std::make_shared<AutoFillExtensionCallback>();
    if (fillCallback != nullptr) {
        extensionCallback->SetFillRequestCallback(fillCallback);
    } else {
        extensionCallback->SetSaveRequestCallback(saveCallback);
    }
    Ace::ModalUIExtensionCallbacks callback;
    callback.onResult = std::bind(
        &AutoFillExtensionCallback::OnResult, extensionCallback, std::placeholders::_1, std::placeholders::_2);
    callback.onRelease = std::bind(
        &AutoFillExtensionCallback::OnRelease, extensionCallback, std::placeholders::_1);
    callback.onError = std::bind(&AutoFillExtensionCallback::OnError,
        extensionCallback, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    callback.onReceive = std::bind(&AutoFillExtensionCallback::OnReceive, extensionCallback, std::placeholders::_1);
    callback.onRemoteReady = std::bind(&AutoFillExtensionCallback::onRemoteReady,
        extensionCallback, std::placeholders::_1);
    callback.onDestroy = std::bind(&AutoFillExtensionCallback::onDestroy, extensionCallback);

    bool isSmartAutoFill = false;
    auto autoFillWindowType = ConvertAutoFillWindowType(request, isSmartAutoFill);
    isPopup = autoFillWindowType == AutoFill::AutoFillWindowType::POPUP_WINDOW ? true : false;
    auto sessionId = CreateAutoFillExtension(uiContent, request, callback, autoFillWindowType, isSmartAutoFill);
    if (sessionId == AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Create ui extension is failed.");
        RemoveEvent(eventId_);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    extensionCallback->SetUIContent(uiContent);
    extensionCallback->SetSessionId(sessionId);
    extensionCallback->SetEventId(eventId_);
    extensionCallback->SetWindowType(autoFillWindowType);
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    extensionCallbacks_.emplace(eventId_, extensionCallback);
    return AutoFill::AUTO_FILL_SUCCESS;
}

void AutoFillManager::UpdateCustomPopupUIExtension(Ace::UIContent *uiContent, const AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent is nullptr.");
        return;
    }

    std::shared_ptr<Ace::ModalUIExtensionProxy> modalUIExtensionProxy;
    {
        std::lock_guard<std::mutex> lock(modalProxyMapMutex_);
        auto it = modalUIExtensionProxyMap_.find(uiContent);
        if (it == modalUIExtensionProxyMap_.end()) {
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Content is not in map.");
            return;
        }
        modalUIExtensionProxy = it->second;
    }

    if (modalUIExtensionProxy != nullptr) {
        AAFwk::WantParams wantParams;
        wantParams.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY,
            AAFwk::Integer::Box(static_cast<int32_t>(AutoFill::AutoFillCommand::UPDATE)));
        wantParams.SetParam(WANT_PARAMS_VIEW_DATA_KEY, AAFwk::String::Box(viewData.ToJsonString()));
        modalUIExtensionProxy->SendData(wantParams);
    }
}

void AutoFillManager::SetAutoFillExtensionProxy(Ace::UIContent *uiContent,
    const std::shared_ptr<Ace::ModalUIExtensionProxy> &modalUIExtensionProxy)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (uiContent == nullptr || modalUIExtensionProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent or proxy is nullptr.");
        return;
    }

    std::lock_guard<std::mutex> lock(modalProxyMapMutex_);
    auto it = modalUIExtensionProxyMap_.find(uiContent);
    if (it != modalUIExtensionProxyMap_.end()) {
        modalUIExtensionProxyMap_.erase(it);
    }
    modalUIExtensionProxyMap_.emplace(uiContent, modalUIExtensionProxy);
}

void AutoFillManager::RemoveAutoFillExtensionProxy(Ace::UIContent *uiContent)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Content is nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(modalProxyMapMutex_);
    auto it = modalUIExtensionProxyMap_.find(uiContent);
    if (it != modalUIExtensionProxyMap_.end()) {
        modalUIExtensionProxyMap_.erase(it);
    }
}

int32_t AutoFillManager::CreateAutoFillExtension(Ace::UIContent *uiContent,
    const AutoFill::AutoFillRequest &request,
    const Ace::ModalUIExtensionCallbacks &callback,
    const AutoFill::AutoFillWindowType autoFillWindowType,
    bool isSmartAutoFill)
{
    int32_t sessionId = AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID;
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Content is nullptr.");
        return sessionId;
    }

    AAFwk::Want want;
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, static_cast<int32_t>(request.autoFillCommand));
    want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(request.autoFillType));
    want.SetParam(WANT_PARAMS_VIEW_DATA_KEY, request.viewData.ToJsonString());
    isSmartAutoFill ? want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_SMART_EXTENSION_TYPE) :
        want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_EXTENSION_TYPE);

    if (autoFillWindowType == AutoFill::AutoFillWindowType::POPUP_WINDOW) {
        want.SetParam(WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY, true);
        Ace::CustomPopupUIExtensionConfig popupConfig;
        AutoFillManagerUtil::ConvertToPopupUIExtensionConfig(request.config, popupConfig);
        sessionId = uiContent->CreateCustomPopupUIExtension(want, callback, popupConfig);
    } else if (autoFillWindowType == AutoFill::AutoFillWindowType::MODAL_WINDOW) {
        want.SetParam(WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY, false);
        Ace::ModalUIExtensionConfig config;
        config.isAsyncModalBinding = true;
        sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    }
    return sessionId;
}

AutoFill::AutoFillWindowType AutoFillManager::ConvertAutoFillWindowType(const AutoFill::AutoFillRequest &request,
    bool &isSmartAutoFill)
{
    AutoFill::AutoFillWindowType autoFillWindowType = AutoFill::AutoFillWindowType::MODAL_WINDOW;
    AbilityBase::AutoFillType autoFillType = request.autoFillType;
    if (autoFillType >= AbilityBase::AutoFillType::FULL_STREET_ADDRESS &&
        autoFillType <= AbilityBase::AutoFillType::FORMAT_ADDRESS) {
        autoFillWindowType = AutoFill::AutoFillWindowType::POPUP_WINDOW;
        isSmartAutoFill = true;
    } else if (autoFillType == AbilityBase::AutoFillType::PASSWORD ||
        autoFillType == AbilityBase::AutoFillType::USER_NAME ||
        autoFillType == AbilityBase::AutoFillType::NEW_PASSWORD) {
        autoFillWindowType = AutoFill::AutoFillWindowType::MODAL_WINDOW;
        isSmartAutoFill = false;
    }

    autoFillWindowType = request.autoFillCommand == AutoFill::AutoFillCommand::SAVE ?
        AutoFill::AutoFillWindowType::MODAL_WINDOW : autoFillWindowType;
    return autoFillWindowType;
}

void AutoFillManager::SetTimeOutEvent(uint32_t eventId)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    auto runner = AppExecFwk::EventRunner::Create(AUTO_FILL_MANAGER_THREAD);
    if (eventHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Eventhandler is nullptr.");
        eventHandler_ = std::make_shared<AutoFillEventHandler>(runner);
    }
    eventHandler_->SendEvent(eventId, AUTO_FILL_REQUEST_TIME_OUT_VALUE);
}

void AutoFillManager::RemoveEvent(uint32_t eventId)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (eventHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Eventhandler is nullptr.");
        return;
    }
    eventHandler_->RemoveEvent(eventId);

    std::lock_guard<std::mutex> lock(mutexLock_);
    auto ret = extensionCallbacks_.find(eventId);
    if (ret != extensionCallbacks_.end()) {
        extensionCallbacks_.erase(ret);
    }
}

void AutoFillManager::HandleTimeOut(uint32_t eventId)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    auto ret = extensionCallbacks_.find(eventId);
    if (ret == extensionCallbacks_.end()) {
        TAG_LOGW(AAFwkTag::AUTOFILLMGR, "Event id is not find.");
        return;
    }
    auto extensionCallback = ret->second.lock();
    if (extensionCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Extension callback is nullptr.");
        return;
    }
    extensionCallback->HandleTimeOut();
    extensionCallbacks_.erase(ret);
}
} // namespace AbilityRuntime
} // namespace OHOS
