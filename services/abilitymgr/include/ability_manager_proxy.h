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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_PROXY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_PROXY_H

#include "ability_manager_interface.h"
#include "auto_startup_info.h"
#include "hilog_wrapper.h"
#include "iremote_proxy.h"
#include "mission_info.h"

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
/**
 * @class AbilityManagerProxy
 * AbilityManagerProxy.
 */
class AbilityManagerProxy : public IRemoteProxy<IAbilityManager> {
public:
    explicit AbilityManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityManager>(impl)
    {}

    virtual ~AbilityManagerProxy()
    {}

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param requestCode, Ability request code.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param requestCode the resultCode of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbility by insight intent, send want to ability manager service.
     *
     * @param want Ability want.
     * @param callerToken caller ability token.
     * @param intentId insight intent id.
     * @param userId userId of target ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t StartAbilityByInsightIntent(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint64_t intentId,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param callerToken caller ability token.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start options.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param callerToken current caller ability token.
     * @param asCallerSourceToken source caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE,
        bool isSendDialogResult = false) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken current caller ability token.
     * @param asCallerSourceToken source caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Start ui session ability with extension session info, send session info to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param sessionInfo the information of UIExtensionContentSession.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Start ui session ability with extension session info, send session info to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken, caller ability token.
     * @param sessionInfo the information of UIExtensionContentSession.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Start extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED) override;

    /**
     * Create UIExtension with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int RequestModalUIExtension(const Want &want) override;

    /**
     * Start ui extension ability with extension session info, send extension session info to ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIExtensionAbility(
        const sptr<SessionInfo> &extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Start ui ability with want, send want to ability manager service.
     *
     * @param sessionInfo the session info of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo) override;

    /**
     * Stop extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to stop.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be stopped.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopExtensionAbility(
        const Want& want,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED) override;
    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) override;

    /**
     * TerminateUIExtensionAbility, terminate the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int resultCode,
        const Want *resultWant) override;

    /**
     * CloseUIAbilityBySCB, close the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo) override;

    /**
     * SendResultToAbility with want, return want from ability manager service.(Only used for dms)
     *
     * @param requestCode, request code.
     * @param resultCode, resultCode to return.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendResultToAbility(int32_t requestCode, int32_t resultCode, Want& resultWant) override;

    /**
     * MoveAbilityToBackground.
     *
     * @param token, the token of the ability to move.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MoveAbilityToBackground(const sptr<IRemoteObject> &token) override;

    /**
     * CloseAbility, close the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) override;

    /**
     * MinimizeAbility, minimize the special ability.
     *
     * @param token, ability token.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) override;

    /**
     * MinimizeUIExtensionAbility, minimize the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        bool fromUser = false) override;

    /**
     * MinimizeUIAbilityBySCB, minimize the special ability by scb.
     *
     * @param sessionInfo the session info of the ability to minimize.
     * @param fromUser, Whether form user.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser = false) override;

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    virtual int ConnectAbilityCommon(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isQueryExtensionOnly = false) override;

    virtual int ConnectUIExtensionAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr) override;

    /**
     * DisconnectAbility, connect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(const sptr<IAbilityConnection> &connect) override;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param isKill, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool isKill, const sptr<IRemoteObject> &callerToken) override;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityScheduler, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    virtual int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) override;

    /**
     * AttachAbilityThread, ability call this interface after loaded.
     *
     * @param scheduler,.the interface handler of kit ability.
     * @param token,.ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AttachAbilityThread(
        const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) override;

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token,.ability's token.
     * @param state,.the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override;

    /**
     * ScheduleConnectAbilityDone, service ability call this interface while session was connected.
     *
     * @param token,.service ability's token.
     * @param remoteObject,.the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) override;

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override;

    /**
     * ScheduleCommandAbilityDone, service ability call this interface while session was commanded.
     *
     * @param token,.service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override;

    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd) override;

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) override;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID) override;

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @param token ability's token.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) override;

    /**
     * Get top ability.
     *
     * @param isNeedLocalDeviceId is need local device id.
     * @return Returns front desk focus ability elementName.
     */
    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true) override;

    /**
     * Get element name by token.
     *
     * @param token ability's token.
     * @param isNeedLocalDeviceId is need local device id.
     * @return Returns front desk focus ability elementName by token.
     */
    virtual AppExecFwk::ElementName GetElementNameByToken(sptr<IRemoteObject> token,
        bool isNeedLocalDeviceId = true) override;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName) override;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * force timeout ability.
     *
     * @param abilityName.
     * @param state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override;
    #endif

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return
     */
    virtual int ClearUpApplicationData(const std::string &bundleName,
        const int32_t userId = -1) override;

    /**
     * Uninstall app
     *
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName, int32_t uid) override;

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) override;

    virtual int SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo) override;

    virtual void CancelWantSender(const sptr<IWantSender> &sender) override;

    virtual int GetPendingWantUid(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) override;

    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantCode(const sptr<IWantSender> &target) override;

    virtual int GetPendingWantType(const sptr<IWantSender> &target) override;

    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    virtual void UnregisterCancelListener(
        const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) override;

    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override;

    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override;

    virtual int GetAppMemorySize() override;

    virtual bool IsRamConstrainedDevice() override;
    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        const std::string &bundleName, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) override;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override;

    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) override;

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    virtual int LockMissionForCleanup(int32_t missionId) override;

    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    virtual void SetLockedState(int32_t sessionId, bool lockedState) override;

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfos) override;

    virtual int CleanMission(int32_t missionId) override;

    virtual int CleanAllMissions() override;

    virtual int MoveMissionToFront(int32_t missionId) override;

    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override;

    virtual int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override;

    virtual int MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result) override;

    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param accountId Indicates the account to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE) override;

    /**
     * CallRequestDone, after invoke callRequest, ability will call this interface to return callee.
     *
     * @param token, ability's token.
     * @param callStub, ability's callee.
     */
    void CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub) override;

    /**
     * Release the call between Ability, disconnect session with common ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param element, the element of target service.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ReleaseCall(
        const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) override;

    virtual int StartUser(int userId) override;

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override;

    virtual int LogoutUser(int32_t userId) override;

    virtual int SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state) override;

#ifdef SUPPORT_GRAPHICS
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override;

    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler) override;

    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override;

    virtual int PrepareTerminateAbility(
        const sptr<IRemoteObject> &token, sptr<IPrepareTerminateCallback> &callback) override;

    virtual int GetDialogSessionInfo(const std::string dialogSessionId, sptr<DialogSessionInfo> &info) override;

    virtual int SendDialogResult(const Want &want, const std::string dialogSessionId, bool isAllow) override;
#endif

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override;

    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override;

    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override;

    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    virtual int RegisterOnListener(const std::string &type,
        const sptr<IRemoteOnListener> &listener) override;

    virtual int RegisterOffListener(const std::string &deviceId,
        const sptr<IRemoteOnListener> &listener) override;

    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override;

    virtual int SetComponentInterception(
        const sptr<AppExecFwk::IComponentInterception> &componentInterception) override;

    virtual int32_t SendResultToAbilityByToken(const Want &want, const sptr<IRemoteObject> &abilityToken,
        int32_t requestCode, int32_t resultCode, int32_t userId) override;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() override;

    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;

    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;

    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override;

    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;

     /**
     * GetTopAbility, get the token of top ability.
     *
     * @param token, the token of top ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int GetTopAbility(sptr<IRemoteObject> &token) override;

    virtual int CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused) override;

    /**
     * The delegator calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override;

    /**
     * The delegator calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override;

    /**
     * Calls this interface to move the ability to the foreground.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    /**
     * Calls this interface to move the ability to the background.
     *
     * @param token, ability's token.
     * @param flag, use for lock or unlock flag and so on.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) override;

    /**
     * Send not response process ID to ability manager service.
     *
     * @param pid The not response process ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendANRProcessID(int pid) override;

    /**
     * Get mission id by ability token.
     *
     * @param token The token of ability.
     * @return Returns -1 if do not find mission, otherwise return mission id.
     */
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override;

    /**
     * Get ability token by connect.
     *
     * @param token The token of ability.
     * @param callStub The callee object.
     */
    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAmsService() override;

    /**
     * Block ability.
     *
     * @param abilityRecordId The Ability Record Id.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAbility(int32_t abilityRecordId) override;

    /**
     * Block app service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() override;
    #endif

    /**
     * Call free install from remote.
     *
     * @param want, the want of the ability to start.
     * @param userId, Designation User ID.
     * @param requestCode, Ability request code.
     * @param callback, Callback from remote.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Add FreeInstall Observer
     *
     * @param observer the observer of ability free install start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int AddFreeInstallObserver(const sptr<AbilityRuntime::IFreeInstallObserver> &observer) override;

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken) override;

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     * @param pixelMap The snapshot.
     */
    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<Media::PixelMap> &pixelMap) override;

    virtual void EnableRecoverAbility(const sptr<IRemoteObject>& token) override;
    virtual void ScheduleRecoverAbility(const sptr<IRemoteObject> &token, int32_t reason,
        const Want *want = nullptr) override;

    /**
     * Called to verify that the MissionId is valid.
     * @param missionIds Query mission list.
     * @param results Output parameters, return results up to 20 query results.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t IsValidMissionIds(
        const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results) override;

    /**
     * Query whether the application of the specified PID and UID has been granted a certain permission
     * @param permission
     * @param pid Process id
     * @param uid
     * @return Returns ERR_OK if the current process has the permission, others on failure.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid) override;

    /**
     * Request dialog service with want, send want to ability manager service.
     *
     * @param want, the want of the dialog service to start.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken) override;

    int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) override;

    virtual int32_t AcquireShareData(
        const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData) override;
    virtual int32_t ShareDataDone(const sptr<IRemoteObject> &token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam) override;

    /**
     * Force app exit and record exit reason.
     * @param pid Process id .
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t ForceExitApp(const int32_t pid, Reason exitReason) override;

    /**
     * Record app exit reason.
     * @param exitReason The reason of app exit.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RecordAppExitReason(Reason exitReason) override;

    /**
     * Set rootSceneSession by SCB.
     *
     * @param rootSceneSession Indicates root scene session of SCB.
     */
    virtual void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession) override;

    /**
     * Call UIAbility by SCB.
     *
     * @param sessionInfo the session info of the ability to be called.
     */
    virtual void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo) override;

    /**
     * Start specified ability by SCB.
     *
     * @param want Want information.
     */
    void StartSpecifiedAbilityBySCB(const Want &want) override;

    /**
     * Notify sandbox app the result of saving file.
     * @param want Result of saving file, which contains the file's uri if success.
     * @param resultCode Indicates the action's result.
     * @param requestCode Pass the requestCode to match request.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t NotifySaveAsResult(const Want &want, int resultCode, int requestCode) override;

    /**
     * Set sessionManagerService
     * @param sessionManagerService the point of sessionManagerService.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService) override;

    /**
     * @brief Register collaborator.
     * @param type collaborator type.
     * @param impl collaborator.
     * @return 0 or else.
    */
    virtual int32_t RegisterIAbilityManagerCollaborator(
        int32_t type, const sptr<IAbilityManagerCollaborator> &impl) override;

    /**
     * @brief Unregister collaborator.
     * @param type collaborator type.
     * @return 0 or else.
    */
    virtual int32_t UnregisterIAbilityManagerCollaborator(int32_t type) override;

    /**
     * @brief Notify to move mission to backround.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t MoveMissionToBackground(int32_t missionId) override;

    /**
     * @brief Notify to terminate mission. it is not clear.
     * @param missionId missionId.
     * @return 0 or else.
    */
    virtual int32_t TerminateMission(int32_t missionId) override;

    /**
     * @brief Register auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback) override;

    /**
     * @brief Unregister auto start up callback for system api.
     * @param callback The point of JsAbilityAutoStartupCallBack.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback) override;

    /**
     * @brief Set every application auto start up state.
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t SetApplicationAutoStartup(const AutoStartupInfo &info) override;

    /**
     * @brief Cancel every application auto start up .
     * @param info The auto startup info,include bundle name, module name, ability name.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info) override;

    /**
     * @brief Query auto startup state all application.
     * @param infoList Output parameters, return auto startup info list.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList) override;

    /**
     * PrepareTerminateAbilityBySCB, prepare to terminate ability by scb.
     *
     * @param sessionInfo the session info of the ability to start.
     * @param isPrepareTerminate the result of ability onPrepareToTermiante.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isPrepareTerminate) override;
    virtual int RegisterSessionHandler(const sptr<IRemoteObject> &object) override;

    /**
     * @brief Register app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override;

    /**
     * @brief Unregister app debug listener.
     * @param listener App debug listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override;

    /**
     * @brief Attach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t AttachAppDebug(const std::string &bundleName) override;

    /**
     * @brief Detach app debug.
     * @param bundleName The application bundle name.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t DetachAppDebug(const std::string &bundleName) override;

    /**
     * @brief Execute intent.
     * @param key The key of intent executing client.
     * @param callerToken Caller ability token.
     * @param param The Intent execute param.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param) override;

    /**
     * @brief Check if ability controller can start.
     * @param want The want of ability to start.
     * @return Return true to allow ability to start, or false to reject.
     */
    virtual bool IsAbilityControllerStart(const Want &want) override;

    /**
     * @brief Called when insight intent execute finished.
     *
     * @param token ability's token.
     * @param intentId insight intent id.
     * @param result insight intent execute result.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result) override;

    /**
     * @brief Set application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override;

    /**
     * @brief Cancel application auto start up state by EDM.
     * @param info The auto startup info, include bundle name, module name, ability name.
     * @param flag Indicate whether to allow the application to change the auto start up state.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override;

    /**
     * @brief Get foreground ui abilities.
     * @param list Foreground ui abilities.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) override;

    /**
     * @brief Open file by uri.
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @return int The file descriptor.
     */
    virtual int32_t OpenFile(const Uri& uri, uint32_t flag) override;

    /**
     * @brief Update session info.
     * @param sessionInfos The vector of session info.
     */
    virtual void UpdateSessionInfoBySCB(const std::vector<SessionInfo> &sessionInfos, int32_t userId) override;
private:
    template <typename T>
    int GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos);
    bool WriteInterfaceToken(MessageParcel &data);
    // flag = true : terminate; flag = false : close
    int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant, bool flag);
    ErrCode SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data, MessageParcel &reply,
        MessageOption& option);
    int CheckUISessionParams(MessageParcel &data, const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo, int32_t userId, int requestCode);

private:
    static inline BrokerDelegator<AbilityManagerProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
