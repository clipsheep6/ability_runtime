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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_CUSTOM_CONFIG_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_CUSTOM_CONFIG_H

#include <optional>
#include <string>

namespace OHOS {
namespace AbilityRuntime {
namespace AutoFill {
enum class PopupPlacement {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    LEFT_TOP,
    LEFT_BOTTOM,
    RIGHT_TOP,
    RIGHT_BOTTOM,
    NONE,
};
 
enum class PopupDimensionUnit {
    PX = 0,
    VP,
    FP,
    PERCENT,
    LPX,
    AUTO,
    CALC,
};
 
struct PopupOffset {
    PopupDimensionUnit unit = PopupDimensionUnit::PX;
    double deltaX = 0.0;
    double deltaY = 0.0;
};
 
struct PopupSize {
    PopupDimensionUnit unit = PopupDimensionUnit::PX;
    double width = 0.0;
    double height = 0.0;
};
 
struct PopupLength {
    PopupDimensionUnit unit = PopupDimensionUnit::PX;
    double length = 0.0;
};
 
struct AutoFillCustomConfig {
    bool isShowInSubWindow = true;
    std::string inspectorId;
    std::int32_t nodeId = -1;
    std::optional<bool> isAutoCancel;
    std::optional<bool> isEnableArrow;
    std::optional<PopupSize> targetSize;
    std::optional<PopupOffset> targetOffset;
    std::optional<PopupLength> targetSpace;
    std::optional<PopupLength> arrowOffset;
    std::optional<PopupPlacement> placement;
    std::optional<int32_t> backgroundColor;
    std::optional<int32_t> maskColor;
    std::function<void(const std::string&)> onStateChange;
};
} // AutoFill
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_CUSTOM_CONFIG_H