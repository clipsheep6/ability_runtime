/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "app_exit_reason_data_manager.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string BUNDLE_NAME = "bundle.name";
const std::string MODULE_NAME = "module_name";
const std::string ABILITY_NAME = "ability_name";
const int SESSION_ID = 111;
}  // namespace

class AppExitReasonDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonDataManagerTest::SetUpTestCase(void)
{}

void AppExitReasonDataManagerTest::TearDownTestCase(void)
{}

void AppExitReasonDataManagerTest::SetUp()
{
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME);
}

void AppExitReasonDataManagerTest::TearDown()
{}

/**
 * @tc.name: AppExitReasonDataManager_AddAbilityRecoverInfo_001
 * @tc.desc: AddAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_AddAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAbilityRecoverInfo_001
 * @tc.desc: DeleteAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001
 * @tc.desc: SetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(
    AppExitReasonDataManagerTest, AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001, TestSize.Level1)
{
    std::string bundleName = "com.test.demo";
    std::vector<std::string> extensionList;
    extensionList.push_back("testEntryUIExtAbility");
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetUIExtensionAbilityExitReason(
        bundleName, extensionList, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_GetUIExtensionAbilityExitReason_001
 * @tc.desc: GetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(
    AppExitReasonDataManagerTest, AppExitReasonDataManager_GetUIExtensionAbilityExitReason_001, TestSize.Level1)
{
    std::string keyEx = "com.test.demotestnoEntryUIExtAbility";
    AAFwk::ExitReason exitReason = { AAFwk::REASON_UNKNOWN, "" };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetUIExtensionAbilityExitReason(
        keyEx, exitReason);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: AppExitReasonDataManager_GetAbilityRecoverInfo_001
 * @tc.desc: GetAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAbilityRecoverInfo_001, TestSize.Level1)
{
    bool hasRecoverInfo = false;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(hasRecoverInfo, false);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(hasRecoverInfo, true);
}

/**
 * @tc.name: AppExitReasonDataManager_GetAbilitySessionId_001
 * @tc.desc: GetAbilitySessionId
 * @tc.type: FUNC
 * @tc.require: issuesI7R9JC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAbilitySessionId_001, TestSize.Level1)
{
    int sessionId = 0;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilitySessionId(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, sessionId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(sessionId, 0);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilitySessionId(
        BUNDLE_NAME, MODULE_NAME, ABILITY_NAME, sessionId);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(sessionId, SESSION_ID);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
