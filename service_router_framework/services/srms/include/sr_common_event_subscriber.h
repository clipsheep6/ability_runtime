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

#ifndef OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_SERVICES_INCLUDE_SR_COMMON_EVENT_SUBSCRIBER_H
#define OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_SERVICES_INCLUDE_SR_COMMON_EVENT_SUBSCRIBER_H

#include "common_event_subscriber.h"
#include "event_handler.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
class SrCommonEventSubscriber : public EventFwk::CommonEventSubscriber,
    public std::enable_shared_from_this<SrCommonEventSubscriber> {
public:
    SrCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);

    ~SrCommonEventSubscriber();

    void OnReceiveEvent(const EventFwk::CommonEventData &data);

    /**
     * @brief SetEventHandler.
     * @param handler event handler
     */
    inline void SetEventHandler(const std::shared_ptr<EventHandler> &handler)
    {
        eventHandler_ = handler;
    }
private:
    std::shared_ptr<EventHandler> eventHandler_ = nullptr;
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_SERVICES_INCLUDE_SR_COMMON_EVENT_SUBSCRIBER_H