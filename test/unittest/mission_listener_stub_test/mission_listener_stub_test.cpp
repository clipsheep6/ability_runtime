/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "mock_mission_listener_stub.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class MissionListenerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel &data);
    sptr<MockMissionListenerStub> stub_ {nullptr};
};

void MissionListenerStubTest::SetUpTestCase(void)
{}
void MissionListenerStubTest::TearDownTestCase(void)
{}
void MissionListenerStubTest::SetUp()
{
    stub_ = new MockMissionListenerStub();
}
void MissionListenerStubTest::TearDown()
{}

void MissionListenerStubTest::WriteInterfaceToken(MessageParcel &data)
{
    data.WriteInterfaceToken(MockMissionListenerStub::GetDescriptor());
}

/*
 * Feature: MissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);
    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnRemoteRequest
 * EnvConditions: Description abnormal
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int res = stub_->OnRemoteRequest(IMissionListener::ON_MISSION_CREATED, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/*
 * Feature: MissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnRemoteRequest
 * EnvConditions: Code is ON_MISSION_CREATED
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*stub_, OnMissionCreated(_)).Times(1).WillOnce(Return());
    int res = stub_->OnRemoteRequest(IMissionListener::ON_MISSION_CREATED, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionCreatedInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionCreatedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission created inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionCreatedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionCreated(_)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionCreatedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionDestroyedInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionDestroyedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission destroyed inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionDestroyedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionDestroyed(_)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionDestroyedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionSnapshotChangedInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionSnapshotChangedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission snapshot changed inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionSnapshotChangedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionSnapshotChanged(_)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionSnapshotChangedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionMovedToFrontInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionMovedToFrontInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission moved to front inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionMovedToFrontInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionMovedToFront(_)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionMovedToFrontInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionIconUpdatedInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionIconUpdatedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission icon updated inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionIconUpdatedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionIconUpdated(_, _)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionIconUpdatedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionClosedInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionClosedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission closed inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionClosedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionClosed(_)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionClosedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: MissionListenerStub
 * Function: OnMissionLabelUpdatedInner
 * SubFunction: NA
 * FunctionPoints: MissionListenerStub OnMissionLabelUpdatedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that on mission label updated inner is normal
 */
HWTEST_F(MissionListenerStubTest, MissionListenerStubTest_OnMissionLabelUpdatedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, OnMissionLabelUpdated(_)).Times(1).WillOnce(Return());
    int res = stub_->OnMissionLabelUpdatedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
