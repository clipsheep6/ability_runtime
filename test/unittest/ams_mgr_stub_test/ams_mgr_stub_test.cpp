/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "mock_ability_debug_response_stub.h"
#include "mock_ams_mgr_scheduler.h"
#include "mock_app_debug_listener_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
    const std::string STRING_BUNDLE_NAME = "bundleName";
    const std::string EMPTY_BUNDLE_NAME = "";
}  // namespace

class AmsMgrStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppDebugListenerStub> listener_;
    sptr<MockAbilityDebugResponseStub> response_;
    sptr<MockAmsMgrScheduler> mockAmsMgrScheduler_;

    void WriteInterfaceToken(MessageParcel& data);
};

void AmsMgrStubTest::SetUpTestCase(void)
{}

void AmsMgrStubTest::TearDownTestCase(void)
{}

void AmsMgrStubTest::SetUp()
{
    GTEST_LOG_(INFO) << "AmsMgrStubTest::SetUp()";

    listener_ = new MockAppDebugListenerStub();
    response_ = new MockAbilityDebugResponseStub();
    mockAmsMgrScheduler_ = new MockAmsMgrScheduler();
}

void AmsMgrStubTest::TearDown()
{}

void AmsMgrStubTest::WriteInterfaceToken(MessageParcel& data)
{
    GTEST_LOG_(INFO) << "AmsMgrStubTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AmsMgrStub::GetDescriptor());
}

/**
 * @tc.name: HandleRegisterAppDebugListener_0100
 * @tc.desc: Handle register app debug listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAppDebugListener(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    EXPECT_NE(listener_, nullptr);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterAppDebugListener_0200
 * @tc.desc: Handle register app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAppDebugListener_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAppDebugListener(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    listener_ = nullptr;
    WriteInterfaceToken(data);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
/**
 * @tc.name: HandleUnregisterAppDebugListener_0100
 * @tc.desc: Handle unregister app debug listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleUnregisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, UnregisterAppDebugListener(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    EXPECT_NE(listener_, nullptr);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterAppDebugListener_0200
 * @tc.desc: Handle unregister app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleUnregisterAppDebugListener_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, UnregisterAppDebugListener(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    listener_ = nullptr;
    WriteInterfaceToken(data);
    data.WriteRemoteObject(listener_);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleAttachAppDebug_0100
 * @tc.desc: Handle attach app, begin debug mode.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleAttachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, AttachAppDebug(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(STRING_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleAttachAppDebug_0200
 * @tc.desc: Handle attach app, begin debug mode, check empty bunle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleAttachAppDebug_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, AttachAppDebug(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(EMPTY_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleSetAppWaitingDebug_0100
 * @tc.desc: Handle set app waiting debug.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleSetAppWaitingDebug_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SetAppWaitingDebug(_, _)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WriteInterfaceToken(data);
    data.WriteString(STRING_BUNDLE_NAME);
    data.WriteBool(false);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::SET_APP_WAITING_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleCancelAppWaitingDebug_0100
 * @tc.desc: Handle cancel app waiting debug.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleCancelAppWaitingDebug_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, CancelAppWaitingDebug()).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WriteInterfaceToken(data);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::CANCEL_APP_WAITING_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleGetWaitingDebugApp_0100
 * @tc.desc: Handle get waiting debug app.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleGetWaitingDebugApp_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, GetWaitingDebugApp(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WriteInterfaceToken(data);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::GET_WAITING_DEBUG_APP), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleIsWaitingDebugApp_0100
 * @tc.desc: Handle get waiting debug app.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleIsWaitingDebugApp_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, IsWaitingDebugApp(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WriteInterfaceToken(data);
    data.WriteString(STRING_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::IS_WAITING_DEBUG_APP), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleClearNonPersistWaitingDebugFlag_0100
 * @tc.desc: Handle clear non persist waiting debug flag.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleClearNonPersistWaitingDebugFlag_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, ClearNonPersistWaitingDebugFlag()).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    WriteInterfaceToken(data);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleDetachAppDebug_0100
 * @tc.desc: Handle detach app, end debug mode.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleDetachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, DetachAppDebug(_)).Times(1);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(STRING_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleDetachAppDebug_0200
 * @tc.desc: Handle detach app, check empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleDetachAppDebug_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, DetachAppDebug(_)).Times(0);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    data.WriteString(EMPTY_BUNDLE_NAME);

    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleRegisterAbilityDebugResponse_0100
 * @tc.desc: Handle register ability debug response.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    WriteInterfaceToken(data);
    EXPECT_NE(response_, nullptr);
    data.WriteRemoteObject(response_);

    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAbilityDebugResponse(_)).Times(1);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterAbilityDebugResponse_0200
 * @tc.desc: Handle register ability debug response, check nullptr response.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrStubTest, HandleRegisterAbilityDebugResponse_0200, TestSize.Level1)
{
    EXPECT_NE(mockAmsMgrScheduler_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    response_ = nullptr;
    WriteInterfaceToken(data);
    data.WriteRemoteObject(response_);

    EXPECT_CALL(*mockAmsMgrScheduler_, RegisterAbilityDebugResponse(_)).Times(0);
    auto result = mockAmsMgrScheduler_->OnRemoteRequest(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}
}  // namespace AppExecFwk
}  // namespace OHOS
