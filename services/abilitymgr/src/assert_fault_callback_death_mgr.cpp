/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "assert_fault_callback_death_mgr.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

AssertFaultCallbackDeathMgr::~AssertFaultCallbackDeathMgr()
{
    for (auto &item : assertFaultSessionDailogs_) {
        if (item.second.first == nullptr || item.second.second == nullptr) {
            HILOG_WARN("Callback is nullptr.");
            continue;
        }
        item.second.first->RemoveDeathRecipient(item.second.second);
    }

    assertFaultSessionDailogs_.clear();
}

void AssertFaultCallbackDeathMgr::AddAssertFaultCallback(sptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("Called.");
    if (remote == nullptr) {
        HILOG_ERROR("Params remote is nullptr.");
        return;
    }

    std::weak_ptr<AssertFaultCallbackDeathMgr> weakThis = shared_from_this();
    sptr<AssertFaultRemoteDeathRecipient> deathRecipient =
        new (std::nothrow) AssertFaultRemoteDeathRecipient([weakThis] (const wptr<IRemoteObject> &remote) {
            auto ams = weakThis.lock();
            if (ams == nullptr) {
                HILOG_ERROR("Invalid manager instance.");
                return;
            }
            ams->RemoveAssertFaultCallback(remote);
        });

    remote->AddDeathRecipient(deathRecipient);
    uint64_t assertFaultSessionId = reinterpret_cast<uint64_t>(remote.GetRefPtr());
    std::unique_lock<std::mutex> lock(assertFaultSessionMutex_);
    assertFaultSessionDailogs_[assertFaultSessionId] =
        std::pair<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>(remote, deathRecipient);
}

void AssertFaultCallbackDeathMgr::RemoveAssertFaultCallback(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("Called.");
    auto callback = remote.promote();
    if (callback == nullptr) {
        HILOG_ERROR("Invalid dead remote object.");
        return;
    }

    uint64_t assertFaultSessionId = reinterpret_cast<uint64_t>(callback.GetRefPtr());
    std::unique_lock<std::mutex> lock(assertFaultSessionMutex_);
    auto iter = assertFaultSessionDailogs_.find(assertFaultSessionId);
    if (iter == assertFaultSessionDailogs_.end()) {
        HILOG_ERROR("Find assert fault session id failed.");
        return;
    }

    if (iter->second.first != nullptr && iter->second.second != nullptr) {
        iter->second.first->RemoveDeathRecipient(iter->second.second);
    }

    assertFaultSessionDailogs_.erase(iter);
}

void AssertFaultCallbackDeathMgr::CallAssertFaultCallback(uint64_t assertFaultSessionId, AAFwk::UserStatus status)
{
    HILOG_DEBUG("Called.");
    std::unique_lock<std::mutex> lock(assertFaultSessionMutex_);
    auto iter = assertFaultSessionDailogs_.find(assertFaultSessionId);
    if (iter == assertFaultSessionDailogs_.end()) {
        HILOG_ERROR("Not find assert fault session by id.");
        return;
    }

    sptr<AssertFaultProxy> callback = iface_cast<AssertFaultProxy>(iter->second.first);
    if (callback == nullptr) {
        HILOG_ERROR("Convert assert fault proxy failed, callback is nullptr.");
        return;
    }

    callback->NotifyDebugAssertResult(status);
}
} // namespace AbilityRuntime
} // namespace OHOS
