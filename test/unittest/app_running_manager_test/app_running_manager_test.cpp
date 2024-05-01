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
#include <gtest/hwext/gtest-multithread.h>

#define private public
#include "app_running_manager.h"
#include "child_process_record.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "window_visibility_info.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t DEBUGINFOS_SIZE = 0;
constexpr int32_t ABILITYTOKENS_SIZE = 0;
constexpr int32_t RECORD_ID = 1;
constexpr uint32_t WINDOW_ID = 100;
constexpr pid_t PID = 10;
constexpr int32_t RECORD_MAP_SIZE = 1;
constexpr int32_t DEBUG_INFOS_SIZE = 1;
constexpr int32_t ABILITY_TOKENS_SIZE = 1;
}
class AppRunningManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppRunningManagerTest::SetUpTestCase(void)
{}

void AppRunningManagerTest::TearDownTestCase(void)
{}

void AppRunningManagerTest::SetUp()
{}

void AppRunningManagerTest::TearDown()
{}

/**
 * @tc.name: AppRunningManager_SetAttachAppDebug_0100
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_SetAttachAppDebug_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, true);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord->GetBundleName() == bundleName) {
            appRecord->SetAttachDebug(true);
            EXPECT_EQ(appRecord->isAttachDebug_, true);
        }
    }
}

/**
 * @tc.name: AppRunningManager_SetAttachAppDebug_0200
 * @tc.desc: Test the state of AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_SetAttachAppDebug_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord->GetBundleName() == bundleName) {
            appRecord->SetAttachDebug(true);
            EXPECT_EQ(appRecord->isAttachDebug_, true);
        }
    }
}

/**
 * @tc.name: AppRunningManager_GetAppDebugInfoByBundleName_0100
 * @tc.desc: Test the state of GetAppDebugInfoByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppDebugInfoByBundleName_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::vector<AppDebugInfo> debugInfos;
    bool isDetachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->GetAppDebugInfosByBundleName(bundleName, isDetachDebug);
    EXPECT_EQ(appRunningManager->appRunningRecordMap_.size(), RECORD_MAP_SIZE);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        AppDebugInfo debugInfo;
        debugInfos.emplace_back(debugInfo);
        EXPECT_EQ(debugInfos.size(), DEBUG_INFOS_SIZE);
    }
}

/**
 * @tc.name: AppRunningManager_GetAbilityTokensByBundleName_0100
 * @tc.desc: Test the state of GetAbilityTokensByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAbilityTokensByBundleName_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::vector<sptr<IRemoteObject>> abilityTokens;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->GetAbilityTokensByBundleName(bundleName, abilityTokens);
    for (const auto &item : appRunningManager->appRunningRecordMap_) {
        const auto &appRecord = item.second;
        for (const auto &token : appRecord->GetAbilities()) {
            abilityTokens.emplace_back(token.first);
            EXPECT_EQ(abilityTokens.size(), ABILITY_TOKENS_SIZE);
        }
    }
}

/**
 * @tc.name: AppRunningManager_OnWindowVisibilityChanged_0100
 * @tc.desc: verify the function of OnWindowVisibilityChanged : set windowIds and isUpdateStateFromService_
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_OnWindowVisibilityChanged_0100, TestSize.Level1)
{
    // 1. create ApprunningManager
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    // 2. createAppRunningRecord and put it into appRunningRecordMap_ of AppRunningManager
    std::string processName = "processName";
    std::string appName = "appName";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = appName;
    int32_t recordId = AppRecordId::Create();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    EXPECT_NE(appRunningRecord, nullptr);
    appRunningRecord->curState_ = ApplicationState::APP_STATE_BACKGROUND;
    appRunningRecord->isUpdateStateFromService_ = false;
    appRunningRecord->GetPriorityObject()->SetPid(PID);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRunningRecord);

    // 3. construct WindowVisibilityInfos
    std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> windowVisibilityInfos;
    auto info = new (std::nothrow) Rosen::WindowVisibilityInfo();
    EXPECT_NE(info, nullptr);
    info->windowId_ = WINDOW_ID;
    info->pid_ = PID;
    info->visibilityState_ = Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    windowVisibilityInfos.push_back(info);

    // 4. verify the function
    appRunningManager->OnWindowVisibilityChanged(windowVisibilityInfos);
    EXPECT_FALSE(appRunningManager->appRunningRecordMap_.empty());
    EXPECT_FALSE(appRunningManager->appRunningRecordMap_.at(1)->windowIds_.empty());
    EXPECT_TRUE(appRunningManager->appRunningRecordMap_.at(1)->isUpdateStateFromService_);
}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByChildProcessPid_0100
 * @tc.desc: Test GetAppRunningRecordByChildProcessPid works
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_GetAppRunningRecordByChildProcessPid_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByChildProcessPid_0100 called.");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);

    auto appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(PID, "./ets/AProcess.ts", appRecord, 1, false);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->AddChildProcessRecord(childPid, childRecord);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRecord));

    auto record = appRunningManager->GetAppRunningRecordByChildProcessPid(childPid);
    EXPECT_NE(record, nullptr);
}

/**
 * @tc.name: AppRunningManager_UpdateConfiguration_0100
 * @tc.desc: Test UpdateConfiguration works
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningManager_UpdateConfiguration_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    std::vector<AppDebugInfo> debugInfos;
    bool isDetachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = 1;
    std::string processName;
    Configuration config;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(recordId, appRunningRecord);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, nullptr);
    appRunningRecord->SetState(ApplicationState::APP_STATE_READY);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, appRunningRecord);
    appInfo->name = "com.huawei.shell_assistant";
    appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.emplace(++recordId, appRunningRecord);
    EXPECT_EQ(appRunningManager->appRunningRecordMap_.size(), recordId);
    auto ret = appRunningManager->UpdateConfiguration(config);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveAppRunningRecordById_0100
 * @tc.desc: Remove app running record by id.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, RemoveAppRunningRecordById_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    std::string processName = "test.ProcessName";
    BundleInfo bundleInfo;
    auto appRecord = appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    ASSERT_NE(appRecord, nullptr);

    int32_t uiExtensionAbilityId = 1000;
    pid_t hostPid = 1001;
    pid_t providerPid = 1002;
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    appRunningManager->RemoveAppRunningRecordById(appRecord->GetRecordId());
}

/**
 * @tc.name: AppRunningRecord_0100
 * @tc.desc: AppRunningRecord test multi-thread.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, AppRunningRecord_0100, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager_Record0100 = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager_Record0100, nullptr);
    auto task = []() {
        TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d.", gettid());
        std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
        static std::string processName = "test.ProcessName";
        BundleInfo bundleInfo;
        auto appRecord = appRunningManager_Record0100->CreateAppRunningRecord(appInfo, processName, bundleInfo);
        ASSERT_NE(appRecord, nullptr);
        processName += "a";

        static int32_t uiExtensionAbilityId = 1;
        static pid_t hostPid = 100000;
        static pid_t providerPid = 200000;
        appRunningManager_Record0100->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);
        uiExtensionAbilityId++;
        hostPid += 2;
        providerPid += 3;

        appRunningManager_Record0100->RemoveUIExtensionLauncherItem(hostPid);
        appRunningManager_Record0100->RemoveUIExtensionLauncherItem(providerPid);

        std::vector<pid_t> hostPids;
        std::vector<pid_t> providerPids;
        appRunningManager_Record0100->GetAllUIExtensionRootHostPid(hostPid, providerPids);
        appRunningManager_Record0100->GetAllUIExtensionProviderPid(providerPid, hostPids);

        appRunningManager_Record0100->RemoveAppRunningRecordById(appRecord->GetRecordId());
    };

    SET_THREAD_NUM(100);
    GTEST_RUN_TASK(task);
}

/**
 * @tc.name: UIExtensionReleationship_0100
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0100, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityId = 1000;
    pid_t hostPid = 1001;
    pid_t providerPid = 1002;
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPid), ERR_OK);
    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPid), ERR_OK);

    // Get after remove.
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0200
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0200, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityId = 1000;
    pid_t hostPid = 1001;
    pid_t providerPid = 1001; // same with host pid, ie uiability and uiextensionability in same process.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPid), ERR_OK);
    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPid), ERR_OK);

    // Get after remove.
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0300
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0300, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPid = 1001;
    pid_t providerPidA = 2001;
    pid_t providerPidB = 2002; // a root host start two uiextensionability.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPid, providerPidA);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPid, providerPidB);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidA, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidB, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 2);
    EXPECT_EQ(providerPids[0], providerPidA);
    EXPECT_EQ(providerPids[1], providerPidB);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPidA), ERR_OK);

    // Get after remove one provider
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidA, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidB, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPid);
    hostPids.clear();

    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPidB);
}

/**
 * @tc.name: UIExtensionReleationship_0400
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0400, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPid = 1001;
    pid_t providerPidA = 2001;
    pid_t providerPidB = 2002; // a root host start two uiextensionability.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPid, providerPidA);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPid, providerPidB);

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPid), ERR_OK);

    // Get after remove one provider
    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidA, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPidB, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(hostPid, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0500
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0500, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPidA = 1001;
    pid_t hostPidB = 1002;
    pid_t providerPid = 2001; // a uiextensionability has two root host info.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPidA, providerPid);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPidB, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 2);
    EXPECT_EQ(hostPids[0], hostPidA);
    EXPECT_EQ(hostPids[1], hostPidB);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(providerPid), ERR_OK);

    // Get after remove one provider
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 0);

    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0600
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0600, TestSize.Level1)
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager, nullptr);

    int32_t uiExtensionAbilityIdA = 1;
    int32_t uiExtensionAbilityIdB = 2;
    pid_t hostPidA = 1001;
    pid_t hostPidB = 1002;
    pid_t providerPid = 2001; // a uiextensionability has two root host info.
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdA, hostPidA, providerPid);
    appRunningManager->AddUIExtensionLauncherItem(uiExtensionAbilityIdB, hostPidB, providerPid);

    std::vector<pid_t> hostPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 2);
    EXPECT_EQ(hostPids[0], hostPidA);
    EXPECT_EQ(hostPids[1], hostPidB);
    hostPids.clear();

    std::vector<pid_t> providerPids;
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItem(hostPidA), ERR_OK);

    // Get after remove one provider
    EXPECT_EQ(appRunningManager->GetAllUIExtensionRootHostPid(providerPid, hostPids), ERR_OK);
    EXPECT_EQ(hostPids.size(), 1);
    EXPECT_EQ(hostPids[0], hostPidB); // cause host pid A has removed.

    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidA, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 1);
    EXPECT_EQ(providerPids[0], providerPid);
    providerPids.clear();

    EXPECT_EQ(appRunningManager->RemoveUIExtensionLauncherItemById(uiExtensionAbilityIdB), ERR_OK);
    EXPECT_EQ(appRunningManager->GetAllUIExtensionProviderPid(hostPidB, providerPids), ERR_OK);
    EXPECT_EQ(providerPids.size(), 0);
}

/**
 * @tc.name: UIExtensionReleationship_0700
 * @tc.desc: Root host pid and provider pid map test.
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerTest, UIExtensionReleationship_0700, TestSize.Level1)
{
    static std::shared_ptr<AppRunningManager> appRunningManager_Rel0700 = std::make_shared<AppRunningManager>();
    ASSERT_NE(appRunningManager_Rel0700, nullptr);
    auto task = []() {
        TAG_LOGI(AAFwkTag::TEST, "Running in thread %{public}d.", gettid());
        static int32_t uiExtensionAbilityId = 1;
        static pid_t hostPid = 100000;
        static pid_t providerPid = 200000;
        appRunningManager_Rel0700->AddUIExtensionLauncherItem(uiExtensionAbilityId, hostPid, providerPid);
        uiExtensionAbilityId++;
        hostPid += 2;
        providerPid += 3;

        appRunningManager_Rel0700->RemoveUIExtensionLauncherItemById(uiExtensionAbilityId);
        appRunningManager_Rel0700->RemoveUIExtensionLauncherItem(hostPid);
        appRunningManager_Rel0700->RemoveUIExtensionLauncherItem(providerPid);

        std::vector<pid_t> hostPids;
        std::vector<pid_t> providerPids;
        appRunningManager_Rel0700->GetAllUIExtensionRootHostPid(hostPid, providerPids);
        appRunningManager_Rel0700->GetAllUIExtensionProviderPid(providerPid, hostPids);
    };

    SET_THREAD_NUM(100);
    GTEST_RUN_TASK(task);
}
} // namespace AppExecFwk
} // namespace OHOS
