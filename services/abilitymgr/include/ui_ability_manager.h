/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_MANAGER_H

#include <queue>

#include "ability_record.h"
#include "session_info.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityManager {
public:
    UIAbilityManager() = default;
    virtual ~UIAbilityManager() = default;

    void Init();

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @param sessionInfo, the info of scene session
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo);

    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) const;
    sptr<IRemoteObject> GetTokenBySceneSession(uint64_t persistentId);

private:
    void UpdateAbilityRecordLaunchReason(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void EnqueueWaitingAbilityToFront(const AbilityRequest &abilityRequest);
    bool IsContainsAbility(std::shared_ptr<AbilityRecord> &abilityRecord) const;
    int StartAbilityLocked(const AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo);

    mutable std::recursive_mutex sessionLock_;
    std::map<sptr<SessionInfo>, std::shared_ptr<AbilityRecord>> sessionItems_;
    std::queue<AbilityRequest> waitingAbilityQueue_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_ABILITY_MANAGER_H
