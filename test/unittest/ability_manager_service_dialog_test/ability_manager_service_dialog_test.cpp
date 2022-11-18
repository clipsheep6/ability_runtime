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
#include "implicit_start_processor.h"
#include "ability_info.h"
#include "ability_record.h"
#include "system_dialog_scheduler.h"
#undef private
#include "ability_manager_errors.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
const int32_t DEFAULT_USERID = 100;

namespace OHOS {
namespace AAFwk  {
class AbilityMgrServiceDialogTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ImplicitStartProcessor> implicitStartProcessor_;
    std::shared_ptr<SystemDialogScheduler> systemDialogScheduler_;
    AbilityRequest abilityRequest_;
};

void AbilityMgrServiceDialogTest::SetUpTestCase(void) {}

void AbilityMgrServiceDialogTest::TearDownTestCase(void) {}

void AbilityMgrServiceDialogTest::SetUp()
{
    int testCode = 123;
    abilityRequest_.requestCode = testCode;

    abilityRequest_.abilityInfo.package = "test";
    abilityRequest_.abilityInfo.name = "test";
    abilityRequest_.abilityInfo.label = "test";
    abilityRequest_.abilityInfo.description = "test";
    abilityRequest_.abilityInfo.iconPath = "/test";
    abilityRequest_.abilityInfo.visible = false;
    abilityRequest_.abilityInfo.kind = "page";
    abilityRequest_.abilityInfo.permissions = {};
    abilityRequest_.abilityInfo.bundleName = "test";
    abilityRequest_.abilityInfo.applicationName = "test";
    abilityRequest_.abilityInfo.deviceId = "test";
    abilityRequest_.abilityInfo.codePath = "/test";
    abilityRequest_.abilityInfo.resourcePath = "/test";
    abilityRequest_.abilityInfo.libPath = "/test";

    abilityRequest_.appInfo.name = "test";
    abilityRequest_.appInfo.bundleName = "test";
    abilityRequest_.appInfo.deviceId = "test";
    abilityRequest_.appInfo.signatureKey = "test";

    implicitStartProcessor_ = std::make_shared<ImplicitStartProcessor>();
    systemDialogScheduler_ = std::make_shared<SystemDialogScheduler>();
}

void AbilityMgrServiceDialogTest::TearDown() {}

/*
 * @tc.number    : AbilityMgrServiceDialog_0100
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test TipsDialog
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityMgrServiceDialog_0100 start");
    auto ret = implicitStartProcessor_->ImplicitStartAbility(abilityRequest_, DEFAULT_USERID);
    EXPECT_EQ(ret, ERR_IMPLICIT_START_ABILITY_FAIL);
    HILOG_INFO("AbilityMgrServiceDialog_0100 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0200
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test GetTipsDialogWant
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0200, TestSize.Level1)
{
    HILOG_INFO("AbilityMgrServiceDialog_0200 start");
    auto want = systemDialogScheduler_->GetTipsDialogWant();
    EXPECT_EQ(want.GetElement().GetBundleName(), "com.ohos.amsdialog");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "TipsDialog");
    HILOG_INFO("AbilityMgrServiceDialog_0200 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0300
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test GetSelectorDialogWant
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0300, TestSize.Level1)
{
    HILOG_INFO("AbilityMgrServiceDialog_0300 start");
    std::vector<DialogAppInfo> dialogAppInfos;
    Want targetWant;
    auto want = systemDialogScheduler_->GetSelectorDialogWant(dialogAppInfos, targetWant);
    EXPECT_EQ(want.GetElement().GetBundleName(), "com.ohos.amsdialog");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "SelectorDialog");
    HILOG_INFO("AbilityMgrServiceDialog_0300 end");
}
}  // namespace AAFwk
}  // namespace OHOS