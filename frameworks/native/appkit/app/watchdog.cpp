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
}
std::shared_ptr<EventHandler> Watchdog::appMainHandler_ = nullptr;

Watchdog::Watchdog()
{}

void Watchdog::Init(const std::shared_ptr<EventHandler> &mainHandler)
{
    Watchdog::appMainHandler_ = mainHandler;
    auto watchdogTask = std::bind(&Watchdog::Timer, this);
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        INI_TIMER_FIRST_SECOND, CHECK_INTERVAL_TIME);
}

void Watchdog::Stop()
{
    HILOG_DEBUG("Watchdog is stop !");
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
    return true;
}

bool Watchdog::IsStopWatchdog()
{
    return stopWatchdog_;
}

bool Watchdog::WaitForDuration(uint32_t duration)
{
    std::unique_lock<std::mutex> lock(cvMutex_);
    auto condition = [this] {
        return this->IsStopWatchdog();
    };
    if (cvWatchdog_.wait_for(lock, std::chrono::milliseconds(duration), condition)) {
        return true;
    }
    return false;
}

bool Watchdog::Timer()
{
    if (WaitForDuration(INI_TIMER_FIRST_SECOND)) {
        HILOG_DEBUG("cvWatchDog1 is stopped");
        
        return true;
    }
    while (!stopWatchdog_) {
        if (appMainHandler_ != nullptr) {
            appMainHandler_->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE);
        }
        if (WaitForDuration(CHECK_INTERVAL_TIME)) {
            HILOG_DEBUG("cvWatchDog2 is stopped");
            return true;
        }
        if (!needReport_) {
            HILOG_ERROR("Watchdog timeout, wait for the handler to recover, and do not send event.");
            continue;
        }
        if (IsReportEvent()) {
            const int bufferLen = 128;
            char paramOutBuf[bufferLen] = {0};
            const char *hook_mode = "startup:";
            int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
            if (ret <= 0 || strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) != 0) {
                reportEvent();
            }
        }
    }
    return true;
}

void Watchdog::reportEvent()
{
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

    OHOS::HiviewDFX::HiSysEvent::Write(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, eventType,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, EVENT_KEY_UID, applicationInfo_->uid,
        EVENT_KEY_PID, static_cast<int32_t>(getpid()), EVENT_KEY_PACKAGE_NAME, applicationInfo_->bundleName,
        EVENT_KEY_PROCESS_NAME, applicationInfo_->process, EVENT_KEY_MESSAGE, msgContent);
    HILOG_INFO("reportEvent success, %{public}zu %{public}s", msgContent.size(), msgContent.c_str());
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
