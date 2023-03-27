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

#include <gtest/gtest.h>

#define private public
#include "app_mgr_service.h"
#undef private
#include "mock_native_token.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<EventRunner> runner_ {nullptr};
};

void AppMgrServiceTest::SetUpTestCase(void)
{}

void AppMgrServiceTest::TearDownTestCase(void)
{}

void AppMgrServiceTest::SetUp()
{
    runner_ = EventRunner::Create(Constants::APP_MGR_SERVICE_NAME);
}

void AppMgrServiceTest::TearDown()
{}

/*
 * Feature: AppMgrService
 * Function: OnStop
 * SubFunction: NA
 * FunctionPoints: AppMgrService OnStop
 * EnvConditions: NA
 * CaseDescription: Verify OnStop
 */
HWTEST_F(AppMgrServiceTest, OnStop_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    appMgrService->OnStop();
}

/*
 * Feature: AppMgrService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: AppMgrService QueryServiceState
 * EnvConditions: NA
 * CaseDescription: Verify QueryServiceState
 */
HWTEST_F(AppMgrServiceTest, QueryServiceState_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    appMgrService->appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_RUNNING;
    auto res = appMgrService->QueryServiceState();
    EXPECT_EQ(res.serviceRunningState, ServiceRunningState::STATE_RUNNING);
}

/*
 * Feature: AppMgrService
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AppMgrService Init
 * EnvConditions: NA
 * CaseDescription: Verify Init
 */
HWTEST_F(AppMgrServiceTest, Init_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    ErrCode res = appMgrService->Init();
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: AttachApplication
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachApplication
 * EnvConditions: NA
 * CaseDescription: Verify AttachApplication
 */
HWTEST_F(AppMgrServiceTest, AttachApplication_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IRemoteObject> app = nullptr;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AttachApplication(app);
}

/*
 * Feature: AppMgrService
 * Function: AttachApplication
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachApplication
 * EnvConditions: NA
 * CaseDescription: Verify AttachApplication
 */
HWTEST_F(AppMgrServiceTest, AttachApplication_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IRemoteObject> app = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->AttachApplication(app);
}

/*
 * Feature: AppMgrService
 * Function: ApplicationForegrounded
 * SubFunction: NA
 * FunctionPoints: AppMgrService ApplicationForegrounded
 * EnvConditions: NA
 * CaseDescription: Verify ApplicationForegrounded
 */
HWTEST_F(AppMgrServiceTest, ApplicationForegrounded_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->ApplicationForegrounded(recordId);
}

/*
 * Feature: AppMgrService
 * Function: ApplicationBackgrounded
 * SubFunction: NA
 * FunctionPoints: AppMgrService ApplicationBackgrounded
 * EnvConditions: NA
 * CaseDescription: Verify ApplicationBackgrounded
 */
HWTEST_F(AppMgrServiceTest, ApplicationBackgrounded_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->ApplicationBackgrounded(recordId);
}

/*
 * Feature: AppMgrService
 * Function: ApplicationTerminated
 * SubFunction: NA
 * FunctionPoints: AppMgrService ApplicationTerminated
 * EnvConditions: NA
 * CaseDescription: Verify ApplicationTerminated
 */
HWTEST_F(AppMgrServiceTest, ApplicationTerminated_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->ApplicationTerminated(recordId);
}

/*
 * Feature: AppMgrService
 * Function: AbilityCleaned
 * SubFunction: NA
 * FunctionPoints: AppMgrService AbilityCleaned
 * EnvConditions: NA
 * CaseDescription: Verify AbilityCleaned
 */
HWTEST_F(AppMgrServiceTest, AbilityCleaned_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    appMgrService->AbilityCleaned(nullptr);
}

/*
 * Feature: AppMgrService
 * Function: AbilityCleaned
 * SubFunction: NA
 * FunctionPoints: AppMgrService AbilityCleaned
 * EnvConditions: NA
 * CaseDescription: Verify AbilityCleaned
 */
HWTEST_F(AppMgrServiceTest, AbilityCleaned_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->AbilityCleaned(nullptr);
}

/*
 * Feature: AppMgrService
 * Function: AddAppDeathRecipient
 * SubFunction: NA
 * FunctionPoints: AppMgrService AddAppDeathRecipient
 * EnvConditions: NA
 * CaseDescription: Verify AddAppDeathRecipient
 */
HWTEST_F(AppMgrServiceTest, AddAppDeathRecipient_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t pid = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AddAppDeathRecipient(pid);
}

/*
 * Feature: AppMgrService
 * Function: AddAppDeathRecipient
 * SubFunction: NA
 * FunctionPoints: AppMgrService AddAppDeathRecipient
 * EnvConditions: NA
 * CaseDescription: Verify AddAppDeathRecipient
 */
HWTEST_F(AppMgrServiceTest, AddAppDeathRecipient_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t pid = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->AddAppDeathRecipient(pid);
}

/*
 * Feature: AppMgrService
 * Function: StartupResidentProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartupResidentProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartupResidentProcess
 */
HWTEST_F(AppMgrServiceTest, StartupResidentProcess_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    appMgrService->SetInnerService(nullptr);
    appMgrService->StartupResidentProcess(bundleInfos);
}

/*
 * Feature: AppMgrService
 * Function: StartupResidentProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartupResidentProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartupResidentProcess
 */
HWTEST_F(AppMgrServiceTest, StartupResidentProcess_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->StartupResidentProcess(bundleInfos);
}

/*
 * Feature: AppMgrService
 * Function: ClearUpApplicationData
 * SubFunction: NA
 * FunctionPoints: AppMgrService ClearUpApplicationData
 * EnvConditions: NA
 * CaseDescription: Verify ClearUpApplicationData
 */
HWTEST_F(AppMgrServiceTest, ClearUpApplicationData_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->ClearUpApplicationData(bundleName);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningProcesses_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RunningProcessInfo> info;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetAllRunningProcesses(info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAllRunningProcesses
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAllRunningProcesses
 * EnvConditions: NA
 * CaseDescription: Verify GetAllRunningProcesses
 */
HWTEST_F(AppMgrServiceTest, GetAllRunningProcesses_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RunningProcessInfo> info;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetAllRunningProcesses(info);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetProcessRunningInfosByUserId
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetProcessRunningInfosByUserId
 * EnvConditions: NA
 * CaseDescription: Verify GetProcessRunningInfosByUserId
 */
HWTEST_F(AppMgrServiceTest, GetProcessRunningInfosByUserId_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<RunningProcessInfo> info;
    int32_t userId = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetProcessRunningInfosByUserId(info, userId);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyMemoryLevel
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyMemoryLevel
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMemoryLevel
 */
HWTEST_F(AppMgrServiceTest, NotifyMemoryLevel_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t level = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyMemoryLevel(level);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyMemoryLevel
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyMemoryLevel
 * EnvConditions: NA
 * CaseDescription: Verify NotifyMemoryLevel
 */
HWTEST_F(AppMgrServiceTest, NotifyMemoryLevel_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t level = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->NotifyMemoryLevel(level);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: AddAbilityStageDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService AddAbilityStageDone
 * EnvConditions: NA
 * CaseDescription: Verify AddAbilityStageDone
 */
HWTEST_F(AppMgrServiceTest, AddAbilityStageDone_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AddAbilityStageDone(recordId);
}

/*
 * Feature: AppMgrService
 * Function: AddAbilityStageDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService AddAbilityStageDone
 * EnvConditions: NA
 * CaseDescription: Verify AddAbilityStageDone
 */
HWTEST_F(AppMgrServiceTest, AddAbilityStageDone_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->AddAbilityStageDone(recordId);
}

/*
 * Feature: AppMgrService
 * Function: RegisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterApplicationStateObserver
 */
HWTEST_F(AppMgrServiceTest, RegisterApplicationStateObserver_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IApplicationStateObserver> observer = nullptr;
    std::vector<std::string> bundleNameList;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->RegisterApplicationStateObserver(observer, bundleNameList);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterApplicationStateObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterApplicationStateObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterApplicationStateObserver
 */
HWTEST_F(AppMgrServiceTest, UnregisterApplicationStateObserver_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IApplicationStateObserver> observer = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UnregisterApplicationStateObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetForegroundApplications
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetForegroundApplications
 * EnvConditions: NA
 * CaseDescription: Verify GetForegroundApplications
 */
HWTEST_F(AppMgrServiceTest, GetForegroundApplications_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<AppStateData> list;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetForegroundApplications(list);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetForegroundApplications
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetForegroundApplications
 * EnvConditions: NA
 * CaseDescription: Verify GetForegroundApplications
 */
HWTEST_F(AppMgrServiceTest, GetForegroundApplications_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<AppStateData> list;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetForegroundApplications(list);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: StartUserTestProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartUserTestProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartUserTestProcess
 */
HWTEST_F(AppMgrServiceTest, StartUserTestProcess_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    BundleInfo bundleInfo;
    int32_t userId = 1;
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->StartUserTestProcess(want, observer, bundleInfo, userId);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: StartUserTestProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartUserTestProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartUserTestProcess
 */
HWTEST_F(AppMgrServiceTest, StartUserTestProcess_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Want want;
    sptr<IRemoteObject> observer = nullptr;
    BundleInfo bundleInfo;
    int32_t userId = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int res = appMgrService->StartUserTestProcess(want, observer, bundleInfo, userId);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AppMgrService FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify FinishUserTest
 */
HWTEST_F(AppMgrServiceTest, FinishUserTest_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string msg = "msg";
    int64_t resultCode = 1;
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: FinishUserTest
 * SubFunction: NA
 * FunctionPoints: AppMgrService FinishUserTest
 * EnvConditions: NA
 * CaseDescription: Verify FinishUserTest
 */
HWTEST_F(AppMgrServiceTest, FinishUserTest_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string msg = "msg";
    int64_t resultCode = 1;
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int res = appMgrService->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: ScheduleAcceptWantDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService ScheduleAcceptWantDone
 * EnvConditions: NA
 * CaseDescription: Verify ScheduleAcceptWantDone
 */
HWTEST_F(AppMgrServiceTest, ScheduleAcceptWantDone_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    Want want;
    std::string flag = "flag";
    appMgrService->SetInnerService(nullptr);
    appMgrService->ScheduleAcceptWantDone(recordId, want, flag);
}

/*
 * Feature: AppMgrService
 * Function: ScheduleAcceptWantDone
 * SubFunction: NA
 * FunctionPoints: AppMgrService ScheduleAcceptWantDone
 * EnvConditions: NA
 * CaseDescription: Verify ScheduleAcceptWantDone
 */
HWTEST_F(AppMgrServiceTest, ScheduleAcceptWantDone_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t recordId = 1;
    Want want;
    std::string flag = "flag";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->ScheduleAcceptWantDone(recordId, want, flag);
}

/*
 * Feature: AppMgrService
 * Function: GetAbilityRecordsByProcessID
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAbilityRecordsByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordsByProcessID
 */
HWTEST_F(AppMgrServiceTest, GetAbilityRecordsByProcessID_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    std::vector<sptr<IRemoteObject>> tokens;
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->GetAbilityRecordsByProcessID(pid, tokens);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetAbilityRecordsByProcessID
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAbilityRecordsByProcessID
 * EnvConditions: NA
 * CaseDescription: Verify GetAbilityRecordsByProcessID
 */
HWTEST_F(AppMgrServiceTest, GetAbilityRecordsByProcessID_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    std::vector<sptr<IRemoteObject>> tokens;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int res = appMgrService->GetAbilityRecordsByProcessID(pid, tokens);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int ret = appMgrService->PreStartNWebSpawnProcess();
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_002
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceTest, PreStartNWebSpawnProcess_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int ret = appMgrService->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: StartRenderProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartRenderProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartRenderProcess
 */
HWTEST_F(AppMgrServiceTest, StartRenderProcess_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string renderParam = "renderParam";
    int32_t ipcFd = 1;
    int32_t sharedFd = 1;
    pid_t renderPid = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->StartRenderProcess(renderParam, ipcFd, sharedFd, renderPid);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: StartRenderProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService StartRenderProcess
 * EnvConditions: NA
 * CaseDescription: Verify StartRenderProcess
 */
HWTEST_F(AppMgrServiceTest, StartRenderProcess_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string renderParam = "renderParam";
    int32_t ipcFd = 1;
    int32_t sharedFd = 1;
    pid_t renderPid = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->StartRenderProcess(renderParam, ipcFd, sharedFd, renderPid);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: AttachRenderProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachRenderProcess
 * EnvConditions: NA
 * CaseDescription: Verify AttachRenderProcess
 */
HWTEST_F(AppMgrServiceTest, AttachRenderProcess_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IRemoteObject> scheduler = nullptr;
    appMgrService->SetInnerService(nullptr);
    appMgrService->AttachRenderProcess(scheduler);
}

/*
 * Feature: AppMgrService
 * Function: AttachRenderProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService AttachRenderProcess
 * EnvConditions: NA
 * CaseDescription: Verify AttachRenderProcess
 */
HWTEST_F(AppMgrServiceTest, AttachRenderProcess_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IRemoteObject> scheduler = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    appMgrService->AttachRenderProcess(scheduler);
}

/*
 * Feature: AppMgrService
 * Function: GetRenderProcessTerminationStatus
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRenderProcessTerminationStatus
 * EnvConditions: NA
 * CaseDescription: Verify GetRenderProcessTerminationStatus
 */
HWTEST_F(AppMgrServiceTest, GetRenderProcessTerminationStatus_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t renderPid = 1;
    int status = 1;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetRenderProcessTerminationStatus
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetRenderProcessTerminationStatus
 * EnvConditions: NA
 * CaseDescription: Verify GetRenderProcessTerminationStatus
 */
HWTEST_F(AppMgrServiceTest, GetRenderProcessTerminationStatus_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    pid_t renderPid = 1;
    int status = 1;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppMgrServiceTest, GetConfiguration_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->GetConfiguration(config);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: GetConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify GetConfiguration
 */
HWTEST_F(AppMgrServiceTest, GetConfiguration_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->GetConfiguration(config);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfiguration
 */
HWTEST_F(AppMgrServiceTest, UpdateConfiguration_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UpdateConfiguration(config);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UpdateConfiguration
 * SubFunction: NA
 * FunctionPoints: AppMgrService UpdateConfiguration
 * EnvConditions: NA
 * CaseDescription: Verify UpdateConfiguration
 */
HWTEST_F(AppMgrServiceTest, UpdateConfiguration_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    Configuration config;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UpdateConfiguration(config);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: RegisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, RegisterConfigurationObserver_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->RegisterConfigurationObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: RegisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService RegisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, RegisterConfigurationObserver_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->RegisterConfigurationObserver(observer);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, UnregisterConfigurationObserver_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->UnregisterConfigurationObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: UnregisterConfigurationObserver
 * SubFunction: NA
 * FunctionPoints: AppMgrService UnregisterConfigurationObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterConfigurationObserver
 */
HWTEST_F(AppMgrServiceTest, UnregisterConfigurationObserver_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    sptr<IConfigurationObserver> observer = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->UnregisterConfigurationObserver(observer);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/*
 * Feature: AppMgrService
 * Function: BlockAppService
 * SubFunction: NA
 * FunctionPoints: AppMgrService BlockAppService
 * EnvConditions: NA
 * CaseDescription: Verify BlockAppService
 */
HWTEST_F(AppMgrServiceTest, BlockAppService_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(nullptr);
    int res = appMgrService->BlockAppService();
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: BlockAppService
 * SubFunction: NA
 * FunctionPoints: AppMgrService BlockAppService
 * EnvConditions: NA
 * CaseDescription: Verify BlockAppService
 */
HWTEST_F(AppMgrServiceTest, BlockAppService_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int res = appMgrService->BlockAppService();
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}
#endif

/*
 * Feature: AppMgrService
 * Function: GetAppRunningStateByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAppRunningStateByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetAppRunningStateByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetAppRunningStateByBundleName_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(nullptr);
    bool res = appMgrService->GetAppRunningStateByBundleName(bundleName);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppMgrService
 * Function: GetAppRunningStateByBundleName
 * SubFunction: NA
 * FunctionPoints: AppMgrService GetAppRunningStateByBundleName
 * EnvConditions: NA
 * CaseDescription: Verify GetAppRunningStateByBundleName
 */
HWTEST_F(AppMgrServiceTest, GetAppRunningStateByBundleName_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    bool res = appMgrService->GetAppRunningStateByBundleName(bundleName);
    EXPECT_FALSE(res);
}

/*
 * Feature: AppMgrService
 * Function: NotifyLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyLoadRepairPatch_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyLoadRepairPatch_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int32_t res = appMgrService->NotifyLoadRepairPatch(bundleName, callback);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyHotReloadPage
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyHotReloadPage
 * EnvConditions: NA
 * CaseDescription: Verify NotifyHotReloadPage
 */
HWTEST_F(AppMgrServiceTest, NotifyHotReloadPage_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyHotReloadPage(bundleName, callback);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyHotReloadPage
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyHotReloadPage
 * EnvConditions: NA
 * CaseDescription: Verify NotifyHotReloadPage
 */
HWTEST_F(AppMgrServiceTest, NotifyHotReloadPage_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int32_t res = appMgrService->NotifyHotReloadPage(bundleName, callback);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
/*
 * Feature: AppMgrService
 * Function: SetContinuousTaskProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetContinuousTaskProcess
 * EnvConditions: NA
 * CaseDescription: Verify SetContinuousTaskProcess
 */
HWTEST_F(AppMgrServiceTest, SetContinuousTaskProcess_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t pid = 1;
    bool isContinuousTask = false;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->SetContinuousTaskProcess(pid, isContinuousTask);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: SetContinuousTaskProcess
 * SubFunction: NA
 * FunctionPoints: AppMgrService SetContinuousTaskProcess
 * EnvConditions: NA
 * CaseDescription: Verify SetContinuousTaskProcess
 */
HWTEST_F(AppMgrServiceTest, SetContinuousTaskProcess_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int32_t pid = 1;
    bool isContinuousTask = false;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    int32_t res = appMgrService->SetContinuousTaskProcess(pid, isContinuousTask);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}
#endif

/*
 * Feature: AppMgrService
 * Function: NotifyUnLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyUnLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyUnLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyUnLoadRepairPatch_001, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(nullptr);
    int32_t res = appMgrService->NotifyUnLoadRepairPatch(bundleName, callback);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
}

/*
 * Feature: AppMgrService
 * Function: NotifyUnLoadRepairPatch
 * SubFunction: NA
 * FunctionPoints: AppMgrService NotifyUnLoadRepairPatch
 * EnvConditions: NA
 * CaseDescription: Verify NotifyUnLoadRepairPatch
 */
HWTEST_F(AppMgrServiceTest, NotifyUnLoadRepairPatch_002, TestSize.Level0)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string bundleName = "bundleName";
    sptr<IQuickFixCallback> callback = nullptr;
    appMgrService->SetInnerService(std::make_shared<AppMgrServiceInner>());
    appMgrService->handler_ = std::make_shared<AMSEventHandler>(runner_, appMgrService->appMgrServiceInner_);
    OHOS::AppExecFwk::MockNativeToken::SetNativeToken();
    int32_t res = appMgrService->NotifyUnLoadRepairPatch(bundleName, callback);
    EXPECT_NE(res, ERR_INVALID_OPERATION);
}
} // namespace AppExecFwk
} // namespace OHOS
