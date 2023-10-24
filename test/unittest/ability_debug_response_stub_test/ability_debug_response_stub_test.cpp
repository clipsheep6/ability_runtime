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

#include "mock_ability_token.h"
#include "hilog_wrapper.h"
#define private public
#include "mock_ability_debug_response_stub.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing;
using namespace testing::ext;

class AbilityDebugResponseStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void WriteInterfaceToken(MessageParcel &parcel);
};

void AbilityDebugResponseStubTest::SetUpTestCase(void)
{}

void AbilityDebugResponseStubTest::TearDownTestCase(void)
{}

void AbilityDebugResponseStubTest::SetUp()
{}

void AbilityDebugResponseStubTest::TearDown()
{}

void AbilityDebugResponseStubTest::WriteInterfaceToken(MessageParcel &parcel)
{
    parcel.WriteInterfaceToken(AbilityDebugResponseStub::GetDescriptor());
}

/**
 * @tc.name: AbilityDebugResponseProxyTest_HandleOnAbilitysDebugStarted_0100
 * @tc.desc: Verify the HandleOnAbilitysDebugStarted calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugResponseStubTest, HandleOnAbilitysDebugStarted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityDebugResponseProxyTest_HandleOnAbilitysDebugStarted_0100 start";
    auto stub = new (std::nothrow) MockAbilityDebugResponseStub();
    EXPECT_TRUE(stub);
    EXPECT_CALL(*stub, OnAbilitysDebugStarted(_)).Times(1);

    sptr<MockAbilityToken> token = new (std::nothrow) MockAbilityToken();
    EXPECT_TRUE(token);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int tokenSize = 1;
    WriteInterfaceToken(data);
    data.WriteInt32(tokenSize);
    data.WriteRemoteObject(token);

    auto result = stub->OnRemoteRequest(
        static_cast<uint32_t>(IAbilityDebugResponse::Message::ON_ABILITYS_DEBUG_STARTED), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "AbilityDebugResponseProxyTest_HandleOnAbilitysDebugStarted_0100 end";
}

/**
 * @tc.name: AbilityDebugResponseProxyTest_HandleOnAbilitysDebugStoped_0100
 * @tc.desc: Verify the HandleOnAbilitysDebugStarted calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugResponseStubTest, HandleOnAbilitysDebugStoped_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityDebugResponseProxyTest_HandleOnAbilitysDebugStoped_0100 start";
    auto stub = new (std::nothrow) MockAbilityDebugResponseStub();
    EXPECT_TRUE(stub);
    EXPECT_CALL(*stub, OnAbilitysDebugStarted(_)).Times(1);
    
    sptr<MockAbilityToken> token = new (std::nothrow) MockAbilityToken();
    EXPECT_TRUE(token);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int tokenSize = 1;
    WriteInterfaceToken(data);
    data.WriteInt32(tokenSize);
    data.WriteRemoteObject(token);

    auto result = stub->OnRemoteRequest(
        static_cast<uint32_t>(IAbilityDebugResponse::Message::ON_ABILITYS_DEBUG_STOPED), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    GTEST_LOG_(INFO) << "AbilityDebugResponseProxyTest_HandleOnAbilitysDebugStoped_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS