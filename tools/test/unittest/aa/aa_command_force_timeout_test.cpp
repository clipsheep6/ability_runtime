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
#include <gmock/gmock.h>

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
using testing::_;
using testing::Invoke;
using testing::Return;

class AaCommandForceTimeOut : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void MakeMockObjects() const;

    std::string cmd_ = "force-timeout";
};

void AaCommandForceTimeOut::SetUpTestCase()
{}

void AaCommandForceTimeOut::TearDownTestCase()
{}

void AaCommandForceTimeOut::SetUp()
{
    // reset optind to 0
    optind = 0;

    // make mock objects
    MakeMockObjects();
}

void AaCommandForceTimeOut::TearDown()
{}

void AaCommandForceTimeOut::MakeMockObjects() const
{
    // mock a stub
    auto managerStubPtr = sptr<IAbilityManager>(new MockAbilityManagerStub());

    // set the mock stub
    auto managerClientPtr = AbilityManagerClient::GetInstance();
    managerClientPtr->proxy_ = managerStubPtr;
}

/**
 * @tc.number: Aa_Command_Force_Timeout_0100
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa force-timeout" command.
 */
HWTEST_F(AaCommandForceTimeOut, Aa_Command_Force_Timeout_0100, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_FORCE_TIMEOUT + "\n");
}

/**
 * @tc.number: Aa_Command_Force_Timeout_0200
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa force-timeout xxx" command.
 */
HWTEST_F(AaCommandForceTimeOut, Aa_Command_Force_Timeout_0200, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"xxx",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), HELP_MSG_FORCE_TIMEOUT + "\n");
}

/**
 * @tc.number: Aa_Command_Force_Timeout_0300
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa force-timeout clean" command.
 */
HWTEST_F(AaCommandForceTimeOut, Aa_Command_Force_Timeout_0300, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"clean",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    AbilityManagerShellCommand cmd(argc, argv);
    EXPECT_EQ(cmd.ExecCommand(), STRING_FORCE_TIMEOUT_OK + "\n");
}

/**
 * @tc.number: Aa_Command_Force_Timeout_0400
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa force-timeout xxx INITIAL" command.
 */
HWTEST_F(AaCommandForceTimeOut, Aa_Command_Force_Timeout_0400, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"clean",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, ForceTimeoutForTest(_, _))
        .Times(1)
        .WillOnce(Return(-1));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.ExecCommand(), STRING_FORCE_TIMEOUT_NG + "\n");
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}

/**
 * @tc.number: Aa_Command_Force_Timeout_0500
 * @tc.name: ExecCommand
 * @tc.desc: Verify the "aa force-timeout ability INITIAL" command.
 */
HWTEST_F(AaCommandForceTimeOut, Aa_Command_Force_Timeout_0500, Function | MediumTest | Level1)
{
    char* argv[] = {
        (char*)TOOL_NAME.c_str(),
        (char*)cmd_.c_str(),
        (char*)"clean",
        (char*)"",
    };
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    AbilityManagerShellCommand cmd(argc, argv);

    auto managerClientPtr = AbilityManagerClient::GetInstance();
    auto mockAbilityManagerStub = sptr<MockAbilityManagerStub>(new MockAbilityManagerStub());
    ASSERT_NE(mockAbilityManagerStub, nullptr);
    EXPECT_CALL(*mockAbilityManagerStub, ForceTimeoutForTest(_, _))
        .Times(1)
        .WillOnce(Return(0));
    managerClientPtr->proxy_ = static_cast<IAbilityManager*>(mockAbilityManagerStub);

    EXPECT_EQ(cmd.ExecCommand(), STRING_FORCE_TIMEOUT_OK + "\n");
    testing::Mock::AllowLeak(mockAbilityManagerStub);
}