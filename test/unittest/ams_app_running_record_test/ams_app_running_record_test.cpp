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

#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "module_running_record.h"
#undef private

#include <limits>
#include <gtest/gtest.h>
#include "iremote_object.h"
#include "refbase.h"
#include "application_info.h"
#include "app_record_id.h"
#include "app_scheduler_host.h"
#include "ability_info.h"
#include "ability_running_record.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "mock_app_scheduler.h"
#include "mock_ability_token.h"
#include "mock_app_spawn_client.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_iapp_state_callback.h"
#include "mock_bundle_manager.h"
#include "mock_render_scheduler.h"

using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AppExecFwk {
namespace {
static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
}
class AmsAppRunningRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

protected:
    static const std::string GetTestProcessName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAppName()
    {
        return "com.ohos.test.helloworld";
    }
    static const std::string GetTestAbilityName()
    {
        return "test_ability_name";
    }
    static int GetTestUid()
    {
        // a valid inner uid value which is not border value.
        const static int VALID_UID_VALUE = 1010;
        return VALID_UID_VALUE;
    }

    std::shared_ptr<AppRunningRecord> GetTestAppRunningRecord();
    sptr<IAppScheduler> GetMockedAppSchedulerClient() const;
    std::shared_ptr<AppRunningRecord> StartLoadAbility(const sptr<IRemoteObject>& token,
        const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
        const pid_t newPid) const;
    sptr<MockAbilityToken> GetMockToken() const
    {
        return mock_token_;
    }

protected:
    std::shared_ptr<AbilityRunningRecord> testAbilityRecord_;
    sptr<IAppScheduler> client_;
    sptr<MockAppScheduler> mockAppSchedulerClient_;
    std::shared_ptr<AppRunningRecord> testAppRecord_;
    std::unique_ptr<AppMgrServiceInner> service_;
    sptr<MockAbilityToken> mock_token_;
    sptr<BundleMgrService> mockBundleMgr;
};

void AmsAppRunningRecordTest::SetUpTestCase()
{}

void AmsAppRunningRecordTest::TearDownTestCase()
{}

void AmsAppRunningRecordTest::SetUp()
{
    mockAppSchedulerClient_ = new (std::nothrow) MockAppScheduler();
    service_.reset(new (std::nothrow) AppMgrServiceInner());
    mock_token_ = new (std::nothrow) MockAbilityToken();
    client_ = iface_cast<IAppScheduler>(mockAppSchedulerClient_.GetRefPtr());
    mockBundleMgr = new (std::nothrow) BundleMgrService();
    service_->SetBundleManager(mockBundleMgr);
}

void AmsAppRunningRecordTest::TearDown()
{
    testAbilityRecord_.reset();
    testAppRecord_.reset();
}

sptr<IAppScheduler> AmsAppRunningRecordTest::GetMockedAppSchedulerClient() const
{
    if (client_) {
        return client_;
    }
    return nullptr;
}

std::shared_ptr<AppRunningRecord> AmsAppRunningRecordTest::GetTestAppRunningRecord()
{
    if (!testAppRecord_) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        appInfo->name = GetTestAppName();
        testAppRecord_ = std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), GetTestProcessName());
        testAppRecord_->SetApplicationClient(GetMockedAppSchedulerClient());
    }
    return testAppRecord_;
}

std::shared_ptr<AppRunningRecord> AmsAppRunningRecordTest::StartLoadAbility(const sptr<IRemoteObject>& token,
    const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
    const pid_t newPid) const
{
    std::shared_ptr<MockAppSpawnClient> mockClientPtr = std::make_shared<MockAppSpawnClient>();
    service_->SetAppSpawnClient(mockClientPtr);
    EXPECT_CALL(*mockClientPtr, StartProcess(_, _)).Times(1).WillOnce(DoAll(SetArgReferee<1>(newPid), Return(ERR_OK)));

    service_->LoadAbility(token, nullptr, abilityInfo, appInfo, nullptr);

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto record = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, GetTestProcessName(), appInfo->uid, bundleInfo);

    EXPECT_TRUE(record);
    auto clent = GetMockedAppSchedulerClient();
    record->SetApplicationClient(clent);
    EXPECT_EQ(record->GetPriorityObject()->GetPid(), newPid);
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    return record;
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using correct args with app/ability not exists.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord to get result.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);
    EXPECT_EQ(record->GetName(), GetTestAppName());

    EXPECT_EQ(record->GetProcessName(), GetTestProcessName());

    auto abilityRecord = record->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints Create using correct args with app/ability exists.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord twice to create/get a AppRunningRecord.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    abilityInfo->applicationInfo.uid = 1010;
    appInfo->uid = 1010;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    // Create
    sptr<IRemoteObject> token = GetMockToken();
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(1010);
    // Get
    auto record1 = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record1 != nullptr);
    EXPECT_EQ(record1->GetName(), GetTestAppName());
    EXPECT_EQ(record1->GetProcessName(), GetTestProcessName());
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using correct args with app exists but ability not.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord twice which second call uses a different ability info.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_003, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    abilityInfo->applicationInfo.uid = 1010;
    appInfo->uid = 1010;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(1010);

    auto anotherAbilityInfo = std::make_shared<AbilityInfo>();
    anotherAbilityInfo->name = "Another_ability";
    anotherAbilityInfo->applicationInfo.uid = 1010;
    sptr<IRemoteObject> anotherToken = new (std::nothrow) MockAbilityToken();
    auto record1 = service_->CreateAppRunningRecord(GetMockToken(),
        anotherToken,
        appInfo,
        anotherAbilityInfo,
        GetTestProcessName(),
        bundleInfo,
        hapModuleInfo,
        nullptr);
    EXPECT_EQ(record1->GetName(), GetTestAppName());
    EXPECT_EQ(record1->GetProcessName(), GetTestProcessName());

    auto abilityRecord = record1->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(abilityRecord != nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using empty appInfo.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord using empty appInfo.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_004, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    // Create
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, nullptr, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record == nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Create using empty abilityInfo.
 * EnvConditions: NA
 * CaseDescription: Call CreateAppRunningRecord using empty abilityInfo.
 */
HWTEST_F(AmsAppRunningRecordTest, CreateAppRunningRecord_005, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    // Create
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, nullptr, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test launch application.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchApplication_001, TestSize.Level1)
{
    Configuration config;
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(2);
    record->LaunchApplication(config);

    std::string bundleName = "test_mainBundleName";
    record->mainBundleName_ = bundleName;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    record->appInfos_.emplace(bundleName, appInfo);
    record->LaunchApplication(config);

    record->appLifeCycleDeal_->SetApplicationClient(nullptr);
    record->LaunchApplication(config);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test launch ability via AppRunningRecord using valid name.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LaunchAbility which is exists.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbility_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbility_001 start");
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    auto record = GetTestAppRunningRecord();
    EXPECT_TRUE(record);
    record->AddModule(appInfo, nullptr, GetMockToken(), hapModuleInfo, nullptr);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
    auto abilityRecord = moduleRecord->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_EQ(nullptr, abilityRecord);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);
    record->LaunchAbility(abilityRecord);

    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    std::shared_ptr<AbilityInfo> abilityInfo_sptr = std::make_shared<AbilityInfo>(abilityInfo);

    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord =
        std::make_shared<AbilityRunningRecord>(abilityInfo_sptr, nullptr);
    EXPECT_NE(nullptr, abilityRunningRecord);
    EXPECT_EQ(nullptr, abilityRunningRecord->GetToken());
    record->LaunchAbility(abilityRunningRecord);

    sptr<IRemoteObject> token = new MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord1 =
        std::make_shared<AbilityRunningRecord>(abilityInfo_sptr, token);
    EXPECT_NE(nullptr, abilityRunningRecord1);
    EXPECT_NE(nullptr, abilityRunningRecord1->GetToken());
    record->LaunchAbility(abilityRunningRecord1);

    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord2 =
        std::make_shared<AbilityRunningRecord>(abilityInfo_sptr, GetMockToken());
    EXPECT_NE(nullptr, abilityRunningRecord2);
    record->AddModule(appInfo, abilityInfo_sptr, GetMockToken(), hapModuleInfo, nullptr);
    record->LaunchAbility(abilityRunningRecord2);

    record->appLifeCycleDeal_ = nullptr;
    record->LaunchAbility(abilityRecord);
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbility_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test launch ability via AppRunningRecord using empty name.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LaunchAbility which is not exists.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbility_002, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    auto record = GetTestAppRunningRecord();
    record->AddModule(appInfo, abilityInfo, GetMockToken(), hapModuleInfo, nullptr);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
    auto abilityRecord = moduleRecord->GetAbilityRunningRecordByToken(GetMockToken());

    EXPECT_TRUE(abilityRecord);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);

    record->LaunchAbility(abilityRecord);

    EXPECT_EQ(AbilityState::ABILITY_STATE_READY, abilityRecord->GetState());
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application terminate by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleTerminate.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleTerminate_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleTerminateApplication()).Times(1);
    record->ScheduleTerminate();

    record->appLifeCycleDeal_ = nullptr;
    record->ScheduleTerminate();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application foreground by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleForegroundRunning.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleForegroundRunning_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleForegroundApplication()).Times(1);
    record->ScheduleForegroundRunning();

    record->appLifeCycleDeal_ = nullptr;
    record->ScheduleForegroundRunning();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application background by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleBackgroundRunning.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleBackgroundRunning_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleBackgroundApplication()).Times(1);
    record->ScheduleBackgroundRunning();

    record->appLifeCycleDeal_ = nullptr;
    record->ScheduleBackgroundRunning();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule process security exit by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleProcessSecurityExit.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleProcessSecurityExit_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleProcessSecurityExit()).Times(1);
    record->ScheduleProcessSecurityExit();

    record->appLifeCycleDeal_ = nullptr;
    record->ScheduleProcessSecurityExit();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule memory level by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleMemoryLevel.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleMemoryLevel_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleMemoryLevel(_)).Times(1);
    record->ScheduleMemoryLevel(1);

    record->appLifeCycleDeal_ = nullptr;
    record->ScheduleMemoryLevel(1);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Schedule application trim memory by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call ScheduleTrimMemory.
 */
HWTEST_F(AmsAppRunningRecordTest, ScheduleTrimMemory_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleShrinkMemory(_)).Times(1);
    EXPECT_NE(nullptr, record->GetPriorityObject());
    record->ScheduleTrimMemory();

    record->appLifeCycleDeal_ = nullptr;
    record->ScheduleTrimMemory();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Test low memory warning notification handling.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call LowMemoryWarning.
 */
HWTEST_F(AmsAppRunningRecordTest, LowMemoryWarning_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLowMemory()).Times(1);
    record->LowMemoryWarning();

    record->appLifeCycleDeal_ = nullptr;
    record->LowMemoryWarning();
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Add modules by AppRunningRecord.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call AddModules.
 */
HWTEST_F(AmsAppRunningRecordTest, AddModules_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AddModules_001 start");
    auto record = GetTestAppRunningRecord();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    std::vector<HapModuleInfo> moduleInfos;
    record->AddModules(appInfo, moduleInfos);

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    moduleInfos.push_back(hapModuleInfo);
    record->AddModules(appInfo, moduleInfos);
    HILOG_INFO("AmsAppRunningRecordTest AddModules_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update application state using correct args.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call SetState in a for-each cycle.
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateAppRunningRecord_001, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    for (ApplicationState state = ApplicationState::APP_STATE_CREATE; state < ApplicationState::APP_STATE_END;
        state = (ApplicationState)(static_cast<std::underlying_type<ApplicationState>::type>(state) + 1)) {
        record->SetState(state);
        EXPECT_EQ(record->GetState(), state);
    }
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Update application state using wrong args.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call SetState using arg |APP_STATE_END|.
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateAppRunningRecord_002, TestSize.Level1)
{
    auto record = GetTestAppRunningRecord();
    record->SetState(ApplicationState::APP_STATE_END);
    EXPECT_NE(record->GetState(), ApplicationState::APP_STATE_END);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: Delete application record info when application terminated.
 * EnvConditions: NA
 * CaseDescription: Create an AppRunningRecord and call AppMgrService::ApplicationTerminated passing exists
 |RecordId|.
 */
HWTEST_F(AmsAppRunningRecordTest, DeleteAppRunningRecord_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->ApplicationTerminated(record->GetRecordId());
    record = service_->GetAppRunningRecordByAppRecordId(record->GetRecordId());
    EXPECT_TRUE(record == nullptr);
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received normal pid attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();

    const pid_t newPid = 1234;
    EXPECT_TRUE(service_);
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);
    service_->AttachApplication(newPid, mockAppSchedulerClient_);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received invalid pid attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_002, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();
    EXPECT_TRUE(service_ != nullptr);
    const pid_t newPid = 1234;
    const pid_t invalidPid = -1;
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(0);
    service_->AttachApplication(invalidPid, GetMockedAppSchedulerClient());
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_002 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received non-exist pid attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_003, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_003 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();
    EXPECT_TRUE(service_ != nullptr);
    const pid_t newPid = 1234;
    const pid_t anotherPid = 1000;
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(0);
    service_->AttachApplication(anotherPid, GetMockedAppSchedulerClient());
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_003 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received null appClient attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_004, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_004 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    sptr<IRemoteObject> token = GetMockToken();
    EXPECT_TRUE(service_ != nullptr);
    const pid_t newPid = 1234;
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);
    service_->AttachApplication(newPid, nullptr);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_CREATE);
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_004 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received multiple same attachApplication request.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_005, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_005 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    sptr<IRemoteObject> token = GetMockToken();
    const pid_t newPid = 1234;
    EXPECT_TRUE(service_ != nullptr);
    auto record = StartLoadAbility(token, abilityInfo, appInfo, newPid);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);
    service_->AttachApplication(newPid, GetMockedAppSchedulerClient());
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(0);
    service_->AttachApplication(newPid, GetMockedAppSchedulerClient());
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_005 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server received attachApplication request.
 * EnvConditions: NA
 * CaseDescription: Test server received attachApplication request after multiple loadAbility.
 */
HWTEST_F(AmsAppRunningRecordTest, AttachApplication_006, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_006 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();

    auto abilityInfo3 = std::make_shared<AbilityInfo>();
    abilityInfo3->name = GetTestAbilityName() + "_2";
    abilityInfo3->applicationName = GetTestAppName();
    abilityInfo3->process = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    sptr<IRemoteObject> token = GetMockToken();
    const uint32_t EXPECT_RECORD_SIZE = 3;
    const int EXPECT_ABILITY_LAUNCH_TIME = 3;
    const pid_t PID = 1234;
    EXPECT_TRUE(service_ != nullptr);
    auto record = StartLoadAbility(token, abilityInfo, appInfo, PID);

    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    service_->LoadAbility(token2, nullptr, abilityInfo2, appInfo, nullptr);
    sptr<IRemoteObject> token3 = new (std::nothrow) MockAbilityToken();
    service_->LoadAbility(token3, nullptr, abilityInfo3, appInfo, nullptr);
    EXPECT_EQ(record->GetAbilities().size(), EXPECT_RECORD_SIZE);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(EXPECT_ABILITY_LAUNCH_TIME);
    service_->AttachApplication(PID, mockAppSchedulerClient_);
    EXPECT_NE(record->GetApplicationClient(), nullptr);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest AttachApplication_006 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of LaunchAbility after LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of multiple LaunchAbility after LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_002, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_002 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    auto abilityInfo3 = std::make_shared<AbilityInfo>();
    abilityInfo3->name = GetTestAbilityName() + "_2";
    abilityInfo3->applicationName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    const int EXPECT_ABILITY_LAUNCH_TIME = 3;
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo, nullptr);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);
    auto abilityRecord2 = moduleRecord->GetAbilityRunningRecordByToken(token2);
    EXPECT_TRUE(abilityRecord2 != nullptr);

    sptr<IRemoteObject> token3 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo3, token3, hapModuleInfo, nullptr);
    auto moduleRecord3 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord3);
    auto abilityRecord3 = moduleRecord3->GetAbilityRunningRecordByToken(token3);
    EXPECT_TRUE(abilityRecord3 != nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(EXPECT_ABILITY_LAUNCH_TIME);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_002 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test abnormal case of LaunchApplication with wrong state.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_003, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_003 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    record->SetState(ApplicationState::APP_STATE_READY);
    record->SetApplicationClient(GetMockedAppSchedulerClient());

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(0);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(0);
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_003 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of LoadAbility after LaunchAbility and LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_004, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_004 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    abilityInfo2->process = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(0);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(1);
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    service_->LoadAbility(token2, nullptr, abilityInfo2, appInfo, nullptr);
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_004 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: NA
 * FunctionPoints: When server LaunchApplication and LaunchAbility.
 * EnvConditions: NA
 * CaseDescription: Test normal case of multiple LaunchAbility with wrong state after LaunchApplication.
 */
HWTEST_F(AmsAppRunningRecordTest, LaunchAbilityForApp_005, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_005 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    auto abilityInfo3 = std::make_shared<AbilityInfo>();
    abilityInfo3->name = GetTestAbilityName() + "_2";
    abilityInfo3->applicationName = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    const int EXPECT_ABILITY_LAUNCH_TIME = 2;
    EXPECT_TRUE(service_ != nullptr);

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo, nullptr);
    auto moduleRecord2 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord2);
    auto abilityRecord2 = moduleRecord2->GetAbilityRunningRecordByToken(token2);
    abilityRecord2->SetState(AbilityState::ABILITY_STATE_READY);

    sptr<IRemoteObject> token3 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo3, token3, hapModuleInfo, nullptr);
    auto moduleRecord3 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord3);
    auto abilityRecord3 = moduleRecord3->GetAbilityRunningRecordByToken(token3);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchAbility(_, _, _)).Times(EXPECT_ABILITY_LAUNCH_TIME);
    record->SetApplicationClient(GetMockedAppSchedulerClient());
    service_->LaunchApplication(record);
    EXPECT_EQ(record->GetState(), ApplicationState::APP_STATE_READY);
    HILOG_INFO("AmsAppRunningRecordTest LaunchAbilityForApp_005 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: TerminateAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function TerminateAbility can check the token which not added.
 */
HWTEST_F(AmsAppRunningRecordTest, TerminateAbility_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest TerminateAbility_001 start");

    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleCleanAbility(_)).Times(0);
    record->TerminateAbility(GetMockToken(), false);

    HILOG_INFO("AmsAppRunningRecordTest TerminateAbility_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: TerminateAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function TerminateAbility can check the state not in background.
 */
HWTEST_F(AmsAppRunningRecordTest, TerminateAbility_002, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest TerminateAbility_002 start");

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();

    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleCleanAbility(_)).Times(0);
    record->TerminateAbility(GetMockToken(), false);

    HILOG_INFO("AmsAppRunningRecordTest TerminateAbility_002 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: AbilityTerminated
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function AbilityTerminated can check the token is nullptr.
 */
HWTEST_F(AmsAppRunningRecordTest, AbilityTerminated_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AbilityTerminated_001 start");

    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleTerminateApplication()).Times(0);
    record->AbilityTerminated(nullptr);

    HILOG_INFO("AmsAppRunningRecordTest AbilityTerminated_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord
 * SubFunction: GetAbilityRunningRecordByToken
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function GetAbilityRunningRecordByToken can check token is nullptr.
 */
HWTEST_F(AmsAppRunningRecordTest, GetAbilityRunningRecordByToken_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest GetAbilityRunningRecordByToken_001 start");

    auto record = GetTestAppRunningRecord();
    EXPECT_EQ(nullptr, record->GetAbilityRunningRecordByToken(nullptr));

    HILOG_INFO("AmsAppRunningRecordTest GetAbilityRunningRecordByToken_001 end");
}

/*
 * Feature: AMS
 * Function: AppRunningRecord::SetUid, AppRunningRecord::GetUid()
 * SubFunction: GetAbilityRunningRecordByToken
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Verify the function GetAbilityRunningRecordByToken can check token is nullptr.
 */

HWTEST_F(AmsAppRunningRecordTest, SetUid_GetUid_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);
    record->SetUid(102);

    auto otherRecord = service_->GetAppRunningRecordByAppRecordId(record->GetRecordId());
    EXPECT_TRUE(record != nullptr);

    EXPECT_EQ(otherRecord->GetUid(), 102);
}

/*
 * Feature: AMS
 * Function: OnAbilityStateChanged
 * SubFunction: App state switch
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Notify ability when the status of the app changes
 */

HWTEST_F(AmsAppRunningRecordTest, OnAbilityStateChanged_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord != nullptr);
    auto abilityRecord = record->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(abilityRecord != nullptr);

    sptr<MockAppStateCallback> callback = new (std::nothrow) MockAppStateCallback();
    EXPECT_CALL(*callback, OnAbilityRequestDone(_, _)).Times(0);

    moduleRecord->OnAbilityStateChanged(nullptr, AbilityState::ABILITY_STATE_FOREGROUND);

    EXPECT_NE(AbilityState::ABILITY_STATE_FOREGROUND, abilityRecord->GetState());

    std::shared_ptr<AppMgrServiceInner> serviceInner;
    serviceInner.reset(new (std::nothrow) AppMgrServiceInner());
    EXPECT_TRUE(serviceInner);

    EXPECT_CALL(*callback, OnAbilityRequestDone(_, _)).Times(2);
    serviceInner->RegisterAppStateCallback(callback);
    record->SetAppMgrServiceInner(serviceInner);

    moduleRecord->OnAbilityStateChanged(abilityRecord, AbilityState::ABILITY_STATE_FOREGROUND);
    EXPECT_EQ(AbilityState::ABILITY_STATE_FOREGROUND, abilityRecord->GetState());

    moduleRecord->OnAbilityStateChanged(abilityRecord, AbilityState::ABILITY_STATE_BACKGROUND);
    EXPECT_EQ(AbilityState::ABILITY_STATE_BACKGROUND, abilityRecord->GetState());
}

/*
 * Feature: AMS
 * Function: AddModule
 * SubFunction: AddModule
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: add module
 */

HWTEST_F(AmsAppRunningRecordTest, AddModule_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AddModule_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo0 = std::make_shared<ApplicationInfo>();
    appInfo0->name = GetTestAppName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo0;
    hapModuleInfo0.moduleName = "module123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo0, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo0, nullptr);
    EXPECT_TRUE(record != nullptr);
    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<ModuleRunningRecord> moduleRecord =
        record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(!moduleRecord);

    record->AddModule(nullptr, nullptr, nullptr, hapModuleInfo, nullptr);
    record->AddModule(appInfo, nullptr, nullptr, hapModuleInfo, nullptr);
    record->AddModule(appInfo, abilityInfo, nullptr, hapModuleInfo, nullptr);
    record->AddModule(appInfo, nullptr, token2, hapModuleInfo, nullptr);
    moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_FALSE(!moduleRecord);
    record->AddModule(appInfo, abilityInfo, token2, hapModuleInfo, nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1, nullptr);

    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 2);
    HILOG_INFO("AmsAppRunningRecordTest AddModule_001 end");
}

/*
 * Feature: AMS
 * Function: AddModule
 * SubFunction: AddModule
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: add module
 */

HWTEST_F(AmsAppRunningRecordTest, AddModule_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);

    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo, nullptr);

    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);
}

/*
 * Feature: AMS
 * Function: GetModuleRecordByModuleName
 * SubFunction: GetModuleRecordByModuleName
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get ModuleRecord By ModuleName
 */

HWTEST_F(AmsAppRunningRecordTest, GetModuleRecordByModuleName_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest GetModuleRecordByModuleName_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    auto appInfo1 = std::make_shared<ApplicationInfo>();
    appInfo1->name = GetTestAppName() + "_1";
    appInfo1->bundleName = GetTestAppName() + "_1";

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->appRunningManager_->CreateAppRunningRecord(appInfo, GetTestProcessName(), bundleInfo);
    EXPECT_TRUE(record != nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 0);
    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord == nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo1, abilityInfo2, token2, hapModuleInfo1, nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 1);
    moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord == nullptr);

    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1, nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 2);

    std::string moduleName1 = "module123";
    moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, moduleName1);
    EXPECT_TRUE(moduleRecord != nullptr);

    moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord == nullptr);
    HILOG_INFO("AmsAppRunningRecordTest GetModuleRecordByModuleName_001 end");
}

/*
 * Feature: AMS
 * Function: GetAbilities
 * SubFunction: GetAbilities
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get All Abilities
 */

HWTEST_F(AmsAppRunningRecordTest, GetAbilities_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo, nullptr);

    auto abilities = record->GetAbilities();
    EXPECT_TRUE(abilities.size() == 2);
}

/*
 * Feature: AMS
 * Function: GetAbilities
 * SubFunction: GetAbilities
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get All Abilities
 */

HWTEST_F(AmsAppRunningRecordTest, GetAbilities_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1, nullptr);

    auto abilities = record->GetAbilities();
    EXPECT_TRUE(abilities.size() == 2);
}

/*
 * Feature: AMS
 * Function: RemoveModuleRecord
 * SubFunction: RemoveModuleRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, RemoveModuleRecord_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest RemoveModuleRecord_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1, nullptr);

    std::shared_ptr<ModuleRunningRecord> moduleRecord0;
    record->RemoveModuleRecord(moduleRecord0);
    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 2);

    moduleRecord0 = std::make_shared<ModuleRunningRecord>(appInfo, nullptr);
    HapModuleInfo hapModuleInfo0;
    hapModuleInfo0.moduleName = "module0";
    moduleRecord0->Init(hapModuleInfo0);
    record->RemoveModuleRecord(moduleRecord0);
    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 2);

    auto moduleRecord = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    EXPECT_TRUE(moduleRecord);

    record->RemoveModuleRecord(moduleRecord);
    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 1);

    auto moduleRecord1 = record->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo1.moduleName);
    EXPECT_TRUE(moduleRecord1);

    record->RemoveModuleRecord(moduleRecord1);
    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 0);

    record->RemoveModuleRecord(moduleRecord1);
    moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 0);
    HILOG_INFO("AmsAppRunningRecordTest RemoveModuleRecord_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: Environmental Change Notification
 * EnvConditions: NA
 * CaseDescription: Make an environment object and update
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateConfiguration_001, TestSize.Level1)
{
    auto testLanguge = std::string("ch-zh");
    auto configUpdate = [testLanguge](const Configuration& config) {
        auto l = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        EXPECT_TRUE(testLanguge == l);
    };

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, testLanguge);
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleConfigurationUpdated(_))
        .Times(1)
        .WillOnce(testing::Invoke(configUpdate));

    record->UpdateConfiguration(config);
}

/*
 * Feature: AbilityManagerService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: Environmental Change Notification
 * EnvConditions: NA
 * CaseDescription: Make an environment object and update
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateConfiguration_002, TestSize.Level1)
{
    auto test = std::string("colour");
    auto configUpdate = [test](const Configuration& config) {
        auto l = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        EXPECT_TRUE(test == l);
    };

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, test);
    auto record = GetTestAppRunningRecord();
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleConfigurationUpdated(_))
        .Times(1)
        .WillOnce(testing::Invoke(configUpdate));

    record->UpdateConfiguration(config);
}

/*
 * Feature: AMS
 * Function: SetSpecifiedAbilityFlagAndWant
 * SubFunction: SetSpecifiedAbilityFlagAndWant
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, SetSpecifiedAbilityFlagAndWant_001, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    int32_t recordId = 11;
    std::string processName = "processName";
    auto record = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);

    bool flag = true;
    AAFwk::Want want;
    std::string moduleName = "module123";
    record->SetSpecifiedAbilityFlagAndWant(flag, want, moduleName);
    EXPECT_TRUE(record->isSpecifiedAbility_ == flag);
    EXPECT_TRUE(record->moduleName_ == moduleName);
}

/*
 * Feature: AMS
 * Function: IsStartSpecifiedAbility
 * SubFunction: IsStartSpecifiedAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, IsStartSpecifiedAbility_001, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    int32_t recordId = 11;
    std::string processName = "processName";
    auto record = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);

    bool flag = true;
    AAFwk::Want want;
    std::string moduleName = "module123";
    record->SetSpecifiedAbilityFlagAndWant(flag, want, moduleName);
    EXPECT_TRUE(record->IsStartSpecifiedAbility() == flag);
}

/*
 * Feature: AMS
 * Function: GetSpecifiedWant
 * SubFunction: GetSpecifiedWant
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, GetSpecifiedWant_001, TestSize.Level1)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    int32_t recordId = 11;
    std::string processName = "processName";
    auto record = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);

    bool flag = true;
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    std::string moduleName = "module123";
    record->SetSpecifiedAbilityFlagAndWant(flag, want, moduleName);
    EXPECT_TRUE(record->GetSpecifiedWant().GetBundle() == want.GetBundle());
}

/*
 * Feature: AMS
 * Function: RegisterStartSpecifiedAbilityResponse
 * SubFunction: RegisterStartSpecifiedAbilityResponse
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, RegisterStartSpecifiedAbilityResponse_001, TestSize.Level1)
{
    sptr<IStartSpecifiedAbilityResponse> response;
    service_->RegisterStartSpecifiedAbilityResponse(response);
    EXPECT_TRUE(service_->startSpecifiedAbilityResponse_ == response);
}

/*
 * Feature: AMS
 * Function: StartSpecifiedAbility
 * SubFunction: StartSpecifiedAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, StartSpecifiedAbility_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);
    EXPECT_EQ(record->GetName(), GetTestAppName());
    EXPECT_EQ(record->GetProcessName(), GetTestProcessName());

    Want want;
    want.SetElementName("DemoDeviceIdB", "DemoBundleNameB", "DemoAbilityNameB");
    service_->StartSpecifiedAbility(want, *abilityInfo);
}

/*
 * Feature: AMS
 * Function: StartSpecifiedAbility
 * SubFunction: StartSpecifiedAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, StartSpecifiedAbility_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    EXPECT_TRUE(record != nullptr);
    EXPECT_EQ(record->GetName(), GetTestAppName());
    EXPECT_EQ(record->GetProcessName(), GetTestProcessName());

    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->name = "test_ability_name_2";
    Want want;
    want.SetElementName("DemoDeviceId", "DemoBundleName", "DemoAbilityName");
    service_->StartSpecifiedAbility(want, *abilityInfo1);
}

/*
 * Feature: AMS
 * Function: LaunchApplication
 * SubFunction: LaunchApplication
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Remove ModuleRecord
 */
HWTEST_F(AmsAppRunningRecordTest, Specified_LaunchApplication_001, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();
    abilityInfo->process = GetTestAppName();
    abilityInfo->applicationInfo.bundleName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    EXPECT_TRUE(service_ != nullptr);
    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    record->SetApplicationClient(GetMockedAppSchedulerClient());
    record->isSpecifiedAbility_ = true;
    EXPECT_CALL(*mockAppSchedulerClient_, ScheduleLaunchApplication(_, _)).Times(1);
    service_->LaunchApplication(record);
    auto ability = record->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(ability->GetState() != AbilityState::ABILITY_STATE_READY);
}

/*
 * Feature: AMS
 * Function: RenderRecord
 * SubFunction: RenderRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: New RenderRecord
 */
HWTEST_F(AmsAppRunningRecordTest, NewRenderRecord_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    int32_t ipcFd = 0;
    int32_t sharedFd = 0;
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, ipcFd, sharedFd, host);
    EXPECT_NE(renderRecord, nullptr);
    delete renderRecord;
}

/*
 * Feature: AMS
 * Function: CreateRenderRecord
 * SubFunction: CreateRenderRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Create Render Record
 */
HWTEST_F(AmsAppRunningRecordTest, CreateRenderRecord_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    pid_t hostPid1 = 1;
    std::string renderParam = "";
    std::string renderParam1 = "test_render_param";
    int32_t ipcFd = 0;
    int32_t ipcFd1 = 1;
    int32_t sharedFd = 0;
    int32_t sharedFd1 = 1;
    std::shared_ptr<AppRunningRecord> host;

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    int32_t recordId = 11;
    std::string processName = "processName";
    std::shared_ptr<AppRunningRecord> host1 = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);

    std::shared_ptr<RenderRecord> renderRecord =
        RenderRecord::CreateRenderRecord(hostPid, renderParam, ipcFd, sharedFd, host);
    EXPECT_EQ(renderRecord, nullptr);

    renderRecord = RenderRecord::CreateRenderRecord(hostPid1, renderParam, ipcFd, sharedFd, host);
    EXPECT_EQ(renderRecord, nullptr);
    renderRecord = RenderRecord::CreateRenderRecord(hostPid1, renderParam1, ipcFd, sharedFd, host);
    EXPECT_EQ(renderRecord, nullptr);
    renderRecord = RenderRecord::CreateRenderRecord(hostPid1, renderParam1, ipcFd1, sharedFd, host);
    EXPECT_EQ(renderRecord, nullptr);
    renderRecord = RenderRecord::CreateRenderRecord(hostPid1, renderParam1, ipcFd1, sharedFd1, host);
    EXPECT_EQ(renderRecord, nullptr);
    renderRecord = RenderRecord::CreateRenderRecord(hostPid1, renderParam1, ipcFd1, sharedFd1, host1);
    EXPECT_NE(renderRecord, nullptr);
}

/*
 * Feature: AMS
 * Function: Set/GetPid
 * SubFunction: Set/GetPid
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Set/Get Pid
 */
HWTEST_F(AmsAppRunningRecordTest, SetPid_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    pid_t pid = 0;
    renderRecord->SetPid(pid);
    EXPECT_EQ(renderRecord->GetPid(), pid);
}

/*
 * Feature: AMS
 * Function: Set/GetHostUid
 * SubFunction: Set/GetHostUid
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Set/Get HostUid
 */
HWTEST_F(AmsAppRunningRecordTest, SetHostUid_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    int32_t uid = 1;
    renderRecord->SetHostUid(uid);
    EXPECT_EQ(renderRecord->GetHostUid(), uid);
}

/*
 * Feature: AMS
 * Function: Set/GetHostBundleName
 * SubFunction: Set/GetHostBundleName
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Set/Get HostBundleName
 */
HWTEST_F(AmsAppRunningRecordTest, SetHostBundleName_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    std::string hostBundleName = "testhostBundleName";
    renderRecord->SetHostBundleName(hostBundleName);
    EXPECT_EQ(renderRecord->GetHostBundleName(), hostBundleName);
}

/*
 * Feature: AMS
 * Function: GetRenderParam
 * SubFunction: GetRenderParam
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get Render Param
 */
HWTEST_F(AmsAppRunningRecordTest, GetRenderParam_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    EXPECT_EQ(renderRecord->GetRenderParam(), renderParam);
}

/*
 * Feature: AMS
 * Function: GetIpcFd
 * SubFunction: GetIpcFd
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get Ipc Fd
 */
HWTEST_F(AmsAppRunningRecordTest, GetIpcFd_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 1, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    EXPECT_EQ(renderRecord->GetIpcFd(), 1);
}

/*
 * Feature: AMS
 * Function: GetSharedFd
 * SubFunction: GetSharedFd
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get Share Fd
 */
HWTEST_F(AmsAppRunningRecordTest, GetSharedFd_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 1, host);
    EXPECT_NE(renderRecord, nullptr);
    EXPECT_EQ(renderRecord->GetSharedFd(), 1);
}

/*
 * Feature: AMS
 * Function: GetHostRecord
 * SubFunction: GetHostRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get Host Record
 */
HWTEST_F(AmsAppRunningRecordTest, GetHostRecord_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 1, host);
    EXPECT_NE(renderRecord, nullptr);
    EXPECT_EQ(renderRecord->GetHostRecord(), host);
}

/*
 * Feature: AMS
 * Function: Set/GetScheduler
 * SubFunction: Set/GetScheduler
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Set/Get Scheduler
 */
HWTEST_F(AmsAppRunningRecordTest, SetScheduler_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    sptr<IRenderScheduler> scheduler;
    renderRecord->SetScheduler(scheduler);
    EXPECT_EQ(renderRecord->GetScheduler(), scheduler);
}

/*
 * Feature: AMS
 * Function: SetDeathRecipient
 * SubFunction: SetDeathRecipient
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Set Death Recipient
 */
HWTEST_F(AmsAppRunningRecordTest, SetDeathRecipient_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    sptr<AppDeathRecipient> recipient;
    renderRecord->SetDeathRecipient(recipient);
    EXPECT_EQ(renderRecord->deathRecipient_, recipient);
}

/*
 * Feature: AMS
 * Function: RegisterDeathRecipient
 * SubFunction: RegisterDeathRecipient
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Register Death Recipient
 */
HWTEST_F(AmsAppRunningRecordTest, RegisterDeathRecipient_001, TestSize.Level1)
{
    pid_t hostPid = 0;
    std::string renderParam = "test_render_param";
    std::shared_ptr<AppRunningRecord> host;
    RenderRecord* renderRecord =
        new RenderRecord(hostPid, renderParam, 0, 0, host);
    EXPECT_NE(renderRecord, nullptr);
    renderRecord->RegisterDeathRecipient();

    sptr<MockRenderScheduler> mockRenderScheduler = new (std::nothrow) MockRenderScheduler();
    renderRecord->SetScheduler(mockRenderScheduler);
    renderRecord->RegisterDeathRecipient();

    sptr<AppDeathRecipient> recipient;
    renderRecord->SetDeathRecipient(recipient);
    renderRecord->RegisterDeathRecipient();

    renderRecord->SetScheduler(nullptr);
    renderRecord->RegisterDeathRecipient();
}

/*
 * Feature: AMS
 * Function: NewAppRunningRecord
 * SubFunction: NewAppRunningRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: New AppRunningRecord
 */
HWTEST_F(AmsAppRunningRecordTest, NewAppRunningRecord_001, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo;
    std::shared_ptr<AppRunningRecord> appRunningRecord =
        std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), GetTestProcessName());
    EXPECT_NE(appRunningRecord, nullptr);

    appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    std::shared_ptr<AppRunningRecord> appRunningRecord1 =
        std::make_shared<AppRunningRecord>(appInfo, AppRecordId::Create(), GetTestProcessName());
    EXPECT_NE(appRunningRecord1, nullptr);

    bool isLauncherApp = appRunningRecord1->IsLauncherApp();
    EXPECT_FALSE(isLauncherApp);

    appRunningRecord1->SetState(ApplicationState::APP_STATE_END);
    EXPECT_EQ(appRunningRecord1->GetState(), ApplicationState::APP_STATE_CREATE);

    appRunningRecord1->SetState(ApplicationState::APP_STATE_READY);
    EXPECT_EQ(appRunningRecord1->GetState(), ApplicationState::APP_STATE_READY);

    std::string reason = "test_reason";
    appRunningRecord1->ForceKillApp(reason);
    std::string description = "test_description";
    appRunningRecord1->ScheduleAppCrash(description);
}

/*
 * Feature: AMS
 * Function: GetAbilityRunningRecord
 * SubFunction: GetAbilityRunningRecord
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get AbilityRunningRecord
 */
HWTEST_F(AmsAppRunningRecordTest, GetAbilityRunningRecord_002, TestSize.Level1)
{
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    abilityInfo->applicationName = GetTestAppName();

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();

    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";

    std::shared_ptr<AppRunningRecord> record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo, token, hapModuleInfo, nullptr);
    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->name = GetTestAbilityName() + "_1";
    abilityInfo1->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "";
    sptr<IRemoteObject> token1 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo1, token1, hapModuleInfo1, nullptr);
    auto moduleRecordList = record->GetAllModuleRecord();
    EXPECT_TRUE(moduleRecordList.size() == 2);

    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord1 = record->GetAbilityRunningRecord(999);
    EXPECT_EQ(abilityRunningRecord1, nullptr);
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord2 = record->GetAbilityRunningRecord(123);
    EXPECT_EQ(abilityRunningRecord2, nullptr);
}

/*
 * Feature: AMS
 * Function: AddAbilityStage
 * SubFunction: AddAbilityStage
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Add AbilityStage
 */
HWTEST_F(AmsAppRunningRecordTest, AddAbilityStage_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AddAbilityStage_001 start");
    std::string bundleName = "test_mainBundleName";
    std::string bundleName1 = "test_mainBundleName1";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = bundleName1;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);

    record->AddAbilityStage();

    record->isStageBasedModel_ = true;
    record->AddAbilityStage();

    record->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    record->AddAbilityStage();

    appInfo->bundleName = bundleName;
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1, nullptr);
    record->mainBundleName_ = bundleName;
    record->AddAbilityStage();
    HILOG_INFO("AmsAppRunningRecordTest AddAbilityStage_001 end");
}

/*
 * Feature: AMS
 * Function: AddAbilityStageBySpecifiedAbility
 * SubFunction: AddAbilityStageBySpecifiedAbility
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Add Ability Stage By Specified Ability
 */
HWTEST_F(AmsAppRunningRecordTest, AddAbilityStageBySpecifiedAbility_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AddAbilityStageBySpecifiedAbility_001 start");
    std::string bundleName = "test_mainBundleName";
    std::string bundleName1 = "test_mainBundleName1";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = bundleName;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);

    record->AddAbilityStageBySpecifiedAbility(bundleName1);

    auto runner = EventRunner::Create("AmsAppLifeCycleModuleTest");
    std::shared_ptr<AppMgrServiceInner> serviceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AMSEventHandler> handler = std::make_shared<AMSEventHandler>(runner, serviceInner);
    record->eventHandler_ = handler;
    record->AddAbilityStageBySpecifiedAbility(bundleName1);
    record->AddAbilityStageBySpecifiedAbility(bundleName);

    record->eventHandler_->SendEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG, 1, 0);
    record->AddAbilityStageBySpecifiedAbility(bundleName);

    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName() + "_1";
    abilityInfo2->applicationName = GetTestAppName();
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo, abilityInfo2, token2, hapModuleInfo1, nullptr);

    record->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    record->AddAbilityStageBySpecifiedAbility(bundleName);

    HILOG_INFO("AmsAppRunningRecordTest AddAbilityStageBySpecifiedAbility_001 end");
}

/*
 * Feature: AMS
 * Function: AddAbilityStageDone
 * SubFunction: AddAbilityStageDone
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Add Ability Stage Done
 */
HWTEST_F(AmsAppRunningRecordTest, AddAbilityStageDone_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AddAbilityStageDone_001 start");
    std::string bundleName = "test_mainBundleName";
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = bundleName;
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);
    record->AddAbilityStageDone();

    auto runner = EventRunner::Create("AmsAppLifeCycleModuleTest");
    std::shared_ptr<AppMgrServiceInner> serviceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AMSEventHandler> handler = std::make_shared<AMSEventHandler>(runner, serviceInner);
    record->eventHandler_ = handler;
    record->AddAbilityStageDone();

    record->eventHandler_->SendEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG, 1, 0);
    record->eventHandler_->SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, 1, 0);
    record->AddAbilityStageDone();

    record->isSpecifiedAbility_ = true;
    record->AddAbilityStageDone();

    HILOG_INFO("AmsAppRunningRecordTest AddAbilityStageDone_001 end");
}

/*
 * Feature: AMS
 * Function: StateChangedNotifyObserver
 * SubFunction: StateChangedNotifyObserver
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: State Changed Notify Observer
 */
HWTEST_F(AmsAppRunningRecordTest, StateChangedNotifyObserver_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest StateChangedNotifyObserver_001 start");
    auto record = GetTestAppRunningRecord();
    record->StateChangedNotifyObserver(nullptr, 0, false);

    sptr<IRemoteObject> token = new MockAbilityToken();
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord =
        std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord1 =
        std::make_shared<AbilityRunningRecord>(nullptr, token);
    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->name = GetTestAbilityName();
    abilityInfo1->type = AbilityType::EXTENSION;
    std::shared_ptr<AbilityRunningRecord> abilityRunningRecord2 =
        std::make_shared<AbilityRunningRecord>(abilityInfo1, token);
    record->StateChangedNotifyObserver(abilityRunningRecord, 0, false);
    record->StateChangedNotifyObserver(abilityRunningRecord1, 0, true);
    record->StateChangedNotifyObserver(abilityRunningRecord, 0, true);
    record->StateChangedNotifyObserver(abilityRunningRecord2, 0, true);

    HILOG_INFO("AmsAppRunningRecordTest StateChangedNotifyObserver_001 end");
}

/*
 * Feature: AMS
 * Function: GetModuleRunningRecordByToken
 * SubFunction: GetModuleRunningRecordByToken
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get Module Running Record By Token
 */
HWTEST_F(AmsAppRunningRecordTest, GetModuleRunningRecordByToken_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest GetModuleRunningRecordByToken_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    auto appInfo1 = std::make_shared<ApplicationInfo>();
    appInfo1->name = GetTestAppName() + "_1";
    appInfo1->bundleName = GetTestAppName() + "_1";
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->appRunningManager_->CreateAppRunningRecord(appInfo, GetTestProcessName(), bundleInfo);
    EXPECT_TRUE(record != nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 0);

    std::shared_ptr<ModuleRunningRecord> moduleRecord = record->GetModuleRunningRecordByToken(nullptr);
    EXPECT_TRUE(moduleRecord == nullptr);
    moduleRecord = record->GetModuleRunningRecordByToken(token);
    EXPECT_TRUE(moduleRecord == nullptr);

    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->name = GetTestAbilityName() + "_1";
    abilityInfo1->applicationName = GetTestAppName() + "_1";
    HapModuleInfo hapModuleInfo1;
    hapModuleInfo1.moduleName = "module123";
    sptr<IRemoteObject> token1 = new (std::nothrow) MockAbilityToken();
    record->AddModule(appInfo1, abilityInfo1, token1, hapModuleInfo1, nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 1);
    moduleRecord = record->GetModuleRunningRecordByToken(token);
    EXPECT_TRUE(moduleRecord == nullptr);

    record->AddModule(appInfo, abilityInfo, token, hapModuleInfo, nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 2);
    moduleRecord = record->GetModuleRunningRecordByToken(token);
    EXPECT_TRUE(moduleRecord != nullptr);
    HILOG_INFO("AmsAppRunningRecordTest GetModuleRunningRecordByToken_001 end");
}

/*
 * Feature: AMS
 * Function: GetModuleRunningRecordByTerminateLists
 * SubFunction: GetModuleRunningRecordByTerminateLists
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Get Module Running Record By Terminate Lists
 */
HWTEST_F(AmsAppRunningRecordTest, GetModuleRunningRecordByTerminateLists_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest GetModuleRunningRecordByTerminateLists_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto abilityInfo1 = std::make_shared<AbilityInfo>();
    abilityInfo1->name = GetTestAbilityName() + "_1";
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    appInfo->bundleName = GetTestAppName();
    auto appInfo1 = std::make_shared<ApplicationInfo>();
    appInfo1->name = GetTestAppName() + "_1";
    appInfo1->bundleName = GetTestAppName() + "_1";
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    sptr<IRemoteObject> token1 = new (std::nothrow) MockAbilityToken();
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->appRunningManager_->CreateAppRunningRecord(appInfo, GetTestProcessName(), bundleInfo);
    EXPECT_TRUE(record != nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 0);

    std::shared_ptr<ModuleRunningRecord> moduleRecord = record->GetModuleRunningRecordByTerminateLists(nullptr);
    EXPECT_TRUE(moduleRecord == nullptr);
    moduleRecord = record->GetModuleRunningRecordByTerminateLists(token);
    EXPECT_TRUE(moduleRecord == nullptr);

    record->AddModule(appInfo, abilityInfo, token, hapModuleInfo, nullptr);
    record->AddModule(appInfo1, abilityInfo1, token1, hapModuleInfo, nullptr);
    EXPECT_TRUE(record->hapModules_.size() == 2);
    moduleRecord = record->GetModuleRunningRecordByTerminateLists(token);
    EXPECT_TRUE(moduleRecord == nullptr);

    std::vector<std::shared_ptr<ModuleRunningRecord>> moduleRecords = record->hapModules_[appInfo->bundleName];
    EXPECT_TRUE(moduleRecords.size() == 1);
    std::shared_ptr<ModuleRunningRecord> moduleRecord1 = moduleRecords.front();
    std::shared_ptr<AbilityRunningRecord> abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    moduleRecord1->terminateAbilities_.emplace(token, abilityRecord);

    moduleRecord = record->GetModuleRunningRecordByTerminateLists(token);
    EXPECT_EQ(moduleRecord, moduleRecord1);

    HILOG_INFO("AmsAppRunningRecordTest GetModuleRunningRecordByTerminateLists_001 end");
}

/*
 * Feature: AMS
 * Function: UpdateAbilityFocusState
 * SubFunction: UpdateAbilityFocusState
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Update Ability Focus State
 */
HWTEST_F(AmsAppRunningRecordTest, UpdateAbilityFocusState_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest UpdateAbilityFocusState_001 start");

    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName();
    BundleInfo bundleInfo;
    bundleInfo.appId = "com.ohos.test.helloworld_code123";
    bundleInfo.jointUserId = "joint456";
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module789";
    EXPECT_TRUE(service_ != nullptr);
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, GetTestProcessName(), bundleInfo, hapModuleInfo, nullptr);
    EXPECT_TRUE(record != nullptr);

    auto abilityRecord = record->GetAbilityRunningRecordByToken(GetMockToken());
    EXPECT_TRUE(abilityRecord != nullptr);

    EXPECT_FALSE(abilityRecord->GetFocusFlag());
    record->UpdateAbilityFocusState(GetMockToken(), true);
    record->UpdateAbilityFocusState(GetMockToken(), false);

    abilityRecord->UpdateFocusState(true);
    record->UpdateAbilityFocusState(GetMockToken(), true);
    record->UpdateAbilityFocusState(GetMockToken(), false);

    HILOG_INFO("AmsAppRunningRecordTest UpdateAbilityFocusState_001 end");
}

/*
 * Feature: AMS
 * Function: AbilityForeground
 * SubFunction: AbilityForeground
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Foreground
 */
HWTEST_F(AmsAppRunningRecordTest, AbilityForeground_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AbilityForeground_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRecord;
    auto record = GetTestAppRunningRecord();
    record->AbilityForeground(abilityRecord);

    abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    record->AbilityForeground(abilityRecord);

    abilityRecord->SetState(AbilityState::ABILITY_STATE_READY);
    record->AbilityForeground(abilityRecord);

    abilityRecord->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
    record->AbilityForeground(abilityRecord);

    abilityRecord->SetState(AbilityState::ABILITY_STATE_FOREGROUND);
    record->AbilityForeground(abilityRecord);

    record->SetState(ApplicationState::APP_STATE_TERMINATED);
    record->AbilityForeground(abilityRecord);

    HILOG_INFO("AmsAppRunningRecordTest AbilityForeground_001 end");
}

/*
 * Feature: AMS
 * Function: AbilityBackground
 * SubFunction: AbilityBackground
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Background
 */
HWTEST_F(AmsAppRunningRecordTest, AbilityBackground_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AbilityBackground_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRecord;
    auto record = GetTestAppRunningRecord();
    record->AbilityBackground(abilityRecord);

    abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    record->AbilityBackground(abilityRecord);

    abilityRecord->SetState(AbilityState::ABILITY_STATE_BACKGROUND);
    record->AbilityBackground(abilityRecord);

    HILOG_INFO("AmsAppRunningRecordTest AbilityBackground_001 end");
}

/*
 * Feature: AMS
 * Function: AbilityFocused
 * SubFunction: AbilityFocused
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Focused
 */
HWTEST_F(AmsAppRunningRecordTest, AbilityFocused_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AbilityFocused_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRecord;
    auto record = GetTestAppRunningRecord();
    record->AbilityFocused(abilityRecord);

    abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    record->AbilityFocused(abilityRecord);

    HILOG_INFO("AmsAppRunningRecordTest AbilityFocused_001 end");
}

/*
 * Feature: AMS
 * Function: AbilityUnfocused
 * SubFunction: AbilityUnfocused
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Unfocused
 */
HWTEST_F(AmsAppRunningRecordTest, AbilityUnfocused_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest AbilityUnfocused_001 start");
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    std::shared_ptr<AbilityRunningRecord> abilityRecord;
    auto record = GetTestAppRunningRecord();
    record->AbilityUnfocused(abilityRecord);

    abilityRecord = std::make_shared<AbilityRunningRecord>(abilityInfo, token);
    record->AbilityUnfocused(abilityRecord);

    HILOG_INFO("AmsAppRunningRecordTest AbilityUnfocused_001 end");
}

/*
 * Feature: AMS
 * Function: SetRestartTimeMillis
 * SubFunction: SetRestartTimeMillis
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Unfocused
 * @tc.require: issueI6588V
 */
HWTEST_F(AmsAppRunningRecordTest, SetRestartTimeMillis_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest SetRestartTimeMillis_001 start");
    std::shared_ptr<AppRunningRecord> record = GetTestAppRunningRecord();
    record->SetRestartTimeMillis(1000);
    EXPECT_EQ(record->restartTimeMillis_, 1000);

    HILOG_INFO("AmsAppRunningRecordTest SetRestartTimeMillis_001 end");
}

/*
 * Feature: AMS
 * Function: CanRestartResidentProc
 * SubFunction: CanRestartResidentProc
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Unfocused
 * @tc.require: issueI6588V
 */
HWTEST_F(AmsAppRunningRecordTest, CanRestartResidentProc_001, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest CanRestartResidentProc_001 start");
    std::shared_ptr<AppRunningRecord> record = GetTestAppRunningRecord();
    record->restartResidentProcCount_ = 1;
    EXPECT_TRUE(record->CanRestartResidentProc());

    HILOG_INFO("AmsAppRunningRecordTest CanRestartResidentProc_001 end");
}

/*
 * Feature: AMS
 * Function: CanRestartResidentProc
 * SubFunction: CanRestartResidentProc
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Unfocused
 * @tc.require: issueI6588V
 */
HWTEST_F(AmsAppRunningRecordTest, CanRestartResidentProc_002, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest CanRestartResidentProc_002 start");
    std::shared_ptr<AppRunningRecord> record = GetTestAppRunningRecord();
    record->restartResidentProcCount_ = -1;
    record->restartTimeMillis_ = 0;
    EXPECT_TRUE(record->CanRestartResidentProc());

    HILOG_INFO("AmsAppRunningRecordTest CanRestartResidentProc_002 end");
}

/*
 * Feature: AMS
 * Function: CanRestartResidentProc
 * SubFunction: CanRestartResidentProc
 * FunctionPoints: check params
 * EnvConditions: Mobile that can run ohos test framework
 * CaseDescription: Ability Unfocused
 * @tc.require: issueI6588V
 */
HWTEST_F(AmsAppRunningRecordTest, CanRestartResidentProc_003, TestSize.Level1)
{
    HILOG_INFO("AmsAppRunningRecordTest CanRestartResidentProc_003 start");
    std::shared_ptr<AppRunningRecord> record = GetTestAppRunningRecord();
    record->restartResidentProcCount_ = -1;
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t systemTimeMillis = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    record->restartTimeMillis_ = systemTimeMillis + 1000;
    EXPECT_FALSE(record->CanRestartResidentProc());

    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    record->SetRestartResidentProcCount(0);
    EXPECT_TRUE(record->CanRestartResidentProc());

    HILOG_INFO("AmsAppRunningRecordTest CanRestartResidentProc_003 end");
}
}  // namespace AppExecFwk
}  // namespace OHOS
