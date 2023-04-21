/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SIDE_BAR_SIDE_BAR_CONTAINER_LAYOUT_PROPERTY_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SIDE_BAR_SIDE_BAR_CONTAINER_LAYOUT_PROPERTY_H

#include <string>

#include "core/components/common/properties/color.h"
#include "core/components/declaration/button/button_declaration.h"
#include "core/components_ng/layout/layout_property.h"

namespace OHOS::Ace::NG {

struct ControlButtonStyle {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonWidth, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonHeight, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonLeft, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonTop, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonShowIconStr, std::string);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonHiddenIconStr, std::string);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(ControlButtonSwitchingIconStr, std::string);
};

struct DividerStyle {
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DividerStrokeWidth, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DividerStartMargin, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DividerEndMargin, Dimension);
    ACE_DEFINE_PROPERTY_GROUP_ITEM(DividerColor, Color);
};

class ACE_EXPORT SideBarContainerLayoutProperty : public LayoutProperty {
    DECLARE_ACE_TYPE(SideBarContainerLayoutProperty, LayoutProperty);

public:
    SideBarContainerLayoutProperty() = default;
    ~SideBarContainerLayoutProperty() override = default;

    RefPtr<LayoutProperty> Clone() const override
    {
        auto value = MakeRefPtr<SideBarContainerLayoutProperty>();
        value->LayoutProperty::UpdateLayoutProperty(AceType::DynamicCast<LayoutProperty>(this));
        value->propSideBarContainerType_ = CloneSideBarContainerType();
        value->propShowSideBar_ = CloneShowSideBar();
        value->propShowControlButton_ = CloneShowControlButton();
        value->propSideBarWidth_ = CloneSideBarWidth();
        value->propMinSideBarWidth_ = CloneMinSideBarWidth();
        value->propMaxSideBarWidth_ = CloneMaxSideBarWidth();
        value->propAutoHide_ = CloneAutoHide();
        value->propSideBarPosition_ = CloneSideBarPosition();
        value->propControlButtonStyle_ = CloneControlButtonStyle();
        value->propDividerStyle_ = CloneDividerStyle();
        return value;
    }

    void Reset() override
    {
        LayoutProperty::Reset();
        ResetSideBarContainerType();
        ResetShowSideBar();
        ResetShowControlButton();
        ResetSideBarWidth();
        ResetMinSideBarWidth();
        ResetMaxSideBarWidth();
        ResetAutoHide();
        ResetSideBarPosition();
        ResetControlButtonStyle();
        ResetDividerStyle();
    }

    void ToJsonValue(std::unique_ptr<JsonValue>& json) const override
    {
        LayoutProperty::ToJsonValue(json);

        constexpr Dimension DEFAULT_CONTROL_BUTTON_WIDTH = 32.0_vp;
        constexpr Dimension DEFAULT_CONTROL_BUTTON_HEIGHT = 32.0_vp;
        constexpr Dimension DEFAULT_CONTROL_BUTTON_LEFT = 16.0_vp;
        constexpr Dimension DEFAULT_CONTROL_BUTTON_TOP = 48.0_vp;
        constexpr Dimension DEFAULT_SIDE_BAR_WIDTH = 200.0_vp;
        constexpr Dimension DEFAULT_MIN_SIDE_BAR_WIDTH = 200.0_vp;
        constexpr Dimension DEFAULT_MAX_SIDE_BAR_WIDTH = 280.0_vp;
        constexpr Dimension DEFAULT_DIVIDER_STROKE_WIDTH = 1.0_vp;
        constexpr Dimension DEFAULT_DIVIDER_START_MARGIN = 0.0_vp;
        constexpr Dimension DEFAULT_DIVIDER_END_MARGIN = 0.0_vp;
        constexpr Color DEFAULT_DIVIDER_COLOR = Color(0x08000000);

        auto type = propSideBarContainerType_.value_or(SideBarContainerType::EMBED);
        auto sideBarWidth = propSideBarWidth_.value_or(DEFAULT_SIDE_BAR_WIDTH);
        auto minSideBarWidth = propMinSideBarWidth_.value_or(DEFAULT_MIN_SIDE_BAR_WIDTH);
        auto maxSideBarWidth = propMaxSideBarWidth_.value_or(DEFAULT_MAX_SIDE_BAR_WIDTH);
        auto sideBarPosition = propSideBarPosition_.value_or(SideBarPosition::START);
        json->Put("type",
            type == SideBarContainerType::EMBED ? "SideBarContainerType.Embed" : "SideBarContainerType.OVERLAY");
        json->Put("showSideBar", propShowSideBar_.value_or(true) ? "true" : "false");
        json->Put("showControlButton", propShowControlButton_.value_or(true) ? "true" : "false");
        json->Put("sideBarWidth", std::to_string(sideBarWidth.Value()).c_str());
        json->Put("minSideBarWidth", std::to_string(minSideBarWidth.Value()).c_str());
        json->Put("maxSideBarWidth", std::to_string(maxSideBarWidth.Value()).c_str());
        json->Put("autoHide", propAutoHide_.value_or(true) ? "true" : "false");
        json->Put("sideBarPosition",
            sideBarPosition == SideBarPosition::START ? "SideBarPosition.Start" : "SideBarPosition.End");

        // divider
        Dimension strokeWidth = DEFAULT_DIVIDER_STROKE_WIDTH;
        Dimension startMargin = DEFAULT_DIVIDER_START_MARGIN;
        Dimension endMargin = DEFAULT_DIVIDER_END_MARGIN;
        Color color = DEFAULT_DIVIDER_COLOR;

        if (propDividerStyle_) {
            strokeWidth = propDividerStyle_->propDividerStrokeWidth.value_or(DEFAULT_DIVIDER_STROKE_WIDTH);
            startMargin = propDividerStyle_->propDividerStartMargin.value_or(DEFAULT_DIVIDER_START_MARGIN);
            endMargin = propDividerStyle_->propDividerEndMargin.value_or(DEFAULT_DIVIDER_END_MARGIN);
            color = propDividerStyle_->propDividerColor.value_or(DEFAULT_DIVIDER_COLOR);
        }
        auto jsonDivider = JsonUtil::Create(true);
        CHECK_NULL_VOID(jsonDivider);
        jsonDivider->Put("strokeWidth", strokeWidth.ToString().c_str());
        jsonDivider->Put("startMargin", startMargin.ToString().c_str());
        jsonDivider->Put("endMargin", endMargin.ToString().c_str());
        jsonDivider->Put("color", color.ColorToString().c_str());
        json->Put("divider", jsonDivider->ToString().c_str());

        CHECK_NULL_VOID(propControlButtonStyle_);
        auto left = propControlButtonStyle_->propControlButtonLeft.value_or(DEFAULT_CONTROL_BUTTON_LEFT);
        auto top = propControlButtonStyle_->propControlButtonTop.value_or(DEFAULT_CONTROL_BUTTON_TOP);
        auto width = propControlButtonStyle_->propControlButtonWidth.value_or(DEFAULT_CONTROL_BUTTON_WIDTH);
        auto height = propControlButtonStyle_->propControlButtonHeight.value_or(DEFAULT_CONTROL_BUTTON_HEIGHT);

        auto jsonControl = JsonUtil::Create(true);
        CHECK_NULL_VOID(jsonControl);
        jsonControl->Put("left", std::to_string(left.Value()).c_str());
        jsonControl->Put("top", std::to_string(top.Value()).c_str());
        jsonControl->Put("width", std::to_string(width.Value()).c_str());
        jsonControl->Put("height", std::to_string(height.Value()).c_str());

        auto jsonIcon = JsonUtil::Create(true);
        CHECK_NULL_VOID(jsonIcon);
        jsonIcon->Put("shown", propControlButtonStyle_->propControlButtonShowIconStr->c_str());
        jsonIcon->Put("hidden", propControlButtonStyle_->propControlButtonHiddenIconStr->c_str());
        jsonIcon->Put("switching", propControlButtonStyle_->propControlButtonSwitchingIconStr->c_str());

        jsonControl->Put("icon", jsonIcon);
        json->Put("controlButton", jsonControl->ToString().c_str());
    }

    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SideBarContainerType, SideBarContainerType, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowSideBar, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(ShowControlButton, bool, PROPERTY_UPDATE_NORMAL);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SideBarWidth, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(MinSideBarWidth, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(MaxSideBarWidth, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(AutoHide, bool, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITHOUT_GROUP(SideBarPosition, SideBarPosition, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_GROUP(ControlButtonStyle, ControlButtonStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(ControlButtonStyle, ControlButtonWidth, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(ControlButtonStyle, ControlButtonHeight, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(ControlButtonStyle, ControlButtonLeft, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(ControlButtonStyle, ControlButtonTop, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(
        ControlButtonStyle, ControlButtonShowIconStr, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(
        ControlButtonStyle, ControlButtonHiddenIconStr, std::string, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(
        ControlButtonStyle, ControlButtonSwitchingIconStr, std::string, PROPERTY_UPDATE_MEASURE);

    ACE_DEFINE_PROPERTY_GROUP(DividerStyle, DividerStyle);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(DividerStyle, DividerStrokeWidth, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(DividerStyle, DividerStartMargin, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(DividerStyle, DividerEndMargin, Dimension, PROPERTY_UPDATE_MEASURE);
    ACE_DEFINE_PROPERTY_ITEM_WITH_GROUP(DividerStyle, DividerColor, Color, PROPERTY_UPDATE_MEASURE);
private:
    ACE_DISALLOW_COPY_AND_MOVE(SideBarContainerLayoutProperty);
};

} // namespace OHOS::Ace::NG
#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERN_SIDE_BAR_SIDE_BAR_CONTAINER_LAYOUT_PROPERTY_H