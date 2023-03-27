/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H

#include <future>
#include <memory>
#include <shared_mutex>
#include <singleton.h>
#include <thread_ex.h>
#include <unordered_map>
#include <map>

#include "ability_connect_manager.h"
#include "ability_event_handler.h"
#include "ability_interceptor_executer.h"
#include "ability_manager_stub.h"
#include "app_scheduler.h"
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_observer.h"
#endif
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#include "data_ability_manager.h"
#include "free_install_manager.h"
#include "hilog_wrapper.h"
#include "iremote_object.h"
#include "mission_list_manager.h"
#include "permission_verification.h"
#include "system_ability.h"
#include "uri.h"
#include "ability_config.h"
#include "parameter.h"
#include "pending_want_manager.h"
#include "ams_configuration_parameter.h"
#include "user_controller.h"
#include "resident_process_manager.h"
#ifdef SUPPORT_GRAPHICS
#include "implicit_start_processor.h"
#include "system_dialog_scheduler.h"
#endif
#include "event_report.h"

namespace OHOS {
namespace AAFwk {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
const int32_t BASE_USER_RANGE = 200000;
const int32_t U0_USER_ID = 0;
constexpr int32_t INVALID_USER_ID = -1;
using OHOS::AppExecFwk::IAbilityController;
class PendingWantManager;
/**
 * @class AbilityManagerService
 * AbilityManagerService provides a facility for managing ability life cycle.
 */
class AbilityManagerService : public SystemAbility,
                              public AbilityManagerStub,
                              public AppStateCallback,
                              public std::enable_shared_from_this<AbilityManagerService> {
    DECLARE_DELAYED_SINGLETON(AbilityManagerService)
    DECLEAR_SYSTEM_ABILITY(AbilityManagerService)
public:
    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param requestCode, Ability request code.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * StartAbility with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability with specific start settings.
     *
     * @param want Indicates the ability to start.
     * @param abilityStartSetting Indicates the setting ability used to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param requestCode the resultCode of the ability to start.
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
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param userId Designation User ID.
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
     * @param callerToken caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
            const Want &want,
            const sptr<IRemoteObject> &callerToken,
            int32_t userId = DEFAULT_INVAL_VALUE,
            int requestCode = DEFAULT_INVAL_VALUE) override;

    /**
     * Starts a new ability using the original caller information.
     *
     * @param want the want of the ability to start.
     * @param startOptions Indicates the options used to start.
     * @param callerToken caller ability token.
     * @param userId Designation User ID.
     * @param requestCode the resultCode of the ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
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
     * Start ui extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param extensionSessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartUIExtensionAbility(
        const Want &want,
        const sptr<SessionInfo> &extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED) override;

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
    virtual int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;

    /**
     * TerminateAbility, terminate the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode, the resultCode of the ui extension ability to terminate.
     * @param resultWant, the Want of the ui extension ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        int resultCode = DEFAULT_INVAL_VALUE, const Want *resultWant = nullptr) override;

    /**
     * SendResultToAbility with want, return want from ability manager service.
     *
     * @param requestCode, request code.
     * @param resultCode, resultCode to return.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendResultToAbility(int32_t requestCode, int32_t resultCode, Want& resultWant) override;

    /**
     * TerminateAbility, terminate the special ability.
     *
     * @param callerToken, caller ability token.
     * @param requestCode, Ability request code.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int TerminateAbilityByCaller(const sptr<IRemoteObject> &callerToken, int requestCode) override;

    /**
     * CloseAbility, close the special ability.
     *
     * @param token, the token of the ability to terminate.
     * @param resultCode, the resultCode of the ability to terminate.
     * @param resultWant, the Want of the ability to return.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;

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
        int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * ContinueMission, continue ability from mission center.
     *
     * @param srcDeviceId, origin deviceId.
     * @param dstDeviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param callBack, notify result back.
     * @param wantParams, extended params.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override;

    /**
     * ContinueAbility, continue ability to ability.
     *
     * @param deviceId, target deviceId.
     * @param missionId, indicates which ability to continue.
     * @param versionCode, target bundle version.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) override;

    /**
     * StartContinuation, continue ability to remote.
     *
     * @param want, Indicates the ability to start.
     * @param abilityToken, Caller ability token.
     * @param status, continue status.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override;

    /**
     * NotifyCompleteContinuation, notify continuation complete to dms.
     * @param deviceId, source device which start a continuation.
     * @param sessionId, represent a continuaion.
     * @param isSuccess, continuation result.
     * @return
     */
    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override;

    /**
     * NotifyContinuationResult, notify continue result to ability.
     *
     * @param missionId, Caller mission id.
     * @param result, continuation result.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) override;

    /**
     * RegisterMissionListener, register remote device mission listener.
     *
     * @param deviceId, Indicates the remote device Id.
     * @param listener, listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;

    /**
     * UnRegisterMissionListener, unregister remote device mission listener.
     *
     * @param deviceId, Indicates the remote device Id.
     * @param listener, listener.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener)override;

    virtual int DisconnectAbility(const sptr<IAbilityConnection> &connect) override;

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) override;

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityToken, specifies the data ability that will be released.
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

    /**
     * GetEventHandler, get the ability manager service's handler.
     *
     * @return Returns AbilityEventHandler ptr.
     */
    std::shared_ptr<AbilityEventHandler> GetEventHandler();

    /**
     * @brief Ability hidump.
     * @param fd Indicates the fd.
     * @param args Indicates the params.
     * @return Returns the dump result.
     */
    int Dump(int fd, const std::vector<std::u16string>& args) override;

    /**
     * dump ability stack info, about userID, mission stack info,
     * mission record info and ability info.
     *
     * @param state Ability stack info.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual void DumpState(const std::string &args, std::vector<std::string> &info) override;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID) override;

    /**
     * Destroys this Service ability if the number of times it
     * has been started equals the number represented by
     * the given startId.
     *
     * @param token ability's token.
     * @param startId is incremented by 1 every time this ability is started.
     * @return Returns true if the startId matches the number of startup times
     * and this Service ability will be destroyed; returns false otherwise.
     */
    virtual int TerminateAbilityResult(const sptr<IRemoteObject> &token, int startId) override;

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE) override;

    /**
     * Kill the process immediately.
     *
     * @param bundleName.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int KillProcess(const std::string &bundleName) override;

    /**
     * ClearUpApplicationData, call ClearUpApplicationData() through proxy project,
     * clear the application data.
     *
     * @param bundleName, bundle name in Application record.
     * @return ERR_OK, return back success, others fail.
     */
    virtual int ClearUpApplicationData(const std::string &bundleName) override;

    /**
     * Uninstall app
     *
     * @param bundleName bundle name of uninstalling app.
     * @param uid uid of bundle.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int UninstallApp(const std::string &bundleName, int32_t uid) override;
    /**
     * InitMissionListManager, set the user id of mission list manager.
     *
     * @param userId, user id.
     */
    void InitMissionListManager(int userId, bool switchUser);

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken) override;

    virtual int SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo) override;

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

    virtual int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer) override;

    virtual int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer) override;

    virtual int GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos) override;

    virtual int LockMissionForCleanup(int32_t missionId) override;

    virtual int UnlockMissionForCleanup(int32_t missionId) override;

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override;

    virtual int GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;

    virtual int GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override;

    virtual int CleanMission(int32_t missionId) override;

    virtual int CleanAllMissions() override;

    virtual int MoveMissionToFront(int32_t missionId) override;

    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override;

    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override;

    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;

    virtual int StopSyncRemoteMissions(const std::string& devId) override;

    virtual int GetAppMemorySize() override;

    virtual bool IsRamConstrainedDevice() override;
    /**
     * Start Ability, connect session with common ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartAbilityByCall(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) override;

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

    /**
     * get service record by element name.
     *
     */
    std::shared_ptr<AbilityRecord> GetServiceRecordByElementName(const std::string &element);
    std::list<std::shared_ptr<ConnectionRecord>> GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    void OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord);
    void OnCallConnectDied(std::shared_ptr<CallRecord> callRecord);
    void HandleLoadTimeOut(int64_t eventId);
    void HandleActiveTimeOut(int64_t eventId);
    void HandleInactiveTimeOut(int64_t eventId);
    void HandleForegroundTimeOut(int64_t eventId);
    void HandleBackgroundTimeOut(int64_t eventId);

    int StartAbilityInner(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode,
        int callerUid = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isStartAsCaller = false);

    int StartAbilityForOptionInner(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isStartAsCaller = false);

    int CheckPermission(const std::string &bundleName, const std::string &permission);

    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag);
    void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want);

    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override;
    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override;
    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override;
    int GetProcessRunningInfosByUserId(std::vector<AppExecFwk::RunningProcessInfo> &info, int32_t userId);
    void GetAbilityRunningInfo(std::vector<AbilityRunningInfo> &info, std::shared_ptr<AbilityRecord> &abilityRecord);
    void GetExtensionRunningInfo(std::shared_ptr<AbilityRecord> &abilityRecord, const int32_t userId,
        std::vector<ExtensionRunningInfo> &info);

    /**
     * generate ability request.
     *
     */
    int GenerateAbilityRequest(
        const Want &want,
        int requestCode,
        AbilityRequest &request,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId);

    /**
     * Get mission id by target ability token.
     *
     * @param token target ability token.
     * @return the missionId of target mission.
     */
    int32_t GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token);

    /**
     * Get ability token by target mission id.
     *
     * @param missionId target missionId.
     * @return the ability token of target mission.
     */
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId);

    virtual int StartUser(int userId) override;

    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override;

    /**
     * Called when client complete dump.
     *
     * @param infos The dump info.
     * @param callerToken The caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken) override;

#ifdef SUPPORT_GRAPHICS
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override;

    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;

    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler) override;

    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override;

    sptr<IWindowManagerServiceHandler> GetWMSHandler() const;
#endif

    void ClearUserData(int32_t userId);

    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;

    virtual int32_t GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;

    /**
     * Set ability controller.
     *
     * @param abilityController, The ability controller.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetAbilityController(const sptr<IAbilityController> &abilityController, bool imAStabilityTest) override;

    /**
     * Is user a stability test.
     *
     * @return Returns true if user is a stability test.
     */
    virtual bool IsRunningInStabilityTest() override;

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
     * Set component interception.
     *
     * @param componentInterception, component interception.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SetComponentInterception(
        const sptr<AppExecFwk::IComponentInterception> &componentInterception) override;

    virtual int32_t SendResultToAbilityByToken(const Want &want, const sptr<IRemoteObject> &abilityToken,
        int32_t requestCode, int32_t resultCode, int32_t userId) override;

    bool IsAbilityControllerStart(const Want &want, const std::string &bundleName);

    bool IsAbilityControllerForeground(const std::string &bundleName);

    bool IsAbilityControllerStartById(int32_t missionId);

    bool IsComponentInterceptionStart(const Want &want, ComponentRequest &componentRequest, AbilityRequest &request);

    void NotifyHandleAbilityStateChange(const sptr<IRemoteObject> &abilityToken, int opCode);

    /**
     * Send not response process ID to ability manager service.
     * @param pid The not response process ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int SendANRProcessID(int pid) override;

    #ifdef ABILITY_COMMAND_FOR_TEST
    /**
     * Block ability manager service.
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
     * Block app manager service.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int BlockAppService() override;

    /**
     * force timeout ability.
     *
     * @param abilityName.
     * @param state.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override;
    #endif

    bool GetDataAbilityUri(const std::vector<AppExecFwk::AbilityInfo> &abilityInfos,
        const std::string &mainAbility, std::string &uri);

    virtual AppExecFwk::ElementName GetTopAbility() override;

    /**
     * AtomicServiceStatusCallback OnInstallFinished callback.
     *
     * @param resultCode FreeInstall result code.
     * @param want Want has been installed.
     * @param userId User id.
     */
    void OnInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * AtomicServiceStatusCallback OnRemoteInstallFinished callback.
     *
     * @param resultCode FreeInstall result code.
     * @param want Want has been installed.
     * @param userId User id.
     */
    void OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId);

    /**
     * FreeInstall form remote call.
     *
     * @param want Want need to install.
     * @param callback DMS callback.
     * @param userId User id.
     * @param requestCode Ability request code.
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
     * Check the uid is background task uid.
     *
     * @param uid userId.
     * @return Returns whether the uid is background task uid.
     */
    bool IsBackgroundTaskUid(const int uid);

    bool GetLocalDeviceId(std::string& localDeviceId);

    int JudgeAbilityVisibleControl(const AppExecFwk::AbilityInfo &abilityInfo, int callerUid = -1);

    /**
     * Called to update mission snapshot.
     * @param token The target ability.
     */
    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject>& token) override;
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
        const std::vector<int32_t> &missionIds, std::vector<MissionVaildResult> &results) override;

    bool GetStartUpNewRuleFlag() const;

    std::shared_ptr<AbilityRecord> GetFocusAbility();

    /**
     * Query whether the application of the specified PID and UID has been granted a certain permission
     * @param permission
     * @param pid Process id
     * @param uid
     * @return Returns ERR_OK if the current process has the permission, others on failure.
     */
    virtual int VerifyPermission(const std::string &permission, int pid, int uid) override;

    // MSG 0 - 20 represents timeout message
    static constexpr uint32_t LOAD_TIMEOUT_MSG = 0;
    static constexpr uint32_t ACTIVE_TIMEOUT_MSG = 1;
    static constexpr uint32_t INACTIVE_TIMEOUT_MSG = 2;
    static constexpr uint32_t TERMINATE_TIMEOUT_MSG = 4;
    static constexpr uint32_t FOREGROUND_TIMEOUT_MSG = 5;
    static constexpr uint32_t BACKGROUND_TIMEOUT_MSG = 6;

#ifdef SUPPORT_ASAN
    static constexpr uint32_t LOAD_TIMEOUT = 150000;            // ms
    static constexpr uint32_t ACTIVE_TIMEOUT = 75000;          // ms
    static constexpr uint32_t INACTIVE_TIMEOUT = 7500;         // ms
#else
    static constexpr uint32_t LOAD_TIMEOUT = 10000;            // ms
    static constexpr uint32_t ACTIVE_TIMEOUT = 5000;          // ms
    static constexpr uint32_t INACTIVE_TIMEOUT = 500;         // ms
#endif

    static constexpr uint32_t MIN_DUMP_ARGUMENT_NUM = 2;
    static constexpr uint32_t MAX_WAIT_SYSTEM_UI_NUM = 600;
    static constexpr uint32_t MAX_WAIT_SETTINGS_DATA_NUM = 300;

    enum DumpKey {
        KEY_DUMP_ALL = 0,
        KEY_DUMP_STACK_LIST,
        KEY_DUMP_STACK,
        KEY_DUMP_MISSION,
        KEY_DUMP_TOP_ABILITY,
        KEY_DUMP_WAIT_QUEUE,
        KEY_DUMP_SERVICE,
        KEY_DUMP_DATA,
        KEY_DUMP_FOCUS_ABILITY,
        KEY_DUMP_WINDOW_MODE,
        KEY_DUMP_MISSION_LIST,
        KEY_DUMP_MISSION_INFOS,
    };

    enum DumpsysKey {
        KEY_DUMPSYS_ALL = 0,
        KEY_DUMPSYS_MISSION_LIST,
        KEY_DUMPSYS_ABILITY,
        KEY_DUMPSYS_SERVICE,
        KEY_DUMPSYS_PENDING,
        KEY_DUMPSYS_PROCESS,
        KEY_DUMPSYS_DATA,
    };

    enum {
        ABILITY_MOVE_TO_FOREGROUND_CODE = 0,
        ABILITY_MOVE_TO_BACKGROUND_CODE,
        TERMINATE_ABILITY_CODE
    };

    friend class UserController;

protected:
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) override;
    int GetUidByBundleName(std::string bundleName);

    void OnAppStateChanged(const AppInfo &info) override;

private:
    int TerminateAbilityWithFlag(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr, bool flag = true);
    /**
     * initialization of ability manager service.
     *
     */
    bool Init();
    /**
     * initialization of u0 user.
     *
     */
    void InitU0User();
    /**
     * start highest priority ability.
     *
     */
    void StartHighestPriorityAbility(int32_t userId, bool isBoot);
    /**
     * connet bms.
     *
     */
    void ConnectBmsService();
    /**
     * get the user id.
     *
     */
    int GetUserId();
    /**
     * Determine whether it is a system APP
     *
     */
    bool IsSystemUiApp(const AppExecFwk::AbilityInfo &info) const;
    /**
     * Init parameters from the global
     *
     */
    void InitGlobalConfiguration();

    sptr<AppExecFwk::IBundleMgr> GetBundleManager();
    int StartRemoteAbility(const Want &want, int requestCode, int32_t validUserId,
        const sptr<IRemoteObject> &callerToken);
    int ConnectLocalAbility(
        const Want &want,
        const int32_t userId,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType);
    int DisconnectLocalAbility(const sptr<IAbilityConnection> &connect);
    int ConnectRemoteAbility(Want &want, const sptr<IRemoteObject> &callerToken, const sptr<IRemoteObject> &connect);
    int DisconnectRemoteAbility(const sptr<IRemoteObject> &connect);
    int PreLoadAppDataAbilities(const std::string &bundleName, const int32_t userId);
    void UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken);

    bool CheckIfOperateRemote(const Want &want);
    std::string AnonymizeDeviceId(const std::string& deviceId);
    bool VerificationToken(const sptr<IRemoteObject> &token);
    void RequestPermission(const Want *resultWant);

    bool CheckIsRemote(const std::string& deviceId);
    int GetRemoteMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos);
    int GetRemoteMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo);
    int32_t GetRemoteMissionSnapshotInfo(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& missionSnapshot);
    int StartRemoteAbilityByCall(const Want &want, const sptr<IRemoteObject> &callerToken,
        const sptr<IRemoteObject> &connect);
    int ReleaseRemoteAbility(const sptr<IRemoteObject> &connect, const AppExecFwk::ElementName &element);

    void DumpInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInner(const std::string &args, std::vector<std::string> &info);
    void DumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DataDumpStateInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionListInner(const std::string &args, std::vector<std::string> &info);
    void DumpMissionInfosInner(const std::string &args, std::vector<std::string> &info);
    void DumpFuncInit();

    bool JudgeMultiUserConcurrency(const int32_t userId);
    /**
     * dumpsys info
     *
     */
    void DumpSysFuncInit();
    void DumpSysInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysMissionListInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysAbilityInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysStateInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysPendingInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DumpSysProcess(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    void DataDumpSysStateInner(
        const std::string &args, std::vector<std::string> &info, bool isClient, bool isUserID, int userId);
    ErrCode ProcessMultiParam(std::vector<std::string>& argsStr, std::string& result);
    void ShowHelp(std::string& result);
    void ShowIllegalInfomation(std::string& result);
    int Dump(const std::vector<std::u16string>& args, std::string& result);

    void InitConnectManager(int32_t userId, bool switchUser);
    void InitDataAbilityManager(int32_t userId, bool switchUser);
    void InitPendWantManager(int32_t userId, bool switchUser);

    int32_t InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
        AppExecFwk::AbilityInfo &abilityInfo);

    // multi user
    void StartFreezingScreen();
    void StopFreezingScreen();
    void UserStarted(int32_t userId);
    void SwitchToUser(int32_t userId);
    void SwitchToUser(int32_t oldUserId, int32_t userId);
    void SwitchManagers(int32_t userId, bool switchUser = true);
    void StartUserApps(int32_t userId, bool isBoot);
    void PauseOldUser(int32_t userId);
    void PauseOldMissionListManager(int32_t userId);
    void PauseOldConnectManager(int32_t userId);
    bool IsSystemUI(const std::string &bundleName) const;

    bool VerificationAllToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler);
    std::shared_ptr<MissionListManager> GetListManagerByUserId(int32_t userId);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByUserId(int32_t userId);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByUserId(int32_t userId);
    std::shared_ptr<MissionListManager> GetListManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByToken(const sptr<IRemoteObject> &token);
    std::shared_ptr<AbilityConnectManager> GetConnectManagerBySessionInfo(const sptr<SessionInfo> &sessionInfo);
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token);
    bool JudgeSelfCalled(const std::shared_ptr<AbilityRecord> &abilityRecord);

    int32_t GetValidUserId(const int32_t userId);

    int DelegatorMoveMissionToFront(int32_t missionId);

    bool IsNeedTimeoutForTest(const std::string &abilityName, const std::string &state) const;

    void StartResidentApps();

    int VerifyAccountPermission(int32_t userId);

    bool CheckCallerEligibility(const AppExecFwk::AbilityInfo &abilityInfo, int callerUid);

    using DumpFuncType = void (AbilityManagerService::*)(const std::string &args, std::vector<std::string> &info);
    std::map<uint32_t, DumpFuncType> dumpFuncMap_;

    using DumpSysFuncType = void (AbilityManagerService::*)(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID);
    std::map<uint32_t, DumpSysFuncType> dumpsysFuncMap_;

    int CheckStaticCfgPermission(AppExecFwk::AbilityInfo &abilityInfo, bool isStartAsCaller,
        uint32_t callerTokenId);

    bool GetValidDataAbilityUri(const std::string &abilityInfoUri, std::string &adjustUri);

    int GenerateExtensionAbilityRequest(const Want &want, AbilityRequest &request,
        const sptr<IRemoteObject> &callerToken, int32_t userId);
    int CheckOptExtensionAbility(const Want &want, AbilityRequest &abilityRequest,
        int32_t validUserId, AppExecFwk::ExtensionAbilityType extensionType);

    void SubscribeBackgroundTask();

    void ReportAbilitStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo);

    void ReportEventToSuspendManager(const AppExecFwk::AbilityInfo &abilityInfo);

    void ReportAppRecoverResult(const int32_t appId, const AppExecFwk::ApplicationInfo &appInfo,
        const std::string& abilityName, const std::string& result);

    /**
     * Check if Caller is allowed to start ServiceAbility(FA) or ServiceExtension(Stage) or DataShareExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Service.
     */
    int CheckCallServicePermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start DataAbility(FA)
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start DataAbility.
     */
    int CheckCallDataAbilityPermission(AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start ServiceExtension(Stage) or DataShareExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start ServiceExtension.
     */
    int CheckCallServiceExtensionPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start other Extension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start OtherExtension.
     */
    int CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start ServiceAbility(FA).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start ServiceAbility.
     */
    int CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start PageAbility(FA) or Ability(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Ability.
     */
    int CheckCallAbilityPermission(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start Ability(Stage) by call.
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Ability by call.
     */
    int CheckStartByCallPermission(const AbilityRequest &abilityRequest);

    /**
     * Judge if Caller-Application is in background state.
     *
     * @param abilityRequest, abilityRequest.
     * @param isBackgroundCall, Indicates the Caller-Application state.
     *                          TRUE: The Caller-Application is not in focus and not in foreground state.
     *                          FALSE: The Caller-Application is in focus or in foreground state.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall);

    bool IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest);

    /**
     *  Temporary, use old rule to check permission.
     *
     * @param abilityRequest, abilityRequest.
     * @param isStartByCall, Indicates whether it is the StartAbilityByCall function call.
     *                       TRUE: Is StartAbilityByCall function call.
     *                       FALSE: Is other function call, such as StartAbility, ConnectAbility and so on.
     * @return Returns ERR_OK on check success, others on check failure.
     */
    int CheckCallerPermissionOldRule(const AbilityRequest &abilityRequest, const bool isStartByCall = false);

    /**
     *  Temporary, judge is use new check-permission rule to start ability.
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns TRUE: Use new rule.
     *                 FALSE: Use old rule.
     */
    bool IsUseNewStartUpRule(const AbilityRequest &abilityRequest);

    bool CheckNewRuleSwitchState(const std::string &param);

    void UpdateFocusState(std::vector<AbilityRunningInfo> &info);

    AAFwk::PermissionVerification::VerificationInfo CreateVerificationInfo(
        const AbilityRequest &abilityRequest);

    int AddStartControlParam(Want &want, const sptr<IRemoteObject> &callerToken);

    void RecoverAbilityRestart(const Want &want);

    AAFwk::EventInfo BuildEventInfo(const Want &want, int32_t userId);

    int CheckDlpForExtension(
        const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, AAFwk::EventInfo &eventInfo, const EventName &eventName);

    void InitStartupFlag();

    void UpdateAbilityRequestInfo(const sptr<Want> &want, AbilityRequest &request);

    ComponentRequest initComponentRequest(const sptr<IRemoteObject> &callerToken = nullptr,
        const int requestCode = -1, const int componentStatus = 0);

    inline bool IsCrossUserCall(int32_t userId)
    {
        return (userId != INVALID_USER_ID && userId != U0_USER_ID && userId != GetUserId());
    }

    constexpr static int REPOLL_TIME_MICRO_SECONDS = 1000000;
    constexpr static int WAITING_BOOT_ANIMATION_TIMER = 5;

    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<AbilityEventHandler> handler_;
    ServiceRunningState state_;
    std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> connectManagers_;
    std::shared_ptr<AbilityConnectManager> connectManager_;
    sptr<AppExecFwk::IBundleMgr> iBundleManager_;
    std::unordered_map<int, std::shared_ptr<DataAbilityManager>> dataAbilityManagers_;
    std::shared_ptr<DataAbilityManager> dataAbilityManager_;
    std::shared_ptr<DataAbilityManager> systemDataAbilityManager_;
    std::unordered_map<int, std::shared_ptr<PendingWantManager>> pendingWantManagers_;
    std::shared_ptr<PendingWantManager> pendingWantManager_;
    const static std::map<std::string, AbilityManagerService::DumpKey> dumpMap;
    const static std::map<std::string, AbilityManagerService::DumpsysKey> dumpsysMap;
    const static std::map<int32_t, AppExecFwk::SupportWindowMode> windowModeMap;

    std::unordered_map<int, std::shared_ptr<MissionListManager>> missionListManagers_;
    std::shared_ptr<MissionListManager> currentMissionListManager_;

    std::shared_ptr<FreeInstallManager> freeInstallManager_;

    std::shared_ptr<UserController> userController_;
    sptr<AppExecFwk::IAbilityController> abilityController_ = nullptr;
    bool controllerIsAStabilityTest_ = false;
    std::recursive_mutex globalLock_;
    std::shared_mutex managersMutex_;
    sptr<AppExecFwk::IComponentInterception> componentInterception_ = nullptr;

    std::multimap<std::string, std::string> timeoutMap_;

    static sptr<AbilityManagerService> instance_;

    // Component StartUp rule switch
    bool startUpNewRule_ = true;
    /** It only takes effect when startUpNewRule_ is TRUE
     *  TRUE: When Caller-Application is Launcher or SystemUI, use old rule.
     *  FALSE: Apply new rule to all application
     */
    bool newRuleExceptLauncherSystemUI_ = true;
    /** Indicates the criteria for judging whether the Caller-Application is in the background
     *  TRUE: Determine the state by AAFwk::AppState::FOREGROUND.
     *  FALSE: Determine the state by AppExecFwk::AppProcessState::APP_STATE_FOCUS.
     */
    bool backgroundJudgeFlag_ = true;
    /** The applications in white list use old rule
     *  TRUE: white list enable.
     *  FALSE: white list unable.
     */
    bool whiteListNormalFlag_ = true;
    /** The applications in white list can associatedWakeUp
     *  TRUE: white list enable.
     *  FALSE: white list unable.
     */
    bool whiteListassociatedWakeUpFlag_ = true;

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    std::shared_ptr<BackgroundTaskObserver> bgtaskObserver_;
#endif

#ifdef SUPPORT_GRAPHICS
    int32_t ShowPickerDialog(const Want& want, int32_t userId, const sptr<IRemoteObject> &token);
    bool CheckWindowMode(int32_t windowMode, const std::vector<AppExecFwk::SupportWindowMode>& windowModes) const;
    std::shared_ptr<ImplicitStartProcessor> implicitStartProcessor_;
    sptr<IWindowManagerServiceHandler> wmsHandler_;
#endif
    std::shared_ptr<AbilityInterceptorExecuter> interceptorExecuter_;
    std::unordered_map<int32_t, int64_t> appRecoveryHistory_; // uid:time
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_SERVICE_H
