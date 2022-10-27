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


#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_record.h"
#undef private
#undef protected

#include <gtest/gtest.h>
#include "ability_manager_errors.h"
#include "distributed_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
    const int32_t API_VERSION = 9;
    const int32_t USER_ID_U100 = 100;
}
class AbilityManagerServiceDistributedTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
public:
    inline static std::shared_ptr<AbilityManagerService> abilityMs_ {nullptr};
    inline static std::shared_ptr<AbilityRecord> abilityRecord_ {nullptr};
};

void AbilityManagerServiceDistributedTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceDistributedTest SetUpTestCase called";
    abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
    abilityMs_->OnStart();
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.apiTargetVersion = API_VERSION;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
}
void AbilityManagerServiceDistributedTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "AbilityManagerServiceDistributedTest TearDownTestCase called";
    abilityMs_->OnStop();
}
void AbilityManagerServiceDistributedTest::SetUp()
{}
void AbilityManagerServiceDistributedTest::TearDown()
{}

/**
 * @tc.name: StartRemoteAbility_0001
 * @tc.desc: StartRemoteAbility Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, StartRemoteAbility_0001, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->StartRemoteAbility(want, 0, USER_ID_U100, nullptr);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: StartRemoteAbility_0002
 * @tc.desc: StartRemoteAbility Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, StartRemoteAbility_0002, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->StartRemoteAbility(want, 0, USER_ID_U100, abilityRecord_->GetToken());
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: StartRemoteAbility_0003
 * @tc.desc: StartRemoteAbility Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, StartRemoteAbility_0003, TestSize.Level3)
{
    Want want;
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    int result = abilityMs_->StartRemoteAbility(want, 0, USER_ID_U100, abilityRecord_->GetToken());
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: StartRemoteAbility_0004
 * @tc.desc: StartRemoteAbility Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, StartRemoteAbility_0004, TestSize.Level3)
{
    Want want;
    want.AddFlags(want.FLAG_ABILITY_CONTINUATION);
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    int result = abilityMs_->StartRemoteAbility(want, 0, USER_ID_U100, abilityRecord_->GetToken());
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ConnectRemoteAbility_0001
 * @tc.desc: ConnectRemoteAbility Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, ConnectRemoteAbility_0001, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->ConnectRemoteAbility(want, nullptr, nullptr);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: ConnectRemoteAbility_0002
 * @tc.desc: ConnectRemoteAbility Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, ConnectRemoteAbility_0002, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->ConnectRemoteAbility(want, abilityRecord_->GetToken(), nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: StartRemoteAbilityByCall_0001
 * @tc.desc: StartRemoteAbilityByCall Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, StartRemoteAbilityByCall_0001, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->StartRemoteAbilityByCall(want, nullptr, nullptr);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: StartRemoteAbilityByCall_0002
 * @tc.desc: StartRemoteAbilityByCall Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, StartRemoteAbilityByCall_0002, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->StartRemoteAbilityByCall(want, abilityRecord_->GetToken(), nullptr);
    EXPECT_TRUE(result != ERR_OK);
}

/**
 * @tc.name: AddStartControlParam_0001
 * @tc.desc: AddStartControlParam Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, AddStartControlParam_0001, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->AddStartControlParam(want, nullptr);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AddStartControlParam_0002
 * @tc.desc: AddStartControlParam Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, AddStartControlParam_0002, TestSize.Level3)
{
    Want want;
    int result = abilityMs_->AddStartControlParam(want, abilityRecord_->GetToken());
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AddStartControlParam_0003
 * @tc.desc: AddStartControlParam Test
 * @tc.type: FUNC
 * @tc.require: issueI5T6HF
 */
HWTEST_F(AbilityManagerServiceDistributedTest, AddStartControlParam_0003, TestSize.Level3)
{
    Want want;
    abilityMs_->backgroundJudgeFlag_ = false;
    int result = abilityMs_->AddStartControlParam(want, abilityRecord_->GetToken());
    EXPECT_EQ(result, ERR_OK);
}
}  // namespace AAFwk
}  // namespace OHOS
