/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <vector>

#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "iservice_registry.h"
#undef private

#include "ability_info.h"
#include "application_info.h"
#include "app_record_id.h"
#include "app_scheduler_host.h"
#include "app_scheduler_proxy.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "mock_application.h"
#include "mock_app_scheduler.h"
#include "mock_app_spawn_client.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_native_token.h"
#include "mock_system_ability_manager.h"
#include "singleton.h"

using namespace testing::ext;
using OHOS::iface_cast;
using OHOS::IRemoteObject;
using OHOS::sptr;
using testing::_;
using testing::Invoke;
using testing::InvokeWithoutArgs;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();
} // namespace
class AppRunningProcessesInfoModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockBundleInstallerAndSA() const;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
protected:
    std::string GetTestAppName(const unsigned long num) const
    {
        if (num < appName_.size()) {
            return appName_[num];
        }
        return "";
    }

    std::string GetTestAbilityName(const unsigned long num) const
    {
        if (num < abilityName_.size()) {
            return abilityName_[num];
        }
        return "";
    }

    void CheckLaunchApplication(const sptr<MockApplication>& mockApplication, const unsigned long index,
        std::shared_ptr<AppRunningRecord> record, const std::string& testPoint) const
    {
        EXPECT_TRUE(record != nullptr) << "record is nullptr!";
        sptr<IAppScheduler> client = iface_cast<IAppScheduler>(mockApplication);
        record->SetApplicationClient(client);

        std::string applicationName(GetTestAppName(index));
        ApplicationInfo info;
        info.name = applicationName;
        std::string processInfoName(GetTestAppName(index));
        pid_t pidId = 123;
        ProcessInfo processInfo(processInfoName, pidId);

        AppLaunchData launchData;
        launchData.SetApplicationInfo(info);
        launchData.SetProcessInfo(processInfo);

        EXPECT_CALL(*mockApplication, ScheduleLaunchApplication(_, _))
            .Times(1)
            .WillOnce(Invoke(mockApplication.GetRefPtr(), &MockApplication::LaunchApplication));
        Configuration config;
        record->LaunchApplication(config);
        mockApplication->Wait();

        bool isEqual = mockApplication->CompareAppLaunchData(launchData);
        EXPECT_EQ(isEqual, true) << testPoint << ",fail";
    }

    void CheckAppRunningRecording(const std::shared_ptr<ApplicationInfo> appInfo,
        const std::shared_ptr<AbilityInfo> abilityInfo, const std::shared_ptr<AppRunningRecord> record, const int index,
        RecordQueryResult& result) const
    {
        EXPECT_TRUE(service_ != nullptr) << "init service fail!";
        EXPECT_TRUE(appInfo != nullptr) << "appInfo is nullptr!";
        EXPECT_TRUE(abilityInfo != nullptr) << "abilityInfo is nullptr!";
        EXPECT_TRUE(record != nullptr) << "record is nullptr!";
        auto abilityRecord = record->GetAbilityRunningRecordByToken(GetMockToken());
        int32_t id = record->GetRecordId();
        auto name = record->GetName();
        sptr<IRemoteObject> token = abilityRecord->GetToken();
        auto abilityName = abilityRecord->GetName();
        std::string processName = GetTestAppName(index);
        BundleInfo bundleInfo;
        HapModuleInfo hapModuleInfo;
        EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));

        auto appRecordFromServ = service_->appRunningManager_->CheckAppRunningRecordIsExist(
            appInfo->name, processName, appInfo->uid, bundleInfo);
        EXPECT_TRUE(appRecordFromServ);
        auto abilityRecordFromServ = appRecordFromServ->GetAbilityRunningRecordByToken(GetMockToken());
        int32_t idFromServ = appRecordFromServ->GetRecordId();
        sptr<IRemoteObject> tokenFromServ = abilityRecordFromServ->GetToken();
        auto nameFromServ = appRecordFromServ->GetName();
        auto abilityNameFromServ = abilityRecordFromServ->GetName();
        EXPECT_TRUE(id == idFromServ) << "fail, RecordId is not equal!";
        EXPECT_TRUE(tokenFromServ.GetRefPtr() == token.GetRefPtr()) << "fail, token is not equal!";
        EXPECT_EQ(name, nameFromServ) << "fail, app record name is not equal!";
        EXPECT_EQ(abilityName, abilityNameFromServ) << "fail, app record name is not equal!";
    }

    std::unique_ptr<AppMgrServiceInner> service_{ nullptr };

    sptr<MockAbilityToken> GetMockToken() const
    {
        return mockToken_;
    }

private:
    std::vector<std::string> appName_ = {
        "test_app_name1",
        "test_app_name2",
        "test_app_name3",
        "test_app_name4",
        "test_app_name5",
    };
    std::vector<std::string> abilityName_ = {
        "test_ability_name1",
        "test_ability_name2",
        "test_ability_name3",
        "test_ability_name4",
        "test_ability_name5",
    };
    sptr<MockAbilityToken> mockToken_{ nullptr };
};

void AppRunningProcessesInfoModuleTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
}

void AppRunningProcessesInfoModuleTest::TearDownTestCase()
{}

void AppRunningProcessesInfoModuleTest::SetUp()
{
    service_.reset(new (std::nothrow) AppMgrServiceInner());
    mockToken_ = new (std::nothrow) MockAbilityToken();
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AppRunningProcessesInfoModuleTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void AppRunningProcessesInfoModuleTest::MockBundleInstallerAndSA() const
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_))
        .WillOnce(testing::Invoke(mockGetSystemAbility))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, mock object, call query function.
 */
HWTEST_F(AppRunningProcessesInfoModuleTest, ApplicationStart_001, TestSize.Level1)
{
    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, GetHapModuleInfo(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(true))
        .WillRepeatedly(testing::Return(true));
    // init AppRunningRecord
    unsigned long index = 0L;
    int uid = 100;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName(index);
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName(index);
    appInfo->uid = uid;
    std::string processName = GetTestAppName(index);
    RecordQueryResult result;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    // check apprunningrecord
    CheckAppRunningRecording(appInfo, abilityInfo, record, index, result);

    // LaunchApplication
    sptr<MockApplication> mockApplication(new MockApplication());
    std::string testPoint = "ApplicationStart_001";
    CheckLaunchApplication(mockApplication, index, record, testPoint);

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, processName, appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    RunningProcessInfo info;
    service_->GetRunningProcessInfoByToken(GetMockToken(), info);
    EXPECT_TRUE(info.processName_ == processName);
    EXPECT_TRUE(info.uid_ == uid);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, mock object, call query function.
 */
HWTEST_F(AppRunningProcessesInfoModuleTest, ApplicationStart_002, TestSize.Level1)
{
    // init AppRunningRecord
    unsigned long index = 0L;
    int uid = 0;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName(index);
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName(index);
    appInfo->uid = uid;
    std::string processName = GetTestAppName(index);
    RecordQueryResult result;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    // check apprunningrecord
    CheckAppRunningRecording(appInfo, abilityInfo, record, index, result);

    // LaunchApplication
    sptr<MockApplication> mockApplication(new MockApplication());
    std::string testPoint = "ApplicationStart_001";
    CheckLaunchApplication(mockApplication, index, record, testPoint);

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, processName, appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    std::vector<RunningProcessInfo> info;
    size_t infoCount{ 1 };
    record->SetSpawned();
    auto res = service_->GetAllRunningProcesses(info);
    EXPECT_TRUE(res == ERR_OK);
    EXPECT_TRUE(info.size() == infoCount);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat two apprunningrecords, set record state, mock object, call query function.
 */
HWTEST_F(AppRunningProcessesInfoModuleTest, ApplicationStart_003, TestSize.Level1)
{
    // init AppRunningRecord
    unsigned long index = 0L;
    int uid = 0;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName(index);
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName(index);
    appInfo->uid = uid;
    std::string processName = GetTestAppName(index);
    RecordQueryResult result;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    // check apprunningrecord
    CheckAppRunningRecording(appInfo, abilityInfo, record, index, result);

    // LaunchApplication
    sptr<MockApplication> mockApplication(new MockApplication());
    std::string testPoint = "ApplicationStart_001";
    CheckLaunchApplication(mockApplication, index, record, testPoint);

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, processName, appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    index = 1L;
    auto abilityInfo2 = std::make_shared<AbilityInfo>();
    abilityInfo2->name = GetTestAbilityName(index);
    abilityInfo2->applicationInfo.uid = uid;
    auto appInfo2 = std::make_shared<ApplicationInfo>();
    appInfo2->name = GetTestAppName(index);
    appInfo2->uid = uid;
    std::string processName2 = GetTestAppName(index);
    BundleInfo bundleInfo2;
    HapModuleInfo hapModuleInfo2;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo2, appInfo2, bundleInfo2, hapModuleInfo2));
    sptr<MockAbilityToken> mockToken = new (std::nothrow) MockAbilityToken();
    auto record2 = service_->CreateAppRunningRecord(
        mockToken, nullptr, appInfo2, abilityInfo2, processName2, bundleInfo2, hapModuleInfo2, nullptr);
    record2->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    std::vector<RunningProcessInfo> info;
    size_t infoCount{ 2 };
    record->SetSpawned();
    record2->SetSpawned();
    auto res = service_->GetAllRunningProcesses(info);
    EXPECT_TRUE(res == ERR_OK);
    EXPECT_TRUE(info.size() == infoCount);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByToken
 * SubFunction: NA
 * FunctionPoints: get running process info by token.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecords, set record state, mock object, call query function.
 */
HWTEST_F(AppRunningProcessesInfoModuleTest, ApplicationStart_004, TestSize.Level1)
{
    // init AppRunningRecord
    unsigned long index = 0L;
    int uid = 0;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName(index);
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName(index);
    appInfo->uid = uid;
    std::string processName = GetTestAppName(index);
    RecordQueryResult result;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(uid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    CheckAppRunningRecording(appInfo, abilityInfo, record, index, result);

    sptr<MockApplication> mockApplication(new MockApplication());
    std::string testPoint = "ApplicationStart_001";
    CheckLaunchApplication(mockApplication, index, record, testPoint);

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, processName, appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_BACKGROUND);

    RunningProcessInfo info;
    service_->appRunningManager_->GetRunningProcessInfoByToken(GetMockToken(), info);
    EXPECT_TRUE(info.processName_ == processName);
    EXPECT_TRUE(info.uid_ == uid);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByPid
 * SubFunction: NA
 * FunctionPoints: get running process info by pid.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecord, set record state, mock object, call query function.
 */
HWTEST_F(AppRunningProcessesInfoModuleTest, ApplicationStart_005, TestSize.Level1)
{
    // init AppRunningRecord
    unsigned long index = 0L;
    int uid = 100;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName(index);
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName(index);
    appInfo->uid = uid;
    std::string processName = GetTestAppName(index);
    RecordQueryResult result;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(uid);
    pid_t pid = 16738;
    record->GetPriorityObject()->SetPid(pid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    // check apprunningrecord
    CheckAppRunningRecording(appInfo, abilityInfo, record, index, result);

    // LaunchApplication
    sptr<MockApplication> mockApplication(new MockApplication());
    std::string testPoint = "ApplicationStart_005";
    CheckLaunchApplication(mockApplication, index, record, testPoint);

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_FOREGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, processName, appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_FOREGROUND);

    RunningProcessInfo info;
    service_->GetRunningProcessInfoByPid(pid, info);
    EXPECT_TRUE(info.processName_ == processName);
    EXPECT_TRUE(info.uid_ == uid);
}

/*
 * Feature: AppMgrServiceInner
 * Function: GetRunningProcessInfoByPid
 * SubFunction: NA
 * FunctionPoints: get running process info by pid.
 * EnvConditions: NA
 * CaseDescription: creat apprunningrecords, set record state, mock object, call query function.
 */
HWTEST_F(AppRunningProcessesInfoModuleTest, ApplicationStart_006, TestSize.Level1)
{
    // init AppRunningRecord
    unsigned long index = 0L;
    int uid = 0;
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = GetTestAbilityName(index);
    abilityInfo->applicationInfo.uid = uid;
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = GetTestAppName(index);
    appInfo->uid = uid;
    std::string processName = GetTestAppName(index);
    RecordQueryResult result;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    EXPECT_TRUE(service_->GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo));
    auto record = service_->CreateAppRunningRecord(
        GetMockToken(), nullptr, appInfo, abilityInfo, processName, bundleInfo, hapModuleInfo, nullptr);
    record->SetUid(uid);
    pid_t pid = 16739;
    record->GetPriorityObject()->SetPid(pid);
    EXPECT_TRUE(record != nullptr) << ",create apprunningrecord fail!";

    CheckAppRunningRecording(appInfo, abilityInfo, record, index, result);

    sptr<MockApplication> mockApplication(new MockApplication());
    std::string testPoint = "ApplicationStart_006";
    CheckLaunchApplication(mockApplication, index, record, testPoint);

    EXPECT_CALL(*mockApplication, ScheduleForegroundApplication())
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockApplication.GetRefPtr(), &MockApplication::Post));
    // application enter in foreground and check the result
    record->ScheduleForegroundRunning();
    mockApplication->Wait();

    // update application state and check the state
    record->SetState(ApplicationState::APP_STATE_BACKGROUND);
    auto newRecord = service_->appRunningManager_->CheckAppRunningRecordIsExist(
        appInfo->name, processName, appInfo->uid, bundleInfo);
    EXPECT_TRUE(newRecord);
    newRecord->SetUid(uid);
    auto stateFromRec = newRecord->GetState();
    EXPECT_EQ(stateFromRec, ApplicationState::APP_STATE_BACKGROUND);

    RunningProcessInfo info;
    service_->appRunningManager_->GetRunningProcessInfoByPid(pid, info);
    EXPECT_TRUE(info.processName_ == processName);
    EXPECT_TRUE(info.uid_ == uid);
}
}  // namespace AppExecFwk
}  // namespace OHOS
