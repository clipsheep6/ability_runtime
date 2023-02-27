/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "app_recovery.h"
#include "hisysevent.h"
#include "hilog_wrapper.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr uint32_t CHECK_MAIN_THREAD_IS_ALIVE = 1;
constexpr int RESET_RATIO = 2;

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

void Watchdog::SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &applicationInfo)
{
    applicationInfo_ = applicationInfo;
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

    if (isInBackground_) {
        appMainThreadIsAlive_.store(true);
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

    if (applicationInfo_ == nullptr) {
        HILOG_ERROR("reportEvent fail, applicationInfo_ is nullptr.");
        return;
    }

    if (!needReport_) {
        return;
    }

    std::string eventType;
    if (isSixSecondEvent_) {
        eventType = "THREAD_BLOCK_6S";
        needReport_.store(false);
    } else {
        eventType = "THREAD_BLOCK_3S";
        isSixSecondEvent_.store(true);
    }
    std::string msgContent = "App main thread is not response!";
    MainHandlerDumper handlerDumper;
    appMainHandler_->Dump(handlerDumper);
    msgContent += handlerDumper.GetDumpInfo();

    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, eventType,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_UID, applicationInfo_->uid,
        EVENT_KEY_PID, static_cast<int32_t>(getpid()), EVENT_KEY_PACKAGE_NAME, applicationInfo_->bundleName,
        EVENT_KEY_PROCESS_NAME, applicationInfo_->process, EVENT_KEY_MESSAGE, msgContent);
    HILOG_INFO("reportEvent success, %{public}zu %{public}s", msgContent.size(), msgContent.c_str());

    // should call error manager-> appRecovery
    if (isSixSecondEvent_) {
        AppRecovery::GetInstance().ScheduleSaveAppState(StateReason::APP_FREEZE);
        AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::APP_FREEZE);
    }
}

void MainHandlerDumper::Dump(const std::string &message)
{
    HILOG_DEBUG("message is %{public}s", message.c_str());
    dumpInfo += message;
}

std::string MainHandlerDumper::GetTag()
{
    return "";
}

std::string MainHandlerDumper::GetDumpInfo()
{
    return dumpInfo;
}
}  // namespace AppExecFwk
}  // namespace OHOS
