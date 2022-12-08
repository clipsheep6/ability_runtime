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

#define private public
#include "remote_mission_listener_proxy.h"
#undef private
#include "remote_mission_listener_stub_mock.h"
#include "ipc_types.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class RemoteMissionListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<RemoteMissionListenerProxy> proxy_ {nullptr};
    sptr<RemoteMissionListenerStubMock> mock_ {nullptr};
};

void RemoteMissionListenerProxyTest::SetUpTestCase(void)
{}
void RemoteMissionListenerProxyTest::TearDownTestCase(void)
{}
void RemoteMissionListenerProxyTest::TearDown(void)
{}
void RemoteMissionListenerProxyTest::SetUp()
{
    mock_ = new RemoteMissionListenerStubMock();
    proxy_ = std::make_shared<RemoteMissionListenerProxy>(mock_);
}

/*
 * Feature: RemoteMissionListenerProxy
 * Function: NotifyMissionsChanged
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerProxy NotifyMissionsChanged
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionsChanged
 */
HWTEST_F(RemoteMissionListenerProxyTest, NotifyMissionsChanged_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteMissionListenerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    proxy_->NotifyMissionsChanged(deviceId);
    EXPECT_EQ(IRemoteMissionListener::NOTIFY_MISSION_CHANGED, mock_->code_);
}

/*
 * Feature: RemoteMissionListenerProxy
 * Function: NotifyMissionsChanged
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerProxy NotifyMissionsChanged
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMissionsChanged
 */
HWTEST_F(RemoteMissionListenerProxyTest, NotifyMissionsChanged_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteMissionListenerStubMock::InvokeErrorSendRequest));
    std::string deviceId = "test";
    proxy_->NotifyMissionsChanged(deviceId);
    EXPECT_EQ(IRemoteMissionListener::NOTIFY_MISSION_CHANGED, mock_->code_);
}

/*
 * Feature: RemoteMissionListenerProxy
 * Function: NotifySnapshot
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerProxy NotifySnapshot
 * EnvConditions: NA
 * CaseDescription: Verify NotifySnapshot
 */
HWTEST_F(RemoteMissionListenerProxyTest, NotifySnapshot_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteMissionListenerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t missionId = 0;
    proxy_->NotifySnapshot(deviceId, missionId);
    EXPECT_EQ(IRemoteMissionListener::NOTIFY_SNAPSHOT, mock_->code_);
}

/*
 * Feature: RemoteMissionListenerProxy
 * Function: NotifySnapshot
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerProxy NotifySnapshot
 * EnvConditions: NA
 * CaseDescription: Verify NotifySnapshot
 */
HWTEST_F(RemoteMissionListenerProxyTest, NotifySnapshot_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteMissionListenerStubMock::InvokeErrorSendRequest));
    std::string deviceId = "test";
    int32_t missionId = 0;
    proxy_->NotifySnapshot(deviceId, missionId);
    EXPECT_EQ(IRemoteMissionListener::NOTIFY_SNAPSHOT, mock_->code_);
}

/*
 * Feature: RemoteMissionListenerProxy
 * Function: NotifyNetDisconnect
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerProxy NotifyNetDisconnect
 * EnvConditions: NA
 * CaseDescription: Verify NotifyNetDisconnect
 */
HWTEST_F(RemoteMissionListenerProxyTest, NotifyNetDisconnect_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteMissionListenerStubMock::InvokeSendRequest));
    std::string deviceId = "";
    int32_t state = 0;
    proxy_->NotifyNetDisconnect(deviceId, state);
    EXPECT_EQ(IRemoteMissionListener::NOTIFY_NET_DISCONNECT, mock_->code_);
}

/*
 * Feature: RemoteMissionListenerProxy
 * Function: NotifyNetDisconnect
 * SubFunction: NA
 * FunctionPoints: RemoteMissionListenerProxy NotifyNetDisconnect
 * EnvConditions: NA
 * CaseDescription: Verify NotifyNetDisconnect
 */
HWTEST_F(RemoteMissionListenerProxyTest, NotifyNetDisconnect_002, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &RemoteMissionListenerStubMock::InvokeErrorSendRequest));
    std::string deviceId = "test";
    int32_t state = 0;
    proxy_->NotifyNetDisconnect(deviceId, state);
    EXPECT_EQ(IRemoteMissionListener::NOTIFY_NET_DISCONNECT, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS
