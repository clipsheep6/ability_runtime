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

#define private public
#include "ability_manager_client.h"
#undef private
#include "ability_manager_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int USER_ID = 100;
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
HWTEST_F(AbilityManagerClientTest, AbilityManagerClient_DumpSysState_0100, TestSize.Level0)
{
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    EXPECT_NE(abilityManagerService, nullptr);

    auto abilityManagerClient = std::make_shared<AbilityManagerClient>();
    EXPECT_NE(abilityManagerClient, nullptr);
    abilityManagerClient->remoteObject_ = sptr<IRemoteObject>(abilityManagerService.get());

    std::string args = "-a";
    std::vector<std::string> state;
    bool isClient = false;
    bool isUserID = true;
    auto result = abilityManagerClient->DumpSysState(args, state, isClient, isUserID, USER_ID);
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
