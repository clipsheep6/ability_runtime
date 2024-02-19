/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
const size_t SIZE_ONE = 1;
}  // namespace

class AbilityManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityManagerClientTest::SetUpTestCase(void)
{}

void AbilityManagerClientTest::TearDownTestCase(void)
{}

void AbilityManagerClientTest::SetUp()
{}

void AbilityManagerClientTest::TearDown()
{}

/**
 * @tc.name: AbilityManagerClient_DumpSysState_0100
 * @tc.desc: DumpSysState
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerClient_DumpSysState_0100 start");

    std::string args = "-a";
    std::vector<std::string> state;
    bool isClient = false;
    bool isUserID = true;
    auto result = AbilityManagerClient::GetInstance()->DumpSysState(args, state, isClient, isUserID, USER_ID);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_GT(state.size(), SIZE_ONE);

    HILOG_INFO("state.size() = %{public}zu", state.size());
    for (auto item : state) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("AbilityManagerClient_DumpSysState_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_ForceExitApp_0100
 * @tc.desc: ForceExitApp
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_ForceExitApp_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerClient_ForceExitApp_0100 start");
    int32_t pid = 0;
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->ForceExitApp(pid, exitReason);
    EXPECT_EQ(result, ERR_OK);
    HILOG_INFO("AbilityManagerClient_ForceExitApp_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RecordAppExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordAppExitReason_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerClient_RecordAppExitReason_0100 start");
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, GET_BUNDLE_INFO_FAILED);
    HILOG_INFO("AbilityManagerClient_RecordAppExitReason_0100 end");
}

/**
 * @tc.name: AbilityManagerClient_RecordProcessExitReason_0100
 * @tc.desc: RecordAppExitReason
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_RecordProcessExitReason_0100, TestSize.Level1)
{
    HILOG_INFO("AbilityManagerClient_RecordProcessExitReason_0100 start");
    ExitReason exitReason = { REASON_JS_ERROR, "Js Error." };
    auto result = AbilityManagerClient::GetInstance()->RecordAppExitReason(exitReason);
    EXPECT_EQ(result, GET_BUNDLE_INFO_FAILED);
    HILOG_INFO("AbilityManagerClient_RecordProcessExitReason_0100 end");
}
}  // namespace AAFwk
}  // namespace OHOS
