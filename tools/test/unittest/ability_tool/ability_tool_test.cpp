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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "ability_tool_command.h"
#undef protected
#include "mock_ability_manager_stub.h"
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string ABILITY_TOOL_HELP_MSG =
    "usage: ability_tool <command> <options>\n"
    "ability_tool commands list:\n"
    "  help                        list available commands\n"
    "  start                       start ability with options\n"
    "  stop-service                stop service with options\n"
    "  force-stop                  force stop the process with bundle name\n"
    "  test                        start the test framework with options\n";

const std::string ABILITY_TOOL_HELP_MSG_START =
    "usage: ability_tool start <options>\n"
    "ability_tool start options list:\n"
    "  --help                      list available options\n"
    "  --device <device-id>        device Id\n"
    "  --ability <ability-name>    ability name, mandatory\n"
    "  --bundle <bundle-name>      bundle name, mandatory\n"
    "  --options <key> <value>     start options, such as windowMode 102\n"
    "  --flags <flag>              flags in a want\n"
    "  -C                          cold start\n"
    "  -D                          start with debug mode\n";

const std::string ABILITY_TOOL_HELP_MSG_STOP_SERVICE =
    "usage: ability_tool stop-service <options>\n"
    "ability_tool stop-service options list:\n"
    "  --help                      list available options\n"
    "  --device <device-id>        device Id\n"
    "  --ability <ability-name>    ability name, mandatory\n"
    "  --bundle <bundle-name>      bundle name, mandatory\n";

const std::string ABILITY_TOOL_HELP_MSG_FORCE_STOP =
    "usage: ability_tool force-stop <options>\n"
    "ability_tool force-stop options list:\n"
    "  --help                      list available options\n"
    "  <bundle-name>               bundle name, mandatory\n";

const std::string ABILITY_TOOL_HELP_MSG_TEST =
    "usage: ability_tool test <options>\n"
    "ability_tool test options list:\n"
    "  --help                              list available options\n"
    "  --bundle <bundle-name>              bundle name, mandatory\n"
    "  --options unittest <test-runner>    test runner need to start, mandatory\n"
    "  --package-name <package-name>       package name, required for the FA model\n"
    "  --module-name <module-name>         module name, required for the STAGE model\n"
    "  --options <key> <value>             test options, such as testcase test_001\n"
    "  --watchdog <wait-time>              max execute time for this test\n"
    "  -D                                  test with debug mode\n";

const std::string ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION = "error: --ability <ability-name> is expected";
const std::string ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION = "error: --bundle <bundle-name> is expected";
const std::string ABILITY_TOOL_HELP_MSG_WINDOW_MODE_INVALID = "error: --options windowMode <value> with invalid param";
const std::string ABILITY_TOOL_HELP_MSG_LACK_VALUE = "error: lack of value of key";
const std::string ABILITY_TOOL_HELP_MSG_ONLY_NUM = "error: current option only support number";
const std::string ABILITY_TOOL_HELP_LACK_OPTIONS = "error: lack of essential args";
} // namespace

class AbilityToolTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityToolTest::SetUpTestCase()
{}

void AbilityToolTest::TearDownTestCase()
{}

void AbilityToolTest::SetUp()
{
    // reset optind to 0
    optind = 0;
}

void AbilityToolTest::TearDown()
{}

/**
 * @tc.name: AbilityTool_Cmd_0100
 * @tc.desc: "ability_tool" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Cmd_0100, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG);
}

/**
 * @tc.name: AbilityTool_Cmd_0200
 * @tc.desc: invalid options "ability_tool xxx" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Cmd_0200, TestSize.Level1)
{
    // "ability_tool"
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("xxx"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), cmd.GetCommandErrorMsg() + ABILITY_TOOL_HELP_MSG);
}

/**
 * @tc.name: AbilityTool_Cmd_0300
 * @tc.desc: "ability_tool help" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Cmd_0300, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("help"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG);
}

/**
 * @tc.name: AbilityTool_Start_0100
 * @tc.desc: "ability_tool start" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0100, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_START);
}

/**
 * @tc.name: AbilityTool_Start_0200
 * @tc.desc: "ability_tool start --help" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0200, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--help"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_START);
}

/**
 * @tc.name: AbilityTool_Start_0300
 * @tc.desc: without bundleName "ability_tool start --ability TestAbility" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0300, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_START);
}

/**
 * @tc.name: AbilityTool_Start_0400
 * @tc.desc: without abilityName "ability_tool start --bundle com.example.abilitytooltest" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0400, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_START);
}

/**
 * @tc.name: AbilityTool_Start_0500
 * @tc.desc: lack of windowMode test.
 * "ability_tool start --ability TestAbility --bundle com.example.abilitytooltest --options windowMode"
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0500, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("windowMode"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n" +
        ABILITY_TOOL_HELP_MSG_START);
}

/**
 * @tc.name: AbilityTool_Start_0600
 * @tc.desc: invalid windowMode test.
 * "ability_tool start --ability TestAbility --bundle com.example.abilitytooltest --options windowMode 20"
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0600, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("windowMode"),
        const_cast<char*>("20"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_NE(cmd.ExecCommand().find(ABILITY_TOOL_HELP_MSG_START), string::npos);
}

/**
 * @tc.name: AbilityTool_Start_0700
 * @tc.desc: flag isn't num test. flag is not a number, didn't parse.
 * "ability_tool start --ability TestAbility --bundle com.example.abilitytooltest --flags abc"
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0700, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--flags"),
        const_cast<char*>("abc"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_NG + "\n");
}

/**
 * @tc.name: AbilityTool_StopService_0100
 * @tc.desc: "ability_tool stop-service" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0100, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_STOP_SERVICE);
}

/**
 * @tc.name: AbilityTool_StopService_0200
 * @tc.desc: "ability_tool stop-service --help" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0200, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
        const_cast<char*>("--help"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_STOP_SERVICE);
}

/**
 * @tc.name: AbilityTool_StopService_0300
 * @tc.desc: without bundleName "ability_tool stop-service --ability TestAbility" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0300, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_STOP_SERVICE);
}

/**
 * @tc.name: AbilityTool_StopService_0400
 * @tc.desc: without abilityName "ability_tool stop-service --ability TestAbility" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0400, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
        ABILITY_TOOL_HELP_MSG_STOP_SERVICE);
}

/**
 * @tc.name: AbilityTool_StopService_0500
 * @tc.desc: stop-service "ability_tool stop-service --ability TestAbility" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0500, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
        const_cast<char*>("--ability"),
        const_cast<char*>("com.ohos.screenshot.ServiceExtAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.ohos.screenshot"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_STOP_SERVICE_ABILITY_NG + "\n");
}

/**
 * @tc.name: AbilityTool_ForceStop_0100
 * @tc.desc: "ability_tool force-stop" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_ForceStop_0100, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("force-stop"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_FORCE_STOP);
}

/**
 * @tc.name: AbilityTool_Test_0100
 * @tc.desc: "ability_tool test" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0100, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_LACK_OPTIONS + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

/**
 * @tc.name: AbilityTool_Test_0200
 * @tc.desc: "ability_tool test --help" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0200, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--help"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_LACK_OPTIONS + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

/**
 * @tc.name: AbilityTool_Test_0300
 * @tc.desc: without bundleName "ability_tool test --options unittest testRunner" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0300, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--options"),
        const_cast<char*>("unittest"),
        const_cast<char*>("testRunner"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_NE(cmd.ExecCommand().find(ABILITY_TOOL_HELP_MSG_TEST), string::npos);
}

/**
 * @tc.name: AbilityTool_Test_0400
 * @tc.desc: without unittest "ability_tool test --bundle com.example.abilitytooltest" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0400, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_LACK_OPTIONS + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

/**
 * @tc.name: AbilityTool_Test_0500
 * @tc.desc: without value of unittest "ability_tool test --bundle com.example.abilitytooltest --options unittest" test.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0500, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("unittest"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

/**
 * @tc.number: AbilityTool_Test_0600
 * @tc.name: RunAsTestCommand
 * @tc.desc: "ability_tool test --bundle com.example.abilitytooltest --options --package-name unittest"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0600, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("--package-name"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

 /**
 * @tc.number: AbilityTool_Test_0700
 * @tc.name: RunAsTestCommand
 * @tc.desc: "ability_tool test --bundle com.example.abilitytooltest --options --module-name unittest"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0700, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("--module-name"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

 /**
 * @tc.number: AbilityTool_Test_0800
 * @tc.name: RunAsTestCommand
 * @tc.desc: "ability_tool test --bundle com.example.abilitytooltest --options --watchdog unittest"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0800, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("--watchdog"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

 /**
 * @tc.number: AbilityTool_Test_0900
 * @tc.name: RunAsTestCommand
 * @tc.desc: "ability_tool test --bundle com.example.abilitytooltest --options -D unittest"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Test_0900, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("test"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--options"),
        const_cast<char*>("-D"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_LACK_VALUE + "\n" + ABILITY_TOOL_HELP_MSG_TEST);
}

/**
 * @tc.number: AbilityTool_Start_0800
 * @tc.name: RunAsStartAbility
 * @tc.desc: Run As StartAbility sucess
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0800, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--flags"),
        const_cast<char*>("abc"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.RunAsStartAbility(), OHOS::ERR_OK);
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: AbilityTool_Start_0900
 * @tc.name: RunAsStartAbility
 * @tc.desc: "ability_tool start --device --ability"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_0900, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--device"),
        const_cast<char*>("--ability"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
    ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + ABILITY_TOOL_HELP_MSG_START);
}

/**
 * @tc.number: AbilityTool_Start_1000
 * @tc.name: RunAsStartAbility
 * @tc.desc: "ability_tool start --ability TestAbility --bundle com.example.abilitytooltest --flags --cold-start"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_1000, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--flags"),
        const_cast<char*>("--cold-start"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: AbilityTool_Start_1100
 * @tc.name: RunAsStartAbility
 * @tc.desc: "ability_tool start --ability TestAbility --bundle com.example.abilitytooltest --flags -D"
 */
HWTEST_F(AbilityToolTest, AbilityTool_Start_1100, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("start"),
        const_cast<char*>("--ability"),
        const_cast<char*>("TestAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.example.abilitytooltest"),
        const_cast<char*>("--flags"),
        const_cast<char*>("-D"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.name: AbilityTool_StopService_0600
 * @tc.desc: RunAsStopService
 * @tc.type: stop-service "ability_tool stop-service --ability TestAbility" test.
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0600, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
        const_cast<char*>("--ability"),
        const_cast<char*>("com.ohos.screenshot.ServiceExtAbility"),
        const_cast<char*>("--bundle"),
        const_cast<char*>("com.ohos.screenshot"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.RunAsStopService(), OHOS::ERR_OK);
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.name: AbilityTool_StopService_0700
 * @tc.desc: RunAsStopService
 * @tc.type: stop-service "ability_tool stop-service --device --ability" test.
 */
HWTEST_F(AbilityToolTest, AbilityTool_StopService_0700, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("stop-service"),
        const_cast<char*>("--device"),
        const_cast<char*>("--ability"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), ABILITY_TOOL_HELP_MSG_NO_ABILITY_NAME_OPTION + "\n" +
    ABILITY_TOOL_HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + ABILITY_TOOL_HELP_MSG_STOP_SERVICE);
}

/**
 * @tc.name: AbilityTool_ForceStop_0200
 * @tc.desc: RunAsForceStop
 * @tc.type: "ability_tool force-stop" test
 */
HWTEST_F(AbilityToolTest, AbilityTool_ForceStop_0200, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("force-stop"),
        const_cast<char*>("com.ohos.screenshot.ServiceExtAbility"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_FORCE_STOP_OK + "\n");
}

/**
 * @tc.name: AbilityTool_ForceStop_0300
 * @tc.desc: RunAsForceStop
 * @tc.type: "ability_tool force-stop" test
 */
HWTEST_F(AbilityToolTest, AbilityTool_ForceStop_0300, TestSize.Level1)
{
    char* argv[] = {
        const_cast<char*>("ability_tool"),
        const_cast<char*>("force-stop"),
        const_cast<char*>("com.ohos.screenshot.ServiceExtAbility"),
    };
    int argc = sizeof(argv) / sizeof(argv[0]);
    AbilityToolCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, KillProcess(testing::_))
        .Times(1)
        .WillOnce(testing::Return(0));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.RunAsForceStop(), OHOS::ERR_OK);
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}
} // namespace AAFwk
} // namespace OHOS
