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

#define private public
#define protected public
#include "ability_first_frame_state_observer_manager.h"
#undef private
#undef protected
#include <gtest/gtest.h>
#include "ability_first_frame_state_observer_stub.h"
#include "iremote_broker.h"
#include "mock/include/mock_permission_verification.h"
#include "mock/include/mock_my_flag.h"
#include "application_info.h"
#include "ability_info.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
using OHOS::AppExecFwk::AbilityFirstFrameStateObserverStub;
using OHOS::AppExecFwk::AbilityFirstFrameStateData;

class MockAbilityFirstFrameStateObserver : public AbilityFirstFrameStateObserverStub {
public:
    MockAbilityFirstFrameStateObserver() = default;
    ~MockAbilityFirstFrameStateObserver() = default;
    void OnAbilityFirstFrameState(const AbilityFirstFrameStateData& abilityFirstFrameStateData) override
    {}
    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        code_ = code;
        return 0;
    }
    uint32_t code_ = 0;
};

class AbilityFirstFrameStateObserverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityFirstFrameStateObserverManagerTest::SetUpTestCase()
{}

void AbilityFirstFrameStateObserverManagerTest::TearDownTestCase()
{}

void AbilityFirstFrameStateObserverManagerTest::SetUp()
{}

void AbilityFirstFrameStateObserverManagerTest::TearDown()
{}

/*
 * Feature: AbilityFirstFrameStateObserverManager
 * Function: RegisterAbilityFirstFrameStateObserver
 * SubFunction: NA
 * FunctionPoints: AbilityFirstFrameStateObserverManager RegisterAbilityFirstFrameStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAbilityFirstFrameStateObserver
 */
HWTEST_F(AbilityFirstFrameStateObserverManagerTest, RegisterAbilityFirstFrameStateObserver_001, TestSize.Level0)
{
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->Init();
    std::string bundleName = "com.example.test";
    int32_t res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(nullptr, bundleName);
    EXPECT_EQ(res, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: AbilityFirstFrameStateObserverManager
 * Function: RegisterAbilityFirstFrameStateObserver
 * SubFunction: NA
 * FunctionPoints: AbilityFirstFrameStateObserverManager RegisterAbilityFirstFrameStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAbilityFirstFrameStateObserver
 */
HWTEST_F(AbilityFirstFrameStateObserverManagerTest, RegisterAbilityFirstFrameStateObserver_002, TestSize.Level0)
{
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->Init();
    std::string bundleName = "com.example.test";
    MyFlag::isSystemApp_ = true;
    MyFlag::flag_ = ERR_PERMISSION_DENIED;
    int32_t res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(nullptr, bundleName);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AbilityFirstFrameStateObserverManager
 * Function: RegisterAbilityFirstFrameStateObserver
 * SubFunction: NA
 * FunctionPoints: AbilityFirstFrameStateObserverManager RegisterAbilityFirstFrameStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAbilityFirstFrameStateObserver
 */
HWTEST_F(AbilityFirstFrameStateObserverManagerTest, RegisterAbilityFirstFrameStateObserver_003, TestSize.Level0)
{
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->Init();
    std::string bundleName = "";
    MyFlag::isSystemApp_ = true;
    MyFlag::flag_ = ERR_OK;
    int32_t res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(nullptr, bundleName);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AbilityFirstFrameStateObserverManager
 * Function: RegisterAbilityFirstFrameStateObserver
 * SubFunction: NA
 * FunctionPoints: AbilityFirstFrameStateObserverManager RegisterAbilityFirstFrameStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAbilityFirstFrameStateObserver
 */
HWTEST_F(AbilityFirstFrameStateObserverManagerTest, RegisterAbilityFirstFrameStateObserver_004, TestSize.Level0)
{
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->Init();
    std::string bundleName = "";
    MyFlag::isSystemApp_ = true;
    MyFlag::flag_ = ERR_OK;
    // step1. register not null remoteObj,expect success.
    sptr<MockAbilityFirstFrameStateObserver> remoteObject = new MockAbilityFirstFrameStateObserver();
    int32_t res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(remoteObject, bundleName);
    EXPECT_EQ(res, ERR_OK);
    // step2. duplicate register not null remoteObj,expect success.
    res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(remoteObject, bundleName);
    EXPECT_EQ(res, ERR_OK);
    auto observerMap = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->observerMap_;
    auto recipientMap = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->recipientMap_;
    // step3. duplicate register same remoteObj, expect only one observer register success.
    EXPECT_EQ(observerMap.size(), 1); // 1 means: observer num
    EXPECT_EQ(recipientMap.size(), 1); // 1 means: death recipient num
    // step4. unregister remoteObject, expect unregister success.
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        UnregisterAbilityFirstFrameStateObserver(remoteObject);
    auto observerMap2 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->observerMap_;
    auto recipientMap2 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->recipientMap_;
    EXPECT_EQ(observerMap2.size(), 0);
    EXPECT_EQ(recipientMap2.size(), 0);
}

/*
 * Feature: AbilityFirstFrameStateObserverManager
 * Function: RegisterAbilityFirstFrameStateObserver
 * SubFunction: NA
 * FunctionPoints: AbilityFirstFrameStateObserverManager RegisterAbilityFirstFrameStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAbilityFirstFrameStateObserver
 */
HWTEST_F(AbilityFirstFrameStateObserverManagerTest, RegisterAbilityFirstFrameStateObserver_005, TestSize.Level0)
{
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->Init();
    std::string bundleName = "com.example.test";
    MyFlag::isSystemApp_ = true;
    MyFlag::flag_ = ERR_OK;
    // step1. register not null remoteObj,expect success.
    sptr<MockAbilityFirstFrameStateObserver> remoteObject = new MockAbilityFirstFrameStateObserver();
    int32_t res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(remoteObject, bundleName);
    EXPECT_EQ(res, ERR_OK);
    // step2. duplicate register not null remoteObj,expect success.
    res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(remoteObject, bundleName);
    EXPECT_EQ(res, ERR_OK);
    auto observerMap = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForBundleName_->observerMap_;
    auto recipientMap = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForBundleName_->recipientMap_;
    // step3. duplicate register same remoteObj, expect only one observer register success.
    EXPECT_EQ(observerMap.size(), 1); // 1 means observerMap size.
    EXPECT_EQ(recipientMap.size(), 1); // 1 means recipientMap size.
    for (auto it : observerMap) {
        EXPECT_EQ(it.second, bundleName);
    }
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        UnregisterAbilityFirstFrameStateObserver(remoteObject);
    // step4. unregister remoteObject, expect unregister success.
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        UnregisterAbilityFirstFrameStateObserver(remoteObject);
    auto observerMap2 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->observerMap_;
    auto recipientMap2 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->recipientMap_;
    EXPECT_EQ(observerMap2.size(), 0);
    EXPECT_EQ(recipientMap2.size(), 0);
}

/*
 * Feature: AbilityFirstFrameStateObserverManager
 * Function: HandleOnFirstFrameState
 * SubFunction: NA
 * FunctionPoints: AbilityFirstFrameStateObserverManager HandleOnFirstFrameState
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnFirstFrameState
 */
HWTEST_F(AbilityFirstFrameStateObserverManagerTest, HandleOnFirstFrameState_001, TestSize.Level0)
{
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->Init();
    std::string bundleName = "com.example.test";
    MyFlag::isSystemApp_ = true;
    MyFlag::flag_ = ERR_OK;
    // step1. register not null remoteObj, expect register observer success.
    sptr<MockAbilityFirstFrameStateObserver> remoteObject1 = new MockAbilityFirstFrameStateObserver();
    sptr<MockAbilityFirstFrameStateObserver> remoteObject2 = new MockAbilityFirstFrameStateObserver();
    EXPECT_CALL(*remoteObject1, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(remoteObject1.GetRefPtr(), &MockAbilityFirstFrameStateObserver::InvokeSendRequest));
    EXPECT_CALL(*remoteObject2, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(remoteObject1.GetRefPtr(), &MockAbilityFirstFrameStateObserver::InvokeSendRequest));
    int32_t res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(remoteObject1, bundleName);
    res = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        RegisterAbilityFirstFrameStateObserver(remoteObject2, "");
    auto observerMap1 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForBundleName_->observerMap_;
    auto recipientMap1 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForBundleName_->recipientMap_;
    auto observerMap2 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->observerMap_;
    auto recipientMap2 = DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        stateObserverSetForAllBundles_->recipientMap_;
    EXPECT_EQ(observerMap1.size(), 1); // 1 means observerMap size.
    EXPECT_EQ(recipientMap1.size(), 1); // 1 means recipientMap size.
    EXPECT_EQ(observerMap2.size(), 1); // 1 means observerMap size.
    EXPECT_EQ(recipientMap2.size(), 1); // 1 means recipientMap size.
    Want want;
    AppExecFwk::AbilityInfo info;
    AppExecFwk::ApplicationInfo appInfo;
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(want, info, appInfo);
    DelayedSingleton<AppExecFwk::AbilityFirstFrameStateObserverManager>::GetInstance()->
        HandleOnFirstFrameState(abilityRecord);
    EXPECT_EQ(remoteObject1->code_,
        static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE));
    EXPECT_EQ(remoteObject2->code_,
        static_cast<uint32_t>(IAbilityFirstFrameStateObserver::Message::ON_ABILITY_FIRST_FRAME_STATE));
}
} // namespace AppExecFwk
} // namespace OHOS
