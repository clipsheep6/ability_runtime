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
#include "mock_system_ability_token_callback_stub.h"
#include "want.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
const std::u16string SYSTEM_ABILITY_TOKEN_CALLBACK_INTERFACE_TOKEN = u"ohos.aafwk.ISystemAbilityTokenCallback";
}

class SystemAbilityTokenCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel &data);
    sptr<MockSystemAbilityTokenCallbackStub> stub_ {nullptr};
};

void SystemAbilityTokenCallbackStubTest::SetUpTestCase(void)
{}
void SystemAbilityTokenCallbackStubTest::TearDownTestCase(void)
{}
void SystemAbilityTokenCallbackStubTest::TearDown()
{}

void SystemAbilityTokenCallbackStubTest::SetUp()
{
    stub_ = new MockSystemAbilityTokenCallbackStub();
}

void SystemAbilityTokenCallbackStubTest::WriteInterfaceToken(MessageParcel &data)
{
    data.WriteInterfaceToken(SYSTEM_ABILITY_TOKEN_CALLBACK_INTERFACE_TOKEN);
}

/*
 * Feature: SystemAbilityTokenCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: SystemAbilityTokenCallbackStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(SystemAbilityTokenCallbackStubTest, SystemAbilityTokenCallbackStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);

    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}

/*
 * Feature: SystemAbilityTokenCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: SystemAbilityTokenCallbackStub OnRemoteRequest
 * EnvConditions: Description abnormal
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(SystemAbilityTokenCallbackStubTest, SystemAbilityTokenCallbackStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int res = stub_->OnRemoteRequest(ISystemAbilityTokenCallback::SEND_RESULT, data, reply, option);

    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: SystemAbilityTokenCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: SystemAbilityTokenCallbackStub OnRemoteRequest
 * EnvConditions: Code is SEND_RESULT, want is nullptr
 * CaseDescription: Verify that SEND_RESULT want readParcelable failed
 */
HWTEST_F(SystemAbilityTokenCallbackStubTest, SystemAbilityTokenCallbackStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteParcelable(nullptr);
    int res = stub_->OnRemoteRequest(ISystemAbilityTokenCallback::SEND_RESULT, data, reply, option);

    EXPECT_EQ(res, ERR_NULL_OBJECT);
}

/*
 * Feature: SystemAbilityTokenCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: SystemAbilityTokenCallbackStub OnRemoteRequest
 * EnvConditions: Code is SEND_RESULT, want is not nullptr
 * CaseDescription: Verify that on remote request is normal
 */
HWTEST_F(SystemAbilityTokenCallbackStubTest, SystemAbilityTokenCallbackStubTest_OnRemoteRequest_004, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    Want want;
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);
    EXPECT_CALL(*stub_, SendResult(_, _, _, _, _)).Times(1).WillOnce(Return(0));
    int res = stub_->OnRemoteRequest(ISystemAbilityTokenCallback::SEND_RESULT, data, reply, option);

    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
