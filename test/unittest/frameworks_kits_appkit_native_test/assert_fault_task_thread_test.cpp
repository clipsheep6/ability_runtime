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
#include "assert_fault_task_thread.h"
#include "main_thread.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class AssertFaultTaskThreadTest : public testing::Test {
public:
    AssertFaultTaskThreadTest()
    {}
    ~AssertFaultTaskThreadTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AssertFaultTaskThreadTest::SetUpTestCase(void)
{}

void AssertFaultTaskThreadTest::TearDownTestCase(void)
{}

void AssertFaultTaskThreadTest::SetUp(void)
{}

void AssertFaultTaskThreadTest::TearDown(void)
{}

/**
 * @tc.number: RequestAssertResult_0100
 * @tc.name: RequestAssertResult
 * @tc.desc: Test whether RequestAssertResult and are called normally.
 */
HWTEST_F(AssertFaultTaskThreadTest, RequestAssertResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RequestAssertResult_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();
    AAFwk::UserStatus status = assertThread->RequestAssertResult("RequestAssertResult string");
    const AAFwk::UserStatus ASSERT_FAULT_DEFAULT_VALUE = AAFwk::UserStatus::ASSERT_TERMINATE;
    EXPECT_EQ(status, ASSERT_FAULT_DEFAULT_VALUE);

    GTEST_LOG_(INFO) << "RequestAssertResult_0100 end";
}

/**
 * @tc.number: InitAssertFaultTask_0100
 * @tc.name: InitAssertFaultTask
 * @tc.desc: Test whether InitAssertFaultTask and are called normally.
 */
HWTEST_F(AssertFaultTaskThreadTest, InitAssertFaultTask_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "InitAssertFaultTask_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();
    auto mainThread = wptr<OHOS::AppExecFwk::MainThread>(new (std::nothrow) OHOS::AppExecFwk::MainThread());
    EXPECT_NE(mainThread, nullptr);
    assertThread->InitAssertFaultTask(mainThread, true);

    GTEST_LOG_(INFO) << "InitAssertFaultTask_0100 end";
}

/**
 * @tc.number: Stop_0100
 * @tc.name: Stop
 * @tc.desc: Test whether Stop and are called normally.
 */
HWTEST_F(AssertFaultTaskThreadTest, Stop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Stop_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();
    EXPECT_NE(assertThread, nullptr);
    assertThread->Stop();

    auto runner = AppExecFwk::EventRunner::Create("assertFaultTHR");
    assertThread->assertHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    EXPECT_NE(assertThread->assertHandler_, nullptr);
    assertThread->Stop();

    GTEST_LOG_(INFO) << "Stop_0100 end";
}

/**
 * @tc.number: HandleAssertCallback_0100
 * @tc.name: HandleAssertCallback
 * @tc.desc: Test whether HandleAssertCallback and are called normally.
 */
HWTEST_F(AssertFaultTaskThreadTest, HandleAssertCallback_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "HandleAssertCallback_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();
    std::string exprStr = "RequestAssertResult string";
    AAFwk::UserStatus status = assertThread->HandleAssertCallback(exprStr);
    const AAFwk::UserStatus ASSERT_FAULT_DEFAULT_VALUE = AAFwk::UserStatus::ASSERT_TERMINATE;
    EXPECT_EQ(status, ASSERT_FAULT_DEFAULT_VALUE);

    GTEST_LOG_(INFO) << "HandleAssertCallback_0100 end";
}

/**
 * @tc.number: NotifyReleaseLongWaiting_0100
 * @tc.name: NotifyReleaseLongWaiting
 * @tc.desc: Test whether NotifyReleaseLongWaiting and are called normally.
 */
HWTEST_F(AssertFaultTaskThreadTest, NotifyReleaseLongWaiting_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "NotifyReleaseLongWaiting_0100 start";
    auto assertThread = std::make_shared<AssertFaultTaskThread>();
    EXPECT_NE(assertThread, nullptr);

    assertThread->NotifyReleaseLongWaiting();
    GTEST_LOG_(INFO) << "NotifyReleaseLongWaiting_0100 end";
}
}
}