/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#define private public
#include "ability_debug_deal.h"
#include "ability_record.h"
#undef private
#include "want.h"

namespace OHOS {
namespace AAFwk {
using namespace testing;
using namespace testing::ext;
namespace {
    const std::string STRING_PROCESS_NAME = "process_name";
}
class AbilityDebugDealTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AbilityDebugDeal> deal_;
    std::vector<sptr<IRemoteObject>> tokens_;
};

void AbilityDebugDealTest::SetUpTestCase(void)
{}

void AbilityDebugDealTest::TearDownTestCase(void)
{}

void AbilityDebugDealTest::SetUp()
{
    deal_ = std::make_shared<AbilityDebugDeal>();
}

void AbilityDebugDealTest::TearDown()
{}

/**
 * @tc.name: AbilityDebugDealTest_RegisterAbilityDebugResponse_0100
 * @tc.desc: Verify register ability debug response calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, RegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityDebugDealTest_RegisterAbilityDebugResponse_0100 start";
    EXPECT_EQ(deal_->abilityDebugResponse_, nullptr);
    deal_->RegisterAbilityDebugResponse();
    EXPECT_NE(deal_->abilityDebugResponse_, nullptr);
    GTEST_LOG_(INFO) << "AbilityDebugDealTest_RegisterAbilityDebugResponse_0100 end";
}

/**
 * @tc.name: OnAbilitysDebugStarted_0100
 * @tc.desc: Verify that OnAbilitysDebugStoped set isAttachDebug_ of AbilityRecord to true correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysDebugStarted_0100, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAttachDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysDebugStarted(tokens_);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAttachDebug_);
}

/**
 * @tc.name: OnAbilitysDebugStoped_0100
 * @tc.desc: Verify that OnAbilitysDebugStoped set isAttachDebug_ of AbilityRecord to false correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, OnAbilitysDebugStoped_0100, TestSize.Level1)
{
    EXPECT_NE(deal_, nullptr);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = STRING_PROCESS_NAME;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto record = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    EXPECT_NE(record, nullptr);
    EXPECT_FALSE(record->isAttachDebug_);
    const sptr<IRemoteObject> token = new Token(record);
    tokens_.emplace_back(token);

    deal_->OnAbilitysDebugStarted(tokens_);
    auto ability_record = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(ability_record->isAttachDebug_);
    deal_->OnAbilitysDebugStoped(tokens_);
    EXPECT_FALSE(ability_record->isAttachDebug_);
}
}  // namespace AppExecFwk
}  // namespace OHOS