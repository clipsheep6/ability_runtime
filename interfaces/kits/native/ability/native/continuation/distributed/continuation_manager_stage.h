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

#ifndef OHOS_ABILITY_RUNTIME_CONTINUATION_MANAGER_STAGE_H
#define OHOS_ABILITY_RUNTIME_CONTINUATION_MANAGER_STAGE_H

#include <memory>
#include <mutex>

#include "ability_info.h"
#include "continuation_state.h"
#include "event_handler.h"
#include "iremote_object.h"
#include "want.h"

using OHOS::AAFwk::WantParams;
namespace OHOS {
namespace AbilityRuntime {
class UIAbility;
}
namespace AppExecFwk {
class ContinuationHandlerStage;
class IAbilityContinuation;
class ContinuationManagerStage : public std::enable_shared_from_this<ContinuationManagerStage> {
public:
    /**
     * @brief constructed function
     */
    ContinuationManagerStage();
    virtual ~ContinuationManagerStage() = default;

    /**
     * @brief Init the ContinuationManagerStage
     * @param ability Indicates the ability to Init
     * @param continueToken Indicates the continueToken to Init
     * @param abilityInfo Indicate the Ability information
     * @param continuationHandler Indicate the continuation Handler instance
     */
    bool Init(const std::shared_ptr<AbilityRuntime::UIAbility> &ability, const sptr<IRemoteObject> &continueToken,
        const std::shared_ptr<AbilityInfo> &abilityInfo,
        const std::shared_ptr<ContinuationHandlerStage> &continuationHandler);

    /**
     * @brief Obtains the migration state of this ability.
     * @return Returns the migration state.
     */
    ContinuationState GetContinuationState();

    /**
     * @brief Obtains the ID of the source device from which this ability is migrated.
     * @return Returns the source device ID.
     */
    std::string GetOriginalDeviceId();

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to.
     * @param versionCode Target bundle version.
     */
    void ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode);

    /**
     * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
     * ability slices must implement the IAbilityContinuation interface.
     * @param reversible Parameter of Boolean type, passed in true or false
     * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
     * is null, this method has the same effect as continueAbility().
     */
    void ContinueAbility(bool reversible, const std::string &deviceId);

    /**
     * @brief Reverse Continue Ability
     * @return If the success returns true, the failure returns false.
     */
    bool ReverseContinueAbility();

    /**
     * @brief Start Continuation Ability
     * @return If the success returns true, the failure returns false.
     */
    bool StartContinuation();

    /**
     * @brief Prepare user data of local Ability.
     * @param wantParams Indicates the user data to be saved.
     * @return If the ability is willing to continue and data saved successfully, it returns 0;
     * otherwise, it returns errcode.
     */
    int32_t OnContinue(WantParams &wantParams);

    /**
     * @brief OnStart And Save Data
     * @param wantParams Indicates the user data.
     * @return If the success code is returned successfully, otherwise the failure code is returned.
     */
    int32_t OnStartAndSaveData(WantParams &wantParams);

    /**
     * @brief Determine whether to continue the continuous management phase of the page stack
     * @param wantParams Indicates the user data.
     * @return If you want to continue the continuous management phase of the
     * page stack, return true, otherwise return false
     */
    bool IsContinuePageStack(const WantParams &wantParams);

    /**
     * @brief Handle the continuation request and retrieve content information if needed
     * @param wantParams Indicates the user data.
     * @return An error code indicating the success or failure of the operation
     */
    int32_t OnContinueAndGetContent(WantParams &wantParams);

    /**
     * @brief Save Data for continuation
     * @param saveData Indicates WantParams data to be saved
     * @return If the success returns true, the failure returns false.
     */
    bool SaveData(WantParams &saveData);

    /**
     * @brief Restore data for continuation
     * @param restoreData The WantParams containing the data
     * @param reversible reversible A flag indicating whether the continuation is reversible
     * @param originalDeviceId the original device ID
     * @return If the success returns true, othrewise returns false.
     */
    bool RestoreData(const WantParams &restoreData, bool reversible, const std::string &originalDeviceId);

    /**
     * @brief Notifies the completion of continuation
     * @param originDeviceId The ID of the originating device
     * @param sessionId The session ID associated with the continuation
     * @param success A flag indicating the success of the continuation
     * @param reverseScheduler A remote object for reverse scheduling
     */
    void NotifyCompleteContinuation(
        const std::string &originDeviceId, int sessionId, bool success,
        [[maybe_unused]] const sptr<IRemoteObject> &reverseScheduler);

    /**
     * @brief complete the continuation process
     * @param result The result of the continuation process
     */
    void CompleteContinuation(int result);

    /**
     * @brief Restore from a remote continuation
     * @param The WantParams containing the data for restoration
     * @return true if restoration from was successful,otherwise false
     */
    bool RestoreFromRemote(const WantParams &restoreData);

    /**
     * @brief Notify that remote continuation has terminated
     * @return true indicating successful notification of remote termination,otherwise false
     */
    bool NotifyRemoteTerminated();

    /**
     * @brief Change the process state to Initial state and remove timeout task
     */
    void ChangeProcessStateToInit();

    enum OnContinueResult {
        AGREE = 0,
        REJECT = 1,
        MISMATCH = 2
    };
private:
    enum ProgressState { INITIAL, WAITING_SCHEDULE, IN_PROGRESS };
    bool CheckContinuationIllegal();
    bool HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode);
    bool HandleContinueAbility(bool reversible, const std::string &deviceId);
    ProgressState GetProcessState();
    void ChangeProcessState(const ProgressState &newState);
    void RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState);
    void InitMainHandlerIfNeed();
    bool CheckAbilityToken();
    void CheckDmsInterfaceResult(int result, const std::string &interfaceName);
    bool DoScheduleStartContinuation();
    bool DoScheduleSaveData(WantParams &saveData);
    bool DoScheduleRestoreData(const WantParams &restoreData);
    bool DoRestoreFromRemote(const WantParams &restoreData);
#ifdef SUPPORT_GRAPHICS
    bool GetContentInfo(WantParams &wantParams);
#endif
    sptr<IRemoteObject> continueToken_ = nullptr;
    std::weak_ptr<AbilityRuntime::UIAbility> ability_;
    std::weak_ptr<AbilityInfo> abilityInfo_;
    ProgressState progressState_ = ProgressState::INITIAL;
    bool reversible_ = false;
    ContinuationState continuationState_ = ContinuationState::LOCAL_RUNNING;
    std::string originalDeviceId_;
    std::weak_ptr<ContinuationHandlerStage> continuationHandler_;
    std::shared_ptr<EventHandler> mainHandler_ = nullptr;
    std::mutex lock_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CONTINUATION_MANAGER_STAGE_H
