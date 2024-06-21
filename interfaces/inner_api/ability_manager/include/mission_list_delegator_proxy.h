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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_LIST_DELEGATOR_PROXY_H
#define OHOS_ABILITY_RUNTIME_MISSION_LIST_DELEGATOR_PROXY_H

#include "mission_list_delegator_interface.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
class MissionListDelegatorProxy : public IRemoteProxy<IMissionListDelegator> {
public:
    explicit MissionListDelegatorProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IMissionListDelegator>(impl) {}

    ~MissionListDelegatorProxy() = default;

    // the real is SetMissionLockedState
    int32_t LockMissionForCleanup(int32_t missionId) override;
    int32_t UnlockMissionForCleanup(int32_t missionId) override;

    int32_t RegisterMissionListener(sptr<IMissionListener> listener) override;
    int32_t UnRegisterMissionListener(sptr<IMissionListener> listener) override;

    // this need dms client
    int32_t GetMissionInfos(const std::string& deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override;
    int32_t GetMissionInfo(const std::string& deviceId, int32_t missionId,
        MissionInfo &missionInfo) override;

    // need controller
    int32_t CleanMission(int32_t missionId) override;
    int32_t CleanAllMissions() override;

    int32_t RegisterSnapshotHandler(sptr<ISnapshotHandler> handler) override;
#ifdef SUPPORT_SCREEN
    int32_t RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler) override;
    void UpdateMissionSnapShot(sptr<IRemoteObject> token, std::shared_ptr<Media::PixelMap> pixelMap) override;
    int32_t SetMissionLabel(sptr<IRemoteObject> token, const std::string& label) override;
    int32_t SetMissionIcon(sptr<IRemoteObject> abilityToken, std::shared_ptr<Media::PixelMap> icon) override;
#endif
    int32_t MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId) override;
    int32_t MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
        std::vector<int32_t>& result) override;
    int32_t GetTopAbility(sptr<IRemoteObject> &token) override;
    AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId) override;

    int32_t GetMissionIdByToken(sptr<IRemoteObject> token) override;
    int32_t MoveAbilityToBackground(sptr<IRemoteObject> token) override;
    int32_t GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) override;

    int32_t DelegatorDoAbilityForeground(sptr<IRemoteObject> token) override;
    int32_t DelegatorDoAbilityBackground(sptr<IRemoteObject> token) override;

    int32_t SetMissionContinueState(sptr<IRemoteObject> token, ContinueState state) override;
    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results) override;
private:
    bool WriteInterfaceToken(MessageParcel &data);
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);

private:
    static inline BrokerDelegator<MissionListDelegatorProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_MISSION_LIST_DELEGATOR_PROXY_H