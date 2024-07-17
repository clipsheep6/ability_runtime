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
#define protected public
#include "ability_info.h"
#include "ability_record.h"
#include "implicit_start_processor.h"
#include "system_dialog_scheduler.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;
const int32_t DEFAULT_USERID = 100;

namespace OHOS {
namespace AAFwk {
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
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0100 start");
    auto ret = implicitStartProcessor_->ImplicitStartAbility(abilityRequest_, DEFAULT_USERID);
    EXPECT_EQ(ret, ERR_IMPLICIT_START_ABILITY_FAIL);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0100 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0200
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test GetTipsDialogWant
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0200 start");
    auto want = systemDialogScheduler_->GetTipsDialogWant(nullptr);
    EXPECT_EQ(want.GetElement().GetBundleName(), "com.ohos.amsdialog");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "TipsDialog");
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0200 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0300
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test GetSelectorDialogWant
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0300 start");
    std::vector<DialogAppInfo> dialogAppInfos;
    Want requestWant;
    Want targetWant;
    auto ret = systemDialogScheduler_->GetSelectorDialogWant(dialogAppInfos, requestWant, targetWant, nullptr);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(requestWant.GetElement().GetBundleName(), "com.ohos.amsdialog");
        EXPECT_EQ(requestWant.GetElement().GetAbilityName(), "SelectorDialog");
    }
    EXPECT_NE(systemDialogScheduler_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0300 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0400
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test GetSelectorParams
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0400 start");
    std::vector<DialogAppInfo> dialogAppInfos;
    auto params = systemDialogScheduler_->GetSelectorParams(dialogAppInfos);
    EXPECT_EQ(params.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0400 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0500
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test GetSelectorParams
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0500 start");
    DialogAppInfo dialogAppInfo = {
        0, 0, 0, 0, "com.example.test", "MainAbility", "entry"
    };
    std::vector<DialogAppInfo> dialogAppInfos = { dialogAppInfo };
    auto params = systemDialogScheduler_->GetSelectorParams(dialogAppInfos);
    nlohmann::json jsonObj = nlohmann::json::parse(params);
    EXPECT_EQ(jsonObj["hapList"].size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0500 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0600
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test IsImplicitStartAction
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0600 start");
    Want want;
    want.SetElementName("com.example.test", "");
    auto ret = implicitStartProcessor_->IsImplicitStartAction(want);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0600 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0700
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test IsImplicitStartAction
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0700 start");
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    auto ret = implicitStartProcessor_->IsImplicitStartAction(want);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0700 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0800
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test IsImplicitStartAction
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0800 start");
    Want want;
    want.SetElementName("com.example.test", "");
    want.SetAction("ohos.want.action.select");
    auto ret = implicitStartProcessor_->IsImplicitStartAction(want);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0800 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialog_0900
 * @tc.name      : AbilityMgrServiceDialog
 * @tc.desc      : 1.Test IsImplicitStartAction
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialog_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0900 start");
    Want want;
    want.SetElementName("com.example.test", "");
    want.SetAction("ohos.want.action.viewData");
    auto ret = implicitStartProcessor_->IsImplicitStartAction(want);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialog_0900 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1000
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1000 start");
    DialogPosition position;
    position.height = 0;
    const float densityPixels = 1.50;
    const int lineNums = 9;
    systemDialogScheduler_->DialogPortraitPositionAdaptive(position, densityPixels, lineNums);
    EXPECT_NE(position.height, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1000 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1100
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1100 start");
    DialogPosition position;
    position.height = 0;
    const float densityPixels = 1.50;
    const int lineNums = 5;
    systemDialogScheduler_->DialogPortraitPositionAdaptive(position, densityPixels, lineNums);
    EXPECT_NE(position.height, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1100 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1200
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1200 start");
    DialogPosition position;
    position.height = 0;
    const float densityPixels = 1.50;
    const int lineNums = 1;
    systemDialogScheduler_->DialogPortraitPositionAdaptive(position, densityPixels, lineNums);
    EXPECT_NE(position.height, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1200 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1300
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1300 start");
    DialogPosition position;
    position.height = 0;
    const float densityPixels = 1.50;
    const int lineNums = 9;
    systemDialogScheduler_->DialogLandscapePositionAdaptive(position, densityPixels, lineNums);
    EXPECT_NE(position.height, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1300 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1400
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1400 start");
    DialogPosition position;
    position.height = 0;
    const float densityPixels = 1.50;
    const int lineNums = 5;
    systemDialogScheduler_->DialogLandscapePositionAdaptive(position, densityPixels, lineNums);
    EXPECT_NE(position.height, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1400 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1500
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1500 start");
    DialogPosition position;
    position.height = 0;
    const float densityPixels = 1.50;
    const int lineNums = 1;
    systemDialogScheduler_->DialogLandscapePositionAdaptive(position, densityPixels, lineNums);
    EXPECT_NE(position.height, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1500 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1600
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1600 start");
    DialogPosition position;
    position.height = 0;
    position.offsetX = 0;
    const int32_t height = 500;
    const int32_t width = 320;
    const int lineNums = 1;
    const float densityPixels = 1.50;
    systemDialogScheduler_->GetSelectorDialogPortraitPosition(position, height, width, lineNums, densityPixels);
    EXPECT_NE(position.height, 0);
    EXPECT_NE(position.offsetX, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1600 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1700
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1700 start");
    DialogPosition position;
    position.height = 0;
    position.offsetX = 0;
    const int32_t height = 100;
    const int32_t width = 100;
    const int lineNums = 1;
    const float densityPixels = 1.50;
    systemDialogScheduler_->GetSelectorDialogPortraitPosition(position, height, width, lineNums, densityPixels);
    EXPECT_EQ(position.oversizeHeight, true);
    EXPECT_NE(position.offsetX, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1700 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1700
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1800 start");
    DialogPosition position;
    position.height = 0;
    position.offsetX = 0;
    const int32_t height = 500;
    const int32_t width = 320;
    const int lineNums = 1;
    const float densityPixels = 1.50;
    systemDialogScheduler_->GetSelectorDialogLandscapePosition(position, height, width, lineNums, densityPixels);
    EXPECT_NE(position.height, 0);
    EXPECT_NE(position.offsetX, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1800 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_1900
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_1900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1900 start");
    DialogPosition position;
    position.height = 1000;
    position.offsetX = 0;
    const int32_t height = 500;
    const int32_t width = 700;
    const int lineNums = 1;
    const float densityPixels = 1.50;
    systemDialogScheduler_->GetSelectorDialogLandscapePosition(position, height, width, lineNums, densityPixels);
    EXPECT_TRUE(position.oversizeHeight);
    EXPECT_NE(position.offsetX, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_1900 end");
}

/*
 * @tc.number    : AbilityMgrServiceDialogTest_2000
 * @tc.name      : AbilityMgrServiceDialogTest
 * @tc.desc      : 1.Test DialogPortraitPositionAdaptive
 */
HWTEST_F(AbilityMgrServiceDialogTest, AbilityMgrServiceDialogTest_2000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_2000 start");
    DialogPosition position;
    position.height = 1000;
    position.offsetX = 0;
    const int32_t height = 500;
    const int32_t width = 700;
    const int lineNums = 1;
    const float densityPixels = 1.50;
    systemDialogScheduler_->GetSelectorDialogLandscapePosition(position, height, width, lineNums, densityPixels);
    EXPECT_TRUE(position.oversizeHeight);
    EXPECT_NE(position.offsetX, 0);
    TAG_LOGI(AAFwkTag::TEST, "AbilityMgrServiceDialogTest_2000 end");
}
} // namespace AAFwk
} // namespace OHOS