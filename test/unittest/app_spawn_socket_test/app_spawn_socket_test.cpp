/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "app_spawn_socket.h"
#undef private

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppSpawn;

namespace OHOS {
namespace AppExecFwk {
class AppSpawnSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AppSpawnSocketTest::SetUpTestCase()
{}

void AppSpawnSocketTest::TearDownTestCase()
{}

void AppSpawnSocketTest::SetUp()
{}

void AppSpawnSocketTest::TearDown()
{}

/*
 * Feature: AppSpawnSocket
 * Function: OpenAppSpawnConnection
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket OpenAppSpawnConnection
 * EnvConditions: NA
 * CaseDescription: Verify OpenAppSpawnConnection
 */
HWTEST_F(AppSpawnSocketTest, OpenAppSpawnConnection_001, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    ErrCode res1 = appSpawnSocket->OpenAppSpawnConnection();
    EXPECT_EQ(res1, ERR_OK);
    appSpawnSocket->clientSocket_ = nullptr;
    ErrCode res2 = appSpawnSocket->OpenAppSpawnConnection();
    EXPECT_EQ(res2, ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET);
}

/*
 * Feature: AppSpawnSocket
 * Function: CloseAppSpawnConnection
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket CloseAppSpawnConnection
 * EnvConditions: NA
 * CaseDescription: Verify CloseAppSpawnConnection
 */
HWTEST_F(AppSpawnSocketTest, CloseAppSpawnConnection_001, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    appSpawnSocket->CloseAppSpawnConnection();
    appSpawnSocket->clientSocket_ = nullptr;
    appSpawnSocket->CloseAppSpawnConnection();
}

/*
 * Feature: AppSpawnSocket
 * Function: WriteMessage
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket WriteMessage
 * EnvConditions: NA
 * CaseDescription: Verify WriteMessage
 */
HWTEST_F(AppSpawnSocketTest, WriteMessage_001, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    const void *buf = nullptr;
    int32_t len = 0;
    ErrCode res = appSpawnSocket->WriteMessage(buf, len);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppSpawnSocket
 * Function: WriteMessage
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket WriteMessage
 * EnvConditions: NA
 * CaseDescription: Verify WriteMessage
 */
HWTEST_F(AppSpawnSocketTest, WriteMessage_002, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    const void *buf = nullptr;
    int32_t len = 1;
    ErrCode res = appSpawnSocket->WriteMessage(buf, len);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppSpawnSocket
 * Function: WriteMessage
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket WriteMessage
 * EnvConditions: NA
 * CaseDescription: Verify WriteMessage
 */
HWTEST_F(AppSpawnSocketTest, WriteMessage_003, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    char data = 'a';
    const void *buf = &data;
    int32_t len = 1;
    ErrCode res1 = appSpawnSocket->WriteMessage(buf, len);
    EXPECT_EQ(res1, ERR_APPEXECFWK_SOCKET_WRITE_FAILED);
    appSpawnSocket->clientSocket_ = nullptr;
    ErrCode res2 = appSpawnSocket->WriteMessage(buf, len);
    EXPECT_EQ(res2, ERR_APPEXECFWK_BAD_APPSPAWN_SOCKET);
}

/*
 * Feature: AppSpawnSocket
 * Function: ReadMessage
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket ReadMessage
 * EnvConditions: NA
 * CaseDescription: Verify ReadMessage
 */
HWTEST_F(AppSpawnSocketTest, ReadMessage_001, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    void *buf = nullptr;
    int32_t len = 0;
    ErrCode res = appSpawnSocket->ReadMessage(buf, len);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppSpawnSocket
 * Function: ReadMessage
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket ReadMessage
 * EnvConditions: NA
 * CaseDescription: Verify ReadMessage
 */
HWTEST_F(AppSpawnSocketTest, ReadMessage_002, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    void *buf = nullptr;
    int32_t len = 1;
    ErrCode res = appSpawnSocket->ReadMessage(buf, len);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppSpawnSocket
 * Function: ReadMessage
 * SubFunction: NA
 * FunctionPoints: AppSpawnSocket ReadMessage
 * EnvConditions: NA
 * CaseDescription: Verify ReadMessage
 */
HWTEST_F(AppSpawnSocketTest, ReadMessage_003, TestSize.Level0)
{
    auto appSpawnSocket = std::make_shared<AppSpawnSocket>(true);
    char data = 'a';
    void *buf = &data;
    int32_t len = 1;
    ErrCode res1 = appSpawnSocket->ReadMessage(buf, len);
    EXPECT_EQ(res1, ERR_APPEXECFWK_SOCKET_READ_FAILED);
    appSpawnSocket->clientSocket_ = nullptr;
    ErrCode res2 = appSpawnSocket->ReadMessage(buf, len);
    EXPECT_EQ(res2, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);
}
}  // namespace AppExecFwk
}  // namespace OHOS
