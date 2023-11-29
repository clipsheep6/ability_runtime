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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_EVENT_HANDLER_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_EVENT_HANDLER_H

#include <memory>

#include "event_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;
class AbilityEventHandler : public EventHandlerWrap {
public:
    AbilityEventHandler(
        const std::shared_ptr<TaskHandlerWrap> &taskHandler, const std::weak_ptr<AbilityManagerService> &server) {}
    virtual ~AbilityEventHandler() = default;

    void ProcessEvent(const EventWrap &event) override {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_EVENT_HANDLER_H
