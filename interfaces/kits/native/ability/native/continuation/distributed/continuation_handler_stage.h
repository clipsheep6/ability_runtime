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

#ifndef OHOS_ABILITY_RUNTIME_CONTINUATION_HANDLER_STAGE_H
#define OHOS_ABILITY_RUNTIME_CONTINUATION_HANDLER_STAGE_H

#include <string>

#include "continuation_manager_stage.h"
#include "distribute_schedule_handler_interface.h"
#include "iremote_broker.h"
#include "reverse_continuation_scheduler_primary_handler_interface.h"
#include "reverse_continuation_scheduler_primary_interface.h"
#include "reverse_continuation_scheduler_primary_proxy.h"
#include "reverse_continuation_scheduler_primary_stub.h"
#include "reverse_continuation_scheduler_recipient.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "reverse_continuation_scheduler_replica_interface.h"
#include "reverse_continuation_scheduler_replica_proxy.h"
#include "ui_ability.h"
#include "want.h"
#include "want_params.h"

using Want = OHOS::AAFwk::Want;
namespace OHOS {
namespace AppExecFwk {
class ContinuationHandlerStage : public IDistributeScheduleHandler,
                                 public IReverseContinuationSchedulerPrimaryHandler,
                                 public IReverseContinuationSchedulerReplicaHandler {
public:
    /**
     * @brief constructed function
     */
    ContinuationHandlerStage(const std::weak_ptr<ContinuationManagerStage> &continuationManager,
        const std::weak_ptr<AbilityRuntime::UIAbility> &uiAbility);
    virtual ~ContinuationHandlerStage() = default;

    /**
     * @brief Handle continuation from DMS.
     * @param token The token
     * @param deviceId The device identifier. This value doesn't matter.
     * @return zero if success.
     */
    bool HandleStartContinuation(const sptr<IRemoteObject> &token, const std::string &deviceId) override;

    /**
     * @brief Receive a scheduler which could handle reverse continuation.
     *        Remote side will pass an scheduler before continuation completed if this continuation is
     *        reversible. This method will not be called if this continuation is not reversible.
     * @param remoteReplica A scheduler to handle reverse continuation request.
     */
    void HandleReceiveRemoteScheduler(const sptr<IRemoteObject> &remoteReplica) override;

    /**
     * @brief Called by DMS when the migrate device finished.
     * @param result Zero indicate the continuation is success, otherwise integer less than zero.
     */
    void HandleCompleteContinuation(int result) override;

    /**
     * @brief Pass the primary reverse continuation scheduler object
     * @param Primary The remote object repersenting the reverse continuation sch
     */
    void PassPrimary(const sptr<IRemoteObject> &Primary) override;

    /**
     * @brief Indicate a reverse continuation
     * @return true if the reverse continuation was successful initiated,otherwise false
     */
    bool ReverseContinuation() override;

    /**
     * @brief Notify the result of a reverse continuation
     * @param reverseResult The result of the reverse continuation
     */
    void NotifyReverseResult(int reverseResult) override;

    /**
     * @brief Handle the continuation back request from the primary
     * @param want The want object representing the continuation back request
     * @return true indicating successful handling of the continuation back request
     */
    bool ContinuationBack(const Want &want) override;

    /**
     * @brief Notify Replica Terminated
     */
    void NotifyReplicaTerminated() override;

    /**
     * @brief Notify the primary that the replica has terminated
     */
    void NotifyTerminationToPrimary();

    /**
     * @brief Set Reversible
     * @param reversible Indicates the boolen
     */
    void SetReversible(bool reversible);

    /**
     * @brief Set Ability Inforamtion
     * @param abilityInfo Indicates the ability Inforamtion
     */
    void SetAbilityInfo(std::shared_ptr<AbilityInfo> &abilityInfo);

    /**
     * @brief Set the Primary Stub
     * @param Primary Indicates the Primary to be set up
     */
    void SetPrimaryStub(const sptr<IRemoteObject> &Primary);

    /**
     * @brief Reverse Continue Ability
     * @return If the success returns true, the failure returns false.
     */
    bool ReverseContinueAbility();

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     * @param token Indicates the token
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to.
     * @param versionCode Target bundle version.
     */
    bool HandleStartContinuationWithStack(
        const sptr<IRemoteObject> &token, const std::string &deviceId, uint32_t versionCode);

    static const std::string ORIGINAL_DEVICE_ID;

private:
    void OnReplicaDied(const wptr<IRemoteObject> &remote);
    void ClearDeviceInfo(std::shared_ptr<AbilityInfo> &abilityInfo);
    void CleanUpAfterReverse();
    Want SetWantParams(const WantParams &wantParams);

    std::shared_ptr<AbilityInfo> abilityInfo_ = nullptr;
    std::weak_ptr<AbilityRuntime::UIAbility> ability_;
    std::weak_ptr<ContinuationManagerStage> continuationManager_;
    bool reversible_ = false;
    sptr<IReverseContinuationSchedulerReplica> remoteReplicaProxy_ = nullptr;
    sptr<IReverseContinuationSchedulerPrimary> remotePrimaryProxy_ = nullptr;
    sptr<IRemoteObject> remotePrimaryStub_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> schedulerDeathRecipient_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CONTINUATION_HANDLER_STAGE_H
