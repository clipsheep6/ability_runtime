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

#include "mock_ams_mgr_scheduler.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {}  // namespace

class AmsMgrStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAmsMgrScheduler> mockAmsMgrScheduler_;

    void WriteInterfaceToken(MessageParcel &data);
};

void AmsMgrStubTest::SetUpTestCase(void)
{}

void AmsMgrStubTest::TearDownTestCase(void)
{}

void AmsMgrStubTest::SetUp()
{
    GTEST_LOG_(INFO) << "AmsMgrStubTest::SetUp()";

    mockAmsMgrScheduler_ = new MockAmsMgrScheduler();
}

void AmsMgrStubTest::TearDown()
{}

void AmsMgrStubTest::WriteInterfaceToken(MessageParcel &data)
{
    GTEST_LOG_(INFO) << "AmsMgrStubTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AmsMgrStub::GetDescriptor());
}

/**
 * @tc.name: AmsMgrStub_GetConfiguration_0100
 * @tc.desc: GetConfiguration
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AmsMgrStubTest, AmsMgrStub_GetConfiguration_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AmsMgrStub_GetConfiguration_0100 start";

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);

    EXPECT_CALL(*mockAmsMgrScheduler_, GetConfiguration(_)).Times(1);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::GET_CONFIGURATION), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "AmsMgrStub_GetConfiguration_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
