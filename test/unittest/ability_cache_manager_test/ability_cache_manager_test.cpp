/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <memory>

#include "ability_connect_manager.h"

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "ability_cache_manager.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class AbilityCacheManagerTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityCacheManagerTest::SetUpTestCase(void)
{}

void AbilityCacheManagerTest::TearDownTestCase(void)
{}

void AbilityCacheManagerTest::SetUp()
{}

void AbilityCacheManagerTest::TearDown()
{}

/**
 * @tc.name: AbilityCacheManagerPutAndGetTest_001
 * @tc.desc: Put a single ability record into cache and get it back with default ability name in want
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerPutAndGetTest_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 1;
    Want want;
    ElementName element("", "", "ability", "");
    want.SetElement(element);
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    int recId = abilityRecord_->GetRecordId();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    ElementName elementGet("", "", "AnotherAbility", "");
    abilityRequest.want.SetElement(elementGet);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr); // incorrect ability name
    abilityRequest.want.SetElement(element);
    abilityRequest.abilityInfo.moduleName = "wrongModuleName";
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr); // incorrect module name
    abilityRequest.abilityInfo.moduleName = "TestModuleName";
    abilityRequest.appInfo.accessTokenId = 0;
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr); // incorrect access token Id
    abilityRequest.appInfo.accessTokenId = 1;
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(rec->GetWant().GetElement().GetAbilityName(), "ability");
    EXPECT_EQ(rec->GetRecordId(), recId);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr);
}

/**
 * @tc.name: AbilityCacheManagerRemoveTest_001
 * @tc.desc: Put a single ability record into cache and remove with wrong accessTokenId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerRemoveTest_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 1;
    Want want;
    ElementName element("", "", "ability", "");
    want.SetElement(element);
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    int recId = abilityRecord_->GetRecordId();
    EXPECT_EQ(rec, nullptr);
    applicationInfo.accessTokenId = 0;
    auto removeRec = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(removeRec);
    applicationInfo.accessTokenId = 1;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    abilityRequest.want.SetElement(element);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(rec->GetWant().GetElement().GetAbilityName(), "ability");
    EXPECT_EQ(rec->GetRecordId(), recId);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr);
}

/**
 * @tc.name: AbilityCacheManagerRemoveTest_002
 * @tc.desc: Put a single ability record into cache and remove with wrong access module name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerRemoveTest_002, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 1;
    Want want;
    ElementName element("", "", "ability", "");
    want.SetElement(element);
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    int recId = abilityRecord_->GetRecordId();
    EXPECT_EQ(rec, nullptr);
    abilityInfo.moduleName = "WrongModuleName";
    auto removeRec = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(removeRec);
    abilityInfo.moduleName = "TestModuleName";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    abilityRequest.want.SetElement(element);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(rec->GetWant().GetElement().GetAbilityName(), "ability");
    EXPECT_EQ(rec->GetRecordId(), recId);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr);
}

/**
 * @tc.name: AbilityCacheManagerRemoveTest_003
 * @tc.desc: Put a single ability record into cache and remove with wrong ability name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerRemoveTest_003, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 1;
    Want want;
    ElementName element("", "", "ability", "");
    want.SetElement(element);
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    int recId = abilityRecord_->GetRecordId();
    EXPECT_EQ(rec, nullptr);
    ElementName wrongeElement("", "", "wrongAbility", "");
    want.SetElement(wrongeElement);
    auto removeRec = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(removeRec);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    abilityRequest.want.SetElement(element);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(rec->GetWant().GetElement().GetAbilityName(), "ability");
    EXPECT_EQ(rec->GetRecordId(), recId);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr);
}

/**
 * @tc.name: AbilityCacheManagerPutTest_001
 * @tc.desc: Put multi ability records so that dev cache is full
 *           and eliminate the first ability record in dev cache and use
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerPutTest_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(2, 1);
    OHOS::AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.moduleName = "TestModuleName1";
    abilityInfo1.bundleName = "TestBundleName1";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo1;
    applicationInfo1.accessTokenId = 1;
    Want want;
    auto abilityRecord1 = std::make_shared<AbilityRecord>(want, abilityInfo1, applicationInfo1);
    abilityRecord1->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord1);
    EXPECT_EQ(rec, nullptr);
    int recId1 = abilityRecord1->GetRecordId();
    OHOS::AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.moduleName = "TestModuleName2";
    abilityInfo2.bundleName = "TestBundleName2";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo2;
    applicationInfo2.accessTokenId = 2;
    auto abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo2, applicationInfo2);
    abilityRecord2->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord2);
    EXPECT_EQ(rec, nullptr);

    OHOS::AppExecFwk::AbilityInfo abilityInfo3;
    abilityInfo3.moduleName = "TestModuleName3";
    abilityInfo3.bundleName = "TestBundleName3";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo3;
    applicationInfo3.accessTokenId = 3;
    auto abilityRecord3 = std::make_shared<AbilityRecord>(want, abilityInfo3, applicationInfo3);
    abilityRecord3->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord3);

    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo1.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo1.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo1.bundleName);
    EXPECT_EQ(rec->GetRecordId(), recId1);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord2);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord3);
}

/**
 * @tc.name: AbilityCacheManagerPutTest_002
 * @tc.desc: Put multi ability records so that proc cache is full and eliminate the first ability record in proc cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerPutTest_002, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(3, 1);
    OHOS::AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.moduleName = "TestModuleName1";
    abilityInfo1.bundleName = "TestBundleName1";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo1;
    applicationInfo1.accessTokenId = 1;
    Want want;
    auto abilityRecord1 = std::make_shared<AbilityRecord>(want, abilityInfo1, applicationInfo1);
    abilityRecord1->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord1);
    EXPECT_EQ(rec, nullptr);

    OHOS::AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.moduleName = "TestModuleName2";
    abilityInfo2.bundleName = "TestBundleName2";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo2;
    applicationInfo2.accessTokenId = 2;
    auto abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo2, applicationInfo2);
    abilityRecord2->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord2);
    EXPECT_EQ(rec, nullptr);
    int recId2 = abilityRecord2->GetRecordId();
    OHOS::AppExecFwk::AbilityInfo abilityInfo3;
    abilityInfo3.moduleName = "TestModuleName3";
    abilityInfo3.bundleName = "TestBundleName3";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo3;
    applicationInfo3.accessTokenId = 2;
    auto abilityRecord3 = std::make_shared<AbilityRecord>(want, abilityInfo3, applicationInfo3);
    abilityRecord3->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord3);

    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo2.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo2.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo2.bundleName);
    EXPECT_EQ(rec->GetRecordId(), recId2);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord1);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord3);
}

/**
 * @tc.name: AbilityCacheManagerPutTest_003
 * @tc.desc: Put multi ability records so that proc cache are full at the same time
 *           and eliminate the first ability record in proc cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerPutTest_003, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(3, 1);
    OHOS::AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.moduleName = "TestModuleName1";
    abilityInfo1.bundleName = "TestBundleName1";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo1;
    applicationInfo1.accessTokenId = 1;
    Want want;
    auto abilityRecord1 = std::make_shared<AbilityRecord>(want, abilityInfo1, applicationInfo1);
    abilityRecord1->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord1);
    EXPECT_EQ(rec, nullptr);

    OHOS::AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.moduleName = "TestModuleName2";
    abilityInfo2.bundleName = "TestBundleName2";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo2;
    applicationInfo2.accessTokenId = 2;

    auto abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo2, applicationInfo2);
    abilityRecord2->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord2);
    EXPECT_EQ(rec, nullptr);

    OHOS::AppExecFwk::AbilityInfo abilityInfo3;
    abilityInfo3.moduleName = "TestModuleName3";
    abilityInfo3.bundleName = "TestBundleName3";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo3;
    applicationInfo3.accessTokenId = 3;
    auto abilityRecord3 = std::make_shared<AbilityRecord>(want, abilityInfo3, applicationInfo3);
    abilityRecord3->Init();
    int recId3 = abilityRecord3->GetRecordId();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord3);
    EXPECT_EQ(rec, nullptr);

    OHOS::AppExecFwk::AbilityInfo abilityInfo4;
    abilityInfo4.moduleName = "TestModuleName4";
    abilityInfo4.bundleName = "TestBundleName4";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo4;
    applicationInfo4.accessTokenId = 3;

    auto abilityRecord4 = std::make_shared<AbilityRecord>(want, abilityInfo4, applicationInfo4);
    abilityRecord4->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord4);
    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo3.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo3.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo3.bundleName);
    EXPECT_EQ(rec->GetRecordId(), recId3);

    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord1);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord2);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord4);
}

/**
 * @tc.name: AbilityCacheManagerPutTest_004
 * @tc.desc: Put multi ability records so that dev cache is full and eliminate the first ability record in dev cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerPutTest_004, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(2, 1);
    OHOS::AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.moduleName = "TestModuleName1";
    abilityInfo1.bundleName = "TestBundleName1";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo1;
    applicationInfo1.accessTokenId = 1;
    Want want;
    auto abilityRecord1 = std::make_shared<AbilityRecord>(want, abilityInfo1, applicationInfo1);
    abilityRecord1->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord1);
    EXPECT_EQ(rec, nullptr);
    int recId1 = abilityRecord1->GetRecordId();
    OHOS::AppExecFwk::AbilityInfo abilityInfo2;
    abilityInfo2.moduleName = "TestModuleName2";
    abilityInfo2.bundleName = "TestBundleName2";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo2;
    applicationInfo2.accessTokenId = 2;
    auto abilityRecord2 = std::make_shared<AbilityRecord>(want, abilityInfo2, applicationInfo2);
    abilityRecord2->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord2);
    EXPECT_EQ(rec, nullptr);

    OHOS::AppExecFwk::AbilityInfo abilityInfo3;
    abilityInfo3.moduleName = "TestModuleName3";
    abilityInfo3.bundleName = "TestBundleName3";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo3;
    applicationInfo3.accessTokenId = 3;
    auto abilityRecord3 = std::make_shared<AbilityRecord>(want, abilityInfo3, applicationInfo3);
    abilityRecord3->Init();
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord3);

    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo1.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo1.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo1.bundleName);
    EXPECT_EQ(rec->GetRecordId(), recId1);

    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord2);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord3);
}

/**
 * @tc.name: AbilityCacheManagerPutAndRemoveTest_001
 * @tc.desc: Put a single ability record into cache and remove it then get a nullptr with request
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerPutAndRemoveTest_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    Want want;
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord_);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr);
    OHOS::AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo.moduleName = "TestModuleName1";
    abilityInfo.bundleName = "TestBundleName1";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo1;
    applicationInfo.accessTokenId = 1;
    auto abilityRecord1 = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord1->Init();
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord1);
}

/**
 * @tc.name: AbilityCacheManagerFindByToken_001
 * @tc.desc: Put a single ability record into cache and find it, find will not remove cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerFindByToken_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    Want want;
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    int recId = abilityRecord_->GetRecordId();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    auto recordFind = OHOS::AAFwk::AbilityCacheManager::GetInstance().FindRecordByToken(abilityRecord_->GetToken());
    EXPECT_EQ(recordFind->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recordFind->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recordFind->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recordFind->GetRecordId(), recId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    auto recGet = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(recGet->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recGet->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recGet->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recGet->GetRecordId(), recId);
}

/**
 * @tc.name: AbilityCacheManagerGetAbilityList_001
 * @tc.desc: Put a single ability record into cache and get ability list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerGetAbilityList_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    Want want;
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    int recId = abilityRecord_->GetRecordId();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    auto abilityList = OHOS::AAFwk::AbilityCacheManager::GetInstance().GetAbilityList();
    EXPECT_EQ(abilityList.size(), 1);
    auto recordFind = *(abilityList.begin());
    EXPECT_EQ(recordFind->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recordFind->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recordFind->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recordFind->GetRecordId(), recId);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Remove(abilityRecord_);
}

/**
 * @tc.name: AbilityCacheManagerFindBySessionId_001
 * @tc.desc: Put a single ability record into cache and find it, find will not remove cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerFindBySessionId_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    std::string sessionId = "TestSessionId";
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    Want want;
    want.SetParam(Want::PARAM_ASSERT_FAULT_SESSION_ID, sessionId);
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    int recId = abilityRecord_->GetRecordId();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    auto recordFind = OHOS::AAFwk::AbilityCacheManager::GetInstance().FindRecordBySessionId(sessionId);
    EXPECT_EQ(recordFind->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recordFind->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recordFind->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recordFind->GetRecordId(), recId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    auto recGet = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(recGet->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recGet->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recGet->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recGet->GetRecordId(), recId);
}

/**
 * @tc.name: AbilityCacheManagerFindByServiceKey_001
 * @tc.desc: Put a single ability record into cache and find it, find will not remove cache
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerFindByServiceKey_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    Want want;
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    int recId = abilityRecord_->GetRecordId();
    std::string serviceKey = abilityRecord_->GetURI();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    auto recordFind = OHOS::AAFwk::AbilityCacheManager::GetInstance().FindRecordByServiceKey(serviceKey);
    EXPECT_EQ(recordFind->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recordFind->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recordFind->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recordFind->GetRecordId(), recId);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    auto recGet = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(recGet->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(recGet->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(recGet->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(recGet->GetRecordId(), recId);
}

/**
 * @tc.name: AbilityCacheManagerSignRestartAppFlag_001
 * @tc.desc: Put a single ability record into cache and sign restart app flag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerSignRestartAppFlag_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    applicationInfo.bundleName = abilityInfo.bundleName;
    Want want;
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    int recId = abilityRecord_->GetRecordId();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().SignRestartAppFlag(applicationInfo.bundleName);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    auto recordFind = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(recordFind->GetRestartAppFlag(), true);
}

/**
 * @tc.name: AbilityCacheManagerDeleteInvalidRecord_001
 * @tc.desc: Put a single ability record into cache and delete it by bundleName
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityCacheManagerTest, AbilityCacheManagerDeleteInvalidRecord_001, TestSize.Level0)
{
    OHOS::AAFwk::AbilityCacheManager::GetInstance().Init(10, 5);
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.moduleName = "TestModuleName";
    abilityInfo.bundleName = "TestBundleName";
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.accessTokenId = 0;
    Want want;
    auto abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord_->Init();
    int recId = abilityRecord_->GetRecordId();
    std::shared_ptr<AbilityRecord> rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Put(abilityRecord_);
    EXPECT_EQ(rec, nullptr);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = applicationInfo;
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec->GetApplicationInfo().accessTokenId, applicationInfo.accessTokenId);
    EXPECT_EQ(rec->GetAbilityInfo().moduleName, abilityInfo.moduleName);
    EXPECT_EQ(rec->GetAbilityInfo().bundleName, abilityInfo.bundleName);
    EXPECT_EQ(rec->GetRecordId(), recId);
    OHOS::AAFwk::AbilityCacheManager::GetInstance().DeleteInvalidServiceRecord(abilityInfo.bundleName);
    rec = OHOS::AAFwk::AbilityCacheManager::GetInstance().Get(abilityRequest);
    EXPECT_EQ(rec, nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS
