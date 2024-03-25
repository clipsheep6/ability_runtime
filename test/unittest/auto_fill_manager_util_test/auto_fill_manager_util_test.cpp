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

#define private public
#include "auto_fill_manager_util.h"
#undef private

using namespace testing;
using namespace testing::ext;
using PopupDimensionUnit = OHOS::AbilityRuntime::AutoFill::PopupDimensionUnit;
using PopupPlacement = OHOS::AbilityRuntime::AutoFill::PopupPlacement;
namespace OHOS {
namespace AAFwk {
namespace {
    AbilityRuntime::AutoFill::PopupSize popupSize {
        .unit = AbilityRuntime::AutoFill::PopupDimensionUnit::PX,
        .width = 3.0,
        .height = 4.0
    };

    AbilityRuntime::AutoFill::PopupOffset popupOffset {
        .unit = AbilityRuntime::AutoFill::PopupDimensionUnit::VP,
        .deltaX = 6.1,
        .deltaY = 7.1
    };

    AbilityRuntime::AutoFill::PopupLength popupLength {
        .unit = AbilityRuntime::AutoFill::PopupDimensionUnit::FP,
        .length = 10.0
    };

    AbilityRuntime::AutoFill::PopupLength arrowLength {
        .unit = AbilityRuntime::AutoFill::PopupDimensionUnit::PERCENT,
        .length = 20.0
    };

    std::function<void(std::string)> stringValue = [](const std::string &str) {
        std::cout << "string value" << str << std::endl;
    };
}
class AutoFillManagerUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AutoFillManagerUtilTest::SetUpTestCase(void)
{}

void AutoFillManagerUtilTest::TearDownTestCase(void)
{}

void AutoFillManagerUtilTest::SetUp()
{}

void AutoFillManagerUtilTest::TearDown()
{}

/**
 * @tc.name: ConvertToPopupUIExtensionConfig_0100
 * @tc.desc: ConvertToPopupUIExtensionConfig
 */
HWTEST_F(AutoFillManagerUtilTest, ConvertToPopupUIExtensionConfig_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerUtilTest, ConvertToPopupUIExtensionConfig_0100, TestSize.Level1";
    auto popup = std::make_shared<AbilityRuntime::AutoFillManagerUtil>();

    AbilityRuntime::AutoFill::AutoFillCustomConfig config = {
        true,
        "password123456",
        1,
        true,
        true,
        popupSize,
        popupOffset,
        popupLength,
        arrowLength,
        AbilityRuntime::AutoFill::PopupPlacement::NONE,
        0xffffff,
        0xffffff,
        std::move(stringValue)
    };

    Ace::CustomPopupUIExtensionConfig popupConfig;
    popup->ConvertToPopupUIExtensionConfig(config, popupConfig);

    EXPECT_EQ(config.isShowInSubWindow, popupConfig.isShowInSubWindow);
    EXPECT_EQ(config.inspectorId, popupConfig.inspectorId);
    EXPECT_EQ(config.nodeId, popupConfig.nodeId);
    EXPECT_EQ(config.isAutoCancel, popupConfig.isAutoCancel);
    EXPECT_EQ(config.isEnableArrow, popupConfig.isEnableArrow);
    EXPECT_TRUE(popupConfig.targetSize.has_value());
    EXPECT_TRUE(popupConfig.targetOffset.has_value());
    EXPECT_TRUE(popupConfig.targetSpace.has_value());
    EXPECT_TRUE(popupConfig.arrowOffset.has_value());
    EXPECT_TRUE(popupConfig.placement.has_value());
    EXPECT_EQ(config.backgroundColor, popupConfig.backgroundColor);
    EXPECT_EQ(config.maskColor, popupConfig.maskColor);
    EXPECT_NE(popupConfig.onStateChange, NULL);
}

/**
 * @tc.name: ConvertPopupUnit_0100
 * @tc.desc: ConvertPopupUnit
 */
HWTEST_F(AutoFillManagerUtilTest, ConvertPopupUnit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerUtilTest, ConvertPopupUnit_0100, TestSize.Level1";
    auto popup = std::make_shared<AbilityRuntime::AutoFillManagerUtil>();

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::PX)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::PX));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::VP)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::VP));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::FP)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::FP));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::PERCENT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::PERCENT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::LPX)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::LPX));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::AUTO)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::AUTO));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupUnit(PopupDimensionUnit::CALC)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupDimensionUnit::CALC));
}

/**
 * @tc.name: ConvertPopupPlacement_0100
 * @tc.desc: ConvertPopupPlacement
 */
HWTEST_F(AutoFillManagerUtilTest, ConvertPopupPlacement_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoFillManagerUtilTest, ConvertPopupPlacement_0100, TestSize.Level1";
    auto popup = std::make_shared<AbilityRuntime::AutoFillManagerUtil>();

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::LEFT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::LEFT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::RIGHT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::RIGHT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::TOP_LEFT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::TOP_LEFT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::TOP_RIGHT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::TOP_RIGHT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::BOTTOM_LEFT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::BOTTOM_LEFT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::BOTTOM_RIGHT)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::BOTTOM_RIGHT));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::LEFT_TOP)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::LEFT_TOP));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::LEFT_BOTTOM)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::LEFT_BOTTOM));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::RIGHT_TOP)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::RIGHT_TOP));

    EXPECT_EQ(static_cast<int32_t>(popup->ConvertPopupPlacement(PopupPlacement::RIGHT_BOTTOM)),
        static_cast<int32_t>(AbilityRuntime::AutoFill::PopupPlacement::RIGHT_BOTTOM));
}
} // namespace AppExecFwk
} // namespace OHOS