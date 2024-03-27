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

#include "app_scheduler.h"
#include "assert_fault_callback_death_mgr.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
AssertFaultCallbackDeathMgr::~AssertFaultCallbackDeathMgr()
{
    for (auto &item : assertFaultSessionDailogs_) {
        if (item.second.iremote_ == nullptr || item.second.deathObj_ == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Callback is nullptr.");
            continue;
        }
        item.second.iremote_->RemoveDeathRecipient(item.second.deathObj_);
    }

    assertFaultSessionDailogs_.clear();
}

void AssertFaultCallbackDeathMgr::AddAssertFaultCallback(sptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Params remote is nullptr.");
        return;
    }

    std::weak_ptr<AssertFaultCallbackDeathMgr> weakThis = shared_from_this();
    sptr<AssertFaultRemoteDeathRecipient> deathRecipient =
        new (std::nothrow) AssertFaultRemoteDeathRecipient([weakThis] (const wptr<IRemoteObject> &remote) {
            auto callbackDeathMgr = weakThis.lock();
            if (callbackDeathMgr == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid manager instance.");
                return;
            }
            callbackDeathMgr->RemoveAssertFaultCallback(remote);
        });

    remote->AddDeathRecipient(deathRecipient);
    auto callerPid = IPCSkeleton::GetCallingPid();
    uint64_t assertFaultSessionId = reinterpret_cast<uint64_t>(remote.GetRefPtr());
    std::unique_lock<std::mutex> lock(assertFaultSessionMutex_);
    assertFaultSessionDailogs_[assertFaultSessionId] = {callerPid, remote, deathRecipient};
    auto appScheduler = DelayedSingleton<AAFwk::AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get app scheduler instance is nullptr.");
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(appScheduler->SetAppAssertionPauseState(callerPid, true));
}

void AssertFaultCallbackDeathMgr::RemoveAssertFaultCallback(const wptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto callback = remote.promote();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid dead remote object.");
        return;
    }

    uint64_t assertFaultSessionId = reinterpret_cast<uint64_t>(callback.GetRefPtr());
    std::unique_lock<std::mutex> lock(assertFaultSessionMutex_);
    auto iter = assertFaultSessionDailogs_.find(assertFaultSessionId);
    if (iter == assertFaultSessionDailogs_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Find assert fault session id failed.");
        return;
    }

    if (iter->second.iremote_ != nullptr && iter->second.deathObj_ != nullptr) {
        iter->second.iremote_->RemoveDeathRecipient(iter->second.deathObj_);
    }

    assertFaultSessionDailogs_.erase(iter);
}

void AssertFaultCallbackDeathMgr::CallAssertFaultCallback(uint64_t assertFaultSessionId, AAFwk::UserStatus status)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    DeathItem item;
    {
        std::unique_lock<std::mutex> lock(assertFaultSessionMutex_);
        auto iter = assertFaultSessionDailogs_.find(assertFaultSessionId);
        if (iter == assertFaultSessionDailogs_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Not find assert fault session by id.");
            return;
        }

        item = iter->second;
    }

    RemoveAssertFaultCallback(item.iremote_);
    sptr<AssertFaultProxy> callback = iface_cast<AssertFaultProxy>(item.iremote_);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Convert assert fault proxy failed, callback is nullptr.");
        return;
    }
    callback->NotifyDebugAssertResult(status);
    auto appScheduler = DelayedSingleton<AAFwk::AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get app scheduler instance is nullptr.");
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(appScheduler->SetAppAssertionPauseState(item.pid_, false));
}
} // namespace AbilityRuntime
} // namespace OHOS
