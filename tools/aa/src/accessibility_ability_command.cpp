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
#include "accessibility_ability_command.h"
#include "accessibility_ability_utils.h"

#include <csignal>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <regex>

#include "ability_manager_client.h"
#include "accessibility_config.h"
#include "accessibility_system_ability_client.h"
#include "bool_wrapper.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "mission_snapshot.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "test_observer.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string ENABLE_SHORT_OPTIONS = "a:b:c:";
const std::string DISABLE_SHORT_OPTIONS = "a:b:";
const std::string SET_SHORT_OPTIONS = "v:";
const std::string STRING_LIST_ABILITY_NG = "error: failed to list abilities";
const int32_t MOUSE_AUTO_CLICK_TIME_LOW_BOUND = 1000;
const int32_t MOUSE_AUTO_CLICK_TIME_HIGH_BOUND = 5000;
const int32_t CONTENT_TIME_OUT_TIME_LOW_BOUND = 0;
const int32_t CONTENT_TIME_OUT_TIME_HIGH_BOUND = 5000;
const int32_t AUDIO_BALANCE_LOW_BOUND = -100;
const int32_t AUDIO_BALANCE_HIGH_BOUND = 100;
const int32_t BRIGHTNESS_DISCOUNT_LOW_BOUND = 0;
const int32_t BRIGHTNESS_DISCOUNT_HIGH_BOUND = 100;
const int32_t DALTIONIZATION_TYPE_LOW_BOUND = 0;
const int32_t DALTIONIZATION_TYPE_HIGH_BOUND = 3;
const int32_t TYPE_NORMAL = 0;
const int32_t TYPE_PROTANOMALY = 1;
const int32_t TYPE_DEUTERANOMALY = 2;
const int32_t TYPE_TRITANOMALY = 3;
const int32_t ACCESSIBILITY_ENABLE_COMMAND_ARGUMENT_NUM = 3;
const int32_t ACCESSIBILITY_DISABLE_COMMAND_ARGUMENT_NUM = 2;
const int32_t ACCESSIBILITY_SET_COMMAND_ARGUMENT_NUM = 1;

const std::string ACCESSIBILITY_TOOL_NAME = "accessibility";
const std::string ACCESSIBILITY_STRING_ENABLE_ABILITY_OK = "enable ability successfully.";
const std::string ACCESSIBILITY_STRING_ENABLE_ABILITY_NG = "failed to enable ability.";

const std::string ACCESSIBILITY_SET_SCREEN_MAGNIFICATION_STATE_OK = "set screen magnification state successfully.\n";
const std::string ACCESSIBILITY_SET_SHORT_KEY_STATE_OK = "set short key state successfully.\n";
const std::string ACCESSIBILITY_SET_MOUSE_KEY_STATE_OK = "set mouse key state successfully.\n";
const std::string ACCESSIBILITY_SET_CAPTION_STATE_OK = "set caption state successfully.\n";
const std::string ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_OK = "set high contrast text state successfully.\n";
const std::string ACCESSIBILITY_SET_INVERT_COLOR_STATE_OK = "set invert color state successfully.\n";
const std::string ACCESSIBILITY_SET_ANIMATION_OFF_STATE_OK = "set animation off state successfully.\n";
const std::string ACCESSIBILITY_SET_AUDIO_MONO_STATE_OK = "set audio mono state successfully.\n";

const std::string ACCESSIBILITY_SET_AUTO_CLICK_TIME_OK = "set mouse auto click time successfully.\n";
const std::string ACCESSIBILITY_SET_SHORT_KEY_TARGET_OK = "set short key target successfully.\n";
const std::string ACCESSIBILITY_SET_SHORT_KEY_TARGET_NG = "failed to set short key target.\n";
const std::string ACCESSIBILITY_SET_AUDIO_BALANCE_OK = "set audio balance successfully.\n";
const std::string ACCESSIBILITY_SET_CONTENT_TIME_OK = "set content timeout successfully.\n";
const std::string ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_OK = "set brightness discount successfully.\n";
const std::string ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK =
    "set daltonization color filter successfully.\n";

const std::string ACCESSIBILITY_ABILITY_NOT_FOUND = " was not found!";
const std::string ACCESSIBILITY_HELP_MSG_NO_OPTION = "missing options.";

const std::string ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT =
    "argument -b <bundle-name> or --bundle=<bundle-name> is required!";
const std::string ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT =
    "argument -a <ability-name> or --ability=<ability-name> is required!";
const std::string ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT =
    "argument -c <capabilities-abbr>"
    " or --capabilities=<capabilities-abbr> is required!";

const std::string ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT = "there are too many arguments ";
const std::string ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT = "there are duplicate arguments.";

const std::string ACCESSIBILITY_STRING_DISABLE_ABILITY_OK = "disable ability successfully.";
const std::string ACCESSIBILITY_STRING_DISABLE_ABILITY_NG = "error: failed to disable ability.";

const std::string ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE = "option -a requires a value.";
const std::string ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE = "option -b requires a value.";
const std::string ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT_VALUE = "option -c requires a value.";
const std::string ACCESSIBILITY_ABILITY_NO_SET_ARGUMENT_VALUE = "option -v requires a value.";

const std::string ACCESSIBILITY_ABILITY_SET_VALUE_INVALID = "value is invalid.";

const std::string ACCESSIBILITY_HELP_MSG =
    "usage: accessibility <command>\n"
    "these are common accessibility commands list:\n"
    "  help                        list available commands\n"
    "  enable                      enable ability with options\n"
    "  disable                     disable ability with options\n"
    "  list                        list the installed abilities info\n"
    "  setShortKeyState            set the state of the short key configuration item\n"
    "  setMouseKeyState            set the state of the mouse key configuration item\n"
    "  setCaptionState             set the state of the caption configuration item\n"
    "  setMouseAutoClick           set the time of the mouse auto click configuration item\n"
    "  setShortKeyTarget           set the name of the short key target configuration item\n"
    "  setHighContrastTextState    set the state of the high contrast text configuration item\n"
    "  setInvertColorState         set the state of the invert color configuration item\n"
    "  setDaltonizationColorFilter set the type of the daltonization color filter configuration item\n"
    "  setContentTimeout           set the time of the toast content duration configuration item\n"
    "  setAnimationOffState        set the state of the animation off configuration item\n"
    "  setBrightnessDiscount       set the discount of the screen brightness configuration item\n"
    "  setAudioMonoState           set the state of the audio mono configuration item\n"
    "  setAudioBalance             set the value of the audio balance configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_ENABLE_ABILITY =
    "usage: accessibility enable [-a <ability-name>] [-b <bundle-name>] [-c <capabilities-abbr>]\n"
    "enable the auxiliary application with bundle-name and ability-name and capabilities-abbr\n";

const std::string ACCESSIBILITY_HELP_MSG_DISABLE_ABILITY =
    "usage: accessibility disable [-a <ability-name>] [-b <bundle-name>]\n"
    "disable the auxiliary application with bundle-name and ability-name\n";

const std::string ACCESSIBILITY_HELP_MSG_LIST_ABILITIES =
    "the auxiliary capabilities supported by the accessibility subsystem are as follows:\n"
    "r:retrieve                                enable the retrieval capability of auxiliary application\n"
    "t:touch_guide                             enable the touch guide capability of auxiliary application\n"
    "g:gesture                                 enable the gesture injection capability of auxiliary application\n"
    "k:key_event_observer                      enable the key event interception capability of auxiliary application\n"
    "z:zoom                                    enable the gesture zoom capability of auxiliary application\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_SCREEN_MAGNIFICATION_STATE =
    "usage: accessibility setScreenMagnificationState [-v <0 | 1> ]\n"
    "set the state of the screen magnification configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_STATE =
    "usage: accessibility setShortKeyState [-v <0 | 1> ]\n"
    "set the state of the short key configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_MOUSE_KEY_STATE =
    "usage: accessibility setMouseKeyState [-v <0 | 1> ]\n"
    "set the state of the mouse key configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_CAPTION_STATE =
    "usage: accessibility setCaptionState [-v <0 | 1> ]\n"
    "set the state of the caption configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_HIGH_CONTRAST_TEXT_STATE =
    "usage: accessibility setHighContrastTextState [-v <0 | 1> ]\n"
    "set the state of the high contrast text configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_INVERT_COLOR_STATE =
    "usage: accessibility setInvertColorState [-v <0 | 1>]\n"
    "set the state of the invert color configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_DALTONIZATION_COLOR_FILTER =
    "usage: accessibility setDaltonizationColorFilter [-v <0 | 1 | 2 | 3>]\n"
    "normal = 0, protanomaly = 1, deuteranomaly = 2, tritanomaly = 3\n"
    "set the type of the daltonization color filter configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_ANIMATION_OFF_STATE =
    "usage: accessibility setAnimationOffState [-v <0 | 1>]\n"
    "set the state of the animation off configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_AUDIO_MONO_STATE =
    "usage: accessibility setAudioMonoState [-v <0 | 1>]\n"
    "set the state of the audio mono configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_AUTO_CLICK_TIME =
    "usage: accessibility setMouseAutoClick [-v <time-value>]\n"
    "the range of time-value is 1000 to 5000 and the default unit is ms\n"
    "set the time of the mouse auto click configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_TARGET =
    "usage: accessibility setShortKeyTarget -a <ability-name> -b <bundle-name>\n"
    "set the name of the short key target configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_CONTENT_TIME_OUT =
    "usage: accessibility setContentTimeout [-v <time-value>]\n"
    "the range of time-value is 0 to 5000 and the default unit is ms\n"
    "set the time of the toast content duration configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_BRIGHTNESS_DISCOUNT =
    "usage: accessibility setBrightnessDiscount [-v <discount-value>]\n"
    "the percentage of discount-value is 0 to 100\n"
    "set the discount of the screen brightness configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_AUDIO_BALANCE =
    "usage: accessibility setAudioBalance [-v <balance-value>]\n"
    "the percentage of balance-value is -100 to 100\n"
    "the negative values represent the left channel and positive values represent the right channel\n"
    "set the value of the audio balance configuration item\n";

constexpr struct option ENABLE_LONG_OPTIONS[] = {
    {"ability", required_argument, nullptr, 'a'},
    {"bundle", required_argument, nullptr, 'b'},
    {"capabilities", required_argument, nullptr, 'c'},
    {nullptr, 0, nullptr, 0},
};

constexpr struct option DISABLE_LONG_OPTIONS[] = {
    {"ability", required_argument, nullptr, 'a'},
    {"bundle", required_argument, nullptr, 'b'},
    {nullptr, 0, nullptr, 0},
};

constexpr struct option SET_LONG_OPTIONS[] = {
    {"value", required_argument, nullptr, 'v'},
    {nullptr, 0, nullptr, 0},
};
}  // namespace

AccessibilityAbilityShellCommand::AccessibilityAbilityShellCommand(int argc, char *argv[])
    : ShellCommand(argc, argv, ACCESSIBILITY_TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        HILOG_INFO("argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
    aaShellCmd_ = std::make_shared<AbilityManagerShellCommand>(argc, argv);
    if (aaShellCmd_.get() == nullptr) {
        HILOG_ERROR("Get aa command failed.");
    }
    if (abilityClientPtr_ == nullptr) {
        abilityClientPtr_ = Accessibility::AccessibilitySystemAbilityClient::GetInstance();
        if (abilityClientPtr_ == nullptr) {
            HILOG_ERROR("Get accessibility system ability client failed.");
        }
    }
}

ErrCode AccessibilityAbilityShellCommand::init()
{
    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::CreateMessageMap()
{
    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", std::bind(&AccessibilityAbilityShellCommand::RunAsHelpCommand, this)},
        {"enable", std::bind(&AccessibilityAbilityShellCommand::RunAsEnableAbility, this)},
        {"disable", std::bind(&AccessibilityAbilityShellCommand::RunAsDisableAbility, this)},
        {"list", std::bind(&AccessibilityAbilityShellCommand::RunAsGetInstalledAbilities, this)},
        {"setShortKeyState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetShortKeyState, this)},
        {"setMouseKeyState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetMouseKeyState, this)},
        {"setCaptionState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetCaptionState, this)},
        {"setMouseAutoClick", std::bind(&AccessibilityAbilityShellCommand::RunAsSetMouseAutoClick, this)},
        {"setShortKeyTarget", std::bind(&AccessibilityAbilityShellCommand::RunAsSetShortKeyTarget, this)},
        {"setHighContrastTextState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetHighContrastTextState, this)},
        {"setInvertColorState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetInvertColorState, this)},
        {"setDaltonizationColorFilter",
            std::bind(&AccessibilityAbilityShellCommand::RunAsSetDaltonizationColorFilter, this)},
        {"setContentTimeout", std::bind(&AccessibilityAbilityShellCommand::RunAsSetContentTimeout, this)},
        {"setAnimationOffState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetAnimationOffState, this)},
        {"setBrightnessDiscount", std::bind(&AccessibilityAbilityShellCommand::RunAsSetBrightnessDiscount, this)},
        {"setAudioMonoState", std::bind(&AccessibilityAbilityShellCommand::RunAsSetAudioMonoState, this)},
        {"setAudioBalance", std::bind(&AccessibilityAbilityShellCommand::RunAsSetAudioBalance, this)},
    };

    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(ACCESSIBILITY_HELP_MSG);

    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::MakeEnableCommandArgumentFromCmd(AccessibilityCommandArgument &argument)
{
    int option = -1;
    int counter = 0;
    argument.command = "enable";
    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, ENABLE_SHORT_OPTIONS.c_str(), ENABLE_LONG_OPTIONS, nullptr);

        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
                return OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        if (option == '?') {
            switch (optopt) {
                case 'a': {
                    resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                case 'b': {
                    resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                case 'c': {
                    resultReceiver_.append(argument.command + ": " +
                        ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                default: {
                    break;
                }
            }
        }

        switch (option) {
            case 'a': {
                argument.abilityName = optarg;
                argument.abilityArgumentNum++;
                break;
            }
            case 'b': {
                argument.bundleName = optarg;
                argument.bundleArgumentNum++;
                break;
            }
            case 'c': {
                argument.capabilityNames = optarg;
                argument.capabilityNamesArgumentNum++;
                break;
            }
            default: {
                argument.unknownArgumentNum++;
                argument.unknownArguments.push_back(argv_[optind - 1]);
                break;
            }
        }
    }
    return CheckEnableCommandArgument(argument, resultReceiver_);
}

ErrCode AccessibilityAbilityShellCommand::MakeDisableCommandArgumentFromCmd(AccessibilityCommandArgument &argument)
{
    argument.command = "disable";
    ErrCode result = MakeCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK) {
        return CheckDisableCommandArgument(argument, resultReceiver_);
    }
    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsEnableAbility()
{
    AccessibilityCommandArgument argument;
    ErrCode result = MakeEnableCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        std::string name = argument.bundleName + "/" + argument.abilityName;
        std::string capabilityNames = argument.capabilityNames;
        uint32_t capabilities = AccessibilityUtils::GetCapabilityValue(capabilityNames);
        if (config.EnableAbility(name, capabilities)) {
            resultReceiver_ = ACCESSIBILITY_STRING_ENABLE_ABILITY_OK + "\n";
        } else {
            resultReceiver_ = ACCESSIBILITY_STRING_ENABLE_ABILITY_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_ENABLE_ABILITY);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsDisableAbility()
{
    AccessibilityCommandArgument argument;
    ErrCode result = MakeDisableCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        std::string name = argument.bundleName + "/" + argument.abilityName;
        if (config.DisableAbility(name)) {
            resultReceiver_ = ACCESSIBILITY_STRING_DISABLE_ABILITY_OK + "\n";
        } else {
            resultReceiver_ = ACCESSIBILITY_STRING_DISABLE_ABILITY_NG + "\n";
            resultReceiver_.append(GetMessageFromCode(result));
        }
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_DISABLE_ABILITY);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsGetEnabledAbilities()
{
    ErrCode result = OHOS::ERR_OK;
    std::vector<std::string> enabledAbilities = GetEnabledAbilities();
    if (!enabledAbilities.empty()) {
        for (const auto &ability : enabledAbilities) {
            resultReceiver_.append(ability + "\n");
        }
    } else {
        result = OHOS::ERR_INVALID_VALUE;
    }
    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsGetInstalledAbilities()
{
    ErrCode result = OHOS::ERR_OK;
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities = GetInstalledAbilities();
    if (!installedAbilities.empty()) {
        resultReceiver_.append(AccessibilityUtils::FormatAbilityInfos(installedAbilities));
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_LIST_ABILITIES);
    } else {
        result = OHOS::ERR_INVALID_VALUE;
    }
    return result;
}

const std::vector<std::string> AccessibilityAbilityShellCommand::GetEnabledAbilities()
{
    std::vector<std::string> enabledAbilities;
    if (abilityClientPtr_ != nullptr &&
        (abilityClientPtr_->GetEnabledAbilities(enabledAbilities) != Accessibility::RET_OK)) {
        HILOG_ERROR("Failed to GetEnabledAbilities");
    }
    return enabledAbilities;
}

const std::vector<Accessibility::AccessibilityAbilityInfo> AccessibilityAbilityShellCommand::GetInstalledAbilities()
{
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    const uint32_t allTypes = Accessibility::AccessibilityAbilityTypes::ACCESSIBILITY_ABILITY_TYPE_ALL;
    const Accessibility::AbilityStateType stateType = Accessibility::AbilityStateType::ABILITY_STATE_INSTALLED;
    if (abilityClientPtr_ != nullptr &&
        (abilityClientPtr_->GetAbilityList(allTypes, stateType, installedAbilities) != Accessibility::RET_OK)) {
        HILOG_ERROR("Failed to GetInstalledAbilities");
    }
    return installedAbilities;
}

bool AccessibilityAbilityShellCommand::CheckBundleArgument(
    const AccessibilityCommandArgument &argument,
    std::string &resultMessage)
{
    if (argument.bundleArgumentNum == 0) {
        resultMessage.append(ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT);
        return false;
    }
    if (argument.bundleArgumentNum > 1) {
        resultMessage.append(ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT);
        return false;
    }
    if (argument.bundleName.empty() || argument.bundleName[0] == '-') {
        resultMessage.append(ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE);
        return false;
    }
    return true;
}

bool AccessibilityAbilityShellCommand::CheckAbilityArgument(
    const AccessibilityCommandArgument &argument,
    std::string &resultMessage)
{
    if (argument.abilityArgumentNum == 0) {
        resultMessage.append(ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT);
        return OHOS::ERR_INVALID_VALUE;
    }
    if (argument.abilityArgumentNum > 1) {
        resultMessage.append(ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT);
        return OHOS::ERR_INVALID_VALUE;
    }
    if (argument.abilityName.empty() || argument.abilityName[0] == '-') {
        resultMessage.append(ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE);
        return OHOS::ERR_INVALID_VALUE;
    }
    return true;
}

bool AccessibilityAbilityShellCommand::CheckCapabilitiesArgument(
    const AccessibilityCommandArgument &argument,
    std::vector<Accessibility::AccessibilityAbilityInfo> &installedAbilities,
    std::string &resultMessage)
{
    const std::string &capabilityNames = argument.capabilityNames;
    if (argument.capabilityNamesArgumentNum == 0) {
        resultMessage.append(ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT);
        return false;
    }
    if (argument.capabilityNamesArgumentNum > 1) {
        resultMessage.append(ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT);
        return false;
    }
    if (argument.capabilityNames.empty() || argument.capabilityNames[0] == '-') {
        resultMessage.append(ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT_VALUE);
        return false;
    }
    for (auto &ability : installedAbilities) {
        AccessibilityCommandArgument argument;
        AccessibilityUtils::GetCommandArgument(ability, argument);
        std::string invalidCapabilityNames = AccessibilityUtils::GetInvalidCapabilityNames(capabilityNames,
            argument.capabilityNames);
        if (!invalidCapabilityNames.empty()) {
            resultMessage.append("the capabilities " + invalidCapabilityNames + ACCESSIBILITY_ABILITY_NOT_FOUND);
            return false;
        }
    }
    return true;
}

bool AccessibilityAbilityShellCommand::CheckAbilityNameAndBundleName(
    const AccessibilityCommandArgument &argument,
    std::vector<Accessibility::AccessibilityAbilityInfo> &installedAbilities,
    std::string &resultMessage)
{
    const std::string &abilityName = argument.abilityName;
    const std::string &bundleName = argument.bundleName;
    bool isExisted = false;
    for (auto &ability : installedAbilities) {
        if (ability.GetName() == abilityName && ability.GetPackageName() == bundleName) {
            isExisted = true;
        }
    }
    if (!isExisted) {
        resultMessage.append("the auxiliary application " +
            bundleName + "/" + abilityName + ACCESSIBILITY_ABILITY_NOT_FOUND);
        return false;
    }
    return true;
}

ErrCode AccessibilityAbilityShellCommand::CheckEnableCommandArgument(const AccessibilityCommandArgument &argument,
    std::string &resultMessage)
{
    ErrCode result = OHOS::ERR_OK;
    resultMessage.append(argument.command + ": ");
    int32_t totalArgumentNum = argument.bundleArgumentNum + argument.abilityArgumentNum +
        argument.capabilityNamesArgumentNum + argument.unknownArgumentNum;
    if (totalArgumentNum > ACCESSIBILITY_ENABLE_COMMAND_ARGUMENT_NUM) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT);
        if (argument.bundleArgumentNum > 1 || argument.abilityArgumentNum > 1 ||
            argument.capabilityNamesArgumentNum > 1) {
            resultMessage.append("and exist duplicated arguments");
        }
        if (argument.unknownArgumentNum > 0) {
            resultMessage.append("and exist unknown arguments ");
            resultMessage.append(AccessibilityUtils::GetUnknownArgumentsMsg(argument.unknownArguments));
        }
        return OHOS::ERR_INVALID_VALUE;
    }
    if (argument.unknownArgumentNum > 0) {
        resultMessage.append("unknown arguments ");
        resultMessage.append(AccessibilityUtils::GetUnknownArgumentsMsg(argument.unknownArguments));
        return OHOS::ERR_INVALID_VALUE;
    }
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities = GetInstalledAbilities();
    if (!CheckBundleArgument(argument, resultMessage)) {
        HILOG_ERROR("bundleName = %{public}s is invalid", argument.bundleName.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    if (!CheckAbilityArgument(argument, resultMessage)) {
        HILOG_ERROR("abilityName = %{public}s is invalid", argument.abilityName.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    if (!CheckCapabilitiesArgument(argument, installedAbilities, resultMessage)) {
        HILOG_ERROR("capabilityNames = %{public}s is invalid", argument.capabilityNames.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    if (!CheckAbilityNameAndBundleName(argument, installedAbilities, resultMessage)) {
        HILOG_ERROR("%{public}s/%{public}s is not installed",
            argument.bundleName.c_str(), argument.abilityName.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    std::vector<std::string> enabledAbilities = GetEnabledAbilities();
    std::string currentAbility = argument.bundleName + "/" + argument.abilityName;
    for (const auto &ability : enabledAbilities) {
        if (ability == currentAbility) {
            resultMessage.append("the auxiliary application has been enabled.");
            return OHOS::ERR_INVALID_VALUE;
        }
    }
    return result;
}

ErrCode AccessibilityAbilityShellCommand::CheckDisableCommandArgument(const AccessibilityCommandArgument &argument,
    std::string &resultMessage)
{
    ErrCode result = CheckCommandArgument(argument, resultMessage);
    if (result == OHOS::ERR_OK) {
        std::vector<std::string> enabledAbilities = GetEnabledAbilities();
        std::string currentAbility = argument.bundleName + "/" + argument.abilityName;
        bool isEnabled = false;
        for (const auto &ability : enabledAbilities) {
            if (ability == currentAbility) {
                isEnabled = true;
                break;
            }
        }
        if (!isEnabled) {
            resultMessage.append("the auxiliary application was not enabled and could not be disabled.");
            return OHOS::ERR_INVALID_VALUE;
        }
    }
    return result;
}

ErrCode AccessibilityAbilityShellCommand::CheckCommandArgument(const AccessibilityCommandArgument &argument,
    std::string &resultMessage)
{
    resultMessage.append(argument.command + ": ");
    int32_t totalArgumentNum = argument.bundleArgumentNum + argument.abilityArgumentNum +
        argument.capabilityNamesArgumentNum + argument.unknownArgumentNum;
    if (totalArgumentNum > ACCESSIBILITY_DISABLE_COMMAND_ARGUMENT_NUM) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT);
        if (argument.bundleArgumentNum > 1 || argument.abilityArgumentNum > 1) {
            resultMessage.append("and exist duplicated arguments.");
        }
        if (argument.unknownArgumentNum > 0) {
            resultMessage.append("and exist unknown arguments ");
            resultMessage.append(AccessibilityUtils::GetUnknownArgumentsMsg(argument.unknownArguments));
        }
        return OHOS::ERR_INVALID_VALUE;
    }
    if (argument.unknownArgumentNum > 0) {
        resultMessage.append("unknown arguments ");
        resultMessage.append(AccessibilityUtils::GetUnknownArgumentsMsg(argument.unknownArguments));
        return OHOS::ERR_INVALID_VALUE;
    }
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities = GetInstalledAbilities();
    if (!CheckBundleArgument(argument, resultMessage)) {
        HILOG_ERROR("bundleName = %{public}s is invalid", argument.bundleName.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    if (!CheckAbilityArgument(argument, resultMessage)) {
        HILOG_ERROR("abilityName = %{public}s is invalid", argument.abilityName.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    if (!CheckAbilityNameAndBundleName(argument, installedAbilities, resultMessage)) {
        HILOG_ERROR("%{public}s/%{public}s is not installed",
            argument.bundleName.c_str(), argument.abilityName.c_str());
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::CheckSetCommandArgument(const AccessibilityCommandArgument &argument,
    std::string &resultMessage)
{
    resultMessage.append(argument.command + ": ");
    int32_t totalArgumentNum = argument.setArgumentNum + argument.unknownArgumentNum;
    if (totalArgumentNum > ACCESSIBILITY_SET_COMMAND_ARGUMENT_NUM) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT);
        if (argument.setArgumentNum > 1) {
            resultMessage.append("and exist duplicated arguments.");
        }
        if (argument.unknownArgumentNum > 0) {
            resultMessage.append("and exist unknown arguments ");
            resultMessage.append(AccessibilityUtils::GetUnknownArgumentsMsg(argument.unknownArguments));
        }
        return OHOS::ERR_INVALID_VALUE;
    }
    if (argument.unknownArgumentNum > 0) {
        resultMessage.append("unknown arguments ");
        resultMessage.append(AccessibilityUtils::GetUnknownArgumentsMsg(argument.unknownArguments));
        return OHOS::ERR_INVALID_VALUE;
    }
    if (argument.setArgumentNum == 0) {
        resultMessage.append(ACCESSIBILITY_HELP_MSG_NO_OPTION);
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetScreenMagnificationState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setScreenMagnificationState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetScreenMagnificationState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_SCREEN_MAGNIFICATION_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_SCREEN_MAGNIFICATION_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetShortKeyState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setShortKeyState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetShortKeyState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_SHORT_KEY_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetMouseKeyState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setMouseKeyState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetMouseKeyState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_MOUSE_KEY_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_MOUSE_KEY_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetCaptionState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setCaptionState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetCaptionsState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_CAPTION_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_CAPTION_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetMouseAutoClick()
{
    AccessibilityCommandArgument argument;
    argument.command = "setMouseAutoClick";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidIntString(argument.value,
        MOUSE_AUTO_CLICK_TIME_LOW_BOUND, MOUSE_AUTO_CLICK_TIME_HIGH_BOUND)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        float time = 1.0 * std::atoi(argument.value.c_str()) / 100;
        config.SetMouseAutoClick(time);
        resultReceiver_.append(ACCESSIBILITY_SET_AUTO_CLICK_TIME_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_AUTO_CLICK_TIME);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetShortKeyTarget()
{
    AccessibilityCommandArgument argument;
    ErrCode result = MakeSetShortKeyTargetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        std::string name = argument.bundleName + "/" + argument.abilityName;
        config.SetShortkeyTarget(name);
        resultReceiver_ = ACCESSIBILITY_SET_SHORT_KEY_TARGET_OK + "\n";
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_TARGET);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetHighContrastTextState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setHighContrastTextState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetHighContrastTextState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_HIGH_CONTRAST_TEXT_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetInvertColorState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setInvertColorState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetInvertColorState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_INVERT_COLOR_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_INVERT_COLOR_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetDaltonizationColorFilter()
{
    AccessibilityCommandArgument argument;
    argument.command = "setDaltonizationColorFilter";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidIntString(argument.value,
        DALTIONIZATION_TYPE_LOW_BOUND, DALTIONIZATION_TYPE_HIGH_BOUND)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        return OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        int32_t value = std::atoi(argument.value.c_str());
        AccessibilityConfig::DALTONIZATION_TYPE type = AccessibilityConfig::DALTONIZATION_TYPE::Normal;
        switch (value) {
            case TYPE_NORMAL: {
                type = AccessibilityConfig::DALTONIZATION_TYPE::Normal;
                break;
            }
            case TYPE_PROTANOMALY: {
                type = AccessibilityConfig::DALTONIZATION_TYPE::Protanomaly;
                break;
            }
            case TYPE_DEUTERANOMALY: {
                type = AccessibilityConfig::DALTONIZATION_TYPE::Deuteranomaly;
                break;
            }
            case TYPE_TRITANOMALY: {
                type = AccessibilityConfig::DALTONIZATION_TYPE::Tritanomaly;
                break;
            }
            default: {
                break;
            }
        }
        config.SetDaltonizationColorFilter(type);
        resultReceiver_.append(ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_DALTONIZATION_COLOR_FILTER);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetContentTimeout()
{
    AccessibilityCommandArgument argument;
    argument.command = "setContentTimeout";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidIntString(argument.value,
        CONTENT_TIME_OUT_TIME_LOW_BOUND, CONTENT_TIME_OUT_TIME_HIGH_BOUND)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        int32_t time = std::atoi(argument.value.c_str());
        config.SetContentTimeout(time);
        resultReceiver_.append(ACCESSIBILITY_SET_CONTENT_TIME_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_CONTENT_TIME_OUT);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetAnimationOffState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setAnimationOffState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetAnimationOffState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_ANIMATION_OFF_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_ANIMATION_OFF_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetBrightnessDiscount()
{
    AccessibilityCommandArgument argument;
    argument.command = "setSetBrightnessDiscount";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidIntString(argument.value,
        BRIGHTNESS_DISCOUNT_LOW_BOUND, BRIGHTNESS_DISCOUNT_HIGH_BOUND)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        float discount = 1.0 * std::atoi(argument.value.c_str()) / 100;
        config.SetBrightnessDiscount(discount);
        resultReceiver_.append(ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_BRIGHTNESS_DISCOUNT);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetAudioMonoState()
{
    AccessibilityCommandArgument argument;
    argument.command = "setAudioMonoState";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidStateString(argument.value)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        bool state = std::atoi(argument.value.c_str()) == 1;
        config.SetAudioMonoState(state);
        resultReceiver_.append(ACCESSIBILITY_SET_AUDIO_MONO_STATE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_AUDIO_MONO_STATE);
        result = OHOS::ERR_INVALID_VALUE;
    }

    return result;
}

ErrCode AccessibilityAbilityShellCommand::RunAsSetAudioBalance()
{
    AccessibilityCommandArgument argument;
    argument.command = "setAudioBalance";
    ErrCode result = MakeSetCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK && !AccessibilityUtils::IsValidIntString(argument.value,
        AUDIO_BALANCE_LOW_BOUND, AUDIO_BALANCE_HIGH_BOUND)) {
        resultReceiver_.append(ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
        result = OHOS::ERR_INVALID_VALUE;
    }
    if (result == OHOS::ERR_OK) {
        auto &config = OHOS::AccessibilityConfig::AccessibilityConfig::GetInstance();
        (void)config.InitializeContext();
        float balance = 1.0 * std::atoi(argument.value.c_str()) / 100;
        config.SetAudioBalance(balance);
        resultReceiver_.append(ACCESSIBILITY_SET_AUDIO_BALANCE_OK);
    } else {
        resultReceiver_.append("\n");
        resultReceiver_.append(ACCESSIBILITY_HELP_MSG_SET_AUDIO_BALANCE);
        result = OHOS::ERR_INVALID_VALUE;
    }
    return result;
}

ErrCode AccessibilityAbilityShellCommand::MakeSetShortKeyTargetCommandArgumentFromCmd(
    AccessibilityCommandArgument &argument)
{
    argument.command = "setShortkeyTarget";
    ErrCode result = MakeCommandArgumentFromCmd(argument);
    if (result == OHOS::ERR_OK) {
        return CheckCommandArgument(argument, resultReceiver_);
    }
    return result;
}

ErrCode AccessibilityAbilityShellCommand::MakeCommandArgumentFromCmd(AccessibilityCommandArgument &argument)
{
    int option = -1;
    int counter = 0;
    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, DISABLE_SHORT_OPTIONS.c_str(), DISABLE_LONG_OPTIONS, nullptr);

        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
                return OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        if (option == '?') {
            option = optopt;
            switch (option) {
                case 'a': {
                    resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                case 'b': {
                    resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                default: {
                    break;
                }
            }
        }

        switch (option) {
            case 'a': {
                argument.abilityName = optarg;
                argument.abilityArgumentNum++;
                break;
            }
            case 'b': {
                argument.bundleName = optarg;
                argument.bundleArgumentNum++;
                break;
            }
            default: {
                argument.unknownArgumentNum++;
                argument.unknownArguments.push_back(argv_[optind - 1]);
                break;
            }
        }
    }
    return OHOS::ERR_OK;
}

ErrCode AccessibilityAbilityShellCommand::MakeSetCommandArgumentFromCmd(AccessibilityCommandArgument &argument)
{
    int option = -1;
    int counter = 0;

    while (true) {
        counter++;

        option = getopt_long(argc_, argv_, SET_SHORT_OPTIONS.c_str(), SET_LONG_OPTIONS, nullptr);

        HILOG_INFO("option: %{public}d, optopt: %{public}d, optind: %{public}d", option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
                return OHOS::ERR_INVALID_VALUE;
            }
            break;
        }

        if (option == '?') {
            option = optopt;
            switch (option) {
                case 'v': {
                    resultReceiver_.append(argument.command + ": " + ACCESSIBILITY_ABILITY_NO_SET_ARGUMENT_VALUE);
                    return OHOS::ERR_INVALID_VALUE;
                }
                default: {
                    break;
                }
            }
        }

        switch (option) {
            case 'v': {
                argument.value = optarg;
                argument.setArgumentNum++;
                break;
            }
            default: {
                argument.unknownArgumentNum++;
                argument.unknownArguments.push_back(argv_[optind - 1]);
                break;
            }
        }
    }
    return CheckSetCommandArgument(argument, resultReceiver_);
}

}  // namespace AAFwk
}  // namespace OHOS
