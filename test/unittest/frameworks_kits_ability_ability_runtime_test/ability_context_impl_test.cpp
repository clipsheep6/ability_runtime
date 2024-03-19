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
#define private public
#include "ability.h"
#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_info.h"
#define protected public
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "ability_thread.h"
#include "iability_callback.h"
#include "mock_context.h"
#include "mock_lifecycle_observer.h"
#include "mock_serviceability_manager_service.h"
#include "scene_board_judgement.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
namespace {
std::string TEST_LABEL = "testLabel";
OHOS::sptr<MockServiceAbilityManagerService> g_mockAbilityMs = nullptr;
}

class MyAbilityCallback : public IAbilityCallback {
public:
    virtual int GetCurrentWindowMode()
    {
        return 0;
    }

    virtual ErrCode SetMissionLabel(const std::string& label)
    {
        return 0;
    }

    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap>& icon)
    {
        GTEST_LOG_(INFO) << "========AbilityCallback SetMissionIcon------------------------.";
        return 0;
    }

    virtual void GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
    {
        return;
    }

    virtual Ace::UIContent* GetUIContent()
    {
        return nullptr;
    }

    void EraseUIExtension(int32_t sessionId)
    {
        return;
    }

    void RegisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
    {
    }

    void UnregisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
    {
    }
};

class AbilityContextImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::unique_ptr<AbilityContextImpl> context_ = nullptr;
    std::shared_ptr<MockContext> mock_ = nullptr;
};

void AbilityContextImplTest::SetUpTestCase(void)
{
    g_mockAbilityMs = new (std::nothrow) MockServiceAbilityManagerService();
    auto sysMgr = OHOS::DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == NULL) {
        GTEST_LOG_(ERROR) << "fail to get ISystemAbilityManager";
        return;
    }
    sysMgr->RegisterSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID, g_mockAbilityMs);
}

void AbilityContextImplTest::TearDownTestCase(void)
{}

void AbilityContextImplTest::SetUp(void)
{
    context_ = std::make_unique<AbilityContextImpl>();
    mock_ = std::make_shared<MockContext>();
}

void AbilityContextImplTest::TearDown(void)
{}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process StartAbility, and the result is success(localCallContainer_ is null).
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0100, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});

    ErrCode ret = context_->StartAbilityByCall(want, callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process StartAbility, and the result is success(localCallContainer_ is not null).
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0200, Function | MediumTest | Level1)
{
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});

    context_->localCallContainer_ = std::make_shared<LocalCallContainer>();
    EXPECT_NE(context_->localCallContainer_, nullptr);

    ErrCode ret = context_->StartAbilityByCall(want, callback);
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_ReleaseCall_0100
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process ReleaseCall, and the result is success.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReleaseCall_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "========Ability_Context_Impl_ReleaseCall_0100beagin==============.";

    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    callback->SetCallBack([](const sptr<IRemoteObject>&) {});
    std::shared_ptr<CallerCallBack> callbackSec = std::make_shared<CallerCallBack>();
    callbackSec->SetCallBack([](const sptr<IRemoteObject>&) {});

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);
    localCallRecord->AddCaller(callbackSec);

    context_->localCallContainer_ = std::make_shared<LocalCallContainer>();
    EXPECT_NE(context_->localCallContainer_, nullptr);

    context_->localCallContainer_->SetCallLocalRecord(elementName, localCallRecord);

    ErrCode ret = context_->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "========Ability_Context_Impl_ReleaseCall_0100end==============.";
}

/**
 * @tc.number: Ability_Context_Impl_ReleaseCall_0200
 * @tc.name: StartAbility
 * @tc.desc: Ability context to process ReleaseCall, and the result is fail because localCallContainer is null.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ReleaseCall_0200, Function | MediumTest | Level1)
{
    std::shared_ptr<CallerCallBack> callback = std::make_shared<CallerCallBack>();
    ErrCode ret = context_->ReleaseCall(callback);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Context_Impl_IsTerminating_0100
 * @tc.name: IsTerminating
 * @tc.desc: Test IsTerminating return value when called SetTerminating.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_IsTerminating_0100, Function | MediumTest | Level1)
{
    context_->SetTerminating(true);
    bool ret = context_->IsTerminating();
    EXPECT_TRUE(ret);
    context_->SetTerminating(false);
    ret = context_->IsTerminating();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionContinueState_0100
 * @tc.desc: test set mission continue state.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionContinueState_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);

    AAFwk::ContinueState state = AAFwk::ContinueState::CONTINUESTATE_ACTIVE;
    auto ret = context_->SetMissionContinueState(state);
    EXPECT_NE(ret, 0);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionContinueState(state);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionLabel_0100
 * @tc.desc: test set mission label.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionLabel_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);

    auto ret = context_->SetMissionLabel(TEST_LABEL);
    EXPECT_NE(ret, 0);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionLabel(TEST_LABEL);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ret = context_->SetMissionLabel(TEST_LABEL);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.name: Ability_Context_Impl_SetMissionIcon_0100
 * @tc.desc: test set mission icon.
 * @tc.type: FUNC
 * @tc.require: I5OB2Y
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetMissionIcon_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(g_mockAbilityMs != nullptr);
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    g_mockAbilityMs->SetCommonMockResult(false);
    usleep(10);

    std::shared_ptr<OHOS::Media::PixelMap> icon = nullptr;
    auto ret = context_->SetMissionIcon(icon);
    EXPECT_NE(ret, 0);

    g_mockAbilityMs->SetCommonMockResult(true);
    ret = context_->SetMissionIcon(icon);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ret = context_->SetMissionIcon(icon);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, 0);
    }

    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}

/**
 * @tc.number: Ability_Context_Impl_GetCurrentWindowMode_0100
 * @tc.name: GetCurrentWindowMode
 * @tc.desc: Get Current Window Mode failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCurrentWindowMode_0100, Function | MediumTest | Level1)
{
    std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback;
    context_->RegisterAbilityCallback(abilityCallback);
    auto ret = context_->GetCurrentWindowMode();
    EXPECT_EQ(ret, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
}

/**
 * @tc.number: Ability_Context_Impl_GetCurrentWindowMode_0200
 * @tc.name: GetCurrentWindowMode
 * @tc.desc: Get Current Window Mode sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCurrentWindowMode_0200, Function | MediumTest | Level1)
{
    auto abilityCallback = std::weak_ptr<AppExecFwk::IAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    auto ret = context_->GetCurrentWindowMode();
    EXPECT_EQ(ret, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectAbility_0100
 * @tc.name: ConnectAbility
 * @tc.desc: Connect Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ConnectAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context_->ConnectAbility(want, connectCallback);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_ConnectAbilityWithAccount_0100
 * @tc.name: ConnectAbilityWithAccount
 * @tc.desc: Connect Ability With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ConnectAbilityWithAccount_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    sptr<AbilityConnectCallback> connectCallback;
    auto ret = context_->ConnectAbilityWithAccount(want, accountId, connectCallback);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_TerminateSelf_0100
 * @tc.name: TerminateSelf
 * @tc.desc: Terminate Self
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_TerminateSelf_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    auto ret = context_->TerminateSelf();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, false);
    }
}

/**
 * @tc.number: Ability_Context_Impl_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo
 * @tc.desc: Set AbilityInfo
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetAbilityInfo_0100, Function | MediumTest | Level1)
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->name = "AbilityContextImplTest";
    context_->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(context_->GetAbilityInfo(), abilityInfo);
}

/**
 * @tc.number: Ability_Context_Impl_SetConfiguration_0100
 * @tc.name: SetConfiguration
 * @tc.desc: Set Configuration
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetConfiguration_0100, Function | MediumTest | Level1)
{
    auto config = std::make_shared<AppExecFwk::Configuration>();
    context_->SetConfiguration(config);
    EXPECT_EQ(context_->GetConfiguration(), config);
}

/**
 * @tc.number: Ability_Context_Impl_CloseAbility_0100
 * @tc.name: CloseAbility
 * @tc.desc: Close Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CloseAbility_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    auto ret = context_->CloseAbility();
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_NE(ret, ERR_OK);
    }
}

/**
 * @tc.number: Ability_Context_Impl_TerminateAbilityWithResult_0100
 * @tc.name: TerminateAbilityWithResult
 * @tc.desc: Terminate Ability With Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_TerminateAbilityWithResult_0100, Function | MediumTest | Level1)
{
    ASSERT_TRUE(context_ != nullptr);
    AAFwk::Want want;
    int32_t resultCode = 1;
    auto ret = context_->TerminateAbilityWithResult(want, resultCode);
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        EXPECT_EQ(ret, ERR_OK);
    }
}

/**
 * @tc.number: Ability_Context_Impl_StopServiceExtensionAbility_0100
 * @tc.name: StopServiceExtensionAbility
 * @tc.desc: Stop Service Extension Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StopServiceExtensionAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    auto ret = context_->StopServiceExtensionAbility(want, accountId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartServiceExtensionAbility_0100
 * @tc.name: StartServiceExtensionAbility
 * @tc.desc: Start Service Extension Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartServiceExtensionAbility_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    auto ret = context_->StartServiceExtensionAbility(want, accountId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResultWithAccount_0100
 * @tc.name: StartAbilityForResultWithAccount
 * @tc.desc: Start Ability For Result With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResultWithAccount_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResultWithAccount_0100 task called"; };
    auto ret = context_->StartAbilityForResultWithAccount(want, accountId, startOptions, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResult_0100
 * @tc.name: StartAbilityForResult
 * @tc.desc: Start Ability For Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResult_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_0100 task called"; };
    auto ret = context_->StartAbilityForResult(want, startOptions, requestCode, std::move(task));
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResultWithAccount_0200
 * @tc.name: StartAbilityForResultWithAccount
 * @tc.desc: Start Ability For Result With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResultWithAccount_0200,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t requestCode = 1;
    int32_t accountId = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResultWithAccount_0200 task called"; };
    auto ret = context_->StartAbilityForResultWithAccount(want, accountId, requestCode, std::move(task));
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityForResult_0200
 * @tc.name: StartAbilityForResult
 * @tc.desc: Start Ability For Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityForResult_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t requestCode = 1;
    RuntimeTask task = [](const int32_t count, const Want& want, bool isInner)
    { GTEST_LOG_(INFO) << "Ability_Context_Impl_StartAbilityForResult_0200 task called"; };
    auto ret = context_->StartAbilityForResult(want, requestCode, std::move(task));
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityWithAccount_0100
 * @tc.name: StartAbilityWithAccount
 * @tc.desc: Start Ability With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityWithAccount_0100, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    auto ret = context_->StartAbilityWithAccount(want, accountId, startOptions, requestCode);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityWithAccount_0200
 * @tc.name: StartAbilityWithAccount
 * @tc.desc: Start Ability With Account
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityWithAccount_0200, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t accountId = 1;
    int32_t requestCode = 1;
    auto ret = context_->StartAbilityWithAccount(want, accountId, requestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0300
 * @tc.name: StartAbility
 * @tc.desc: Start Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0300, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    int32_t requestCode = 1;
    auto ret = context_->StartAbility(want, startOptions, requestCode);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0400
 * @tc.name: StartAbility
 * @tc.desc: Start Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0400, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    int32_t requestCode = 1;
    auto ret = context_->StartAbility(want, requestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbility_0500
 * @tc.name: StartAbility
 * @tc.desc: Start Ability
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbility_0500, Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.extra.param.key.startupMode", 1);
    want.SetFlags(Want::FLAG_ABILITY_CONTINUATION);
    int32_t requestCode = 1;
    auto ret = context_->StartAbility(want, requestCode);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_OnAbilityResult_0100
 * @tc.name: OnAbilityResult
 * @tc.desc: On Ability Result
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_OnAbilityResult_0100, Function | MediumTest | Level1)
{
    int32_t code = 2;
    int32_t resultCode = 2;
    AAFwk::Want resultData;
    context_->OnAbilityResult(code, resultCode, resultData);
    auto count = context_->resultCallbacks_.size();
    EXPECT_EQ(count, 0);
}

/**
 * @tc.number: Ability_Context_Impl_GetDeviceType_0100
 * @tc.name: GetDeviceType
 * @tc.desc: Get Device Type sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDeviceType_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_NOT_SET);
}

/**
 * @tc.number: Ability_Context_Impl_GetDeviceType_0200
 * @tc.name: GetDeviceType
 * @tc.desc: Get Device Type failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDeviceType_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetDeviceType();
    EXPECT_EQ(ret, Global::Resource::DeviceType::DEVICE_PHONE);
}

/**
 * @tc.number: Ability_Context_Impl_GetBaseDir_0100
 * @tc.name: GetBaseDir
 * @tc.desc: Get Base Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBaseDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBaseDir();
    EXPECT_EQ(ret, "/data/app/base");
}

/**
 * @tc.number: Ability_Context_Impl_GetBaseDir_0200
 * @tc.name: GetBaseDir
 * @tc.desc: Get Base Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBaseDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodeDir_0100
 * @tc.name: GetBundleCodeDir
 * @tc.desc: Get Bundle Code Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodeDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "/code");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodeDir_0200
 * @tc.name: GetBundleCodeDir
 * @tc.desc: Get Bundle Code Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodeDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBundleCodeDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetCacheDir_0100
 * @tc.name: GetCacheDir
 * @tc.desc: Get Cache Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCacheDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "/cache");
}

/**
 * @tc.number: Ability_Context_Impl_GetCacheDir_0200
 * @tc.name: GetCacheDir
 * @tc.desc: Get Cache Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetCacheDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetCacheDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetDatabaseDir_0100
 * @tc.name: GetDatabaseDir
 * @tc.desc: Get Database Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDatabaseDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "/data/app/database");
}

/**
 * @tc.number: Ability_Context_Impl_GetDatabaseDir_0200
 * @tc.name: GetDatabaseDir
 * @tc.desc: Get Database Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDatabaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetDatabaseDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetPreferencesDir_0100
 * @tc.name: GetPreferencesDir
 * @tc.desc: Get Preferences Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetPreferencesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "/preferences");
}

/**
 * @tc.number: Ability_Context_Impl_GetPreferencesDir_0200
 * @tc.name: GetPreferencesDir
 * @tc.desc: Get Preferences Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetPreferencesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetPreferencesDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetTempDir_0100
 * @tc.name: GetTempDir
 * @tc.desc: Get Temp Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetTempDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "/temp");
}

/**
 * @tc.number: Ability_Context_Impl_GetTempDir_0200
 * @tc.name: GetTempDir
 * @tc.desc: Get Temp Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetTempDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetTempDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceDir_0100
 * @tc.name: GetResourceDir
 * @tc.desc: Get resource Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetResourceDir();
    EXPECT_EQ(ret, "/resfile");
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceDir_0200
 * @tc.name: GetResourceDir
 * @tc.desc: Get resource Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetResourceDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetGroupDir_0100
 * @tc.name: GetGroupDir
 * @tc.desc: Get Group Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetGroupDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetGroupDir("1");
    EXPECT_EQ(ret, "/group");
}

/**
 * @tc.number: Ability_Context_Impl_GetGroupDir_0200
 * @tc.name: GetGroupDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetGroupDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetGroupDir("1");
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetSystemDatabaseDir_0100
 * @tc.name: GetSystemDatabaseDir
 * @tc.desc: Get Group Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemDatabaseDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string databaseDir;
    auto ret = context_->GetSystemDatabaseDir("1", true, databaseDir);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_GetSystemDatabaseDir_0200
 * @tc.name: GetSystemDatabaseDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemDatabaseDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string databaseDir;
    auto ret = context_->GetSystemDatabaseDir("1", true, databaseDir);
    EXPECT_EQ(ret, OHOS::ERR_INVALID_VALUE);
}


/**
 * @tc.number: Ability_Context_Impl_GetSystemPreferencesDir_0100
 * @tc.name: GetSystemPreferencesDir
 * @tc.desc: Get Group Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemPreferencesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string preferencesDir;
    auto ret = context_->GetSystemPreferencesDir("1", true, preferencesDir);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_GetSystemPreferencesDir_0200
 * @tc.name: GetSystemPreferencesDir
 * @tc.desc: Get Group Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetSystemPreferencesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string databaseDir;
    auto ret = context_->GetSystemPreferencesDir("1", true, databaseDir);
    EXPECT_EQ(ret, OHOS::ERR_INVALID_VALUE);
}


/**
 * @tc.number: Ability_Context_Impl_GetFilesDir_0100
 * @tc.name: GetFilesDir
 * @tc.desc: Get Files Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetFilesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "/files");
}

/**
 * @tc.number: Ability_Context_Impl_GetFilesDir_0200
 * @tc.name: GetFilesDir
 * @tc.desc: Get Files Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetFilesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetFilesDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetDistributedFilesDir_0100
 * @tc.name: GetDistributedFilesDir
 * @tc.desc: Get Distributed Files Dir sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDistributedFilesDir_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "/mnt/hmdfs/device_view/local/data/bundleName");
}

/**
 * @tc.number: Ability_Context_Impl_GetDistributedFilesDir_0200
 * @tc.name: GetDistributedFilesDir
 * @tc.desc: Get Distributed Files Dir failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetDistributedFilesDir_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetDistributedFilesDir();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_IsUpdatingConfigurations_0100
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Is Updating Configurations true
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_IsUpdatingConfigurations_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: Ability_Context_Impl_IsUpdatingConfigurations_0200
 * @tc.name: IsUpdatingConfigurations
 * @tc.desc: Is Updating Configurations false
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_IsUpdatingConfigurations_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->IsUpdatingConfigurations();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: Ability_Context_Impl_PrintDrawnCompleted_0100
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Print Drawn Completed true
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_PrintDrawnCompleted_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.number: Ability_Context_Impl_PrintDrawnCompleted_0200
 * @tc.name: PrintDrawnCompleted
 * @tc.desc: Print Drawn Completed false
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_PrintDrawnCompleted_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->PrintDrawnCompleted();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.number: Ability_Context_Impl_SwitchArea_0200
 * @tc.name: SwitchArea
 * @tc.desc: Switch Area failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SwitchArea_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    int32_t mode = 2;
    context_->SwitchArea(mode);
    EXPECT_EQ(context_->GetArea(), 1);
}

/**
 * @tc.number: Ability_Context_Impl_SwitchArea_0100
 * @tc.name: SwitchArea
 * @tc.desc: Switch Area sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SwitchArea_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    int32_t mode = 2;
    context_->SwitchArea(mode);
    EXPECT_EQ(context_->GetArea(), mode);
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleName_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "com.test.bundleName");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleName_0200
 * @tc.name: GetBundleName
 * @tc.desc: Get BundleName failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleName_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBundleName();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetApplicationInfo_0100
 * @tc.name: GetApplicationInfo
 * @tc.desc: Get ApplicationInfo sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetApplicationInfo_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetApplicationInfo();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetApplicationInfo_0200
 * @tc.name: GetApplicationInfo
 * @tc.desc: Get ApplicationInfo failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetApplicationInfo_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetApplicationInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodePath_0100
 * @tc.name: GetBundleCodePath
 * @tc.desc: Get Bundle Code Path sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodePath_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "codePath");
}

/**
 * @tc.number: Ability_Context_Impl_GetBundleCodePath_0200
 * @tc.name: GetBundleCodePath
 * @tc.desc: Get Bundle Code Path failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetBundleCodePath_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetBundleCodePath();
    EXPECT_EQ(ret, "");
}

/**
 * @tc.number: Ability_Context_Impl_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Get Hap Module Info sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetHapModuleInfo_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetHapModuleInfo_0200
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Get Hap Module Info failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetHapModuleInfo_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceManager_0100
 * @tc.name: GetResourceManager
 * @tc.desc: Get Resource Manager sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceManager_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    auto ret = context_->GetResourceManager();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_GetResourceManager_0200
 * @tc.name: GetResourceManager
 * @tc.desc: Get Resource Manager failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_GetResourceManager_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetResourceManager();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Create Bundle Context sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateBundleContext_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateBundleContext_0200
 * @tc.name: CreateBundleContext
 * @tc.desc: Create Bundle Context failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateBundleContext_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateBundleContext(bundleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0100
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string moduleName = "com.test.moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0200
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string moduleName = "com.test.moduleName";
    auto ret = context_->CreateModuleContext(moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateSystemHspModuleResourceManager_0100
 * @tc.name: CreateSystemHspModuleResourceManager
 * @tc.desc: Create Module Context sucess
 */
HWTEST_F(AbilityContextImplTest,
         Ability_Context_Impl_CreateSystemHspModuleResourceManager_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string moduleName = "com.test.moduleName";
    std::string bundleName = "com.test.bundleName";
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    auto ret = context_->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0300
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context sucess
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0300, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    std::string moduleName = "com.test.moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_CreateModuleContext_0400
 * @tc.name: CreateModuleContext
 * @tc.desc: Create Module Context failed
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_CreateModuleContext_0400, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    std::string moduleName = "com.test.moduleName";
    std::string bundleName = "com.test.bundleName";
    auto ret = context_->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_ClearFailedCallConnection_0100
 * @tc.name: ClearFailedCallConnection
 * @tc.desc: clear failed call connection execute normally
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_ClearFailedCallConnection_0100, Function | MediumTest | Level1)
{
    context_->ClearFailedCallConnection(nullptr);
    EXPECT_EQ(context_->localCallContainer_, nullptr);
    context_->localCallContainer_ = std::make_shared<LocalCallContainer>();
    context_->ClearFailedCallConnection(nullptr);
    EXPECT_NE(context_->localCallContainer_, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_SetWeakSessionToken_0100
 * @tc.name: SetWeakSessionToken
 * @tc.desc: Set weak sessionToken
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_SetWeakSessionToken_0100, Function | MediumTest | Level1)
{
    context_->SetWeakSessionToken(nullptr);
    EXPECT_EQ(context_->sessionToken_, nullptr);
}

/**
 * @tc.number: Ability_Context_Impl_MoveUIAbilityToBackground_0100
 * @tc.name: MoveUIAbilityToBackground
 * @tc.desc: move UIAbility to background
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_MoveUIAbilityToBackground_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "Ability_Context_Impl_MoveUIAbilityToBackground_0100 begin.";
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = g_mockAbilityMs;
    ErrCode ret = context_->MoveUIAbilityToBackground();
    EXPECT_TRUE(ret == ERR_OK);
}

/**
 * @tc.number: Ability_Context_Impl_StartAbilityByType_0100
 * @tc.name: StartAbilityByType
 * @tc.desc: start UIAbility or UIExtensionAbility by type
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_StartAbilityByType_0100, Function | MediumTest | Level1)
{
    AAFwk::WantParams wantParams;
    const std::string type = "share";
    ErrCode ret = context_->StartAbilityByType(type, wantParams, nullptr);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
}

/**
 * @tc.number: Ability_Context_Impl_RegisterAbilityLifecycleObserver_0100
 * @tc.name: RegisterAbilityLifecycleObserver/UnregisterAbilityLifecycleObserver
 * @tc.desc: test register/unregister ability lifecycle observer.
 */
HWTEST_F(AbilityContextImplTest, Ability_Context_Impl_RegisterAbilityLifecycleObserver_0100,
    Function | MediumTest | Level1)
{
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_NE(ability, nullptr);
    std::shared_ptr<AbilityContextImpl> context = std::make_shared<AbilityContextImpl>();

    // attach ability to ability context, so that ability can be registered as lifecycle observer into ability context.
    ability->AttachAbilityContext(context);
    EXPECT_NE(ability->GetAbilityContext(), nullptr);

    // init ability to make sure lifecycle is created.
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);
    ability->Init(abilityInfo, nullptr, handler, nullptr);
    std::shared_ptr<LifeCycle> lifeCycle = ability->GetLifecycle();
    EXPECT_NE(lifeCycle, nullptr);

    // register lifecycle observer on ability, so that it can receive lifecycle callback from ability.
    std::shared_ptr<MockLifecycleObserver> observer = std::make_shared<MockLifecycleObserver>();
    EXPECT_EQ(LifeCycle::Event::UNDEFINED, observer->GetLifecycleState());
    context->RegisterAbilityLifecycleObserver(observer);

    // mock ability lifecycle events, expecting that observer can observe them.
    Want want;
    ability->OnStart(want);
    EXPECT_EQ(LifeCycle::Event::ON_START, lifeCycle->GetLifecycleState());
    EXPECT_EQ(LifeCycle::Event::ON_START, observer->GetLifecycleState());
    LifeCycle::Event finalObservedState = observer->GetLifecycleState();

    // unregister lifecycle observer on ability, expecting that observer remains in the previous state,
    // can not observe later lifecycle events anymore.
    context->UnregisterAbilityLifecycleObserver(observer);
    ability->OnStop();
    EXPECT_EQ(LifeCycle::Event::ON_STOP, lifeCycle->GetLifecycleState());
    EXPECT_EQ(finalObservedState, observer->GetLifecycleState());
}
} // namespace AppExecFwk
} // namespace OHOS
