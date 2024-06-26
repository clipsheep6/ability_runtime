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

#ifndef OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_COMMOND_COMMON_TEST_H
#define OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_COMMOND_COMMON_TEST_H
namespace OHOS {
namespace AAFwk {
const std::string ACCESSIBILITY_TOOL_NAME = "accessibility";
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

const std::string ACCESSIBILITY_SET_SCREEN_MAGNIFICATION_STATE_OK = "set screen magnification state successfully.";
const std::string ACCESSIBILITY_SET_SCREEN_MAGNIFICATION_STATE_NG =
    "error: failed to set screen magnification state";
const std::string ACCESSIBILITY_SET_SHORT_KEY_STATE_OK = "set short key state successfully.";
const std::string ACCESSIBILITY_SET_SHORT_KEY_STATE_NG = "error: failed to set short key state.";
const std::string ACCESSIBILITY_SET_MOUSE_KEY_STATE_OK = "set mouse key state successfully.";
const std::string ACCESSIBILITY_SET_MOUSE_KEY_STATE_NG = "error: failed to set mouse key state.";
const std::string ACCESSIBILITY_SET_CAPTION_STATE_OK = "set caption state successfully.";
const std::string ACCESSIBILITY_SET_CAPTION_STATE_NG = "error: failed to set caption state.";
const std::string ACCESSIBILITY_SET_AUTO_CLICK_TIME_OK = "set mouse auto click time successfully.";
const std::string ACCESSIBILITY_SET_AUTO_CLICK_TIME_NG = "error: failed to set mouse auto click time.";
const std::string ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_OK = "set high contrast text state successfully.";
const std::string ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_NG = "error: failed to set high contrast text state.";
const std::string ACCESSIBILITY_SET_INVERT_COLOR_STATE_OK = "set invert color state successfully.";
const std::string ACCESSIBILITY_SET_INVERT_COLOR_STATE_NG = "error: failed to set invert color state.";
const std::string ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK =
    "set daltonization color filter successfully.";
const std::string ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_NG =
    "error: failed to set daltonization color filter.";
const std::string ACCESSIBILITY_SET_CONTENT_TIME_OK = "set content timeout successfully.";
const std::string ACCESSIBILITY_SET_CONTENT_TIME_NG = "error: failed to set content timeout.";
const std::string ACCESSIBILITY_SET_ANIMATION_OFF_STATE_OK = "set animation off state successfully.";
const std::string ACCESSIBILITY_SET_ANIMATION_OFF_STATE_NG = "error: failed to set animation off state.";
const std::string ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_OK = "set brightness discount successfully.";
const std::string ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_NG = "error: failed to set brightness discount.\n";
const std::string ACCESSIBILITY_SET_AUDIO_MONO_STATE_OK = "set audio mono state successfully.";
const std::string ACCESSIBILITY_SET_AUDIO_MONO_STATE_NG = "error: failed to set audio mono state.";
const std::string ACCESSIBILITY_SET_AUDIO_BALANCE_OK = "set audio balance successfully.";
const std::string ACCESSIBILITY_SET_AUDIO_BALANCE_NG = "error: failed to set audio balance successfully.";

const std::string ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT =
    "argument -a <ability-name> or --ability=<ability-name> is required!";
const std::string ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT = "there are duplicate arguments.";
const std::string ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE = "option -a requires a value.";
const std::string ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT =
    "argument -b <bundle-name> or --bundle=<bundle-name> is required!";
const std::string ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE = "option -b requires a value.";
const std::string ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT =
    "argument -c <capabilities-abbr>"
    " or --capabilities=<capabilities-abbr> is required!";
const std::string ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT_VALUE = "option -c requires a value.";
const std::string ACCESSIBILITY_ABILITY_NO_SET_ARGUMENT_VALUE = "option -v requires a value.";
const std::string ACCESSIBILITY_ABILITY_SET_VALUE_INVALID = "value is invalid.";
const std::string ACCESSIBILITY_HELP_MSG_NO_OPTION = "missing options.";
const std::string ACCESSIBILITY_ABILITY_NOT_FOUND = " was not found!";
const std::string ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT = "there are too many arguments ";

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

const std::string ACCESSIBILITY_HELP_MSG_SET_AUTO_CLICK_TIME =
    "usage: accessibility setMouseAutoClick [-v <time-value>]\n"
    "the range of time-value is 1000 to 5000 and the default unit is ms\n"
    "set the time of the mouse auto click configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_TARGET =
    "usage: accessibility setShortKeyTarget -a <ability-name> -b <bundle-name>\n"
    "set the name of the short key target configuration item\n";

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

const std::string ACCESSIBILITY_HELP_MSG_SET_CONTENT_TIME_OUT =
    "usage: accessibility setContentTimeout [-v <time-value>]\n"
    "the range of time-value is 0 to 5000 and the default unit is ms\n"
    "set the time of the toast content duration configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_ANIMATION_OFF_STATE =
    "usage: accessibility setAnimationOffState [-v <0 | 1>]\n"
    "set the state of the animation off configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_BRIGHTNESS_DISCOUNT =
    "usage: accessibility setBrightnessDiscount [-v <discount-value>]\n"
    "the percentage of discount-value is 0 to 100\n"
    "set the discount of the screen brightness configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_AUDIO_MONO_STATE =
    "usage: accessibility setAudioMonoState [-v <0 | 1>]\n"
    "set the state of the audio mono configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_SET_AUDIO_BALANCE =
    "usage: accessibility setAudioBalance [-v <balance-value>]\n"
    "the percentage of balance-value is -100 to 100\n"
    "the negative values represent the left channel and positive values represent the right channel\n"
    "set the value of the audio balance configuration item\n";

const std::string ACCESSIBILITY_HELP_MSG_ENABLE_ABILITY =
    "usage: accessibility enable [-a <ability-name>] [-b <bundle-name>] [-c <capabilities-abbr>]\n"
    "enable the auxiliary application with bundle-name and ability-name and capabilities-abbr\n";

const std::string ACCESSIBILITY_HELP_MSG_DISABLE_ABILITY =
    "usage: accessibility disable [-a <ability-name>] [-b <bundle-name>]\n"
    "disable the auxiliary application with bundle-name and ability-name\n";
}  // AAFwk
}  // OHOS
#endif // OHOS_ABILITY_RUNTIME_ACCESSIBILITY_ABILITY_COMMOND_COMMON_TEST_H