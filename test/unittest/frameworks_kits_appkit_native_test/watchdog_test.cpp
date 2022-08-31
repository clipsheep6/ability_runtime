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
#include <thread>

#include "main_thread.h"
#include "mock_app_thread.h"
#include "watchdog.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class WatchdogTest : public testing::Test {
public:
    WatchdogTest()
    {}
    ~WatchdogTest()
    {}
    std::shared_ptr<MockHandler> mockHandler_ = nullptr;
    std::shared_ptr<EventRunner> runner_ = nullptr;
    std::shared_ptr<Watchdog> watchdog_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WatchdogTest::SetUpTestCase(void)
{}

void WatchdogTest::TearDownTestCase(void)
{}

void WatchdogTest::SetUp(void)
{
    runner_ = EventRunner::Create("");
    mockHandler_ = std::make_shared<MockHandler>(runner_);

    watchdog_ = std::make_shared<Watchdog>();
    watchdog_->Init(mockHandler_);
}

void WatchdogTest::TearDown(void)
{
    watchdog_->Stop();
}

/**
 * @tc.number: AppExecFwk_Watchdog_IsReportEvent_0001
 * @tc.name: IsReportEvent
 * @tc.desc: Test the abnormal state of IsReportEvent.
 */
HWTEST_F(WatchdogTest, AppExecFwk_Watchdog_IsReportEvent_0001, Function | MediumTest | Level3)
{
    bool ret = watchdog_->IsReportEvent();
    EXPECT_FALSE(ret);
}

/**
 * @tc.number: AppExecFwk_Watchdog_IsReportEvent_0002
 * @tc.name: IsReportEvent
 * @tc.desc: Test the change state of IsReportEvent.
 */
HWTEST_F(WatchdogTest, AppExecFwk_Watchdog_IsReportEvent_0002, Function | MediumTest | Level3)
{
    watchdog_->SetAppMainThreadState(true);
    watchdog_->AllowReportEvent();
    bool ret = watchdog_->IsReportEvent();
    EXPECT_TRUE(ret);
}
}  // namespace AppExecFwk
}  // namespace OHOS
