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

#include <gtest/gtest.h>

#define private public
#include "app_lifecycle_deal.h"
#include "mock_app_scheduler.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppLifecycleDealTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppLifecycleDealTest::SetUpTestCase(void)
{}

void AppLifecycleDealTest::TearDownTestCase(void)
{}

void AppLifecycleDealTest::SetUp()
{}

void AppLifecycleDealTest::TearDown()
{}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, NotifyAppFault_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    FaultData faultData;
    int32_t result = appLifeCycle->NotifyAppFault(faultData);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyAppFault_002
 * @tc.desc: Verify that the NotifyAppFault interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, NotifyAppFault_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    FaultData faultData;
    auto retsult = appLifeCycle->NotifyAppFault(faultData);
    EXPECT_EQ(ERR_OK, retsult);
}
} // namespace AppExecFwk
} // namespace OHOS
