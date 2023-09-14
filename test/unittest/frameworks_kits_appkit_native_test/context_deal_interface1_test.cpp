/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "gmock/gmock.h"
#include <gtest/gtest.h>
#include <singleton.h>

#include "ability_handler.h"
#include "ability_info.h"
#include "ability.h"
#include "context_deal.h"
#include "ohos_application.h"

#include "mock_ability_manager_client_interface1.h"
#include "mock_resourceManager_interface1.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class ContextDealInterfaceTest : public testing::Test {
public:
    ContextDealInterfaceTest()
    {}
    ~ContextDealInterfaceTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextDealInterfaceTest::SetUpTestCase(void)
{}
void ContextDealInterfaceTest::TearDownTestCase(void)
{}
void ContextDealInterfaceTest::SetUp()
{}
void ContextDealInterfaceTest::TearDown()
{}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    const std::string testValue = "./1234test/preferences";
    abilityInfo->name = "test1234.1234test";
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0100 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0101
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPreferencesDir_0101, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0101 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    const std::string testValue = "./1234test/preferences";
    abilityInfo->name = "1234test";
    abilityInfo->isNativeAbility = true;
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0101 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetPreferencesDir_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    const std::string testValue = "";
    abilityInfo->name = "1234test";
    abilityInfo->isNativeAbility = false;
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    EXPECT_STREQ(testValue.c_str(), contextDeal->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetPreferencesDir_0200 end";
}

/**
 * @tc.number: AppExecFwk_ContextDeal_GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetDisplayOrientation is correct.
 */
HWTEST_F(ContextDealInterfaceTest, AppExecFwk_ContextDeal_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetDisplayOrientation_0100 start";
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    int testValue = 1;
    abilityInfo->orientation = static_cast<DisplayOrientation>(testValue);
    contextDeal->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(testValue, contextDeal->GetDisplayOrientation());
    GTEST_LOG_(INFO) << "AppExecFwk_ContextDeal_GetDisplayOrientation_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
