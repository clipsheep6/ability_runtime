/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ams_mgr_proxy.h"
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

class AmsMgrProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAmsMgrScheduler> mockAmsMgrScheduler_;
    sptr<AmsMgrProxy> amsMgrProxy_;
    sptr<MockAppDebugListenerStub> listener_;
    sptr<MockAbilityDebugResponseStub> response_;
};

void AmsMgrProxyTest::SetUpTestCase(void)
{}

void AmsMgrProxyTest::TearDownTestCase(void)
{}

void AmsMgrProxyTest::SetUp()
{
    GTEST_LOG_(INFO) << "AmsMgrProxyTest::SetUp()";

    mockAmsMgrScheduler_ = new MockAmsMgrScheduler();
    amsMgrProxy_ = new AmsMgrProxy(mockAmsMgrScheduler_);
    listener_ = new MockAppDebugListenerStub();
    response_ = new MockAbilityDebugResponseStub();
}

void AmsMgrProxyTest::TearDown()
{}

/**
 * @tc.name: RegisterAppDebugListener_0100
 * @tc.desc: Register app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, RegisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(listener_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->RegisterAppDebugListener(listener_);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    listener_ = nullptr;
    result = amsMgrProxy_->RegisterAppDebugListener(listener_);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: UnregisterAppDebugListener_0100
 * @tc.desc: Unregister app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, UnregisterAppDebugListener_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(listener_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->UnregisterAppDebugListener(listener_);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    listener_ = nullptr;
    result = amsMgrProxy_->UnregisterAppDebugListener(listener_);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: AttachAppDebug_0100
 * @tc.desc: Attach app debug by bundle name, check empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, AttachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->AttachAppDebug(STRING_BUNDLE_NAME);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    result = amsMgrProxy_->AttachAppDebug(EMPTY_BUNDLE_NAME);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: DetachAppDebug_0100
 * @tc.desc: Detach app debug by bundleName, check empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, DetachAppDebug_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->DetachAppDebug(STRING_BUNDLE_NAME);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    result = amsMgrProxy_->DetachAppDebug(EMPTY_BUNDLE_NAME);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: RegisterAbilityDebugResponse_0100
 * @tc.desc: Register ability debug response, check nullptr response.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, RegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(response_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(0));
    auto result = amsMgrProxy_->RegisterAbilityDebugResponse(response_);
    EXPECT_EQ(result, NO_ERROR);

    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _)).Times(0);
    response_ = nullptr;
    result = amsMgrProxy_->RegisterAbilityDebugResponse(response_);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: NotifyAppMgrRecordExitReason_0100
 * @tc.desc: NotifyAppMgrRecordExitReason.
 * @tc.type: FUNC
 */
HWTEST_F(AmsMgrProxyTest, NotifyAppMgrRecordExitReason_0100, TestSize.Level1)
{
    EXPECT_NE(amsMgrProxy_, nullptr);
    EXPECT_NE(response_, nullptr);
    EXPECT_CALL(*mockAmsMgrScheduler_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(NO_ERROR));

    int32_t reason = 0;
    int32_t pid = 1;
    std::string exitMsg = "JsError";
    auto result = amsMgrProxy_->NotifyAppMgrRecordExitReason(reason, pid, exitMsg);
    EXPECT_EQ(result, NO_ERROR);
}
}  // namespace AppExecFwk
}  // namespace OHOS
