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

#include "auto_fill_event_handler.h"

#include "auto_fill_manager.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
AutoFillEventHandler::AutoFillEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{}

void AutoFillEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    HILOG_DEBUG("Called.");
    if (event == nullptr) {
        HILOG_ERROR("Event is nullptr.");
        return;
    }
    AutoFillManager::GetInstance().HandleTimeOut(event->GetInnerEventId());
}
} // namespace AbilityRuntime
} // namespace OHOS