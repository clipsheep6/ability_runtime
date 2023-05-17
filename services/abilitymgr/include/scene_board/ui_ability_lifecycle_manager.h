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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H

#include <queue>

#include "ability_record.h"
#include "session_info.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityLifecycleManager : public std::enable_shared_from_this<UIAbilityLifecycleManager> {
public:
    UIAbilityLifecycleManager() = default;
    virtual ~UIAbilityLifecycleManager() = default;

    /**
     * StartUIAbility with request.
     *
     * @param abilityRequest the request of the service ability to start.
     * @param sessionInfo the info of scene session
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo);

    /**
     * @brief execute after the ability schedule the lifecycle
     *
     * @param token the ability token
     * @param state the ability state
     * @param saveData the saved data
     * @return execute error code
     */
    int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData);

    /**
     * attach ability thread ipc object.
     *
     * @param scheduler ability thread ipc object.
     * @param token the token of ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    /**
     * app manager service call this interface after ability request done.
     *
     * @param token ability's token.
     * @param state the state of ability lift cycle.
     */
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) const;

    /**
     * Check whether the UIAbility is alive.
     *
     * @param token ability's token.
     * @return Returns true on alive.
     */
    bool IsContainsAbility(const sptr<IRemoteObject> &token) const;

    /**
     * MinimizeUIAbility, minimize the special ability by scb.
     *
     * @param abilityRecord, the ability to minimize.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * GetServiceRecordBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIAbilityRecordBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    /**
     * CloseUIAbility, close the special ability by scb.
     *
     * @param abilityRecord, the ability to close.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CloseUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
private:
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const;
    void UpdateAbilityRecordLaunchReason(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void EraseAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);
    void CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleForegroundTimeoutOrFailed(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);
    void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId);
    void DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    mutable std::recursive_mutex sessionLock_;
    std::map<uint64_t, std::shared_ptr<AbilityRecord>> sessionAbilityMap_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityList_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H