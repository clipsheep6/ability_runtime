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
constexpr int64_t MS_PER_NS = 1000000;
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

void IdleTime::InitVSyncReceiver()
{
    if (needStop_) {
        return;
    }

    if (receiver_ == nullptr) {
        auto& rsClient = Rosen::RSInterfaces::GetInstance();
        receiver_ = rsClient.CreateVSyncReceiver("ABILITY", eventHandler_);
        if (receiver_ == nullptr) {
            HILOG_ERROR("Create VSync receiver failed.");
            return;
        }
        receiver_->Init();
    }
}

void IdleTime::EventTask()
{
     if(receiver_ == nullptr) {
        HILOG_ERROR("no VSyncReceiver.");
        return;
    }

    if(callback_ == nullptr) {
        HILOG_ERROR("no callback_.");
        return;
    }
    
    int64_t period = 0;
    int64_t lastVSyncTime = 0;
    VsyncError err = receiver_->GetVSyncPeriodAndLastTimeStamp(period, lastVSyncTime);
    int64_t occurTimestamp = GetSysTimeNs();
    if(GSERROR_OK == err && period > 0 && lastVSyncTime > 0 && occurTimestamp > lastVSyncTime) {
        int64_t elapsedTime = occurTimestamp - lastVSyncTime;
        int64_t idleTime = period - elapsedTime;
        if(idleTime > 0)
            callback_(idleTime / MS_PER_NS);
    }
    PostTask();
}

void IdleTime::PostTask()
{
    if (needStop_) {
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
    eventHandler_->PostTask(task, "IdleTime:PostTask", 0, EventQueue::Priority::IDLE);
}

void IdleTime::Start()
{
    InitVSyncReceiver();
    PostTask();
}

void IdleTime::SetNeedStop(bool needStop)
{
    needStop_ = needStop;
}

bool IdleTime::GetNeedStop()
{
    return needStop_;
}

IdleNotifyStatusCallback IdleTime::GetIdleNotifyFunc()
{
    IdleNotifyStatusCallback cb = [this](bool needStop) {
        if (this->GetNeedStop() == needStop) {
            return;
        }

        this->SetNeedStop(needStop);
        if (needStop == false) {
            this->Start();
        }
    };
    return cb;
}
} // AppExecFwk
} // namespace OHOS