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

#include <list>
#include <map>
#include <memory>
#include <queue>
#include <unordered_map>
#include "cpp/mutex.h"

#include "ability_record.h"
#include "isession_handler_interface.h"
#include "session/host/include/zidl/session_interface.h"

namespace OHOS {
namespace AAFwk {
class SessionInfo;
struct AbilityRunningInfo;
struct MissionVaildResult;

class UIAbilityLifecycleManager : public std::enable_shared_from_this<UIAbilityLifecycleManager> {
public:
    UIAbilityLifecycleManager() = default;
    virtual ~UIAbilityLifecycleManager() = default;

    struct SpecifiedInfo {
        std::string abilityName = "";
        std::string bundleName = "";
        std::string flag = "";
    };
    struct key_compare {
        bool operator()(const SpecifiedInfo &info1, const SpecifiedInfo &info2) const
        {
            if (info1.abilityName < info2.abilityName || info1.bundleName < info2.bundleName ||
                info1.flag < info2.flag) {
                return true;
            }
            return false;
        }
    };

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
    int AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const AppExecFwk::PacMap &saveData);

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
     * @param fromUser, Whether form user.
     * @return Returns ERR_OK on success, others on failure.
     */
    int MinimizeUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser = false);

    /**
     * GetUIAbilityRecordBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIAbilityRecordBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    /**
     * CloseUIAbility, close the special ability by scb.
     *
     * @param abilityRecord, the ability to close.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    int CloseUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
        int resultCode, const Want *resultWant);

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession);

    int NotifySCBToStartUIAbility(const AbilityRequest &abilityRequest, int32_t userId);

    /**
     * @brief handle time out event
     *
     * @param msgId the msg id in ability record
     * @param abilityRecordId the id of ability record
     * @param isHalf is half
     */
    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf = false);

    /**
     * @brief handle when ability died
     *
     * @param abilityRecord the died ability
     */
    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * resolve the call ipc of ability for scheduling oncall.
     *
     * @param abilityRequest target ability request.
     */
    int ResolveLocked(const AbilityRequest &abilityRequest, int32_t userId);

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     */
    void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo);

    /**
     * OnAcceptWantResponse.
     *
     * @param want the want of the ability to start.
     * @param abilityRequest the flag of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag);

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     * @param userId Use id.
     */
    void StartSpecifiedAbilityBySCB(const Want &want, int32_t userId);

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param abilityRecord ability's record.
     * @param callStub ability's callee.
     */
    void CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord, const sptr<IRemoteObject> &callStub);

    /**
     * release the connection of this call.
     *
     * @param connect caller callback ipc.
     * @param element target ability name.
     */
    int ReleaseCallLocked(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element);

    /**
     * @brief handle when call connection died
     *
     * @param callRecord the died call connection
     */
    void OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord);

    /**
     * Get sessionId by ability token.
     *
     * @param token the ability token.
     * @return Returns sessionId on success, zero on failure.
     */
    int32_t GetSessionIdByAbilityToken(const sptr<IRemoteObject> &token);

    void GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityList);

    bool PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void SetSessionHandler(const sptr<ISessionHandler> &handler);

    /**
     * Get abilityRecord by session id.
     *
     * @param sessionId the session id.
     * @return Returns abilityRecord on success, nullptr on failure.
     */
    std::shared_ptr<AbilityRecord> GetAbilityRecordsById(int32_t sessionId) const;

    void GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityList,
        int32_t targetUserId) const;

    void OnAppStateChanged(const AppInfo &info, int32_t targetUserId);

    void UninstallApp(const std::string &bundleName, int32_t uid, int32_t targetUserId);

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm, int32_t userId) const;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    int BlockAbility(int abilityRecordId, int32_t targetUserId) const;
    #endif

private:
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token) const;
    int32_t GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest, bool &reuse, int32_t userId) const;
    int32_t GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest, bool &reuse, int32_t userId) const;
    int32_t GetReusedStandardPersistentId(const AbilityRequest &abilityRequest, bool &reuse, int32_t userId) const;
    void UpdateAbilityRecordLaunchReason(const AbilityRequest &abilityRequest,
        std::shared_ptr<AbilityRecord> &abilityRecord) const;
    void EraseAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state);
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
        AbilityState state = AbilityState::INITIAL);
    void CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void HandleForegroundFailed(const std::shared_ptr<AbilityRecord> &ability,
        AbilityState state = AbilityState::INITIAL);
    void HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void NotifySCBToHandleException(const std::shared_ptr<AbilityRecord> &ability, int32_t errorCode,
        std::string errorReason);
    void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId, bool isHalf = false);
    void DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord);
    bool IsContainsAbilityInner(const sptr<IRemoteObject> &token) const;
    void ReportEventToSuspendManager(const AppExecFwk::AbilityInfo &abilityInfo) const;
    bool CheckProperties(const std::shared_ptr<AbilityRecord> &abilityRecord, const AbilityRequest &abilityRequest,
        AppExecFwk::LaunchMode launchMode, int32_t userId) const;
    void NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest) const;

    // byCall
    int CallAbilityLocked(const AbilityRequest &abilityRequest, int32_t userId);
    sptr<SessionInfo> CreateSessionInfo(const AbilityRequest &abilityRequest) const;
    int NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo, const AbilityRequest &abilityRequest) const;
    int ResolveAbility(const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest) const;
    std::vector<std::shared_ptr<AbilityRecord>> GetAbilityRecordsByName(const AppExecFwk::ElementName &element);

    void EnqueueAbilityToFront(const AbilityRequest &abilityRequest);
    void NotifyStartSpecifiedAbility(AbilityRequest &request, const AAFwk::Want &want);
    void NotifyRestartSpecifiedAbility(AbilityRequest &request, const sptr<IRemoteObject> &token);
    int MoveAbilityToFront(const AbilityRequest &abilityRequest, const std::shared_ptr<AbilityRecord> &abilityRecord,
        std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions = nullptr);
    int SendSessionInfoToSCB(std::shared_ptr<AbilityRecord> &callerAbility, sptr<SessionInfo> &sessionInfo);
    int StartAbilityBySpecifed(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &callerAbility);
    std::shared_ptr<AbilityRecord> GetReusedSpecifiedAbility(const AAFwk::Want &want, const std::string &flag);
    void EraseSpecifiedAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void SetLastExitReason(std::shared_ptr<AbilityRecord> &abilityRecord) const;
    LastExitReason CovertAppExitReasonToLastReason(const Reason exitReason) const;
    void SetRevicerInfo(const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord) const;

    bool CheckPrepareTerminateEnable(const std::shared_ptr<AbilityRecord> &abilityRecord);

    mutable ffrt::mutex sessionLock_;
    std::unordered_map<int32_t, std::shared_ptr<AbilityRecord>> sessionAbilityMap_;
    std::unordered_map<int64_t, std::shared_ptr<AbilityRecord>> tmpAbilityMap_;
    std::list<std::shared_ptr<AbilityRecord>> terminateAbilityList_;
    sptr<Rosen::ISession> rootSceneSession_;
    std::map<SpecifiedInfo, std::shared_ptr<AbilityRecord>, key_compare> specifiedAbilityMap_;
    std::queue<AbilityRequest> abilityQueue_;
    std::queue<SpecifiedInfo> specifiedInfoQueue_;
    sptr<ISessionHandler> handler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_UI_ABILITY_LIFECYCLE_MANAGER_H