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
#define protected public
#include "ability.h"
#undef protected
#undef private
#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "abs_shared_result_set.h"
#include "app_loader.h"
#include "configuration.h"
#include "context_deal.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_wrapper.h"
#include "mock_page_ability.h"
#include "ohos_application.h"
#include "runtime.h"
#include "uri.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using OHOS::Parcel;

class AbilityBaseTest : public testing::Test {
public:
    AbilityBaseTest() : ability_(nullptr)
    {}
    ~AbilityBaseTest()
    {}
    std::shared_ptr<Ability> ability_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityBaseTest::SetUpTestCase(void)
{}

void AbilityBaseTest::TearDownTestCase(void)
{}

void AbilityBaseTest::SetUp(void)
{
    ability_ = std::make_shared<Ability>();
}

void AbilityBaseTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_Ability_Name_0100
 * @tc.name: GetAbilityName
 * @tc.desc: Verify that the return value of getabilityname is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_Name_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_Name_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);
    EXPECT_STREQ(abilityInfo->name.c_str(), ability_->GetAbilityName().c_str());

    GTEST_LOG_(INFO) << "AaFwk_Ability_Name_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetLifecycle_0100
 * @tc.name: GetLifecycle
 * @tc.desc: Verify that the return value of getlifecycle is not empty.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetLifecycle_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetLifecycle_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_NE(lifeCycle, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetLifecycle_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetState_0100
 * @tc.name: GetState
 * @tc.desc: Verify that the return value of getstate is equal to active.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnActive();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetState_0200
 * @tc.name: GetState
 * @tc.desc: Getstate exception test.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetState_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0200 start";

    ability_->OnActive();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetState_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_Dump_0100
 * @tc.name: Dump
 * @tc.desc: Test dump normal flow.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_Dump_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_Dump_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    std::string extra = "";
    ability_->Dump(extra);

    GTEST_LOG_(INFO) << "AaFwk_Ability_Dump_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnNewWant_0100
 * @tc.name: OnNewWant
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnNewWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnNewWant_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnNewWant(want);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnNewWant_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnRestoreAbilityState_0100
 * @tc.name: OnRestoreAbilityState
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnRestoreAbilityState_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    PacMap inState;
    ability_->OnRestoreAbilityState(inState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetAbilityName_0100
 * @tc.name: GetAbilityName
 * @tc.desc: Verify that the getabilityname return value is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetAbilityName_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    std::string name = "LOL";
    abilityInfo->name = name;
    ability_->Init(abilityInfo, application, handler, token);

    EXPECT_STREQ(ability_->GetAbilityName().c_str(), name.c_str());

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetAbilityName_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetApplication_0100
 * @tc.name: GetApplication
 * @tc.desc: Verify that the getapplication return value is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetApplication_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);
    std::shared_ptr<OHOSApplication> applicationRet = ability_->GetApplication();
    EXPECT_EQ(application, applicationRet);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_GetApplication_0200
 * @tc.name: GetApplication
 * @tc.desc: Test getapplication exception status.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetApplication_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, nullptr, handler, token);
    std::shared_ptr<OHOSApplication> application = ability_->GetApplication();
    EXPECT_EQ(application, nullptr);

    GTEST_LOG_(INFO) << "AaFwk_Ability_GetApplication_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnSaveAbilityState_0100
 * @tc.name: OnSaveAbilityState
 * @tc.desc: Test whether onsaveabilitystate is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnSaveAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSaveAbilityState_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;

    ability_->Init(abilityInfo, application, handler, token);

    PacMap outState;
    ability_->OnSaveAbilityState(outState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnSaveAbilityState_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_SetWant_GetWant_0100
 * @tc.name: OnSaveAbilityState
 * @tc.desc: Verify that setwant creates the object normally,
 *           and judge whether the return value of getwant is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_SetWant_GetWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetWant_GetWant_0100 start";

    std::string abilityName = "Ability";
    std::string bundleName = "Bundle";
    AAFwk::Want want;
    want.SetElementName(bundleName, abilityName);
    ability_->SetWant(want);

    EXPECT_STREQ(ability_->GetWant()->GetElement().GetBundleName().c_str(), bundleName.c_str());
    EXPECT_STREQ(ability_->GetWant()->GetElement().GetAbilityName().c_str(), abilityName.c_str());
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetWant_GetWant_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_SetResult_0100
 * @tc.name: SetResult
 * @tc.desc: Test whether setresult is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_SetResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_SetResult_0100 start";

    int resultCode = 0;
    Want want;
    std::string action = "Action";
    want.SetAction(action);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->SetResult(resultCode, want);

    GTEST_LOG_(INFO) << "AaFwk_Ability_SetResult_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbilityForResult_0100
 * @tc.name: StartAbilityForResult
 * @tc.desc: Test whether startabilityforesult is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbilityForResult_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilityForResult_0100 start";

    int resultCode = 0;
    Want want;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->StartAbilityForResult(want, resultCode);

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilityForResult_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Test whether startability is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbility_0100 start";

    Want want;
    ability_->StartAbility(want);

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbility_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_TerminateAbility_0100
 * @tc.name: TerminateAbility
 * @tc.desc: Test whether terminateability is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_TerminateAbility_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_TerminateAbility_0100 start";

    ability_->TerminateAbility();

    GTEST_LOG_(INFO) << "AaFwk_Ability_TerminateAbility_0100 end";
}

HWTEST_F(AbilityBaseTest, AaFwk_Ability_GetWindow_001, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
}

/**
 * @tc.number: AaFwk_Ability_OnStart_0100
 * @tc.name: OnStart
 * @tc.desc: Test whether OnStart is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStart_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnStart(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnStart_0200
 * @tc.name: OnStart
 * @tc.desc: Test the OnStart exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStart_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0200 start";

    Want want;
    ability_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStart_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: Test whether onstop is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnStop();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnStop_0200
 * @tc.name: OnStop
 * @tc.desc: Test the OnStop exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnStop_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0200 start";

    ability_->OnStop();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnStop_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnActive_0100
 * @tc.name: OnActive
 * @tc.desc: Test whether onactive is called normally and verify whether the member is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnActive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);
    EXPECT_EQ(LifeCycle::Event::ON_ACTIVE, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnActive_0200
 * @tc.name: OnActive
 * @tc.desc: Test the OnActive exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnActive_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0200 start";

    ability_->OnActive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnActive_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnInactive_0100
 * @tc.name: OnInactive
 * @tc.desc: Test whether oninactive is called normally and verify whether the member is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnInactive_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnInactive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INACTIVE, state);
    EXPECT_EQ(LifeCycle::Event::ON_INACTIVE, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnInactive_0200
 * @tc.name: OnInactive
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnInactive_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0200 start";

    ability_->OnInactive();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnInactive_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnForeground_0100
 * @tc.name: OnForeground
 * @tc.desc: Test whether onforegroup is called normally, and verify whether the member is correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnForeground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INACTIVE, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnForeground_0200
 * @tc.name: OnForeground
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnForeground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0200 start";

    Want want;
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnForeground_0300
 * @tc.name: OnForeground
 * @tc.desc: Test the OnForeground exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnForeground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    Want want;
    ability_->OnForeground(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnForeground_0300 end";
}


/**
 * @tc.number: AaFwk_Ability_OnBackground_0100
 * @tc.name: OnBackground
 * @tc.desc: Test whether onbackground is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    ability_->OnBackground();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND, state);
    EXPECT_EQ(LifeCycle::Event::ON_BACKGROUND, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnBackground_0200
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->OnBackground();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND, state);
    EXPECT_TRUE(lifeCycle);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_OnBackground_0300
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackground_0300 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);
    ability_->OnBackground();

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();

    // Sence is nullptr, so lifecycle schedule failed.
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::UNDEFINED, lifeCycleState);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OBackground_0300 end";
}

/**
 * @tc.number: AaFwk_Ability_OnConnect_0100
 * @tc.name: OnConnect
 * @tc.desc: Test whether onconnect is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnConnect_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnConnect_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnConnect(want);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnConnect_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnCommond_0100
 * @tc.name: OnCommand
 * @tc.desc: Test whether oncommand is called normally and verify whether the members are correct.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnCommond_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnCommond_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    ability_->Init(abilityInfo, application, handler, token);

    Want want;
    ability_->OnCommand(want, false, 0);

    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();

    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::ACTIVE, state);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnCommond_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_OnDisconnect_0100
 * @tc.name: OnDisconnect
 * @tc.desc: Test whether ondisconnect is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnDisconnect_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnDisconnect_0100 start";

    Want want;
    ability_->OnDisconnect(want);

    GTEST_LOG_(INFO) << "AaFwk_Ability_OnDisconnect_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbilitySetting_0100
 * @tc.name: StartAbility
 * @tc.desc: Test whether startability is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbilitySetting_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0100 start";

    Want want;
    std::shared_ptr<AbilityStartSetting> setting = AbilityStartSetting::GetEmptySetting();

    ability_->StartAbility(want, *setting.get());

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_StartAbilitySetting_0200
 * @tc.name: StartAbility
 * @tc.desc: Test startability exception status.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_StartAbilitySetting_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0200 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    Want want;
    std::shared_ptr<AbilityStartSetting> setting = AbilityStartSetting::GetEmptySetting();
    ability_->StartAbility(want, *setting.get());

    GTEST_LOG_(INFO) << "AaFwk_Ability_StartAbilitySetting_0200 end";
}

/**
 * @tc.number: AaFwk_Ability_PostTask_0100
 * @tc.name: PostTask
 * @tc.desc: Test whether posttask is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_PostTask_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_PostTask_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);

    ability_->Init(abilityInfo, nullptr, handler, nullptr);
    auto task = []() { GTEST_LOG_(INFO) << "AaFwk_Ability_PostTask_001 task called"; };
    ability_->PostTask(task, 1000);

    GTEST_LOG_(INFO) << "AaFwk_Ability_PostTask_0100 end";
}

/**
 * @tc.number: AaFwk_Ability_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Test whether ExecuteBatch is called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_ExecuteBatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_ExecuteBatch_0100 start";

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::DATA;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    abilityInfo->isNativeAbility = true;

    ability_->Init(abilityInfo, nullptr, handler, nullptr);

    OHOS::NativeRdb::ValuesBucket calllogValues;
    calllogValues.PutString("phone_number", "12345");
    OHOS::NativeRdb::DataAbilityPredicates predicates;
    predicates.GreaterThan("id", "0");
    std::shared_ptr<OHOS::NativeRdb::ValuesBucket> values =
        std::make_shared<OHOS::NativeRdb::ValuesBucket>(calllogValues);
    std::shared_ptr<OHOS::NativeRdb::DataAbilityPredicates> executePredicates =
        std::make_shared<OHOS::NativeRdb::DataAbilityPredicates>(predicates);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("dataability:///com.ohos.test");
    std::shared_ptr<DataAbilityOperation> operation =
        DataAbilityOperation::NewUpdateBuilder(uri)
        ->WithValuesBucket(values)
        ->WithPredicatesBackReference(0, 0)
        ->WithPredicates(executePredicates)
        ->WithInterruptionAllowed(true)
        ->Build();
    std::vector<std::shared_ptr<DataAbilityOperation>> executeBatchOperations;
    executeBatchOperations.push_back(operation);

    std::vector<std::shared_ptr<DataAbilityResult>> ret = ability_->ExecuteBatch(executeBatchOperations);

    EXPECT_STREQ(ret.at(0)->GetUri().ToString().c_str(), uri->ToString().c_str());

    GTEST_LOG_(INFO) << "AaFwk_Ability_ExecuteBatch_0100 end";
}

class AbilityTest final : public Ability {
public:
    AbilityTest() {}
    virtual ~AbilityTest() {}

    void OnBackPressed() override
    {
        Ability::OnBackPressed();
        onBackPressed_ = true;
    }

public:
    bool onBackPressed_ = false;
};
/**
 * @tc.number: AaFwk_Ability_OnBackPressed_0100
 * @tc.name: OnBackPress
 * @tc.desc: Test whether OnBackPress can be called normally.
 */
HWTEST_F(AbilityBaseTest, AaFwk_Ability_OnBackPressed_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackPressed_0100 start";
    std::shared_ptr<AbilityTest> ability = std::make_shared<AbilityTest>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(abilityInfo, nullptr, handler, nullptr);
    ability->OnBackPressed();
    EXPECT_TRUE(ability->onBackPressed_);
    GTEST_LOG_(INFO) << "AaFwk_Ability_OnBackPressed_0100 end";
}

/**
 * @tc.name: AbilityCreate_0100
 * @tc.desc: Ability create test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCreate_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = Ability::Create(runtime);
    EXPECT_NE(ability, nullptr);

    AbilityRuntime::Runtime::Options anotherOptions;
    anotherOptions.lang = static_cast<AbilityRuntime::Runtime::Language>(100); // invalid Runtime::Language
    auto anotherRuntime = AbilityRuntime::Runtime::Create(anotherOptions);
    auto anotherAbility = Ability::Create(anotherRuntime);
    EXPECT_NE(anotherAbility, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityIsUpdatingConfigurations_0100
 * @tc.desc: Ability IsUpdatingConfigurations test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityIsUpdatingConfigurations_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto ret = ability_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, false);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnStop_0100
 * @tc.desc: Ability onStop test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnStop_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    bool isAsyncCallback = true;
    ability_->OnStop(nullptr, isAsyncCallback);
    ability_->OnStopCallback();
    EXPECT_EQ(isAsyncCallback, false);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0100
 * @tc.desc: Ability Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // branch when abilityContext_ is nullptr
    auto ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, false);

    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability->AttachAbilityContext(abilityContext);
    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = LaunchReason::LAUNCHREASON_START_ABILITY;
    ability->SetLaunchParam(launchParam);

    // branch when launchReason is not LAUNCHREASON_CONTINUATION
    ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, false);

    launchParam.launchReason = LaunchReason::LAUNCHREASON_CONTINUATION;
    ability->SetLaunchParam(launchParam);

    // branch when contentStorage_ is nullptr
    ret = ability->IsRestoredInContinuation();
    EXPECT_EQ(ret, false);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0200
 * @tc.desc: Ability ShouldRecoverState test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // branch when abilityRecovery_ is nullptr
    Want want;
    bool ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);
    ability->HandleCreateAsRecovery(want);

    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery);

    // branch when abilityContext_ is nullptr
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    ability->HandleCreateAsRecovery(want);
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    auto abilityContext = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    ability->AttachAbilityContext(abilityContext);

    // branch when contentStorage_ is nullpt
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    // branch when want parameter didn't set
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    // NativeEngine is hard to construct
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    ret = ability->ShouldRecoverState(want);
    EXPECT_EQ(ret, false);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0300
 * @tc.desc: Ability NotifyContinuationResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    Want want;
    bool success = false;
    ability->NotifyContinuationResult(want, success);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0400
 * @tc.desc: Ability Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    auto state = ability->GetContinuationState();
    EXPECT_EQ(state, ContinuationState::LOCAL_RUNNING);

    std::string deviceId = ability->GetOriginalDeviceId();
    ability->ContinueAbilityReversibly(deviceId);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    (void)ability->GetContinuationState();
    ability->ContinueAbilityReversibly(deviceId);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0500
 * @tc.desc: Ability Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0500, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // branch when abilityInfo_ is nullptr
    auto regMgr = ability->GetContinuationRegisterManager();
    EXPECT_EQ(regMgr.lock(), nullptr);

    // branch when type is not page
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    auto eventRunner = EventRunner::Create(serviceAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    regMgr = ability->GetContinuationRegisterManager();
    EXPECT_EQ(regMgr.lock(), nullptr);

    // branch when deviceId is emptry
    std::string deviceId;
    uint32_t versionCode = 1;
    ability->ContinueAbility(deviceId);
    ability->ContinueAbilityWithStack(deviceId, versionCode);

    // branch when continuationRegisterManager_ is nullptr
    ability->ContinueAbility("deviceId");
    ability->ContinueAbilityWithStack("deviceId", versionCode);

    // branch when abilityInfo_ is nullptr and continuation register manager need init
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    regMgr = ability->GetContinuationRegisterManager();
    EXPECT_NE(regMgr.lock(), nullptr);
    ability->ContinueAbility("deviceId");
    ability->ContinueAbilityWithStack("deviceId", versionCode);

    WantParams wantParams;
    auto state = ability->OnContinue(wantParams);
    EXPECT_EQ(state, ContinuationManager::OnContinueResult::REJECT);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityContinuation_0600
 * @tc.desc: Ability HandleCreateAsContinuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityContinuation_0600, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Want want;
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    // branch when continuation manager is nullptr
    ability->HandleCreateAsContinuation(want);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ability->HandleCreateAsContinuation(want);

    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE);
    ability->HandleCreateAsContinuation(want);

    ability->ContinuationRestore(want);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityStartAbilityForResult_0100
 * @tc.desc: Ability StartAbilityForResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityStartAbilityForResult_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    Want want;
    std::string bundleName = "BundleName";
    std::string abilityName = "abilityName";
    want.SetElementName(bundleName, abilityName);
    int requestCode = 0;
    AbilityStartSetting abilityStartSetting;

    // branch when abilityInfo_ is nullptr
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    ability->StartAbilityForResult(want, requestCode, abilityStartSetting);

    // branch when type is not PAGE
    std::shared_ptr<AbilityHandler> handler = nullptr;
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    ability->StartAbilityForResult(want, requestCode, abilityStartSetting);

    // branch when type is PAGE
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ability->StartAbilityForResult(want, requestCode, abilityStartSetting);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityGetType_0100
 * @tc.desc: Ability GetType test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityGetType_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Uri uri("test_get_type");
    auto type = ability->GetType(uri);
    EXPECT_EQ(type, "");

    std::string mimeTypeFilter;
    auto types = ability->GetFileTypes(uri, mimeTypeFilter);
    auto size = static_cast<int>(types.size());
    EXPECT_EQ(size, 0);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityInsert_0100
 * @tc.desc: Ability Insert test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityInsert_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Uri uri("test_insert");
    NativeRdb::ValuesBucket value;
    auto ret = ability->Insert(uri, value);
    EXPECT_EQ(ret, 0);

    std::vector<NativeRdb::ValuesBucket> values;
    values.push_back(value);
    auto cnt = ability->BatchInsert(uri, values);
    EXPECT_EQ(cnt, 1);

    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates;
    auto resultSet = ability->Query(uri, columns, predicates);
    EXPECT_EQ(resultSet, nullptr);

    ret = ability->Delete(uri, predicates);
    EXPECT_EQ(ret, 0);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCall_0100
 * @tc.desc: Ability Call test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCall_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Uri uri("test_call");
    std::string method;
    std::string arg;
    AppExecFwk::PacMap pacMap;
    auto ret = ability->Call(uri, method, arg, pacMap);
    EXPECT_EQ(ret, nullptr);

    auto reloadRet = ability->Reload(uri, pacMap);
    EXPECT_EQ(reloadRet, false);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnMemoryLevel_0100
 * @tc.desc: Ability OnMemoryLevel test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnMemoryLevel_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    int level = 0;
    ability->OnMemoryLevel(level);

    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ability->OnMemoryLevel(level);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOpenRawFile_0100
 * @tc.desc: Ability OpenRawFile test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOpenRawFile_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Uri uri("test_open_file");
    std::string mode;
    auto ret = ability->OpenRawFile(uri, mode);
    EXPECT_EQ(ret, -1);

    ret = ability->OpenFile(uri, mode);
    EXPECT_EQ(ret, -1);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityVirtualFunc_0100
 * @tc.desc: Ability virtual function test, such as OnAbilityResult, IsTerminating and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityVirtualFunc_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Configuration configuration;
    ability->OnConfigurationUpdated(configuration);

    auto ret = ability->IsTerminating();
    EXPECT_EQ(ret, false);

    int requestCode = 0;
    int resultCode = 0;
    Want want;
    ability->OnAbilityResult(requestCode, resultCode, want);

    ability->OnEventDispatch();

    std::vector<std::string> params;
    std::vector<std::string> info;
    ability->Dump(params, info);

    Uri originUri("test_virtual_function");
    auto destUri = ability->NormalizeUri(originUri);
    (void)ability->DenormalizeUri(destUri);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityVirtualFunc_0200
 * @tc.desc: Ability virtual function test, such as OnStartContinuation, OnSaveData and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityVirtualFunc_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    bool ret = ability->OnStartContinuation();
    EXPECT_EQ(ret, false);

    WantParams data;
    ret = ability->OnSaveData(data);
    EXPECT_EQ(ret, false);

    ret = ability->OnRestoreData(data);
    EXPECT_EQ(ret, false);

    int32_t reason = 0;
    EXPECT_EQ(ability->OnSaveState(reason, data), 0);

    int result = 0;
    ability->OnCompleteContinuation(result);
    ability->OnRemoteTerminated();

    (void)ability->OnSetCaller();

    std::string taskstr;
    auto timeout = ability->CreatePostEventTimeouter(taskstr);
    EXPECT_NE(timeout, nullptr);

    AAFwk::LaunchParam launchParam;
    ability->SetLaunchParam(launchParam);
    (void)ability->GetLaunchParam();

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityBackgroundRunning_0100
 * @tc.desc: Ability function test, including StopBackgroundRunning and StartBackgroundRunning
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityBackgroundRunning_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    auto bundleMgr = ability->GetBundleMgr();
    ability->SetBundleManager(bundleMgr);

    AbilityRuntime::WantAgent::WantAgent wantAgent;
    ability->StartBackgroundRunning(wantAgent);

    int id = 0;
    NotificationRequest notificationRequest;
    ability->KeepBackgroundRunning(id, notificationRequest);

    ability->CancelBackgroundRunning();

    ability->StopBackgroundRunning();

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityParseValuesBucketReference_0100
 * @tc.desc: Ability ParseValuesBucketReference test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityParseValuesBucketReference_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    std::vector<std::shared_ptr<DataAbilityResult>> results;
    std::shared_ptr<DataAbilityOperation> operation = nullptr;
    int numRefs = 0;
    auto reference = ability->ParseValuesBucketReference(results, operation, numRefs);
    EXPECT_EQ(reference, nullptr);

    operation = std::make_shared<DataAbilityOperation>();
    reference = ability->ParseValuesBucketReference(results, operation, numRefs);
    EXPECT_NE(reference, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityChangeRef2Value_0100
 * @tc.desc: Ability ChangeRef2Value test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityChangeRef2Value_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // index larger than or equal to numRefs
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    int numRefs = 0;
    int index = 0;
    auto value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    // index larger than or equal to results size
    numRefs = 1;
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    // result is nullptr
    auto resultIndex0 = std::make_shared<DataAbilityResult>(1);
    results.push_back(resultIndex0);
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, 1);

    Uri uri("test_change");
    auto resultIndex1 = std::make_shared<DataAbilityResult>(uri, 1);
    results.push_back(resultIndex1);
    index = 1;
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    results.push_back(nullptr);
    index = 2;
    value = ability->ChangeRef2Value(results, numRefs, index);
    EXPECT_EQ(value, -1);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityCheckAssertQueryResult_0100
 * @tc.desc: Ability CheckAssertQueryResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityCheckAssertQueryResult_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // queryResult is nullptr
    std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult = nullptr;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    bool ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    // valuesBucket is nullptr
    queryResult = std::make_shared<NativeRdb::AbsSharedResultSet>();
    ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    // valuesBucket is empty
    ret = ability->CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
    EXPECT_EQ(ret, true);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityStartFeatureAbilityForResult_0100
 * @tc.desc: Ability StartFeatureAbilityForResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityStartFeatureAbilityForResult_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    Want want;
    int requestCode = 0;
    FeatureAbilityTask task = [](int resultCode, const AAFwk::Want& want) {
        HILOG_INFO("async callback is called");
    };
    auto ret = ability->StartFeatureAbilityForResult(want, requestCode, std::move(task));
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFuncList_0100
 * @tc.desc: Ability function test, including CallRequest, IsUseNewStartUpRule and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityFuncList_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    auto object = ability->CallRequest();
    EXPECT_EQ(object, nullptr);

    AAFwk::Want want;
    want.SetParam("component.startup.newRules", true);
    ability->SetWant(want);
    bool isNewRule = ability->IsUseNewStartUpRule();
    EXPECT_EQ(isNewRule, true);

    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery);

    bool isCompleted = ability->PrintDrawnCompleted();
    EXPECT_EQ(isCompleted, false);

    auto ret = ability->GetVolumeTypeAdjustedByKey();
    EXPECT_EQ(ret, 0);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFuncList_0200
 * @tc.desc: Ability function test, including SetVolumeTypeAdjustedByKey, OnKeyPressAndHold and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityFuncList_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    ability->OnLeaveForeground();

    int volumeType = 0;
    ability->SetVolumeTypeAdjustedByKey(volumeType);

    int red = 0;
    int green = 0;
    int blue = 0;
    EXPECT_EQ(ability->SetWindowBackgroundColor(red, green, blue), -1);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetShowOnLockScreen_0100
 * @tc.desc: Ability SetShowOnLockScreen test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetShowOnLockScreen_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    ability->SetShowOnLockScreen(true);
    ability->SetShowOnLockScreen(false);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);

    ability->SetShowOnLockScreen(true);
    ability->SetShowOnLockScreen(false);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityScene_0100
 * @tc.desc: Ability Scene test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityScene_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    ability->OnSceneCreated();
    ability->OnSceneRestored();
    ability->onSceneDestroyed();

    auto scene = ability->GetScene();
    EXPECT_EQ(scene, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetUIContent_0100
 * @tc.desc: Ability SetUIContent test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetUIContent_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    ComponentContainer componentContainer;
    ability->SetUIContent(componentContainer);

    int layoutRes = 0;
    ability->SetUIContent(layoutRes);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFormFunction_0100
 * @tc.desc: Ability GetFormRemoteObject test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityFormFunction_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    int64_t formId = 0;
    ability->OnUpdate(formId);

    Want want;
    auto ret = ability->OnAcquireFormState(want);
    EXPECT_EQ(ret, FormState::DEFAULT);

    auto object = ability->GetFormRemoteObject();
    EXPECT_NE(object, nullptr);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityGetCurrentWindowMode_0100
 * @tc.desc: Ability GetCurrentWindowMode test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityGetCurrentWindowMode_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // scene_ is nullptr
    int windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));

    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetMissionLabel_0100
 * @tc.desc: Ability SetMissionLabel test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetMissionLabel_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    std::string label = "test_label";
    auto ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    // ability type is not page
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    auto eventRunner = EventRunner::Create(serviceAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    // stage mode
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    eventRunner = EventRunner::Create(pageAbilityInfo->name);
    handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    // fa mode
    pageAbilityInfo->isStageBasedModel = false;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetMissionIcon_0100
 * @tc.desc: Ability SetMissionIcon test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetMissionIcon_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    auto icon = std::make_shared<Media::PixelMap>();
    auto ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    // ability type is not page
    std::shared_ptr<AbilityInfo> serviceAbilityInfo = std::make_shared<AbilityInfo>();
    serviceAbilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    auto eventRunner = EventRunner::Create(serviceAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(serviceAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    // stage mode
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    eventRunner = EventRunner::Create(pageAbilityInfo->name);
    handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    // fa mode
    pageAbilityInfo->isStageBasedModel = false;
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnChange_0100
 * @tc.desc: Ability OnChange test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnChange_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // application is nullptr
    Rosen::DisplayId displayId = 0;
    ability->OnCreate(displayId);
    ability->OnDestroy(displayId);
    ability->OnChange(displayId);

    auto application = std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName());
    ability->Init(pageAbilityInfo, application, handler, nullptr);
    ability->OnChange(displayId);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityOnDisplayMove_0100
 * @tc.desc: Ability OnDisplayMove test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityOnDisplayMove_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // application is nullptr
    Rosen::DisplayId fromDisplayId = 1;
    Rosen::DisplayId toDisplayId = 0;
    ability->OnDisplayMove(fromDisplayId, toDisplayId);

    auto application = std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName());
    ability->Init(pageAbilityInfo, application, handler, nullptr);
    ability->OnDisplayMove(fromDisplayId, toDisplayId);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityRequestFocus_0100
 * @tc.desc: Ability RequestFocus test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilityRequestFocus_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // ability window is nullptr
    Want want;
    ability->RequestFocus(want);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // window is nullptr
    ability->RequestFocus(want);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->RequestFocus(want);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetWakeUpScreen_0100
 * @tc.desc: Ability SetWakeUpScreen test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetWakeUpScreen_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // ability window is nullptr
    bool wakeUp = false;
    ability->SetWakeUpScreen(wakeUp);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // window is nullptr
    ability->SetWakeUpScreen(wakeUp);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->SetWakeUpScreen(wakeUp);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilitySetDisplayOrientation_0100
 * @tc.desc: Ability SetDisplayOrientation test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(AbilityBaseTest, AbilitySetDisplayOrientation_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();

    // ability window is nullptr
    int orientation = static_cast<int>(DisplayOrientation::FOLLOWRECENT);
    ability->SetDisplayOrientation(orientation);
    int ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, 0);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    ability->Init(pageAbilityInfo, nullptr, handler, nullptr);

    // window is nullptr
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, 0);

    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, 0);

    orientation = static_cast<int>(DisplayOrientation::LANDSCAPE);
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, static_cast<int>(DisplayOrientation::LANDSCAPE));

    orientation = static_cast<int>(DisplayOrientation::PORTRAIT);
    ability->SetDisplayOrientation(orientation);
    ret = ability->GetDisplayOrientation();
    EXPECT_EQ(ret, static_cast<int>(DisplayOrientation::PORTRAIT));

    HILOG_INFO("%{public}s end.", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
