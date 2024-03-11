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

#define private public
#define protected public
#include "ui_ability.h"
#undef protected
#undef private
#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_recovery.h"
#include "fa_ability_thread.h"
#include "hilog_wrapper.h"
#include "mock_lifecycle_observer.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using OHOS::Parcel;

class UIAbilityBaseTest : public testing::Test {
public:
    UIAbilityBaseTest() : ability_(nullptr) {}
    ~UIAbilityBaseTest() {}
    std::shared_ptr<AbilityRuntime::UIAbility> ability_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UIAbilityBaseTest::SetUpTestCase(void) {}

void UIAbilityBaseTest::TearDownTestCase(void) {}

void UIAbilityBaseTest::SetUp(void)
{
    ability_ = std::make_shared<AbilityRuntime::UIAbility>();
}

void UIAbilityBaseTest::TearDown(void) {}

/**
 * @tc.number: AbilityRuntime_Name_0100
 * @tc.name: GetUIAbilityName
 * @tc.desc: Verify that the return value of getabilityname is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_Name_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_Name_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "UIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    EXPECT_STREQ(abilityInfo->name.c_str(), ability_->GetAbilityName().c_str());
    GTEST_LOG_(INFO) << "AbilityRuntime_Name_0100 end";
}

/**
 * @tc.number: AbilityRuntime_GetAbilityName_0100
 * @tc.name: GetAbilityName
 * @tc.desc: Verify that the getabilityname return value is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetAbilityName_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetAbilityName_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    std::string name = "LOL";
    abilityInfo->name = name;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    EXPECT_STREQ(ability_->GetAbilityName().c_str(), name.c_str());
    GTEST_LOG_(INFO) << "AbilityRuntime_GetAbilityName_0100 end";
}

/**
 * @tc.number: AbilityRuntime_GetLifecycle_0100
 * @tc.name: GetLifecycle
 * @tc.desc: Verify that the return value of getlifecycle is not empty.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetLifecycle_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetLifecycle_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);
    GTEST_LOG_(INFO) << "AaFwk_Ability_GetLifecycle_0100 end";
}

/**
 * @tc.number: UIAbility_Create_0100
 * @tc.name: UIAbility_Create_0100
 * @tc.desc: Create JS ability.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_Create_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::UIAbility::Create(runtime);
    EXPECT_NE(ability, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_Create_0200
 * @tc.name: UIAbility_Create_0200
 * @tc.desc: Create ability which runtime is nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_Create_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto ability = AbilityRuntime::UIAbility::Create(nullptr);
    EXPECT_NE(ability, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: AbilityRuntime_OnNewWant_0100
 * @tc.name: OnNewWant
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnNewWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnNewWant_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    Want want;
    ability_->OnNewWant(want);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnNewWant_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnRestoreAbilityState_0100
 * @tc.name: OnRestoreAbilityState
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnRestoreAbilityState_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnRestoreAbilityState_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    PacMap inState;
    ability_->OnRestoreAbilityState(inState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnRestoreAbilityState_0100 end";
}

/**
 * @tc.number: AbilityRuntime_TerminateAbility_0100
 * @tc.name: GetWindow
 * @tc.desc: Test whether GetWindow is called normally.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_GetWindow_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_GetWindow_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    EXPECT_EQ(ability_->GetWindow(), nullptr);
    GTEST_LOG_(INFO) << "AbilityRuntime_GetWindow_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnStart_0100
 * @tc.name: OnStart
 * @tc.desc: Test whether OnStart is called normally and verify whether the members are correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    Want want;
    ability_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnStart_0200
 * @tc.name: OnStart
 * @tc.desc: Test the OnStart exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0200 start";
    Want want;
    ability_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStart_0200 end";
}

/**
 * @tc.name: AbilityRuntime_OnStart_0300
 * @tc.desc: UIAbility OnStart test when configuration is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    application->SetConfiguration(config);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability->Init(abilityRecord, application, handler, token);
    Want want;
    ability->OnStart(want);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityRuntime_OnStart_0400
 * @tc.desc: UIAbility OnStart test when ability lifecycle executor or lifecycle is nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStart_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    ability->abilityInfo_ = abilityInfo;
    Want want;
    ability->OnStart(want);
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnStart(want);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: AbilityRuntime_OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: Test whether onstop is called normally and verify whether the members are correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    ability_->OnStop();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnStop_0200
 * @tc.name: OnStop
 * @tc.desc: Test the OnStop exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStop_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0200 start";
    ability_->OnStop();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnStop_0200 end";
}

/**
 * @tc.name: AaFwk_Ability_OnStop_0300
 * @tc.desc: UIAbility OnStop test when ability recovery, window is not nullptr.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnStop_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    EXPECT_NE(abilityRecovery, nullptr);
    ability->EnableAbilityRecovery(abilityRecovery);
    ability->OnStop();

    // window is not nullptr
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->OnStop();

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->OnStop();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: DestroyInstance_0100
 * @tc.desc: UIAbility DestroyInstance test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, DestroyInstance_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_DestroyInstance";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->isStageBasedModel = false;
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability->Init(abilityRecord, application, handler, token);
    ability->DestroyInstance();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: AbilityRuntime_OnForeground_0100
 * @tc.name: OnForeground
 * @tc.desc: Test whether onforegroup is called normally, and verify whether the member is correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnForeground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    Want want;
    ability_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnForeground_0200
 * @tc.name: OnForeground
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnForeground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0200 start";
    Want want;
    ability_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_OnForeground_0300
 * @tc.name: OnForeground
 * @tc.desc: Test the OnForeground exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnForeground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    Want want;
    ability_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnForeground_0300 end";
}

/**
 * @tc.number: AbilityRuntime_OnBackground_0100
 * @tc.name: OnBackground
 * @tc.desc: Test whether onbackground is called normally and verify whether the members are correct.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    ability_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_BACKGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0100 end";
}

/**
 * @tc.number: AbilityRuntime_OnBackground_0200
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnBackground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    ability_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW, state);
    EXPECT_TRUE(lifeCycle);
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0200 end";
}

/**
 * @tc.number: AbilityRuntime_OnBackground_0300
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(UIAbilityBaseTest, AaFwk_Ability_OnBackground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_OnBackground_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    ability_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    // Sence is nullptr, so lifecycle schedule failed.
    EXPECT_NE(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_NE(LifeCycle::Event::UNDEFINED, lifeCycleState);
    GTEST_LOG_(INFO) << "AbilityRuntime_OBackground_0300 end";
}

/**
 * @tc.name: AaFwk_Ability_OnBackground_0400
 * @tc.desc: Ability OnBackground basic test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityRuntime_OnBackground_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    // ability info is nullptr
    ability->OnBackground();
    // stage mode, scene is not nullptr
    auto abilityInfo = std::make_shared<AbilityInfo>();
    EXPECT_NE(abilityInfo, nullptr);
    abilityInfo->name = "test_OnStart";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    auto eventRunner = EventRunner::Create(abilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability->Init(abilityRecord, application, handler, token);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->OnBackground();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityCreate_0100
 * @tc.desc: UIAbility create test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityCreate_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::UIAbility::Create(runtime);
    EXPECT_NE(ability, nullptr);
    AbilityRuntime::Runtime::Options anotherOptions;
    anotherOptions.lang = static_cast<AbilityRuntime::Runtime::Language>(100); // invalid Runtime::Language
    auto anotherRuntime = AbilityRuntime::Runtime::Create(anotherOptions);
    auto anotherAbility = AbilityRuntime::UIAbility::Create(anotherRuntime);
    EXPECT_NE(anotherAbility, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnStop_0100
 * @tc.desc: UIAbility onStop test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnStop_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    bool isAsyncCallback = true;
    ability_->OnStop(nullptr, isAsyncCallback);
    ability_->OnStopCallback();
    EXPECT_EQ(isAsyncCallback, false);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityContinuation_0100
 * @tc.desc: UIAbility Continuation test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityContinuation_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

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
 * @tc.name: UIAbilityContinuation_0200
 * @tc.desc: UIAbility ShouldRecoverState test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityContinuation_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

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
 * @tc.name: UIAbilityContinuation_0300
 * @tc.desc: UIAbility NotifyContinuationResult test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityContinuation_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken);
    ability->Init(abilityRecord, nullptr, handler, nullptr);
    Want want;
    bool success = false;
    ability->NotifyContinuationResult(want, success);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: InitConfigurationProperties_0100
 * @tc.desc: UIAbility InitConfigurationProperties test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, InitConfigurationProperties_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    config.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    ability->InitConfigurationProperties(config, language, colormode, hasPointerDevice);
    EXPECT_EQ(language, "en");
    EXPECT_EQ(colormode, "dark");
    EXPECT_EQ(hasPointerDevice, "true");

    // branch when setting is not nullptr
    auto setting = std::make_shared<AbilityStartSetting>();
    ability->SetStartAbilitySetting(setting);
    ability->InitConfigurationProperties(config, language, colormode, hasPointerDevice);
    EXPECT_EQ(language, "en");
    EXPECT_EQ(colormode, "dark");
    EXPECT_EQ(hasPointerDevice, "true");
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnMemoryLevel_0100
 * @tc.desc: UIAbility OnMemoryLevel test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnMemoryLevel_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    int level = 0;
    ability->OnMemoryLevel(level);
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    ability->OnMemoryLevel(level);
    auto contentInfo = ability->GetContentInfo();
    EXPECT_EQ(contentInfo, "");
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityVirtualFunc_0100
 * @tc.desc: UIAbility virtual function test, such as OnAbilityResult, IsTerminating and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityVirtualFunc_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    Configuration configuration;
    ability->OnConfigurationUpdated(configuration);
    int requestCode = 0;
    int resultCode = 0;
    Want want;
    ability->OnAbilityResult(requestCode, resultCode, want);
    std::vector<std::string> params;
    std::vector<std::string> info;
    ability->Dump(params, info);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityVirtualFunc_0200
 * @tc.desc: UIAbility virtual function test, such as OnStartContinuation, OnSaveData and so on.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityVirtualFunc_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
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
    AAFwk::LaunchParam launchParam;
    ability->SetLaunchParam(launchParam);
    (void)ability->GetLaunchParam();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: DispatchLifecycleOnForeground_0200
 * @tc.desc: UIAbility DispatchLifecycleOnForeground test.
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, DispatchLifecycleOnForeground_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // lifecycle executor is nullptr
    Want want;
    ability->DispatchLifecycleOnForeground(want);

    // lifecycle is nullptr and lifecycle executor is not nullptr
    auto lifecycleExecutor = std::make_shared<AbilityLifecycleExecutor>();
    ability->abilityLifecycleExecutor_ = lifecycleExecutor;
    ability->DispatchLifecycleOnForeground(want);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityFuncList_0100
 * @tc.desc: UIAbility function test, including CallRequest, IsUseNewStartUpRule and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityFuncList_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->CallRequest();
    AAFwk::Want want;
    want.SetParam("component.startup.newRules", true);
    ability->SetWant(want);
    bool isNewRule = ability->IsUseNewStartUpRule();
    EXPECT_EQ(isNewRule, true);
    auto abilityRecovery = std::make_shared<AbilityRecovery>();
    ability->EnableAbilityRecovery(abilityRecovery);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: AbilityFuncList_0200
 * @tc.desc: Ability function test, including OnLeaveForeground and so on
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityFuncList_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnLeaveForeground();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityScene_0100
 * @tc.desc: UIAbility Scene test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityScene_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    ability->OnSceneCreated();
    ability->OnSceneRestored();
    ability->onSceneDestroyed();
    auto scene = ability->GetScene();
    EXPECT_EQ(scene, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityGetCurrentWindowMode_0100
 * @tc.desc: UIAbility GetCurrentWindowMode test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, AbilityGetCurrentWindowMode_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // scene_ is nullptr
    int windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));
    ability->scene_ = std::make_shared<Rosen::WindowScene>();
    windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    windowMode = ability->GetCurrentWindowMode();
    EXPECT_EQ(windowMode, static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilitySetMissionLabel_0100
 * @tc.desc: UIAbility SetMissionLabel test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilitySetMissionLabel_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::string label = "test_label";
    auto ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(pageAbilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken);
    ability->Init(abilityRecord, nullptr, handler, nullptr);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ret = ability->SetMissionLabel(label);
    EXPECT_EQ(ret, -1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilitySetMissionIcon_0100
 * @tc.desc: UIAbility SetMissionIcon test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilitySetMissionIcon_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    auto icon = std::make_shared<Media::PixelMap>();
    auto ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);

    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(pageAbilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken);
    ability->Init(abilityRecord, nullptr, handler, nullptr);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ret = ability->SetMissionIcon(icon);
    EXPECT_EQ(ret, -1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnChange_0100
 * @tc.desc: UIAbility OnChange test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnChange_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken);
    ability->Init(abilityRecord, nullptr, handler, nullptr);

    // application is nullptr
    Rosen::DisplayId displayId = 0;
    ability->OnCreate(displayId);
    ability->OnDestroy(displayId);
    ability->OnChange(displayId);
    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    application->SetConfiguration(config);
    ability->Init(abilityRecord, application, handler, nullptr);
    ability->OnChange(displayId);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityOnDisplayMove_0100
 * @tc.desc: UIAbility OnDisplayMove test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityOnDisplayMove_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken);
    ability->Init(abilityRecord, nullptr, handler, nullptr);

    // application is nullptr
    Rosen::DisplayId fromDisplayId = 1;
    Rosen::DisplayId toDisplayId = 0;
    ability->OnDisplayMove(fromDisplayId, toDisplayId);

    auto application = std::make_shared<OHOSApplication>();
    EXPECT_NE(application, nullptr);
    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    application->SetConfiguration(config);
    ability->Init(abilityRecord, application, handler, nullptr);
    ability->OnDisplayMove(fromDisplayId, toDisplayId);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbilityRequestFocus_0100
 * @tc.desc: UIAbility RequestFocus test
 * @tc.type: FUNC
 * @tc.require: issueI60B7N
 */
HWTEST_F(UIAbilityBaseTest, UIAbilityRequestFocus_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    ASSERT_NE(ability, nullptr);

    // ability window is nullptr
    Want want;
    ability->RequestFocus(want);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, abilityToken);
    ability->Init(abilityRecord, nullptr, handler, nullptr);

    // window is nullptr
    ability->RequestFocus(want);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability->InitWindow(displayId, option);
    ability->RequestFocus(want);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get ResourceManager by function GetResourceManager.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetResourceManager_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    auto resourceMgr = ability->GetResourceManager();
    EXPECT_EQ(resourceMgr, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnStop_AsyncCallback_0100
 * @tc.name: OnStop
 * @tc.desc: Verify OnStop with AsyncCallback.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnStop_AsyncCallback_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    bool isAsyncCallback = false;
    ability_->OnStop(nullptr, isAsyncCallback);
    AbilityLifecycleExecutor::LifecycleState state = ability_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = ability_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_OnConfigurationUpdatedNotify_0100
 * @tc.name: OnConfigurationUpdatedNotify
 * @tc.desc: Verify OnConfigurationUpdatedNotify.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_OnConfigurationUpdatedNotify_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    Configuration configuration;
    ability_->OnConfigurationUpdatedNotify(configuration);
    EXPECT_NE(ability_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_InitWindow_0100
 * @tc.desc: InitWindow test
 * @tc.desc: Verify function InitWindow.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_InitWindow_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    ability_->InitWindow(displayId, option);
    ASSERT_NE(ability_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: SetSceneListener_0100
 * @tc.desc: SetSceneListener test
 * @tc.desc: Verify function SetSceneListener.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_SetSceneListener_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<Rosen::IWindowLifeCycle> listener;
    ability_->SetSceneListener(listener);
    ASSERT_NE(ability_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: Ability_GetWindowOption_0100
 * @tc.desc: GetWindowOption test
 * @tc.desc: Verify function GetWindowOption.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetWindowOption_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    CustomizeData custData = CustomizeData("ShowOnLockScreen", "0", "");
    std::vector<CustomizeData> vecCustData;
    vecCustData.push_back(custData);
    abilityInfo->metaData.customizeData = vecCustData;
    Want want;
    auto option = ability_->GetWindowOption(want);
    ASSERT_NE(option, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_DoOnForeground_0100
 * @tc.desc: DoOnForeground test
 * @tc.desc: Verify function DoOnForeground when abilityWindow is not nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_DoOnForeground_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = false;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    Want want;
    ability_->DoOnForeground(want);
    ASSERT_NE(ability_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: UIAbility_DoOnForeground_0200
 * @tc.desc: DoOnForeground test
 * @tc.desc: Verify function DoOnForeground when abilityWindow is nullptr.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_DoOnForeground_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "ability";
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new AbilityRuntime::FAAbilityThread());
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);
    ability_->Init(abilityRecord, application, handler, token);
    Want want;
    ability_->DoOnForeground(want);
    ASSERT_NE(ability_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetAbilityContext_0100
 * @tc.desc: GetAbilityContext test
 * @tc.desc: Verify function GetAbilityContext.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_GetAbilityContext_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto abilityContext = ability_->GetAbilityContext();
    ASSERT_EQ(abilityContext, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.number: UIAbility_RegisterAbilityLifecycleObserver_0100
 * @tc.name: UIAbility RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver test.
 * @tc.desc: Verify function RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver.
 */
HWTEST_F(UIAbilityBaseTest, UIAbility_RegisterAbilityLifecycleObserver_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<AbilityRuntime::UIAbility> ability = std::make_shared<AbilityRuntime::UIAbility>();
    EXPECT_NE(ability, nullptr);

    // init UIAbility to make sure lifecycle is created.
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, nullptr);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);
    ability->Init(abilityRecord, nullptr, handler, nullptr);
    std::shared_ptr<LifeCycle> lifeCycle = ability->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);

    // register lifecycle observer on UIAbility, so that it can receive lifecycle callback from UIAbility.
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    EXPECT_EQ(LifeCycle::Event::UNDEFINED, observer->GetLifecycleState());
    ability->RegisterAbilityLifecycleObserver(observer);

    // mock UIAbility lifecycle events, expecting that observer can observe them.
    Want want;
    ability->OnStart(want);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycle->GetLifecycleState());
    EXPECT_EQ(LifeCycle::Event::ON_START, observer->GetLifecycleState());
    LifeCycle::Event finalObservedState = observer->GetLifecycleState();

    // unregister lifecycle observer on UIAbility, expecting that observer remains in the previous state,
    // can not observe later lifecycle events anymore.
    ability->UnregisterAbilityLifecycleObserver(observer);
    ability->OnStop();
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycle->GetLifecycleState());
    EXPECT_EQ(finalObservedState, observer->GetLifecycleState());
}
} // namespace AppExecFwk
} // namespace OHOS
