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
#include "ability_manager_service.h"
#include "user_controller.h"
#undef private
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UserControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UserControllerTest::SetUpTestCase()
{
    auto abilityMs = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    AmsConfigurationParameter::GetInstance().Parse();
}

void UserControllerTest::TearDownTestCase()
{}

void UserControllerTest::SetUp()
{}

void UserControllerTest::TearDown()
{}

/**
 * @tc.name: UserItemSetState_0100
 * @tc.desc: UserItemSetState Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, UserItemSetState_0100, TestSize.Level0)
{
    UserItem item(100);
    item.SetState(UserState::STATE_BOOTING);
    EXPECT_TRUE(item.GetState() == UserState::STATE_BOOTING);

    item.SetState(UserState::STATE_STARTED);
    EXPECT_TRUE(item.GetState() == UserState::STATE_STARTED);
}

/**
 * @tc.name: StartUserTest_0100
 * @tc.desc: StartUser Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, StartUserTest_0100, TestSize.Level0)
{
    UserController userController;
    userController.StartUser(0, true);
    userController.StartUser(-1, true);
    userController.StartUser(100, true);
    EXPECT_TRUE(userController.StartUser(100, true) == 0);
}

/**
 * @tc.name: StartUserTest_0200
 * @tc.desc: StartUser Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, StartUserTest_0200, TestSize.Level0)
{
    UserController userController;
    EXPECT_TRUE(userController.StartUser(666, true) == -1);
}

/**
 * @tc.name: StopUserTest_0100
 * @tc.desc: StopUser Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, StopUserTest_0100, TestSize.Level0)
{
    UserController userController;
    userController.StopUser(-1);
    userController.StopUser(0);
    EXPECT_TRUE(userController.StopUser(100) == -1);
}

/**
 * @tc.name: StopUserTest_0200
 * @tc.desc: StopUser Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, StopUserTest_0200, TestSize.Level0)
{
    UserController userController;
    EXPECT_TRUE(userController.StopUser(666) == -1);
}

/**
 * @tc.name: StopUserTest_0300
 * @tc.desc: StopUser Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, StopUserTest_0300, TestSize.Level0)
{
    UserController userController;
    userController.GetOrCreateUserItem(1000);
    userController.StopUser(1000);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: HandleContinueUserSwitchTest_0100
 * @tc.desc: HandleContinueUserSwitch Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, HandleContinueUserSwitchTest_0100, TestSize.Level0)
{
    UserController userController;
    auto userItem = std::make_shared<UserItem>(1000);
    userController.HandleContinueUserSwitch(1000, 1000, userItem);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendUserSwitchDoneTest_0100
 * @tc.desc: SendUserSwitchDone Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, SendUserSwitchDoneTest_0100, TestSize.Level0)
{
    UserController userController;
    userController.SendUserSwitchDone(1000);
    userController.Init();
    userController.SendUserSwitchDone(1001);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendContinueUserSwitchTest_0200
 * @tc.desc: SendContinueUserSwitch Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, SendContinueUserSwitchTest_0200, TestSize.Level0)
{
    UserController userController;
    auto userItem = std::make_shared<UserItem>(1000);
    userController.SendContinueUserSwitch(1000, 1000, userItem);
    userController.Init();
    userController.SendContinueUserSwitch(1000, 1000, userItem);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendUserSwitchTimeoutTest_0100
 * @tc.desc: SendUserSwitchTimeout Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, SendUserSwitchTimeoutTest_0100, TestSize.Level0)
{
    UserController userController;
    auto userItem = std::make_shared<UserItem>(1000);
    userController.SendUserSwitchTimeout(1000, 1000, userItem);
    userController.Init();
    userController.SendUserSwitchTimeout(1000, 1000, userItem);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendReportUserSwitchTest_0100
 * @tc.desc: SendReportUserSwitch Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, SendReportUserSwitchTest_0100, TestSize.Level0)
{
    UserController userController;
    auto userItem = std::make_shared<UserItem>(1000);
    userController.SendReportUserSwitch(1000, 1000, userItem);
    userController.Init();
    userController.SendReportUserSwitch(1000, 1000, userItem);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendSystemUserCurrentTest_0100
 * @tc.desc: SendSystemUserCurrent Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, SendSystemUserCurrentTest_0100, TestSize.Level0)
{
    UserController userController;
    userController.SendSystemUserCurrent(1000, 1000);
    userController.Init();
    userController.SendSystemUserCurrent(1000, 1000);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: SendSystemUserStartTest_0100
 * @tc.desc: SendSystemUserStart Test
 * @tc.type: FUNC
 * @tc.require: issueI581SE
 */
HWTEST_F(UserControllerTest, SendSystemUserStartTest_0100, TestSize.Level0)
{
    UserController userController;
    userController.SendSystemUserStart(1000);
    userController.Init();
    userController.SendSystemUserStart(1000);
    EXPECT_TRUE(true);
}
}  // namespace AAFwk
}  // namespace OHOS
