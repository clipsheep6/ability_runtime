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
#include "nocopyable.h"
#include "save_request_callback_interface.h"
#include "task_handler_wrap.h"
#ifdef SUPPORT_GRAPHICS
#include "ui_content.h"
#endif // SUPPORT_GRAPHICS
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
    std::function<void()> doAfterAsyncModalBinding;
};

struct AutoFillResult {
    bool isPopup = false;
    uint32_t autoFillSessionId = 0;
};
}
#ifdef SUPPORT_GRAPHICS
class AutoFillManager {
public:
    static AutoFillManager &GetInstance();

    int32_t RequestAutoFill(Ace::UIContent *uiContent, const AutoFill::AutoFillRequest &request,
        const std::shared_ptr<IFillRequestCallback> &fillCallback, AutoFill::AutoFillResult &result);

    int32_t RequestAutoSave(Ace::UIContent *uiContent, const AutoFill::AutoFillRequest &request,
        const std::shared_ptr<ISaveRequestCallback> &saveCallback, AutoFill::AutoFillResult &result);

    void UpdateCustomPopupUIExtension(uint32_t autoFillSessionId, const AbilityBase::ViewData &viewData);

    void CloseUIExtension(uint32_t autoFillSessionId);

    void HandleTimeOut(uint32_t eventId);
    void SetTimeOutEvent(uint32_t eventId);
    void RemoveEvent(uint32_t eventId);

    void BindModalUIExtensionCallback(const std::shared_ptr<AutoFillExtensionCallback> &extensionCallback,
        Ace::ModalUIExtensionCallbacks &callback);
    void RemoveAutoFillExtensionCallback(uint32_t callbackId);
private:
    AutoFillManager();
    ~AutoFillManager();
    DISALLOW_COPY_AND_MOVE(AutoFillManager);

    int32_t HandleRequestExecuteInner(Ace::UIContent *uiContent, const AutoFill::AutoFillRequest &request,
        const std::shared_ptr<IFillRequestCallback> &fillCallback,
        const std::shared_ptr<ISaveRequestCallback> &saveCallback,
        AutoFill::AutoFillResult &result);

    int32_t CreateAutoFillExtension(Ace::UIContent *uiContent,
        const AutoFill::AutoFillRequest &request,
        const Ace::ModalUIExtensionCallbacks &callback,
        const AutoFill::AutoFillWindowType &autoFillWindowType,
        bool isSmartAutoFill);

    bool ConvertAutoFillWindowType(const AutoFill::AutoFillRequest &request,
        bool &isSmartAutoFill, AutoFill::AutoFillWindowType &autoFillWindowType);
    std::shared_ptr<AutoFillExtensionCallback> GetAutoFillExtensionCallback(uint32_t callbackId);
    bool IsPreviousRequestFinished(Ace::UIContent *uiContent);
    bool IsNeed2SaveRequest(const AbilityBase::ViewData &viewData, bool &isSmartAutoFill);

    std::mutex extensionCallbacksMutex_;
    std::map<uint32_t, std::shared_ptr<AutoFillExtensionCallback>> extensionCallbacks_;

    std::shared_ptr<AutoFillEventHandler> eventHandler_;
};
#endif // SUPPORT_GRAPHICS
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H