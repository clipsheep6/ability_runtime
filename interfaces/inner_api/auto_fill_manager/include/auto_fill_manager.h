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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H

#include <string>

#include "auto_fill_custom_config.h"
#include "auto_fill_event_handler.h"
#include "auto_fill_extension_callback.h"
#include "fill_request_callback_interface.h"
#include "save_request_callback_interface.h"
#include "task_handler_wrap.h"
#include "ui_content.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
namespace AutoFill {
enum class AutoFillCommand {
    NONE,
    FILL,
    SAVE,
    UPDATE,
    RESIZE,
    INPUT,
    RELOAD_IN_MODAL
};

/**
 * @struct AutoFillRequest
 * AutoFillRequest is used to define the auto fill request parameter structure.
 */
struct AutoFillRequest {
    AbilityBase::AutoFillType autoFillType = AbilityBase::AutoFillType::UNSPECIFIED;
    AutoFillCommand autoFillCommand = AutoFillCommand::NONE;
    AbilityBase::ViewData viewData;
    AutoFillCustomConfig config;
};

/**
 * @struct ReloadInModalRequest
 * ReloadInModalRequest is used to define the reload in modal request parameter structure.
 */
struct ReloadInModalRequest {
    Ace::UIContent *uiContent = nullptr;
    bool isSmartAutoFill = false;
    int32_t nodeId;
    std::string customData;
    AutoFillWindowType autoFillWindowType;
    AbilityBase::AutoFillType autoFillType = AbilityBase::AutoFillType::UNSPECIFIED;
    std::shared_ptr<AutoFillExtensionCallback> extensionCallback;
};
}
class AutoFillManager {
public:
    AutoFillManager() = default;
    ~AutoFillManager();

    static AutoFillManager &GetInstance();

    int32_t RequestAutoFill(
        Ace::UIContent *uiContent,
        const AutoFill::AutoFillRequest &request,
        const std::shared_ptr<IFillRequestCallback> &fillCallback, bool &isPopup);

    int32_t RequestAutoSave(
        Ace::UIContent *uiContent,
        const AutoFill::AutoFillRequest &request,
        const std::shared_ptr<ISaveRequestCallback> &saveCallback);

    void UpdateCustomPopupUIExtension(Ace::UIContent *uiContent, const AbilityBase::ViewData &viewData);
    void SetAutoFillExtensionProxy(Ace::UIContent *uiContent,
        const std::shared_ptr<Ace::ModalUIExtensionProxy> &modalUIExtensionProxy);
    void RemoveAutoFillExtensionProxy(Ace::UIContent *uiContent);
    int32_t ReloadInModal(const AutoFill::ReloadInModalRequest &request);
    void HandleTimeOut(uint32_t eventId);
    void RemoveEvent(uint32_t eventId);
private:
    int32_t HandleRequestExecuteInner(
        Ace::UIContent *uiContent,
        const AutoFill::AutoFillRequest &request,
        const std::shared_ptr<IFillRequestCallback> &fillCallback,
        const std::shared_ptr<ISaveRequestCallback> &saveCallback, bool &isPopup);
    int32_t CreateAutoFillExtension(Ace::UIContent *uiContent,
        const AutoFill::AutoFillRequest &request,
        const Ace::ModalUIExtensionCallbacks &callback,
        const AutoFill::AutoFillWindowType &autoFillWindowType,
        bool isSmartAutoFill);
    void BindModalUIExtensionCallback(
        const std::shared_ptr<AutoFillExtensionCallback> &extensionCallback, Ace::ModalUIExtensionCallbacks &callback);
    void SetTimeOutEvent(uint32_t eventId);
    AutoFill::AutoFillWindowType ConvertAutoFillWindowType(const AutoFill::AutoFillRequest &request,
        bool &isSmartAutoFill);

    std::mutex extensionCallbacksMutex_;
    std::mutex modalProxyMapMutex_;
    std::map<uint32_t, std::weak_ptr<AutoFillExtensionCallback>> extensionCallbacks_;
    std::map<Ace::UIContent *, std::shared_ptr<Ace::ModalUIExtensionProxy>> modalUIExtensionProxyMap_;
    uint32_t eventId_ = 0;
    std::shared_ptr<AutoFillEventHandler> eventHandler_;
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H