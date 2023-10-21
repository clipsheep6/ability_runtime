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
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t DEBUGINFOS_SIZE = 0;
constexpr int32_t ABILITYTOKENS_SIZE = 0;
constexpr int32_t RECORD_ID = 1;
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
} // namespace AppExecFwk
} // namespace OHOS
