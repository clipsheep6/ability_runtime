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

#include <gtest/gtest.h>
#include <iremote_stub.h>
#define private public
#define protected public
#include "ability_manager_errors.h"
#include "distributed_client.h"
#include "distributed_parcel_helper.h"
#include "iservice_registry.h"
#include "iremote_object.h"
#include "mock_ability_connect_callback.h"
#include "parcel.h"
#undef protected
#undef private

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace testing;
using namespace testing::ext;
class DistributedClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
void DistributedClientTest::SetUpTestCase()
{}
void DistributedClientTest::TearDownTestCase()
{}
void DistributedClientTest::SetUp()
{}
void DistributedClientTest::TearDown()
{}

/**
 * @tc.number: GetDmsProxy_0100
 * @tc.name: GetDmsProxy
 * @tc.desc: GetDmsProxy Test, return is not nullptr.
 */
HWTEST_F(DistributedClientTest, GetDmsProxy_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest GetDmsProxy_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    auto result = client->GetDmsProxy();
    if (result != nullptr) {
        EXPECT_NE(result, nullptr);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest GetDmsProxy_0100 end";
}

/**
 * @tc.number: StartRemoteAbility_0100
 * @tc.name: StartRemoteAbility
 * @tc.desc: StartRemoteAbility Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, StartRemoteAbility_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteAbility_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t callerUid = 5;
    uint32_t accessToken = 0;
    int32_t requestCode = 0;
    int32_t result = client->StartRemoteAbility(want , callerUid , accessToken , requestCode);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteAbility_0100 end";
}

/**
 * @tc.number: ConnectRemoteAbility_0100
 * @tc.name: ConnectRemoteAbility
 * @tc.desc: ConnectRemoteAbility Test, when connect is nullptr, return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, ConnectRemoteAbility_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ConnectRemoteAbility_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t result = client->ConnectRemoteAbility(want , nullptr);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest ConnectRemoteAbility_0100 end";
}

/**
 * @tc.number: ConnectRemoteAbility_0200
 * @tc.name: ConnectRemoteAbility
 * @tc.desc: ConnectRemoteAbility Test, when connect is not nullptr, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, ConnectRemoteAbility_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ConnectRemoteAbility_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    sptr<IRemoteObject> connect = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    int32_t result = client->ConnectRemoteAbility(want, connect);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest ConnectRemoteAbility_0200 end";
}

/**
 * @tc.number: DisconnectRemoteAbility_0100
 * @tc.name: DisconnectRemoteAbility
 * @tc.desc: DisconnectRemoteAbility Test, when connect is nullptr, return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, DisconnectRemoteAbility_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest DisconnectRemoteAbility_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    int32_t callerUid = 0;
    uint32_t accessToken = 0;
    OHOS::AAFwk::Want want;
    sptr<IRemoteObject> connect = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    client->ConnectRemoteAbility(want , connect);
    int32_t result = client->DisconnectRemoteAbility(nullptr , callerUid , accessToken);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest DisconnectRemoteAbility_0100 end";
}

/**
 * @tc.number: DisconnectRemoteAbility_0200
 * @tc.name: DisconnectRemoteAbility
 * @tc.desc: DisconnectRemoteAbility Test, when connect is not nullptr, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, DisconnectRemoteAbility_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest DisconnectRemoteAbility_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    int32_t callerUid = 0;
    uint32_t accessToken = 0;
    OHOS::AAFwk::Want want;
    sptr<IRemoteObject> connect = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    client->ConnectRemoteAbility(want , connect);
    int32_t result = client->DisconnectRemoteAbility(connect , callerUid , accessToken);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest DisconnectRemoteAbility_0200 end";
}

/**
 * @tc.number: ContinueMission_0100
 * @tc.name: ContinueMission
 * @tc.desc: ContinueMission Test, when callback is nullptr, return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, ContinueMission_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ContinueMission_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string srcDeviceId = "deviceId";
    std::string dstDeviceId = "deviceId";
    int32_t missionId = 0 ;
    OHOS::AAFwk::WantParams wantParams;
    int32_t result = client->ContinueMission(srcDeviceId , dstDeviceId , missionId , nullptr , wantParams);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest ContinueMission_0100 end";
}

/**
 * @tc.number: ContinueMission_0200
 * @tc.name: ContinueMission
 * @tc.desc: ContinueMission Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, ContinueMission_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ContinueMission_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string srcDeviceId = "deviceId";
    std::string dstDeviceId = "deviceId";
    int32_t missionId = 0 ;
    OHOS::AAFwk::WantParams wantParams;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    int32_t result = client->ContinueMission(srcDeviceId , dstDeviceId , missionId , callback , wantParams);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest ContinueMission_0200 end";
}

/**
 * @tc.number: NotifyCompleteContinuation_0100
 * @tc.name: NotifyCompleteContinuation
 * @tc.desc: NotifyCompleteContinuation Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, NotifyCompleteContinuation_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest NotifyCompleteContinuation_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::u16string devId = to_utf16("deviceId");
    int32_t sessionId = 0;
    bool isSuccess = true;
    auto result = client->NotifyCompleteContinuation(devId , sessionId , isSuccess);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest NotifyCompleteContinuation_0100 end";
}

/**
 * @tc.number: StartSyncRemoteMissions_0100
 * @tc.name: StartSyncRemoteMissions
 * @tc.desc: StartSyncRemoteMissions Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, StartSyncRemoteMissions_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartSyncRemoteMissions_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string devId = "";
    bool fixConflict = true;
    int64_t tag = 0;
    int32_t result = client->StartSyncRemoteMissions(devId , fixConflict , tag);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest StartSyncRemoteMissions_0100 end";
}

/**
 * @tc.number: StopSyncRemoteMissions_0100
 * @tc.name: StopSyncRemoteMissions 
 * @tc.desc: StopSyncRemoteMissions Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, StopSyncRemoteMissions_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StopSyncRemoteMissions_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string devId = "";
    int32_t result = client->StopSyncRemoteMissions(devId);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest StopSyncRemoteMissions_0100 end";
}

/**
 * @tc.number: RegisterMissionListener_0100
 * @tc.name: RegisterMissionListener
 * @tc.desc: RegisterMissionListener Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, RegisterMissionListener_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest RegisterMissionListener_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::u16string devId = to_utf16("deviceId");
    sptr<IRemoteObject> obj = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    int32_t result = client->RegisterMissionListener(devId , obj);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest RegisterMissionListener_0100 end";
}

/**
 * @tc.number: UnRegisterMissionListener_0100
 * @tc.name: UnRegisterMissionListener
 * @tc.desc: UnRegisterMissionListener Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, UnRegisterMissionListener_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest UnRegisterMissionListener_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::u16string devId = to_utf16("deviceId");
    sptr<IRemoteObject> obj = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    client->RegisterMissionListener(devId , obj);
    int32_t result = client->UnRegisterMissionListener(devId , obj);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest UnRegisterMissionListener_0100 end";
}

/**
 * @tc.number: GetMissionInfos_0100
 * @tc.name: GetMissionInfos
 * @tc.desc: GetMissionInfosTest, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, GetMissionInfos_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest GetMissionInfos_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string deviceId = "";
    int32_t numMissions = 0;
    std::vector<AAFwk::MissionInfo> missionInfos;
    int32_t result = client->GetMissionInfos(deviceId , numMissions , missionInfos);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest GetMissionInfos_0100 end";
}

/**
 * @tc.number: GetRemoteMissionSnapshotInfo_0100
 * @tc.name: GetRemoteMissionSnapshotInfo 
 * @tc.desc: GetRemoteMissionSnapshotInfo Test, deviceId is empty , return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, GetRemoteMissionSnapshotInfo_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest GetRemoteMissionSnapshotInfo_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string deviceId;
    int32_t numMissions = 0;
    std::unique_ptr<OHOS::AAFwk::MissionSnapshot> missionSnapshot = std::make_unique<OHOS::AAFwk::MissionSnapshot>();
    int32_t result = client->GetRemoteMissionSnapshotInfo(deviceId , numMissions , missionSnapshot);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest GetRemoteMissionSnapshotInfo_0100 end";
}

/**
 * @tc.number: GetRemoteMissionSnapshotInfo_0200
 * @tc.name: GetRemoteMissionSnapshotInfo 
 * @tc.desc: GetRemoteMissionSnapshotInfo Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, GetRemoteMissionSnapshotInfo_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest GetRemoteMissionSnapshotInfo_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    std::string deviceId ="deviceId";
    int32_t numMissions = 0;
    std::unique_ptr<OHOS::AAFwk::MissionSnapshot> missionSnapshot = std::make_unique<OHOS::AAFwk::MissionSnapshot>();
    int32_t result = client->GetRemoteMissionSnapshotInfo(deviceId , numMissions , missionSnapshot);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest GetRemoteMissionSnapshotInfo_0200 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0100
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test, hasMissions is 1 , len is 1.
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteInt32(1);
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0100 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0200
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test, hasMissions is 1 ，len is -1.
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    parcel.WriteInt32(1);
    parcel.WriteInt32(-1);
    std::vector<AAFwk::MissionInfo> missionInfos;
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0200 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0300
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test, when len = parcel.GetReadableBytes() + 1;
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0300, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0300 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    std::vector<AAFwk::MissionInfo> missionInfos;
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, 1);
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, parcel.GetReadableBytes() + 1);
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_FALSE(result);
    EXPECT_TRUE(missionInfos.empty());
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0300 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0400
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test,  when len = missionInfos.max_size() + 1;
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0400, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0400 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    std::vector<AAFwk::MissionInfo> missionInfos;
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, 1);
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, missionInfos.max_size() + 1);
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_FALSE(result);
    EXPECT_TRUE(missionInfos.empty());
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0400 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0500
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test,  hasMissions is not 1.
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0500, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0500 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    std::vector<AAFwk::MissionInfo> missionInfos;
    parcel.WriteInt32(2);
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0500 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0600
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test, when len = missionInfos.max_size() - 1;
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0600, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0600 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    std::vector<AAFwk::MissionInfo> missionInfos;
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, 1);
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, missionInfos.max_size() - 1);
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_FALSE(result);
    EXPECT_TRUE(missionInfos.empty());
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0600 end";
}

/**
 * @tc.number: ReadMissionInfosFromParcel_0700
 * @tc.name: ReadMissionInfosFromParcel
 * @tc.desc: ReadMissionInfosFromParcel Test, when missionInfo is not nullptr.
 */
HWTEST_F(DistributedClientTest, ReadMissionInfosFromParcel_0700, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0700 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    Parcel parcel;
    std::vector<AAFwk::MissionInfo> missionInfos;
    AAFwk::MissionInfo missionInfo;
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, 1);
    PARCEL_WRITE_HELPER_NORET(parcel, Int32, 1);
    PARCEL_WRITE_HELPER_NORET(parcel, Parcelable, &missionInfo);
    auto result = client->ReadMissionInfosFromParcel(parcel , missionInfos);
    EXPECT_TRUE(result);
    EXPECT_FALSE(missionInfos.empty());
    GTEST_LOG_(INFO) << "DistributedClientTest ReadMissionInfosFromParcel_0700 end";
}

/**
 * @tc.number: StartContinuation_0100
 * @tc.name: StartContinuation
 * @tc.desc: StartContinuation Test, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, StartContinuation_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartContinuation_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t missionId = 0;
    int32_t callerUid = -1;
    int32_t status =0;
    uint32_t accessToken = 0;
    int32_t result = client->StartContinuation(want , missionId , callerUid , status , accessToken);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest StartContinuation_0100 end";
}

/**
 * @tc.number: StartRemoteAbilityByCall_0100
 * @tc.name: StartRemoteAbilityByCall
 * @tc.desc: StartRemoteAbilityByCall Test, connect is nullptr, return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, StartRemoteAbilityByCall_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteAbilityByCall_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t result = client->StartRemoteAbilityByCall(want , nullptr);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteAbilityByCall_0100 end";
}

/**
 * @tc.number: StartRemoteAbilityByCall_0200
 * @tc.name: StartRemoteAbilityByCall
 * @tc.desc: StartRemoteAbilityByCall Test, connect is not nullptr, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, StartRemoteAbilityByCall_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteAbilityByCall_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    sptr<IRemoteObject> connect = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    int32_t result = client->StartRemoteAbilityByCall(want , connect);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteAbilityByCall_0200 end";
}

/**
 * @tc.number: ReleaseRemoteAbility_0100
 * @tc.name: ReleaseRemoteAbility
 * @tc.desc: ReleaseRemoteAbility Test, connect is nullptr, return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, ReleaseRemoteAbility_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReleaseRemoteAbility_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    AppExecFwk::ElementName element;
    int32_t result = client->ReleaseRemoteAbility(nullptr, element);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest ReleaseRemoteAbility_0100 end";
}

/**
 * @tc.number: ReleaseRemoteAbility_0200
 * @tc.name: ReleaseRemoteAbility
 * @tc.desc: ReleaseRemoteAbility Test, connect is not nullptr, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, ReleaseRemoteAbility_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest ReleaseRemoteAbility_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t callerUid = 0;
    uint32_t accessToken = 0;
    int32_t requestCode = 0;
    client->StartRemoteAbility(want , callerUid , accessToken , requestCode);
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> connect = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    int32_t result = client->ReleaseRemoteAbility(connect, element);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest ReleaseRemoteAbility_0200 end";
}

/**
 * @tc.number: StartRemoteFreeInstall_0100
 * @tc.name: StartRemoteFreeInstall
 * @tc.desc: StartRemoteFreeInstall Test, callback is nullptr, return ERR_NULL_OBJECT.
 */
HWTEST_F(DistributedClientTest, StartRemoteFreeInstall_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteFreeInstall_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t callerUid = 0;
    int32_t requestCode = 0;
    uint32_t accessToken = 0;
    int32_t result = client->StartRemoteFreeInstall(want , callerUid , requestCode , accessToken , nullptr);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteFreeInstall_0100 end";
}

/**
 * @tc.number: StartRemoteFreeInstall_0200
 * @tc.name: StartRemoteFreeInstall
 * @tc.desc: StartRemoteFreeInstall Test, callback is not nullptr, return DMS_PERMISSION_DENIED.
 */
HWTEST_F(DistributedClientTest, StartRemoteFreeInstall_0200, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteFreeInstall_0200 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    int32_t callerUid = 0;
    int32_t requestCode = 0;
    uint32_t accessToken = 0;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    int32_t result = client->StartRemoteFreeInstall(want , callerUid , requestCode , accessToken , callback);
    if (client->GetDmsProxy() != nullptr) {
        EXPECT_EQ(result, OHOS::AAFwk::DMS_PERMISSION_DENIED);
    } else {
        EXPECT_EQ(result, OHOS::AAFwk::INVALID_PARAMETERS_ERR);
    }
    GTEST_LOG_(INFO) << "DistributedClientTest StartRemoteFreeInstall_0200 end";
}

/**
 * @tc.number: WriteInfosToParcel_0100
 * @tc.name: WriteInfosToParcel
 * @tc.desc: WriteInfosToParcel Test, callback is not nullptr, return true.
 */
HWTEST_F(DistributedClientTest, WriteInfosToParcel_0100, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "DistributedClientTest WriteInfosToParcel_0100 start";
    auto client = std::make_shared<OHOS::AAFwk::DistributedClient>();
    OHOS::AAFwk::Want want;
    MessageParcel data;
    sptr<IRemoteObject> callback = new (std::nothrow) OHOS::AAFwk::AbilityConnectCallback();
    auto result = client->WriteInfosToParcel(data , want , callback);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "DistributedClientTest WriteInfosToParcel_0100 end";
}