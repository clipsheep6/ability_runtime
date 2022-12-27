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
#include "mock_page_ability.h"
#include "mock_ability_lifecycle_callbacks.h"
#include "mock_ability_token.h"
#define private public
#define protected public
#include "page_ability_impl.h"
#undef protected
#undef private
#include "ohos_application.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

class PageAbilityImplTest : public testing::Test {
public:
    PageAbilityImplTest() : pageAbilityImpl_(nullptr)
    {}
    ~PageAbilityImplTest()
    {
        pageAbilityImpl_ = nullptr;
    }
    std::shared_ptr<PageAbilityImpl> pageAbilityImpl_;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PageAbilityImplTest::SetUpTestCase(void)
{}

void PageAbilityImplTest::TearDownTestCase(void)
{}

void PageAbilityImplTest::SetUp(void)
{
    pageAbilityImpl_ = std::make_shared<PageAbilityImpl>();
}

void PageAbilityImplTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoKeyDown_0100
 * @tc.name: DoKeyDown
 * @tc.desc: Verify that the event is handled by an exception
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoKeyDown_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyDown_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    auto keyEvent = MMI::KeyEvent::Create();
    pageAbilityImpl_->DoKeyDown(keyEvent);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyDown_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoKeyDown_0200
 * @tc.name: DoKeyDown
 * @tc.desc: Verify that the event is handled properly
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoKeyDown_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyDown_0200 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    auto keyEvent = MMI::KeyEvent::Create();
    pageAbilityImpl_->DoKeyDown(keyEvent);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyDown_0200 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoKeyUp_0100
 * @tc.name: DoKeyUp
 * @tc.desc: Verify that the event is handled by an exception
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoKeyUp_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyUp_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    auto keyEvent = MMI::KeyEvent::Create();
    pageAbilityImpl_->DoKeyUp(keyEvent);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyUp_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoKeyUp_0200
 * @tc.name: DoKeyUp
 * @tc.desc: Verify that the event is handled properly
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoKeyUp_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyUp_0200 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    auto keyEvent = MMI::KeyEvent::Create();
    pageAbilityImpl_->DoKeyUp(keyEvent);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyUp_0200 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoTouchEvent_0100
 * @tc.name: DoTouchEvent
 * @tc.desc: Verify that the event is handled by an exception
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoTouchEvent_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoTouchEvent_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    auto pointerEvent = MMI::PointerEvent::Create();
    pageAbilityImpl_->DoPointerEvent(pointerEvent);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoTouchEvent_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoTouchEvent_0200
 * @tc.name: DoTouchEvent
 * @tc.desc: Verify that the event is handled properly
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoTouchEvent_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoTouchEvent_0200 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    auto pointerEvent = MMI::PointerEvent::Create();
    pageAbilityImpl_->DoPointerEvent(pointerEvent);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoTouchEvent_0200 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Initial state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_INITIAL;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0100 midle";
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0100 midle2";
    EXPECT_EQ(ABILITY_STATE_INITIAL, pageAbilityImpl_->GetCurrentState());

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0200
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Inactive state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0200 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_INACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0200 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0300
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Active state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0300 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0300 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0400
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Background state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0400 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_BACKGROUND;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0400 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0500
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Active and Inactive states
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0500 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_INACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0500 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0600
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Active and Background states
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0600 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;

    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_BACKGROUND;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0600 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0700
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Active and Background states
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0700 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;

    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_BACKGROUND;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0700 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0800
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Active and Background states
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0800 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_BACKGROUND;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_INACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0800 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_0900
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the Active/Background/Inactive/Initial states
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0900 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_BACKGROUND;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_INACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    state.state = ABILITY_STATE_INITIAL;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_0900 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DispatchSaveAbilityState_0100
 * @tc.name: DispatchSaveAbilityState
 * @tc.desc: Test the normal behavior of the AbilityImpl::DisoatcgSaveAbilityState
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DispatchSaveAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DispatchSaveAbilityState_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<MockAbilityLifecycleCallbacks> callback = std::make_shared<MockAbilityLifecycleCallbacks>();
    application->RegisterAbilityLifecycleCallbacks(callback);

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);
    pageAbilityImpl_->DispatchSaveAbilityState();

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DispatchSaveAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DispatchRestoreAbilityState_0100
 * @tc.name: DispatchRestoreAbilityState
 * @tc.desc: Test the abnormal behavior of the AbilityImpl::DispatchRestoreAbilityState
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DispatchRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DispatchRestoreAbilityState_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    PacMap inState;
    pageAbilityImpl_->DispatchRestoreAbilityState(inState);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DispatchRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_SendResult_0100
 * @tc.name: SendResult
 * @tc.desc: Test the normal behavior of the AbilityImpl::SendResult
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_SendResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_SendResult_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    int requestCode = 1;
    int resultCode = 2;
    Want want;
    pageAbilityImpl_->SendResult(requestCode, resultCode, want);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_SendResult_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_NewWant_0100
 * @tc.name: NewWant
 * @tc.desc: Test the normal behavior of the AbilityImpl::NewWant
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_NewWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_NewWant_0100 start";

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    pageAbilityImpl_->NewWant(want);

    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_NewWant_0100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1000
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the ABILITY_STATE_FOREGROUND_NEW state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1000 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_FOREGROUND_NEW;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1000 midle";
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_FOREGROUND_NEW;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1000 midle2";
    EXPECT_EQ(ABILITY_STATE_FOREGROUND_NEW, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1000 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the ABILITY_STATE_BACKGROUND_NEW and ABILITY_STATE_ACTIVE state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1100 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_INACTIVE;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1100 midle";
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_BACKGROUND_NEW;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1100 midle2";
    EXPECT_EQ(ABILITY_STATE_BACKGROUND_NEW, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1100 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1200
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Handle transactions in the ABILITY_STATE_INACTIVE and ABILITY_STATE_ACTIVE state
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1200 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_INACTIVE;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1200 midle";
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1200 midle2";
    EXPECT_EQ(ABILITY_STATE_ACTIVE, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1200 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1300
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Case ABILITY_STATE_INITIAL in test AbilityTransaction is the state of ABILITY_STATE_INACTIVE
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1300 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_INITIAL;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1300 midle";
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_INACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1300 midle2";
    EXPECT_EQ(ABILITY_STATE_INACTIVE, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1300 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1400
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Case ABILITY_STATE_FOREGROUND_NEW in test AbilityTransaction is the state of ABILITY_STATE_INITIAL
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1400 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_FOREGROUND_NEW;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1400 midle";
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_INITIAL;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1400 midle2";
    EXPECT_EQ(ABILITY_STATE_INITIAL, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1400 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1500
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Case ABILITY_STATE_BACKGROUND_NEW in test AbilityTransaction is the state of ABILITY_STATE_INACTIVE
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1500 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_BACKGROUND_NEW;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1500 midle";
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1500 midle2";
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_INACTIVE;
    pageAbilityImpl_->HandleAbilityTransaction(want, state);
    EXPECT_EQ(ABILITY_STATE_INACTIVE, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1500 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_HandleAbilityTransaction_1600
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Case ABILITY_STATE_BACKGROUND_NEW in test AbilityTransaction is the state of ABILITY_STATE_INACTIVE
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_HandleAbilityTransaction_1600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1600 start";
    auto application = std::make_shared<OHOSApplication>();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "pageAbility";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);

    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);

    std::shared_ptr<Ability> ability = std::make_shared<MockPageAbility>();

    auto contextDeal = std::make_shared<ContextDeal>();
    pageAbilityImpl_->Init(application, record, ability, handler, token, contextDeal);

    Want want;
    AAFwk::LifeCycleStateInfo state;
    state.state = ABILITY_STATE_ACTIVE;
    pageAbilityImpl_->lifecycleState_ = ABILITY_STATE_BACKGROUND;
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1600 midle";
    pageAbilityImpl_->AbilityTransaction(want, state);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1600 midle2";
    EXPECT_EQ(ABILITY_STATE_BACKGROUND, pageAbilityImpl_->GetCurrentState());
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_HandleAbilityTransaction_1600 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoKeyDown_0300
 * @tc.name: DoKeyDown
 * @tc.desc: Test DoKeyDown function when ability_ is nullptr
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoKeyDown_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyDown_0300 start";
    auto keyEvent = MMI::KeyEvent::Create();
    pageAbilityImpl_->ability_ = nullptr;
    pageAbilityImpl_->DoKeyDown(keyEvent);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyDown_0300 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoKeyUp_0300
 * @tc.name: DoKeyUp
 * @tc.desc: Test DoKeyUp function when ability_ is nullptr
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoKeyUp_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyUp_0300 start";
    auto keyEvent = MMI::KeyEvent::Create();
    pageAbilityImpl_->ability_ = nullptr;
    pageAbilityImpl_->DoKeyUp(keyEvent);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoKeyUp_0300 end";
}

/**
 * @tc.number: AaFwk_PageAbilityImpl_DoTouchEvent_0300
 * @tc.name: DoTouchEvent
 * @tc.desc: Test DoPointerEvent function when ability_ is nullptr
 */
HWTEST_F(PageAbilityImplTest, AaFwk_PageAbilityImpl_DoTouchEvent_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoTouchEvent_0300 start";
    auto pointerEvent = MMI::PointerEvent::Create();
    pageAbilityImpl_->ability_ = nullptr;
    pageAbilityImpl_->DoPointerEvent(pointerEvent);
    GTEST_LOG_(INFO) << "AaFwk_PageAbilityImpl_DoTouchEvent_0300 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
