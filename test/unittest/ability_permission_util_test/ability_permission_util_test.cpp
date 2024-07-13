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

#include "mock_permission_verification.h"

#define private public
#define protected public
#include "utils/ability_permission_util.h"
#include "ability_connect_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "app_utils.h"
#include "connection_observer_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_ability_token.h"
#include "mock_ability_controller.h"
#include "mock_app_debug_listener_stub.h"
#include "mock_ability_manager_collaborator.h"
#include "mock_prepare_terminate_callback.h"
#include "mock_sa_call.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
}  // namespace
class AbilityPermissionUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

std::shared_ptr<AbilityRecord> AbilityPermissionUtilTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AbilityPermissionUtilTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> AbilityPermissionUtilTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void AbilityPermissionUtilTest::SetUpTestCase() {}

void AbilityPermissionUtilTest::TearDownTestCase() {}

void AbilityPermissionUtilTest::SetUp() {}

void AbilityPermissionUtilTest::TearDown() {}

/*
 * Feature: AbilityPermissionUtil
 * Function: IsCallFromBackground
 * SubFunction: NA
 * FunctionPoints: AbilityPermissionUtil IsCallFromBackground
 */
HWTEST_F(AbilityPermissionUtilTest, IsCallFromBackground_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtilTest IsCallFromBackground_001 start");
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto util = std::make_shared<AbilityPermissionUtil>();
    AbilityRequest abilityRequest;
    bool isBackgroundCall = true;
    bool backgroundJudgeFlag = true;

    // IsSACall
    MyFlag::flag_ = 1;
    EXPECT_TRUE(util->IsCallFromBackground(abilityRequest, isBackgroundCall, backgroundJudgeFlag));

    // IsShellCall
    MyFlag::flag_ = 2;
    EXPECT_TRUE(util->IsCallFromBackground(abilityRequest, isBackgroundCall, backgroundJudgeFlag));

    // IsStartFreeInstall
    unsigned int flag = 0x00000800;
    abilityRequest.want.SetFlags(flag);
    EXPECT_TRUE(util->IsCallFromBackground(abilityRequest, isBackgroundCall, backgroundJudgeFlag));

    // set abilityRequest.callerToken and abilityRequest.want
    MyFlag::flag_ = 0;
    abilityRequest.callerToken = nullptr;
    abilityRequest.want.SetFlags(0);
    EXPECT_TRUE(util->IsCallFromBackground(abilityRequest, isBackgroundCall, backgroundJudgeFlag));

    abilityRequest.callerToken = MockToken(AbilityType::PAGE);
    abilityRequest.want.SetParam("isDelegatorCall", true);
    EXPECT_TRUE(util->IsCallFromBackground(abilityRequest, isBackgroundCall, backgroundJudgeFlag));

    abilityRequest.callerToken = nullptr;
    abilityRequest.want.SetParam("isDelegatorCall", true);
    EXPECT_TRUE(util->IsCallFromBackground(abilityRequest, isBackgroundCall, backgroundJudgeFlag));
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtilTest IsCallFromBackground_001 end");
}

/*
 * Feature: AbilityPermissionUtil
 * Function: CheckStaticCfgPermission
 * SubFunction: NA
 * FunctionPoints: AbilityPermissionUtil CheckStaticCfgPermission
 */
HWTEST_F(AbilityPermissionUtilTest, CheckStaticCfgPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtilTest CheckStaticCfgPermission_001 start");
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto util = std::make_shared<AbilityPermissionUtil>();
    AppExecFwk::AbilityRequest abilityRequest;
    MyFlag::flag_ = 1;
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    MyFlag::flag_ = 0;
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    abilityRequest.abilityInfo.applicationInfo.accessTokenId = 0;
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1), ERR_OK);

    // abilityInfo.permissions is empty
    abilityRequest.abilityInfo.applicationInfo.accessTokenId = -1;
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_GRANTED);

    // abilityInfo.permissions is not empty
    abilityRequest.abilityInfo.permissions.push_back("test1");
    abilityRequest.abilityInfo.permissions.push_back("test2");
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1), ERR_OK);

    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = ExtensionAbilityType::DATASHARE;
    abilityRequest.abilityInfo.readPermission = "test";
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    abilityRequest.abilityInfo.readPermission.clear();
    abilityRequest.abilityInfo.writePermission = "test";
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);

    Skill skill;
    skill.permissions.push_back("test");
    abilityRequest.abilityInfo.skills.push_back(skill);
    EXPECT_EQ(util->CheckStaticCfgPermission(abilityRequest, false, -1, false, false, true),
        AppExecFwk::Constants::PERMISSION_NOT_GRANTED);
    TAG_LOGI(AAFwkTag::TEST, "AbilityPermissionUtilTest CheckStaticCfgPermission_001 end");
}
} // namespace AAFwk
} // namespace OHOS
