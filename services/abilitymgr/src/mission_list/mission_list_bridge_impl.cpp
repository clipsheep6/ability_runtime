/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mission_list_bridge.h"

#include <mutex>
#include "hilog_tag_wrapper.h"
#include "mission_list_wrapper.h"
#include "mission_list_delegator_stub.h"

namespace OHOS {
namespace AAFwk {
namespace {
std::mutex g_missionListDelegatorMUtex_;
sptr<IMissionListDelegator> g_missionListDelegator;
}

class MissionListBridgeImpl : public MissionListBridge {
public:
    ~MissionListBridgeImpl() = default;

    // param not const &
    int32_t StartAbility(AbilityRequest &abilityRequest, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().StartAbility(abilityRequest, userId);
    }

    int32_t MoveAbilityToBackground(std::shared_ptr<AbilityRecord> abilityRecord, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().MoveAbilityToBackground(abilityRecord, userId);
    }

    // is this ths same as MoveAbilityToBackground, or this could be moved to client
    int32_t MinimizeAbility(sptr<IRemoteObject> token, bool fromUser, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().MinimizeAbility(token, fromUser, userId);
    }

    int32_t TerminateAbility(std::shared_ptr<AbilityRecord> abilityRecord, int32_t resultCode,
        const Want *resultWant, bool flag, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().TerminateAbility(abilityRecord, resultCode,
            resultWant, flag, userId);
    }
#ifdef SUPPORT_SCREEN
    sptr<IRemoteObject> GetFocusWindow() override
    {
        return MissionListWrapper::GetInstance().GetFocusWindow();
    }
    void CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken, int32_t userId) override
    {
        MissionListWrapper::GetInstance().CompleteFirstFrameDrawing(abilityToken, userId);
    }
    std::string GetLabel(std::string loadPath, int32_t labelId, int32_t abilityId) override
    {
        return MissionListWrapper::GetInstance().GetLabel(loadPath, labelId, abilityId);
    }
    void ReleaseAbilityBgInfo(int32_t abilityId) override
    {
        MissionListWrapper::GetInstance().ReleaseAbilityBgInfo(abilityId);
    }
    void StartingWindowHot(sptr<AbilityTransitionInfo> info, int32_t missionId) override
    {
        MissionListWrapper::GetInstance().StartingWindowHot(info, missionId);
    }
    void StartingWindowCold(sptr<AbilityTransitionInfo> info, uint32_t bgImageId, uint32_t bgColorId,
        int32_t abilityId, const std::string &loadPath) override
    {
        MissionListWrapper::GetInstance().StartingWindowCold(info, bgImageId, bgColorId, abilityId, loadPath);
    }
    void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo,
        bool& animaEnabled) override
    {
        MissionListWrapper::GetInstance().NotifyWindowTransition(fromInfo, toInfo, animaEnabled);
    }
    void CancelStartingWindow(sptr<IRemoteObject> token) override
    {
        MissionListWrapper::GetInstance().CancelStartingWindow(token);
    }
    void SetMissionAbilityState(int32_t missionId, AbilityState state) override
    {
        MissionListWrapper::GetInstance().SetMissionAbilityState(missionId, state);
    }
    void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info) override
    {
        MissionListWrapper::GetInstance().NotifyAnimationAbilityDied(info);
    }
    void RegisterFocusListener() override
    {
        MissionListWrapper::GetInstance().RegisterFocusListener();
    }
#endif
    int32_t MoveMissionToFront(int32_t missionId) override
    {
        return MissionListWrapper::GetInstance().MoveMissionToFront(missionId);
    }

    int32_t MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions) override
    {
        return MissionListWrapper::GetInstance().MoveMissionToFront(missionId, startOptions);
    }

    // for GetInnerMissionInfoById
    int32_t GetMissionWantById(int32_t missionId, Want &missionWant) override
    {
        return MissionListWrapper::GetInstance().GetMissionWantById(missionId, missionWant);
    }

    int32_t AttachAbilityThread(sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().AttachAbilityThread(scheduler, token, userId);
    }

    void DumpMissionList(const std::string &args, std::vector<std::string> &info,
        bool isClient, bool isUserID, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().DumpMissionList(args, info, isClient, isUserID, userId);
    }
    void DumpSysAbilityInner(std::vector<std::string> &info, bool isClient, int32_t abilityId,
        const std::vector<std::string> &params, bool isUserID, int32_t userId) override
    {
        MissionListWrapper::GetInstance().DumpSysAbilityInner(info, isClient, abilityId, params, isUserID, userId);
    }
    void DumpInner(std::vector<std::string> &info) override
    {
        MissionListWrapper::GetInstance().DumpInner(info);
    }
    void DumpMissionListInner(std::vector<std::string> &info) override
    {
        MissionListWrapper::GetInstance().DumpMissionListInner(info);
    }
    void DumpMissionInfosInner(std::vector<std::string> &info) override
    {
        MissionListWrapper::GetInstance().DumpMissionInfosInner(info);
    }
    void DumpMissionInner(int32_t missionId, std::vector<std::string> &info) override
    {
        MissionListWrapper::GetInstance().DumpMissionInner(missionId, info);
    }
    int32_t AbilityTransitionDone(sptr<IRemoteObject> token, int32_t state, const PacMap &saveData,
        int32_t userId) override
    {
        return MissionListWrapper::GetInstance().AbilityTransitionDone(token, state, saveData, userId);
    }
    void OnAbilityRequestDone(sptr<IRemoteObject> token, int32_t state, int32_t userId) override
    {
        MissionListWrapper::GetInstance().OnAbilityRequestDone(token, state, userId);
    }
    void OnAppStateChanged(const AppInfo &info) override
    {
        MissionListWrapper::GetInstance().OnAppStateChanged(info);
    }
    bool OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId) override
    {
        return MissionListWrapper::GetInstance().OnAbilityDied(abilityRecord, currentUserId);
    }
    void OnCallConnectDied(std::shared_ptr<CallRecord> callRecord) override
    {
        MissionListWrapper::GetInstance().OnCallConnectDied(callRecord);
    }
    void HandleLoadTimeOut(int32_t abilityRecordId, bool isHalf) override
    {
        MissionListWrapper::GetInstance().HandleLoadTimeOut(abilityRecordId, isHalf);
    }
    void HandleActiveTimeOut(int32_t abilityRecordId) override
    {
        MissionListWrapper::GetInstance().HandleActiveTimeOut(abilityRecordId);
    }
    void HandleInactiveTimeOut(int32_t abilityRecordId) override
    {
        MissionListWrapper::GetInstance().HandleInactiveTimeOut(abilityRecordId);
    }
    void HandleForegroundTimeOut(int32_t abilityRecordId, bool isHalf) override
    {
        MissionListWrapper::GetInstance().HandleForegroundTimeOut(abilityRecordId, isHalf);
    }
    bool VerificationToken(sptr<IRemoteObject> token) override
    {
        return MissionListWrapper::GetInstance().VerificationToken(token);
    }
    bool VerificationAllToken(sptr<IRemoteObject> token) override
    {
        return MissionListWrapper::GetInstance().VerificationAllToken(token);
    }
    int32_t GetMissionIdByAbilityToken(sptr<IRemoteObject> token, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().GetMissionIdByAbilityToken(token, userId);
    }
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId) override
    {
        return MissionListWrapper::GetInstance().GetAbilityTokenByMissionId(missionId);
    }
    int32_t ResolveLocked(const AbilityRequest &abilityRequest, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().ResolveLocked(abilityRequest, userId);
    }
    int32_t ReleaseCall(sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element) override
    {
        return MissionListWrapper::GetInstance().ReleaseCall(connect, element);
    }
    void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) override
    {
        MissionListWrapper::GetInstance().OnAcceptWantResponse(want, flag);
    }
    void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want) override
    {
        MissionListWrapper::GetInstance().OnStartSpecifiedAbilityTimeoutResponse(want);
    }
    int32_t GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) override
    {
        return MissionListWrapper::GetInstance().GetAbilityRunningInfos(info, isPerm);
    }
    void EnableRecoverAbility(int32_t missionId, int32_t userId) override
    {
        MissionListWrapper::GetInstance().EnableRecoverAbility(missionId, userId);
    }
    void PauseOldMissionListManager(int32_t userId) override
    {
        MissionListWrapper::GetInstance().PauseOldMissionListManager(userId);
    }
    void StartUserApps() override
    {
        MissionListWrapper::GetInstance().StartUserApps();
    }
    int32_t DoAbilityForeground(std::shared_ptr<AbilityRecord> abilityRecord, uint32_t flag) override
    {
        return MissionListWrapper::GetInstance().DoAbilityForeground(abilityRecord, flag);
    }
#ifdef ABILITY_COMMAND_FOR_TEST
    int32_t BlockAbility(int32_t abilityRecordId) override
    {
        return MissionListWrapper::GetInstance().BlockAbility(abilityRecordId);
    }
#endif
    std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int32_t missionId) override
    {
        return MissionListWrapper::GetInstance().GetAbilityRecordByMissionId(missionId);
    }
    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> targetRecord,
        int32_t userId) override
    {
        return MissionListWrapper::GetInstance().IsAbilityStarted(abilityRequest, targetRecord, userId);
    }
    void CallRequestDone(std::shared_ptr<AbilityRecord> abilityRecord, sptr<IRemoteObject> callStub) override
    {
        MissionListWrapper::GetInstance().CallRequestDone(abilityRecord, callStub);
    }
    int32_t SignRestartAppFlag(const std::string &bundleName, int32_t userId) override
    {
        return MissionListWrapper::GetInstance().SignRestartAppFlag(bundleName, userId);
    }

    void GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityLists,
        int32_t pid, ActiveAbilityMode mode, int32_t userId) override
    {
        MissionListWrapper::GetInstance().GetActiveAbilityList(bundleName, abilityLists, pid, mode, userId);
    }

    void InitMissionListManager(int32_t userId, bool switchUser) override
    {
        MissionListWrapper::GetInstance().InitMissionListManager(userId, switchUser);
    }

    void UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid) override
    {
        MissionListWrapper::GetInstance().UninstallAppInMissionListManagers(userId, bundleName, uid);
    }

    void RemoveUser(int32_t userId) override
    {
        MissionListWrapper::GetInstance().RemoveUser(userId);
    }

    sptr<IRemoteObject> GetMissionListDelegator() override
    {
        std::lock_guard lock(g_missionListDelegatorMUtex_);
        if (g_missionListDelegator == nullptr) {
            g_missionListDelegator = sptr<IMissionListDelegator>(new MissionListDelegatorStub);
        }
        return g_missionListDelegator->AsObject();
    }

    int32_t GetTopAbility(sptr<IRemoteObject> &token) override
    {
        return MissionListWrapper::GetInstance().GetTopAbility(token);
    }

    AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId) override
    {
        return MissionListWrapper::GetInstance().GetTopAbility(isNeedLocalDeviceId);
    }
};
} // namespace AAFwk
} // namespace OHOS


extern "C" __attribute__((visibility("default"))) OHOS::AAFwk::MissionListBridge* CreateMissionListBridge()
{
    return new OHOS::AAFwk::MissionListBridgeImpl();
}