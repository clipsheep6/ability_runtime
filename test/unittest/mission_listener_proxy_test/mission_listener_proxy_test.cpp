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
#include "mission_listener_proxy.h"
#undef private
#include "mission_listener_stub_mock.h"
#include "ipc_types.h"
#include "message_parcel.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class MissionListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MissionListenerProxy> proxy_ {nullptr};
    sptr<MissionListenerStubMock> mock_ {nullptr};
};

void MissionListenerProxyTest::SetUpTestCase(void)
{}
void MissionListenerProxyTest::TearDownTestCase(void)
{}
void MissionListenerProxyTest::TearDown(void)
{}
void MissionListenerProxyTest::SetUp()
{
    mock_ = new MissionListenerStubMock();
    proxy_ = std::make_shared<MissionListenerProxy>(mock_);
}

/*
 * Feature: MissionListenerProxy
 * Function: OnMissionCreated
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionCreated
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionCreated
 */
HWTEST_F(MissionListenerProxyTest, OnMissionCreated_001, TestSize.Level1)
{
    int32_t missionId = 0;
    proxy_->OnMissionCreated(missionId);
}

/*
 * Feature: MissionListenerProxy
 * Function: OnMissionDestroyed
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionDestroyed
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionDestroyed
 */
HWTEST_F(MissionListenerProxyTest, OnMissionDestroyed_001, TestSize.Level1)
{
    int32_t missionId = 0;
    proxy_->OnMissionDestroyed(missionId);
}

/*
 * Feature: MissionListenerProxy
 * Function: OnMissionSnapshotChanged
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionSnapshotChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionSnapshotChanged
 */
HWTEST_F(MissionListenerProxyTest, OnMissionSnapshotChanged_001, TestSize.Level1)
{
    int32_t missionId = 0;
    proxy_->OnMissionSnapshotChanged(missionId);
}

/*
 * Feature: MissionListenerProxy
 * Function: OnMissionMovedToFront
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionMovedToFront
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionMovedToFront
 */
HWTEST_F(MissionListenerProxyTest, OnMissionMovedToFront_001, TestSize.Level1)
{
    int32_t missionId = 0;
    proxy_->OnMissionMovedToFront(missionId);
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: MissionListenerProxy
 * Function: OnMissionIconUpdated
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionIconUpdated
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionIconUpdated
 */
HWTEST_F(MissionListenerProxyTest, OnMissionIconUpdated_001, TestSize.Level1)
{
    int32_t missionId = 0;
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    proxy_->OnMissionIconUpdated(missionId, icon);
}
#endif

/*
 * Feature: MissionListenerProxy
 * Function: OnMissionClosed
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionClosed
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionClosed
 */
HWTEST_F(MissionListenerProxyTest, OnMissionClosed_001, TestSize.Level1)
{
    int32_t missionId = 0;
    proxy_->OnMissionClosed(missionId);
}

/*
 * Feature: MissionListenerProxy
 * Function: OnMissionLabelUpdated
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy OnMissionLabelUpdated
 * EnvConditions: NA
 * CaseDescription: Verify OnMissionLabelUpdated
 */
HWTEST_F(MissionListenerProxyTest, OnMissionLabelUpdated_001, TestSize.Level1)
{
    int32_t missionId = 0;
    proxy_->OnMissionLabelUpdated(missionId);
}

/*
 * Feature: MissionListenerProxy
 * Function: SendRequestCommon
 * SubFunction: NA
 * FunctionPoints: MissionListenerProxy SendRequestCommon
 * EnvConditions: NA
 * CaseDescription: Verify SendRequestCommon
 */
HWTEST_F(MissionListenerProxyTest, SendRequestCommon_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &MissionListenerStubMock::InvokeSendRequest));
    int32_t missionId = 0;
    IMissionListener::MissionListenerCmd cmd = IMissionListener::ON_MISSION_LABEL_UPDATED;
    proxy_->SendRequestCommon(missionId, cmd);
    EXPECT_EQ(IMissionListener::ON_MISSION_LABEL_UPDATED, mock_->code_);
}
}  // namespace AAFwk
}  // namespace OHOS
