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
#include "extension_ability_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string WANT_PARAMS_AUTO_FILL_CMD = "fill";
const std::string WANT_PARAMS_AUTO_SAVE_CMD = "save";
const std::string WANT_PARAMS_EXTENSION_TYPE = "autoFill/password";
constexpr static char WANT_PARAMS_VIEW_DATA_KEY[] = "ohos.ability.params.viewData";
constexpr static char WANT_PARAMS_AUTO_FILL_CMD_KEY[] = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_EXTENSION_TYPE_KEY[] = "ability.want.params.uiExtensionType";
constexpr static char WANT_PARAMS_AUTO_FILL_TYPE_KEY[] = "ability.want.params.AutoFillType";
constexpr static char AUTO_FILL_MANAGER_THREAD[] = "AutoFillManager";
constexpr static uint32_t AUTO_FILL_REQUEST_TIME_OUT_VALUE = 1000;
} // namespace
AutoFillManager &AutoFillManager::GetInstance()
{
    static AutoFillManager instance;
    return instance;
}

AutoFillManager::~AutoFillManager()
{
    HILOG_DEBUG("Called.");
    if (eventHandler_ != nullptr) {
        eventHandler_.reset();
    }
}

int32_t AutoFillManager::RequestAutoFill(
    const AbilityBase::AutoFillType &autoFillType,
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<IFillRequestCallback> &fillCallback)
{
    HILOG_DEBUG("Called.");
    if (uiContent == nullptr || fillCallback == nullptr) {
        HILOG_ERROR("UIContent or fillCallback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    return HandleRequestExecuteInner(autoFillType, uiContent, viewdata, fillCallback, nullptr);
}

int32_t AutoFillManager::RequestAutoSave(
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback)
{
    HILOG_DEBUG("Called.");
    if (uiContent == nullptr || saveCallback == nullptr) {
        HILOG_ERROR("UIContent or save callback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    return HandleRequestExecuteInner(
        AbilityBase::AutoFillType::UNSPECIFIED, uiContent, viewdata, nullptr, saveCallback);
}

int32_t AutoFillManager::HandleRequestExecuteInner(
    const AbilityBase::AutoFillType &autoFillType,
    Ace::UIContent *uiContent,
    const AbilityBase::ViewData &viewdata,
    const std::shared_ptr<IFillRequestCallback> &fillCallback,
    const std::shared_ptr<ISaveRequestCallback> &saveCallback)
{
    if (uiContent == nullptr || (fillCallback == nullptr && saveCallback == nullptr)) {
        HILOG_ERROR("UIContent or fillCallback&saveCallback is nullptr.");
        return AutoFill::AUTO_FILL_OBJECT_IS_NULL;
    }
    {
        std::lock_guard<std::mutex> lock(mutexLock_);
        SetTimeOutEvent(++eventId_);
    }
    AAFwk::Want want;
    want.SetParam(WANT_PARAMS_EXTENSION_TYPE_KEY, WANT_PARAMS_EXTENSION_TYPE);
    want.SetParam(WANT_PARAMS_VIEW_DATA_KEY, viewdata.ToJsonString());

    auto extensionCallback = std::make_shared<AutoFillExtensionCallback>();
    if (fillCallback != nullptr) {
        want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, WANT_PARAMS_AUTO_FILL_CMD);
        want.SetParam(WANT_PARAMS_AUTO_FILL_TYPE_KEY, static_cast<int32_t>(autoFillType));
        extensionCallback->SetFillRequestCallback(fillCallback);
    } else {
        want.SetParam(WANT_PARAMS_AUTO_FILL_CMD_KEY, WANT_PARAMS_AUTO_SAVE_CMD);
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
    Ace::ModalUIExtensionConfig config;
    config.isAsyncModalBinding = true;
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        HILOG_ERROR("Create modal ui extension is failed.");
        RemoveEvent(eventId_);
        return AutoFill::AUTO_FILL_CREATE_MODULE_UI_EXTENSION_FAILED;
    }
    extensionCallback->SetUIContent(uiContent);
    extensionCallback->SetSessionId(sessionId);
    extensionCallback->SetEventId(eventId_);
    std::lock_guard<std::mutex> lock(mutexLock_);
    extensionCallbacks_.emplace(eventId_, extensionCallback);
    return AutoFill::AUTO_FILL_SUCCESS;
}

void AutoFillManager::SetTimeOutEvent(uint32_t eventId)
{
    HILOG_DEBUG("Called.");
    auto runner = AppExecFwk::EventRunner::Create(AUTO_FILL_MANAGER_THREAD);
    if (eventHandler_ == nullptr) {
        HILOG_DEBUG("Eventhandler is nullptr.");
        eventHandler_ = std::make_shared<AutoFillEventHandler>(runner);
    }
    eventHandler_->SendEvent(eventId, AUTO_FILL_REQUEST_TIME_OUT_VALUE);
}

void AutoFillManager::RemoveEvent(uint32_t eventId)
{
    HILOG_DEBUG("Called.");
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("Eventhandler is nullptr.");
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
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(mutexLock_);
    auto ret = extensionCallbacks_.find(eventId);
    if (ret == extensionCallbacks_.end()) {
        HILOG_WARN("Event id is not find.");
        return;
    }
    auto extensionCallback = ret->second.lock();
    if (extensionCallback == nullptr) {
        HILOG_ERROR("Extension callback is nullptr.");
        return;
    }
    extensionCallback->HandleTimeOut();
    extensionCallbacks_.erase(ret);
}
} // namespace AbilityRuntime
} // namespace OHOS
