/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H

#include "ability_manager_interface.h"

#include <iremote_object.h>
#include <iremote_stub.h>

#include "dlp_connection_info.h"
#include "hilog_wrapper.h"
#include "iconnection_observer.h"

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
/**
 * @class AbilityManagerStub
 * AbilityManagerStub.
 */
class AbilityManagerStub : public IRemoteStub<IAbilityManager> {
public:
    AbilityManagerStub();
    ~AbilityManagerStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

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

    virtual int RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    virtual int UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    virtual int GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos);

    virtual int GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &connectionData);

private:
    void FirstStepInit();
    void SecondStepInit();
    void ThirdStepInit();
    void FourthStepInit();
    int TerminateAbilityInner(MessageParcel &data, MessageParcel &reply);
    int TerminateUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int CloseUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int SendResultToAbilityInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeAbilityInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int MinimizeUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply);
    int AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleCommandAbilityWindowDoneInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionSnapshotInner(MessageParcel &data, MessageParcel &reply);
    int AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply);
    int KillProcessInner(MessageParcel &data, MessageParcel &reply);
    int UninstallAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpgradeAppInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityInnerSpecifyTokenId(MessageParcel &data, MessageParcel &reply);
    int StartAbilityByUIContentSessionAddCallerInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityByUIContentSessionForOptionsInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityAsCallerByTokenInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityAsCallerForOptionInner(MessageParcel &data, MessageParcel &reply);
    int StartExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int StopExtensionAbilityInner(MessageParcel& data, MessageParcel& reply);
    int StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply);
    int ConnectAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ConnectAbilityWithTypeInner(MessageParcel &data, MessageParcel &reply);
    int ConnectUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply);
    int DumpStateInner(MessageParcel &data, MessageParcel &reply);
    int DumpSysStateInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForSettingsInner(MessageParcel &data, MessageParcel &reply);
    int StartAbilityForOptionsInner(MessageParcel &data, MessageParcel &reply);
    int RequestModalUIExtensionInner(MessageParcel &data, MessageParcel &reply);
    int ChangeAbilityVisibilityInner(MessageParcel &data, MessageParcel &reply);
    int ChangeUIAbilityVisibilityBySCBInner(MessageParcel &data, MessageParcel &reply);

    int GetWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int SendWantSenderInner(MessageParcel &data, MessageParcel &reply);
    int CancelWantSenderInner(MessageParcel &data, MessageParcel &reply);

    int GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply);
    int GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply);

    int RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply);

    int GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply);
    int GetWantSenderInfoInner(MessageParcel &data, MessageParcel &reply);

    int GetAppMemorySizeInner(MessageParcel &data, MessageParcel &reply);
    int IsRamConstrainedDeviceInner(MessageParcel &data, MessageParcel &reply);
    int ClearUpApplicationDataInner(MessageParcel &data, MessageParcel &reply);

    int ContinueMissionInner(MessageParcel &data, MessageParcel &reply);
    int ContinueMissionOfBundleNameInner(MessageParcel &data, MessageParcel &reply);
    int ContinueAbilityInner(MessageParcel &data, MessageParcel &reply);
    int StartContinuationInner(MessageParcel &data, MessageParcel &reply);
    int NotifyCompleteContinuationInner(MessageParcel &data, MessageParcel &reply);
    int NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteOnListenerInner(MessageParcel &data, MessageParcel &reply);
    int RegisterRemoteOffListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnRegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply);

    int LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply);
    int UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply);
    int SetLockedStateInner(MessageParcel &data, MessageParcel &reply);
    int RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionInfoInner(MessageParcel &data, MessageParcel &reply);
    int CleanMissionInner(MessageParcel &data, MessageParcel &reply);
    int CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionToFrontInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply);

    // for new version ability (call ability)
    int StartAbilityByCallInner(MessageParcel &data, MessageParcel &reply);
    int CallRequestDoneInner(MessageParcel &data, MessageParcel &reply);
    int ReleaseCallInner(MessageParcel &data, MessageParcel &reply);
    int StartUserInner(MessageParcel &data, MessageParcel &reply);
    int StopUserInner(MessageParcel &data, MessageParcel &reply);
    int LogoutUserInner(MessageParcel &data, MessageParcel &reply);
    int GetAbilityRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetExtensionRunningInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetProcessRunningInfosInner(MessageParcel &data, MessageParcel &reply);

    int StartSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply);
    int StopSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply);
    int RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply);
    int GetMissionSnapshotInfoInner(MessageParcel &data, MessageParcel &reply);
    int SendANRProcessIDInner(MessageParcel &data, MessageParcel &reply);

    int SetAbilityControllerInner(MessageParcel &data, MessageParcel &reply);

    int StartUserTestInner(MessageParcel &data, MessageParcel &reply);
    int FinishUserTestInner(MessageParcel &data, MessageParcel &reply);
    int GetTopAbilityTokenInner(MessageParcel &data, MessageParcel &reply);
    int CheckUIExtensionIsFocusedInner(MessageParcel &data, MessageParcel &reply);
    int DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply);
    int DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int DoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply);
    int DoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply);

    int IsRunningInStabilityTestInner(MessageParcel &data, MessageParcel &reply);
    int MoveMissionToFrontByOptionsInner(MessageParcel &data, MessageParcel &reply);

    int UpdateMissionSnapShotFromWMSInner(MessageParcel &data, MessageParcel &reply);
    int RegisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply);
    int UnregisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply);
    int GetDlpConnectionInfosInner(MessageParcel &data, MessageParcel &reply);
    int GetConnectionDataInner(MessageParcel &data, MessageParcel &reply);
    int MoveAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int32_t MoveUIAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply);
    int SetMissionContinueStateInner(MessageParcel &data, MessageParcel &reply);
#ifdef SUPPORT_GRAPHICS
    int SetMissionLabelInner(MessageParcel &data, MessageParcel &reply);
    int SetMissionIconInner(MessageParcel &data, MessageParcel &reply);
    int RegisterWindowManagerServiceHandlerInner(MessageParcel &data, MessageParcel &reply);
    int CompleteFirstFrameDrawingInner(MessageParcel &data, MessageParcel &reply);
    int PrepareTerminateAbilityInner(MessageParcel &data, MessageParcel &reply);
    int GetDialogSessionInfoInner(MessageParcel &data, MessageParcel &reply);
    int SendDialogResultInner(MessageParcel &data, MessageParcel &reply);
#endif

    #ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTestInner(MessageParcel &data, MessageParcel &reply);
    #endif

    int FreeInstallAbilityFromRemoteInner(MessageParcel &data, MessageParcel &reply);
    int AddFreeInstallObserverInner(MessageParcel &data, MessageParcel &reply);

    using RequestFuncType = int (AbilityManagerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;

    #ifdef ABILITY_COMMAND_FOR_TEST
    int BlockAmsServiceInner(MessageParcel &data, MessageParcel &reply);
    int BlockAbilityInner(MessageParcel &data, MessageParcel &reply);
    int BlockAppServiceInner(MessageParcel &data, MessageParcel &reply);
    #endif
    int EnableRecoverAbilityInner(MessageParcel &data, MessageParcel &reply);
    int ScheduleRecoverAbilityInner(MessageParcel &data, MessageParcel &reply);
    int GetTopAbilityInner(MessageParcel &data, MessageParcel &reply);
    int GetElementNameByTokenInner(MessageParcel &data, MessageParcel &reply);
    int DumpAbilityInfoDoneInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsValidMissionIdsInner(MessageParcel &data, MessageParcel &reply);

    int VerifyPermissionInner(MessageParcel &data, MessageParcel &reply);

    int HandleRequestDialogService(MessageParcel &data, MessageParcel &reply);
    int32_t HandleReportDrawnCompleted(MessageParcel &data, MessageParcel &reply);

    int AcquireShareDataInner(MessageParcel &data, MessageParcel &reply);
    int ShareDataDoneInner(MessageParcel &data, MessageParcel &reply);
    int GetAbilityTokenByCalleeObjInner(MessageParcel &data, MessageParcel &reply);

    int32_t ForceExitAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t RecordAppExitReasonInner(MessageParcel &data, MessageParcel &reply);
    int32_t RecordProcessExitReasonInner(MessageParcel &data, MessageParcel &reply);

    int SetRootSceneSessionInner(MessageParcel &data, MessageParcel &reply);
    int CallUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int StartSpecifiedAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int NotifySaveAsResultInner(MessageParcel &data, MessageParcel &reply);

    int SetSessionManagerServiceInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply);
    int32_t CancelApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryAllAutoStartupApplicationsInner(MessageParcel &data, MessageParcel &reply);

    int PrepareTerminateAbilityBySCBInner(MessageParcel &data, MessageParcel &reply);
    int RegisterSessionHandlerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateSessionInfoBySCBInner(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t AttachAppDebugInner(MessageParcel &data, MessageParcel &reply);
    int32_t DetachAppDebugInner(MessageParcel &data, MessageParcel &reply);
    int32_t ExecuteIntentInner(MessageParcel &data, MessageParcel &reply);

    int32_t SetApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply);
    int32_t CancelApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply);

    int32_t IsAbilityControllerStartInner(MessageParcel &data, MessageParcel &reply);
    int32_t OpenFileInner(MessageParcel &data, MessageParcel &reply);

    int32_t GetElementNameByAppIdInner(MessageParcel &data, MessageParcel &reply);
    int32_t OpenAtomicServiceInner(MessageParcel &data, MessageParcel &reply);
    int32_t IsEmbeddedOpenAllowedInner(MessageParcel &data, MessageParcel &reply);

    //insight intent related
    int32_t StartAbilityByInsightIntentInner(MessageParcel &data, MessageParcel &reply);
    int32_t ExecuteInsightIntentDoneInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetForegroundUIAbilitiesInner(MessageParcel &data, MessageParcel &reply);

    int32_t GetUIExtensionRootHostInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t RestartAppInner(MessageParcel &data, MessageParcel &reply);
    int32_t RequestAssertFaultDialogInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyDebugAssertResultInner(MessageParcel &data, MessageParcel &reply);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_STUB_H
