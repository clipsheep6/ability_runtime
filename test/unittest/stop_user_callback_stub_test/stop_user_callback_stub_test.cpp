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
#include "stop_user_callback_stub.h"
#include "message_parcel.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using namespace testing;

class MockStopUserCallbackStub : public StopUserCallbackStub {
public:
    MockStopUserCallbackStub() = default;
    virtual ~MockStopUserCallbackStub()
    {}
    void OnStopUserDone(int userId, int errcode) override
    {}
};

class StopUserCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StopUserCallbackStubTest::SetUpTestCase(void)
{}
void StopUserCallbackStubTest::TearDownTestCase(void)
{}
void StopUserCallbackStubTest::SetUp()
{}
void StopUserCallbackStubTest::TearDown()
{}

/**
 * @tc.name: StopUserCallbackStubTest_001
 * @tc.desc: Verify OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(StopUserCallbackStubTest, StopUserCallbackStubTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopUserCallbackStubTest_001 start";
    std::shared_ptr<StopUserCallbackStub> backStub = std::make_shared<MockStopUserCallbackStub>();
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::u16string value = u"ohos.abilityshell.DistributedConnection";
    data.WriteString16(value);
    auto result = backStub->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(result, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "StopUserCallbackStubTest_001 end";
}

/**
 * @tc.name: StopUserCallbackStubTest_002
 * @tc.desc: Verify OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(StopUserCallbackStubTest, StopUserCallbackStubTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopUserCallbackStubTest_002 start";
    std::shared_ptr<StopUserCallbackStub> backStub = std::make_shared<MockStopUserCallbackStub>();
    uint32_t code = 2;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::u16string metaDescriptor_ = u"ohos.aafwk.StopUserCallback";
    data.WriteInterfaceToken(metaDescriptor_);
    auto result = backStub->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
    GTEST_LOG_(INFO) << "StopUserCallbackStubTest_002 end";
}

/**
 * @tc.name: StopUserCallbackStubTest_003
 * @tc.desc: Verify OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(StopUserCallbackStubTest, StopUserCallbackStubTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopUserCallbackStubTest_003 start";
    std::shared_ptr<StopUserCallbackStub> backStub = std::make_shared<MockStopUserCallbackStub>();
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::u16string metaDescriptor_ = u"ohos.aafwk.StopUserCallback";
    data.WriteInterfaceToken(metaDescriptor_);
    auto result = backStub->OnRemoteRequest(code, data, reply, option);

    EXPECT_EQ(result, ERR_NONE);
    GTEST_LOG_(INFO) << "StopUserCallbackStubTest_003 end";
}
