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
#define private public
#include "ability_context_impl.h"
#define protected public
#include "ability_loader.h"
#include "ability_thread.h"
#include "iability_callback.h"
#include "mock_serviceability_manager_service.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"

namespace OHOS {
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

    virtual ErrCode SetMissionLabel(const std::string &label)
    {
        return 0;
    }

    virtual ErrCode SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
    {
        GTEST_LOG_(INFO) << "========AbilityCallback SetMissionIcon------------------------.";
        return 0;
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
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

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
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    context_->localCallContainer_ = new (std::nothrow)LocalCallContainer();
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
    callback->SetCallBack([](const sptr<IRemoteObject> &) {});

    AppExecFwk::ElementName elementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::shared_ptr<LocalCallRecord> localCallRecord = std::make_shared<LocalCallRecord>(elementName);
    localCallRecord->AddCaller(callback);

    context_->localCallContainer_ = new (std::nothrow) LocalCallContainer();
    EXPECT_NE(context_->localCallContainer_, nullptr);

    std::string uri = elementName.GetURI();
    context_->localCallContainer_->callProxyRecords_.emplace(uri, localCallRecord);

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
    EXPECT_EQ(ret, 0);

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ret = context_->SetMissionLabel(TEST_LABEL);
    EXPECT_EQ(ret, 0);

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
    EXPECT_EQ(ret, 0);

    std::shared_ptr<MyAbilityCallback> abilityCallback = std::make_shared<MyAbilityCallback>();
    context_->RegisterAbilityCallback(abilityCallback);
    ret = context_->SetMissionIcon(icon);
    EXPECT_EQ(ret, 0);

    abilityCallback.reset();
    context_->RegisterAbilityCallback(abilityCallback);
    AAFwk::AbilityManagerClient::GetInstance()->proxy_ = nullptr;
}
} // namespace AppExecFwk
} // namespace OHOS
