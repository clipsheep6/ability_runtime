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

#include "event_handler_wrap.h"

#include <mutex>
#include "cpp/mutex.h"

namespace OHOS {
namespace AAFwk {
EventHandlerWrap::EventHandlerWrap() : taskHandler_(TaskHandlerWrap::GetFfrtHandler())
{
    eventMutex_ = std::make_unique<ffrt::mutex>();
}

EventHandlerWrap::EventHandlerWrap(std::shared_ptr<TaskHandlerWrap> taskHandler)
    : taskHandler_(taskHandler)
{
    eventMutex_ = std::make_unique<ffrt::mutex>();
}

EventHandlerWrap::~EventHandlerWrap() = default;

void EventHandlerWrap::ProcessEvent(const EventWrap &event)
{
    if (eventCallback_) {
        eventCallback_(event);
    }
}
bool EventHandlerWrap::SendEvent(uint32_t eventId)
{
    return SendEvent(EventWrap(eventId, 0));
}
bool EventHandlerWrap::SendEvent(uint32_t eventId, int64_t delayMillis)
{
    return SendEvent(EventWrap(eventId, 0), delayMillis);
}
bool EventHandlerWrap::SendEvent(EventWrap event)
{
    return SendEvent(event, 0);
}
bool EventHandlerWrap::SendEvent(EventWrap event, int64_t delayMillis, bool forceInsert)
{
    if (!taskHandler_) {
        return false;
    }
    auto eventStr = event.GetEventString();
    std::lock_guard<ffrt::mutex> guard(*eventMutex_);
    auto it  = eventMap_.find(eventStr);
    if (it != eventMap_.end() && !forceInsert) {
        return false;
    }

    event.SetEventTask(taskHandler_->SubmitTask([wthis = weak_from_this(), event]() {
        auto pthis = wthis.lock();
        if (pthis) {
            pthis->ProcessEvent(event);
            pthis->RemoveEvent(event, false);
        }
    }, delayMillis));

    if (it != eventMap_.end()) {
        it->second = event;
    } else {
        eventMap_.emplace(eventStr, event);
    }

    return true;
}

bool EventHandlerWrap::RemoveEvent(uint32_t eventId, int64_t param)
{
    return RemoveEvent(EventWrap(eventId, param));
}

bool EventHandlerWrap::RemoveEvent(EventWrap event, bool force)
{
    std::lock_guard<ffrt::mutex> guard(*eventMutex_);
    auto it = eventMap_.find(event.GetEventString());
    if (it == eventMap_.end()) {
        return false;
    }
    if (force || it->second.IsSame(event)) {
        it->second.GetEventTask().Cancel();
        eventMap_.erase(it);
        return true;
    }
    return false;
}
}  // namespace AAFWK
}  // namespace OHOS