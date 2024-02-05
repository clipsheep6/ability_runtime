/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#define protected public
#include "ability_command.h"
#undef protected
#include "mock_ability_manager_stub.h"
#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_interface.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
const std::string STRING_DEVICE = "device";
const std::string STRING_ABILITY_NAME = "ability";
const std::string STRING_ABILITY_NAME_INVALID = "invalid_ability";
const std::string STRING_BUNDLE_NAME = "bundle";
const std::string STRING_BUNDLE_NAME_INVALID = "invalid_bundle";
const std::string STRING_RECORD_ID = "1024";
const std::string STRING_RECORD_ID_INVALID = "2048";
const std::string STRING_STATE_ON = "on";
const std::string STRING_STATE_ON_INVALID = "invalid_on";
const std::string STRING_STATE_OFF = "off";
const std::string STRING_STATE_OFF_INVALID = "invalid_off";
const std::string STRING_INVALID_PARAMETER_INTEGER_OPTION = "invalid parameter invalid_integer for integer option";
const std::string STRING_INVALID_PARAMETER_BOOL_OPTION = "invalid parameter invalid_bool for bool option";
const std::string STRING_INVALID_NUMBER_INTEGER_OPTION = "invalid number of parameters for option --pi";
const std::string STRING_INVALID_NUMBER_BOOL_OPTION = "invalid number of parameters for option --pb";
const std::string STRING_INVALID_NUMBER_STRING_OPTION = "invalid number of parameters for option --ps";
const std::string STRING_INVALID_NUMBER_NULL_STRING_OPTION = "invalid number of parameters for option --psn";
const std::string STRING_ACTION = "action";
const std::string STRING_URI = "https://valid.uri.com";
const std::string STRING_TYPE = "type";
const std::string STRING_ENTITY = "entity";
const std::string STRING_PARAMETER_KEY_INTEGER = "kinteger";
const std::string STRING_PARAMETER_KEY_BOOL = "kbool";
const std::string STRING_PARAMETER_KEY_STRING = "kstring";
const std::string STRING_PARAMETER_KEY_NULL_STRING = "knullstring";
const std::string STRING_PARAMETER_VALUE_INTEGER = "100";
const std::string STRING_PARAMETER_VALUE_BOOL = "true";
const std::string STRING_PARAMETER_VALUE_STRING = "string_value";
const std::string STRING_PARAMETER_VALUE_INTEGER_INVALID = "invalid_integer";
const std::string STRING_PARAMETER_VALUE_BOOL_INVALID = "invalid_bool";
const std::string STRING_PARAMETER_VALUE_STRING_INVALID = "invalid_string_value";
}  // namespace

class AaCommandStartTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "start";
};

void AaCommandStartTest::SetUpTestCase()
{}

void AaCommandStartTest::TearDownTestCase()
{}

void AaCommandStartTest::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandStartTest::TearDown()
{}

void AaCommandStartTest::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_Start_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0100, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start xxx" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0200, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -x" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0300, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-x",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -xxx" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0400, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --x" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0500, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--x",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --xxx" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0600, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "fail: unknown option.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -h" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0700, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-h",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start --help" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0800, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"--help",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_0900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_0900, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_0900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1000, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_1100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1100, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1200, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_NO_BUNDLE_NAME_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -b" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1300, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -b <bundle-name>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1400, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_1500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1500, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1600, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_1700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -a" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1700, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -a <ability-name> -b" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1800, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: option requires a value.\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_1900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -a <ability-name> -b <bundle-name>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_1900, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_1900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -a <ability-name> -b <bundle-name> -D" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2000, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2100, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME_INVALID.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_NG + "\n" + cmd.messageMap_.at(RESOLVE_ABILITY_ERR) + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name>" command.
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2200, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_NG + "\n" + cmd.messageMap_.at(RESOLVE_APP_ERR) + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -D" command.
 * @tc.type: FUNC
 * @tc.require: SR000GH1HD
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2300, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -D" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2400, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2400");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -D" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2500, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), "error: -b <bundle-name> is expected\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_2600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -b <bundle-name> -D" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2600, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> -D" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2700, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"-D",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --pi <key> <integer-value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2800, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)STRING_PARAMETER_KEY_INTEGER.c_str(),
        (char*)STRING_PARAMETER_VALUE_INTEGER.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_2900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --pb <key> <bool-value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_2900, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_2900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)STRING_PARAMETER_KEY_BOOL.c_str(),
        (char*)STRING_PARAMETER_VALUE_BOOL.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_3000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --ps <key> <value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3000, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--ps",
        (char*)STRING_PARAMETER_KEY_STRING.c_str(),
        (char*)STRING_PARAMETER_VALUE_STRING.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_3100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --psn <key>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3100, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--psn",
        (char*)STRING_PARAMETER_KEY_NULL_STRING.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_3200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -A <action>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3200, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3200");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-A",
        (char*)STRING_ACTION.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_3300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -U <URI> -t <type>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3300, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3300");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-U",
        (char*)STRING_URI.c_str(),
        (char*)"-t",
        (char*)STRING_TYPE.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_3500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -e <entity>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3500, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3500");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-e",
        (char*)STRING_ENTITY.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_START_ABILITY_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Start_3600
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --pi <key> <integer-value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3600, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3600");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)STRING_PARAMETER_KEY_INTEGER.c_str(),
        (char*)STRING_PARAMETER_VALUE_INTEGER_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_INVALID_PARAMETER_INTEGER_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_3700
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --pb <key> <bool-value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3700, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3700");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)STRING_PARAMETER_KEY_BOOL.c_str(),
        (char*)STRING_PARAMETER_VALUE_BOOL_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_INVALID_PARAMETER_BOOL_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_3800
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --ps <key> <value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3800, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3800");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--ps",
        (char*)STRING_PARAMETER_KEY_STRING.c_str(),
        (char*)STRING_PARAMETER_VALUE_STRING.c_str(),
        (char*)STRING_PARAMETER_VALUE_STRING_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_INVALID_NUMBER_STRING_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_3900
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --pi <key> <integer-value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_3900, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_3900");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pi",
        (char*)STRING_PARAMETER_KEY_INTEGER.c_str(),
        (char*)STRING_PARAMETER_VALUE_INTEGER.c_str(),
        (char*)STRING_PARAMETER_VALUE_INTEGER_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_INVALID_NUMBER_INTEGER_OPTION + "\n" + HELP_MSG_START);
}

/**
 * @tc.number: Aa_Command_Start_4000
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --pb <key> <bool-value>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_4000, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_4000");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--pb",
        (char*)STRING_PARAMETER_KEY_BOOL.c_str(),
        (char*)STRING_PARAMETER_VALUE_BOOL.c_str(),
        (char*)STRING_PARAMETER_VALUE_BOOL_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_INVALID_NUMBER_BOOL_OPTION + "\n" + HELP_MSG_START);
}
/**
 * @tc.number: Aa_Command_Start_4100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa start -d <device-id> -a <ability-name> -b <bundle-name> --psn <key>" command.
 * @tc.type: FUNC
 * @tc.require: AR000GJUN4
 */
HWTEST_F(AaCommandStartTest, Aa_Command_Start_4100, Function | MediumTest | Level1)
{
    HILOG_INFO("Aa_Command_Start_4100");

    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"-d",
        (char*)STRING_DEVICE.c_str(),
        (char*)"-a",
        (char*)STRING_ABILITY_NAME.c_str(),
        (char*)"-b",
        (char*)STRING_BUNDLE_NAME.c_str(),
        (char*)"--psn",
        (char*)STRING_PARAMETER_KEY_NULL_STRING.c_str(),
        (char*)STRING_PARAMETER_VALUE_STRING_INVALID.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_INVALID_NUMBER_NULL_STRING_OPTION + "\n" + HELP_MSG_START);
}
