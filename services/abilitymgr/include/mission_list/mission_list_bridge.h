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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_BRIDGE_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_BRIDGE_H

#include <memory>

#include "ability_record.h"
#include "ability_running_info.h"
#include "mission_list_base.h"
#ifdef SUPPORT_SCREEN
#include "window_info.h"
#endif

namespace OHOS {
namespace AAFwk {

class MissionListBridge {
public:
    virtual ~MissionListBridge() = default;

    // param not const &
    virtual int32_t StartAbility(AbilityRequest &abilityRequest, int32_t userId) = 0;

    virtual int32_t MoveAbilityToBackground(std::shared_ptr<AbilityRecord> abilityRecord, int32_t userId) = 0;

    // is this ths same as MoveAbilityToBackground, or this could be moved to client
    virtual int32_t MinimizeAbility(sptr<IRemoteObject> token, bool fromUser, int32_t userId) = 0;

    virtual int32_t TerminateAbility(std::shared_ptr<AbilityRecord> abilityRecord, int32_t resultCode,
        const Want *resultWant, bool flag, int32_t userId) = 0;
#ifdef SUPPORT_SCREEN
    virtual sptr<IRemoteObject> GetFocusWindow() = 0;
    virtual void CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken, int32_t userId) = 0;

    virtual std::string GetLabel(std::string loadPath, int32_t labelId, int32_t abilityId) = 0;
    virtual void ReleaseAbilityBgInfo(int32_t abilityId) = 0;
    virtual void StartingWindowHot(sptr<AbilityTransitionInfo> info, int32_t missionId) = 0;
    virtual void StartingWindowCold(sptr<AbilityTransitionInfo> info, uint32_t bgImageId, uint32_t bgColorId,
        int32_t abilityId, const std::string &loadPath) = 0;
    virtual void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo,
        bool& animaEnabled) = 0;
    virtual void CancelStartingWindow(sptr<IRemoteObject> token) = 0;
    virtual void SetMissionAbilityState(int32_t missionId, AbilityState state) = 0;
    virtual void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info) = 0;
    virtual void RegisterFocusListener() = 0;
#endif
    virtual int32_t MoveMissionToFront(int32_t missionId) = 0;
    virtual int32_t MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions) = 0;

    // for GetInnerMissionInfoById
    virtual int32_t GetMissionWantById(int32_t missionId, Want &missionWant);

    virtual int32_t AttachAbilityThread(sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token,
        int32_t userId) = 0;

    virtual void DumpMissionList(const std::string &args, std::vector<std::string> &info,
        bool isClient, bool isUserID, int32_t userId) = 0;
    virtual void DumpSysAbilityInner(std::vector<std::string> &info, bool isClient, int32_t abilityId,
        const std::vector<std::string> &params, bool isUserID, int32_t userId) = 0;
    virtual void DumpInner(std::vector<std::string> &info) = 0;
    virtual void DumpMissionListInner(std::vector<std::string> &info) = 0;
    virtual void DumpMissionInfosInner(std::vector<std::string> &info) = 0;
    virtual void DumpMissionInner(int32_t missionId, std::vector<std::string> &info) = 0;
    virtual int32_t AbilityTransitionDone(sptr<IRemoteObject> token, int32_t state, const PacMap &saveData,
        int32_t userId) = 0;
    virtual void OnAbilityRequestDone(sptr<IRemoteObject> token, int32_t state, int32_t userId) = 0;
    virtual void OnAppStateChanged(const AppInfo &info) = 0;
    virtual bool OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId) = 0;
    virtual void OnCallConnectDied(std::shared_ptr<CallRecord> callRecord) = 0;
    virtual void HandleLoadTimeOut(int32_t abilityRecordId, bool isHalf) = 0;
    virtual void HandleActiveTimeOut(int32_t abilityRecordId) = 0;
    virtual void HandleInactiveTimeOut(int32_t abilityRecordId) = 0;
    virtual void HandleForegroundTimeOut(int32_t abilityRecordId, bool isHalf) = 0;
    virtual bool VerificationToken(sptr<IRemoteObject> token) = 0;
    virtual bool VerificationAllToken(sptr<IRemoteObject> token) = 0;
    virtual int32_t GetMissionIdByAbilityToken(sptr<IRemoteObject> token, int32_t userId) = 0;
    virtual sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId) = 0;
    virtual int32_t ResolveLocked(const AbilityRequest &abilityRequest, int32_t validUserId);
    virtual int32_t ReleaseCall(sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element) = 0;
    virtual void OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag) = 0;
    virtual void OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want) = 0;
    virtual int32_t GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) = 0;
    virtual void EnableRecoverAbility(int32_t missionId, int32_t userId) = 0;
    virtual void PauseOldMissionListManager(int32_t userId) = 0;
    virtual void StartUserApps() = 0;
    virtual int32_t DoAbilityForeground(std::shared_ptr<AbilityRecord> abilityRecord, uint32_t flag) = 0;
#ifdef ABILITY_COMMAND_FOR_TEST
    virtual int32_t BlockAbility(int32_t abilityRecordId) = 0;
#endif
    virtual std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int32_t missionId) = 0;
    virtual void CallRequestDone(std::shared_ptr<AbilityRecord> abilityRecord, sptr<IRemoteObject> callStub) = 0;
    virtual bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> targetRecord,
        int32_t userId) = 0;
    virtual int32_t SignRestartAppFlag(const std::string &bundleName, int32_t userId) = 0;

    virtual void GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityLists,
        int32_t pid, ActiveAbilityMode mode, int32_t userId);

    virtual void InitMissionListManager(int32_t userId, bool switchUser) = 0;
    virtual void UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid) = 0;
    virtual void RemoveUser(int32_t userId) = 0;
    virtual sptr<IRemoteObject> GetMissionListDelegator() = 0;
    virtual int32_t GetTopAbility(sptr<IRemoteObject> &token) = 0;
    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId) = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MISSION_LIST_BRIDGE_H
