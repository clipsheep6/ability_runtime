/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "accessibility_config.h"

namespace {
    bool g_mockSetBrightnessDiscountRet = true;
}

void MockSetBrightnessDiscount(bool mockRet)
{
    g_mockSetBrightnessDiscountRet = mockRet;
}

namespace OHOS {
namespace AccessibilityConfig {
Accessibility::RetError AccessibilityConfig::SetBrightnessDiscount(const float brightness)
{
    if (g_mockSetBrightnessDiscountRet == false) {
        return Accessibility::RET_ERR_FAILED;
    }
    return Accessibility::RET_OK;
}
} // Accessibility
} // OHOS