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

#ifndef OHOS_ABILITY_RUNTIME_WATCHDOG_H
#define OHOS_ABILITY_RUNTIME_WATCHDOG_H

#include <string>
#include <mutex>
#include <thread>
#include "event_handler.h"
#include "inner_event.h"
#include "application_impl.h"

namespace OHOS {
namespace AppExecFwk {
constexpr uint32_t MAIN_THREAD_IS_ALIVE = 0;
constexpr uint32_t MAIN_THREAD_TIMEOUT_TIME = 6000;
constexpr uint32_t INI_TIMER_FIRST_SECOND = 10000;
constexpr uint32_t INI_TIMER_SECOND = 3000;
constexpr const char* MAIN_THREAD_IS_ALIVE_MSG = "MAIN_THREAD_IS_ALIVE";
class WatchDog : public EventHandler {
public:
    WatchDog(const std::shared_ptr<EventRunner> &runner);
    virtual ~WatchDog() = default;

    /**
     *
     * @brief Init the Watchdog.
     *
     * @param mainHandler The handler of main thread.
     * @param watchDogHandler The handler of watchdog thread.
     */
    void Init(const std::shared_ptr<EventHandler> &mainHandler, const std::shared_ptr<WatchDog> &watchDogHandler);

    /**
     *
     * @brief Stop the mainthread function of watchdog.
     *
     */
    void Stop();

    /**
     *
     * @brief Stop the mainthread function of watchdog.
     *
     */
    void SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &applicationInfo);

    /**
     *
     * @brief Get StopWatchDog flag.
     *
     */
    bool IsStopWatchDog();

    /**
     *
     * @brief Get the eventHandler of watchdog thread.
     *
     * @return Returns the eventHandler of watchdog thread.
     */
    static std::shared_ptr<WatchDog> GetCurrentHandler();

    /**
     *
     * @brief Get the App main thread state.
     *
     * @return Returns the App main thread state.
     */
    static bool GetAppMainThreadState();

protected:
    /**
     *
     * @brief Process the event.
     *
     * @param event the event want to be processed.
     *
     */
    void ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event) override;

private:
    bool Timer();
    bool WaitForDuration(uint32_t duration);

    std::atomic_bool stopWatchDog_ = false;
    std::atomic<bool> timeOut_ = false;
    std::shared_ptr<ApplicationInfo> applicationInfo_ = nullptr;
    std::shared_ptr<std::thread> watchDogThread_ = nullptr;
    std::shared_ptr<EventRunner> watchDogRunner_ = nullptr;
    std::mutex cvMutex_;
    std::condition_variable cvWatchDog_;
    static bool appMainThreadIsAlive_;
    static std::shared_ptr<EventHandler> appMainHandler_;
    static std::shared_ptr<WatchDog> currentHandler_;
};

class MainHandlerDumper : public Dumper {
public:
    virtual void Dump(const std::string &message) override;
    virtual std::string GetTag() override;
    std::string GetDumpInfo();
private:
    std::string dumpInfo;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WATCHDOG_H
