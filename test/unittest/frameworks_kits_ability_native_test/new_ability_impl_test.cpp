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
#include "mock_ability_token.h"
#include "mock_new_ability.h"
#include "new_ability_impl.h"
#include "ohos_application.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

class NewAbilityImplTest : public testing::Test {
public:
    NewAbilityImplTest() : newAbilityImpl_(nullptr)
    {}
    ~NewAbilityImplTest()
    {
        newAbilityImpl_ = nullptr;
    }
    std::shared_ptr<NewAbilityImpl> newAbilityImpl_;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NewAbilityImplTest::SetUpTestCase(void)
{}

void NewAbilityImplTest::TearDownTestCase(void)
{}

void NewAbilityImplTest::SetUp(void)
{
    newAbilityImpl_ = std::make_shared<NewAbilityImpl>();
}

void NewAbilityImplTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_NewAbilityImpl_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Initial state
 */
/**
 * @tc.name: AaFwk_NewAbilityImpl_HandleAbilityTransaction_0100
 * @tc.desc: Handle transactions in the foreground state and isNewWant is true.
 * @tc.type: FUNC
 * @tc.require: I58047
 */
HWTEST_F(NewAbilityImplTest, AaFwk_NewAbilityImpl_HandleAbilityTransaction_0100, Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_NewAbilityImpl_HandleAbilityTransaction_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "newAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability;
    MockNewAbility *newAbility = new (std::nothrow) MockNewAbility();
    EXPECT_NE(newAbility, nullptr);
    if (newAbility != nullptr) {
        ability.reset(newAbility);
    }

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    newAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_FOREGROUND_NEW;
    state.isNewWant = true;
    newAbilityImpl_->HandleAbilityTransaction(want, state);
    EXPECT_EQ(newAbilityImpl_->GetCurrentState(), ABILITY_STATE_INITIAL);
    EXPECT_EQ(newAbility->onNewWantCalled_, true);
    EXPECT_EQ(newAbility->continueRestoreCalled_, true);

    GTEST_LOG_(INFO) << "AaFwk_NewAbilityImpl_HandleAbilityTransaction_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
