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

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t NUM_COLUMN_WIDTH = 10;
const int32_t BUNDLE_NAME_COLUMN_WIDTH = 50;
const int32_t ABILITY_NAME_COLUMN_WIDTH = 30;
const int32_t CAPABILITIES_ABBR_COLUMN_WIDTH = 20;
const int32_t DCAPS_NUM = 0;
const int32_t PERMS_NUM = 2;
const int32_t ACLS_NUM = 0;
const std::string PROCESS_NAME = "aa";
const std::string APL_STR = "system_basic";
} // namespace

std::string AccessibilityUtils::GetStaticCapabilityNames(const Accessibility::AccessibilityAbilityInfo &abilityInfo)
{
    std::string capabilityNames = "";
    std::map<uint32_t, std::string> capabilityNameMap = {
        {0x0001, "r"}, {0x0002, "t"},
        {0x0004, "g"}, {0x0008, "k"},
        {0x0010, "z"},
    };
    for (auto it = capabilityNameMap.begin(); it != capabilityNameMap.end(); it++) {
        if (it->first & abilityInfo.GetStaticCapabilityValues()) {
            capabilityNames.append(it->second);
        }
    }
    return capabilityNames;
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
        std::string capabilityNames = GetStaticCapabilityNames(ability);
        lineStream << std::left << std::setw(NUM_COLUMN_WIDTH) << std::to_string(num)
        << std::left << std::setw(BUNDLE_NAME_COLUMN_WIDTH) << ability.GetPackageName()
        << std::left << std::setw(ABILITY_NAME_COLUMN_WIDTH) << ability.GetName()
        << std::left << capabilityNames << std::endl;
        num++;
        result.append(lineStream.str());
    }
    return result;
}

std::uint32_t AccessibilityUtils::GetCapabilityValue(const std::string &capabilityNames)
{
    uint32_t result = 0;
    std::map<char, uint32_t> capabilityValueMap = {
        {'r', 0x0001}, {'t', 0x0002}, {'g', 0x0004}, {'k', 0x0008}, {'z', 0x0010}
    };
    for (uint32_t i = 0; i < capabilityNames.size(); i++) {
        result |= capabilityValueMap[capabilityNames[i]];
    }
    return result;
}

std::string AccessibilityUtils::GetInvalidCapabilityNames(const std::string &enabledCapabilityNames,
    const std::string &installedCapabilityNames)
{
    std::string result = "";
    std::set<char> installedCapabilityNameSet;
    for (uint32_t i = 0; i < installedCapabilityNames.size(); i++) {
        installedCapabilityNameSet.insert(installedCapabilityNames[i]);
    }
    for (uint32_t i = 0; i < enabledCapabilityNames.size(); i++) {
        if (installedCapabilityNameSet.count(enabledCapabilityNames[i]) == 0) {
            result.push_back(enabledCapabilityNames[i]);
        }
    }
    return result;
}

std::string AccessibilityUtils::GetUnknownArgumentsMsg(const std::vector<std::string> &unknownArguments)
{
    std::string result = "";
    for (const auto &argument : unknownArguments) {
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
    uint32_t index = 0;
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

int32_t AccessibilityUtils::AddPermission()
{
    const char *perms[2];
    perms[0] = OHOS::Accessibility::OHOS_PERMISSION_READ_ACCESSIBILITY_CONFIG.c_str();
    perms[1] = OHOS::Accessibility::OHOS_PERMISSION_WRITE_ACCESSIBILITY_CONFIG.c_str();
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = DCAPS_NUM,
        .permsNum = PERMS_NUM,
        .aclsNum = ACLS_NUM,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = PROCESS_NAME.c_str(),
        .aplStr = APL_STR.c_str(),
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    if (!tokenId) {
        HILOG_ERROR("Set token failed.");
        return -1;
    }
    int32_t setTokenResult = SetSelfTokenID(tokenId);
    if (setTokenResult != 0) {
        HILOG_ERROR("Set token failed.");
        return -1;
    }
    return AccessTokenKit::ReloadNativeTokenInfo();
}

}
}