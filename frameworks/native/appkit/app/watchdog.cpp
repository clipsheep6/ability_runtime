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

#include <unistd.h>
#include "hisysevent.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
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

bool WatchDog::Timer()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(INI_TIMER_FIRST_SECOND));
    while (!stopWatchDog_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(INI_TIMER_SECOND));
        if (!stopWatchDog_) {
            auto timeoutTask = [&]() {
            };
            if (timeOut_) {
                HILOG_ERROR("Watchdog timeout, wait for the handler to recover, and do not send event.");
            } else {
                if (currentHandler_ != nullptr) {
                    currentHandler_->PostTask(timeoutTask, MAIN_THREAD_IS_ALIVE_MSG, MAIN_THREAD_TIMEOUT_TIME);
                }
                if (appMainHandler_ != nullptr) {
                    appMainHandler_->SendEvent(MAIN_THREAD_IS_ALIVE);
                }
            }
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
