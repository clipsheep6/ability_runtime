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

#ifndef OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_DEATH_MGR_H
#define OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_DEATH_MGR_H
#include <memory>
#include <mutex>
#include <unordered_map>

#include "assert_fault_proxy.h"
#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultCallbackDeathMgr : public DelayedSingleton<AssertFaultCallbackDeathMgr>,
    public std::enable_shared_from_this<AssertFaultCallbackDeathMgr> {
    DISALLOW_COPY_AND_MOVE(AssertFaultCallbackDeathMgr);
public:
    using CallbackTask = std::function<void(const std::string &)>;
    AssertFaultCallbackDeathMgr() = default;
    virtual ~AssertFaultCallbackDeathMgr();
    struct DeathItem {
        int32_t pid_;
        sptr<IRemoteObject> iremote_;
        sptr<IRemoteObject::DeathRecipient> deathObj_;
        CallbackTask callbackTask_;
    };

    void AddAssertFaultCallback(sptr<IRemoteObject> &remote, CallbackTask callback);
    void RemoveAssertFaultCallback(const wptr<IRemoteObject> &remote, bool isCallbackDeath = false);
    void CallAssertFaultCallback(
        uint64_t assertFaultSessionId, AAFwk::UserStatus status = AAFwk::UserStatus::ASSERT_TERMINATE);

private:
    using DeathMap = std::unordered_map<uint64_t, DeathItem>;
    std::mutex assertFaultSessionMutex_;
    DeathMap assertFaultSessionDialogs_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_DEATH_MGR_H
