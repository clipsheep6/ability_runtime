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

#ifndef OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_COMMAND_H
#define OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_COMMAND_H

#include "ability_command.h"
#include "ability_manager_interface.h"
#include "accessibility_ability_info.h"
#include "accessibility_system_ability_client.h"
#include "shell_command.h"

namespace OHOS {
namespace AAFwk {
struct AccessibilityCommandArgument {
    int32_t bundleArgumentNum = 0;
    int32_t abilityArgumentNum = 0;
    int32_t capabilityNamesArgumentNum = 0;
    int32_t unknownArgumentNum = 0;
    int32_t setArgumentNum = 0;
    std::vector<std::string> unknownArguments;
    std::string command = "";
    std::string bundleName = "";
    std::string abilityName = "";
    std::string capabilityNames = "";
    std::string value = "";
};

class AccessibilityAbilityShellCommand : public ShellCommand {
public:
    AccessibilityAbilityShellCommand(int argc, char *argv[]);
    ~AccessibilityAbilityShellCommand() override
    {}

private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;
    ErrCode CreateMessageMap() override;
    ErrCode RunAsHelpCommand();
    ErrCode RunAsEnableAbility();
    ErrCode RunAsDisableAbility();
    ErrCode RunAsGetEnabledAbilities();
    ErrCode RunAsGetInstalledAbilities();

    ErrCode MakeEnableCommandArgumentFromCmd(AccessibilityCommandArgument &argument);
    ErrCode MakeDisableCommandArgumentFromCmd(AccessibilityCommandArgument &argument);
    const std::vector<std::string> GetEnabledAbilities();
    const std::vector<Accessibility::AccessibilityAbilityInfo> GetInstalledAbilities();
    bool CheckBundleArgument(
        const AccessibilityCommandArgument &argument,
        std::string &resultMessage);
    bool CheckAbilityArgument(
        const AccessibilityCommandArgument &argument,
        std::string &resultMessage);
    bool CheckCapabilitiesArgument(
        const AccessibilityCommandArgument &argument,
        std::vector<Accessibility::AccessibilityAbilityInfo> &installedAbilities,
        std::string &resultMessage);
    bool CheckParamValidity(
        const AccessibilityCommandArgument &argument,
        std::vector<Accessibility::AccessibilityAbilityInfo> &installedAbilities,
        std::string &resultMessage);
    ErrCode CheckEnableCommandArgument(const AccessibilityCommandArgument &argument, std::string &resultMessage);
    ErrCode CheckDisableCommandArgument(const AccessibilityCommandArgument &argument, std::string &resultMessage);
    ErrCode CheckCommandArgument(const AccessibilityCommandArgument &argument, std::string &resultMessage);
    ErrCode CheckSetCommandArgument(const AccessibilityCommandArgument &argument, std::string &resultMessage);

    ErrCode RunAsSetScreenMagnificationState();
    ErrCode RunAsSetShortKeyState();
    ErrCode RunAsSetMouseKeyState();
    ErrCode RunAsSetCaptionState();
    ErrCode RunAsSetMouseAutoClick();
    ErrCode RunAsSetShortKeyTarget();
    ErrCode RunAsSetHighContrastTextState();
    ErrCode RunAsSetInvertColorState();
    ErrCode RunAsSetDaltonizationColorFilter();
    ErrCode RunAsSetContentTimeout();
    ErrCode RunAsSetAnimationOffState();
    ErrCode RunAsSetBrightnessDiscount();
    ErrCode RunAsSetAudioMonoState();
    ErrCode RunAsSetAudioBalance();
    ErrCode MakeSetShortKeyTargetCommandArgumentFromCmd(AccessibilityCommandArgument &argument);
    ErrCode MakeSetCommandArgumentFromCmd(AccessibilityCommandArgument &argument);
    ErrCode MakeCommandArgumentFromCmd(AccessibilityCommandArgument &argument);

    std::shared_ptr<Accessibility::AccessibilitySystemAbilityClient> abilityClientPtr_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_COMMAND_H
