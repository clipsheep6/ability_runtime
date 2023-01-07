/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "ability_manager_errors.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string DLP_PARAMS_INDEX = "ohos.dlp.params.index";
}
class PermissionVerificationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void PermissionVerificationTest::SetUpTestCase(void)
{}
void PermissionVerificationTest::TearDownTestCase(void)
{}
void PermissionVerificationTest::SetUp()
{}
void PermissionVerificationTest::TearDown()
{}

/**
 * @tc.name: StartUpAbilityPermissionCheck_0100
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, StartUpAbilityPermissionCheck_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUpAbilityPermissionCheck_0200
 * @tc.desc: StartUpAbilityPermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, StartUpAbilityPermissionCheck_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: StartUpServicePermissionCheck_0100
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: StartUpServicePermissionCheck_0200
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: StartUpServicePermissionCheck_0300
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallAbilityPermission_0100
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsGatewayCall_0100
 * @tc.desc: IsGatewayCall Test
 * @tc.type: FUNC
 * @tc.require: issueI65DZW
 */
HWTEST_F(PermissionVerificationTest, IsGatewayCall_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->IsGatewayCall();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CheckSpecificSystemAbilityAccessPermission_0100
 * @tc.desc: CheckSpecificSystemAbilityAccessPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5AFTJ
 */
HWTEST_F(PermissionVerificationTest, CheckSpecificSystemAbilityAccessPermission_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyControllerPerm_0100
 * @tc.desc: VerifyControllerPerm Test
 * @tc.type: FUNC
 * @tc.require: issueI4WS9F
 */
HWTEST_F(PermissionVerificationTest, VerifyControllerPerm_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyControllerPerm();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyDlpPermission_0100
 * @tc.desc: VerifyDlpPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI55UJZ
 */
HWTEST_F(PermissionVerificationTest, VerifyDlpPermission_0100, TestSize.Level0)
{
    Want want;
    want.SetParam(DLP_PARAMS_INDEX, 1);
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyDlpPermission(want);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyMissionPermission_0100
 * @tc.desc: VerifyMissionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5I8XP
 */
HWTEST_F(PermissionVerificationTest, VerifyMissionPermission_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyMissionPermission();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: VerifyInstallBundlePermission_0100
 * @tc.desc: VerifyInstallBundlePermission Test
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(PermissionVerificationTest, VerifyInstallBundlePermission_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyInstallBundlePermission();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: VerifyGetBundleInfoPrivilegedPermission_0100
 * @tc.desc: VerifyGetBundleInfoPrivilegedPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(PermissionVerificationTest, VerifyGetBundleInfoPrivilegedPermission_0100, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->VerifyGetBundleInfoPrivilegedPermission();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: CheckCallAbilityPermission_0200
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallAbilityPermission_0300
 * @tc.desc: CheckCallAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckStartByCallPermission_0100
 * @tc.desc: CheckStartByCallPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckStartByCallPermission_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckStartByCallPermission_0200
 * @tc.desc: CheckStartByCallPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckStartByCallPermission_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckStartByCallPermission_0300
 * @tc.desc: CheckStartByCallPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5QXCQ
 */
HWTEST_F(PermissionVerificationTest, CheckStartByCallPermission_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0100
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = true;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0200
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0300
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0400
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0400, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0500
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0500, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0600
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0600, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0700
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0700, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0800
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0800, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_0900
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_0900, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.visible = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallServiceAbilityPermission_1000
 * @tc.desc: CheckCallServiceAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceAbilityPermission_1000, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.visible = true;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallServiceExtensionPermission_0100
 * @tc.desc: CheckCallServiceExtensionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceExtensionPermission_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallServiceExtensionPermission_0200
 * @tc.desc: CheckCallServiceExtensionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceExtensionPermission_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallServiceExtensionPermission_0300
 * @tc.desc: CheckCallServiceExtensionPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallServiceExtensionPermission_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0100
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0100, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = true;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0200
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0200, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0300
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0300, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0400
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0400, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0500
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0500, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0600
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0600, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0700
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0700, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0800
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0800, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api9 = 9;
    verificationInfo.apiTargetVersion = api9;
    verificationInfo.isBackgroundCall = true;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_0900
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_0900, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = false;
    verificationInfo.isBackgroundCall = false;

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/**
 * @tc.name: CheckCallDataAbilityPermission_1000
 * @tc.desc: CheckCallDataAbilityPermission Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, CheckCallDataAbilityPermission_1000, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    int api8 = 8;
    verificationInfo.apiTargetVersion = api8;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    verificationInfo.associatedWakeUp = false;

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo);
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: JudgeCallerIsAllowedToUseSystemAPI_1000
 * @tc.desc: JudgeCallerIsAllowedToUseSystemAPI Test
 * @tc.type: FUNC
 * @tc.require: issuesI663VU
 */
HWTEST_F(PermissionVerificationTest, JudgeCallerIsAllowedToUseSystemAPI_1000, TestSize.Level0)
{
    bool result = AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI();
    EXPECT_EQ(result, true);
}
}  // namespace AAFwk
}  // namespace OHOS
