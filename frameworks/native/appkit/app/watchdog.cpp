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

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
}
std::shared_ptr<EventHandler> WatchDog::appMainHandler_ = nullptr;
std::shared_ptr<WatchDog> WatchDog::currentHandler_ = nullptr;
bool WatchDog::appMainThreadIsAlive_ = false;

WatchDog::WatchDog(const std::shared_ptr<EventRunner> &runner)
    : AppExecFwk::EventHandler(runner), watchDogRunner_(runner)
{}

/**
 *
 * @brief Process the event.
 *
 * @param event the event want to be processed.
 *
 */
void WatchDog::ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event)
{
    auto eventId = event->GetInnerEventId();
    if (eventId == MAIN_THREAD_IS_ALIVE) {
        WatchDog::appMainThreadIsAlive_ = true;
        if (currentHandler_ != nullptr) {
            currentHandler_->RemoveTask(MAIN_THREAD_IS_ALIVE_MSG);
        }
        timeOut_.store(false);
        isSixSecondEvent_.store(false);
    }
}

void WatchDog::Init(const std::shared_ptr<EventHandler> &mainHandler, const std::shared_ptr<WatchDog> &watchDogHandler)
{
    WatchDog::appMainHandler_ = mainHandler;
    WatchDog::currentHandler_ = watchDogHandler;
    if (watchDogThread_ == nullptr) {
        watchDogThread_ = std::make_shared<std::thread>(&WatchDog::Timer, this);
        HILOG_INFO("Watchdog is running!");
    }
}

void WatchDog::Stop()
{
    HILOG_INFO("Watchdog is stop !");
    stopWatchDog_.store(true);
    cvWatchDog_.notify_all();

    if (watchDogThread_ != nullptr && watchDogThread_->joinable()) {
        watchDogThread_->join();
        watchDogThread_ = nullptr;
    }
    if (watchDogRunner_) {
        watchDogRunner_.reset();
        watchDogRunner_ = nullptr;
    }
    if (currentHandler_) {
        currentHandler_.reset();
        currentHandler_ = nullptr;
    }
    if (appMainHandler_) {
        appMainHandler_.reset();
        appMainHandler_ = nullptr;
    }
}

void WatchDog::SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &applicationInfo)
{
    applicationInfo_ = applicationInfo;
}

std::shared_ptr<WatchDog> WatchDog::GetCurrentHandler()
{
    return currentHandler_;
}

bool WatchDog::GetAppMainThreadState()
{
    return appMainThreadIsAlive_;
}

bool WatchDog::IsStopWatchDog()
{
    return stopWatchDog_;
}

bool WatchDog::WaitForDuration(uint32_t duration)
{
    std::unique_lock<std::mutex> lck(cvMutex_);
    auto condition = [wp = WatchDog::weak_from_this()] {
        auto sp = wp.lock();
        if (!sp) {
            return true;
        }
        auto self = std::static_pointer_cast<WatchDog>(sp);
        if (!self) {
            return true;
        }
        return self->IsStopWatchDog();
    };
    if (cvWatchDog_.wait_for(lck, std::chrono::milliseconds(duration), condition)) {
        return true;
    }
    return false;
}

bool WatchDog::Timer()
{
    if (WaitForDuration(INI_TIMER_FIRST_SECOND)) {
        HILOG_INFO("cvWatchDog1 is stopped");
        return true;
    }
    while (!stopWatchDog_) {
        if (WaitForDuration(INI_TIMER_SECOND)) {
            HILOG_INFO("cvWatchDog2 is stopped");
            return true;
        }
        auto timeoutTask = [&]() {
            timeOut_.store(true);
            appMainThreadIsAlive_ = false;
            std::string eventType = isSixSecondEvent_ ? "THREAD_BLOCK_6S" : "THREAD_BLOCK_3S";
            std::string msgContent = "App main thread is not response!";
            MainHandlerDumper handlerDumper;
            appMainHandler_->Dump(handlerDumper);
            msgContent += handlerDumper.GetDumpInfo();
            if (applicationInfo_ != nullptr) {
                isSixSecondEvent_.store(true);
                OHOS::HiviewDFX::HiSysEvent::Write(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, eventType,
                    OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                    EVENT_KEY_UID, applicationInfo_->uid,
                    EVENT_KEY_PID, static_cast<int32_t>(getpid()),
                    EVENT_KEY_PACKAGE_NAME, applicationInfo_->bundleName,
                    EVENT_KEY_PROCESS_NAME, applicationInfo_->process,
                    EVENT_KEY_MESSAGE, msgContent);
            }
            HILOG_INFO("%{public}zu %{public}s", msgContent.size(), msgContent.c_str());
        };
        if (timeOut_) {
            HILOG_ERROR("Watchdog timeout, wait for the handler to recover, and do not send event.");
            continue;
        }
        if (currentHandler_ != nullptr) {
            // check libc.hook_mode
            const int bufferLen = 128;
            char paramOutBuf[bufferLen] = {0};
            const char *hook_mode = "startup:";
            int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
            if (ret <= 0 || strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) != 0) {
                currentHandler_->PostTask(timeoutTask, MAIN_THREAD_IS_ALIVE_MSG, MAIN_THREAD_TIMEOUT_TIME);
            }
        }
        if (appMainHandler_ != nullptr) {
            appMainHandler_->SendEvent(MAIN_THREAD_IS_ALIVE);
        }
    }
    return true;
}

void MainHandlerDumper::Dump(const std::string &message)
{
    HILOG_INFO("message is %{public}s", message.c_str());
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
