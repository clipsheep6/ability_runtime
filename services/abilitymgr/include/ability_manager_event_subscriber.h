/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H

#include "common_event_data.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityManagerEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit AbilityManagerEventSubscriber(
        const EventFwk::CommonEventSubscribeInfo &subscribeInfo, const std::function<void()> &callback);

    ~AbilityManagerEventSubscriber() override = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

private:
    std::function<void()> callback_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_EVENT_SUBSCRIBER_H