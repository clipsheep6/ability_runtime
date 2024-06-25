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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_WRAPPER_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_WRAPPER_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "ability_record.h"
#include "ability_running_info.h"
#include "ability_state.h"
#include "mission_info.h"
#include "mission_list.h"
#include "mission_list_base.h"
#include "mission_list_manager.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "start_options.h"
#include "snapshot.h"

#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#include "resource_manager.h"
#include "window_info.h"
#include "window_manager_service_handler.h"
#endif

namespace OHOS {
namespace AAFwk {
class WindowFocusChangedListener;
using MissionListMap = std::unordered_map<int32_t, std::shared_ptr<MissionListManager>>;
#ifdef SUPPORT_SCREEN
struct AbilityBgInfo {
    uint32_t bgColor = 0;
    std::shared_ptr<Media::PixelMap> startingWindowBg;
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr;
};
#endif
class MissionListWrapper {
public:
    static MissionListWrapper &GetInstance();
    MissionListWrapper() = default;
    ~MissionListWrapper();
    MissionListWrapper(MissionListWrapper &) = delete;
    void operator=(MissionListWrapper &) = delete;

    int32_t StartAbility(AbilityRequest &abilityRequest, int32_t userId);

    int32_t MoveAbilityToBackground(std::shared_ptr<AbilityRecord> abilityRecord, int32_t userId);

    // is this ths same as MoveAbilityToBackground, or this could be moved to client
    int32_t MinimizeAbility(sptr<IRemoteObject> token, bool fromUser, int32_t userId);

    int32_t TerminateAbility(std::shared_ptr<AbilityRecord> abilityRecord, int32_t resultCode,
        const Want *resultWant, bool flag, int32_t userId);

#ifdef SUPPORT_SCREEN
    int32_t RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler);
    sptr<IWindowManagerServiceHandler> GetWmsHandler();
    sptr<IRemoteObject> GetFocusWindow();
    void CompleteFirstFrameDrawing(sptr<IRemoteObject> abilityToken, int32_t userId);
    void UpdateMissionSnapShot(sptr<IRemoteObject> token, std::shared_ptr<Media::PixelMap> pixelMap);
    int32_t SetMissionLabel(sptr<IRemoteObject> token, const std::string &label);
    int32_t SetMissionIcon(sptr<IRemoteObject> abilityToken, std::shared_ptr<Media::PixelMap> icon);
    std::string GetLabel(std::string loadPath, int32_t labelId, int32_t abilityId);
    void ReleaseAbilityBgInfo(int32_t abilityId);
    void StartingWindowHot(sptr<AbilityTransitionInfo> info, int32_t missionId);
    void StartingWindowCold(sptr<AbilityTransitionInfo> info, uint32_t bgImageId, uint32_t bgColorId,
        int32_t abilityId, const std::string &loadPath);
    void NotifyWindowTransition(sptr<AbilityTransitionInfo> fromInfo, sptr<AbilityTransitionInfo> toInfo,
        bool& animaEnabled);
    void CancelStartingWindow(sptr<IRemoteObject> token);
    void SetMissionAbilityState(int32_t missionId, AbilityState state);
    void NotifyAnimationAbilityDied(sptr<AbilityTransitionInfo> info);
    void RegisterFocusListener();
    void HandleFocusChange(const FocusStateInfo &focusChangeInfo);
#endif

    int32_t MoveMissionToFront(int32_t missionId);
    int32_t MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions);

    // for GetInnerMissionInfoById
    int32_t GetMissionWantById(int32_t missionId, Want &missionWant);

    int32_t AttachAbilityThread(sptr<IAbilityScheduler> scheduler, sptr<IRemoteObject> token, int32_t userId);

    void DumpMissionList(const std::string &args, std::vector<std::string> &info,
        bool isClient, bool isUserID, int32_t userId);
    void DumpSysAbilityInner(std::vector<std::string> &info, bool isClient, int32_t abilityId,
        const std::vector<std::string> &params, bool isUserID, int32_t userId);
    void DumpInner(std::vector<std::string> &info);
    void DumpMissionListInner(std::vector<std::string> &info);
    void DumpMissionInfosInner(std::vector<std::string> &info);
    void DumpMissionInner(int32_t missionId, std::vector<std::string> &info);
    int32_t AbilityTransitionDone(sptr<IRemoteObject> token, int32_t state, const PacMap &saveData, int32_t userId);
    void OnAbilityRequestDone(sptr<IRemoteObject> token, int32_t state, int32_t userId);
    void OnAppStateChanged(const AppInfo &info);
    bool OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId);
    void OnCallConnectDied(std::shared_ptr<CallRecord> callRecord);
    void HandleLoadTimeOut(int32_t abilityRecordId, bool isHalf);
    void HandleActiveTimeOut(int32_t abilityRecordId);
    void HandleInactiveTimeOut(int32_t abilityRecordId);
    void HandleForegroundTimeOut(int32_t abilityRecordId, bool isHalf);
    bool VerificationToken(sptr<IRemoteObject> token);
    bool VerificationAllToken(sptr<IRemoteObject> token);
    int32_t GetMissionIdByAbilityToken(sptr<IRemoteObject> token, int32_t userId);
    sptr<IRemoteObject> GetAbilityTokenByMissionId(int32_t missionId);
    int32_t ResolveLocked(const AbilityRequest &abilityRequest, int32_t userId);
    int32_t ReleaseCall(sptr<IAbilityConnection> connect, const AppExecFwk::ElementName &element);
    void OnAcceptWantResponse(const Want &want, const std::string &flag);
    void OnStartSpecifiedAbilityTimeoutResponse(const Want &want);
    int32_t GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm);
    void EnableRecoverAbility(int32_t missionId, int32_t userId);
    void PauseOldMissionListManager(int32_t userId);
    void StartUserApps();
    int32_t DoAbilityForeground(std::shared_ptr<AbilityRecord> abilityRecord, uint32_t flag);
#ifdef ABILITY_COMMAND_FOR_TEST
    int32_t BlockAbility(int32_t abilityRecordId);
#endif
    std::shared_ptr<AbilityRecord> GetAbilityRecordByMissionId(int32_t missionId);
    bool IsAbilityStarted(AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> targetRecord,
        int32_t userId);
    void CallRequestDone(std::shared_ptr<AbilityRecord> abilityRecord, sptr<IRemoteObject> callStub);
    int32_t SignRestartAppFlag(const std::string &bundleName, int32_t userId);

    // delegator
    int32_t SetMissionLockedState(int32_t missionId, bool lockedState);
    int32_t RegisterMissionListener(sptr<IMissionListener> listener);
    int32_t UnRegisterMissionListener(sptr<IMissionListener> listener);
    int32_t GetMissionInfos(const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos);
    int32_t GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo);
    int32_t CleanMission(int32_t missionId);
    int32_t CleanAllMissions();
    int32_t MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId);
    int32_t MoveMissionsToBackground(const std::vector<int32_t> &missionIds, std::vector<int32_t> &result);
    int32_t GetTopAbility(sptr<IRemoteObject> &token);
    AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId);
    int32_t GetMissionIdByToken(sptr<IRemoteObject> token);
    int32_t RegisterSnapshotHandler(sptr<ISnapshotHandler> handler);
    int32_t MoveAbilityToBackground(sptr<IRemoteObject> token);
    int32_t GetMissionSnapshot(const std::string &deviceId, int32_t missionId,
        MissionSnapshot &snapshot, bool isLowResolution);
    int32_t DelegatorDoAbilityForeground(sptr<IRemoteObject> token);
    int32_t DelegatorDoAbilityBackground(sptr<IRemoteObject> token);
    int32_t SetMissionContinueState(sptr<IRemoteObject> token, ContinueState state);
    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results);

    void GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityLists,
        int32_t pid, ActiveAbilityMode mode, int32_t userId);

    void InitMissionListManager(int32_t userId, bool switchUser);
    void UninstallAppInMissionListManagers(int32_t userId, const std::string &bundleName, int32_t uid);
    void RemoveUser(int32_t userId);
    std::shared_ptr<MissionListManager> GetMissionListManagerByUserId(int32_t userId);

    std::shared_ptr<AbilityRecord> GetFocusAbility();

protected:
    std::shared_ptr<MissionListManager> GetCurrentMissionListManager();
    MissionListMap GetMissionListManagers();
    std::shared_ptr<MissionListManager> GetMissionListManagerByCalling();

    int32_t GetRemoteMissionInfos(const std::string &deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos);
    int32_t GetRemoteMissionInfo(const std::string &deviceId, int32_t missionId,
        MissionInfo &missionInfo);
private:
    std::mutex managersMutex_;
    MissionListMap missionListManagers_;
    int32_t currentUser_;
    std::shared_ptr<MissionListManager> currentMissionListManager_;
#ifdef SUPPORT_SCREEN
    std::mutex wmsHandlerMutex_;
    sptr<IWindowManagerServiceHandler> wmsHandler_;
    std::mutex abilityBgInfoMutex_;
    std::map<int32_t, std::shared_ptr<AbilityBgInfo>> abilityBgInfos_;
    std::mutex focusListenerMutex_;
    sptr<WindowFocusChangedListener> focusListener_;
#endif
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MISSION_LIST_WRAPPER_H
