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

#ifndef OHOS_ABILITY_RUNTIME_EVENT_HANDLER_WRAP_H
#define OHOS_ABILITY_RUNTIME_EVENT_HANDLER_WRAP_H

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>

#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
class EventDataBase {
public:
    virtual ~EventDataBase() = default;
};
class EventWrap {
public:
    EventWrap(uint32_t eventId) : EventWrap(eventId, 0) {}
    EventWrap(uint32_t eventId, int64_t param) : eventId_(eventId), param_(param)
    {
        eventData_ = std::make_shared<EventDataBase>();
    }
    EventWrap(uint32_t eventId, std::shared_ptr<EventDataBase> data)
        : eventId_(eventId), param_(0), eventData_(data)
    {
        if (!eventData_) {
            eventData_ = std::make_shared<EventDataBase>();
        }
    }
    uint32_t GetEventId() const
    {
        return eventId_;
    }
    int64_t GetParam() const
    {
        return param_;
    }
    const std::shared_ptr<EventDataBase>& GetEventData() const
    {
        return eventData_;
    }
    const TaskHandle& GetEventTask() const
    {
        return eventTask_;
    }
    void SetEventTask(const TaskHandle &eventTask)
    {
        eventTask_ = eventTask;
    }
    std::string GetEventString()
    {
        return std::to_string(eventId_) + "_" + std::to_string(param_);
    }
    bool IsSame(const EventWrap &other) const
    {
        return eventData_ == other.eventData_;
    }
    void SetRunCount(int runCount)
    {
        runCount_ = runCount;
    }
    int GetRunCount() const
    {
        return runCount_;
    }
    void SetTimeout(uint32_t timeout)
    {
        timeout_ = timeout;
    }
    uint32_t GetTimeout() const
    {
        return timeout_;
    }
private:
    uint32_t eventId_;
    int64_t param_;
    std::shared_ptr<EventDataBase> eventData_;
    TaskHandle eventTask_;
    int runCount_ = 0;
    uint32_t timeout_ = 0;
};

class EventHandlerWrap : public std::enable_shared_from_this<EventHandlerWrap> {
public:
    EventHandlerWrap();
    EventHandlerWrap(std::shared_ptr<TaskHandlerWrap> taskHandler);
    EventHandlerWrap(EventHandlerWrap &) = delete;
    void operator=(EventHandlerWrap &) = delete;
    virtual ~EventHandlerWrap();
    virtual void ProcessEvent(const EventWrap &event);
    bool SendEvent(uint32_t eventId);
    bool SendEvent(uint32_t eventId, int64_t delayMillis);
    bool SendEvent(EventWrap event);
    bool SendEvent(EventWrap event, int64_t delayMillis, bool forceInsert = true);
    bool RemoveEvent(uint32_t eventId, int64_t param = 0);
    bool RemoveEvent(EventWrap event, bool force = true);

    void SetEventCallback(std::function<void(const EventWrap&)> eventCallback)
    {
        eventCallback_ = eventCallback;
    }
protected:
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::function<void(const EventWrap&)> eventCallback_;

    std::unique_ptr<ffrt::mutex> eventMutex_;
    std::unordered_map<std::string, EventWrap> eventMap_;
};
}  // namespace AAFWK
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EVENT_HANDLER_WRAP_H