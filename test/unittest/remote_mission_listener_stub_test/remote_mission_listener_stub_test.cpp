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
#include "mock_remote_mission_listener_stub.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class RemoteMissionListenerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel &data);
    sptr<MockRemoteMissionListenerStub> stub_ {nullptr};
};

void RemoteMissionListenerStubTest::SetUpTestCase(void)
{}
void RemoteMissionListenerStubTest::TearDownTestCase(void)
{}
void RemoteMissionListenerStubTest::SetUp()
{
    stub_ = new MockRemoteMissionListenerStub();
}
void RemoteMissionListenerStubTest::TearDown()
{}

void RemoteMissionListenerStubTest::WriteInterfaceToken(MessageParcel &data)
{
    data.WriteInterfaceToken(MockRemoteMissionListenerStub::GetDescriptor());
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);
    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub OnRemoteRequest
 * EnvConditions: Description abnormal
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int res = stub_->OnRemoteRequest(IRemoteMissionListener::NOTIFY_MISSION_CHANGED, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub OnRemoteRequest
 * EnvConditions: Code is NOTIFY_MISSION_CHANGED
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*stub_, NotifyMissionsChanged(_)).Times(1).WillOnce(Return());
    int res = stub_->OnRemoteRequest(IRemoteMissionListener::NOTIFY_MISSION_CHANGED, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub OnRemoteRequest
 * EnvConditions: Code is NOTIFY_SNAPSHOT
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_OnRemoteRequest_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*stub_, NotifySnapshot(_, _)).Times(1).WillOnce(Return());
    int res = stub_->OnRemoteRequest(IRemoteMissionListener::NOTIFY_SNAPSHOT, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub OnRemoteRequest
 * EnvConditions: Code is NOTIFY_NET_DISCONNECT
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_OnRemoteRequest_005, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*stub_, NotifyNetDisconnect(_, _)).Times(1).WillOnce(Return());
    int res = stub_->OnRemoteRequest(IRemoteMissionListener::NOTIFY_NET_DISCONNECT, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: NotifyMissionsChangedInner
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub NotifyMissionsChangedInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that notify missions changed inner is normal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_NotifyMissionsChangedInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, NotifyMissionsChanged(_)).Times(1).WillOnce(Return());
    int res = stub_->NotifyMissionsChangedInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: NotifySnapshotInner
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub NotifySnapshotInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that notify snapshot inner is normal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_NotifySnapshotInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, NotifySnapshot(_, _)).Times(1).WillOnce(Return());
    int res = stub_->NotifySnapshotInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: RemoteMissionListenerStub
 * Function: NotifyNetDisconnectInner
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerStub NotifyNetDisconnectInner
 * EnvConditions: Description normal
 * CaseDescription: Verify that notify net disconnect inner is normal
 */
HWTEST_F(RemoteMissionListenerStubTest, RemoteMissionListenerStubTest_NotifyNetDisconnectInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*stub_, NotifyNetDisconnect(_, _)).Times(1).WillOnce(Return());
    int res = stub_->NotifyNetDisconnectInner(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
