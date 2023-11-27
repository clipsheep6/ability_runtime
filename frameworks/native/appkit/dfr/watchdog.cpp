/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "watchdog.h"

#include <parameter.h>
#include <unistd.h>

#include "app_mgr_client.h"
#include "app_recovery.h"
#include "appfreeze_inner.h"
#include "hisysevent.h"
#include "hilog_wrapper.h"
#include "mix_stack_dumper.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr uint32_t CHECK_MAIN_THREAD_IS_ALIVE = 1;
constexpr int RESET_RATIO = 2;

constexpr int32_t BACKGROUND_REPORT_COUNT_MAX = 5;
#ifdef SUPPORT_ASAN
constexpr uint32_t CHECK_INTERVAL_TIME = 45000;
#else
constexpr uint32_t CHECK_INTERVAL_TIME = 3000;
#endif
}
std::shared_ptr<EventHandler> Watchdog::appMainHandler_ = nullptr;

Watchdog::Watchdog()
{}

Watchdog::~Watchdog()
{
    if (!stopWatchdog_) {
        HILOG_DEBUG("Stop watchdog when deconstruct.");
        OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();
    }
}

void Watchdog::Init(const std::shared_ptr<EventHandler> mainHandler)
{
    Watchdog::appMainHandler_ = mainHandler;
    if (appMainHandler_ != nullptr) {
        HILOG_DEBUG("Watchdog init send event");
        appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE);
    }
    lastWatchTime_ = 0;
    auto watchdogTask = std::bind(&Watchdog::Timer, this);
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        CHECK_INTERVAL_TIME, INI_TIMER_FIRST_SECOND);
}

void Watchdog::Stop()
{
    HILOG_DEBUG("Watchdog is stop!");
    std::unique_lock<std::mutex> lock(cvMutex_);
    if (stopWatchdog_) {
        HILOG_DEBUG("Watchdog has stoped.");
        return;
    }
    stopWatchdog_.store(true);
    cvWatchdog_.notify_all();
    OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();

    if (appMainHandler_) {
        appMainHandler_.reset();
        appMainHandler_ = nullptr;
    }
}

void Watchdog::SetAppMainThreadState(const bool appMainThreadState)
{
    appMainThreadIsAlive_.store(appMainThreadState);
}

void Watchdog::SetBackgroundStatus(const bool isInBackground)
{
    isInBackground_.store(isInBackground);
}

void Watchdog::AllowReportEvent()
{
    needReport_.store(true);
    isSixSecondEvent_.store(false);
    backgroundReportCount_.store(0);
}

bool Watchdog::IsReportEvent()
{
    if (appMainThreadIsAlive_) {
        appMainThreadIsAlive_.store(false);
        return false;
    }
    HILOG_DEBUG("AppMainThread is not alive");
    return true;
}

bool Watchdog::IsStopWatchdog()
{
    return stopWatchdog_;
}

void Watchdog::Timer()
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    if (stopWatchdog_) {
        HILOG_DEBUG("Watchdog has stoped.");
        return;
    }
    if (!needReport_) {
        HILOG_ERROR("Watchdog timeout, wait for the handler to recover, and do not send event.");
        return;
    }

    if (IsReportEvent()) {
        const int bufferLen = 128;
        char paramOutBuf[bufferLen] = {0};
        const char *hook_mode = "startup:";
        int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
        if (ret <= 0 || strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) != 0) {
            ReportEvent();
        }
    }
    if (appMainHandler_ != nullptr) {
        appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE);
    }
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if ((now - lastWatchTime_) >= (CHECK_INTERVAL_TIME / RESET_RATIO)) {
        lastWatchTime_ = now;
    }
}

void Watchdog::ReportEvent()
{
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if ((now - lastWatchTime_) > (RESET_RATIO * CHECK_INTERVAL_TIME) ||
        (now - lastWatchTime_) < (CHECK_INTERVAL_TIME / RESET_RATIO)) {
        HILOG_INFO("Thread may be blocked, do not report this time. currTime: %{public}llu, lastTime: %{public}llu",
            static_cast<unsigned long long>(now), static_cast<unsigned long long>(lastWatchTime_));
        return;
    }

    if (isInBackground_ && backgroundReportCount_.load() < BACKGROUND_REPORT_COUNT_MAX) {
        HILOG_INFO("In Background, thread may be blocked in, do not report this time. "
            "currTime: %{public}" PRIu64 ", lastTime: %{public}" PRIu64 "",
            static_cast<uint64_t>(now), static_cast<uint64_t>(lastWatchTime_));
        backgroundReportCount_++;
        return;
    }
    backgroundReportCount_++;

    if (!needReport_) {
        return;
    }

    if (isSixSecondEvent_) {
        needReport_.store(false);
    }
    AppExecFwk::AppfreezeInner::GetInstance()->ThreadBlock(isSixSecondEvent_);
}
}  // namespace AppExecFwk
}  // namespace OHOS
