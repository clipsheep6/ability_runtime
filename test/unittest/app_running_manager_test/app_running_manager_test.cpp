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
#include "app_running_manager.h"
#include "child_process_record.h"
#undef private
#include "hilog_wrapper.h"
#include "window_visibility_info.h"

using namespace testing;
using namespace testing::ext;

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
    HILOG_DEBUG("AppRunningManager_GetAppRunningRecordByChildProcessPid_0100 called.");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    
    auto appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "com.example.child");
    auto childRecord = ChildProcessRecord::CreateChildProcessRecord(PID, "./ets/AProcess.ts", appRecord);
    pid_t childPid = 201;
    childRecord->pid_ = childPid;
    appRecord->AddChildProcessRecord(childPid, childRecord);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRecord));

    auto record = appRunningManager->GetAppRunningRecordByChildProcessPid(childPid);
    EXPECT_NE(record, nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS
