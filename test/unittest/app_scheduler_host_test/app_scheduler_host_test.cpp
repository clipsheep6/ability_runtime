/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "fault_data.h"
#include "message_parcel.h"
#include "mock_app_scheduler.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppSchedulerHostTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppScheduler> mockAppScheduler_;

    void WriteInterfaceToken(MessageParcel& data);
};

void AppSchedulerHostTest::SetUpTestCase(void)
{}

void AppSchedulerHostTest::TearDownTestCase(void)
{}

void AppSchedulerHostTest::SetUp()
{
    GTEST_LOG_(INFO) << "AppSchedulerHostTest::SetUp()";

    mockAppScheduler_ = new MockAppScheduler();
}

void AppSchedulerHostTest::TearDown()
{}

void AppSchedulerHostTest::WriteInterfaceToken(MessageParcel& data)
{
    GTEST_LOG_(INFO) << "AppSchedulerHostTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AppSchedulerHost::GetDescriptor());
}

/**
 * @tc.name: HandleNotifyAppFault_001
 * @tc.desc: Verify that the HandleNotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppSchedulerHostTest, HandleNotifyAppFault_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    FaultData faultData;
    faultData.errorObject.name = "testName";
    faultData.errorObject.message = "testMessage";
    faultData.errorObject.stack = "testStack";
    faultData.faultType = FaultDataType::UNKNOWN;
    data.WriteParcelable(&faultData);
    EXPECT_CALL(*mockAppScheduler_, ScheduleNotifyAppFault(_)).Times(1);
    auto result = mockAppScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAppScheduler::Message::SCHEDULE_NOTIFY_FAULT), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}
} // namespace AppExecFwk
} // namespace OHOS
