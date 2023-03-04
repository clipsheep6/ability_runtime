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

#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "data_ability_manager.h"
#include "app_scheduler.h"
#undef private
#undef protected

#include "ability_scheduler_mock.h"
#include "mock_app_mgr_client.h"
#include "ability_manager_errors.h"

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace {
const std::string STRING_DATA_ABILITY = "com.example.data_ability";
constexpr size_t SIZE_ONE = 1;
}  // namespace

namespace OHOS {
namespace AAFwk {
class DataAbilityManagerTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    sptr<AbilitySchedulerMock> abilitySchedulerMock_{ nullptr };
    AbilityRequest abilityRequest_{};
    std::shared_ptr<AbilityRecord> abilityRecordClient_{ nullptr };
    OHOS::AAFwk::AbilityState abilityState_{};
};

void DataAbilityManagerTest::SetUpTestCase(void)
{}
void DataAbilityManagerTest::TearDownTestCase(void)
{}

void DataAbilityManagerTest::SetUp(void)
{
    if (abilitySchedulerMock_ == nullptr) {
        abilitySchedulerMock_ = new AbilitySchedulerMock();
    }

    abilityRequest_.appInfo.bundleName = "com.test.data_ability";
    abilityRequest_.appInfo.name = "com.test.data_ability";
    abilityRequest_.abilityInfo.name = "DataAbilityHiworld";
    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityRequest_.abilityInfo.bundleName = "com.test.data_ability";
    abilityRequest_.abilityInfo.deviceId = "device";

    if (abilityRecordClient_ == nullptr) {
        OHOS::AppExecFwk::AbilityInfo abilityInfo;
        abilityInfo.name = "DataAbilityClient";
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.bundleName = "com.test.request";
        abilityInfo.deviceId = "device";
        OHOS::AppExecFwk::ApplicationInfo applicationInfo;
        applicationInfo.bundleName = "com.test.request";
        applicationInfo.name = "com.test.request";
        const Want want;
        abilityRecordClient_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
        abilityRecordClient_->Init();
    }
    abilityState_ = INITIAL;
}

void DataAbilityManagerTest::TearDown(void)
{
    abilitySchedulerMock_.clear();
}

/**
 * @tc.name: AaFwk_DataAbilityManager_DumpSysState_0100
 * @tc.desc: DumpSysState with no args
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_0100, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_DumpSysState_0100 start");

    AbilityRequest abilityRequest;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    dataAbilityRecord->ability_ = abilityRecord;

    auto dataAbilityManager = std::make_unique<DataAbilityManager>();
    dataAbilityManager->dataAbilityRecordsLoaded_ = { {STRING_DATA_ABILITY, dataAbilityRecord} };

    std::vector<std::string> info;
    bool isClient = false;
    std::string args = "";
    dataAbilityManager->DumpSysState(info, isClient, args);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("AaFwk_DataAbilityManager_DumpSysState_0100 end");
}

/**
 * @tc.name: AaFwk_DataAbilityManager_DumpSysState_0200
 * @tc.desc: DumpSysState with args
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_0200, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_DumpSysState_0200 start");

    AbilityRequest abilityRequest;
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);

    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    dataAbilityRecord->ability_ = abilityRecord;

    auto dataAbilityManager = std::make_unique<DataAbilityManager>();
    dataAbilityManager->dataAbilityRecordsLoaded_ = { {STRING_DATA_ABILITY, dataAbilityRecord} };

    std::vector<std::string> info;
    bool isClient = false;
    std::string args = STRING_DATA_ABILITY;
    dataAbilityManager->DumpSysState(info, isClient, args);
    EXPECT_GT(info.size(), SIZE_ONE);

    HILOG_INFO("info.size() = %{public}zu", info.size());
    for (auto item : info) {
        HILOG_INFO("item = %{public}s", item.c_str());
    }

    HILOG_INFO("AaFwk_DataAbilityManager_DumpSysState_0200 end");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Normal Flow
 * FunctionPoints: DataAbilityManager simple flow.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify the DataAbilityManager simple flow.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Flow_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Flow_001 start.");

    std::shared_ptr<DataAbilityManager> dataAbilityManager = std::make_shared<DataAbilityManager>();
    std::unique_ptr<MockAppMgrClient> mockAppMgrClient = std::make_unique<MockAppMgrClient>();

    // mock AppScheduler
    DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_ = std::move(mockAppMgrClient);

    auto func = [this, &dataAbilityManager]() {
        usleep(200 * 1000);  // 200 ms
        sptr<IRemoteObject> tokenAsyn =
            (reinterpret_cast<MockAppMgrClient*>(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_.get()))
            ->GetToken();
        dataAbilityManager->AttachAbilityThread(abilitySchedulerMock_, tokenAsyn);
        dataAbilityManager->AbilityTransitionDone(tokenAsyn, ACTIVE);
    };

    std::thread(func).detach();
    EXPECT_CALL(*abilitySchedulerMock_, ScheduleAbilityTransaction(_, _, _)).Times(1);
    EXPECT_NE(dataAbilityManager->Acquire(abilityRequest_, true, abilityRecordClient_->GetToken(), false), nullptr);

    sptr<IRemoteObject> token =
        (reinterpret_cast<MockAppMgrClient*>(DelayedSingleton<AppScheduler>::GetInstance()->appMgrClient_.get()))
        ->GetToken();
    std::shared_ptr<AbilityRecord> abilityRecord = Token::GetAbilityRecordByToken(token);
    EXPECT_TRUE(abilityRecord);

    // existing ability record
    EXPECT_NE(dataAbilityManager->GetAbilityRecordByToken(token), nullptr);
    EXPECT_NE(dataAbilityManager->GetAbilityRecordByScheduler(abilitySchedulerMock_), nullptr);
    EXPECT_NE(dataAbilityManager->GetAbilityRecordById(abilityRecord->GetRecordId()), nullptr);

    // ability died, clear data ability record
    dataAbilityManager->OnAbilityDied(abilityRecord);

    // ability has released
    EXPECT_EQ(dataAbilityManager->GetAbilityRecordByToken(token), nullptr);
    EXPECT_EQ(dataAbilityManager->GetAbilityRecordByScheduler(abilitySchedulerMock_), nullptr);
    EXPECT_EQ(dataAbilityManager->GetAbilityRecordById(abilityRecord->GetRecordId()), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Flow_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire parameter is nullptr.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->Acquire(abilityRequest_, true, nullptr, false), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire parameter ability type is not data
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_002, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_002 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    // page ability type
    abilityRequest_.abilityInfo.type = AbilityType::PAGE;
    EXPECT_EQ(dataAbilityManager->Acquire(abilityRequest_, true, abilityRecordClient_->GetToken(), false), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_002 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire parameter appinfo bundlename empty
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_003, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_003 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    // appinfo bundle name empty
    abilityRequest_.appInfo.bundleName = "";
    EXPECT_EQ(dataAbilityManager->Acquire(abilityRequest_, true, abilityRecordClient_->GetToken(), false), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_003 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire parameter ability name empty
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_004, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_004 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    // ability name empty
    abilityRequest_.abilityInfo.name = "";
    EXPECT_EQ(dataAbilityManager->Acquire(abilityRequest_, true, abilityRecordClient_->GetToken(), false), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_004 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire parameter same bundle name and ability name
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_005, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_005 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    // same bundle name and ability name
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = abilityRequest_.abilityInfo.name;
    abilityInfo.type = AbilityType::PAGE;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleName = abilityRequest_.appInfo.bundleName;
    applicationInfo.name = abilityRequest_.appInfo.name;
    const Want want;
    std::shared_ptr abilityRecordClient = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecordClient->Init();

    EXPECT_EQ(dataAbilityManager->Acquire(abilityRequest_, true, abilityRecordClient->GetToken(), false), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_005 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire waitforloaded timeout.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_006, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_006 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->Acquire(abilityRequest_, true, abilityRecordClient_->GetToken(), false), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_Acquire_006 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire waitforloaded timeout.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_007, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "";
    auto res = dataAbilityManager->Acquire(abilityRequest, true, abilityRecordClient_->GetToken(), false);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire waitforloaded timeout.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_008, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecordClient_;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    dataAbilityRecord->scheduler_ = nullptr;
    auto res = dataAbilityManager->Acquire(abilityRequest, true, dataAbilityRecord->GetToken(), true);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Acquire
 * FunctionPoints: The parameter of function Acquire.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Acquire waitforloaded timeout.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Acquire_009, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecordClient_;
    auto res = dataAbilityManager->Acquire(abilityRequest, true, dataAbilityRecord->GetToken(), true);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Release client is nullptr
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Release_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->Release(abilitySchedulerMock_, nullptr, false), ERR_NULL_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_Release_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Release scheduler is nullptr
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_002, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Release_002 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->Release(nullptr, abilityRecordClient_->GetToken(), false), ERR_NULL_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_Release_002 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Release ability record invalid
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_003, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_Release_003 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->Release(abilitySchedulerMock_, abilityRecordClient_->GetToken(), false),
        ERR_UNKNOWN_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_Release_003 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify Release
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_004, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->abilityInfo_.visible = false;
    abilityRecord->SetAbilityState(ACTIVE);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    int res = dataAbilityManager->Release(abilitySchedulerMock_, dataAbilityRecord->GetToken(), false);
    EXPECT_EQ(res, ABILITY_VISIBLE_FALSE_DENY_REQUEST);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify Release
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_005, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->abilityInfo_.visible = true;
    abilityRecord->SetAbilityState(ACTIVE);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    int res = dataAbilityManager->Release(abilitySchedulerMock_, dataAbilityRecord->GetToken(), false);
    EXPECT_EQ(res, ERR_UNKNOWN_OBJECT);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify Release
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_006, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->abilityInfo_.visible = true;
    abilityRecord->SetAbilityState(ACTIVE);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    DataAbilityRecord::ClientInfo ci;
    ci.client = dataAbilityRecord->GetToken();
    dataAbilityRecord->clients_.push_back(ci);
    int res = dataAbilityManager->Release(abilitySchedulerMock_, dataAbilityRecord->GetToken(), false);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify Release
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_007, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = new AbilitySchedulerMock();
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    int res = dataAbilityManager->Release(abilitySchedulerMock_, dataAbilityRecord->GetToken(), false);
    EXPECT_EQ(res, ERR_UNKNOWN_OBJECT);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify Release
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_008, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = dataAbilityRecord;
    int res = dataAbilityManager->Release(abilitySchedulerMock_, dataAbilityRecord->GetToken(), false);
    EXPECT_EQ(res, ERR_UNKNOWN_OBJECT);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: Release
 * FunctionPoints: The parameter of function Release.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify Release
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_Release_009, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["bundleName.name"] = nullptr;
    int res = dataAbilityManager->Release(abilitySchedulerMock_, dataAbilityRecord->GetToken(), false);
    EXPECT_EQ(res, ERR_UNKNOWN_OBJECT);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: ContainsDataAbility
 * FunctionPoints: The parameter of function ContainsDataAbility.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify ContainsDataAbility
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_ContainsDataAbility_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityRecord->scheduler_ = new AbilitySchedulerMock();
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = dataAbilityRecord;
    dataAbilityRecord->scheduler_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["c"] = dataAbilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["d"] = nullptr;
    int res = dataAbilityManager->ContainsDataAbility(abilitySchedulerMock_);
    EXPECT_EQ(res, ERR_OK);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: AttachAbilityThread
 * FunctionPoints: The parameter of function AttachAbilityThread.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function AttachAbilityThread client is nullptr
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_AttachAbilityThread_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_AttachAbilityThread_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->AttachAbilityThread(abilitySchedulerMock_, nullptr), ERR_NULL_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_AttachAbilityThread_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: AttachAbilityThread
 * FunctionPoints: The parameter of function AttachAbilityThread.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function AttachAbilityThread scheduler is nullptr
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_AttachAbilityThread_002, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_AttachAbilityThread_002 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->AttachAbilityThread(nullptr, abilityRecordClient_->GetToken()), ERR_NULL_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_AttachAbilityThread_002 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: AttachAbilityThread
 * FunctionPoints: The parameter of function AttachAbilityThread.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function AttachAbilityThread ability record invalid
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_AttachAbilityThread_003, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_AttachAbilityThread_003 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(dataAbilityManager->AttachAbilityThread(abilitySchedulerMock_, abilityRecordClient_->GetToken()),
        ERR_UNKNOWN_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_AttachAbilityThread_003 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: AbilityTransitionDone
 * FunctionPoints: The parameter of function AbilityTransitionDone.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function AbilityTransitionDone token is nullptr
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_AbilityTransitionDone_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_AbilityTransitionDone_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    EXPECT_EQ(dataAbilityManager->AbilityTransitionDone(nullptr, INACTIVE), ERR_NULL_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_AbilityTransitionDone_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: AbilityTransitionDone
 * FunctionPoints: The parameter of function AbilityTransitionDone.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function AbilityTransitionDone ability record invalid
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_AbilityTransitionDone_002, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_AbilityTransitionDone_002 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();

    EXPECT_EQ(
        dataAbilityManager->AbilityTransitionDone(abilityRecordClient_->GetToken(), INACTIVE), ERR_UNKNOWN_OBJECT);

    HILOG_INFO("AaFwk_DataAbilityManager_AbilityTransitionDone_002 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: OnAbilityDied
 * FunctionPoints: The parameter of function OnAbilityDied.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify OnAbilityDied
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_OnAbilityDied_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->OnAbilityDied(abilityRecord);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: OnAbilityDied
 * FunctionPoints: The parameter of function OnAbilityDied.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify OnAbilityDied
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_OnAbilityDied_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->OnAbilityDied(abilityRecord);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: OnAbilityDied
 * FunctionPoints: The parameter of function OnAbilityDied.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify OnAbilityDied
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_OnAbilityDied_003, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->OnAbilityDied(abilityRecord);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: OnAppStateChanged
 * FunctionPoints: The parameter of function OnAppStateChanged.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify OnAppStateChanged
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_OnAppStateChanged_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AppInfo info;
    std::string processName = "processName";
    std::string appName = "appName";
    int32_t uid = 0;
    AppData data;
    data.appName = appName;
    data.uid = uid;
    info.processName = processName;
    info.appData.push_back(data);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord1 = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord1 = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoading_["a"] = nullptr;
    abilityRecord1->abilityInfo_.process = processName;
    abilityRecord1->applicationInfo_.bundleName = "";
    abilityRecord1->applicationInfo_.name = appName;
    abilityRecord1->abilityInfo_.applicationInfo.uid = uid;
    dataAbilityRecord1->ability_ = abilityRecord1;
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = dataAbilityRecord1;
    dataAbilityManager->dataAbilityRecordsLoading_["b"] = dataAbilityRecord1;
    auto dataAbilityRecord2 = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord2 = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord2->abilityInfo_.process = "";
    abilityRecord2->applicationInfo_.bundleName = processName;
    abilityRecord2->applicationInfo_.name = "";
    abilityRecord2->abilityInfo_.applicationInfo.uid = 0;
    dataAbilityRecord2->ability_ = abilityRecord2;
    dataAbilityManager->dataAbilityRecordsLoaded_["c"] = dataAbilityRecord2;
    dataAbilityManager->dataAbilityRecordsLoading_["c"] = dataAbilityRecord2;
    auto dataAbilityRecord3 = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord3 = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord3->abilityInfo_.process = "";
    abilityRecord3->applicationInfo_.bundleName = "";
    dataAbilityRecord3->ability_ = abilityRecord3;
    dataAbilityManager->dataAbilityRecordsLoaded_["d"] = dataAbilityRecord3;
    dataAbilityManager->dataAbilityRecordsLoading_["d"] = dataAbilityRecord3;
    auto dataAbilityRecord4 = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord4->ability_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["e"] = dataAbilityRecord4;
    dataAbilityManager->dataAbilityRecordsLoading_["e"] = dataAbilityRecord4;
    dataAbilityManager->OnAppStateChanged(info);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordById
 * FunctionPoints: The parameter of function GetAbilityRecordById.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordById
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRecordById_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    int64_t id = 0;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->recordId_ = 1;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = dataAbilityRecord;
    auto res = dataAbilityManager->GetAbilityRecordById(id);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: The parameter of function GetAbilityRecordByToken.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordByToken token is nullptr.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRecordByToken_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_GetAbilityRecordByToken_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    EXPECT_EQ(dataAbilityManager->GetAbilityRecordByToken(nullptr), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_GetAbilityRecordByToken_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: The parameter of function GetAbilityRecordByToken.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordByToken token is nullptr.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRecordByToken_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoading_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoading_["b"] = dataAbilityRecord;
    auto res = dataAbilityManager->GetAbilityRecordByToken(dataAbilityRecord->GetToken());
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordByScheduler
 * FunctionPoints: The parameter of function GetAbilityRecordByScheduler.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordByScheduler token is nullptr.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRecordByScheduler_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_GetAbilityRecordByScheduler_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    EXPECT_EQ(dataAbilityManager->GetAbilityRecordByScheduler(nullptr), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_GetAbilityRecordByScheduler_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordByScheduler
 * FunctionPoints: The parameter of function GetAbilityRecordByScheduler.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordByScheduler
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRecordByScheduler_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = new AbilitySchedulerMock();
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityRecord->scheduler_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = dataAbilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["c"] = nullptr;
    auto res = dataAbilityManager->GetAbilityRecordByScheduler(abilitySchedulerMock_);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordById
 * FunctionPoints: The parameter of function GetAbilityRecordById.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordById id is -1.
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRecordById_001, TestSize.Level1)
{
    HILOG_INFO("AaFwk_DataAbilityManager_GetAbilityRecordById_001 start.");

    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    EXPECT_EQ(dataAbilityManager->GetAbilityRecordById(-1), nullptr);

    HILOG_INFO("AaFwk_DataAbilityManager_GetAbilityRecordById_001 end.");
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: LoadLocked
 * FunctionPoints: The parameter of function LoadLocked.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function LoadLocked
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_LoadLocked_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::string name = "name";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    dataAbilityManager->dataAbilityRecordsLoading_.clear();
    auto res = dataAbilityManager->LoadLocked(name, abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: LoadLocked
 * FunctionPoints: The parameter of function LoadLocked.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function LoadLocked
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_LoadLocked_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::string name = "name";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoading_[name] = dataAbilityRecord;
    auto res = dataAbilityManager->LoadLocked(name, abilityRequest);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpLocked
 * FunctionPoints: The parameter of function DumpLocked.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpLocked
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpLocked_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    const char func[1] = "";
    int line = 0;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoading_["a"] = nullptr;
    dataAbilityManager->DumpLocked(func, line);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpLocked
 * FunctionPoints: The parameter of function DumpLocked.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpLocked
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpLocked_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    const char func[2] = "a";
    int line = 0;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->dataAbilityRecordsLoading_["a"] = dataAbilityRecord;
    dataAbilityManager->DumpLocked(func, line);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpLocked
 * FunctionPoints: The parameter of function DumpLocked.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpLocked
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpLocked_003, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    const char func[2] = "a";
    int line = -1;
    dataAbilityManager->dataAbilityRecordsLoaded_.clear();
    dataAbilityManager->dataAbilityRecordsLoading_.clear();
    dataAbilityManager->DumpLocked(func, line);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpState
 * FunctionPoints: The parameter of function DumpState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpState_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    std::string args = "args";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = dataAbilityRecord;
    dataAbilityManager->DumpState(info, args);
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = nullptr;
    dataAbilityManager->DumpState(info, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpState
 * FunctionPoints: The parameter of function DumpState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpState_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    std::string args = "args";
    dataAbilityManager->dataAbilityRecordsLoaded_.clear();
    dataAbilityManager->DumpState(info, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpState
 * FunctionPoints: The parameter of function DumpState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpState_003, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    std::string args = "";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = nullptr;
    dataAbilityManager->DumpState(info, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "args";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isReady_ = true;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "args";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isReady_ = false;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_003, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "args";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_004, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "args";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->scheduler_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_005, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "args";
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = nullptr;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_006, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = false;
    std::string args = "args";
    dataAbilityManager->dataAbilityRecordsLoaded_["args"] = nullptr;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_007, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = false;
    std::string args = "args";
    dataAbilityManager->dataAbilityRecordsLoaded_.clear();
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_008, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isReady_ = true;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_009, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->isReady_ = false;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_010, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_011, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "";
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->scheduler_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_012, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = true;
    std::string args = "";
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: DumpSysState
 * FunctionPoints: The parameter of function DumpSysState.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function DumpSysState
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_DumpSysState_013, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<std::string> info;
    bool isClient = false;
    std::string args = "";
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->DumpSysState(info, isClient, args);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRunningInfos
 * FunctionPoints: The parameter of function GetAbilityRunningInfos.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRunningInfos
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRunningInfos_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<AbilityRunningInfo> info;
    bool isPerm = true;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->GetAbilityRunningInfos(info, isPerm);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRunningInfos
 * FunctionPoints: The parameter of function GetAbilityRunningInfos.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRunningInfos
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRunningInfos_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<AbilityRunningInfo> info;
    bool isPerm = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->GetAbilityRunningInfos(info, isPerm);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRunningInfos
 * FunctionPoints: The parameter of function GetAbilityRunningInfos.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRunningInfos
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRunningInfos_003, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<AbilityRunningInfo> info;
    bool isPerm = false;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->applicationInfo_.accessTokenId = -1;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->GetAbilityRunningInfos(info, isPerm);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRunningInfos
 * FunctionPoints: The parameter of function GetAbilityRunningInfos.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRunningInfos
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_GetAbilityRunningInfos_004, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<AbilityRunningInfo> info;
    bool isPerm = false;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->applicationInfo_.accessTokenId = IPCSkeleton::GetCallingTokenID();
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = dataAbilityRecord;
    dataAbilityManager->GetAbilityRunningInfos(info, isPerm);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: RestartDataAbility
 * FunctionPoints: The parameter of function RestartDataAbility.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function RestartDataAbility
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_RestartDataAbility_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    std::vector<AbilityRunningInfo> info;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityManager->RestartDataAbility(abilityRecord);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: ReportDataAbilityAcquired
 * FunctionPoints: The parameter of function ReportDataAbilityAcquired.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function ReportDataAbilityAcquired
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_ReportDataAbilityAcquired_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    bool isNotHap = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->ReportDataAbilityAcquired(dataAbilityRecord->GetToken(), isNotHap, dataAbilityRecord);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: ReportDataAbilityAcquired
 * FunctionPoints: The parameter of function ReportDataAbilityAcquired.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function ReportDataAbilityAcquired
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_ReportDataAbilityAcquired_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    bool isNotHap = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityManager->ReportDataAbilityAcquired(nullptr, isNotHap, dataAbilityRecord);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: ReportDataAbilityReleased
 * FunctionPoints: The parameter of function ReportDataAbilityReleased.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function ReportDataAbilityReleased
 */
HWTEST_F(DataAbilityManagerTest, AaFwk_DataAbilityManager_ReportDataAbilityReleased_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    bool isNotHap = true;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->ReportDataAbilityReleased(dataAbilityRecord->GetToken(), isNotHap, dataAbilityRecord);
}
}  // namespace AAFwk
}  // namespace OHOS
