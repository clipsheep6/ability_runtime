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
#include "ability_manager_service.h"
#undef private
#undef protected

#include "application_util.h"
#include "bundlemgr/mock_bundle_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
static void WaitUntilTaskFinished()
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    auto handler = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

class AbilityManagerServiceCrowdtestTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
    inline static std::shared_ptr<AbilityManagerService> abilityMs_;
};

void AbilityManagerServiceCrowdtestTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceCrowdtestTest SetUpTestCase called";
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMs_->OnStart();
    WaitUntilTaskFinished();
}

void AbilityManagerServiceCrowdtestTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceCrowdtestTest TearDownTestCase called";
    abilityMs_->OnStop();
    OHOS::DelayedSingleton<SaMgrClient>::DestroyInstance();
    OHOS::DelayedSingleton<AbilityManagerService>::DestroyInstance();
}

void AbilityManagerServiceCrowdtestTest::SetUp()
{}

void AbilityManagerServiceCrowdtestTest::TearDown()
{}

/**
 * @tc.name: AbilityManagerServiceCrowdtestTest_IsCrowdtestExpired_002
 * @tc.desc: IsCrowtestExpired
 * @tc.type: FUNC
 * @tc.require: issueI5I0DY
 */
HWTEST_F(AbilityManagerServiceCrowdtestTest, IsCrowdtestExpired_001, TestSize.Level1)
{
    Want want;
    ElementName element("", "com.crowdtest.unexpired", "CrowdtestUnExpired");
    want.SetElement(element);
    auto result = AAFwk::ApplicationUtil::IsCrowdtestExpired(want, 0);
    EXPECT_FALSE(result);
}
}
}