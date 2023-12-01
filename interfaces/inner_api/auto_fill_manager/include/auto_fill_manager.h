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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H

#include <string>

#include "auto_fill_event_handler.h"
#include "auto_fill_extension_callback.h"
#include "fill_request_callback_interface.h"
#include "save_request_callback_interface.h"
#include "task_handler_wrap.h"
#include "ui_content.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
class AutoFillManager {
public:
    AutoFillManager() = default;
    ~AutoFillManager();

    static AutoFillManager &GetInstance();

    int32_t RequestAutoFill(
        const AbilityBase::AutoFillType &autoFillType,
        Ace::UIContent *uiContent,
        const AbilityBase::ViewData &viewdata,
        const std::shared_ptr<IFillRequestCallback> &fillCallback);

    int32_t RequestAutoSave(
        Ace::UIContent *uiContent,
        const AbilityBase::ViewData &viewdata,
        const std::shared_ptr<ISaveRequestCallback> &saveCallback);

    void HandleTimeOut(uint32_t eventId);
    void RemoveEvent(uint32_t eventId);
private:
    void SetTimeOutEvent(uint32_t eventId);
    int32_t HandleRequestExecuteInner(
        const AbilityBase::AutoFillType &autoFillType,
        Ace::UIContent *uiContent,
        const AbilityBase::ViewData &viewdata,
        const std::shared_ptr<IFillRequestCallback> &fillCallback,
        const std::shared_ptr<ISaveRequestCallback> &saveCallback);

    std::mutex mutexLock_;
    std::map<uint32_t, std::weak_ptr<AutoFillExtensionCallback>> extensionCallbacks_;
    uint32_t eventId_ = 0;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AutoFillEventHandler> eventHandler_;
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_H