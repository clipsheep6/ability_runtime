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

#include <cstdlib>
#include <gtest/gtest.h>

#define private public
#include "app_mgr_proxy.h"
#include "app_mgr_stub.h"
#include "main_thread.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "process_info.h"
#include "quick_fix_callback_stub.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "ohos_application.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackImpl : public AppExecFwk::QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode, int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnUnloadPatchDone(int32_t resultCode, int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnReloadPageDone(int32_t resultCode, int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
    }
};

class MainThreadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MainThread> mainThread_ = nullptr;
};

void MainThreadTest::SetUpTestCase()
{
    sptr<IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    auto sysMgr = DelayedSingleton<SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }

    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void MainThreadTest::TearDownTestCase()
{}

void MainThreadTest::SetUp()
{
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ASSERT_NE(runner, nullptr);

    mainThread_ = sptr<MainThread>(new (std::nothrow) MainThread());
    ASSERT_NE(mainThread_, nullptr);

    mainThread_->Init(runner);
}

void MainThreadTest::TearDown()
{
    mainThread_->applicationForDump_.reset();
}

class MockAppMgrStub : public AppMgrStub {
    MockAppMgrStub() = default;
    virtual ~MockAppMgrStub() = default;

    void AttachApplication(const sptr<IRemoteObject> &app) override
    {}

    void ApplicationForegrounded(const int32_t recordId) override
    {}

    void ApplicationBackgrounded(const int32_t recordId) override
    {}

    void ApplicationTerminated(const int32_t recordId) override
    {}

    int CheckPermission(const int32_t recordId, const std::string &permission) override
    {
        return 0;
    }

    void AbilityCleaned(const sptr<IRemoteObject> &token) override
    {}

    sptr<IAmsMgr> GetAmsMgr() override
    {
        return nullptr;
    }

    int32_t ClearUpApplicationData(const std::string &bundleName) override
    {
        return 0;
    }

    int GetAllRunningProcesses(std::vector<RunningProcessInfo> &info) override
    {
        return 0;
    }

    int GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId) override
    {
        return 0;
    }

    int NotifyMemoryLevel(int32_t level) override
    {
        return 0;
    }

    void AddAbilityStageDone(const int32_t recordId) override
    {}

    void StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override
    {}

    int32_t RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
        const std::vector<std::string> &bundleNameList = {}) override
    {
        return 0;
    }

    int32_t UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer) override
    {
        return 0;
    }

    int32_t GetForegroundApplications(std::vector<AppStateData> &list) override
    {
        return 0;
    }

    int StartUserTestProcess(
        const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId) override
    {
        return 0;
    }

    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override
    {
        return 0;
    }

    void ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag) override
    {}

    int GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens) override
    {
        return 0;
    }

    #ifdef ABILITY_COMMAND_FOR_TEST
    int BlockAppService() override
    {
        return 0;
    }
    #endif

    int PreStartNWebSpawnProcess() override
    {
        return 0;
    }

    int StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
        int32_t sharedFd, pid_t &renderPid) override
    {
        return 0;
    }

    void AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler) override
    {}

    int GetRenderProcessTerminationStatus(pid_t renderPid, int &status) override
    {
        return 0;
    }

    int32_t GetConfiguration(Configuration& config) override
    {
        return 0;
    }

    int32_t UpdateConfiguration(const Configuration &config) override
    {
        return 0;
    }

    int32_t RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override
    {
        return 0;
    }

    int32_t UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer) override
    {
        return 0;
    }

    bool GetAppRunningStateByBundleName(const std::string &bundleName) override
    {
        return false;
    }

    int32_t NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override
    {
        return 0;
    }

    int32_t NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override
    {
        return 0;
    }

    int32_t NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback) override
    {
        return 0;
    }
};

/*
 * Feature: MainThread
 * Function: GetMainThreadState
 * SubFunction: NA
 * FunctionPoints: MainThread GetMainThreadState
 * EnvConditions: NA
 * CaseDescription: Verify GetMainThreadState
 */
HWTEST_F(MainThreadTest, GetMainThreadState_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetMainThreadState(), MainThreadState::INIT);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: SetRunnerStarted
 * SubFunction: NA
 * FunctionPoints: MainThread SetRunnerStarted
 * EnvConditions: NA
 * CaseDescription: Verify SetRunnerStarted
 */
HWTEST_F(MainThreadTest, SetRunnerStarted_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->SetRunnerStarted(true);
    EXPECT_TRUE(mainThread_->isRunnerStarted_);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetRunnerStarted
 * SubFunction: NA
 * FunctionPoints: MainThread GetRunnerStarted
 * EnvConditions: NA
 * CaseDescription: Verify GetRunnerStarted
 */
HWTEST_F(MainThreadTest, GetRunnerStarted_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_FALSE(mainThread_->GetRunnerStarted());
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetNewThreadId
 * SubFunction: NA
 * FunctionPoints: MainThread GetNewThreadId
 * EnvConditions: NA
 * CaseDescription: Verify GetNewThreadId
 */
HWTEST_F(MainThreadTest, GetNewThreadId_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetNewThreadId(), -1);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetApplication
 * SubFunction: NA
 * FunctionPoints: MainThread GetApplication
 * EnvConditions: NA
 * CaseDescription: Verify GetApplication
 */
HWTEST_F(MainThreadTest, GetApplication_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetApplication(), nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetApplicationInfo
 * SubFunction: NA
 * FunctionPoints: MainThread GetApplicationInfo
 * EnvConditions: NA
 * CaseDescription: Verify GetApplicationInfo
 */
HWTEST_F(MainThreadTest, GetApplicationInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetApplicationInfo(), nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetApplicationImpl
 * SubFunction: NA
 * FunctionPoints: MainThread GetApplicationImpl
 * EnvConditions: NA
 * CaseDescription: Verify GetApplicationImpl
 */
HWTEST_F(MainThreadTest, GetApplicationImpl_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_EQ(mainThread_->GetApplicationImpl(), nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: GetMainHandler
 * SubFunction: NA
 * FunctionPoints: MainThread GetMainHandler
 * EnvConditions: NA
 * CaseDescription: Verify GetMainHandler
 */
HWTEST_F(MainThreadTest, GetMainHandler_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_NE(mainThread_->GetMainHandler(), nullptr);
    mainThread_->ScheduleForegroundApplication();
    mainThread_->ScheduleBackgroundApplication();
    mainThread_->ScheduleTerminateApplication();
    mainThread_->ScheduleShrinkMemory(1);
    mainThread_->ScheduleMemoryLevel(1);
    mainThread_->ScheduleProcessSecurityExit();
    mainThread_->ScheduleLowMemory();
    AppLaunchData data;
    Configuration config;
    mainThread_->ScheduleLaunchApplication(data, config);
    HapModuleInfo abilityStage;
    mainThread_->ScheduleAbilityStage(abilityStage);
    AbilityInfo info;
    sptr<IRemoteObject> Token = nullptr;
    std::shared_ptr<AAFwk::Want> want;
    mainThread_->ScheduleLaunchAbility(info, Token, want);
    mainThread_->ScheduleCleanAbility(Token);
    Profile profile;
    mainThread_->ScheduleProfileChanged(profile);
    mainThread_->ScheduleConfigurationUpdated(config);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: InitCreate
 * SubFunction: NA
 * FunctionPoints: MainThread InitCreate
 * EnvConditions: NA
 * CaseDescription: Verify InitCreate
 */
HWTEST_F(MainThreadTest, InitCreate_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<ContextDeal> contextDeal;
    ApplicationInfo appInfo;
    ProcessInfo processInfo;
    Profile appProfile;
    EXPECT_TRUE(mainThread_->InitCreate(contextDeal, appInfo, processInfo, appProfile));

    mainThread_->watchdog_ = nullptr;
    EXPECT_TRUE(mainThread_->InitCreate(contextDeal, appInfo, processInfo, appProfile));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyLoadRepairPatch_0100
 * @tc.desc: schedule notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyLoadRepairPatch_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto ret = mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    EXPECT_EQ(ret, NO_ERROR);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyHotReloadPage_0100
 * @tc.desc: schedule notify ace hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyHotReloadPage_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto ret = mainThread_->ScheduleNotifyHotReloadPage(callback, recordId);
    EXPECT_EQ(ret, NO_ERROR);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetHqfFileAndHapPath_0100
 * @tc.desc: get patch file and hap path.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, GetHqfFileAndHapPath_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    ProcessInfo processInfo("test_quickfix", 1);
    mainThread_->processInfo_ = std::make_shared<ProcessInfo>(processInfo);
    std::string bundleName = "com.ohos.quickfix";
    std::vector<std::pair<std::string, std::string>> fileMap;
    auto ret = mainThread_->GetHqfFileAndHapPath(bundleName, fileMap);
    EXPECT_TRUE(ret);
    ASSERT_EQ(fileMap.size(), 2);
    EXPECT_EQ(fileMap[0].first, "/data/storage/el1/bundle/patch_1000/entry1.hqf");
    EXPECT_EQ(fileMap[0].second, "/data/storage/el1/bundle/entry1");
    EXPECT_EQ(fileMap[1].first, "/data/storage/el1/bundle/patch_1000/entry2.hqf");
    EXPECT_EQ(fileMap[1].second, "/data/storage/el1/bundle/entry2");
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyUnLoadRepairPatch_0100
 * @tc.desc: schedule notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, ScheduleNotifyUnLoadRepairPatch_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName;
    int32_t recordId = 0;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto ret = mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    EXPECT_EQ(ret, NO_ERROR);

    mainThread_->mainHandler_ = nullptr;
    EXPECT_EQ(mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId), ERR_INVALID_VALUE);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: InitResourceManager_0100
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, InitResourceManager_0100, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    HapModuleInfo info;
    Configuration config;
    bool multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);
    multiProjects = false;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.name = "com.ohos.contactsdataability";
    info.moduleName = "entry";
    info.description = "dataability_description";
    info.iconPath = "$media:icon";
    info.deviceTypes = { "smartVision" };
    info.bundleName = "com.ohos.contactsdataability";
    multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);

    multiProjects = false;
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.resourcePath = "/data/app/el1/budle/public/com.ohos.contactsdataability"\
        "/com.ohos.contactsdataability/assets/entry/resources.index";
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.hapPath = "/system/app/com.ohos.contactsdataability/Contacts_DataAbility.hap";
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);

    info.resourcePath = "";
    mainThread_->InitResourceManager(resourceManager, info, info.bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0100
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0100, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdata;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdata.SetApplicationInfo(appinf);
    lanchdata.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdata, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);

    lanchdata.SetAppIndex(1);
    mainThread_->HandleLaunchApplication(lanchdata, config);
}

/**
 * @tc.name: SetNativeLibPath_0100
 * @tc.desc: set native lib path.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, SetNativeLibPath_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    Configuration config;
    AppLaunchData launchData;
    ProcessInfo processInfo("test_quickfix", 9999);
    ApplicationInfo appInfo;
    appInfo.name = "MainAbility";
    appInfo.bundleName = "com.ohos.quickfix";
    launchData.SetApplicationInfo(appInfo);
    launchData.SetProcessInfo(processInfo);

    // SetNativeLibPath is implemented in anonymous space, called by HandleLaunchApplication
    mainThread_->HandleLaunchApplication(launchData, config);
    ASSERT_NE(mainThread_->application_, nullptr);
    EXPECT_NE(mainThread_->application_->abilityRuntimeContext_, nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ConnectToAppMgr_0100
 * @tc.desc: ConnectToAppMgr.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, ConnectToAppMgr_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_TRUE(mainThread_->ConnectToAppMgr());
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: Attach_0100
 * @tc.desc: Attach.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, Attach_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->Attach();
    EXPECT_EQ(MainThreadState::ATTACH, mainThread_->mainThreadState_);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0100
 * @tc.desc: RemoveAppMgrDeathRecipient.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, RemoveAppMgrDeathRecipient_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->RemoveAppMgrDeathRecipient();
    EXPECT_TRUE(mainThread_->ConnectToAppMgr());
    mainThread_->RemoveAppMgrDeathRecipient();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0100
 * @tc.desc: CheckLaunchApplicationParam.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckLaunchApplicationParam_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckLaunchApplicationParam(appLaunchData));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckLaunchApplicationParam_0200
 * @tc.desc: CheckLaunchApplicationParam.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckLaunchApplicationParam_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "test";
    ProcessInfo processInfo("", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckLaunchApplicationParam(appLaunchData));

    ProcessInfo processInfo2("test", 1);
    appLaunchData.SetProcessInfo(processInfo2);
    EXPECT_TRUE(mainThread_->CheckLaunchApplicationParam(appLaunchData));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0100
 * @tc.desc: CheckAbilityItem.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckAbilityItem_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(info, nullptr);
    EXPECT_FALSE(mainThread_->CheckAbilityItem(record));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0200
 * @tc.desc: CheckAbilityItem.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckAbilityItem_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(nullptr, nullptr);
    EXPECT_FALSE(mainThread_->CheckAbilityItem(record));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckAbilityItem_0300
 * @tc.desc: CheckAbilityItem.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckAbilityItem_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_FALSE(mainThread_->CheckAbilityItem(nullptr));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0100
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->HandleTerminateApplicationLocal();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0200
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->signalHandler_->SetEventRunner(nullptr);
    mainThread_->HandleTerminateApplicationLocal();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0300
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->mainHandler_->SetEventRunner(nullptr);
    mainThread_->HandleTerminateApplicationLocal();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplicationLocal_0400
 * @tc.desc: HandleTerminateApplicationLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplicationLocal_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->watchdog_ = nullptr;
    mainThread_->HandleTerminateApplicationLocal();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleProcessSecurityExit_0100
 * @tc.desc: HandleProcessSecurityExit.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleProcessSecurityExit_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->abilityRecordMgr_ = nullptr;
    mainThread_->HandleProcessSecurityExit();

    std::shared_ptr<ContextDeal> contextDeal;
    ApplicationInfo appInfo;
    ProcessInfo processInfo;
    Profile appProfile;
    mainThread_->InitCreate(contextDeal, appInfo, processInfo, appProfile);
    mainThread_->HandleProcessSecurityExit();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleProcessSecurityExit_0200
 * @tc.desc: HandleProcessSecurityExit.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleProcessSecurityExit_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleProcessSecurityExit();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckForHandleLaunchApplication_0100
 * @tc.desc: CheckForHandleLaunchApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckForHandleLaunchApplication_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "test";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_TRUE(mainThread_->CheckForHandleLaunchApplication(appLaunchData));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckForHandleLaunchApplication_0200
 * @tc.desc: CheckForHandleLaunchApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckForHandleLaunchApplication_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "test";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckForHandleLaunchApplication(appLaunchData));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckForHandleLaunchApplication_0300
 * @tc.desc: CheckForHandleLaunchApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckForHandleLaunchApplication_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    AppLaunchData appLaunchData;
    ApplicationInfo appInfo;
    appInfo.name = "";
    ProcessInfo processInfo("test", 1);
    appLaunchData.SetApplicationInfo(appInfo);
    appLaunchData.SetProcessInfo(processInfo);
    EXPECT_FALSE(mainThread_->CheckForHandleLaunchApplication(appLaunchData));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0100
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0200
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0300
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->appMgr_ = nullptr;
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleAbilityStage_0400
 * @tc.desc: HandleAbilityStage.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleAbilityStage_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->applicationImpl_ = nullptr;
    HapModuleInfo info;
    mainThread_->HandleAbilityStage(info);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0100
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    std::shared_ptr<UserTestRecord> usertestInfo = std::make_shared<UserTestRecord>();
    bool isStageBased = true;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(mainThread_->PrepareAbilityDelegator(usertestInfo, isStageBased, hapModuleInfo));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0200
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    bool isStageBased = true;
    HapModuleInfo hapModuleInfo;
    EXPECT_FALSE(mainThread_->PrepareAbilityDelegator(nullptr, isStageBased, hapModuleInfo));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0300
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    std::shared_ptr<UserTestRecord> usertestInfo = std::make_shared<UserTestRecord>();
    bool isStageBased = false;
    AbilityInfo abilityInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
    EXPECT_TRUE(mainThread_->PrepareAbilityDelegator(usertestInfo, isStageBased, hapModuleInfo));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: PrepareAbilityDelegator_0400
 * @tc.desc: PrepareAbilityDelegator.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, PrepareAbilityDelegator_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    std::shared_ptr<UserTestRecord> usertestInfo = std::make_shared<UserTestRecord>();
    bool isStageBased = false;
    HapModuleInfo hapModuleInfo;
    EXPECT_FALSE(mainThread_->PrepareAbilityDelegator(usertestInfo, isStageBased, hapModuleInfo));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleLaunchAbility_0100
 * @tc.desc: HandleLaunchAbility.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleLaunchAbility_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(nullptr, nullptr);
    mainThread_->HandleLaunchAbility(abilityRecord);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleCleanAbilityLocal_0100
 * @tc.desc: HandleCleanAbilityLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleCleanAbilityLocal_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->HandleCleanAbilityLocal(nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleCleanAbilityLocal_0200
 * @tc.desc: HandleCleanAbilityLocal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleCleanAbilityLocal_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleCleanAbilityLocal(nullptr);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleForegroundApplication_0100
 * @tc.desc: HandleForegroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleForegroundApplication_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleForegroundApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleForegroundApplication_0200
 * @tc.desc: HandleForegroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleForegroundApplication_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->HandleForegroundApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleForegroundApplication_0300
 * @tc.desc: HandleForegroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleForegroundApplication_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->appMgr_ = nullptr;
    mainThread_->HandleForegroundApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleBackgroundApplication_0100
 * @tc.desc: HandleBackgroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleBackgroundApplication_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleBackgroundApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleBackgroundApplication_0200
 * @tc.desc: HandleBackgroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleBackgroundApplication_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->HandleBackgroundApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleBackgroundApplication_0300
 * @tc.desc: HandleBackgroundApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleBackgroundApplication_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->appMgr_ = nullptr;
    mainThread_->HandleBackgroundApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0100
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleTerminateApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0200
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->HandleTerminateApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0300
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->appMgr_ = nullptr;
    mainThread_->HandleTerminateApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0400
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->signalHandler_->SetEventRunner(nullptr);
    mainThread_->HandleTerminateApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0500
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0500, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->mainHandler_->SetEventRunner(nullptr);
    mainThread_->HandleTerminateApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleTerminateApplication_0600
 * @tc.desc: HandleTerminateApplication.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleTerminateApplication_0600, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->watchdog_ = nullptr;
    mainThread_->HandleTerminateApplication();
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleShrinkMemory_0100
 * @tc.desc: HandleShrinkMemory.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleShrinkMemory_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleShrinkMemory(1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleShrinkMemory_0200
 * @tc.desc: HandleShrinkMemory.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleShrinkMemory_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->applicationImpl_ = nullptr;
    mainThread_->HandleShrinkMemory(1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleMemoryLevel_0100
 * @tc.desc: HandleMemoryLevel.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleMemoryLevel_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleMemoryLevel(1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleMemoryLevel_0200
 * @tc.desc: HandleMemoryLevel.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleMemoryLevel_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->HandleMemoryLevel(1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleConfigurationUpdated_0100
 * @tc.desc: HandleConfigurationUpdated.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleConfigurationUpdated_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    Configuration config;
    mainThread_->HandleConfigurationUpdated(config);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleConfigurationUpdated_0200
 * @tc.desc: HandleConfigurationUpdated.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleConfigurationUpdated_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->applicationImpl_ = nullptr;
    Configuration config;
    mainThread_->HandleConfigurationUpdated(config);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleSignal_0100
 * @tc.desc: HandleSignal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleSignal_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    constexpr int SIGNAL_JS_HEAP = 39;
    mainThread_->HandleSignal(SIGNAL_JS_HEAP);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleSignal_0200
 * @tc.desc: HandleSignal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleSignal_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    constexpr int SIGNAL_JS_HEAP_PRIV = 40;
    mainThread_->HandleSignal(SIGNAL_JS_HEAP_PRIV);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleSignal_0300
 * @tc.desc: HandleSignal.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleSignal_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->HandleSignal(-1);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0100
 * @tc.desc: IsApplicationReady.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, IsApplicationReady_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->IsApplicationReady());
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0200
 * @tc.desc: IsApplicationReady.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, IsApplicationReady_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_FALSE(mainThread_->IsApplicationReady());
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: IsApplicationReady_0300
 * @tc.desc: IsApplicationReady.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, IsApplicationReady_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->applicationImpl_ = nullptr;
    EXPECT_FALSE(mainThread_->IsApplicationReady());
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0100
 * @tc.desc: CheckFileType.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckFileType_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string fileName = "test.testExtension";
    std::string extensionName = "testExtension";
    EXPECT_FALSE(mainThread_->CheckFileType(fileName, extensionName));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0200
 * @tc.desc: CheckFileType.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckFileType_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string fileName = "";
    std::string extensionName = "testExtension";
    EXPECT_FALSE(mainThread_->CheckFileType(fileName, extensionName));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: CheckFileType_0300
 * @tc.desc: CheckFileType.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, CheckFileType_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string fileName = "testExtension";
    std::string extensionName = "testExtension";
    EXPECT_FALSE(mainThread_->CheckFileType(fileName, extensionName));
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0100
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    Want want;
    std::string moduleName;
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0200
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->application_ = nullptr;
    Want want;
    std::string moduleName;
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0300
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->appMgr_ = nullptr;
    Want want;
    std::string moduleName;
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleScheduleAcceptWant_0400
 * @tc.desc: HandleScheduleAcceptWant.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadTest, HandleScheduleAcceptWant_0400, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->applicationImpl_ = nullptr;
    Want want;
    std::string moduleName;
    mainThread_->HandleScheduleAcceptWant(want, moduleName);
    HILOG_INFO("%{public}s end.", __func__);
}

#ifdef ABILITY_LIBRARY_LOADER
/*
 * Feature: MainThread
 * Function: LoadNativeLiabrary
 * SubFunction: NA
 * FunctionPoints: MainThread LoadNativeLiabrary
 * EnvConditions: NA
 * CaseDescription: Verify LoadNativeLiabrary
 */
HWTEST_F(MainThreadTest, LoadNativeLiabrary_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string nativeLibraryPath = "";
    mainThread_->LoadNativeLiabrary(nativeLibraryPath);

    nativeLibraryPath = "test/";
    mainThread_->LoadNativeLiabrary(nativeLibraryPath);
    HILOG_INFO("%{public}s end.", __func__);
}
#endif

/*
 * Feature: MainThread
 * Function: TaskTimeoutDetected
 * SubFunction: NA
 * FunctionPoints: MainThread TaskTimeoutDetected
 * EnvConditions: NA
 * CaseDescription: Verify TaskTimeoutDetected
 */
HWTEST_F(MainThreadTest, TaskTimeoutDetected_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    mainThread_->TaskTimeoutDetected(nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    mainThread_->TaskTimeoutDetected(runner);

    mainThread_->mainHandler_.reset();
    mainThread_->TaskTimeoutDetected(runner);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: MainThread
 * Function: HandleDumpHeap
 * SubFunction: NA
 * FunctionPoints: MainThread HandleDumpHeap
 * EnvConditions: NA
 * CaseDescription: Verify HandleDumpHeap
 */
HWTEST_F(MainThreadTest, HandleDumpHeap_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    bool isPrivate = false;
    mainThread_->HandleDumpHeap(isPrivate);

    mainThread_->mainHandler_ = nullptr;
    mainThread_->HandleDumpHeap(isPrivate);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyLoadRepairPatch_0200
 * @tc.desc: schedule notify load repair patch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScheduleNotifyLoadRepairPatch_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto bakHandler = mainThread_->mainHandler_;
    mainThread_->mainHandler_ = nullptr;
    auto ret = mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    mainThread_->mainHandler_ = bakHandler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyHotReloadPage_0200
 * @tc.desc: schedule notify ace hot reload page.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScheduleNotifyHotReloadPage_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    int32_t recordId = 0;
    auto bakHandler = mainThread_->mainHandler_;
    mainThread_->mainHandler_ = nullptr;
    auto ret = mainThread_->ScheduleNotifyHotReloadPage(callback, recordId);
    mainThread_->mainHandler_ = bakHandler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ScheduleNotifyUnLoadRepairPatch_0200
 * @tc.desc: schedule notify unload repair patch.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScheduleNotifyUnLoadRepairPatch_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    std::string bundleName;
    int32_t recordId = 0;
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    auto bakHandler = mainThread_->mainHandler_;
    mainThread_->mainHandler_ = nullptr;
    auto ret = mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    mainThread_->mainHandler_ = bakHandler;
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: InitResourceManager_0200
 * @tc.desc: init resourceManager.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, InitResourceManager_0200, TestSize.Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    EXPECT_TRUE(resourceManager != nullptr);
    Configuration config;
    HapModuleInfo hapModuleInfo = {};
    hapModuleInfo.isStageBasedModel = true;
    const std::string bundleName = "bundleName";
    bool multiProjects = true;
    mainThread_->InitResourceManager(resourceManager, hapModuleInfo, bundleName, multiProjects, config);
    EXPECT_TRUE(resourceManager != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0200
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0200, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    mainThread_->application_ = nullptr;
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
}

/**
 * @tc.name: HandleLaunchApplication_0300
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0300, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    lanchdate.SetProcessInfo(processing);
    appinf.bundleName = "com.ohos.contactsdataability";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    appinf.bundleName = "com.ohos.medialibrary.medialibrarydata";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    appinf.bundleName = "com.ohos.telephonydataability";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    appinf.bundleName = "com.ohos.FusionSearch";
    lanchdate.SetApplicationInfo(appinf);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->handleAbilityLib_.clear();
}

/**
 * @tc.name: HandleLaunchApplication_0400
 * @tc.desc: Handle launch application.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchApplication_0400, TestSize.Level1)
{
    Configuration config;
    AppLaunchData lanchdate;
    ProcessInfo processing("TestProcess", 9999);
    ApplicationInfo appinf;
    appinf.name = "MockTestApplication";
    appinf.moduleSourceDirs.push_back("/hos/lib/libabilitydemo_native.z.so");
    lanchdate.SetAppIndex(1);
    lanchdate.SetApplicationInfo(appinf);
    lanchdate.SetProcessInfo(processing);
    mainThread_->HandleLaunchApplication(lanchdate, config);
    EXPECT_TRUE(mainThread_->application_ != nullptr);
}

/**
 * @tc.name: OnRemoteDied_0100
 * @tc.desc: remote Object OnRemoteDied callde.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, OnRemoteDied_0100, TestSize.Level1)
{
    sptr<AppMgrDeathRecipient> death = new (std::nothrow) AppMgrDeathRecipient();
    EXPECT_TRUE(death != nullptr);
    wptr<IRemoteObject> remote = nullptr;
    death->OnRemoteDied(remote);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0200
 * @tc.desc: Remove app mgr death recipient.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, RemoveAppMgrDeathRecipient_0200, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    EXPECT_TRUE(mainThread_->appMgr_ == nullptr);
    sptr<IRemoteObject> object;
    EXPECT_TRUE(object == nullptr);
    mainThread_->appMgr_ = new (std::nothrow) AppMgrProxy(object);
    mainThread_->RemoveAppMgrDeathRecipient();
    EXPECT_TRUE(mainThread_->appMgr_ != nullptr);
}

/**
 * @tc.name: RemoveAppMgrDeathRecipient_0300
 * @tc.desc: Remove app mgr death recipient.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, RemoveAppMgrDeathRecipient_0300, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    EXPECT_TRUE(mainThread_->appMgr_ == nullptr);
    mainThread_->appMgr_ = new (std::nothrow) AppMgrProxy(mainThread_);
    mainThread_->RemoveAppMgrDeathRecipient();
    EXPECT_TRUE(mainThread_->appMgr_ != nullptr);
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: Post task.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, PostTask_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(mainThread_->applicationInfo_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->applicationImpl_ != nullptr);
    auto startThreadTask = [main = mainThread_] () {
        main->Start();
    };
    std::thread testThread(startThreadTask);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mainThread_->ScheduleForegroundApplication();
    mainThread_->ScheduleBackgroundApplication();
    mainThread_->ScheduleTerminateApplication();
    constexpr int32_t level = 0;
    mainThread_->ScheduleShrinkMemory(level);
    mainThread_->ScheduleMemoryLevel(level);
    mainThread_->ScheduleProcessSecurityExit();
    mainThread_->ScheduleLowMemory();
    AppLaunchData applaunchData = {};
    Configuration configuration = {};
    mainThread_->ScheduleLaunchApplication(applaunchData, configuration);
    HapModuleInfo hapModuleInfo = {};
    mainThread_->ScheduleAbilityStage(hapModuleInfo);
    AbilityInfo abilityInfo = {};
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    mainThread_->ScheduleLaunchAbility(abilityInfo, token, want);
    mainThread_->ScheduleCleanAbility(token);
    Profile profile = {};
    mainThread_->ScheduleProfileChanged(profile);
    mainThread_->ScheduleConfigurationUpdated(configuration);
    bool isPrivate = false;
    mainThread_->HandleDumpHeap(isPrivate);
    const std::string moduleName = "";
    mainThread_->ScheduleAcceptWant(*want, moduleName);
    const std::string bundleName = "";
    sptr<IQuickFixCallback> callback = nullptr;
    constexpr int32_t recordId = 0;
    mainThread_->ScheduleNotifyLoadRepairPatch(bundleName, callback, recordId);
    mainThread_->ScheduleNotifyHotReloadPage(callback, recordId);
    mainThread_->ScheduleNotifyUnLoadRepairPatch(bundleName, callback, recordId);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    mainThread_->HandleTerminateApplicationLocal();
    testThread.join();
}

/**
 * @tc.name: ChangeToLocalPath_0100
 * @tc.desc: Change to local path.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ChangeToLocalPath_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::string bundleName = "";
    const std::vector<std::string> sourceDirs1 = { "test1", "", "test3" };
    std::vector<std::string> localPath = {};
    mainThread_->ChangeToLocalPath(bundleName, sourceDirs1, localPath);

    const std::vector<std::string> sourceDirs2 = {};
    mainThread_->ChangeToLocalPath(bundleName, sourceDirs2, localPath);
}

/**
 * @tc.name: LoadAllExtensions_0200
 * @tc.desc: load all extensions.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, LoadAllExtensions_0200, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    std::shared_ptr<OHOSApplication> application = nullptr;
    const std::string filePath = "/system/lib/module/web/";
    mainThread_->LoadAllExtensions(filePath, application);
}

/**
 * @tc.name: HandleLaunchAbility_0200
 * @tc.desc: handle launch ability.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleLaunchAbility_0200, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::shared_ptr<AbilityLocalRecord> abilityRecord1 = nullptr;
    AAFwk::Want want = {};
    want.SetParam("debugApp", false);
    EXPECT_TRUE(abilityRecord1 == nullptr);
    mainThread_->HandleLaunchAbility(abilityRecord1);

    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    mainThread_->HandleLaunchAbility(abilityRecord1);

    mainThread_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    mainThread_->HandleLaunchAbility(abilityRecord1);

    const std::shared_ptr<AbilityInfo> info1 = nullptr;
    const sptr<IRemoteObject> token = nullptr;
    const std::shared_ptr<AbilityLocalRecord> abilityRecord2 = std::make_shared<AbilityLocalRecord>(info1, token);
    mainThread_->HandleLaunchAbility(abilityRecord2);

    const std::shared_ptr<AbilityInfo> info2 = std::make_shared<AbilityInfo>();
    const std::shared_ptr<AbilityLocalRecord> abilityRecord3 = std::make_shared<AbilityLocalRecord>(info2, token);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    mainThread_->HandleLaunchAbility(abilityRecord3);

    abilityRecord3->token_ = mainThread_;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    mainThread_->application_->SetRuntime(std::move(runtime));
    auto contextDeal = std::make_shared<ContextDeal>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->debug = true;
    contextDeal->SetApplicationInfo(appInfo);
    mainThread_->application_->AttachBaseContext(contextDeal);
    mainThread_->HandleLaunchAbility(abilityRecord3);
}

/**
 * @tc.name: HandleCleanAbility_0100
 * @tc.desc: handle clean ability.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, HandleCleanAbility_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    EXPECT_TRUE(mainThread_->abilityRecordMgr_ != nullptr);
    mainThread_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    EXPECT_TRUE(mainThread_->applicationInfo_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->applicationImpl_ != nullptr);
    mainThread_->appMgr_ = new (std::nothrow) AppMgrProxy(mainThread_);
    EXPECT_TRUE(mainThread_->appMgr_ != nullptr);

    sptr<IRemoteObject> token = nullptr;
    mainThread_->HandleCleanAbility(token);

    token = new (std::nothrow) BundleMgrService();
    mainThread_->HandleCleanAbility(token);

    std::shared_ptr<AbilityLocalRecord> abilityRecord = nullptr;
    mainThread_->abilityRecordMgr_->abilityRecords_[token] = abilityRecord;
    mainThread_->HandleCleanAbility(token);

    std::shared_ptr<AbilityInfo> info = nullptr;
    abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    mainThread_->abilityRecordMgr_->AddAbilityRecord(token, abilityRecord);
    mainThread_->HandleCleanAbility(token);

    info = std::make_shared<AbilityInfo>();
    abilityRecord->abilityInfo_ = info;
    mainThread_->abilityRecordMgr_->AddAbilityRecord(token, abilityRecord);
    mainThread_->HandleCleanAbility(token);

    mainThread_->application_ = nullptr;
    mainThread_->HandleCleanAbility(token);
}

/**
 * @tc.name: LoadAbilityLibrary_0100
 * @tc.desc: Load ability library.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, LoadAbilityLibrary_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::vector<std::string> libraryPaths = {
        "/system/lib/module/web/",
    };

    mainThread_->LoadAbilityLibrary(libraryPaths);
    mainThread_->handleAbilityLib_.clear();
}

/**
 * @tc.name: ScanDir_0100
 * @tc.desc: Close ability library.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ScanDir_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    const std::string path1 = "";
    std::vector<std::string> files1;
    mainThread_->ScanDir(path1, files1);

    const std::string path2 = "/system/lib/module/web/";
    std::vector<std::string> files2;
    mainThread_->ScanDir(path2, files2);
}

/**
 * @tc.name: CheckMainThreadIsAlive_0100
 * @tc.desc: Close ability library.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, CheckMainThreadIsAlive_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->watchdog_ = std::make_shared<Watchdog>();
    EXPECT_TRUE(mainThread_->watchdog_ != nullptr);

    mainThread_->CheckMainThreadIsAlive();
}

/**
 * @tc.name: ProcessEvent_0100
 * @tc.desc: Main handler process event.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, ProcessEvent_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    auto callback = [] () {};
    const std::string name = "";
    auto event = InnerEvent::Get(callback, name);
    mainThread_->mainHandler_->ProcessEvent(event);

    event->innerEventId_ = CHECK_MAIN_THREAD_IS_ALIVE;
    mainThread_->mainHandler_->ProcessEvent(event);

    auto mainThreadObj = mainThread_->mainHandler_->mainThreadObj_.promote();
    mainThread_->mainHandler_->mainThreadObj_ = nullptr;
    mainThread_->mainHandler_->ProcessEvent(event);
    mainThread_->mainHandler_->mainThreadObj_ = mainThreadObj;
}

/**
 * @tc.name: Start_0100
 * @tc.desc: Main thread start.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadTest, Start_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    mainThread_->application_ = std::make_shared<OHOSApplication>();
    EXPECT_TRUE(mainThread_->application_ != nullptr);
    mainThread_->applicationImpl_ = std::make_shared<ApplicationImpl>();
    EXPECT_TRUE(mainThread_->applicationImpl_ != nullptr);
    auto startThreadTask = [main = mainThread_] () {
        GTEST_LOG_(INFO) << "main thread Begin.";
        main->Start();
        GTEST_LOG_(INFO) << "main thread End.";
    };

    std::thread testThread(startThreadTask);
    GTEST_LOG_(INFO) << "Wait main thread run.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    GTEST_LOG_(INFO) << "Stop to main thread called.";
    mainThread_->HandleTerminateApplicationLocal();
    GTEST_LOG_(INFO) << "Wait main thread release.";
    testThread.join();
}
} // namespace AppExecFwk
} // namespace OHOS