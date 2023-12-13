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

#include "mock_permission_verification.h"

#define private public
#include "app_state_observer_manager.h"
#undef private
#include "app_foreground_state_observer_stub.h"
#include "application_state_observer_stub.h"
#include "mock_i_remote_object.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int BUNDLE_NAME_LIST_MAX_SIZE = 128;
}
class MockApplicationStateObserver : public IApplicationStateObserver {
public:
    MockApplicationStateObserver() = default;
    virtual ~MockApplicationStateObserver() = default;
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override
    {}
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override
    {}
    void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override
    {}
    void OnProcessCreated(const ProcessData &processData) override
    {}
    void OnProcessStateChanged(const ProcessData &processData) override
    {}
    void OnProcessDied(const ProcessData &processData) override
    {}
    void OnApplicationStateChanged(const AppStateData &appStateData) override
    {}
    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
    void OnAppStarted(const AppStateData &appStateData) override
    {}
    void OnAppStopped(const AppStateData &appStateData) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
class AppForegroundStateObserver : public AppForegroundStateObserverStub {
public:    
    AppForegroundStateObserver() = default;
    virtual ~AppForegroundStateObserver() = default;
    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
};
class AppSpawnSocketTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AppRunningRecord> MockAppRecord();
    sptr<IApplicationStateObserver> observer_ {nullptr};
};

void AppSpawnSocketTest::SetUpTestCase()
{}

void AppSpawnSocketTest::TearDownTestCase()
{}

void AppSpawnSocketTest::SetUp()
{
    sptr<IApplicationStateObserver> observer_ = new MockApplicationStateObserver();
}

void AppSpawnSocketTest::TearDown()
{}

std::shared_ptr<AppRunningRecord> AppSpawnSocketTest::MockAppRecord()
{
    ApplicationInfo appInfo;
    appInfo.accessTokenId = 1;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>(appInfo);
    info->accessTokenId = 1;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, 0, "process");
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    priorityObject->SetPid(1);
    appRecord->priorityObject_ = priorityObject;
    appRecord->SetUid(1);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appRecord->SetContinuousTaskAppState(false);
    appRecord->SetKeepAliveAppState(false, false);
    appRecord->SetRequestProcCode(1);
    appRecord->isFocused_ = false;
    return appRecord;
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager RegisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterApplicationStateObserver
 */
HWTEST_F(AppSpawnSocketTest, RegisterApplicationStateObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    vector<std::string> bundleNameList;
    int32_t res = manager->RegisterApplicationStateObserver(nullptr, bundleNameList);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager RegisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterApplicationStateObserver
 */
HWTEST_F(AppSpawnSocketTest, RegisterApplicationStateObserver_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    vector<std::string> bundleNameList;
    while (bundleNameList.size() <= BUNDLE_NAME_LIST_MAX_SIZE) {
        bundleNameList.push_back("a");
    }
    int32_t res = manager->RegisterApplicationStateObserver(nullptr, bundleNameList);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: UnregisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager UnregisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterApplicationStateObserver
 */
HWTEST_F(AppSpawnSocketTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    int32_t res = manager->UnregisterApplicationStateObserver(nullptr);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, OnAppStarted_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnAppStarted(appRecord);
    manager->Init();
    manager->OnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, OnAppStopped_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnAppStopped(appRecord);
    manager->Init();
    manager->OnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnAppStateChanged_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    manager->OnAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnAppStateChanged_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    manager->Init();
    manager->OnAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, OnProcessDied_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessDied(appRecord);
    manager->Init();
    manager->OnProcessDied(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnRenderProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnRenderProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify OnRenderProcessDied
 */
HWTEST_F(AppSpawnSocketTest, OnRenderProcessDied_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<RenderRecord> renderRecord;
    manager->OnRenderProcessDied(renderRecord);
    manager->Init();
    manager->OnRenderProcessDied(renderRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, OnProcessStateChanged_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessStateChanged(appRecord);
    manager->Init();
    manager->OnProcessStateChanged(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, OnProcessCreated_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    manager->OnProcessCreated(appRecord);
    manager->Init();
    manager->OnProcessCreated(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnRenderProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnRenderProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify OnRenderProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, OnRenderProcessCreated_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<RenderRecord> renderRecord;
    manager->OnRenderProcessCreated(renderRecord);
    manager->Init();
    manager->OnRenderProcessCreated(renderRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: StateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager StateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify StateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, StateChangedNotifyObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    manager->StateChangedNotifyObserver(abilityStateData, isAbility, false);
    manager->Init();
    manager->StateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnAppStarted(nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStarted_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleOnAppStarted(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnAppStopped(nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStopped
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStopped
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStopped
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppStopped_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleOnAppStopped(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_FOREGROUND;
    bool needNotifyApp = true;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_BACKGROUND;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_BACKGROUND;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_005, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_TERMINATED;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_006, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_007, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleAppStateChanged_008, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_END;
    bool needNotifyApp = false;
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = true;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    abilityStateData.bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppSpawnSocketTest, HandleStateChangedNotifyObserver_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility, false);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppProcessCreated_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnAppProcessCreated(nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    manager->HandleOnAppProcessCreated(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnRenderProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnRenderProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnRenderProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnRenderProcessCreated_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnRenderProcessCreated(nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::shared_ptr<RenderRecord> renderRecord =
        std::make_shared<RenderRecord>(1, "param", 1, 1, 1, appRecord);
    renderRecord->SetPid(1);
    manager->HandleOnRenderProcessCreated(renderRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data.bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest";
    std::string bundleName2 = "com.ohos.unittest";
    data.bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessCreated
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessCreated_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleOnProcessCreated(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->HandleOnProcessStateChanged(nullptr);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessStateChanged(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessStateChanged(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    appRecord->mainBundleName_ = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleOnProcessStateChanged(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessStateChanged
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessStateChanged_005, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    appRecord->mainBundleName_ = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleOnProcessStateChanged(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnAppProcessDied_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    manager->HandleOnAppProcessDied(nullptr);
    manager->HandleOnAppProcessDied(appRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnRenderProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnRenderProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnRenderProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnRenderProcessDied_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    std::shared_ptr<RenderRecord> renderRecord =
        std::make_shared<RenderRecord>(1, "param", 1, 1, 1, appRecord);
    renderRecord->SetPid(1);
    manager->HandleOnRenderProcessDied(nullptr);
    manager->HandleOnRenderProcessDied(renderRecord);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data.bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    data.bundleName = bundleName;
    bundleNameList.push_back(bundleName);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_003, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    data.bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnProcessDied
 */
HWTEST_F(AppSpawnSocketTest, HandleOnProcessDied_004, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    ProcessData data;
    std::vector<std::string> bundleNameList;
    std::string bundleName1 = "com.ohos.unittest1";
    std::string bundleName2 = "com.ohos.unittest2";
    data.bundleName = bundleName1;
    bundleNameList.push_back(bundleName2);
    manager->appStateObserverMap_.emplace(nullptr, bundleNameList);
    manager->HandleOnProcessDied(data);
}

/*
 * Feature: AppStateObserverManager
 * Function: ObserverExist
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager ObserverExist
 * EnvConditions: NA
 * CaseDescription: Verify ObserverExist
 */
HWTEST_F(AppSpawnSocketTest, ObserverExist_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    bool res = manager->ObserverExist(nullptr);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppStateObserverManager
 * Function: ObserverExist
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager ObserverExist
 * EnvConditions: NA
 * CaseDescription: Verify ObserverExist
 */
HWTEST_F(AppSpawnSocketTest, ObserverExist_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IApplicationStateObserver> observer = new MockApplicationStateObserver();
    std::vector<std::string> bundleNameList;
    manager->appStateObserverMap_.emplace(observer, bundleNameList);
    bool res = manager->ObserverExist(observer);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: RegisterAppForegroundStateObserver_0100
 * @tc.desc: Test when observer is not nullptr and permission verification success 
 *      and observer not exist.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, RegisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserver();    
    auto res = manager->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_OK, res);
}

/**
 * @tc.name: UnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test when observer is not nullptr and permission verification success.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, UnregisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserver();    
    manager->appForegroundStateObserverSet_.emplace(observer);
    auto res = manager->UnregisterAppForegroundStateObserver(observer);
    EXPECT_EQ(ERR_OK, res); 
}

/**
 * @tc.name: IsAppForegroundObserverExist_0100
 * @tc.desc: Test when observer and appForegroundStateObserverSet is not nullptr.
 *      and asObject of them is same.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, IsAppForegroundObserverExist_0100, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>(); 
    sptr<IRemoteBroker> observer = new (std::nothrow) AppForegroundStateObserver();
    sptr<IAppForegroundStateObserver> appForegroundStateObserver = new (std::nothrow) AppForegroundStateObserver();
    manager->appForegroundStateObserverSet_.emplace(appForegroundStateObserver);
    auto res = manager->IsAppForegroundObserverExist(observer);
    EXPECT_EQ(true, res);    
}

/**
 * @tc.name: OnObserverDied_0100
 * @tc.desc: Test when observer is not nullptr and type is APPLICATION_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0100, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::APPLICATION_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto appStateObserverMapSize = DelayedSingleton<AppStateObserverManager>::
        GetInstance()->appStateObserverMap_.size();
    EXPECT_EQ(0, appStateObserverMapSize);
}

/**
 * @tc.name: OnObserverDied_0200
 * @tc.desc: Test when observer is not nullptr and type is ABILITY_FOREGROUND_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0200, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto abilityforegroundObserverSetSize = DelayedSingleton<AppStateObserverManager>::
        GetInstance()->abilityforegroundObserverSet_.size();
    EXPECT_EQ(0, abilityforegroundObserverSetSize);
}

/**
 * @tc.name: OnObserverDied_0300
 * @tc.desc: Test when observer is not nullptr and type is ABILITY_FOREGROUND_STATE_OBSERVER.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnSocketTest, OnObserverDied_0300, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) AppForegroundStateObserver();
    wptr<IRemoteObject> remote(remoteObject);
    ObserverType type = ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER;
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnObserverDied(remote, type);
    auto appForegroundStateObserverSetSize = DelayedSingleton<AppStateObserverManager>::
        GetInstance()->appForegroundStateObserverSet_.size();
    EXPECT_EQ(0, appForegroundStateObserverSetSize);
}
}  // namespace AppExecFwk
}  // namespace OHOS
