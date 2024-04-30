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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_OBSERVER_MANAGER_H
#ifdef SUPPORT_GRAPHICS

#include <map>
#include <string>

#include "ability_first_frame_state_observer_interface.h"
#include "ability_record.h"
#include "cpp/mutex.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {
using AbilityFirstFrameStateObserverMap = std::map<sptr<IAbilityFirstFrameStateObserver>, std::string>;

class AbilityFirstFrameStateObserverManager;
class AbilityFirstFrameStateObserverSet final {
public:
    AbilityFirstFrameStateObserverSet(std::weak_ptr<AbilityFirstFrameStateObserverManager> managerPtr,
        bool isNotifyAllBundles = false);
    ~AbilityFirstFrameStateObserverSet() = default;
    int32_t AddAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &bundleName);
    int32_t RemoveAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer);
    void OnAbilityFirstFrameState(const std::shared_ptr<AbilityRecord> &abilityRecord);

protected:
    void AddObserverDeathRecipient(const sptr<IRemoteBroker> &observer);
    void RemoveObserverDeathRecipient(const sptr<IRemoteBroker> &observer);

private:
    ffrt::mutex observerLock_;
    AbilityFirstFrameStateObserverMap observerMap_;
    std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>> recipientMap_;
    std::weak_ptr<AbilityFirstFrameStateObserverManager> abilityFirstFrameStateObserverManager_;
    bool isNotifyAllBundles_;
};

class AbilityFirstFrameStateObserverManager :
    public std::enable_shared_from_this<AbilityFirstFrameStateObserverManager> {
    DECLARE_DELAYED_SINGLETON(AbilityFirstFrameStateObserverManager)
public:
    void Init();
    int32_t RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &targetBundleName);
    int32_t UnregisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer);
    void HandleOnFirstFrameState(const std::shared_ptr<AbilityRecord> &abilityRecord);

private:
    std::unique_ptr<AbilityFirstFrameStateObserverSet> stateObserverSetForBundleName_;
    std::unique_ptr<AbilityFirstFrameStateObserverSet> stateObserverSetForAllBundles_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // SUPPORT_GRAPHICS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_OBSERVER_MANAGER_H
