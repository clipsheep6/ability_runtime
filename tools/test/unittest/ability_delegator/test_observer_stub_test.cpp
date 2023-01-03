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
#include "hilog_wrapper.h"
#include "iremote_proxy.h"
#include "mock_test_observer_stub.h"
#include "test_observer_stub.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

class TestObserverStubTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void TestObserverStubTest::SetUpTestCase()
{}

void TestObserverStubTest::TearDownTestCase()
{}

void TestObserverStubTest::SetUp()
{}

void TestObserverStubTest::TearDown()
{}

/**
 * @tc.number: Test_Observer_Stub_Test_0100
 * @tc.name: Test OnRemoteRequest
 * @tc.desc: Verify the OnRemoteRequest when code is AA_TEST_STATUS return NO_ERROR.
 */
HWTEST_F(TestObserverStubTest, Test_Observer_Stub_Test_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Stub_Test_0100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(TestObserverStub::GetDescriptor());
    std::string args = "test";
    data.WriteString(args);
    data.WriteInt64(0);

    MockTestObserverStub stub;
    int res = stub.OnRemoteRequest(static_cast<uint32_t>(ITestObserver::Message::AA_TEST_STATUS), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    HILOG_INFO("Test_Observer_Stub_Test_0100 end");
}

/**
 * @tc.number: Test_Observer_Stub_Test_0200
 * @tc.name: Test OnRemoteRequest
 * @tc.desc: Verify the OnRemoteRequest when code is AA_TEST_FINISHED return NO_ERROR.
 */
HWTEST_F(TestObserverStubTest, Test_Observer_Stub_Test_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Stub_Test_0200 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string args = "test";
    data.WriteInterfaceToken(TestObserverStub::GetDescriptor());
    data.WriteString(args);
    data.WriteInt64(0);

    MockTestObserverStub stub;
    int res = stub.OnRemoteRequest(static_cast<uint32_t>(ITestObserver::Message::AA_TEST_FINISHED), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    HILOG_INFO("Test_Observer_Stub_Test_0200 end");
}

/**
 * @tc.number: Test_Observer_Stub_Test_0300
 * @tc.name: Test OnRemoteRequest
 * @tc.desc: Verify the OnRemoteRequest when code is AA_EXECUTE_SHELL_COMMAND return NO_ERROR.
 */
HWTEST_F(TestObserverStubTest, Test_Observer_Stub_Test_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Stub_Test_0300 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(TestObserverStub::GetDescriptor());
    std::string args = "test";
    data.WriteString(args);
    data.WriteInt64(0);

    MockTestObserverStub stub;
    int res = stub.OnRemoteRequest(static_cast<uint32_t>(ITestObserver::Message::AA_EXECUTE_SHELL_COMMAND), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    HILOG_INFO("Test_Observer_Stub_Test_0300 end");
}

/**
 * @tc.number: Test_Observer_Stub_Test_0400
 * @tc.name: Test OnRemoteRequest
 * @tc.desc: Verify the OnRemoteRequest when code is 0 return IPC_STUB_UNKNOW_TRANS_ERR.
 */
HWTEST_F(TestObserverStubTest, Test_Observer_Stub_Test_0400, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Stub_Test_0400 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(TestObserverStub::GetDescriptor());
    std::string args = "test";
    data.WriteString(args);
    data.WriteInt64(0);

    MockTestObserverStub stub;
    int res = stub.OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);

    HILOG_INFO("Test_Observer_Stub_Test_0400 end");
}

/**
 * @tc.number: Test_Observer_Stub_Test_0500
 * @tc.name: Test OnRemoteRequest
 * @tc.desc: Verify the OnRemoteRequest when local descriptor is not equal to remote return ERR_TRANSACTION_FAILED.
 */
HWTEST_F(TestObserverStubTest, Test_Observer_Stub_Test_0500, Function | MediumTest | Level1)
{
    HILOG_INFO("Test_Observer_Stub_Test_0500 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::string args = "test";
    data.WriteString(args);
    data.WriteInt64(0);

    MockTestObserverStub stub;
    int res = stub.OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);

    HILOG_INFO("Test_Observer_Stub_Test_0500 end");
}