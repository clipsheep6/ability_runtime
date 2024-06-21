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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_DELEGATOR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_DELEGATOR_INTERFACE_H

#include <memory>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"
#include "mission_info.h"
#include "mission_listener_interface.h"
#include "mission_snapshot.h"
#include "snapshot.h"

#ifdef SUPPORT_SCREEN
#include "pixel_map.h"
#include "window_manager_service_handler.h"
#endif

namespace OHOS {
namespace AAFwk {
class IMissionListDelegator : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.MissionListDelegator");

    virtual ~IMissionListDelegator() = default;

    // the real is SetMissionLockedState
    virtual int32_t LockMissionForCleanup(int32_t missionId) = 0;
    virtual int32_t UnlockMissionForCleanup(int32_t missionId) = 0;

    virtual int32_t RegisterMissionListener(sptr<IMissionListener> listener) = 0;
    virtual int32_t UnRegisterMissionListener(sptr<IMissionListener> listener) = 0;

    // this need dms client
    virtual int32_t GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) = 0;
    virtual int32_t GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) = 0;

    virtual int32_t CleanMission(int32_t missionId) = 0;
    virtual int32_t CleanAllMissions() = 0;

    virtual int32_t RegisterSnapshotHandler(sptr<ISnapshotHandler> handler) = 0;
#ifdef SUPPORT_SCREEN
    virtual int32_t RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler) = 0;
    virtual void UpdateMissionSnapShot(sptr<IRemoteObject> token,
        std::shared_ptr<Media::PixelMap> pixelMap) = 0;
    virtual int32_t SetMissionLabel(sptr<IRemoteObject> token, const std::string& label) = 0;
    virtual int32_t SetMissionIcon(sptr<IRemoteObject> abilityToken,
        std::shared_ptr<Media::PixelMap> icon) = 0;
#endif
    virtual int32_t MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) = 0;
    virtual int32_t MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result) = 0;
    virtual int32_t GetTopAbility(sptr<IRemoteObject> &token) = 0;
    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId) = 0;

    virtual int32_t GetMissionIdByToken(sptr<IRemoteObject> token) = 0;
    virtual int32_t MoveAbilityToBackground(sptr<IRemoteObject> token) = 0;
    virtual int32_t GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) = 0;

    virtual int32_t DelegatorDoAbilityForeground(sptr<IRemoteObject> token) = 0;
    virtual int32_t DelegatorDoAbilityBackground(sptr<IRemoteObject> token) = 0;

    virtual int32_t SetMissionContinueState(sptr<IRemoteObject> token, ContinueState state) = 0;
    virtual int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results) = 0;

    enum {
        LOCK_MISSION_FOR_CLEANUP = 0,
        UNLOCK_MISSION_FOR_CLEANUP,
        REGISTER_MISSION_LISTENER,
        UNREGISTER_MISSION_LISTENER,
        GET_MISSION_INFOS,
        GET_MISSION_INFO_BY_ID,
        CLEAN_MISSION,
        CLEAN_ALL_MISSIONS,
        REGISTER_SNAPSHOT_HANDLER,
        REGISTER_WMS_HANDLER,
        UPDATE_MISSION_SNAPSHOT_FROM_WMS,
        SET_MISSION_LABEL,
        SET_MISSION_ICON,
        MOVE_MISSIONS_TO_FOREGROUND,
        MOVE_MISSIONS_TO_BACKGROUND,
        GET_TOP_ABILITY_TOKEN,
        GET_TOP_ABILITY,
        GET_MISSION_ID_BY_ABILITY_TOKEN,
        MOVE_ABILITY_TO_BACKGROUND,
        GET_MISSION_SNAPSHOT_INFO,
        DELEGATOR_DO_ABILITY_FOREGROUND,
        DELEGATOR_DO_ABILITY_BACKGROUND,
        SET_MISSION_CONTINUE_STATE,
        QUERY_MISSION_VALID,
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MISSION_LIST_DELEGATOR_INTERFACE_H