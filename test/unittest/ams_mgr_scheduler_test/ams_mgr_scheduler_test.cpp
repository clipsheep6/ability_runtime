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

#define private public
#include "ams_mgr_scheduler.h"
#undef private

#include "app_state_callback_host.h"
#include "hilog_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service_inner.h"
#include "application_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Return;

namespace OHOS {
namespace AppExecFwk {
class AmsMgrSchedulerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
public:
protected:
    static const std::string GetTestAppName()
    {
        return "test_app_name";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }

    std::shared_ptr<MockAppMgrServiceInner> GetMockAppMgrServiceInner();
    std::shared_ptr<AMSEventHandler> GetAmsEventHandler();

private:
    std::shared_ptr<MockAppMgrServiceInner> mockAppMgrServiceInner_;
    std::shared_ptr<AMSEventHandler> amsEventHandler_;
};

void AmsMgrSchedulerTest::SetUpTestCase()
{}

void AmsMgrSchedulerTest::TearDownTestCase()
{}

void AmsMgrSchedulerTest::SetUp()
{}

void AmsMgrSchedulerTest::TearDown()
{
    amsEventHandler_.reset();
    mockAppMgrServiceInner_.reset();
}

std::shared_ptr<MockAppMgrServiceInner> AmsMgrSchedulerTest::GetMockAppMgrServiceInner()
{
    if (!mockAppMgrServiceInner_) {
        mockAppMgrServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    }
    return mockAppMgrServiceInner_;
}

std::shared_ptr<AMSEventHandler> AmsMgrSchedulerTest::GetAmsEventHandler()
{
    if (!amsEventHandler_) {
        auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
        amsEventHandler_ =
            std::make_shared<AMSEventHandler>(EventRunner::Create("AmsMgrSchedulerTest"), mockAppMgrServiceInner);
    }
    return amsEventHandler_;
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: LoadAbility
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function LoadAbility can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_001, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_001 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);

    sptr<IRemoteObject> token = new MockAbilityToken();
    sptr<IRemoteObject> preToken = new MockAbilityToken();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = GetTestAppName();

    EXPECT_CALL(*mockAppMgrServiceInner, LoadAbility(_, _, _, _, _))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->LoadAbility(token, preToken, abilityInfo, applicationInfo, nullptr);
    mockAppMgrServiceInner->Wait();

    HILOG_DEBUG("AmsMgrScheduler_001 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: LoadAbility
 * FunctionPoints: Check params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function LoadAbility can check appInfo and abilityInfo.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_002, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_002 start.");

    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    auto eventRunner = EventRunner::Create("AmsMgrSchedulerTest");
    auto amsEventHandler = std::make_shared<AMSEventHandler>(eventRunner, mockAppMgrServiceInner);
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);

    sptr<IRemoteObject> token = new MockAbilityToken();
    sptr<IRemoteObject> preToken = new MockAbilityToken();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    std::shared_ptr<ApplicationInfo> applicationInfo = std::make_shared<ApplicationInfo>();
    applicationInfo->name = GetTestAppName();

    // check token parameter
    EXPECT_CALL(*mockAppMgrServiceInner, LoadAbility(_, _, _, _, _)).Times(0);
    amsMgrScheduler->LoadAbility(token, preToken, nullptr, applicationInfo, nullptr);

    // check pretoken parameter
    EXPECT_CALL(*mockAppMgrServiceInner, LoadAbility(_, _, _, _, _)).Times(0);
    amsMgrScheduler->LoadAbility(token, preToken, abilityInfo, nullptr, nullptr);

    HILOG_DEBUG("AmsMgrScheduler_002 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: UpdateAbilityState
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function UpdateAbilityState can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_003, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_003 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);

    sptr<IRemoteObject> token = new MockAbilityToken();
    AbilityState abilityState = AbilityState::ABILITY_STATE_CREATE;

    EXPECT_CALL(*mockAppMgrServiceInner, UpdateAbilityState(_, _))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->UpdateAbilityState(token, abilityState);
    mockAppMgrServiceInner->Wait();

    HILOG_DEBUG("AmsMgrScheduler_003 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: TerminateAbility
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function TerminateAbility can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_004, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_004 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);
    sptr<IRemoteObject> token = new MockAbilityToken();
    bool clearMissionFlag = true;
    EXPECT_CALL(*mockAppMgrServiceInner, TerminateAbility(_, _))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
    mockAppMgrServiceInner->Wait();

    HILOG_DEBUG("AmsMgrScheduler_004 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: RegisterAppStateCallback
 * FunctionPoints: Act normal
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function RegisterAppStateCallback can works.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_005, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_005 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);

    sptr<AppStateCallbackHost> appStateCallbackHost = new AppStateCallbackHost();
    EXPECT_CALL(*mockAppMgrServiceInner, RegisterAppStateCallback(_))
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));
    amsMgrScheduler->RegisterAppStateCallback(appStateCallbackHost);
    mockAppMgrServiceInner->Wait();

    HILOG_DEBUG("AmsMgrScheduler_005 end.");
}

/*
 * Feature: AMS
 * Function: AmsMgrScheduler
 * SubFunction: IsReady
 * FunctionPoints: Check Params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Verify the function IsReady can check params.
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_007, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_007 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();

    // act normal
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);
    EXPECT_EQ(true, amsMgrScheduler->IsReady());

    // check params AppMgrServiceInner
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler2 = std::make_unique<AmsMgrScheduler>(nullptr, amsEventHandler);
    EXPECT_EQ(false, amsMgrScheduler2->IsReady());

    // check params AMSEventHandler
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler3 =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, nullptr);
    EXPECT_EQ(false, amsMgrScheduler3->IsReady());

    HILOG_DEBUG("AmsMgrScheduler_007 end.");
}

/*
 * Feature: AMS
 * Function: KillApplication
 * SubFunction: IsReady
 * FunctionPoints: Check Params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Kill apps by name
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_008, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_008 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();

    EXPECT_CALL(*mockAppMgrServiceInner, KillApplication(_)).Times(1).WillOnce(Return(ERR_OK));

    // check params AppMgrServiceInner
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler2 = std::make_unique<AmsMgrScheduler>(nullptr, amsEventHandler);
    EXPECT_EQ(false, amsMgrScheduler2->IsReady());

    EXPECT_EQ(ERR_INVALID_OPERATION, amsMgrScheduler2->KillApplication(GetTestAppName()));

    // check params AMSEventHandler
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler3 =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, nullptr);
    EXPECT_EQ(false, amsMgrScheduler3->IsReady());

    EXPECT_EQ(ERR_INVALID_OPERATION, amsMgrScheduler3->KillApplication(GetTestAppName()));

    // act normal
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler4 =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);
    EXPECT_EQ(true, amsMgrScheduler4->IsReady());

    EXPECT_EQ(ERR_OK, amsMgrScheduler4->KillApplication(GetTestAppName()));

    HILOG_DEBUG("AmsMgrScheduler_008 end.");
}

/*
 * Feature: AMS
 * Function: AbilityBehaviorAnalysis
 * SubFunction: IsReady
 * FunctionPoints: Check Params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Optimize based on visibility and perception
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_009, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_009 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);
    EXPECT_EQ(true, amsMgrScheduler->IsReady());

    EXPECT_CALL(*mockAppMgrServiceInner, AbilityBehaviorAnalysis(_, _, _, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockAppMgrServiceInner.get(), &MockAppMgrServiceInner::Post));

    sptr<IRemoteObject> token;
    sptr<IRemoteObject> preToken;
    int32_t visibility = 0;
    int32_t perceptibility = 0;
    int32_t connectionState = 0;

    amsMgrScheduler->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState);

    mockAppMgrServiceInner->Wait();

    mockAppMgrServiceInner.reset();
    amsEventHandler.reset();

    HILOG_DEBUG("AmsMgrScheduler_009 end.");
}

/*
 * Feature: AMS
 * Function: AbilityBehaviorAnalysis
 * SubFunction: IsReady
 * FunctionPoints: Check Params
 * EnvConditions: Mobile that can run ohos test framework.
 * CaseDescription: Optimize based on visibility and perception
 */
HWTEST_F(AmsMgrSchedulerTest, AmsMgrScheduler_010, TestSize.Level1)
{
    HILOG_DEBUG("AmsMgrScheduler_010 start.");

    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();

    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    EXPECT_EQ(false, amsMgrScheduler->IsReady());

    EXPECT_CALL(*mockAppMgrServiceInner, AbilityBehaviorAnalysis(_, _, _, _, _)).Times(0);

    sptr<IRemoteObject> token;
    sptr<IRemoteObject> preToken;
    int32_t visibility = 0;
    int32_t perceptibility = 0;
    int32_t connectionState = 0;

    amsMgrScheduler->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState);

    HILOG_DEBUG("AmsMgrScheduler_010 end.");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: KillApplication interface
 * CaseDescription: test IPC can transact data
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterApplicationStateObserver_001, TestSize.Level0)
{
    HILOG_DEBUG("RegisterApplicationStateObserver_001 start");
    sptr<IApplicationStateObserver> observer = new ApplicationStateObserverStub();
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    mockAppMgrServiceInner->RegisterApplicationStateObserver(observer);
    int32_t err = mockAppMgrServiceInner->RegisterApplicationStateObserver(observer);
    // repeat register return ERR_INVALID_VALUE
    EXPECT_EQ(1, err);
    HILOG_DEBUG("RegisterApplicationStateObserver_001 end");
}

/*
 * Feature: AMS
 * Function: IPC
 * SubFunction: appmgr interface
 * FunctionPoints: KillApplication interface
 * CaseDescription: test IPC can transact data
 */
HWTEST_F(AmsMgrSchedulerTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    HILOG_DEBUG("UnregisterApplicationStateObserver_001 start");
    sptr<IApplicationStateObserver> observer = new ApplicationStateObserverStub();
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    int32_t err1 = mockAppMgrServiceInner->UnregisterApplicationStateObserver(observer);
    // unregister not exist return ERR_INVALID_VALUE
    EXPECT_EQ(1, err1);
    int32_t err2 = mockAppMgrServiceInner->UnregisterApplicationStateObserver(nullptr);
    // unregister null return ERR_INVALID_VALUE
    EXPECT_EQ(1, err2);
    HILOG_DEBUG("UnregisterApplicationStateObserver_001 end");
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = nullptr;
    amsMgrScheduler->LoadAbility(token, preToken, abilityInfo, appInfo, want);
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_002, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::shared_ptr<Want> want = nullptr;
    amsMgrScheduler->LoadAbility(token, preToken, abilityInfo, appInfo, want);
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_003, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_shared<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = nullptr;
    amsMgrScheduler->LoadAbility(token, preToken, abilityInfo, appInfo, want);
}

/*
 * Feature: AmsMgrScheduler
 * Function: LoadAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler LoadAbility
 * EnvConditions: NA
 * CaseDescription: Verify LoadAbility
 */
HWTEST_F(AmsMgrSchedulerTest, LoadAbility_004, TestSize.Level0)
{
    auto mockAppMgrServiceInner = GetMockAppMgrServiceInner();
    auto amsEventHandler = GetAmsEventHandler();
    std::unique_ptr<AmsMgrScheduler> amsMgrScheduler =
        std::make_unique<AmsMgrScheduler>(mockAppMgrServiceInner, amsEventHandler);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<Want> want = nullptr;
    amsMgrScheduler->LoadAbility(token, preToken, abilityInfo, appInfo, want);
}

/*
 * Feature: AmsMgrScheduler
 * Function: UpdateAbilityState
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler UpdateAbilityState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateAbilityState
 */
HWTEST_F(AmsMgrSchedulerTest, UpdateAbilityState_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    AbilityState state = AbilityState::ABILITY_STATE_READY;
    amsMgrScheduler->UpdateAbilityState(token, state);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->UpdateAbilityState(token, state);
}

/*
 * Feature: AmsMgrScheduler
 * Function: UpdateExtensionState
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler UpdateExtensionState
 * EnvConditions: NA
 * CaseDescription: Verify UpdateExtensionState
 */
HWTEST_F(AmsMgrSchedulerTest, UpdateExtensionState_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    ExtensionState state = ExtensionState::EXTENSION_STATE_READY;
    amsMgrScheduler->UpdateExtensionState(token, state);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->UpdateExtensionState(token, state);
}

/*
 * Feature: AmsMgrScheduler
 * Function: TerminateAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler TerminateAbility
 * EnvConditions: NA
 * CaseDescription: Verify TerminateAbility
 */
HWTEST_F(AmsMgrSchedulerTest, TerminateAbility_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    bool clearMissionFlag = true;
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->TerminateAbility(token, clearMissionFlag);
}

/*
 * Feature: AmsMgrScheduler
 * Function: RegisterAppStateCallback
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler RegisterAppStateCallback
 * EnvConditions: NA
 * CaseDescription: Verify RegisterAppStateCallback
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterAppStateCallback_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IAppStateCallback> callback = nullptr;
    amsMgrScheduler->RegisterAppStateCallback(callback);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->RegisterAppStateCallback(callback);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AbilityBehaviorAnalysis
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AbilityBehaviorAnalysis
 * EnvConditions: NA
 * CaseDescription: Verify AbilityBehaviorAnalysis
 */
HWTEST_F(AmsMgrSchedulerTest, AbilityBehaviorAnalysis_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> preToken = nullptr;
    int32_t visibility = 0;
    int32_t perceptibility = 0;
    int32_t connectionState = 0;
    amsMgrScheduler->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->AbilityBehaviorAnalysis(token, preToken, visibility, perceptibility, connectionState);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessesByUserId
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessesByUserId
 * EnvConditions: NA
 * CaseDescription: Verify KillProcessesByUserId
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessesByUserId_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    int32_t userId = 0;
    amsMgrScheduler->KillProcessesByUserId(userId);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillProcessWithAccount
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillProcessWithAccount
 * EnvConditions: NA
 * CaseDescription: Verify KillProcessWithAccount
 */
HWTEST_F(AmsMgrSchedulerTest, KillProcessWithAccount_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    std::string bundleName = "bundleName";
    int accountId = 0;
    int32_t res1 = amsMgrScheduler->KillProcessWithAccount(bundleName, accountId);
    EXPECT_EQ(res1, ERR_INVALID_OPERATION);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    int32_t res2 = amsMgrScheduler->KillProcessWithAccount(bundleName, accountId);
    EXPECT_NE(res2, ERR_INVALID_OPERATION);
}

/*
 * Feature: AmsMgrScheduler
 * Function: AbilityAttachTimeOut
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler AbilityAttachTimeOut
 * EnvConditions: NA
 * CaseDescription: Verify AbilityAttachTimeOut
 */
HWTEST_F(AmsMgrSchedulerTest, AbilityAttachTimeOut_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->AbilityAttachTimeOut(token);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->AbilityAttachTimeOut(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: PrepareTerminate
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler PrepareTerminate
 * EnvConditions: NA
 * CaseDescription: Verify PrepareTerminate
 */
HWTEST_F(AmsMgrSchedulerTest, PrepareTerminate_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    amsMgrScheduler->PrepareTerminate(token);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->PrepareTerminate(token);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplication
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplication
 * EnvConditions: NA
 * CaseDescription: Verify KillApplication
 */
HWTEST_F(AmsMgrSchedulerTest, KillApplication_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    std::string bundleName = "bundleName";
    amsMgrScheduler->KillApplication(bundleName);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->KillApplication(bundleName);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplicationByUid
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplicationByUid
 * EnvConditions: NA
 * CaseDescription: Verify KillApplicationByUid
 */
HWTEST_F(AmsMgrSchedulerTest, KillApplicationByUid_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    std::string bundleName = "bundleName";
    int uid = 0;
    amsMgrScheduler->KillApplicationByUid(bundleName, uid);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->KillApplicationByUid(bundleName, uid);
}

/*
 * Feature: AmsMgrScheduler
 * Function: KillApplicationSelf
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler KillApplicationSelf
 * EnvConditions: NA
 * CaseDescription: Verify KillApplicationSelf
 */
HWTEST_F(AmsMgrSchedulerTest, KillApplicationSelf_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    amsMgrScheduler->KillApplicationSelf();
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->KillApplicationSelf();
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetRunningProcessInfoByToken
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInfoByToken
 */
HWTEST_F(AmsMgrSchedulerTest, GetRunningProcessInfoByToken_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    RunningProcessInfo info;
    amsMgrScheduler->GetRunningProcessInfoByToken(token, info);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->GetRunningProcessInfoByToken(token, info);
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetRunningProcessInfoByPid
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetRunningProcessInfoByPid
 * EnvConditions: NA
 * CaseDescription: Verify GetRunningProcessInfoByPid
 */
HWTEST_F(AmsMgrSchedulerTest, GetRunningProcessInfoByPid_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    pid_t pid = 0;
    RunningProcessInfo info;
    amsMgrScheduler->GetRunningProcessInfoByPid(pid, info);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->GetRunningProcessInfoByPid(pid, info);
}

/*
 * Feature: AmsMgrScheduler
 * Function: StartSpecifiedAbility
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler StartSpecifiedAbility
 * EnvConditions: NA
 * CaseDescription: Verify StartSpecifiedAbility
 */
HWTEST_F(AmsMgrSchedulerTest, StartSpecifiedAbility_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    Want want;
    AbilityInfo abilityInfo;
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->StartSpecifiedAbility(want, abilityInfo);
}

/*
 * Feature: AmsMgrScheduler
 * Function: RegisterStartSpecifiedAbilityResponse
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler RegisterStartSpecifiedAbilityResponse
 * EnvConditions: NA
 * CaseDescription: Verify RegisterStartSpecifiedAbilityResponse
 */
HWTEST_F(AmsMgrSchedulerTest, RegisterStartSpecifiedAbilityResponse_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    sptr<IStartSpecifiedAbilityResponse> response = nullptr;
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    amsMgrScheduler->RegisterStartSpecifiedAbilityResponse(response);
}

/*
 * Feature: AmsMgrScheduler
 * Function: GetApplicationInfoByProcessID
 * SubFunction: NA
 * FunctionPoints: AmsMgrScheduler GetApplicationInfoByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetApplicationInfoByProcessID
 */
HWTEST_F(AmsMgrSchedulerTest, GetApplicationInfoByProcessID_001, TestSize.Level0)
{
    auto amsMgrScheduler = std::make_unique<AmsMgrScheduler>(nullptr, nullptr);
    int pid = 0;
    ApplicationInfo application;
    bool debug = true;
    int res1 = amsMgrScheduler->GetApplicationInfoByProcessID(pid, application, debug);
    EXPECT_EQ(res1, ERR_INVALID_OPERATION);
    amsMgrScheduler->amsMgrServiceInner_ = GetMockAppMgrServiceInner();
    amsMgrScheduler->amsHandler_ = GetAmsEventHandler();
    int res2 = amsMgrScheduler->GetApplicationInfoByProcessID(pid, application, debug);
    EXPECT_NE(res2, ERR_INVALID_OPERATION);
}
}  // namespace AppExecFwk
}  // namespace OHOS
