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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H

#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include "cpp/mutex.h"

#include "ability_connect_callback_interface.h"
#include "task_handler_wrap.h"
#include "event_handler_wrap.h"
#include "ability_record.h"
#include "ability_running_info.h"
#include "extension_config.h"
#include "extension_running_info.h"
#include "connection_record.h"
#include "element_name.h"
#include "ui_extension_ability_connect_info.h"
#include "ui_extension_utils.h"
#include "extension_record_manager.h"
#include "want.h"
#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AppExecFwk::AbilityType;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
using UIExtensionAbilityConnectManager = AbilityRuntime::ExtensionRecordManager;

/**
 * @class AbilityConnectManager
 * AbilityConnectManager provides a facility for managing service ability connection.
 */
class AbilityConnectManager : public std::enable_shared_from_this<AbilityConnectManager> {
public:
    using ConnectMapType = std::map<sptr<IRemoteObject>, std::list<std::shared_ptr<ConnectionRecord>>>;
    using ServiceMapType = std::map<std::string, std::shared_ptr<AbilityRecord>>;
    using ConnectListType = std::list<std::shared_ptr<ConnectionRecord>>;
    using RecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;
    using UIExtWindowMapValType = std::pair<std::weak_ptr<AbilityRecord>, sptr<SessionInfo>>;
    using UIExtensionMapType = std::map<sptr<IRemoteObject>, UIExtWindowMapValType>;
    using WindowExtMapValType = std::pair<uint32_t, sptr<SessionInfo>>;
    using WindowExtensionMapType = std::map<sptr<IRemoteObject>, WindowExtMapValType>;

    explicit AbilityConnectManager(int userId);
    virtual ~AbilityConnectManager();

    /**
     * StartAbility with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbility(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbility with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbility(const sptr<IRemoteObject> &token);

    /**
     * StopServiceAbility with request.
     *
     * @param abilityRequest, request.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbility(const AbilityRequest &abilityRequest);

    /**
     * ConnectAbilityLocked, connect session with service ability.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param connectInfo the connect info.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ConnectAbilityLocked(const AbilityRequest &abilityRequest, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo = nullptr,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect);

    /**
     * AttachAbilityThreadLocked, ability call this interface after loaded.
     *
     * @param scheduler, the interface handler of kit ability.
     * @param token, ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AttachAbilityThreadLocked(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token);

    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state);

    void OnAppStateChanged(const AppInfo &info);

    /**
     * AbilityTransitionDone, ability call this interface after lift cycle was changed.
     *
     * @param token, ability's token.
     * @param state, the state of ability lift cycle.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state);

    /**
     * ScheduleConnectAbilityDoneLocked, service ability call this interface while session was connected.
     *
     * @param token, service ability's token.
     * @param remoteObject, the session proxy of service ability.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleConnectAbilityDoneLocked(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject);

    /**
     * ScheduleDisconnectAbilityDone, service ability call this interface while session was disconnected.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleDisconnectAbilityDoneLocked(const sptr<IRemoteObject> &token);

    /**
     * ScheduleCommandAbilityDoneLocked, service ability call this interface while session was onCommanded.
     *
     * @param token,service ability's token.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ScheduleCommandAbilityDoneLocked(const sptr<IRemoteObject> &token);

    int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo,
        WindowCommand winCmd,
        AbilityCommand abilityCmd);

    /**
     * GetServiceRecordByElementName.
     *
     * @param element, service ability's element.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetServiceRecordByElementName(const std::string &element);

    /**
     * GetUIExtensioBySessionInfo.
     *
     * @param sessionToken, service ability's session token.
     * @return Returns AbilityRecord shared_ptr.
     */
    std::shared_ptr<AbilityRecord> GetUIExtensioBySessionInfo(const sptr<SessionInfo> &sessionInfo);

    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromServiceMap(const sptr<IRemoteObject> &token);
    std::shared_ptr<AbilityRecord> GetExtensionByTokenFromTerminatingMap(const sptr<IRemoteObject> &token);
    ConnectListType GetConnectRecordListByCallback(sptr<IAbilityConnection> callback);

    void GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info,
        const int32_t userId, bool isPerm);

    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);

    void GetExtensionRunningInfo(std::shared_ptr<AbilityRecord> &abilityRecord, const int32_t userId,
        std::vector<ExtensionRunningInfo> &info);

    /**
     * set from ability manager service for sequenced task
     */
    inline void SetTaskHandler(const std::shared_ptr<TaskHandlerWrap> &taskHandler)
    {
        taskHandler_ = taskHandler;
    }
    /**
     * SetEventHandler.
     *
     * @param handler,EventHandler
     */
    inline void SetEventHandler(const std::shared_ptr<EventHandlerWrap> &handler)
    {
        eventHandler_ = handler;
    }

    /**
     * GetServiceMap.
     *
     * @return Returns service ability record map.
     */
    inline const ServiceMapType &GetServiceMap() const
    {
        return serviceMap_;
    }

    /**
     * OnAbilityDied.
     *
     * @param abilityRecord, service ability record.
     */
    void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);

    void DumpState(std::vector<std::string> &info, bool isClient, const std::string &args = "");

    void DumpStateByUri(std::vector<std::string> &info, bool isClient, const std::string &args,
        std::vector<std::string> &params);

    void PauseExtensions();

    void StartRootLauncher(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void OnTimeOut(uint32_t msgId, int64_t abilityRecordId);

    void MoveToForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    /**
     * @brief schedule to background
     *
     * @param abilityRecord the ability to move
     */
    void MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void CommandAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);

    bool IsUIExtensionFocused(uint32_t uiExtensionTokenId, const sptr<IRemoteObject>& focusToken);

    bool IsWindowExtensionFocused(uint32_t extensionTokenId, const sptr<IRemoteObject>& focusToken);

    void HandleProcessFrozen(const std::vector<int32_t> &pidList, int32_t uid);

    void ForegroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void BackgroundAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void TerminateAbilityWindowLocked(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);

    void RemoveLauncherDeathRecipient();

    // MSG 0 - 20 represents timeout message
    static constexpr uint32_t LOAD_TIMEOUT_MSG = 0;
    static constexpr uint32_t CONNECT_TIMEOUT_MSG = 1;

private:
    /**
     * StartAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StartAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * TerminateAbilityLocked with token and result want.
     *
     * @param token, the token of service type's ability to terminate.
     * @param resultCode, the result code of service type's ability to terminate.
     * @param resultWant, the result want for service type's ability to terminate.
     * @return Returns ERR_OK on success, others on failure.
     */
    int TerminateAbilityLocked(const sptr<IRemoteObject> &token);

    /**
     * StopAbilityLocked with request.
     *
     * @param abilityRequest, the request of the service ability to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    int StopServiceAbilityLocked(const AbilityRequest &abilityRequest);

    /**
     * DisconnectAbilityLocked, disconnect session with callback.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param force, Indicates forcing to disconnect and clear. For example, it is called when the source
     * dies and the connection has not completed yet.
     * @return Returns ERR_OK on success, others on failure.
     */
    int DisconnectAbilityLocked(const sptr<IAbilityConnection> &connect, bool force);

    /**
     * LoadAbility.
     *
     * @param abilityRecord, the ptr of the ability to load.
     */
    void LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * ConnectAbility.Schedule connect ability
     *
     * @param abilityRecord, the ptr of the ability to connect.
     */
    void ConnectAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CommandAbility. Schedule command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CommandAbility(const std::shared_ptr<AbilityRecord> &abilityRecord);

    /**
     * CompleteCommandAbility. complete command ability
     *
     * @param abilityRecord, the ptr of the ability to command.
     */
    void CompleteCommandAbility(std::shared_ptr<AbilityRecord> abilityRecord);

    /**
     * TerminateDone.
     *
     * @param abilityRecord, the ptr of the ability to terminate.
     */
    void TerminateDone(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void HandleStartTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int resultCode);
    void HandleStopTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleTerminateDisconnectTask(const ConnectListType& connectlist);
    void HandleCommandTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleCommandWindowTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd);
    void HandleForegroundTimeoutTask(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void HandleRestartResidentTask(const AbilityRequest &abilityRequest);
    void HandleActiveAbility(std::shared_ptr<AbilityRecord> &targetService,
        std::shared_ptr<ConnectionRecord> &connectRecord);
    void HandleCommandDestroy(const sptr<SessionInfo> &sessionInfo);

    /**
     * RemoveConnectionRecordFromMap.
     *
     * @param connect, the ptr of the connect record.
     */
    void RemoveConnectionRecordFromMap(const std::shared_ptr<ConnectionRecord> &connect);

    /**
     * GetOrCreateServiceRecord.
     *
     * @param abilityRequest, Special want for service type's ability.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param targetAbilityRecord, the target service ability record.
     * @param isLoadedAbility, whether the target ability has been loaded.
     */
    void GetOrCreateServiceRecord(const AbilityRequest &abilityRequest, const bool isCreatedByConnect,
        std::shared_ptr<AbilityRecord> &targetAbilityRecord, bool &isLoadedAbility);

    /**
     * AddConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void AddConnectDeathRecipient(const sptr<IAbilityConnection> &connect);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param connect, callback object.
     */
    void RemoveConnectDeathRecipient(const sptr<IAbilityConnection> &connect);

    /**
     * RemoteConnectDeathRecipient.
     *
     * @param remote, callback object.
     */
    void OnCallBackDied(const wptr<IRemoteObject> &remote);

    /**
     * HandleOnCallBackDied.
     *
     * @param connect, callback object.
     */
    void HandleCallBackDiedTask(const sptr<IRemoteObject> &connect);

    /**
     * HandleOnCallBackDied.
     *
     * @param abilityRecord, died ability.
     */
    void HandleAbilityDiedTask(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);
    void HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void RestartAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, int32_t currentUserId);

    /**
     * PostTimeOutTask.
     *
     * @param abilityRecord, ability.
     * @param messageId, message id.
     */
    void PostTimeOutTask(const std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t messageId);

    void CompleteForeground(const std::shared_ptr<AbilityRecord> &abilityRecord);
    void CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void PostRestartResidentTask(const AbilityRequest &abilityRequest);

    bool IsAbilityNeedKeepAlive(const std::shared_ptr<AbilityRecord> &abilityRecord);

    void ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const;

    std::shared_ptr<AbilityRecord> GetAbilityRecordById(int64_t abilityRecordId);
    void HandleInactiveTimeout(const std::shared_ptr<AbilityRecord> &ability);
    void MoveToTerminatingMap(const std::shared_ptr<AbilityRecord>& abilityRecord);

    void DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord, const AbilityRequest &abilityRequest);
    void SaveUIExtRequestSessionInfo(std::shared_ptr<AbilityRecord> abilityRecord, sptr<SessionInfo> sessionInfo);
    void DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
        const sptr<SessionInfo> &sessionInfo);
    void DoTerminateUIExtensionAbility(std::shared_ptr<AbilityRecord> abilityRecord, sptr<SessionInfo> sessionInfo);

    /**
     * When a service is under starting, enque the request and handle it after the service starting completes
     */
    void EnqueueStartServiceReq(const AbilityRequest &abilityRequest, const std::string &serviceUri = "");
    /**
     * After the service starting completes, complete the request list
     */
    void CompleteStartServiceReq(const std::string &serviceUri);

    void AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);
    void RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session);
    void OnUIExtWindowDied(const wptr<IRemoteObject> &remote);
    void HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote);

    /**
     * Post an extension's disconnect task, auto disconnect when extension conected timeout.
     */
    void PostExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Remove the extension's disconnect task.
     */
    void RemoveExtensionDelayDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * Handle extension disconnect task.
     */
    void HandleExtensionDisconnectTask(const std::shared_ptr<ConnectionRecord> &connectRecord);

    /**
     * dispatch service ability life cycle .
     *
     * @param abilityRecord.
     * @param state.
     */
    int DispatchInactive(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
    {
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
        CHECK_POINTER_AND_RETURN(eventHandler_, ERR_INVALID_VALUE);
        if (!abilityRecord->IsAbilityState(AbilityState::INACTIVATING)) {
            HILOG_ERROR("Ability transition life state error. expect %{public}d, actual %{public}d callback %{public}d",
                        AbilityState::INACTIVATING,
                        abilityRecord->GetAbilityState(),
                        state);
            return ERR_INVALID_VALUE;
        }
        eventHandler_->RemoveEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());

        // complete inactive
        abilityRecord->SetAbilityState(AbilityState::INACTIVE);
        if (abilityRecord->IsCreateByConnect()) {
            ConnectAbility(abilityRecord);
        } else {
            CommandAbility(abilityRecord);
            if (abilityRecord->GetConnectRecordList().size() > 0) {
                // It means someone called connectAbility when service was loading
                ConnectAbility(abilityRecord);
            }
        }

        return ERR_OK;
    }
    int DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
        CHECK_POINTER_AND_RETURN(taskHandler_, ERR_INVALID_VALUE);
        // remove foreground timeout task.
        taskHandler_->CancelTask("foreground_" + std::to_string(abilityRecord->GetAbilityRecordId()));

        auto self(shared_from_this());
        auto task = [self, abilityRecord]() { self->CompleteForeground(abilityRecord); };
        taskHandler_->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

        return ERR_OK;
    }
    int DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
        CHECK_POINTER_AND_RETURN(taskHandler_, ERR_INVALID_VALUE);
        // remove background timeout task.
        taskHandler_->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));

        auto self(shared_from_this());
        auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
        taskHandler_->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

        return ERR_OK;
    }
    int DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
        // remove terminate timeout task
        if (taskHandler_ != nullptr) {
            taskHandler_->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
        }
        // complete terminate
        TerminateDone(abilityRecord);
        return ERR_OK;
    }
    /**
     * RemoveServiceAbility.
     *
     * @param service, the ptr of the ability record.
     */
    void RemoveServiceAbility(const std::shared_ptr<AbilityRecord> &service)
    {
        CHECK_POINTER(abilityRecord);
        HILOG_DEBUG("Remove service(%{public}s) from terminating map.", abilityRecord->GetURI().c_str());
        terminatingExtensionMap_.erase(abilityRecord->GetURI());
    }

    /**
     * IsAbilityConnected.
     *
     * @param abilityRecord, the ptr of the connected ability.
     * @param connectRecordList, connect record list.
     * @return true: ability is connected, false: ability is not connected
     */
    bool IsAbilityConnected(const std::shared_ptr<AbilityRecord> &abilityRecord,
                            const std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
    {
        auto isMatch = [abilityRecord](auto connectRecord) -> bool {
            if (abilityRecord == nullptr || connectRecord == nullptr) {
                return false;
            }
            if (abilityRecord != connectRecord->GetAbilityRecord()) {
                return false;
            }
            return true;
        };
        return std::any_of(connectRecordList.begin(), connectRecordList.end(), isMatch);
    }

    /**
     * GetConnectRecordListFromMap.
     *
     * @param connect, callback object.
     * @param isCreatedByConnect, whether is created by connect ability mode.
     * @param connectRecordList, the target connectRecordList.
     * @param isCallbackConnected, whether the callback has been connected.
     */
    void GetConnectRecordListFromMap(
            const sptr<IAbilityConnection> &connect, std::list<std::shared_ptr<ConnectionRecord>> &connectRecordList)
    {
        auto connectMapIter = connectMap_.find(connect->AsObject());
        if (connectMapIter != connectMap_.end()) {
            connectRecordList = connectMapIter->second;
        }
    }

    void PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId)
    {
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr");
            return;
        }

        AppExecFwk::RunningProcessInfo processInfo = {};
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
        if (processInfo.pid_ == 0) {
            HILOG_ERROR("error: the ability[%{public}s], app may fork fail or not running.",
                        ability->GetAbilityInfo().name.data());
            return;
        }
        int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
        std::string msgContent = "ability:" + ability->GetAbilityInfo().name + " ";
        switch (msgId) {
            case AbilityManagerService::LOAD_TIMEOUT_MSG:
                msgContent += "load timeout";
                typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
                break;
            case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
                msgContent += "active timeout";
                break;
            case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
                msgContent += "inactive timeout";
                break;
            case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
                msgContent += "foreground timeout";
                typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
                break;
            case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
                msgContent += "background timeout";
                break;
            case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
                msgContent += "terminate timeout";
                break;
            default:
                return;
        }

        HILOG_WARN("LIFECYCLE_TIMEOUT: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, abilityName: %{public}s,"
                   "msg: %{public}s", processInfo.uid_, processInfo.pid_, ability->GetAbilityInfo().bundleName.c_str(),
                   ability->GetAbilityInfo().name.c_str(), msgContent.c_str());
        AppExecFwk::AppfreezeManager::ParamInfo info = {
                .typeId = typeId,
                .pid = processInfo.pid_,
                .eventName = AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT,
                .bundleName = ability->GetAbilityInfo().bundleName,
                .msg = msgContent
        };
        AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info);
    }

private:
    inline bool CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest);
    inline bool CheckUIExtensionAbilitySessionExistLocked(const std::shared_ptr<AbilityRecord> &abilityRecord);
    inline void RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord);
    int TerminateAbilityInner(const sptr<IRemoteObject> &token)
    {
        auto abilityRecord = GetExtensionFromServiceMapInner(token);
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
        std::string element = abilityRecord->GetURI();
        HILOG_DEBUG("Terminate ability, ability is %{public}s.", element.c_str());
        if (IsUIExtensionAbility(abilityRecord) && !abilityRecord->IsConnectListEmpty()) {
            HILOG_INFO("There exist connection, don't terminate.");
            return ERR_OK;
        }
        MoveToTerminatingMap(abilityRecord);
        return TerminateAbilityLocked(token);
    }
    void TerminateRecord(std::shared_ptr<AbilityRecord> abilityRecord)
    {
        auto timeoutTask = [abilityRecord, connectManager = shared_from_this()]() {
            HILOG_WARN("Disconnect ability terminate timeout.");
            connectManager->HandleStopTimeoutTask(abilityRecord);
        };

        MoveToTerminatingMap(abilityRecord);
        abilityRecord->Terminate(timeoutTask);
    }
    void DisconnectRecordForce(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord)
    {
        auto abilityRecord = connectRecord->GetAbilityRecord();
        if (abilityRecord == nullptr) {
            HILOG_ERROR("Disconnect force abilityRecord null");
            return;
        }
        abilityRecord->RemoveConnectRecordFromList(connectRecord);
        connectRecord->CompleteDisconnect(ERR_OK, true);
        list.emplace_back(connectRecord);
        if (abilityRecord->IsConnectListEmpty() && abilityRecord->GetStartId() == 0) {
            HILOG_WARN("Force terminate ability record state: %{public}d.", abilityRecord->GetAbilityState());
            TerminateRecord(abilityRecord);
        }
    }
    int DisconnectRecordNormal(ConnectListType &list, std::shared_ptr<ConnectionRecord> connectRecord) const
    {
        auto result = connectRecord->DisconnectAbility();
        if (result != ERR_OK) {
            HILOG_ERROR("Disconnect ability fail , ret = %{public}d.", result);
            return result;
        }

        if (connectRecord->GetConnectState() == ConnectionState::DISCONNECTED) {
            HILOG_WARN("This record: %{public}d complete disconnect directly.", connectRecord->GetRecordId());
            connectRecord->CompleteDisconnect(ERR_OK, false);
            list.emplace_back(connectRecord);
        }
        return ERR_OK;
    }
    int32_t GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest, bool isCreatedByConnect,
        const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord, bool &isLoaded)
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
                                        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
        CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
        if (uiExtensionAbilityRecordMgr_->IsBelongToManager(abilityRequest.abilityInfo)) {
            int32_t ret = uiExtensionAbilityRecordMgr_->GetOrCreateExtensionRecord(
                    abilityRequest, hostBundleName, extensionRecord, isLoaded);
            if (ret != ERR_OK) {
                return ret;
            }
            extensionRecord->SetCreateByConnectMode(isCreatedByConnect);
            std::string extensionRecordKey = element.GetURI() + std::to_string(extensionRecord->GetUIExtensionAbilityId());
            extensionRecord->SetURI(extensionRecordKey);
            HILOG_DEBUG("Service map add, hostBundleName:%{public}s, key: %{public}s", hostBundleName.c_str(),
                        extensionRecordKey.c_str());
            serviceMap_.emplace(extensionRecordKey, extensionRecord);
            if (IsAbilityNeedKeepAlive(extensionRecord)) {
                extensionRecord->SetKeepAlive();
                extensionRecord->SetRestartTime(abilityRequest.restartTime);
                extensionRecord->SetRestartCount(abilityRequest.restartCount);
            }
            return ERR_OK;
        }
        return ERR_INVALID_VALUE;
    }
    int32_t GetOrCreateTargetServiceRecord(
        const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
        std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility)
    {
        if (UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType) && connectInfo != nullptr) {
            int32_t ret = GetOrCreateExtensionRecord(
                    abilityRequest, true, connectInfo->hostBundleName, targetService, isLoadedAbility);
            if (ret != ERR_OK || targetService == nullptr) {
                HILOG_ERROR("Failed to get or create extension record.");
                return ERR_NULL_OBJECT;
            }
            connectInfo->uiExtensionAbilityId = targetService->GetUIExtensionAbilityId();
            HILOG_DEBUG("UIExtensionAbility id %{public}d.", connectInfo->uiExtensionAbilityId);
        } else {
            GetOrCreateServiceRecord(abilityRequest, true, targetService, isLoadedAbility);
        }
        CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
        return ERR_OK;
    }
    std::shared_ptr<AbilityRecord> GetServiceRecordByElementNameInner(const std::string &element)
    {
        auto mapIter = serviceMap_.find(element);
        if (mapIter != serviceMap_.end()) {
            return mapIter->second;
        }
        return nullptr;
    }
    std::shared_ptr<AbilityRecord> GetExtensionFromServiceMapInner(const sptr<IRemoteObject> &token)
    {
        auto IsMatch = [token](auto service) {
            if (!service.second) {
                return false;
            }
            sptr<IRemoteObject> srcToken = service.second->GetToken();
            return srcToken == token;
        };
        auto serviceRecord = std::find_if(serviceMap_.begin(), serviceMap_.end(), IsMatch);
        if (serviceRecord != serviceMap_.end()) {
            return serviceRecord->second;
        }
        return nullptr;
    }
    std::shared_ptr<AbilityRecord> GetExtensionFromServiceMapInner(int32_t abilityRecordId)
    {
        for (const auto &[key, value] : serviceMap_) {
            if (value && value->GetAbilityRecordId() == abilityRecordId) {
                return value;
            }
        }
        return nullptr;
    }
    std::shared_ptr<AbilityRecord> GetExtensionFromTerminatingMapInner(const sptr<IRemoteObject> &token)
    {
        auto IsMatch = [token](auto& extension) {
            if (extension.second == nullptr) {
                return false;
            }
            auto&& terminatingToken = extension.second->GetToken();
            if (terminatingToken != nullptr) {
                return terminatingToken->AsObject() == token;
            }
            return false;
        };

        auto terminatingExtensionRecord =
                std::find_if(terminatingExtensionMap_.begin(), terminatingExtensionMap_.end(), IsMatch);
        if (terminatingExtensionRecord != terminatingExtensionMap_.end()) {
            return terminatingExtensionRecord->second;
        }
        return nullptr;
    }
    bool IsLauncher(std::shared_ptr<AbilityRecord> serviceExtension) const
    {
        if (serviceExtension == nullptr) {
            HILOG_ERROR("param is nullptr");
            return false;
        }
        return serviceExtension->GetAbilityInfo().name == AbilityConfig::LAUNCHER_ABILITY_NAME &&
               serviceExtension->GetAbilityInfo().bundleName == AbilityConfig::LAUNCHER_BUNDLE_NAME;
    }
    bool IsSceneBoard(std::shared_ptr<AbilityRecord> serviceExtension) const
    {
        if (serviceExtension == nullptr) {
            HILOG_ERROR("param is nullptr");
            return false;
        }
        return serviceExtension->GetAbilityInfo().name == AbilityConfig::SCENEBOARD_ABILITY_NAME &&
               serviceExtension->GetAbilityInfo().bundleName == AbilityConfig::SCENEBOARD_BUNDLE_NAME;
    }
    /**
     * ability is ui extension ability
     */
    inline bool IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        CHECK_POINTER_AND_RETURN(abilityRecord, false);
        return UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType);
    }
    void KillProcessesByUserId() const
    {
        auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
        if (appScheduler == nullptr) {
            HILOG_ERROR("appScheduler is nullptr");
            return;
        }
        IN_PROCESS_CALL_WITHOUT_RET(appScheduler->KillProcessesByUserId(userId_));
    }

private:
    const std::string TASK_ON_CALLBACK_DIED = "OnCallbackDiedTask";
    const std::string TASK_ON_ABILITY_DIED = "OnAbilityDiedTask";

    ffrt::mutex Lock_;
    ConnectMapType connectMap_;
    ServiceMapType serviceMap_;
    ServiceMapType terminatingExtensionMap_;

    std::mutex recipientMapMutex_;
    RecipientMapType recipientMap_;
    RecipientMapType uiExtRecipientMap_;
    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
    int userId_;
    std::vector<AbilityRequest> restartResidentTaskList_;
    std::unordered_map<std::string, std::shared_ptr<std::list<AbilityRequest>>> startServiceReqList_;
    ffrt::mutex startServiceReqListLock_;
    UIExtensionMapType uiExtensionMap_;
    WindowExtensionMapType windowExtensionMap_;
    std::unique_ptr<UIExtensionAbilityConnectManager> uiExtensionAbilityRecordMgr_ = nullptr;

    DISALLOW_COPY_AND_MOVE(AbilityConnectManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_CONNECT_MANAGER_H
