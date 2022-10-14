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
#include "accessibility_ability_utils.h"

#include <fstream>

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t NUM_COLUMN_WIDTH = 10;
const int32_t BUNDLE_NAME_COLUMN_WIDTH = 50;
const int32_t ABILITY_NAME_COLUMN_WIDTH = 30;
const int32_t CAPABILITIES_ABBR_COLUMN_WIDTH = 20;
} // namespace

void AccessibilityUtils::GetCommandArgument(const Accessibility::AccessibilityAbilityInfo &abilityInfo,
    AccessibilityCommandArgument &argument)
{
    argument.bundleName = abilityInfo.GetPackageName();
    argument.abilityName = abilityInfo.GetName();
    std::string capabilityNames = "";
    const std::map<std::int32_t, std::string> capabilityNameMap = {
        {0x0001, "r"}, {0x0002, "t"},
        {0x0004, "g"}, {0x0008, "k"},
        {0x0010, "z"},
    };
    for (auto it = capabilityNameMap.begin(); it != capabilityNameMap.end(); it++) {
        if (it->first & abilityInfo.GetStaticCapabilityValues()) {
            capabilityNames.append(it->second);
        }
    }
    argument.capabilityNames = capabilityNames;
}

std::string AccessibilityUtils::FormatAbilityInfos(
    const std::vector<Accessibility::AccessibilityAbilityInfo> &installedAbilities)
{
    std::string result = "";
    std::stringstream headerStream;
    headerStream << std::left << std::setw(NUM_COLUMN_WIDTH) << "NO"
    << std::left << std::setw(BUNDLE_NAME_COLUMN_WIDTH) << "bundleName"
    << std::left << std::setw(ABILITY_NAME_COLUMN_WIDTH) << "abilityName"
    << std::left << std::setw(CAPABILITIES_ABBR_COLUMN_WIDTH) << "capabilities-abbr" << std::endl;
    result.append(headerStream.str());
    int num = 1;
    for (auto &ability : installedAbilities) {
        std::stringstream lineStream;
        AccessibilityCommandArgument argument;
        GetCommandArgument(ability, argument);
        lineStream << std::left << std::setw(NUM_COLUMN_WIDTH) << std::to_string(num)
        << std::left << std::setw(BUNDLE_NAME_COLUMN_WIDTH) << argument.bundleName
        << std::left << std::setw(ABILITY_NAME_COLUMN_WIDTH) << argument.abilityName
        << std::left << argument.capabilityNames << std::endl;
        num++;
        result.append(lineStream.str());
    }
    return result;
}

std::int32_t AccessibilityUtils::GetCapabilityValue(const std::string &capabilityNames)
{
    std::int32_t result = 0;
    std::map<char, int32_t> capabilityValueMap = {
        {'r', 0x0001}, {'t', 0x0002}, {'g', 0x0004}, {'k', 0x0008}, {'z', 0x0010}
    };
    for (int i = 0; i < capabilityNames.size(); i++) {
        result |= capabilityValueMap[capabilityNames[i]];
    }
    return result;
}

std::string AccessibilityUtils::GetInvalidCapabilityNames(const std::string &enabledCapabilityNames,
    const std::string &installedCapabilityNames)
{
    std::string result = "";
    std::set<char> installedCapabilityNameSet;
    for (int i = 0; i < installedCapabilityNames.size(); i++) {
        installedCapabilityNameSet.insert(installedCapabilityNames[i]);
    }
    for (int i = 0; i < enabledCapabilityNames.size(); i++) {
        if (installedCapabilityNameSet.count(enabledCapabilityNames[i]) == 0) {
            result.push_back(enabledCapabilityNames[i]);
        }
    }
    return result;
}

std::string AccessibilityUtils::GetUnknownArgumentsMsg(const std::vector<std::string> &unknownArguments)
{
    std::string result = "";
    for (auto &argument : unknownArguments) {
        result.append(argument + " ");
    };
    return result;
}

bool AccessibilityUtils::IsValidStateString(std::string &stateString)
{
    std::string valueStr = Trim(stateString);
    if (valueStr.size() != 1) {
        return false;
    }
    if (valueStr[0] == '0' || valueStr[0] == '1') {
        return true;
    }
    return false;
}

bool AccessibilityUtils::IsValidIntString(std::string &intString, const int32_t lowBound, const int32_t highBound)
{
    int32_t value = 0;
    const int32_t base = 10;
    std::string valueStr = Trim(intString);
    if (valueStr.empty()) {
        return false;
    }
    bool flag = true;
    int index = 0;
    if (valueStr[0] == '-' || valueStr[0] == '+') {
        index++;
        if (valueStr[0] == '-') {
            flag = false;
        }
    }
    while (index < valueStr.size()) {
        if (valueStr[index] >= '0' && valueStr[index] <= '9') {
            value = value * base + valueStr[index] - '0';
            if (value > highBound) {
                return false;
            }
        } else {
            return false;
        }
        index++;
    }
    if (!flag) {
        value = -value;
    }
    return value >= lowBound;
}

std::string& AccessibilityUtils::Trim(std::string &inputStr)
{
    if (inputStr.empty()) {
        return inputStr;
    }
    inputStr.erase(0, inputStr.find_first_not_of(" "));
    inputStr.erase(inputStr.find_last_not_of(" ") + 1);
    return inputStr;
}

}
}