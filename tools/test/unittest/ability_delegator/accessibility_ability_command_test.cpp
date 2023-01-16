/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "accessibility_ability_command.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
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
const std::string ACCESSIBILITY_SET_SHORT_KEY_STATE_OK = "set short key state successfully.";
const std::string ACCESSIBILITY_SET_MOUSE_KEY_STATE_OK = "set mouse key state successfully.";
const std::string ACCESSIBILITY_SET_CAPTION_STATE_OK = "set caption state successfully.";
const std::string ACCESSIBILITY_SET_AUTO_CLICK_TIME_OK = "set mouse auto click time successfully.";
const std::string ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_OK = "set high contrast text state successfully.";
const std::string ACCESSIBILITY_SET_INVERT_COLOR_STATE_OK = "set invert color state successfully.";
const std::string ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK =
    "set daltonization color filter successfully.";
const std::string ACCESSIBILITY_SET_CONTENT_TIME_OK = "set content timeout successfully.";
const std::string ACCESSIBILITY_SET_ANIMATION_OFF_STATE_OK = "set animation off state successfully.";
const std::string ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_OK = "set brightness discount successfully.";
const std::string ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_NG = "error: failed to set brightness discount.\n";
const std::string ACCESSIBILITY_SET_AUDIO_MONO_STATE_OK = "set audio mono state successfully.";
const std::string ACCESSIBILITY_SET_AUDIO_BALANCE_OK = "set audio balance successfully.";

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
}  // namespace

namespace OHOS {
namespace AAFwk {
class AccessibilityAbilityShellCommandTest : public ::testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::string cmdhelp_ = "help";
    std::string cmdenable_ = "enable";
    std::string cmddisable_ = "disable";
    std::string cmdsetShortKeyState_ = "setShortKeyState";
    std::string cmdsetMouseKeyState_ = "setMouseKeyState";
    std::string cmdsetCaptionState_ = "setCaptionState";
    std::string cmdssetMouseAutoClick_ = "setMouseAutoClick";
    std::string cmdsetShortKeyTarget_ = "setShortKeyTarget";
    std::string cmdsetHighContrastTextState_ = "setHighContrastTextState";
    std::string cmdsetInvertColorState_ = "setInvertColorState";
    std::string cmdsetDaltonizationColorFilter_ = "setDaltonizationColorFilter";
    std::string cmdsetContentTimeout_ = "setContentTimeout";
    std::string cmdsetAnimationOffState_ = "setAnimationOffState";
    std::string cmdsetBrightnessDiscount_ = "setBrightnessDiscount";
    std::string cmdsetAudioMonoState_ = "setAudioMonoState";
    std::string cmdsetAudioBalance_ = "setAudioBalance";       
    std::string cmdSetScreenMagnificationState_ = "setScreenMagnificationState";
};

void AccessibilityAbilityShellCommandTest::SetUpTestCase()
{}

void AccessibilityAbilityShellCommandTest::TearDownTestCase()
{}

void AccessibilityAbilityShellCommandTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AccessibilityAbilityShellCommandTest::TearDown()
{}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdssetMouseAutoClick_.c_str(),
        (char*)"-v",
        (char*)"1000",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseAutoClick();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUTO_CLICK_TIME_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdssetMouseAutoClick_.c_str(),
        (char*)"-v",
        (char*)"5000",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseAutoClick();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUTO_CLICK_TIME_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdssetMouseAutoClick_.c_str(),
        (char*)"-v",
        (char*)"5001",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseAutoClick();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setMouseAutoClick: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_SET_AUTO_CLICK_TIME);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0400
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseAutoClick();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n"+ACCESSIBILITY_HELP_MSG_SET_AUTO_CLICK_TIME);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetShortKeyTarget_0100
 * @tc.name: RunAsSetShortKeyTarget
 * @tc.desc: Test whether RunAsSetShortKeyTarget is called normally.
 *           (MakeSetShortKeyTargetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetShortKeyTarget_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyTarget_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetShortKeyTarget();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n"+ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_TARGET);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyTarget_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0100
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetHighContrastTextState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetHighContrastTextState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0200
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetHighContrastTextState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetHighContrastTextState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0300
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetHighContrastTextState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetHighContrastTextState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setHighContrastTextState: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_HIGH_CONTRAST_TEXT_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0400
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetHighContrastTextState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_HIGH_CONTRAST_TEXT_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0100
 * @tc.name: RunAsSetInvertColorState
 * @tc.desc: Test whether RunAsSetInvertColorState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetInvertColorState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetInvertColorState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_INVERT_COLOR_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0200
 * @tc.name: RunAsSetInvertColorState
 * @tc.desc: Test whether RunAsSetInvertColorState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetInvertColorState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetInvertColorState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_INVERT_COLOR_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0300
 * @tc.name: RunAsSetInvertColorState
 * @tc.desc: Test whether RunAsSetInvertColorState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetInvertColorState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetInvertColorState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setInvertColorState: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_SET_INVERT_COLOR_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0400
 * @tc.name: RunAsSetInvertColorState
 * @tc.desc: Test whether RunAsSetInvertColorState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetInvertColorState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n"+ACCESSIBILITY_HELP_MSG_SET_INVERT_COLOR_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0100
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetDaltonizationColorFilter_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetDaltonizationColorFilter();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0200
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetDaltonizationColorFilter_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetDaltonizationColorFilter();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0300
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetDaltonizationColorFilter_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetDaltonizationColorFilter();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0400
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetDaltonizationColorFilter_.c_str(),
        (char*)"-v",
        (char*)"3",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetDaltonizationColorFilter();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0500
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0500 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetDaltonizationColorFilter_.c_str(),
        (char*)"-v",
        (char*)"4",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetDaltonizationColorFilter();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setDaltonizationColorFilter: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0500 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0600
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0600 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetDaltonizationColorFilter();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n"+ACCESSIBILITY_HELP_MSG_SET_DALTONIZATION_COLOR_FILTER);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0600 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0100
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetContentTimeout_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetContentTimeout();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_CONTENT_TIME_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0200
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetContentTimeout_.c_str(),
        (char*)"-v",
        (char*)"5000",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetContentTimeout();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_CONTENT_TIME_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0300
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetContentTimeout_.c_str(),
        (char*)"-v",
        (char*)"5001",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetContentTimeout();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setContentTimeout: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_SET_CONTENT_TIME_OUT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0400
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetContentTimeout();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_CONTENT_TIME_OUT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0100
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAnimationOffState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAnimationOffState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_ANIMATION_OFF_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0200
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAnimationOffState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAnimationOffState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_ANIMATION_OFF_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0300
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAnimationOffState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAnimationOffState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setAnimationOffState: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_ANIMATION_OFF_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0400
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAnimationOffState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_ANIMATION_OFF_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0100
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: Test whether RunAsSetBrightnessDiscount is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetBrightnessDiscount_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetBrightnessDiscount();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_NG);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0200
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: Test whether RunAsSetBrightnessDiscount is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetBrightnessDiscount_.c_str(),
        (char*)"-v",
        (char*)"100",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetBrightnessDiscount();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_NG);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0300
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: Test whether RunAsSetBrightnessDiscount is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetBrightnessDiscount_.c_str(),
        (char*)"-v",
        (char*)"101",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetBrightnessDiscount();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setSetBrightnessDiscount: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_SET_BRIGHTNESS_DISCOUNT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0400
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: Test whether RunAsSetBrightnessDiscount is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetBrightnessDiscount();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_BRIGHTNESS_DISCOUNT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0100
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAudioMonoState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioMonoState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_MONO_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0200
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAudioMonoState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioMonoState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_MONO_STATE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0300
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAudioMonoState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioMonoState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setAudioMonoState: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_SET_AUDIO_MONO_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0400
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioMonoState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_AUDIO_MONO_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0100
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAudioBalance_.c_str(),
        (char*)"-v",
        (char*)"-100",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioBalance();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_BALANCE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0200
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAudioBalance_.c_str(),
        (char*)"-v",
        (char*)"100",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioBalance();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_BALANCE_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0300
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetAudioBalance_.c_str(),
        (char*)"-v",
        (char*)"101",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioBalance();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setAudioBalance: "+ ACCESSIBILITY_ABILITY_SET_VALUE_INVALID + 
    "\n" + ACCESSIBILITY_HELP_MSG_SET_AUDIO_BALANCE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0400
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetAudioBalance();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_AUDIO_BALANCE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeSetShortKeyTargetCommandArgumentFromCmd_0100
 * @tc.name: MakeSetShortKeyTargetCommandArgumentFromCmd
 * @tc.desc: Test whether MakeSetShortKeyTargetCommandArgumentFromCmd is called normally.
 *           (MakeCommandArgumentFromCmd = OHOS::ERR_OK,MakeCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeSetShortKeyTargetCommandArgumentFromCmd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetShortKeyTargetCommandArgumentFromCmd_0100 start";
    AccessibilityCommandArgument argument;
    optind = 5;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyTarget_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeSetShortKeyTargetCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setShortkeyTarget: " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetShortKeyTargetCommandArgumentFromCmd_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0100
 * @tc.name: MakeSetCommandArgumentFromCmd
 * @tc.desc: Test whether MakeSetCommandArgumentFromCmd is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0100 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeSetCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ": ");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0200
 * @tc.name: MakeSetCommandArgumentFromCmd
 * @tc.desc: Test whether MakeSetCommandArgumentFromCmd is called normally.(optind < 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0200 start";
    AccessibilityCommandArgument argument;
    optind = -1;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeSetCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0300
 * @tc.name: MakeSetCommandArgumentFromCmd
 * @tc.desc: Test whether MakeSetCommandArgumentFromCmd is called normally.(option = -1)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0300 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeSetCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, ": " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0400
 * @tc.name: MakeSetCommandArgumentFromCmd
 * @tc.desc: Test whether MakeSetCommandArgumentFromCmd is called normally.(option = ?)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0400 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeSetCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, ": " + ACCESSIBILITY_ABILITY_NO_SET_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0500
 * @tc.name: MakeSetCommandArgumentFromCmd
 * @tc.desc: Test whether MakeSetCommandArgumentFromCmd is called normally.(option = default)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0500 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-a",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeSetCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, ": unknown arguments -a ");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeSetCommandArgumentFromCmd_0500 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0100
 * @tc.name: MakeCommandArgumentFromCmd
 * @tc.desc: Test whether MakeCommandArgumentFromCmd is called normally.(return OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0100 start";
    AccessibilityCommandArgument argument;
    optind = 5;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyTarget_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_OK);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0200
 * @tc.name: MakeCommandArgumentFromCmd
 * @tc.desc: Test whether MakeCommandArgumentFromCmd is called normally.(return OHOS::ERR_OK，option a b)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0200 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyTarget_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)"-v",
        (char*)"value",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_OK);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0300
 * @tc.name: MakeCommandArgumentFromCmd
 * @tc.desc: Test whether MakeCommandArgumentFromCmd is called normally.(missing options)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0300 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyTarget_.c_str(),
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, ": " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0400
 * @tc.name: MakeCommandArgumentFromCmd
 * @tc.desc: Test whether MakeCommandArgumentFromCmd is called normally.(NO_ABILITY)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0400 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyTarget_.c_str(),
        (char*)"-a",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, ": " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0500
 * @tc.name: MakeCommandArgumentFromCmd
 * @tc.desc: Test whether MakeCommandArgumentFromCmd is called normally.(NO_BUNDLE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0500 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyTarget_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, ": " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0500 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0600
 * @tc.name: MakeCommandArgumentFromCmd
 * @tc.desc: Test whether MakeCommandArgumentFromCmd is called normally.(optind = -1)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0600 start";
    AccessibilityCommandArgument argument;
    optind = -1;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeCommandArgumentFromCmd_0600 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeDisableCommandArgumentFromCmd_0100
 * @tc.name: MakeDisableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeDisableCommandArgumentFromCmd is called normally.
 *           (MakeCommandArgumentFromCmd = OHOS::ERR_OK,CheckDisableCommandArgument = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_MakeDisableCommandArgumentFromCmd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeDisableCommandArgumentFromCmd_0100 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 0;
    argument.unknownArgumentNum = 0;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmddisable_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeDisableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeDisableCommandArgumentFromCmd_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckDisableCommandArgument_0100
 * @tc.name: CheckDisableCommandArgument
 * @tc.desc: Test whether CheckDisableCommandArgument is called normally.
 *           (CheckCommandArgument = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_CheckDisableCommandArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckDisableCommandArgument_0100 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 0;
    argument.unknownArgumentNum = 0;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckDisableCommandArgument(argument,resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": the auxiliary application bundle/ability" + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckDisableCommandArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0100
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(totalArgumentNum > 2)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_CheckCommandArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0100 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    argument.abilityArgumentNum = 2;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 0;
    argument.unknownArgumentNum = 1;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCommandArgument(argument,resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": and exist duplicated arguments.and exist unknown arguments ");
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0200
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(unknownArgumentNum > 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_CheckCommandArgument_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0200 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    argument.abilityArgumentNum = 0;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 0;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 0;
    argument.unknownArgumentNum = 1;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCommandArgument(argument,resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": unknown arguments ");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0300
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(CheckAbilityArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_CheckCommandArgument_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0300 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    argument.abilityArgumentNum = 0;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 1;
    argument.unknownArgumentNum = 0;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCommandArgument(argument,resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0400
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(CheckBundleArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_CheckCommandArgument_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0400 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 0;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 1;
    argument.unknownArgumentNum = 0;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCommandArgument(argument,resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0500
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(CheckParamValidity = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandTest, 
AccessibilityAbilityShellCommand_CheckCommandArgument_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0500 start";
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 0;
    argument.unknownArgumentNum = 0;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCommandArgument(argument,resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": the auxiliary application " +
            argument.bundleName + "/" + argument.abilityName + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0500 end";
}
}  // namespace AAFwk
}  // namespace OHOS
