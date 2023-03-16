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
#include "iservice_registry.h"
#include "mock_ability_manager_service.h"
#include "system_ability_definition.h"
#define private public
#define protected public
#include "sys_mgr_client.h"
#undef private
#undef protected

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
int32_t REGISTER_FLAG = 0;
int32_t UN_REGISTER_FLAG = 0;

class SysMgrClient : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SysMgrClient::SetUpTestCase(void)
{}

void SysMgrClient::TearDownTestCase(void)
{}

void SysMgrClient::SetUp()
{}

void SysMgrClient::TearDown()
{}

/**
 * @tc.number: GetSystemAbility_0100
 * @tc.name: GetSystemAbility
 * @tc.desc: Get System Ability Success When abilityManager_ is null.
 */
HWTEST_F(SysMgrClient, GetSystemAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemAbility_0100 start";
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    EXPECT_NE(sysMgr, nullptr);

    EXPECT_EQ(sysMgr->abilityManager_, nullptr);

    auto ret = sysMgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetSystemAbility_0100 end";
}

/**
 * @tc.number: GetSystemAbility_0200
 * @tc.name: GetSystemAbility
 * @tc.desc: Get System Ability Success When abilityManager_ is not null.
 */
HWTEST_F(SysMgrClient, GetSystemAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSystemAbility_0200 start";
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    EXPECT_NE(sysMgr, nullptr);

    sysMgr->abilityManager_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(sysMgr->abilityManager_, nullptr);

    auto ret = sysMgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    EXPECT_NE(ret, nullptr);
    GTEST_LOG_(INFO) << "GetSystemAbility_0200 end";
}

/**
 * @tc.number: RegisterSystemAbility_0100
 * @tc.name: Register System Ability
 * @tc.desc: Register System Ability Success.
 */
HWTEST_F(SysMgrClient, RegisterSystemAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterSystemAbility_0100 start";
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    EXPECT_NE(sysMgr, nullptr);

    OHOS::sptr<OHOS::IRemoteObject> object = new AAFwk::MockAbilityManagerService();

    sysMgr->RegisterSystemAbility( OHOS::ABILITY_MGR_SERVICE_ID, object);
    GTEST_LOG_(INFO) << "RegisterSystemAbility_0100 end";
}

/**
 * @tc.number: UnregisterSystemAbility_0100
 * @tc.name: UnRegister System Ability
 * @tc.desc: Un Register System Ability Success.
 */
HWTEST_F(SysMgrClient, UnregisterSystemAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterSystemAbility_0100 start";
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    EXPECT_NE(sysMgr, nullptr);
    sysMgr->UnregisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    GTEST_LOG_(INFO) << "UnregisterSystemAbility_0100 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS