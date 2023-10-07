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
#include "app_state_observer_manager.h"
#undef private
#include "application_state_observer_stub.h"
#include "hilog_wrapper.h"

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
    void OnPageShow(const PageStateData &PageStateData) override
    {
        HILOG_ERROR("[DongLin] on page show");
    }
    void OnPageHide(const PageStateData &PageStateData) override
    {
        HILOG_ERROR("[DongLin] on page hide");
    }
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
class AppStateObserverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AppRunningRecord> MockAppRecord();
    sptr<IApplicationStateObserver> observer_ {nullptr};
};

void AppStateObserverManagerTest::SetUpTestCase()
{}

void AppStateObserverManagerTest::TearDownTestCase()
{}

void AppStateObserverManagerTest::SetUp()
{
    sptr<IApplicationStateObserver> observer_ = new MockApplicationStateObserver();
}

void AppStateObserverManagerTest::TearDown()
{}

std::shared_ptr<AppRunningRecord> AppStateObserverManagerTest::MockAppRecord()
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
HWTEST_F(AppStateObserverManagerTest, RegisterApplicationStateObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    vector<std::string> bundleNameList;
    int32_t res = manager->RegisterApplicationStateObserver(nullptr, bundleNameList);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager RegisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterApplicationStateObserver
 */
HWTEST_F(AppStateObserverManagerTest, RegisterApplicationStateObserver_002, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    int32_t res = manager->UnregisterApplicationStateObserver(nullptr);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStarted
 */
HWTEST_F(AppStateObserverManagerTest, OnAppStarted_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, OnAppStopped_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, OnAppStateChanged_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    manager->OnAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, OnAppStateChanged_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord;
    ApplicationState state = ApplicationState::APP_STATE_CREATE;
    bool needNotifyApp = false;
    manager->Init();
    manager->OnAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnProcessDied
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager OnProcessDied
 * EnvConditions: NA
 * CaseDescription: Verify OnProcessDied
 */
HWTEST_F(AppStateObserverManagerTest, OnProcessDied_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, OnRenderProcessDied_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, OnProcessStateChanged_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, OnProcessCreated_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, OnRenderProcessCreated_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, StateChangedNotifyObserver_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    manager->StateChangedNotifyObserver(abilityStateData, isAbility);
    manager->Init();
    manager->StateChangedNotifyObserver(abilityStateData, isAbility);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppStarted
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppStarted
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppStarted
 */
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStarted_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStarted_002, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStarted_003, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStarted_004, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStopped_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStopped_002, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStopped_003, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppStopped_004, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_001, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_002, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_003, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_004, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_005, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_006, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_007, TestSize.Level0)
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
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleAppStateChanged
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleAppStateChanged
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppStateChanged
 */
HWTEST_F(AppStateObserverManagerTest, HandleAppStateChanged_008, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    std::vector<std::string> bundleNameList;
    std::shared_ptr<AppRunningRecord> appRecord = MockAppRecord();
    ApplicationState state = ApplicationState::APP_STATE_END;
    bool needNotifyApp = false;
    manager->HandleAppStateChanged(appRecord, state, needNotifyApp);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppStateObserverManagerTest, HandleStateChangedNotifyObserver_001, TestSize.Level0)
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
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppStateObserverManagerTest, HandleStateChangedNotifyObserver_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AbilityStateData abilityStateData;
    bool isAbility = false;
    std::vector<std::string> bundleNameList;
    std::string bundleName = "com.ohos.unittest";
    abilityStateData.bundleName = bundleName;
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppStateObserverManagerTest, HandleStateChangedNotifyObserver_003, TestSize.Level0)
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
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleStateChangedNotifyObserver
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleStateChangedNotifyObserver
 * EnvConditions: NA
 * CaseDescription: Verify HandleStateChangedNotifyObserver
 */
HWTEST_F(AppStateObserverManagerTest, HandleStateChangedNotifyObserver_004, TestSize.Level0)
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
    manager->HandleStateChangedNotifyObserver(abilityStateData, isAbility);
}

/*
 * Feature: AppStateObserverManager
 * Function: HandleOnAppProcessCreated
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager HandleOnAppProcessCreated
 * EnvConditions: NA
 * CaseDescription: Verify HandleOnAppProcessCreated
 */
HWTEST_F(AppStateObserverManagerTest, HandleOnAppProcessCreated_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnRenderProcessCreated_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessCreated_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessCreated_002, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessCreated_003, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessCreated_004, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessStateChanged_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessStateChanged_002, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessStateChanged_003, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessStateChanged_004, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessStateChanged_005, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnAppProcessDied_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnRenderProcessDied_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessDied_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessDied_002, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessDied_003, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, HandleOnProcessDied_004, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, ObserverExist_001, TestSize.Level0)
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
HWTEST_F(AppStateObserverManagerTest, ObserverExist_002, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<IApplicationStateObserver> observer = new MockApplicationStateObserver();
    std::vector<std::string> bundleNameList;
    manager->appStateObserverMap_.emplace(observer, bundleNameList);
    bool res = manager->ObserverExist(observer);
    EXPECT_TRUE(res);
}

/*
 * Feature: AppStateObserverManager
 * Function: AddObserverDeathRecipient
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager AddObserverDeathRecipient
 * EnvConditions: NA
 * CaseDescription: Verify AddObserverDeathRecipient
 */
HWTEST_F(AppStateObserverManagerTest, AddObserverDeathRecipient_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->AddObserverDeathRecipient(nullptr);
    manager->AddObserverDeathRecipient(observer_);
}

/*
 * Feature: AppStateObserverManager
 * Function: RemoveObserverDeathRecipient
 * SubFunction: NA
 * FunctionPoints: AppStateObserverManager RemoveObserverDeathRecipient
 * EnvConditions: NA
 * CaseDescription: Verify RemoveObserverDeathRecipient
 */
HWTEST_F(AppStateObserverManagerTest, RemoveObserverDeathRecipient_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    manager->RemoveObserverDeathRecipient(nullptr);
    manager->AddObserverDeathRecipient(observer_);
}

HWTEST_F(AppStateObserverManagerTest, HandleOnPageShow_001, TestSize.Level0)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    std::vector<std::string> bundleNameList;
    PageStateData pageStateData;
    pageStateData.bundleName = "com.test.demo";
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    
    manager->HandleOnPageShow(pageStateData);
}

HWTEST_F(AppStateObserverManagerTest, HandleOnPageHide_001, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    std::vector<std::string> bundleNameList;
    PageStateData pageStateData;
    pageStateData.bundleName = "com.test.demo";
    manager->appStateObserverMap_.emplace(observer_, bundleNameList);
    
    manager->HandleOnPageHide(pageStateData);
}
}  // namespace AppExecFwk
}  // namespace OHOS
