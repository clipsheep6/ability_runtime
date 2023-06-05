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
#include "ohos_loop_handler.h"

namespace OHOS {
namespace AbilityRuntime {

void OHOSLoopHandler::OnReadable(int32_t)
{
    HILOG_DEBUG("OHOSLoopHandler::OnReadable is triggered");
    OnTriggered();
}

void OHOSLoopHandler::OnWritable(int32_t)
{
    HILOG_DEBUG("OHOSLoopHandler::OnWritable is triggered");
    OnTriggered();
}

void OHOSLoopHandler::OnTriggered()
{
    HILOG_DEBUG("OHOSLoopHandler::OnTriggered is triggered");

    auto fd = uv_backend_fd(uvLoop_);
    struct epoll_event ev;
    do {
        uv_run(uvLoop_, UV_RUN_NOWAIT);
    } while (epoll_wait(fd, &ev, 1, 0) > 0);

    auto eventHandler = GetOwner();
    if (!eventHandler) {
        return;
    }

    int32_t timeout = uv_backend_timeout(uvLoop_);
    if (timeout < 0) {
        if (haveTimerTask_) {
            eventHandler->RemoveTask(TIMER_TASK);
        }
        return;
    }

    int64_t timeStamp = static_cast<int64_t>(uv_now(uvLoop_)) + timeout;
    if (timeStamp == lastTimeStamp_) {
        return;
    }

    if (haveTimerTask_) {
        eventHandler->RemoveTask(TIMER_TASK);
    }

    auto callback = [wp = weak_from_this()] {
        auto sp = wp.lock();
        if (sp) {
            // Timer task is triggered, so there is no timer task now.
            sp->haveTimerTask_ = false;
            sp->OnTriggered();
        }
    };
    eventHandler->PostTask(callback, TIMER_TASK, timeout);
    lastTimeStamp_ = timeStamp;
    haveTimerTask_ = true;
}
} // namespace AbilityRuntime
} // namespace OHOS