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

#include "auto_fill_manager_util.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void AutoFillManagerUtil::ConvertToPopupUIExtensionConfig(const AutoFill::AutoFillCustomConfig &config,
    Ace::CustomPopupUIExtensionConfig &popupConfig)
{
    popupConfig.isShowInSubWindow = config.isShowInSubWindow;
    popupConfig.inspectorId = config.inspectorId;
    popupConfig.nodeId = config.nodeId;
    popupConfig.isAutoCancel = config.isAutoCancel;
    popupConfig.isEnableArrow = config.isEnableArrow;
    popupConfig.isFocusable = false;
    if (config.targetSize.has_value()) {
        Ace::PopupSize popupSize;
        AutoFill::PopupSize targetSize = config.targetSize.value();
        popupSize.unit = ConvertPopupUnit(targetSize.unit);
        popupSize.width = static_cast<double>(targetSize.width);
        popupSize.height = static_cast<double>(targetSize.height);
        popupConfig.targetSize = popupSize;
    }

    if (config.targetOffset.has_value()) {
        Ace::PopupOffset popupOffset;
        AutoFill::PopupOffset targetOffset = config.targetOffset.value();
        popupOffset.unit = ConvertPopupUnit(targetOffset.unit);
        popupOffset.deltaX = static_cast<double>(targetOffset.deltaX);
        popupOffset.deltaY = static_cast<double>(targetOffset.deltaY);
        popupConfig.targetOffset = popupOffset;
    }

    Ace::PopupLength popupLength;
    if (config.targetSpace.has_value()) {
        AutoFill::PopupLength targetSpace = config.targetSpace.value();
        popupLength.unit = ConvertPopupUnit(targetSpace.unit);
        popupLength.length = static_cast<double>(targetSpace.length);
        popupConfig.targetSpace = popupLength;
    }
    if (config.arrowOffset.has_value()) {
        AutoFill::PopupLength arrowOffset = config.arrowOffset.value();
        popupLength.unit = ConvertPopupUnit(arrowOffset.unit);
        popupLength.length = static_cast<double>(arrowOffset.length);
        popupConfig.arrowOffset = popupLength;
    }

    if (config.placement.has_value()) {
        popupConfig.placement = ConvertPopupPlacement(config.placement.value());
    }

    popupConfig.backgroundColor = config.backgroundColor;
    popupConfig.maskColor = config.maskColor;
    popupConfig.onStateChange = config.onStateChange;
}

Ace::PopupDimensionUnit AutoFillManagerUtil::ConvertPopupUnit(const AutoFill::PopupDimensionUnit &unit)
{
    Ace::PopupDimensionUnit popupUnit = Ace::PopupDimensionUnit::PX;
    switch (unit) {
        case AutoFill::PopupDimensionUnit::VP:
            popupUnit = Ace::PopupDimensionUnit::VP;
            break;
        case AutoFill::PopupDimensionUnit::FP:
            popupUnit = Ace::PopupDimensionUnit::FP;
            break;
        case AutoFill::PopupDimensionUnit::PERCENT:
            popupUnit = Ace::PopupDimensionUnit::PERCENT;
            break;
        case AutoFill::PopupDimensionUnit::LPX:
            popupUnit = Ace::PopupDimensionUnit::LPX;
            break;
        case AutoFill::PopupDimensionUnit::AUTO:
            popupUnit = Ace::PopupDimensionUnit::AUTO;
            break;
        case AutoFill::PopupDimensionUnit::CALC:
            popupUnit = Ace::PopupDimensionUnit::CALC;
            break;
        default:
            break;
    }
    return popupUnit;
}

Ace::PopupPlacement AutoFillManagerUtil::ConvertPopupPlacement(const AutoFill::PopupPlacement &placement)
{
    Ace::PopupPlacement popupPlacement = Ace::PopupPlacement::NONE;
    switch (placement) {
        case AutoFill::PopupPlacement::LEFT:
            popupPlacement = Ace::PopupPlacement::LEFT;
            break;
        case AutoFill::PopupPlacement::RIGHT:
            popupPlacement = Ace::PopupPlacement::RIGHT;
            break;
        case AutoFill::PopupPlacement::TOP_LEFT:
            popupPlacement = Ace::PopupPlacement::TOP_LEFT;
            break;
        case AutoFill::PopupPlacement::TOP_RIGHT:
            popupPlacement = Ace::PopupPlacement::TOP_RIGHT;
            break;
        case AutoFill::PopupPlacement::BOTTOM_LEFT:
            popupPlacement = Ace::PopupPlacement::BOTTOM_LEFT;
            break;
        case AutoFill::PopupPlacement::BOTTOM_RIGHT:
            popupPlacement = Ace::PopupPlacement::BOTTOM_RIGHT;
            break;
        case AutoFill::PopupPlacement::LEFT_TOP:
            popupPlacement = Ace::PopupPlacement::LEFT_TOP;
            break;
        case AutoFill::PopupPlacement::LEFT_BOTTOM:
            popupPlacement = Ace::PopupPlacement::LEFT_BOTTOM;
            break;
        case AutoFill::PopupPlacement::RIGHT_TOP:
            popupPlacement = Ace::PopupPlacement::RIGHT_TOP;
            break;
        case AutoFill::PopupPlacement::RIGHT_BOTTOM:
            popupPlacement = Ace::PopupPlacement::RIGHT_BOTTOM;
            break;
        default:
            TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Popup placement is invalid.");
            break;
    }
    return popupPlacement;
}
} // namespace AbilityRuntime
} // namespace OHOS
