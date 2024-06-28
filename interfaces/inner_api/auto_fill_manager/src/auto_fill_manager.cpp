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
#include "parameters.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string WANT_PARAMS_EXTENSION_TYPE = "autoFill/password";
const std::string WANT_PARAMS_SMART_EXTENSION_TYPE = "autoFill/smart";
const std::string AUTO_FILL_START_POPUP_WINDOW = "persist.sys.abilityms.autofill.is_passwd_popup_window";
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
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

AutoFillManager::AutoFillManager()
{
    auto runner = AppExecFwk::EventRunner::Create(AUTO_FILL_MANAGER_THREAD);
    eventHandler_ = std::make_shared<AutoFillEventHandler>(runner);
}

AutoFillManager::~AutoFillManager()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
}

int32_t AutoFillManager::RequestAutoFill(Ace::UIContent *uiContent, const AutoFill::AutoFillRequest &request,
    const std::shared_ptr<IFillRequestCallback> &fillCallback, AutoFill::AutoFillResult &result)
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
    return HandleRequestExecuteInner(uiContent, request, fillCallback, nullptr, result);
}

int32_t AutoFillManager::RequestAutoSave(Ace::UIContent *uiContent, const AutoFill::AutoFillRequest &request,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback, AutoFill::AutoFillResult &result)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (uiContent == nullptr || saveCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent or save callback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    return HandleRequestExecuteInner(uiContent, request, nullptr, saveCallback, result);
}

int32_t AutoFillManager::HandleRequestExecuteInner(Ace::UIContent *uiContent, const AutoFill::AutoFillRequest &request,
    const std::shared_ptr<IFillRequestCallback> &fillCallback,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback,
    AutoFill::AutoFillResult &result)
{
    if (uiContent == nullptr || (fillCallback == nullptr && saveCallback == nullptr)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "UIContent or fillCallback&saveCallback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    if (!IsPreviousRequestFinished(uiContent)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Previous request is not finished.");
        return AutoFill::AUTO_FILL_PREVIOUS_REQUEST_NOT_FINISHED;
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
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }

    auto callbackId = extensionCallback->GetCallbackId();
    SetTimeOutEvent(callbackId);
    result.isPopup = autoFillWindowType == AutoFill::AutoFillWindowType::POPUP_WINDOW ? true : false;
    auto sessionId = CreateAutoFillExtension(uiContent, request, callback, autoFillWindowType, isSmartAutoFill);
    if (sessionId == AUTO_FILL_UI_EXTENSION_SESSION_ID_INVALID) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Create ui extension is failed.");
        RemoveEvent(callbackId);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    result.autoFillSessionId = callbackId;
    extensionCallback->SetInstanceId(uiContent->GetInstanceId());
    extensionCallback->SetSessionId(sessionId);
    extensionCallback->SetViewData(request.viewData);
    extensionCallback->SetWindowType(autoFillWindowType);
    extensionCallback->SetExtensionType(isSmartAutoFill);
    extensionCallback->SetAutoFillType(request.autoFillType);
    extensionCallback->SetAutoFillRequestConfig(request.config);
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "callbackId: %{public}u.", callbackId);
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    extensionCallbacks_.emplace(callbackId, extensionCallback);
    return AutoFill::AUTO_FILL_SUCCESS;
}

void AutoFillManager::UpdateCustomPopupUIExtension(uint32_t autoFillSessionId, const AbilityBase::ViewData &viewData)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    auto extensionCallback = GetAutoFillExtensionCallback(autoFillSessionId);
    if (extensionCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Extension callback is nullptr.");
        return;
    }
    extensionCallback->UpdateCustomPopupUIExtension(viewData);
}

void AutoFillManager::BindModalUIExtensionCallback(
    const std::shared_ptr<AutoFillExtensionCallback> &extensionCallback, Ace::ModalUIExtensionCallbacks &callback)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
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
        config.doAfterAsyncModalBinding = std::move(request.doAfterAsyncModalBinding);
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
    if (eventHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Eventhandler is nullptr.");
        return;
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
}

void AutoFillManager::HandleTimeOut(uint32_t eventId)
{
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "Called.");
    auto extensionCallback = GetAutoFillExtensionCallback(eventId);
    if (extensionCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Extension callback is nullptr.");
        return;
    }
    extensionCallback->HandleTimeOut();
}

std::shared_ptr<AutoFillExtensionCallback> AutoFillManager::GetAutoFillExtensionCallback(uint32_t callbackId)
{
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    auto iter = extensionCallbacks_.find(callbackId);
    if (iter == extensionCallbacks_.end()) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "callback is not find. callbackId: %{public}u", callbackId);
        return nullptr;
    }
    return iter->second;
}

void AutoFillManager::RemoveAutoFillExtensionCallback(uint32_t callbackId)
{
    TAG_LOGI(AAFwkTag::AUTOFILLMGR, "callbackId: %{public}u", callbackId);
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    extensionCallbacks_.erase(callbackId);
}

bool AutoFillManager::IsPreviousRequestFinished(Ace::UIContent *uiContent)
{
    if (uiContent == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(extensionCallbacksMutex_);
    for (const auto& item: extensionCallbacks_) {
        auto extensionCallback = item.second;
        if (extensionCallback == nullptr) {
            continue;
        }
        if (extensionCallback->GetWindowType() == AutoFill::AutoFillWindowType::MODAL_WINDOW &&
            extensionCallback->GetInstanceId() == uiContent->GetInstanceId()) {
            return false;
        }
    }
    return true;
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
