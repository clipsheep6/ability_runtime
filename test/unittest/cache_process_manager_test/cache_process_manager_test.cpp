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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "cache_process_manager.h"
#undef private
#undef protected
#include "mock_app_mgr_service_inner.h"
#include "mock_ability_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

const std::string ABILITY_RECORD_NAME = "Ability_Name_Z";

class CacheProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<AppRunningRecord> MockAppRecord(int apiLevel = 12);
};


void CacheProcessManagerTest::SetUpTestCase(void)
{}

void CacheProcessManagerTest::TearDownTestCase(void)
{}

void CacheProcessManagerTest::SetUp()
{}

void CacheProcessManagerTest::TearDown()
{}

std::shared_ptr<AppRunningRecord> CacheProcessManagerTest::MockAppRecord(int apiLevel)
{
    ApplicationInfo appInfo;
    appInfo.accessTokenId = 1;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>(appInfo);
    info->accessTokenId = 1;
    info->apiTargetVersion = apiLevel;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, 0, "process");
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    priorityObject->SetPid(1);
    appRecord->priorityObject_ = priorityObject;
    appRecord->SetUid(1);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appRecord->SetContinuousTaskAppState(false);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appRecord->SetRequestProcCode(1);
    appRecord->isFocused_ = false;
    return appRecord;
}
/**
 * @tc.name: AppRunningManager_QueryEnableProcessCache_0100
 * @tc.desc: Test the state of QueryEnableProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_QueryEnableProcessCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 0;
    EXPECT_EQ(cacheProcMgr->QueryEnableProcessCache(), false);
}

/**
 * @tc.name: AppRunningManager_QueryEnableProcessCache_0200
 * @tc.desc: Test the state of QueryEnableProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_QueryEnableProcessCache_0200, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 100;
    EXPECT_EQ(cacheProcMgr->QueryEnableProcessCache(), true);
}

/**
 * @tc.name: AppRunningManager_SetAppMgr_0100
 * @tc.desc: Test the state of SetAppMgr
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_SetAppMgr_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_NE(appMgrInner, nullptr);
}

/**
 * @tc.name: AppRunningManager_PenddingCacheProcess_0100
 * @tc.desc: Test the state of PenddingCacheProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_PenddingCacheProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // keepalive not allowed
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord), false);
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord2 = nullptr;
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), false);
    // pending without real shrink
    auto appRecord3 = MockAppRecord();
    EXPECT_NE(appRecord3, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord3), true);
    // pending without real shrink
    auto appRecord4 = MockAppRecord();
    EXPECT_NE(appRecord4, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord4), true);
    // pending with shrinking
    auto appRecord5 = MockAppRecord();
    EXPECT_NE(appRecord5, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord5), true);
}

/**
 * @tc.name: AppRunningManager_CheckAndCacheProcess_0100
 * @tc.desc: Test the state of CheckAndCacheProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_CheckAndCacheProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord), false);
    // not cached
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord2), false);
    // cached but no appMgrSerInner
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord2), true);
    // all normal
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->CheckAndCacheProcess(appRecord2), true);
}

/**
 * @tc.name: AppRunningManager_IsCachedProcess_0100
 * @tc.desc: Test the state of IsCachedProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsCachedProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    EXPECT_EQ(cacheProcMgr->IsCachedProcess(appRecord), false);
    // all normal
    auto appRecord2 = MockAppRecord();
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    EXPECT_EQ(cacheProcMgr->IsCachedProcess(appRecord2), true);
}

/**
 * @tc.name: AppRunningManager_OnProcessKilled_0100
 * @tc.desc: Test the state of OnProcessKilled
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_OnProcessKilled_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    cacheProcMgr->OnProcessKilled(appRecord);
    // not cached
    auto appRecord2 = MockAppRecord();
    cacheProcMgr->OnProcessKilled(appRecord2);
    // cached, but appMgr is nullptr
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->OnProcessKilled(appRecord2);
    // all normal
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->OnProcessKilled(appRecord2);
}

/**
 * @tc.name: AppRunningManager_ReuseCachedProcess_0100
 * @tc.desc: Test the state of ReuseCachedProcess
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_ReuseCachedProcess_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    cacheProcMgr->ReuseCachedProcess(appRecord);
    // not cached
    auto appRecord2 = MockAppRecord();
    cacheProcMgr->ReuseCachedProcess(appRecord2);
    // no appMgr
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->ReuseCachedProcess(appRecord2);
    // all normal
    auto appMgrInner = std::make_shared<MockAppMgrServiceInner>();
    cacheProcMgr->SetAppMgr(appMgrInner);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    cacheProcMgr->OnProcessKilled(appRecord2);
}

/**
 * @tc.name: AppRunningManager_IsAppSupportProcessCache_0100
 * @tc.desc: Test the state of IsAppSupportProcessCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppSupportProcessCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    // nullptr not allowed
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord), false);
    // API earlier than 12 not allowed
    auto appRecord2 = MockAppRecord(11);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord2), false);
    // different supportState
    auto appRecord3 = MockAppRecord(12);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), true);
    appRecord3->SetSupportedProcessCache(true);
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), true);
    appRecord3->procCacheSupportState_ = SupportProcessCacheState::NOT_SUPPORT;
    EXPECT_EQ(cacheProcMgr->IsAppSupportProcessCache(appRecord3), false);
}

/**
 * @tc.name: AppRunningManager_RefreshCacheNum_0100
 * @tc.desc: Test the state of RefreshCacheNum
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_RefreshCacheNum_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    cacheProcMgr->RefreshCacheNum();
}

/**
 * @tc.name: AppRunningManager_GetCurrentCachedProcNum_0100
 * @tc.desc: Test the state of GetCurrentCachedProcNum
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_GetCurrentCachedProcNum_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord), true);
    EXPECT_EQ(cacheProcMgr->GetCurrentCachedProcNum(), 1);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);
    EXPECT_EQ(cacheProcMgr->PenddingCacheProcess(appRecord2), true);
    EXPECT_EQ(cacheProcMgr->GetCurrentCachedProcNum(), 2);
}

/**
 * @tc.name: AppRunningManager_KillProcessByRecord_0100
 * @tc.desc: Test the state of KillProcessByRecord
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_KillProcessByRecord_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(cacheProcMgr->KillProcessByRecord(appRecord), false);
}

/**
 * @tc.name: AppRunningManager_IsAppShouldCache_0100
 * @tc.desc: Test the state of IsAppShouldCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppShouldCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);

    // nullptr check
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(nullptr), false);

    // Not enable
    cacheProcMgr->maxProcCacheNum_ = 0;
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(nullptr), false);

    // Cached app
    cacheProcMgr->maxProcCacheNum_ = 2;
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord);
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(appRecord), true);

    // App not support cache
    cacheProcMgr->cachedAppRecordQueue_.clear();
    appRecord->procCacheSupportState_ = SupportProcessCacheState::NOT_SUPPORT;
    EXPECT_EQ(cacheProcMgr->IsAppShouldCache(appRecord), false);
}

/**
 * @tc.name: AppRunningManager_IsAppAbilitiesEmpty_0100
 * @tc.desc: Test the state of IsAppAbilitiesEmpty
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_IsAppAbilitiesEmpty_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    // Abilities empty
    auto appRecord = MockAppRecord();
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(cacheProcMgr->IsAppAbilitiesEmpty(appRecord), true);
    
    // Not empty
    auto caseAbilityInfo = std::make_shared<AbilityInfo>();
    caseAbilityInfo->name = ABILITY_RECORD_NAME;
    sptr<IRemoteObject> token = new MockAbilityToken();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "Module";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = "com.ohos.test.helloworld";
    appRecord->AddModule(appInfo, caseAbilityInfo, token, hapModuleInfo, nullptr, 0);
    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName,
        hapModuleInfo.moduleName);
    auto caseAbilityRunningRecord = moduleRecord->AddAbility(token, caseAbilityInfo, nullptr, 0);
    EXPECT_TRUE(caseAbilityRunningRecord == nullptr);
    EXPECT_EQ(cacheProcMgr->IsAppAbilitiesEmpty(appRecord), false);
}

/**
 * @tc.name: AppRunningManager_ShrinkAndKillCache_0100
 * @tc.desc: Test the state of ShrinkAndKillCache
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_ShrinkAndKillCache_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto appRecord1 = MockAppRecord();
    EXPECT_NE(appRecord1, nullptr);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);
    auto appRecord3 = MockAppRecord();
    EXPECT_NE(appRecord3, nullptr);

    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord1);
    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord2);
    cacheProcMgr->cachedAppRecordQueue_.push_back(appRecord3);

    cacheProcMgr->ShrinkAndKillCache();
}

/**
 * @tc.name: AppRunningManager_PrintCacheQueue_0100
 * @tc.desc: Test the state of PrintCacheQueue
 * @tc.type: FUNC
 */
HWTEST_F(CacheProcessManagerTest, CacheProcessManager_PrintCacheQueue_0100, TestSize.Level1)
{
    auto cacheProcMgr = std::make_shared<CacheProcessManager>();
    EXPECT_NE(cacheProcMgr, nullptr);
    cacheProcMgr->maxProcCacheNum_ = 2;

    auto appRecord1 = MockAppRecord();
    EXPECT_NE(appRecord1, nullptr);
    auto appRecord2 = MockAppRecord();
    EXPECT_NE(appRecord2, nullptr);

    EXPECT_NE(cacheProcMgr->PrintCacheQueue(), "");
}
} // namespace AppExecFwk
} // namespace OHOS