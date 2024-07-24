/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "extension_permissions_util.h"
#include "hilog_tag_wrapper.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::ExtensionAbilityType;

namespace OHOS {
namespace AAFwk {
class ExtensionPermissionsUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ExtensionPermissionsUtilTest::SetUpTestCase() {}

void ExtensionPermissionsUtilTest::TearDownTestCase() {}

void ExtensionPermissionsUtilTest::SetUp() {}

void ExtensionPermissionsUtilTest::TearDown() {}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_001 start");
    MyFlag::flag_ = 0;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::FORM));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_001 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::FORM));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermission
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 start");

    MyFlag::flag_ = 1;
    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermission(ExtensionAbilityType::FORM));
    MyFlag::flag_ = 0;
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermission_002 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 start");

    MyFlag::hasPerm_ = true;
    EXPECT_TRUE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::FILEACCESS_EXTENSION));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 end");
}

/*
 * Feature: ExtensionPermissionsUtil
 * Function: CheckSAPermission
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService CheckSAPermissionMore
 */
HWTEST_F(ExtensionPermissionsUtilTest, CheckSAPermissionMore_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 start");

    MyFlag::hasPerm_ = false;
    EXPECT_FALSE(ExtensionPermissionsUtil::CheckSAPermissionMore(ExtensionAbilityType::FILEACCESS_EXTENSION));
    TAG_LOGI(AAFwkTag::TEST, "ExtensionPermissionsUtilTest CheckSAPermissionMore_001 end");
}
} // namespace AAFwk
} // namespace OHOS
