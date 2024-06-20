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
#include "accessibility_ability_command_common_test.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {
class AccessibilityAbilityShellCommandSecondTest : public ::testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::string cmdHelp_ = "help";
    std::string cmdEnable_ = "enable";
    std::string cmdDisable_ = "disable";
    std::string cmdsetShortKeyState_ = "setShortKeyState";
    std::string cmdsetMouseKeyState_ = "setMouseKeyState";
    std::string cmdsetCaptionState_ = "setCaptionState";
    std::string cmdsSetMouseAutoClick_ = "setMouseAutoClick";
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

void AccessibilityAbilityShellCommandSecondTest::SetUpTestCase()
{}

void AccessibilityAbilityShellCommandSecondTest::TearDownTestCase()
{}

void AccessibilityAbilityShellCommandSecondTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AccessibilityAbilityShellCommandSecondTest::TearDown()
{}
/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0100
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_BALANCE_NG);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0200
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_BALANCE_NG);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0300
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setAudioBalance: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_AUDIO_BALANCE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioBalance_0400
 * @tc.name: RunAsSetAudioBalance
 * @tc.desc: Test whether RunAsSetAudioBalance is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
        (char*)cmdDisable_.c_str(),
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
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    ErrCode result = cmd.CheckDisableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": the auxiliary application bundle/ability" + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckDisableCommandArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0100
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(totalArgumentNum > 2)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    ErrCode result = cmd.CheckCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": and duplicated arguments exist.and unknown arguments exist.");
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0200
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(unknownArgumentNum > 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    ErrCode result = cmd.CheckCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": unknown arguments exist.");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0300
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(CheckAbilityArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    ErrCode result = cmd.CheckCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0400
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(CheckBundleArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    ErrCode result = cmd.CheckCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCommandArgument_0500
 * @tc.name: CheckCommandArgument
 * @tc.desc: Test whether CheckCommandArgument is called normally.(CheckParamValidity = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandSecondTest,
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
    ErrCode result = cmd.CheckCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": the auxiliary application " + argument.bundleName + "/" +
        argument.abilityName + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCommandArgument_0500 end";
}
}  // namespace AAFwk
}  // namespace OHOS