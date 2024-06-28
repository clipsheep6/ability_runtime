/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "ability_manager_client.h"
#include "assert_fault_callback.h"
#include "assert_fault_task_thread.h"

#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class AssertFaultCallbackTest : public testing::Test {
public:
    AssertFaultCallbackTest()
    {}
    ~AssertFaultCallbackTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AssertFaultCallbackTest::SetUpTestCase(void)
{}

void AssertFaultCallbackTest::TearDownTestCase(void)
{}

void AssertFaultCallbackTest::SetUp(void)
{}

void AssertFaultCallbackTest::TearDown(void)
{}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.name: OnRemoteRequest
 * @tc.desc: Test whether OnRemoteRequest and are called normally.
 */
HWTEST_F(AssertFaultCallbackTest, OnRemoteRequest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();

    auto assertFaultCallback = std::make_shared<AssertFaultCallback>(assertThread);
    EXPECT_EQ(assertFaultCallback->status_, AAFwk::UserStatus::ASSERT_TERMINATE);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t res = assertFaultCallback->OnRemoteRequest(
        AssertFaultCallback::MessageCode::NOTIFY_DEBUG_ASSERT_RESULT, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);

    if (!data.WriteInterfaceToken(u"ohos.IAssertFaultInterface")) {
        GTEST_LOG_(INFO) << "Write interface token failed.";
    }
    res = assertFaultCallback->OnRemoteRequest(
        AssertFaultCallback::MessageCode::NOTIFY_DEBUG_ASSERT_RESULT, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    res = assertFaultCallback->OnRemoteRequest(2, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
    GTEST_LOG_(INFO) << "OnRemoteRequest_0100 end";
}

/**
 * @tc.number: GetAssertResult_0100
 * @tc.name: GetAssertResult
 * @tc.desc: Test whether GetAssertResult and are called normally.
 */
HWTEST_F(AssertFaultCallbackTest, GetAssertResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "GetAssertResult_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();
    auto assertFaultCallback = std::make_shared<AssertFaultCallback>(assertThread);
    EXPECT_EQ(assertFaultCallback->GetAssertResult(), AAFwk::UserStatus::ASSERT_TERMINATE);
    GTEST_LOG_(INFO) << "GetAssertResult_0100 end";
}
}
}