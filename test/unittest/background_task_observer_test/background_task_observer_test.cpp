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
#define private public
#define protected public
#include "background_task_observer.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::BackgroundTaskMgr;

namespace OHOS {
namespace AAFwk {
class BackgroundTaskObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<BackgroundTaskObserver> observer_ {nullptr};
};

void BackgroundTaskObserverTest::SetUpTestCase(void)
{}
void BackgroundTaskObserverTest::TearDownTestCase(void)
{}
void BackgroundTaskObserverTest::TearDown(void)
{}
void BackgroundTaskObserverTest::SetUp()
{
    observer_ = std::make_shared<BackgroundTaskObserver>();
}

/*
 * Feature: BackgroundTaskObserver
 * Function: OnContinuousTaskStart
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver OnContinuousTaskStart
 * EnvConditions: NA
 * CaseDescription: Verify OnContinuousTaskStart
 */
HWTEST_F(BackgroundTaskObserverTest, OnContinuousTaskStart_001, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    observer_->OnContinuousTaskStart(info);
}

/*
 * Feature: BackgroundTaskObserver
 * Function: OnContinuousTaskStart
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver OnContinuousTaskStart
 * EnvConditions: NA
 * CaseDescription: Verify OnContinuousTaskStart
 */
HWTEST_F(BackgroundTaskObserverTest, OnContinuousTaskStart_002, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    observer_->GetAppManager();
    observer_->OnContinuousTaskStart(info);
}

/*
 * Feature: BackgroundTaskObserver
 * Function: OnContinuousTaskStop
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver OnContinuousTaskStop
 * EnvConditions: NA
 * CaseDescription: Verify OnContinuousTaskStop
 */
HWTEST_F(BackgroundTaskObserverTest, OnContinuousTaskStop_001, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    observer_->OnContinuousTaskStop(info);
}

/*
 * Feature: BackgroundTaskObserver
 * Function: OnContinuousTaskStop
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver OnContinuousTaskStop
 * EnvConditions: NA
 * CaseDescription: Verify OnContinuousTaskStop
 */
HWTEST_F(BackgroundTaskObserverTest, OnContinuousTaskStop_002, TestSize.Level1)
{
    std::shared_ptr<ContinuousTaskCallbackInfo> info = std::make_shared<ContinuousTaskCallbackInfo>();
    observer_->GetAppManager();
    observer_->OnContinuousTaskStop(info);
}

/*
 * Feature: BackgroundTaskObserver
 * Function: GetContinuousTaskApps
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver GetContinuousTaskApps
 * EnvConditions: NA
 * CaseDescription: Verify GetContinuousTaskApps
 */
HWTEST_F(BackgroundTaskObserverTest, GetContinuousTaskApps_001, TestSize.Level1)
{
    observer_->GetContinuousTaskApps();
}

/*
 * Feature: BackgroundTaskObserver
 * Function: IsBackgroundTaskUid
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver IsBackgroundTaskUid
 * EnvConditions: NA
 * CaseDescription: Verify IsBackgroundTaskUid
 */
HWTEST_F(BackgroundTaskObserverTest, IsBackgroundTaskUid_001, TestSize.Level1)
{
    int uid = 0;
    bool res = observer_->IsBackgroundTaskUid(uid);
    EXPECT_FALSE(res);
}

/*
 * Feature: BackgroundTaskObserver
 * Function: IsBackgroundTaskUid
 * SubFunction: NA
 * FunctionPoints: BackgroundTaskObserver IsBackgroundTaskUid
 * EnvConditions: NA
 * CaseDescription: Verify IsBackgroundTaskUid
 */
HWTEST_F(BackgroundTaskObserverTest, IsBackgroundTaskUid_002, TestSize.Level1)
{
    int uid = 0;
    observer_->bgTaskUids_.push_front(uid);
    bool res = observer_->IsBackgroundTaskUid(uid);
    EXPECT_TRUE(res);
}
}  // namespace AAFwk
}  // namespace OHOS
