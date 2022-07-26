/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DELEGATOR_STUB_H
#define OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DELEGATOR_STUB_H

#include "gmock/gmock.h"

#include "string_ex.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
class MockAbilityDelegatorStub : public AbilityManagerStub {
public:
    MockAbilityDelegatorStub();
    virtual int StartAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD4(StartAbility, int(const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, int requestCode));
    MOCK_METHOD5(StartAbility, int(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode));
    virtual int StartAbility(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant));
    MOCK_METHOD2(TerminateAbility, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) override;
    MOCK_METHOD4(ConnectAbility, int(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t userId));
    MOCK_METHOD1(DisconnectAbility, int(const sptr<IAbilityConnection> &connect));
    MOCK_METHOD3(AcquireDataAbility,
        sptr<IAbilityScheduler>(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(
        ReleaseDataAbility, int(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(AddWindowInfo, void(const sptr<IRemoteObject> &token, int32_t windowToken));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject> &token, int state, const PacMap &));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject> &token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject> &token));
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) override;
    MOCK_METHOD2(TerminateAbilityResult, int(const sptr<IRemoteObject> &token, int startId));
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD2(TerminateAbilityByCaller, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD1(MoveMissionToTop, int(int32_t missionId));
    MOCK_METHOD1(RemoveMission, int(int id));
    MOCK_METHOD1(RemoveStack, int(int id));
    MOCK_METHOD1(KillProcess, int(const std::string &bundleName));
    MOCK_METHOD2(UninstallApp, int(const std::string &bundleName, int32_t uid));
    MOCK_METHOD2(MoveMissionToEnd, int(const sptr<IRemoteObject> &token, const bool nonFirst));
    MOCK_METHOD1(IsFirstInMission, bool(const sptr<IRemoteObject> &token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string &permission, int pid, int uid, std::string &message));
    MOCK_METHOD2(
        GetWantSender, sptr<IWantSender>(const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(SendWantSender, int(const sptr<IWantSender> &target, const SenderInfo &senderInfo));
    MOCK_METHOD1(CancelWantSender, void(const sptr<IWantSender> &sender));
    MOCK_METHOD1(GetPendingWantUid, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantUserId, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantBundleName, std::string(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantCode, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantType, int(const sptr<IWantSender> &target));
    MOCK_METHOD2(RegisterCancelListener, void(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver));
    MOCK_METHOD2(UnregisterCancelListener, void(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver));
    MOCK_METHOD2(GetPendingRequestWant, int(const sptr<IWantSender> &target, std::shared_ptr<Want> &want));
    MOCK_METHOD1(LockMission, int(int));
    MOCK_METHOD1(UnlockMission, int(int));
    MOCK_METHOD0(GetMissionLockModeState, int());
    MOCK_METHOD2(
        ChangeFocusAbility, int(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken));
    MOCK_METHOD1(MinimizeMultiWindow, int(int missionId));
    MOCK_METHOD1(MaximizeMultiWindow, int(int missionId));
    MOCK_METHOD1(CloseMultiWindow, int(int missionId));
    MOCK_METHOD1(GetPendinTerminateAbilityTestgRequestWant, void(int id));
    MOCK_METHOD1(SetShowOnLockScreen, int(bool isAllow));
    MOCK_METHOD3(StartContinuation, int(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams));
    MOCK_METHOD3(ContinueAbility, int(const std::string &deviceId, int32_t missionId, uint32_t versionCode));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string &deviceId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD1(LockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(UnlockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(RegisterMissionListener, int(const sptr<IMissionListener> &listener));
    MOCK_METHOD1(UnRegisterMissionListener, int(const sptr<IMissionListener> &listener));
    MOCK_METHOD3(
        GetMissionInfos, int(const std::string& deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos));
    MOCK_METHOD3(GetMissionInfo, int(const std::string& deviceId, int32_t missionId, MissionInfo &missionInfo));
    MOCK_METHOD1(CleanMission, int(int32_t missionId));
    MOCK_METHOD0(CleanAllMissions, int());
    MOCK_METHOD1(MoveMissionToFront, int(int32_t missionId));
    MOCK_METHOD2(MoveMissionToFront, int(int32_t missionId, const StartOptions &startOptions));
    MOCK_METHOD1(ClearUpApplicationData, int(const std::string &));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo> &info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo> &info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo> &info));
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info));
    virtual int StartUser(int userId) override;
    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override;
    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;
    virtual int StopSyncRemoteMissions(const std::string& devId) override;
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;
    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;
    virtual int StartAbilityByCall(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) override;
    virtual int ReleaseAbility(const sptr<IAbilityConnection> &connect,
        const AppExecFwk::ElementName &element) override;
    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;
    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject>& token) override;
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;
    virtual int SendANRProcessID(int pid) override;
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override;
    virtual bool IsRunningInStabilityTest() override;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID) override;
    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override;
    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;
    virtual int GetTopAbility(sptr<IRemoteObject> &token) override;
    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override;
    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override;
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag);
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag);
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token);
    virtual int BlockAmsService();
    virtual int BlockAbility(int32_t abilityRecordId);
    virtual int BlockAppService();
#ifdef ABILITY_COMMAND_FOR_TEST
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override;
#endif
#ifdef SUPPORT_GRAPHICS
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject> &token, const std::string &label));
    virtual int SetMissionIcon(
        const sptr<IRemoteObject> &token, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;
    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler);
    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override {}
#endif
    virtual int NotifyProcessWillBeKilled(const std::string &bundleName) override;

public:
    std::string powerState_;
    static bool finishFlag_;
};

class MockAbilityDelegatorStub2 : public AbilityManagerStub {
public:
    MockAbilityDelegatorStub2();
    virtual int StartAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = -1) override;
    MOCK_METHOD4(StartAbility, int(const Want &want, const sptr<IRemoteObject> &callerToken,
        int32_t userId, int requestCode));
    MOCK_METHOD5(StartAbility, int(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode));
    virtual int StartAbility(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD3(TerminateAbility, int(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant));
    MOCK_METHOD2(TerminateAbility, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) override;
    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) override;
    MOCK_METHOD4(ConnectAbility, int(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t userId));
    MOCK_METHOD1(DisconnectAbility, int(const sptr<IAbilityConnection> &connect));
    MOCK_METHOD3(AcquireDataAbility,
        sptr<IAbilityScheduler>(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(
        ReleaseDataAbility, int(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(AddWindowInfo, void(const sptr<IRemoteObject> &token, int32_t windowToken));
    MOCK_METHOD2(AttachAbilityThread, int(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token));
    MOCK_METHOD3(AbilityTransitionDone, int(const sptr<IRemoteObject> &token, int state, const PacMap &));
    MOCK_METHOD2(
        ScheduleConnectAbilityDone, int(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject));
    MOCK_METHOD1(ScheduleDisconnectAbilityDone, int(const sptr<IRemoteObject> &token));
    MOCK_METHOD1(ScheduleCommandAbilityDone, int(const sptr<IRemoteObject> &token));
    virtual void DumpState(const std::string &args, std::vector<std::string> &state) override;
    MOCK_METHOD2(TerminateAbilityResult, int(const sptr<IRemoteObject> &token, int startId));
    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE) override;
    MOCK_METHOD2(TerminateAbilityByCaller, int(const sptr<IRemoteObject> &callerToken, int requestCode));
    MOCK_METHOD1(MoveMissionToTop, int(int32_t missionId));
    MOCK_METHOD1(RemoveMission, int(int id));
    MOCK_METHOD1(RemoveStack, int(int id));
    MOCK_METHOD1(KillProcess, int(const std::string &bundleName));
    MOCK_METHOD2(UninstallApp, int(const std::string &bundleName, int32_t uid));
    MOCK_METHOD2(MoveMissionToEnd, int(const sptr<IRemoteObject> &token, const bool nonFirst));
    MOCK_METHOD1(IsFirstInMission, bool(const sptr<IRemoteObject> &token));
    MOCK_METHOD4(CompelVerifyPermission, int(const std::string &permission, int pid, int uid, std::string &message));
    MOCK_METHOD2(
        GetWantSender, sptr<IWantSender>(const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken));
    MOCK_METHOD2(SendWantSender, int(const sptr<IWantSender> &target, const SenderInfo &senderInfo));
    MOCK_METHOD1(CancelWantSender, void(const sptr<IWantSender> &sender));
    MOCK_METHOD1(GetPendingWantUid, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantUserId, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantBundleName, std::string(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantCode, int(const sptr<IWantSender> &target));
    MOCK_METHOD1(GetPendingWantType, int(const sptr<IWantSender> &target));
    MOCK_METHOD2(RegisterCancelListener, void(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver));
    MOCK_METHOD2(UnregisterCancelListener, void(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver));
    MOCK_METHOD2(GetPendingRequestWant, int(const sptr<IWantSender> &target, std::shared_ptr<Want> &want));
    MOCK_METHOD1(LockMission, int(int));
    MOCK_METHOD1(UnlockMission, int(int));
    MOCK_METHOD0(GetMissionLockModeState, int());
    MOCK_METHOD2(
        ChangeFocusAbility, int(const sptr<IRemoteObject> &lostFocusToken, const sptr<IRemoteObject> &getFocusToken));
    MOCK_METHOD1(MinimizeMultiWindow, int(int missionId));
    MOCK_METHOD1(MaximizeMultiWindow, int(int missionId));
    MOCK_METHOD1(CloseMultiWindow, int(int missionId));
    MOCK_METHOD1(GetPendinTerminateAbilityTestgRequestWant, void(int id));
    MOCK_METHOD1(SetShowOnLockScreen, int(bool isAllow));
    MOCK_METHOD3(StartContinuation, int(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status));
    MOCK_METHOD2(NotifyContinuationResult, int(int32_t missionId, int32_t result));
    MOCK_METHOD5(ContinueMission, int(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams));
    MOCK_METHOD3(ContinueAbility, int(const std::string &deviceId, int32_t missionId, uint32_t versionCode));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::string &deviceId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD1(LockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(UnlockMissionForCleanup, int(int32_t missionId));
    MOCK_METHOD1(RegisterMissionListener, int(const sptr<IMissionListener> &listener));
    MOCK_METHOD1(UnRegisterMissionListener, int(const sptr<IMissionListener> &listener));
    MOCK_METHOD3(
        GetMissionInfos, int(const std::string& deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos));
    MOCK_METHOD3(GetMissionInfo, int(const std::string& deviceId, int32_t missionId, MissionInfo &missionInfo));
    MOCK_METHOD1(CleanMission, int(int32_t missionId));
    MOCK_METHOD0(CleanAllMissions, int());
    MOCK_METHOD1(MoveMissionToFront, int(int32_t missionId));
    MOCK_METHOD2(MoveMissionToFront, int(int32_t missionId, const StartOptions &startOptions));
    MOCK_METHOD1(ClearUpApplicationData, int(const std::string &));
    MOCK_METHOD1(GetAbilityRunningInfos, int(std::vector<AbilityRunningInfo> &info));
    MOCK_METHOD2(GetExtensionRunningInfos, int(int upperLimit, std::vector<ExtensionRunningInfo> &info));
    MOCK_METHOD1(GetProcessRunningInfos, int(std::vector<AppExecFwk::RunningProcessInfo> &info));
    MOCK_METHOD2(GetWantSenderInfo, int(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info));
    virtual int StartUser(int userId) override;
    virtual int StopUser(int userId, const sptr<IStopUserCallback> &callback) override;
    virtual int StartSyncRemoteMissions(const std::string& devId, bool fixConflict, int64_t tag) override;
    virtual int StopSyncRemoteMissions(const std::string& devId) override;
    virtual int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;
    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override;
    virtual int StartAbilityByCall(
        const Want &want, const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken) override;
    virtual int ReleaseAbility(const sptr<IAbilityConnection> &connect,
        const AppExecFwk::ElementName &element) override;
    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;
    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject>& token) override;
    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) override;
    virtual int SendANRProcessID(int pid) override;
    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override;
    virtual bool IsRunningInStabilityTest() override;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& info, bool isClient, bool isUserID, int UserID) override;
    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override;
    virtual int FinishUserTest(
        const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override;
    virtual int GetTopAbility(sptr<IRemoteObject> &token) override;
    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override;
    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override;
    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag);
    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag);
    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token);
    virtual int BlockAmsService();
    virtual int BlockAbility(int32_t abilityRecordId);
    virtual int BlockAppService();
#ifdef ABILITY_COMMAND_FOR_TEST
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override;
#endif
#ifdef SUPPORT_GRAPHICS
    MOCK_METHOD2(SetMissionLabel, int(const sptr<IRemoteObject> &token, const std::string &label));
    virtual int SetMissionIcon(
        const sptr<IRemoteObject> &token, const std::shared_ptr<OHOS::Media::PixelMap> &icon) override;
    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler);
    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override {}
#endif
    virtual int NotifyProcessWillBeKilled(const std::string &bundleName) override;

public:
    std::string powerState_;
    static bool finishFlag_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_MOCK_ABILITY_DELEGATOR_STUB_H
