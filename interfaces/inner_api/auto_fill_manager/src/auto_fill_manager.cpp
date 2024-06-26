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
const std::string AUTO_FILL_START_POPUP_WINDOW = "persist.sys.abilityms.autofill.is_passwd_popup_window";
#ifdef SUPPORT_GRAPHICS
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
constexpr static char WANT_PARAMS_CUSTOM_DATA_KEY[] = "ohos.ability.params.customData";
constexpr static char WANT_PARAMS_AUTO_FILL_CMD_KEY[] = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY[] = "ohos.ability.params.popupWindow";
constexpr static char WANT_PARAMS_EXTENSION_TYPE_KEY[] = "ability.want.params.uiExtensionType";
constexpr static char WANT_PARAMS_AUTO_FILL_TYPE_KEY[] = "ability.want.params.AutoFillType";
constexpr static char AUTO_FILL_MANAGER_THREAD[] = "AutoFillManager";
constexpr static uint32_t AUTO_FILL_REQUEST_TIME_OUT_VALUE = 1000;
constexpr static uint32_t AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID = 0;
#endif //SUPPORT_GRAPHICS
} // namespace
#ifdef SUPPORT_GRAPHICS
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
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Auto fill type is invalid.");
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
    if (!IsPreviousRequestFinished(uiContent)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Previous request is not finished.");
        return AutoFill::AUTO_FILL_PREVIOUS_REQUEST_NOT_FINISHED;
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
    BindModalUIExtensionCallback(extensionCallback, callback);

    bool isSmartAutoFill = false;
    AutoFill::AutoFillWindowType autoFillWindowType = AutoFill::AutoFillWindowType::MODAL_WINDOW;
    if (!ConvertAutoFillWindowType(request, isSmartAutoFill, autoFillWindowType)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Convert auto fill type failed.");
        RemoveEvent(eventId_);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }

    isPopup = autoFillWindowType == AutoFill::AutoFillWindowType::POPUP_WINDOW ? true : false;
    auto sessionId = CreateAutoFillExtension(uiContent, request, callback, autoFillWindowType, isSmartAutoFill);
    if (sessionId == AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Create ui extension is failed.");
        RemoveEvent(eventId_);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    extensionCallback->SetInstanceId(uiContent->GetInstanceId());
    extensionCallback->SetSessionId(sessionId);
    extensionCallback->SetEventId(eventId_);
    extensionCallback->SetViewData(request.viewData);
    extensionCallback->SetWindowType(autoFillWindowType);
    extensionCallback->SetExtensionType(isSmartAutoFill);
    extensionCallback->SetAutoFillType(request.autoFillType);
    extensionCallback->SetAutoFillRequestConfig(request.config);
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
        auto it = modalUIExtensionProxyMap_.find(uiContent->GetInstanceId());
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

int32_t AutoFillManager::UpdateCustomPopupConfig(int32_t instanceId,
    const Ace::CustomPopupUIExtensionConfig &popupConfig)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    auto uiContent = Ace::UIContent::GetUIContent(instanceId);
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    uiContent->UpdateCustomPopupUIExtension(popupConfig);
    return AutoFill::AUTO_FILL_SUCCESS;
}

void AutoFillManager::SetAutoFillExtensionProxy(int32_t instanceId,
    const std::shared_ptr<Ace::ModalUIExtensionProxy> &modalUIExtensionProxy)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (modalUIExtensionProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "proxy is nullptr.");
        return;
    }

    std::lock_guard<std::mutex> lock(modalProxyMapMutex_);
    auto it = modalUIExtensionProxyMap_.find(instanceId);
    if (it != modalUIExtensionProxyMap_.end()) {
        modalUIExtensionProxyMap_.erase(it);
    }
    modalUIExtensionProxyMap_.emplace(instanceId, modalUIExtensionProxy);
}

void AutoFillManager::RemoveAutoFillExtensionProxy(int32_t instanceId)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    std::lock_guard<std::mutex> lock(modalProxyMapMutex_);
    auto it = modalUIExtensionProxyMap_.find(instanceId);
    if (it != modalUIExtensionProxyMap_.end()) {
        modalUIExtensionProxyMap_.erase(it);
    }
}

void AutoFillManager::BindModalUIExtensionCallback(
    const std::shared_ptr<AutoFillExtensionCallback> &extensionCallback, Ace::ModalUIExtensionCallbacks &callback)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    callback.onResult = [extensionCallback](int arg1, int arg2)
    {
        extensionCallback->OnResult(arg1, arg2);
    };

    callback.onRelease = [extensionCallback](int arg1)
    {
        extensionCallback->OnRelease(arg1);
    };

    callback.onError = [extensionCallback](int arg1, int arg2, int arg3)
    {
        extensionCallback->OnError(arg1, arg2, arg3);
    };

    callback.onReceive = [extensionCallback](int arg1)
    {
        extensionCallback->OnReceive(arg1);
    };

    callback.onRemoteReady = [extensionCallback](int arg1)
    {
        extensionCallback->onRemoteReady(arg1);
    };

    callback.onDestroy = [extensionCallback]()
    {
        extensionCallback->onDestroy();
    };
}

int32_t AutoFillManager::ReloadInModal(const AutoFill::ReloadInModalRequest &request)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    auto uiContent = Ace::UIContent::GetUIContent(request.instanceId);
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Content is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }

    if (request.extensionCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Extension callback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }

    if (request.autoFillType == AbilityBase::AutoFillType::UNSPECIFIED) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Auto fill type is invalid.");
        return AutoFill::AUTO_FILL_TYPE_INVALID;
    }

    {
        std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
        SetTimeOutEvent(++eventId_);
    }

    AAFwk::Want want;
    want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, static_cast<int32_t>(AutoFill::AutoFillCommand::RELOAD_IN_MODAL));
    want.SetParam(WANT_PARAMS_CUSTOM_DATA_KEY, request.customData);
    request.isSmartAutoFill ? want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_SMART_EXTENSION_TYPE) :
        want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_EXTENSION_TYPE);
    want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(request.autoFillType));
    want.SetParam(WANT_PARAMS_VIEW_DATA_KEY, request.extensionCallback->GetViewData().ToJsonString());
    want.SetParam(WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY, false);
    Ace::ModalUIExtensionCallbacks callback;
    BindModalUIExtensionCallback(request.extensionCallback, callback);
    Ace::ModalUIExtensionConfig config;
    config.isAsyncModalBinding = true;
    int32_t sessionId = AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID;
    sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Create ui extension is failed.");
        RemoveEvent(eventId_);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    request.extensionCallback->SetSessionId(sessionId);
    request.extensionCallback->SetEventId(eventId_);
    request.extensionCallback->SetWindowType(AutoFill::AutoFillWindowType::MODAL_WINDOW);
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    extensionCallbacks_.emplace(eventId_, request.extensionCallback);
    return AutoFill::AUTO_FILL_SUCCESS;
}

int32_t AutoFillManager::CreateAutoFillExtension(Ace::UIContent *uiContent,
    const AutoFill::AutoFillRequest &request,
    const Ace::ModalUIExtensionCallbacks &callback,
    const AutoFill::AutoFillWindowType &autoFillWindowType,
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

bool AutoFillManager::IsNeed2SaveRequest(const AbilityBase::ViewData& viewData, bool& isSmartAutoFill)
{
    bool ret = false;
    for (auto it = viewData.nodes.begin(); it != viewData.nodes.end(); ++it) {
        if ((it->autoFillType == AbilityBase::AutoFillType::PASSWORD ||
            it->autoFillType == AbilityBase::AutoFillType::USER_NAME ||
            it->autoFillType == AbilityBase::AutoFillType::NEW_PASSWORD) &&
            it->enableAutoFill && !it->value.empty()) {
            isSmartAutoFill = false;
            return true;
        }
        if (AbilityBase::AutoFillType::FULL_STREET_ADDRESS <= it->autoFillType &&
            it->autoFillType <= AbilityBase::AutoFillType::FORMAT_ADDRESS &&
            it->enableAutoFill && !it->value.empty()) {
            isSmartAutoFill = true;
            ret = true;
        }
    }
    return ret;
}

bool AutoFillManager::ConvertAutoFillWindowType(const AutoFill::AutoFillRequest &request,
    bool &isSmartAutoFill, AutoFill::AutoFillWindowType &autoFillWindowType)
{
    bool ret = true;
    autoFillWindowType = AutoFill::AutoFillWindowType::MODAL_WINDOW;
    AbilityBase::AutoFillType autoFillType = request.autoFillType;
    if (autoFillType >= AbilityBase::AutoFillType::FULL_STREET_ADDRESS &&
        autoFillType <= AbilityBase::AutoFillType::FORMAT_ADDRESS) {
        autoFillWindowType = AutoFill::AutoFillWindowType::POPUP_WINDOW;
        isSmartAutoFill = true;
    } else if (autoFillType == AbilityBase::AutoFillType::PASSWORD ||
        autoFillType == AbilityBase::AutoFillType::USER_NAME ||
        autoFillType == AbilityBase::AutoFillType::NEW_PASSWORD) {
        if (system::GetBoolParameter(AUTO_FILL_START_POPUP_WINDOW, false)) {
            autoFillWindowType = AutoFill::AutoFillWindowType::POPUP_WINDOW;
        } else {
            autoFillWindowType = AutoFill::AutoFillWindowType::MODAL_WINDOW;
        }
        isSmartAutoFill = false;
    }

    if (request.autoFillCommand == AutoFill::AutoFillCommand::SAVE) {
        ret = IsNeed2SaveRequest(request.viewData, isSmartAutoFill);
        autoFillWindowType = AutoFill::AutoFillWindowType::MODAL_WINDOW;
    }
    return ret;
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
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "Called.");
    if (eventHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Eventhandler is nullptr.");
        return;
    }
    eventHandler_->RemoveEvent(eventId);

    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    auto ret = extensionCallbacks_.find(eventId);
    if (ret != extensionCallbacks_.end()) {
        extensionCallbacks_.erase(ret);
    }
}

void AutoFillManager::HandleTimeOut(uint32_t eventId)
{
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "Called.");
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

bool AutoFillManager::IsPreviousRequestFinished(Ace::UIContent *uiContent)
{
    if (uiContent == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    for (auto& item: extensionCallbacks_) {
        auto extensionCallback = item.second.lock();
        if (extensionCallback == nullptr) {
            continue;
        }
        if (extensionCallback->GetInstanceId() == uiContent->GetInstanceId()) {
            return false;
        }
    }
    return true;
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
