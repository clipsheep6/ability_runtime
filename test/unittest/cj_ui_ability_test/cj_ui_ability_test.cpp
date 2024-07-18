/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cj_ui_ability.h"
#include "ui_ability.h"

#include "insight_intent_executor_info.h"
#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_recovery.h"
#include "ability_local_record.h"
#include "cj_ability_object.h"
#include "cj_runtime.h"
#include "hilog_wrapper.h"
#include "int_wrapper.h"
#include "mock_lifecycle_observer.h"
#include "ohos_application.h"
#include "runtime.h"
#include "string_wrapper.h"
#include "ability_context_impl.h"

using namespace testing;
using namespace testing::ext;

namespace {
const std::string TEST_BUNDLE_NANE = "test.bundleName";
const std::string TEST_MODULE_NANE = "test.entry";
const std::string TEST_ABILITY_NANE = "test.abilityName";
const std::string TEST_CALLER_BUNDLE_NANE = "test.callerBundleName";
const std::string TEST_PLAY_MUSIC = "PlayMusic";
const std::string ABILITY_STAGE_MONITOR_SRC_ENTRANCE = "MainAbility";
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplication";
const std::string CHANGE_VALUE_TEST_BUNDLE_NAME = "com.example.myapplication1";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner";
const std::string CHANGE_VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner1";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE = "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010";
const std::string CHANGE_VALUE_TEST_CASE =
    "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction00101";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "50";
const std::string CHANGE_VALUE_TEST_WAIT_TIMEOUT = "80";
const std::string SET_VALUE_TEST_BUNDLE_NAME = "com.example.myapplicationset";
const std::string ABILITY_NAME = "com.example.myapplication.MainAbility";
const std::string FINISH_MSG = "finish message";
const int32_t FINISH_RESULT_CODE = 144;
const std::string PRINT_MSG = "print aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
const int ZERO = 0;
const int ONE = 1;
const int TWO = 2;
const int64_t TIMEOUT = 50;
const std::string CMD = "ls -l";
const std::string KEY_TEST_DEBUG{"-D"};
const std::string VALUE_TEST_DEBUG{"true"};
const std::string ABILITY_STAGE_MONITOR_MODULE_NAME{"entry"};
}  // namespace

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr char CJ_UI_ABILITY[] = "cjUIAbility";
constexpr char DEFAULT_LANGUAGE[] = "zh_CN";
}  // namespace
class CjUIAbilityTest : public testing::Test {
public:
    CjUIAbilityTest()
    {}
    ~CjUIAbilityTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AbilityRuntime::CJUIAbility> cjAbility_ = nullptr;
};

void PreSetCJAbilityStageFuncs()
{
    auto registerFunc = [](CJAbilityFuncs *funcs) {
        funcs->cjAbilityCreate = [](const char *name) -> int64_t { return 1; };
        funcs->cjAbilityRelease = [](int64_t id) {};
        funcs->cjAbilityOnStart = [](int64_t id, WantHandle want, CJLaunchParam launchParam) {};
        funcs->cjAbilityOnStop = [](int64_t id) {};
        funcs->cjAbilityOnSceneCreated = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneRestored = [](int64_t id, WindowStagePtr cjWindowStage) {};
        funcs->cjAbilityOnSceneDestroyed = [](int64_t id) {};
        funcs->cjAbilityOnForeground = [](int64_t id, WantHandle want) {};
        funcs->cjAbilityOnBackground = [](int64_t id) {};
        funcs->cjAbilityOnConfigurationUpdated = [](int64_t id, CJConfiguration configuration) {};
        funcs->cjAbilityOnNewWant = [](int64_t id, WantHandle want, CJLaunchParam launchParam) {};
        funcs->cjAbilityDump = [](int64_t id, VectorStringHandle params) { return VectorStringHandle(); };
        funcs->cjAbilityOnContinue = [](int64_t id, const char *params) { return 0; };
        funcs->cjAbilityInit = [](int64_t id, void *ability) {};
    };
    RegisterCJAbilityFuncs(registerFunc);
}

void CjUIAbilityTest::SetUpTestCase(void)
{}

void CjUIAbilityTest::TearDownTestCase(void)
{}

void CjUIAbilityTest::SetUp(void)
{
    auto cjAbilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    cjAbility_ = std::make_shared<AbilityRuntime::CJUIAbility>(*(cjAbilityRuntime.get()));
}

void CjUIAbilityTest::TearDown(void)
{}

/**
 * @tc.number: CJRuntime_Init_0100
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_Init_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CJRuntime_Init_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    cjAbility_->Init(nullptr, application, handler, token);

    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);

    abilityInfo->isModuleJson = true;
    abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);

    GTEST_LOG_(INFO) << "CJRuntime_Init_0100 end";
}

/**
 * @tc.number: CJUIAbility_Create_0100
 * @tc.name: CJUIAbility_Create_0100
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_Create_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::CJ;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::CJUIAbility::Create(runtime);
    EXPECT_NE(ability, nullptr);
}

/**
 * @tc.number: CJUIAbility_Create_0200
 * @tc.name: CJUIAbility_Create_0200
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_Create_0200, TestSize.Level1)
{
    auto ability = AbilityRuntime::CJUIAbility::Create(nullptr);
    EXPECT_NE(ability, nullptr);
}

/**
 * @tc.number: CJRuntime_OnNewWant_0100
 * @tc.name: OnNewWant
 * @tc.desc: Test whether onnewwant can be called normally.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnNewWant_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnNewWant_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    Want want;
    cjAbility_->OnNewWant(want);
    GTEST_LOG_(INFO) << "CJRuntime_OnNewWant_0100 end";
}

HWTEST_F(CjUIAbilityTest, CJRuntime_OnStart_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnStart_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    Want want;
    cjAbility_->OnStart(want);
    GTEST_LOG_(INFO) << "CJRuntime_OnStart_0100 end";
}

/**
 * @tc.number: CJRuntime_OnStart_0300
 * @tc.name: OnStart
 * @tc.desc: Test the OnStart exception.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnStart_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnStart_0300 start";
    Want want;
    cjAbility_->OnStart(want);
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "CJRuntime_OnStart_0300 end";
}

/**
 * @tc.number: CJRuntime_OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: Test whether onstop is called normally and verify whether the members are correct.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnStop_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnStop_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    cjAbility_->OnStop();
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);
    GTEST_LOG_(INFO) << "CJRuntime_OnStop_0100 end";
}

/**
 * @tc.number: CJRuntime_OnBackground_0300
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnBackground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnBackground_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    cjAbility_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    // Sence is nullptr, so lifecycle schedule failed.
    EXPECT_NE(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_NE(LifeCycle::Event::UNDEFINED, lifeCycleState);
    GTEST_LOG_(INFO) << "CJRuntime_OnBackground_0300 end";
}

/**
 * @tc.name: cjUIAbilityCreate_0100
 * @tc.desc: UIAbility create test.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, cjUIAbilityCreate_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    auto ability = AbilityRuntime::CJUIAbility::Create(runtime);
    EXPECT_NE(ability, nullptr);
    AbilityRuntime::Runtime::Options anotherOptions;
    anotherOptions.lang = static_cast<AbilityRuntime::Runtime::Language>(100);  // invalid Runtime::Language
    auto anotherRuntime = AbilityRuntime::Runtime::Create(anotherOptions);
    auto anotherAbility = AbilityRuntime::CJUIAbility::Create(anotherRuntime);
    EXPECT_NE(anotherAbility, nullptr);
}

/**
 * @tc.name: CJUIAbilityOnStop_0100
 * @tc.desc: CJUIAbility onStop test.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, CJUIAbilityOnStop_0100, TestSize.Level1)
{
    bool isAsyncCallback = true;
    cjAbility_->OnStop(nullptr, isAsyncCallback);
    cjAbility_->OnStopCallback();
    EXPECT_EQ(isAsyncCallback, false);
}

/**
 * @tc.name: CJUIAbilityOnMemoryLevel_0100
 * @tc.desc: CJUIAbility OnMemoryLevel test.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, CJUIAbilityOnMemoryLevel_0100, TestSize.Level1)
{
    int level = 0;
    cjAbility_->OnMemoryLevel(level);
    auto contentInfo = cjAbility_->GetContentInfo();
    EXPECT_EQ(contentInfo, "");
}

/**
 * @tc.number: CJUIAbility_OnStop_AsyncCallback_0100
 * @tc.name: OnStop_AsyncCallback
 * @tc.desc: Verify OnStop with AsyncCallback.
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_OnStop_AsyncCallback_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type = AbilityType::PAGE;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    bool isAsyncCallback = false;
    cjAbility_->OnStop(nullptr, isAsyncCallback);
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    auto *callbackInfo = AbilityTransactionCallbackInfo<>::Create();
    cjAbility_->OnStop(callbackInfo, isAsyncCallback);
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::INITIAL, state);
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycleState);
}

/**
 * @tc.number: CJUIAbility_GetCJAbility_0100
 * @tc.name: GetCJAbility
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_GetCJAbility_0100, TestSize.Level1)
{
    std::shared_ptr<AbilityRuntime::CJAbilityObject> ptr = cjAbility_->GetCJAbility();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.number: CJUIAbility_OnShare_0100
 * @tc.name: OnShare
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_OnShare_0100, TestSize.Level1)
{
    WantParams data;
    int32_t ret = cjAbility_->OnShare(data);
    EXPECT_EQ(ERR_OK, ret);
}

#ifdef SUPPORT_GRAPHICS
/**
 * @tc.name: CJUIAbilityScene_0100
 * @tc.desc: CJUIAbility Scene test
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, CJUIAbilityScene_0100, TestSize.Level1)
{
    ASSERT_NE(cjAbility_, nullptr);
    cjAbility_->OnSceneCreated();
    cjAbility_->OnSceneRestored();
    cjAbility_->onSceneDestroyed();
    auto scene = cjAbility_->GetScene();
    EXPECT_EQ(scene, nullptr);
}

/**
 * @tc.number: CJRuntime_OnForeground_0100
 * @tc.name: OnForeground
 * @tc.desc: Test whether onforegroup is called normally, and verify whether the member is correct.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnForeground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnForeground_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    Want want;
    cjAbility_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "CJRuntime_OnForeground_0100 end";
}

/**
 * @tc.number: CJRuntime_OnForeground_0200
 * @tc.name: OnForeground
 * @tc.desc: Test the OnInactive exception.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnForeground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnForeground_0200 start";
    Want want;
    cjAbility_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED, state);
    EXPECT_EQ(nullptr, lifeCycle);
    GTEST_LOG_(INFO) << "CJRuntime_OnForeground_0200 end";
}

/**
 * @tc.number: CJRuntime_OnForeground_0300
 * @tc.name: OnForeground
 * @tc.desc: Test the OnForeground exception.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnForeground_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnForeground_0300 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    abilityInfo->isStageBasedModel = true;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    Want want;
    cjAbility_->OnForeground(want);
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_FOREGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "CJRuntime_OnForeground_0300 end";
}

/**
 * @tc.name: CJUIAbilityVirtualFunc_0100
 * @tc.desc: CJUIAbility virtual function test, such as OnAbilityResult, IsTerminating and so on.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, CJUIAbilityVirtualFunc_0100, TestSize.Level1)
{
    // ability window is nullptr
    Want want;
    cjAbility_->RequestFocus(want);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, nullptr);
    cjAbility_->Init(abilityLocalRecord, nullptr, handler, nullptr);
    cjAbility_->UpdateContextConfiguration();
    int requestCode = 0;
    int resultCode = 0;
    cjAbility_->OnAbilityResult(requestCode, resultCode, want);
    std::vector<std::string> params;
    std::vector<std::string> info;
    cjAbility_->Dump(params, info);
}

/**
 * @tc.name: CJUIAbilityVirtualFunc_0200
 * @tc.desc: CJUIAbility virtual function test, such as OnStartContinuation, OnSaveData and so on.
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, CJUIAbilityVirtualFunc_0200, TestSize.Level1)
{
    bool ret = cjAbility_->OnStartContinuation();
    EXPECT_EQ(ret, false);
    WantParams data;
    ret = cjAbility_->OnSaveData(data);
    EXPECT_EQ(ret, false);
    ret = cjAbility_->OnRestoreData(data);
    EXPECT_EQ(ret, false);
    int onContinueRet = cjAbility_->OnContinue(data);
    int32_t reason = 0;
    EXPECT_EQ(cjAbility_->OnSaveState(reason, data), 0);
    int result = 0;
    cjAbility_->OnCompleteContinuation(result);
    cjAbility_->OnRemoteTerminated();
    sptr<IRemoteObject> ptr = cjAbility_->CallRequest();
    EXPECT_EQ(ptr, nullptr);
}

/**
 * @tc.name: CJUIAbilityRequestFocus_0100
 * @tc.desc: CJUIAbility RequestFocus test
 * @tc.type: FUNC
 */
HWTEST_F(CjUIAbilityTest, CJUIAbilityRequestFocus_0100, TestSize.Level1)
{
    // ability window is nullptr
    Want want;
    cjAbility_->RequestFocus(want);
    std::shared_ptr<AbilityInfo> pageAbilityInfo = std::make_shared<AbilityInfo>();
    pageAbilityInfo->type = AppExecFwk::AbilityType::PAGE;
    pageAbilityInfo->isStageBasedModel = true;
    auto eventRunner = EventRunner::Create(pageAbilityInfo->name);
    auto handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord =
        std::make_shared<AbilityLocalRecord>(pageAbilityInfo, nullptr);
    cjAbility_->Init(abilityLocalRecord, nullptr, handler, nullptr);

    // window is nullptr
    cjAbility_->RequestFocus(want);
    int32_t displayId = 0;
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    cjAbility_->InitWindow(displayId, option);
    cjAbility_->RequestFocus(want);
}

/**
 * @tc.number: CJRuntime_OnBackground_0100
 * @tc.name: OnBackground
 * @tc.desc: Test whether onbackground is called normally and verify whether the members are correct.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnBackground_0100 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    AbilityType type = AbilityType::PAGE;
    abilityInfo->type = type;
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    cjAbility_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    LifeCycle::Event lifeCycleState = lifeCycle->GetLifecycleState();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW, state);
    EXPECT_EQ(LifeCycle::Event::ON_BACKGROUND, lifeCycleState);
    GTEST_LOG_(INFO) << "CJRuntime_OnBackground_0100 end";
}

/**
 * @tc.number: CJRuntime_OnBackground_0200
 * @tc.name: OnBackground
 * @tc.desc: Test the OnBackground exception.
 */
HWTEST_F(CjUIAbilityTest, CJRuntime_OnBackground_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "CJRuntime_OnBackground_0200 start";
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<OHOSApplication> application = nullptr;
    std::shared_ptr<AbilityHandler> handler = nullptr;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    cjAbility_->Init(abilityLocalRecord, application, handler, token);
    cjAbility_->OnBackground();
    AbilityLifecycleExecutor::LifecycleState state = cjAbility_->GetState();
    std::shared_ptr<LifeCycle> lifeCycle = cjAbility_->GetLifecycle();
    EXPECT_EQ(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW, state);
    EXPECT_TRUE(lifeCycle);
    GTEST_LOG_(INFO) << "CJRuntime_OnBackground_0200 end";
}

/**
 * @tc.number: CJUIAbility_OnBackPress_0100
 * @tc.name: OnBackPress
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_OnBackPress_0100, TestSize.Level1)
{
    bool ret = cjAbility_->OnBackPress();
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: CJUIAbility_OnPrepareTerminate_0100
 * @tc.name: OnPrepareTerminate
 */
HWTEST_F(CjUIAbilityTest, CJUIAbility_OnPrepareTerminate_0100, TestSize.Level1)
{
    bool ret = cjAbility_->OnPrepareTerminate();
    EXPECT_TRUE(ret);
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0100, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;
    initedCJUIAbility_->OnStart(want);
    initedCJUIAbility_->OnStop();
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0200, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;

    std::shared_ptr<AbilityRuntime::CJAbilityObject> initedPtr = initedCJUIAbility_->GetCJAbility();
    EXPECT_NE(initedPtr, nullptr);

    initedCJUIAbility_->OnSceneCreated();
    initedCJUIAbility_->OnSceneRestored();
    initedCJUIAbility_->onSceneDestroyed();

    initedCJUIAbility_->OnForeground(want);
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0300, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;
    int requestCode = 0;
    int resultCode = 0;
    initedCJUIAbility_->OnAbilityResult(requestCode, resultCode, want);
    std::vector<std::string> params;
    std::vector<std::string> info;
    initedCJUIAbility_->Dump(params, info);
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0400, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;
    WantParams data;
    int continueRet = initedCJUIAbility_->OnContinue(data);

    initedCJUIAbility_->OnBackground();

    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    initedCJUIAbility_->GetCJRuntime();
    initedCJUIAbility_->RequestFocus(want);
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0500, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;

    WantParams data;
    int continueRet = initedCJUIAbility_->OnContinue(data);

    initedCJUIAbility_->OnBackground();

    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    std::string test = "test";
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    initedCJUIAbility_->ExecuteInsightIntentRepeateForeground(want, nullptr, std::move(callback));
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0600, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;

    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    initedCJUIAbility_->OnSceneCreated();
    initedCJUIAbility_->OnSceneRestored();
    initedCJUIAbility_->OnSceneDestroyed();
}

HWTEST_F(CjUIAbilityTest, InitedCJUIAbilityTest_0700, TestSize.Level1)
{
    auto abilityRuntime = std::make_unique<AbilityRuntime::CJRuntime>();
    std::shared_ptr<AbilityRuntime::CJUIAbility> initedCJUIAbility_ =
        std::make_shared<AbilityRuntime::CJUIAbility>(*(abilityRuntime.get()));
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "CJUIability";
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    sptr<IRemoteObject> token = nullptr;
    abilityInfo->isModuleJson = false;
    PreSetCJAbilityStageFuncs();
    std::shared_ptr<AbilityLocalRecord> abilityLocalRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    initedCJUIAbility_->Init(abilityLocalRecord, application, handler, token);

    Want want;

    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);

    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    initedCJUIAbility_->RequestFocus(want);

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContextImpl =
        std::make_shared<AbilityRuntime::AbilityContextImpl>();
    initedCJUIAbility_->AttachAbilityContext(abilityContextImpl);
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContextRet = initedCJUIAbility_->GetAbilityContext();
    EXPECT_TRUE(abilityContextRet != nullptr);
    int requestCode = 0;
    int resultCode = 0;
    initedCJUIAbility_->OnAbilityResult(requestCode, resultCode, want);
    initedCJUIAbility_->OnStop();
}

#endif

}  // namespace AppExecFwk
}  // namespace OHOS
