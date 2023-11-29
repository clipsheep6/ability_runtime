/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_event_handler.h"

#include <parameter.h>
#include "ability_manager_service.h"
#include "ability_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityEventHandler::AbilityEventHandler(
    const std::shared_ptr<TaskHandlerWrap> &taskHandler, const std::weak_ptr<AbilityManagerService> &server)
    : EventHandlerWrap(taskHandler), server_(server)
{
    HILOG_INFO("Constructors.");
}

void AbilityEventHandler::ProcessEvent(const EventWrap &event)
{
    HILOG_DEBUG("Event id obtained: %{public}u.", event.GetEventId());
    // check libc.hook_mode
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        HILOG_DEBUG("Hook_mode: no process time out");
        return;
    }
    switch (event.GetEventId()) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG: {
            ProcessLoadTimeOut(event);
            break;
        }
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG: {
            ProcessActiveTimeOut(event.GetParam());
            break;
        }
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG: {
            HILOG_INFO("Inactive timeout.");
            // inactivate pre ability immediately in case blocking next ability start
            ProcessInactiveTimeOut(event.GetParam());
            break;
        }
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG: {
            ProcessForegroundTimeOut(event);
            break;
        }
        case AbilityManagerService::SHAREDATA_TIMEOUT_MSG: {
            ProcessShareDataTimeOut(event.GetParam());
            break;
        }
        default: {
            HILOG_WARN("Unsupported timeout message.");
            break;
        }
    }
}

void AbilityEventHandler::ProcessLoadTimeOut(const EventWrap &event)
{
    HILOG_INFO("Attach timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    if (event.GetRunCount() == 0) {
        uint32_t timeout = event.GetTimeout();
        if (timeout == 0) {
            timeout = 3000; // 3000 : default timeout
        }
        auto eventWrap = EventWrap(AbilityManagerService::LOAD_TIMEOUT_MSG, event.GetParam());
        eventWrap.SetRunCount(event.GetRunCount() + 1);
        eventWrap.SetTimeout(timeout);
        SendEvent(eventWrap, timeout);
    }
    server->HandleLoadTimeOut(event.GetParam(), event.GetRunCount() == 0);
}

void AbilityEventHandler::ProcessActiveTimeOut(int64_t abilityRecordId)
{
    HILOG_INFO("Active timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleActiveTimeOut(abilityRecordId);
}

void AbilityEventHandler::ProcessInactiveTimeOut(int64_t abilityRecordId)
{
    HILOG_INFO("Inactive timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleInactiveTimeOut(abilityRecordId);
}

void AbilityEventHandler::ProcessForegroundTimeOut(const EventWrap &event)
{
    HILOG_INFO("Foreground timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    if (event.GetRunCount() == 0) {
        uint32_t timeout = event.GetTimeout();
        if (timeout == 0) {
            timeout = 3000; // 3000 : default timeout
        }
        auto eventWrap = EventWrap(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, event.GetParam());
        eventWrap.SetRunCount(event.GetRunCount() + 1);
        eventWrap.SetTimeout(timeout);
        SendEvent(eventWrap, timeout);
    }
    server->HandleForegroundTimeOut(event.GetParam(), event.GetRunCount() == 0);
}

void AbilityEventHandler::ProcessShareDataTimeOut(int64_t uniqueId)
{
    HILOG_INFO("ShareData timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleShareDataTimeOut(uniqueId);
}

}  // namespace AAFwk
}  // namespace OHOS
