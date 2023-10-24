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

#include<gtest/gtest.h>
#include "mock_app_debug_listener_stub.h"
#include "parcel.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr int32_t DEBUG_INFO_SIZE_ONE = 1;
    constexpr int32_t DEBUG_INFO_SIZE_ZERO = 0;
    constexpr uint32_t UNKNOWN_CODE = 2;
}
class AppDebugListenerStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppDebugListenerStub> mockStub_;
    void WriteInterfaceToken(MessageParcel &parcel);
};

void AppDebugListenerStubTest::SetUpTestCase(void)
{}

void AppDebugListenerStubTest::TearDownTestCase(void)
{}

void AppDebugListenerStubTest::SetUp()
{
    mockStub_ = new MockAppDebugListenerStub();
}

void AppDebugListenerStubTest::TearDown()
{}

void AppDebugListenerStubTest::WriteInterfaceToken(MessageParcel &parcel)
{
    parcel.WriteInterfaceToken(AppDebugListenerStub::GetDescriptor());
}

/**
 * @tc.name: OnRemoteRequest_0100
 * @tc.desc: Receive remote request, call member function.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AppDebugInfo debugInfo;
    WriteInterfaceToken(data);
    data.WriteInt32(DEBUG_INFO_SIZE_ONE);
    data.WriteParcelable(&debugInfo);

    auto result = mockStub_->OnRemoteRequest(
        static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STARTED), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: OnRemoteRequest_0200
 * @tc.desc: Check empty descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    data.WriteInt32(DEBUG_INFO_SIZE_ZERO);

    auto result = mockStub_->OnRemoteRequest(
        static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STARTED), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
}

/**
 * @tc.name: OnRemoteRequest_0300
 * @tc.desc: Check unknown message code.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteInt32(DEBUG_INFO_SIZE_ZERO);

    auto result = mockStub_->OnRemoteRequest(UNKNOWN_CODE, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: OnRemoteRequest_0400
 * @tc.desc: Check min debug info size.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, OnRemoteRequest_0400, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteInt32(DEBUG_INFO_SIZE_ZERO);

    auto result = mockStub_->OnRemoteRequest(
        static_cast<uint32_t>(IAppDebugListener::Message::ON_APP_DEBUG_STARTED), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleOnAppDebugStarted_0100
 * @tc.desc: Handler of remote request: app debug started.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, HandleOnAppDebugStarted_0100, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    AppDebugInfo debugInfo;
    data.WriteInt32(DEBUG_INFO_SIZE_ONE);
    data.WriteParcelable(&debugInfo);

    EXPECT_CALL(*mockStub_, OnAppDebugStarted(_)).Times(1);
    EXPECT_EQ(mockStub_->HandleOnAppDebugStarted(data, reply), NO_ERROR);
}

/**
 * @tc.name: HandleOnAppDebugStarted_0200
 * @tc.desc: Check null AppDebugInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, HandleOnAppDebugStarted_0200, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    AppDebugInfo *debugInfo = nullptr;
    WriteInterfaceToken(data);
    data.WriteInt32(DEBUG_INFO_SIZE_ONE);
    data.WriteParcelable(debugInfo);

    EXPECT_CALL(*mockStub_, OnAppDebugStarted(_)).Times(0);
    EXPECT_EQ(mockStub_->HandleOnAppDebugStarted(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleOnAppDebugStoped_0100
 * @tc.desc: Handler of remote request: app debug stoped.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, HandleOnAppDebugStoped_0100, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    AppDebugInfo debugInfo;
    data.WriteInt32(DEBUG_INFO_SIZE_ONE);
    data.WriteParcelable(&debugInfo);
    
    EXPECT_CALL(*mockStub_, OnAppDebugStoped(_)).Times(1);
    EXPECT_EQ(mockStub_->HandleOnAppDebugStoped(data, reply), NO_ERROR);
}

/**
 * @tc.name: HandleOnAppDebugStoped_0100
 * @tc.desc: Check null AppDebugInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppDebugListenerStubTest, HandleOnAppDebugStoped_0200, TestSize.Level1)
{
    EXPECT_NE(mockStub_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    AppDebugInfo *debugInfo = nullptr;
    WriteInterfaceToken(data);
    data.WriteInt32(DEBUG_INFO_SIZE_ONE);
    data.WriteParcelable(debugInfo);

    EXPECT_CALL(*mockStub_, OnAppDebugStoped(_)).Times(0);
    EXPECT_EQ(mockStub_->HandleOnAppDebugStoped(data, reply), ERR_INVALID_DATA);
}
} // namespace AppExecFwk
} // namespace OHOS
