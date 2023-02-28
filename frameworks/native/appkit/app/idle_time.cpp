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

#include "idle_time.h"

#include "hilog_wrapper.h"
#include "transaction/rs_interfaces.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int64_t PERIOD = 16666666; // ns
constexpr int64_t MS_PER_NS = 1000000;
constexpr int32_t TRY_COUNT_MAX = 6;
constexpr int32_t DEVIATION_MIN = 1000; // ns
std::shared_ptr<Rosen::VSyncReceiver> receiver_ = nullptr;
}

IdleTime::IdleTime(const std::shared_ptr<EventHandler> &eventHandler, IdleTimeCallback idleTimeCallback)
{
    eventHandler_ = eventHandler;
    callback_ = idleTimeCallback;
}

int64_t IdleTime::GetSysTimeNs()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

void IdleTime::OnVSync(int64_t timestamp, void* client)
{
    // only use for 60HZ
    if (continueFailCount_ > TRY_COUNT_MAX) {
        HILOG_ERROR("Only support 60HZ.");
        return;
    }
    int64_t period = timestamp - firstVSyncTime_;
    int64_t lastPeriod = period_ == 0 ? PERIOD : period_;
    int64_t deviation = (period - lastPeriod) > 0 ? period - lastPeriod : lastPeriod - period;
    if (deviation > MS_PER_NS) { // deviation greater than 1ms
        RequestVSync();
        continueFailCount_++;
        HILOG_DEBUG("fail count is %{public}d, timestamp is %{public}" PRId64 ", period is %{public}" PRId64,
            continueFailCount_, timestamp, period);
    } else {
        if (eventHandler_ == nullptr) {
            HILOG_ERROR("eventHandler_ is nullptr.");
            return;
        }
        deviation = deviation < DEVIATION_MIN ? DEVIATION_MIN : deviation;
        int64_t timeOut = lastPeriod / deviation; // up to 16666ms : MS_PER_NS / deviation * lastPeriod / MS_PER_NS
        std::weak_ptr<IdleTime> weak(shared_from_this());
        auto task = [weak]() {
            auto idleTime = weak.lock();
            if (idleTime == nullptr) {
                HILOG_ERROR("idleTime is nullptr.");
                return;
            }
            idleTime->RequestVSync();
        };
        eventHandler_->PostTask(task, timeOut);
        if (successCount_ > TRY_COUNT_MAX) {
            period_ = (period & lastPeriod) + ((period ^ lastPeriod) >> 1); // average
        } else {
            period_ = PERIOD;
            successCount_++;
            HILOG_DEBUG("fail count is %{public}d, timestamp is %{public}" PRId64 ", period is %{public}" PRId64,
                continueFailCount_, timestamp, period);
        }
        continueFailCount_ = 0;
    }

    firstVSyncTime_ = timestamp;
}

void IdleTime::RequestVSync()
{
    if (receiver_ == nullptr) {
        auto& rsClient = Rosen::RSInterfaces::GetInstance();
        receiver_ = rsClient.CreateVSyncReceiver("ABILITY");
        if (receiver_ == nullptr) {
            HILOG_ERROR("Create VSync receiver failed.");
            return;
        }
        receiver_->Init();
    }
    std::weak_ptr<IdleTime> weak(shared_from_this());
    auto task = [weak](int64_t timestamp, void* data) {
        auto idleTime = weak.lock();
        if (idleTime == nullptr) {
            HILOG_ERROR("idleTime is nullptr.");
            return;
        }
        idleTime->OnVSync(timestamp, data);
    };
    Rosen::VSyncReceiver::FrameCallback frameCallback = {
        .userData_ = this,
        .callback_ = task,
    };
    receiver_->RequestNextVSync(frameCallback);
}

void IdleTime::EventTask()
{
    if (firstVSyncTime_ == 0 || period_ == 0) {
        PostTask();
        HILOG_ERROR("no VSync occur.");
        return;
    }
    int64_t period = period_;

    int64_t occurTimestamp = GetSysTimeNs();
    int64_t numPeriod = (occurTimestamp - firstVSyncTime_) / period;
    int64_t lastVSyncTime = numPeriod * period + firstVSyncTime_;
    int64_t elapsedTime = occurTimestamp - lastVSyncTime;
    int64_t idleTime = period - elapsedTime;
    if (callback_ != nullptr) {
        callback_(idleTime / MS_PER_NS);
        PostTask();
    }
}

void IdleTime::PostTask()
{
    if (continueFailCount_ > TRY_COUNT_MAX) {
        HILOG_ERROR("Only support 60HZ.");
        return;
    }

    if (eventHandler_ == nullptr) {
        HILOG_ERROR("eventHandler_ is nullptr.");
        return;
    }
    std::weak_ptr<IdleTime> weak(shared_from_this());
    auto task = [weak]() {
        auto idleTime = weak.lock();
        if (idleTime == nullptr) {
            HILOG_ERROR("idleTime is nullptr.");
            return;
        }
        idleTime->EventTask();
    };
    eventHandler_->PostTask(task, EventQueue::Priority::IDLE);
}

void IdleTime::Start()
{
    RequestVSync();
    PostTask();
}
} // AppExecFwk
} // namespace OHOS