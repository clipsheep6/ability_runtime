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
HWTEST_F(PermissionVerificationTest, CheckCallAbilityPermission_0100, TestSize.Level0)
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
 * @tc.name: StartUpServicePermissionCheck_0400
 * @tc.desc: StartUpServicePermissionCheck Test
 * @tc.type: FUNC
 * @tc.require: issueI5PRJ7
 */
HWTEST_F(PermissionVerificationTest, StartUpServicePermissionCheck_0400, TestSize.Level0)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.visible = true;
    verificationInfo.isBackgroundCall = false;
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallOtherExtensionPermission(verificationInfo);
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
