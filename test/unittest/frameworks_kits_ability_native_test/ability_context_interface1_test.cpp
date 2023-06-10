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

#include <singleton.h>
#include <gtest/gtest.h>
#include "gmock/gmock.h"

#include "ability_context.h"
#include "ohos_application.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability.h"

#include "mock_ability_manager_client_interface1.h"
#include "mock_resourceManager_interface1.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class AbilityContextInterfaceTest : public testing::Test {
public:
    AbilityContextInterfaceTest() : context_(nullptr)
    {}
    ~AbilityContextInterfaceTest()
    {}
    std::unique_ptr<OHOS::AppExecFwk::AbilityContext> context_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityContextInterfaceTest::SetUpTestCase(void)
{}
void AbilityContextInterfaceTest::TearDownTestCase(void)
{}
void AbilityContextInterfaceTest::SetUp()
{}
void AbilityContextInterfaceTest::TearDown()
{}

/**
 * @tc.number: AaFwk_AbilityContext_TerminateAbilityResult_0100
 * @tc.name: TerminateAbilityResult
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbilityResult is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_TerminateAbilityResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbilityResult_0100 start";
    const ErrCode testValue = ERR_OK;
    OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->SetTerminateAbilityResult(testValue);
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    EXPECT_TRUE(abilityContext->TerminateAbilityResult(0));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbilityResult_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_TerminateAbilityResult_0200
 * @tc.name: TerminateAbilityResult
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbilityResult is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_TerminateAbilityResult_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbilityResult_0200 start";
    const ErrCode testValue = ERR_NO_INIT;
    OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->SetTerminateAbilityResult(testValue);
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    EXPECT_FALSE(abilityContext->TerminateAbilityResult(0));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbilityResult_0200 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_TerminateAbilityResult_0300
 * @tc.name: TerminateAbilityResult
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbilityResult is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_TerminateAbilityResult_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbilityResult_0300 start";
    OHOS::AAFwk::MockAbilityManagerClient::SetInstanceNull(false);
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    EXPECT_FALSE(abilityContext->TerminateAbilityResult(0));
    OHOS::AAFwk::MockAbilityManagerClient::SetInstanceNull(true);
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbilityResult_0300 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_StartAbilities_0100
 * @tc.name: StartAbilities
 * @tc.desc: Test the attachbasecontext call to verify that the return value of StartAbilities is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_StartAbilities_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_StartAbilities_0100 start";
    std::shared_ptr<MockAbilityContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    std::shared_ptr<MockAbilityContextTest> abilityContext = std::make_shared<MockAbilityContextTest>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = OHOS::AppExecFwk::AbilityType::DATA;
    contextDeal->SetAbilityInfo(abilityInfo);
    abilityContext->AttachBaseContext(contextDeal);
    std::vector<AAFwk::Want> testlist = { AAFwk::Want(), AAFwk::Want() };
    abilityContext->StartAbilities(testlist);
    EXPECT_TRUE((testlist.size() == abilityContext->GetStartAbilityRunCount()));
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_StartAbilities_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetPreferencesDir is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetPreferencesDir_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<MockAbilityContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    const std::string testValue = "./1234test/preferences";
    abilityInfo->name = "test1234.1234test";
    applicationInfo->dataDir = ".";
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(applicationInfo);
    abilityContext->AttachBaseContext(contextDeal);
    EXPECT_STREQ(testValue.c_str(), abilityContext->GetPreferencesDir().c_str());
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetPreferencesDir_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_GetString_ByIdAndFormat_0100
 * @tc.name: GetString
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetString is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_GetString_ByIdAndFormat_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetString_ByIdAndFormat_0100 start";
    std::shared_ptr<Global::Resource::ResourceManager2> resourceManager(Global::Resource::CreateResourceManager2());
    std::shared_ptr<MockAbilityContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    ASSERT_NE(abilityContext, nullptr);
    std::string testByName = "";
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetString_ByIdAndFormat_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_GetThemeId_0100
 * @tc.name: GetThemeId
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetThemeId is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_GetThemeId_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetThemeId_0100 start";
    std::shared_ptr<MockAbilityContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    ASSERT_NE(abilityContext, nullptr);
    abilityContext->AttachBaseContext(contextDeal);
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetThemeId_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_GetDisplayOrientation_0100
 * @tc.name: GetDisplayOrientation
 * @tc.desc: Test the attachbasecontext call to verify that the return value of GetDisplayOrientation is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_GetDisplayOrientation_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetDisplayOrientation_0100 start";
    std::shared_ptr<MockAbilityContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    int testValue = 1;
    abilityInfo->orientation = static_cast<DisplayOrientation>(testValue);
    contextDeal->SetAbilityInfo(abilityInfo);
    abilityContext->AttachBaseContext(contextDeal);
    EXPECT_EQ(testValue, abilityContext->GetDisplayOrientation());
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetDisplayOrientation_0100 end";
}

/**
 * @tc.number: AaFwk_AbilityContext_TerminateAbility_0100
 * @tc.name: TerminateAbility
 * @tc.desc: Test the attachbasecontext call to verify that the return value of TerminateAbility is correct.
 */
HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_TerminateAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbility_0100 start";
    const ErrCode mockValue = ERR_OK;
    int testValue = 0;
    OHOS::AAFwk::MockAbilityManagerClient::GetInstance()->SetTerminateAbility(mockValue);
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    abilityContext->TerminateAbility(testValue);
    EXPECT_EQ(testValue, AAFwk::MockAbilityManagerClient::GetInstance()->GetTerminateAbilityValue());
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_TerminateAbility_0100 end";
}

HWTEST_F(AbilityContextInterfaceTest, AaFwk_AbilityContext_GetMissionId_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetMissionId_0100 start";
    std::shared_ptr<AbilityContext> abilityContext = std::make_shared<AbilityContext>();
    std::shared_ptr<MockAbilityContextDeal> contextDeal = std::make_shared<MockAbilityContextDeal>();
    const int testValue = 1;

    EXPECT_NE(testValue, abilityContext->GetMissionId());

    abilityContext->AttachBaseContext(contextDeal);

    EXPECT_NE(testValue, abilityContext->GetMissionId());

    AAFwk::LifeCycleStateInfo lifeInfo;
    lifeInfo.missionId = testValue;
    contextDeal->SetLifeCycleStateInfo(lifeInfo);

    EXPECT_EQ(testValue, abilityContext->GetMissionId());

    GTEST_LOG_(INFO) << "AaFwk_AbilityContext_GetMissionId_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
