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

#include "ability_manager_service.h"
#include "ability_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityEventHandler::AbilityEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, const std::weak_ptr<AbilityManagerService> &server)
    : AppExecFwk::EventHandler(runner), server_(server)
{
    HILOG_INFO("Constructors.");
}

void AbilityEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    CHECK_POINTER(event);
    HILOG_DEBUG("Event id obtained: %{public}u.", event->GetInnerEventId());
#ifdef SUPPORT_GRAPHICS
    ProcessTimeOut(event->GetParam());
#endif
    switch (event->GetInnerEventId()) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG: {
            ProcessLoadTimeOut(event->GetParam());
            break;
        }
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG: {
            ProcessActiveTimeOut(event->GetParam());
            break;
        }
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG: {
            HILOG_INFO("Inactive timeout.");
            // inactivate pre ability immediately in case blocking next ability start
            ProcessInactiveTimeOut(event->GetParam());
            break;
        }
        case AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG: {
            ProcessForegroundNewTimeOut(event->GetParam());
            break;
        }
        case AbilityManagerService::BACKGROUNDNEW_TIMEOUT_MSG: {
            ProcessBackgroundNewTimeOut(event->GetParam());
            break;
        }
        default: {
            HILOG_WARN("Unsupported timeout message.");
            break;
        }
    }
}

#ifdef SUPPORT_GRAPHICS
void AbilityEventHandler::ProcessTimeOut(int64_t eventId)
{
    HILOG_INFO("Cancel starting window.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->ProcessTimeOut(eventId);
}
#endif

void AbilityEventHandler::ProcessLoadTimeOut(int64_t eventId)
{
    HILOG_INFO("Attach timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleLoadTimeOut(eventId);
}

void AbilityEventHandler::ProcessActiveTimeOut(int64_t eventId)
{
    HILOG_INFO("Active timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleActiveTimeOut(eventId);
}

void AbilityEventHandler::ProcessInactiveTimeOut(int64_t eventId)
{
    HILOG_INFO("Inactive timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleInactiveTimeOut(eventId);
}

void AbilityEventHandler::ProcessForegroundNewTimeOut(int64_t eventId)
{
    HILOG_INFO("ForegroundNew timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleForegroundNewTimeOut(eventId);
}

void AbilityEventHandler::ProcessBackgroundNewTimeOut(int64_t eventId)
{
    HILOG_INFO("BackgroundNew timeout.");
    auto server = server_.lock();
    CHECK_POINTER(server);
    server->HandleBackgroundNewTimeOut(eventId);
}
}  // namespace AAFwk
}  // namespace OHOS