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

#include <cstdlib>
#include <gtest/gtest.h>
#include <memory>

#define private public
#define proteced public
#include "app_mgr_proxy.h"
#include "app_mgr_stub.h"
#include "assert_fault_task_thread.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "process_info.h"
#include "quick_fix_callback_stub.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "ohos_application.h"
#undef private
#undef proteced

using namespace testing;
using namespace testing::ext;
using namespace std;

namespace OHOS{
namespace AbilityRuntime{
 

class AssertFaultTaskThreadTest : public testing::Test{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    shared_ptr<AssertFaultTaskThread> assertFaultTaskThread_ = nullptr;
};

void AssertFaultTaskThreadTest::SetUpTestCase()
{}

void AssertFaultTaskThreadTest::TearDownTestCase()
{}

void AssertFaultTaskThreadTest::SetUp()
{
    assertFaultTaskThread_ = 
        DelayedSingleton<AbilityRuntime::AssertFaultTaskThread>::GetInstance();
}

void AssertFaultTaskThreadTest::TearDown()
{}

/*
 * Feature: AssertFaultTaskThreadTest
 * Function: InitAssertFaultTask
 * SubFunction: NA
 * FunctionPoints: AssertFaultTaskThreadTest InitAssertFaultTask
 * EnvConditions: NA
 * CaseDescription: Verify InitAssertFaultTask
 */
HWTEST_F(AssertFaultTaskThreadTest, InitAssertFaultTask_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    const wptr<AppExecFwk::MainThread> weak;
    bool isDebugModule = true;
    assertFaultTaskThread_->InitAssertFaultTask(weak, isDebugModule);
    EXPECT_TRUE(assertFaultTaskThread_->isDebugModule_);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: AssertFaultTaskThreadTest
 * Function: NotifyReleaseLongWaiting
 * SubFunction: NA
 * FunctionPoints: AssertFaultTaskThreadTest NotifyReleaseLongWaiting
 * EnvConditions: NA
 * CaseDescription: Verify NotifyReleaseLongWaiting
 */
HWTEST_F(AssertFaultTaskThreadTest, NotifyReleaseLongWaiting_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    assertFaultTaskThread_->NotifyReleaseLongWaiting();
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: AssertFaultTaskThreadTest
 * Function: Stop
 * SubFunction: NA
 * FunctionPoints: AssertFaultTaskThreadTest Stop
 * EnvConditions: NA
 * CaseDescription: Verify Stop
 */
HWTEST_F(AssertFaultTaskThreadTest, Stop_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    assertFaultTaskThread_->Stop();
    EXPECT_EQ(assertFaultTaskThread_->assertRunner_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: AssertFaultTaskThreadTest
 * Function: RequestAssertResult
 * SubFunction: NA
 * FunctionPoints: AssertFaultTaskThreadTest RequestAssertResult
 * EnvConditions: NA
 * CaseDescription: Verify RequestAssertResult
 */
HWTEST_F(AssertFaultTaskThreadTest, RequestAssertResult_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    AssertFailureInfo assertFail;
    std::string textFile = std::string("File:\n") +
        (assertFail.file == nullptr ? "Unknown" : std::string(assertFail.file));
    std::string textFunc = std::string("\nFunction: ") +
        (assertFail.function == nullptr ? "Unknown" : std::string(assertFail.function));
    std::string textLine = std::string("\nLine: ") + std::to_string(assertFail.line);
    std::string textExpression = std::string("\n\nExpression:\n") +
        (assertFail.expression == nullptr ? "Unknown" : std::string(assertFail.expression));
    std::string textDetail = textFile + textFunc + textLine + textExpression + "\n\n(Press Retry to debug the application)";
    
    auto result = assertFaultTaskThread_->RequestAssertResult(textDetail);
    EXPECT_EQ(result, ERR_OK);
    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS
