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

#include "user_controller.h"

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
{}

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
    EXPECT_TRUE(userController.StartUser(0, true) == -1);
    EXPECT_TRUE(userController.StartUser(-1, true) == -1);
    EXPECT_TRUE(userController.StartUser(100, true) == 0);
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
}  // namespace AAFwk
}  // namespace OHOS
