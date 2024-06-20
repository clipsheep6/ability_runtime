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

extern void MockSetBrightnessDiscount(bool mockRet);

namespace OHOS {
namespace AAFwk {
class AccessibilityAbilityShellCommandFirstTest : public ::testing::Test {
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

void AccessibilityAbilityShellCommandFirstTest::SetUpTestCase()
{}

void AccessibilityAbilityShellCommandFirstTest::TearDownTestCase()
{}

void AccessibilityAbilityShellCommandFirstTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AccessibilityAbilityShellCommandFirstTest::TearDown()
{}

/**
 * @tc.number: AccessibilityAbilityShellCommand_init_0100
 * @tc.name: init
 * @tc.desc: Test whether init is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest, AccessibilityAbilityShellCommand_init_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_init_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.init(), OHOS::ERR_OK);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_init_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CreateMessageMap_0100
 * @tc.name: CreateMessageMap
 * @tc.desc: Test whether CreateMessageMap is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CreateMessageMap_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CreateMessageMap_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.CreateMessageMap(), OHOS::ERR_OK);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CreateMessageMap_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsHelpCommand_0100
 * @tc.name: RunAsHelpCommand
 * @tc.desc: Test whether RunAsHelpCommand is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsHelpCommand_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsHelpCommand_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdHelp_.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.RunAsHelpCommand(), OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_HELP_MSG);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsHelpCommand_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_GetEnabledAbilities_0100
 * @tc.name: GetEnabledAbilities
 * @tc.desc: Test whether GetEnabledAbilities is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_GetEnabledAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_GetEnabledAbilities_0100 start";
    AccessibilityCommandArgument argument;
    std::vector<std::string> enabledAbilities;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    enabledAbilities = cmd.GetEnabledAbilities();
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_GetEnabledAbilities_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_GetInstalledAbilities_0100
 * @tc.name: GetInstalledAbilities
 * @tc.desc: Test whether GetInstalledAbilities is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_GetInstalledAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_GetInstalledAbilities_0100 start";
    AccessibilityCommandArgument argument;
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    installedAbilities = cmd.GetInstalledAbilities();
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_GetInstalledAbilities_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckAbilityArgument_0100
 * @tc.name: CheckAbilityArgument
 * @tc.desc: Test whether CheckAbilityArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckAbilityArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0100 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckAbilityArgument(argument, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckAbilityArgument_0200
 * @tc.name: CheckAbilityArgument
 * @tc.desc: Test whether CheckAbilityArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckAbilityArgument_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0200 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 2;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckAbilityArgument(argument, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckAbilityArgument_0300
 * @tc.name: CheckAbilityArgument
 * @tc.desc: Test whether CheckAbilityArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckAbilityArgument_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0300 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName[0] = '-';
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckAbilityArgument(argument, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckAbilityArgument_0400
 * @tc.name: CheckAbilityArgument
 * @tc.desc: Test whether CheckAbilityArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckAbilityArgument_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0400 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckAbilityArgument(argument, resultMessage);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckAbilityArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckBundleArgument_0100
 * @tc.name: CheckBundleArgument
 * @tc.desc: Test whether CheckBundleArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckBundleArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0100 start";
    AccessibilityCommandArgument argument;
    argument.bundleArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckBundleArgument(argument, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckBundleArgument_0200
 * @tc.name: CheckBundleArgument
 * @tc.desc: Test whether CheckAbilityArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckBundleArgument_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0200 start";
    AccessibilityCommandArgument argument;
    argument.bundleArgumentNum = 2;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckBundleArgument(argument, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckBundleArgument_0300
 * @tc.name: CheckBundleArgument
 * @tc.desc: Test whether CheckBundleArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckBundleArgument_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0300 start";
    AccessibilityCommandArgument argument;
    argument.bundleArgumentNum = 1;
    argument.bundleName[0] = '-';
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckBundleArgument(argument, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckBundleArgument_0400
 * @tc.name: CheckBundleArgument
 * @tc.desc: Test whether CheckBundleArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckBundleArgument_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0400 start";
    AccessibilityCommandArgument argument;
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckBundleArgument(argument, resultMessage);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckBundleArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0100
 * @tc.name: CheckCapabilitiesArgument
 * @tc.desc: Test whether CheckCapabilitiesArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0100 start";
    AccessibilityCommandArgument argument;
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    argument.capabilityNamesArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCapabilitiesArgument(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0200
 * @tc.name: CheckCapabilitiesArgument
 * @tc.desc: Test whether CheckCapabilitiesArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0200 start";
    AccessibilityCommandArgument argument;
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    argument.capabilityNamesArgumentNum = 2;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCapabilitiesArgument(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_DUPLICATE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0300
 * @tc.name: CheckCapabilitiesArgument
 * @tc.desc: Test whether CheckCapabilitiesArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0300 start";
    AccessibilityCommandArgument argument;
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    argument.capabilityNamesArgumentNum = 1;
    argument.capabilityNames[0] = '-';
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCapabilitiesArgument(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0400
 * @tc.name: CheckCapabilitiesArgument
 * @tc.desc: Test whether CheckCapabilitiesArgument is called normally.
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0400 start";
    AccessibilityCommandArgument argument;
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    argument.capabilityNamesArgumentNum = 1;
    argument.capabilityNames = "capability";
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckCapabilitiesArgument(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckCapabilitiesArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckSetCommandArgument_0100
 * @tc.name: CheckSetCommandArgument
 * @tc.desc: Test whether CheckSetCommandArgument is called normally.(totalArgumentNum > 1)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckSetCommandArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0100 start";
    AccessibilityCommandArgument argument;
    argument.setArgumentNum = 2;
    argument.unknownArgumentNum = 1;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckSetCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": and exist duplicated arguments.and exist unknown arguments ");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckSetCommandArgument_0200
 * @tc.name: CheckSetCommandArgument
 * @tc.desc: Test whether CheckSetCommandArgument is called normally.(unknownArgumentNum > 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckSetCommandArgument_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0200 start";
    AccessibilityCommandArgument argument;
    argument.setArgumentNum = 0;
    argument.unknownArgumentNum = 1;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckSetCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": unknown arguments ");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckSetCommandArgument_0300
 * @tc.name: CheckSetCommandArgument
 * @tc.desc: Test whether CheckSetCommandArgument is called normally.(setArgumentNum = 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckSetCommandArgument_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0300 start";
    AccessibilityCommandArgument argument;
    argument.setArgumentNum = 0;
    argument.unknownArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckSetCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckSetCommandArgument_0400
 * @tc.name: CheckSetCommandArgument
 * @tc.desc: Test whether CheckSetCommandArgument is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckSetCommandArgument_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0400 start";
    AccessibilityCommandArgument argument;
    argument.setArgumentNum = 1;
    argument.unknownArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckSetCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_OK);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckSetCommandArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0100
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.(optind < 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0100 start";
    AccessibilityCommandArgument argument;
    optind = -1;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0200
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.(option = -1)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0200 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdEnable_.c_str(),
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "enable: " + ACCESSIBILITY_HELP_MSG_NO_OPTION);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0300
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.(option -a requires a value)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0300 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdEnable_.c_str(),
        (char*)"-a",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "enable: " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0400
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.(option -b requires a value)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0400 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdEnable_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "enable: " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0500
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.(option -c requires a value)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0500 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdEnable_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)"-c",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "enable: " + ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0500 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0600
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.
 *           (CheckEnableCommandArgument = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0600 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdEnable_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)"-c",
        (char*)"capabilities",
        (char*)"-v",
        (char*)"value",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "enable: " + ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT +
        "and exist unknown arguments.-v ");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0600 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0700
 * @tc.name: MakeEnableCommandArgumentFromCmd
 * @tc.desc: Test whether MakeEnableCommandArgumentFromCmd is called normally.
 *           (CheckEnableCommandArgument = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0700 start";
    AccessibilityCommandArgument argument;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdEnable_.c_str(),
        (char*)"-a",
        (char*)"ability",
        (char*)"-b",
        (char*)"bundle",
        (char*)"-c",
        (char*)"capabilities",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.MakeEnableCommandArgumentFromCmd(argument);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "enable: the auxiliary application bundle/ability" +
        ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_MakeEnableCommandArgumentFromCmd_0700 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckParamValidity_0100
 * @tc.name: CheckParamValidity
 * @tc.desc: Test whether CheckParamValidity is called normally.(return true)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckParamValidity_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckParamValidity_0100 start";
    Accessibility::AccessibilityAbilityInitParams params;
    params.name = "ability";
    params.bundleName = "bundle";
    Accessibility::AccessibilityAbilityInfo abilityInfo(params);
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    installedAbilities.push_back(abilityInfo);
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    bool result = cmd.CheckParamValidity(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, true);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckParamValidity_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckParamValidity_0200
 * @tc.name: CheckParamValidity
 * @tc.desc: Test whether CheckParamValidity is called normally.(isExisted = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckParamValidity_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckParamValidity_0200 start";
    Accessibility::AccessibilityAbilityInitParams params;
    params.name = "ability";
    params.bundleName = "bundle";
    Accessibility::AccessibilityAbilityInfo abilityInfo(params);
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    installedAbilities.push_back(abilityInfo);
    AccessibilityCommandArgument argument;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    bool result = cmd.CheckParamValidity(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, "the auxiliary application " + argument.bundleName + "/" +
        argument.abilityName + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckParamValidity_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckParamValidity_0300
 * @tc.name: CheckParamValidity
 * @tc.desc: Test whether CheckParamValidity is called normally.(invalidCapabilityNames.empty() = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckParamValidity_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckParamValidity_0300 start";
    Accessibility::AccessibilityAbilityInitParams params;
    params.name = "ability";
    params.bundleName = "bundle";
    params.staticCapabilities = 1;
    Accessibility::AccessibilityAbilityInfo abilityInfo(params);
    std::vector<Accessibility::AccessibilityAbilityInfo> installedAbilities;
    installedAbilities.push_back(abilityInfo);
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 1;
    argument.capabilityNames = "capability";
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    bool result = cmd.CheckParamValidity(argument, installedAbilities, resultMessage);
    EXPECT_EQ(result, false);
    EXPECT_EQ(resultMessage, "the capabilities capability" + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckParamValidity_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0100
 * @tc.name: CheckEnableCommandArgument
 * @tc.desc: Test whether CheckEnableCommandArgument is called normally.(totalArgumentNum > 3)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0100 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 2;
    argument.bundleArgumentNum = 1;
    argument.capabilityNamesArgumentNum = 1;
    argument.unknownArgumentNum = 1;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckEnableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": and exist duplicated argumentsand exist unknown arguments.");
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_ABILITY_TOO_MANY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0200
 * @tc.name: CheckEnableCommandArgument
 * @tc.desc: Test whether CheckEnableCommandArgument is called normally.(unknownArgumentNum > 0)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0200 start";
    AccessibilityCommandArgument argument;
    argument.unknownArgumentNum = 1;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckEnableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": unknown arguments.");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0300
 * @tc.name: CheckEnableCommandArgument
 * @tc.desc: Test whether CheckEnableCommandArgument is called normally.(CheckAbilityArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0300 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 0;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 1;
    argument.capabilityNames = "capabilities";
    argument.unknownArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckEnableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_ABILITY_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0400
 * @tc.name: CheckEnableCommandArgument
 * @tc.desc: Test whether CheckEnableCommandArgument is called normally.(CheckBundleArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0400 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 0;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 1;
    argument.capabilityNames = "capabilities";
    argument.unknownArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckEnableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_BUNDLE_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0500
 * @tc.name: CheckEnableCommandArgument
 * @tc.desc: Test whether CheckEnableCommandArgument is called normally.(CheckCapabilitiesArgument = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0500 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 0;
    argument.capabilityNames = "capabilities";
    argument.unknownArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckEnableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": " + ACCESSIBILITY_ABILITY_NO_CAPABILITIES_ARGUMENT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0500 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0600
 * @tc.name: CheckEnableCommandArgument
 * @tc.desc: Test whether CheckEnableCommandArgument is called normally.(CheckParamValidity = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0600 start";
    AccessibilityCommandArgument argument;
    argument.abilityArgumentNum = 1;
    argument.abilityName = "ability";
    argument.bundleArgumentNum = 1;
    argument.bundleName = "bundle";
    argument.capabilityNamesArgumentNum = 1;
    argument.capabilityNames = "capabilities";
    argument.unknownArgumentNum = 0;
    std::string resultMessage;
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.CheckEnableCommandArgument(argument, resultMessage);
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(resultMessage, ": the auxiliary application bundle/ability" + ACCESSIBILITY_ABILITY_NOT_FOUND);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_CheckEnableCommandArgument_0600 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsEnableAbility_0100
 * @tc.name: RunAsEnableAbility
 * @tc.desc: Test whether RunAsEnableAbility is called normally.
 *           (MakeEnableCommandArgumentFromCmd = false)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsEnableAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsEnableAbility_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsEnableAbility();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_ENABLE_ABILITY);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsEnableAbility_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsDisableAbility_0100
 * @tc.name: RunAsDisableAbility
 * @tc.desc: Test whether RunAsDisableAbility is called normally.
 *           (MakeDisableCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsDisableAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsDisableAbility_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsDisableAbility();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_DISABLE_ABILITY);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsDisableAbility_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsGetEnabledAbilities_0100
 * @tc.name: RunAsGetEnabledAbilities
 * @tc.desc: Test whether RunAsGetEnabledAbilities is called normally.(enabledAbilities is empty)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsGetEnabledAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsGetEnabledAbilities_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsGetEnabledAbilities();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsGetEnabledAbilities_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsGetInstalledAbilities_0100
 * @tc.name: RunAsGetInstalledAbilities
 * @tc.desc: Test whether RunAsGetInstalledAbilities is called normally.(installedAbilities is empty)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsGetInstalledAbilities_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsGetInstalledAbilities_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    cmd.abilityClientPtr_ = nullptr;
    ErrCode result = cmd.RunAsGetInstalledAbilities();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsGetInstalledAbilities_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0100
 * @tc.name: RunAsSetScreenMagnificationState
 * @tc.desc: Test whether RunAsSetScreenMagnificationState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetScreenMagnificationState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_SCREEN_MAGNIFICATION_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0200
 * @tc.name: RunAsSetScreenMagnificationState
 * @tc.desc: Test whether RunAsSetScreenMagnificationState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetScreenMagnificationState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_SCREEN_MAGNIFICATION_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0300
 * @tc.name: RunAsSetScreenMagnificationState
 * @tc.desc: Test whether RunAsSetScreenMagnificationState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetScreenMagnificationState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setScreenMagnificationState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_SCREEN_MAGNIFICATION_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetScreenMagnificationState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0100
 * @tc.name: RunAsSetShortKeyState
 * @tc.desc: Test whether RunAsSetShortKeyState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetShortKeyState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_SHORT_KEY_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0200
 * @tc.name: RunAsSetShortKeyState
 * @tc.desc: Test whether RunAsSetShortKeyState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetShortKeyState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_SHORT_KEY_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0300
 * @tc.name: RunAsSetShortKeyState
 * @tc.desc: Test whether RunAsSetShortKeyState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetShortKeyState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetShortKeyState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setShortKeyState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0400
 * @tc.name: RunAsSetShortKeyState
 * @tc.desc: Test whether RunAsSetShortKeyState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetShortKeyState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0100
 * @tc.name: RunAsSetMouseKeyState
 * @tc.desc: Test whether RunAsSetMouseKeyState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetMouseKeyState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseKeyState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_MOUSE_KEY_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0200
 * @tc.name: RunAsSetMouseKeyState
 * @tc.desc: Test whether RunAsSetMouseKeyState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetMouseKeyState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseKeyState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_MOUSE_KEY_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0300
 * @tc.name: RunAsSetMouseKeyState
 * @tc.desc: Test whether RunAsSetMouseKeyState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetMouseKeyState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseKeyState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setMouseKeyState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_MOUSE_KEY_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0400
 * @tc.name: RunAsSetMouseKeyState
 * @tc.desc: Test whether RunAsSetMouseKeyState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseKeyState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_MOUSE_KEY_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseKeyState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetCaptionState_0100
 * @tc.name: RunAsSetCaptionState
 * @tc.desc: Test whether RunAsSetCaptionState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetCaptionState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetCaptionState_.c_str(),
        (char*)"-v",
        (char*)"0",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetCaptionState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_CAPTION_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetCaptionState_0200
 * @tc.name: RunAsSetCaptionState
 * @tc.desc: Test whether RunAsSetCaptionState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetCaptionState_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetCaptionState_.c_str(),
        (char*)"-v",
        (char*)"1",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetCaptionState();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_CAPTION_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetCaptionState_0300
 * @tc.name: RunAsSetCaptionState
 * @tc.desc: Test whether RunAsSetCaptionState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetCaptionState_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsetCaptionState_.c_str(),
        (char*)"-v",
        (char*)"2",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetCaptionState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "setCaptionState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_CAPTION_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetCaptionState_0400
 * @tc.name: RunAsSetCaptionState
 * @tc.desc: Test whether RunAsSetCaptionState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetCaptionState_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0400 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetCaptionState();
    EXPECT_EQ(result, OHOS::ERR_INVALID_VALUE);
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_CAPTION_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetCaptionState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsSetMouseAutoClick_.c_str(),
        (char*)"-v",
        (char*)"1000",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseAutoClick();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUTO_CLICK_TIME_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsSetMouseAutoClick_.c_str(),
        (char*)"-v",
        (char*)"5000",
        (char*)" ",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AccessibilityAbilityShellCommand cmd(argc, argv);
    ErrCode result = cmd.RunAsSetMouseAutoClick();
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUTO_CLICK_TIME_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300
 * @tc.name: RunAsSetMouseAutoClick
 * @tc.desc: Test whether RunAsSetMouseAutoClick is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0300 start";
    char* argv[] = {
        (char*)ACCESSIBILITY_TOOL_NAME.c_str(),
        (char*)cmdsSetMouseAutoClick_.c_str(),
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
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_AUTO_CLICK_TIME);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetMouseAutoClick_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetShortKeyTarget_0100
 * @tc.name: RunAsSetShortKeyTarget
 * @tc.desc: Test whether RunAsSetShortKeyTarget is called normally.
 *           (MakeSetShortKeyTargetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_SHORT_KEY_TARGET);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetShortKeyTarget_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0100
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0200
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_HIGH_CONTRAST_TEXT_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0300
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setHighContrastTextState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_HIGH_CONTRAST_TEXT_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetHighContrastTextState_0400
 * @tc.name: RunAsSetHighContrastTextState
 * @tc.desc: Test whether RunAsSetHighContrastTextState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_INVERT_COLOR_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0200
 * @tc.name: RunAsSetInvertColorState
 * @tc.desc: Test whether RunAsSetInvertColorState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_INVERT_COLOR_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0300
 * @tc.name: RunAsSetInvertColorState
 * @tc.desc: Test whether RunAsSetInvertColorState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_INVERT_COLOR_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetInvertColorState_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0100
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0200
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0300
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0400
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_DALTONIZATIONZATION_COLOR_FILTER_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0400 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0500
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setDaltonizationColorFilter: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0500 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0600
 * @tc.name: RunAsSetDaltonizationColorFilter
 * @tc.desc: Test whether RunAsSetDaltonizationColorFilter is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "\n" + ACCESSIBILITY_HELP_MSG_SET_DALTONIZATION_COLOR_FILTER);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetDaltonizationColorFilter_0600 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0100
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_CONTENT_TIME_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0200
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_CONTENT_TIME_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0300
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setContentTimeout: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_CONTENT_TIME_OUT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetContentTimeout_0400
 * @tc.name: RunAsSetContentTimeout
 * @tc.desc: Test whether RunAsSetContentTimeout is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_ANIMATION_OFF_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0200
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_ANIMATION_OFF_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0300
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setAnimationOffState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_ANIMATION_OFF_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAnimationOffState_0400
 * @tc.name: RunAsSetAnimationOffState
 * @tc.desc: Test whether RunAsSetAnimationOffState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
 * @tc.desc: 1.Test whether RunAsSetBrightnessDiscount is called normally.(OHOS::ERR_OK)
 *           2.ret is not Accessibility::RET_OK
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    MockSetBrightnessDiscount(false);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_OK + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0200
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: 1.Test whether RunAsSetBrightnessDiscount is called normally.(OHOS::ERR_OK)
 *           2.ret is Accessibility::RET_OK
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    MockSetBrightnessDiscount(true);
    EXPECT_EQ(result, OHOS::ERR_OK);
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_BRIGHTNESS_DISCOUNT_NG);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0300
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: Test whether RunAsSetBrightnessDiscount is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setSetBrightnessDiscount: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_BRIGHTNESS_DISCOUNT);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetBrightnessDiscount_0400
 * @tc.name: RunAsSetBrightnessDiscount
 * @tc.desc: Test whether RunAsSetBrightnessDiscount is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_MONO_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0100 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0200
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.(OHOS::ERR_OK)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, ACCESSIBILITY_SET_AUDIO_MONO_STATE_NG + "\n");
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0200 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0300
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.(value is invalid)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
    EXPECT_EQ(cmd.resultReceiver_, "setAudioMonoState: " + ACCESSIBILITY_ABILITY_SET_VALUE_INVALID +
        "\n" + ACCESSIBILITY_HELP_MSG_SET_AUDIO_MONO_STATE);
    GTEST_LOG_(INFO) << "AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0300 end";
}

/**
 * @tc.number: AccessibilityAbilityShellCommand_RunAsSetAudioMonoState_0400
 * @tc.name: RunAsSetAudioMonoState
 * @tc.desc: Test whether RunAsSetAudioMonoState is called normally.
 *           (MakeSetCommandArgumentFromCmd = OHOS::ERR_INVALID_VALUE)
 */
HWTEST_F(AccessibilityAbilityShellCommandFirstTest,
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
}  // namespace AAFwk
}  // namespace OHOS
