
/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_AAFWK_SERVICES_MOCK_DISTRIBUTED_SCHED_SERVICE_H
#define FOUNDATION_AAFWK_SERVICES_MOCK_DISTRIBUTED_SCHED_SERVICE_H

#include "distributed_sched_interface.h"
#include "iremote_proxy.h"
#include "mission_info.h"
#include "mission/distributed_mission_info.h"

namespace OHOS {
namespace DistributedSchedule {

class DistributedSchedProxy : public IRemoteProxy<IDistributedSched> {
public:
    explicit DistributedSchedProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDistributedSched>(impl) {}
    ~DistributedSchedProxy() {}
    MOCK_METHOD4(StartRemoteAbility, int(const OHOS::AAFwk::Want& want, int32_t callerUid,
        int32_t requestCode, uint32_t accessToken));
    MOCK_METHOD5(StartAbilityFromRemote, int(const OHOS::AAFwk::Want& want,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo));
    MOCK_METHOD5(ContinueMission, int(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams));
    MOCK_METHOD5(StartContinuation, int(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status, uint32_t accessToken));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::u16string& devId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD2(NotifyContinuationResultFromRemote, int(int32_t sessionId, bool isSuccess));
    MOCK_METHOD5(ConnectRemoteAbility, int(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken));
    MOCK_METHOD1(DisconnectRemoteAbility, int(const sptr<IRemoteObject>& connect));
    MOCK_METHOD5(ConnectAbilityFromRemote, int(const OHOS::AAFwk::Want& want,
        const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo));
    MOCK_METHOD3(DisconnectAbilityFromRemote, int(const sptr<IRemoteObject>& connect,
        int32_t uid, const std::string& sourceDeviceId));
    MOCK_METHOD1(NotifyProcessDiedFromRemote, int(const CallerInfo& callerInfo));
    MOCK_METHOD3(StartSyncRemoteMissions, int(const std::string& devId, bool fixConflict, int64_t tag));
    MOCK_METHOD2(StartSyncMissionsFromRemote, int(const CallerInfo& callerInfo,
        std::vector<DstbMissionInfo>& missionInfos));
    MOCK_METHOD1(StopSyncRemoteMissions, int(const std::string& devId));
    MOCK_METHOD1(StopSyncMissionsFromRemote, int(const CallerInfo& callerInfo));
    MOCK_METHOD2(RegisterMissionListener, int(const std::u16string& devId, const sptr<IRemoteObject>& obj));
    MOCK_METHOD2(UnRegisterMissionListener, int(const std::u16string& devId, const sptr<IRemoteObject>& obj));
    MOCK_METHOD3(GetMissionInfos, int(const std::string& deviceId, int32_t numMissions,
        std::vector<AAFwk::MissionInfo>& missionInfos));
    MOCK_METHOD4(StoreSnapshotInfo, int(const std::string& deviceId, int32_t missionId,
        const uint8_t* byteStream, size_t len));
    MOCK_METHOD2(RemoveSnapshotInfo, int(const std::string& deviceId, int32_t missionId));
    MOCK_METHOD2(NotifyMissionsChangedFromRemote, int(const std::vector<DstbMissionInfo>& missionInfos,
        const CallerInfo& callerInfo));
    MOCK_METHOD1(CheckSupportOsd, int(const std::string& deviceId));
    MOCK_METHOD2(GetCachedOsdSwitch, void(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values));
    MOCK_METHOD0(GetOsdSwitchValueFromRemote, int());
    MOCK_METHOD2(UpdateOsdSwitchValueFromRemote, int(int32_t switchVal, const std::string& sourceDeviceId));
    
    MOCK_METHOD3(GetRemoteMissionSnapshotInfo, int(const std::string& networkId, int32_t missionId,
        std::unique_ptr<AAFwk::MissionSnapshot>& missionSnapshot));
    MOCK_METHOD5(StartRemoteAbilityByCall, int(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken));
    MOCK_METHOD2(ReleaseRemoteAbility, int(const sptr<IRemoteObject>& connect,
        const AppExecFwk::ElementName &element));
    MOCK_METHOD4(StartAbilityByCallFromRemote, int(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo));
    MOCK_METHOD3(ReleaseAbilityFromRemote, int(const sptr<IRemoteObject>& connect,
        const AppExecFwk::ElementName &element, const CallerInfo& callerInfo));
private:
    bool CallerInfoMarshalling(const CallerInfo& callerInfo, MessageParcel& data) { return true; }
};

class DistributedSchedStub : public IRemoteStub<IDistributedSched> {
public:
    DistributedSchedStub() {}
    ~DistributedSchedStub() {}
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }
};

class MockDistributedSchedService : public DistributedSchedStub {
public:
    MOCK_METHOD4(StartRemoteAbility, int(const OHOS::AAFwk::Want& want, int32_t callerUid,
        int32_t requestCode, uint32_t accessToken));
    MOCK_METHOD5(StartAbilityFromRemote, int(const OHOS::AAFwk::Want& want,
        const OHOS::AppExecFwk::AbilityInfo& abilityInfo, int32_t requestCode,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo));
    MOCK_METHOD5(ContinueMission, int(const std::string& srcDeviceId, const std::string& dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject>& callback, const OHOS::AAFwk::WantParams& wantParams));
    MOCK_METHOD5(StartContinuation, int(const OHOS::AAFwk::Want& want, int32_t missionId, int32_t callerUid,
        int32_t status, uint32_t accessToken));
    MOCK_METHOD3(NotifyCompleteContinuation, void(const std::u16string& devId, int32_t sessionId, bool isSuccess));
    MOCK_METHOD2(NotifyContinuationResultFromRemote, int(int32_t sessionId, bool isSuccess));
    MOCK_METHOD5(ConnectRemoteAbility, int(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken));
    MOCK_METHOD1(DisconnectRemoteAbility, int(const sptr<IRemoteObject>& connect));
    MOCK_METHOD5(ConnectAbilityFromRemote, int(const OHOS::AAFwk::Want& want,
        const AppExecFwk::AbilityInfo& abilityInfo, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo));
    MOCK_METHOD3(DisconnectAbilityFromRemote, int(const sptr<IRemoteObject>& connect,
        int32_t uid, const std::string& sourceDeviceId));
    MOCK_METHOD1(NotifyProcessDiedFromRemote, int(const CallerInfo& callerInfo));
    MOCK_METHOD3(StartSyncRemoteMissions, int(const std::string& devId, bool fixConflict, int64_t tag));
    MOCK_METHOD2(StartSyncMissionsFromRemote, int(const CallerInfo& callerInfo,
        std::vector<DstbMissionInfo>& missionInfos));
    MOCK_METHOD1(StopSyncRemoteMissions, int(const std::string& devId));
    MOCK_METHOD1(StopSyncMissionsFromRemote, int(const CallerInfo& callerInfo));
    MOCK_METHOD2(RegisterMissionListener, int(const std::u16string& devId, const sptr<IRemoteObject>& obj));
    MOCK_METHOD2(UnRegisterMissionListener, int(const std::u16string& devId, const sptr<IRemoteObject>& obj));
    MOCK_METHOD3(GetMissionInfos, int(const std::string& deviceId, int32_t numMissions,
        std::vector<AAFwk::MissionInfo>& missionInfos));
    MOCK_METHOD4(StoreSnapshotInfo, int(const std::string& deviceId, int32_t missionId,
        const uint8_t* byteStream, size_t len));
    MOCK_METHOD2(RemoveSnapshotInfo, int(const std::string& deviceId, int32_t missionId));
    MOCK_METHOD2(NotifyMissionsChangedFromRemote, int(const std::vector<DstbMissionInfo>& missionInfos,
        const CallerInfo& callerInfo));
    MOCK_METHOD1(CheckSupportOsd, int(const std::string& deviceId));
    MOCK_METHOD2(GetCachedOsdSwitch, void(std::vector<std::u16string>& deviceIds, std::vector<int32_t>& values));
    MOCK_METHOD0(GetOsdSwitchValueFromRemote, int());
    MOCK_METHOD2(UpdateOsdSwitchValueFromRemote, int(int32_t switchVal, const std::string& sourceDeviceId));
    
    MOCK_METHOD3(GetRemoteMissionSnapshotInfo, int(const std::string& networkId, int32_t missionId,
        std::unique_ptr<AAFwk::MissionSnapshot>& missionSnapshot));
    MOCK_METHOD5(StartRemoteAbilityByCall, int(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        int32_t callerUid, int32_t callerPid, uint32_t accessToken));
    MOCK_METHOD2(ReleaseRemoteAbility, int(const sptr<IRemoteObject>& connect,
        const AppExecFwk::ElementName &element));
    MOCK_METHOD4(StartAbilityByCallFromRemote, int(const OHOS::AAFwk::Want& want, const sptr<IRemoteObject>& connect,
        const CallerInfo& callerInfo, const AccountInfo& accountInfo));
    MOCK_METHOD3(ReleaseAbilityFromRemote, int(const sptr<IRemoteObject>& connect,
        const AppExecFwk::ElementName &element, const CallerInfo& callerInfo));
};
}// namespace DistributedSchedule
}// namespace OHOS
#endif // FOUNDATION_AAFWK_SERVICES_MOCK_DISTRIBUTED_SCHED_SERVICE_H
