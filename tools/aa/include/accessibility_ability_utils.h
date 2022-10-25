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

#ifndef OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_UTILS_H
#define OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_UTILS_H

#include "accessibility_ability_command.h"
#include "accessibility_ability_info.h"

namespace OHOS {
namespace AAFwk {
class AccessibilityUtils {
public:
    static std::string GetStaticCapabilityNames(const Accessibility::AccessibilityAbilityInfo &abilityInfo);
    static std::string FormatAbilityInfos(
        const std::vector<Accessibility::AccessibilityAbilityInfo> &installedAbilities);
    static std::uint32_t GetCapabilityValue(const std::string &capabilityNames);
    static std::string GetInvalidCapabilityNames(const std::string &enabledCapabilityNames,
        const std::string &installedCapabilityNames);
    static std::string GetUnknownArgumentsMsg(const std::vector<std::string> &unknownArugments);
    static bool IsValidStateString(std::string &stateString);
    static bool IsValidIntString(std::string &intString, const int32_t lowBound, const int32_t highBound);
    static std::string& Trim(std::string &inputStr);
    static int32_t AddPermission();
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_UTILS_H