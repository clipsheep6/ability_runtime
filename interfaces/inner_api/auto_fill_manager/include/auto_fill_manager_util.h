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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_UTIL_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_UTIL_H

#include "auto_fill_custom_config.h"
#include "popup_ui_extension_config.h"

namespace OHOS {
namespace AbilityRuntime {
class AutoFillManagerUtil {
public:
    AutoFillManagerUtil() = default;
    ~AutoFillManagerUtil() = default;

    static void ConvertToPopupUIExtensionConfig(const AutoFill::AutoFillCustomConfig &config,
        Ace::CustomPopupUIExtensionConfig &popupConfig);
    static Ace::PopupDimensionUnit ConvertPopupUnit(const AutoFill::PopupDimensionUnit &unit);
    static Ace::PopupPlacement ConvertPopupPlacement(const AutoFill::PopupPlacement &placement);
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_MANAGER_UTIL_H