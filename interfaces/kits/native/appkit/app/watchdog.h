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
constexpr uint32_t INI_TIMER_FIRST_SECOND = 10000;
constexpr const char* MAIN_THREAD_TIMEOUT_TASK = "MAIN_THREAD_TIMEOUT_TASK";
class Watchdog {
public:
    Watchdog();
    ~Watchdog();

    /**
     *
     * @brief Init the Watchdog.
     *
     * @param mainHandler The handler of main thread.
     */
    void Init(const std::shared_ptr<EventHandler> mainHandler);

    /**
     *
     * @brief Stop the mainthread function of watchdog.
     *
     */
    void Stop();

    /**
     *
     * @brief Set the info of application.
     *
     * @param applicationInfo The info of application
     */
    void SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &applicationInfo);

    /**
     *
     * @brief Set the state of main thread.
     *
     * @param appMainThreadState The state of main thread.
     */
    void SetAppMainThreadState(const bool appMainThreadState);

    /**
     *
     * @brief Set whether app is in the background or not
     *
     */
    void SetBackgroundStatus(const bool isInBackground);

    /**
     *
     * @brief Allow report the main thread timeout event.
     *
     */
    void AllowReportEvent();

    /**
     *
     * @brief Get StopWatchdog flag.
     *
     */
    bool IsStopWatchdog();

    /**
     *
     * @brief Check and reset the main thread state.
     *
     */
    bool IsReportEvent();

private:
    void Timer();
    void ReportEvent();

    std::atomic_bool appMainThreadIsAlive_ = false;
    std::atomic_bool stopWatchdog_ = false;
    std::atomic_bool needReport_ = true;
    std::atomic_bool isSixSecondEvent_ = false;
    std::atomic_bool isInBackground_ = false;
    std::shared_ptr<ApplicationInfo> applicationInfo_ = nullptr;
    std::mutex cvMutex_;
    std::condition_variable cvWatchdog_;
    static std::shared_ptr<EventHandler> appMainHandler_;
    int64_t lastWatchTime_ = 0;
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
