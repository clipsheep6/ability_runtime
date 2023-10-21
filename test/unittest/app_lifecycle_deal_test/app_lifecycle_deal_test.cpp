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

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: Test the normal state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, AttachAppDebug_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    auto result = appLifeCycle->AttachAppDebug();
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AttachAppDebug_002
 * @tc.desc: Test the abnormal state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, AttachAppDebug_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    auto result = appLifeCycle->AttachAppDebug();
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: DetachAppDebug_001
 * @tc.desc: Test the abnormal state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, DetachAppDebug_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    auto result = appLifeCycle->DetachAppDebug();
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DetachAppDebug_002
 * @tc.desc: Test the normal state of DetachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, DetachAppDebug_002, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appLifeCycle, nullptr);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    auto result = appLifeCycle->DetachAppDebug();
    EXPECT_EQ(ERR_OK, result);
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Verify that the ChangeAppGcState interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(AppLifecycleDealTest, ChangeAppGcState_001, TestSize.Level1)
{
    auto appLifeCycle = std::make_shared<AppLifeCycleDeal>();
    int32_t result = appLifeCycle->ChangeAppGcState(0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    sptr<MockAppScheduler> mockAppScheduler = new (std::nothrow) MockAppScheduler();
    appLifeCycle->SetApplicationClient(mockAppScheduler);
    int32_t result1 = appLifeCycle->ChangeAppGcState(0);
    EXPECT_EQ(ERR_OK, result1);
}
} // namespace AppExecFwk
} // namespace OHOS
