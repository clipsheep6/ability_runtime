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

#define private public
#define protected public
#include "app_scheduler.h"
#include "app_mgr_client.h"
#include "ability_record.h"
#include "app_mgr_constants.h"
#include "hilog_wrapper.h"
#include "mock_ability_debug_response_stub.h"
#include "mock_app_debug_listener_stub.h"
#include "mock_native_token.h"
#include "mock_sa_call.h"
#undef protected
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t APP_NUMBER_ZERO = 0;
const int32_t ERROR_PID = 999999;
const int32_t ERROR_USER_ID = -1;
const int32_t ERROR_STATE = -1;
const std::string EMPTY_STRING = "";
const int32_t INIT_VALUE = 0;
const int32_t ERROR_RET = 3;
}  // namespace

class AppMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName);
};

void AppMgrClientTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrClientTest::TearDownTestCase(void)
{}

void AppMgrClientTest::SetUp()
{}

void AppMgrClientTest::TearDown()
{}

AbilityRequest AppMgrClientTest::GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
    const std::string& appName, const std::string& bundleName)
{
    ElementName element(deviceName, abilityName, bundleName);
    AAFwk::Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.applicationName = appName;
    ApplicationInfo appinfo;
    appinfo.name = appName;

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

/**
 * @tc.name: AppMgrClient_PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->PreStartNWebSpawnProcess();
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_UpdateExtensionState_001
 * @tc.desc: update extension state.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UpdateExtensionState_001, TestSize.Level0)
{
    sptr<IRemoteObject> token;
    ExtensionState state = ExtensionState::EXTENSION_STATE_CREATE;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->UpdateExtensionState(token, state);
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_GetAllRunningProcesses_001
 * @tc.desc: get all running processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAllRunningProcesses_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 start");
    AAFwk::IsMockSaCall::IsMockSaCallWithPermission();
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::vector<RunningProcessInfo> info;
    appMgrClient->GetAllRunningProcesses(info);
    EXPECT_NE(info.size(), APP_NUMBER_ZERO);
    for (int i = 0; i < info.size(); i++) {
        TAG_LOGD(AAFwkTag::TEST, "running %{public}d: name: %{public}s, processType: %{public}d, extensionType: %{public}d",
            i, info[i].processName_.c_str(), info[i].processType_, info[i].extensionType_);
        if (info[i].processName_ == "com.ohos.systemui") {
            EXPECT_EQ(info[i].processType_, ProcessType::EXTENSION);
            EXPECT_EQ(info[i].extensionType_, ExtensionAbilityType::SERVICE);
        } else if (info[i].processName_ == "com.ohos.launcher") {
            EXPECT_EQ(info[i].processType_, ProcessType::EXTENSION);
            EXPECT_EQ(info[i].extensionType_, ExtensionAbilityType::SERVICE);
        }
    }
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningProcesses_001 end");
}

/**
 * @tc.name: AppMgrClient_GetRunningProcessInfoByToken_001
 * @tc.desc: can not get the not running process info by token.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetRunningProcessInfoByToken_001, TestSize.Level0)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::RunningProcessInfo info;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetRunningProcessInfoByToken(token, info);
    EXPECT_EQ(info.bundleNames.size(), APP_NUMBER_ZERO);
}

/**
 * @tc.name: AppMgrClient_GetRunningProcessInfoByPid_001
 * @tc.desc: can not get the not running process info by AccessTokenID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetRunningProcessInfoByPid_001, TestSize.Level0)
{
    AppExecFwk::RunningProcessInfo info;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    pid_t pid = 23689;
    appMgrClient->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(info.bundleNames.size(), APP_NUMBER_ZERO);
}

/**
 * @tc.name: AppMgrClient_GetApplicationInfoByProcessID_001
 * @tc.desc: can not get application info by wrong process ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetApplicationInfoByProcessID_001, TestSize.Level0)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::ApplicationInfo application;
    bool debug = false;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetApplicationInfoByProcessID(ERROR_PID, application, debug);
    EXPECT_EQ(application.bundleName, EMPTY_STRING);
}

/**
 * @tc.name: AppMgrClient_GetAllRenderProcesses_001
 * @tc.desc: get all render processes.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAllRenderProcesses_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRenderProcesses_001 start");
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::vector<RenderProcessInfo> info;
    auto result = appMgrClient->GetAllRenderProcesses(info);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRenderProcesses_001 end");
}

/**
 * @tc.name: AppMgrClient_GetRenderProcessTerminationStatus_001
 * @tc.desc: can not get render process termination status with error pid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetRenderProcessTerminationStatus_001, TestSize.Level0)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    int status;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetRenderProcessTerminationStatus(ERROR_PID, status);
    EXPECT_EQ(status, ERROR_STATE);

    int res = appMgrClient->GetRenderProcessTerminationStatus(0, status);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_GetAbilityRecordsByProcessID_001
 * @tc.desc: can not get ability records by wrong process ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetAbilityRecordsByProcessID_001, TestSize.Level0)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    std::vector<sptr<IRemoteObject>> tokens;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetAbilityRecordsByProcessID(ERROR_PID, tokens);
    EXPECT_EQ(tokens.size(), APP_NUMBER_ZERO);
}

/**
 * @tc.name: AppMgrClient_KillProcessesByUserId_001
 * @tc.desc: can not kill processes by wrong user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_KillProcessesByUserId_001, TestSize.Level0)
{
    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    std::vector<sptr<IRemoteObject>> tokens;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->KillProcessesByUserId(ERROR_USER_ID);
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_StartUserTestProcess_001
 * @tc.desc: can not start user test process with wrong param.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartUserTestProcess_001, TestSize.Level0)
{
    AAFwk::Want want;
    sptr<IRemoteObject> observer = nullptr;
    BundleInfo bundleInfo;
    int32_t userId = INIT_VALUE;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->StartUserTestProcess(want, observer, bundleInfo, userId);
    EXPECT_EQ(ret, ERROR_RET);
}

/**
 * @tc.name: AppMgrClient_KillApplicationSelf_001
 * @tc.desc: kill application self.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_KillApplicationSelf_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->KillApplicationSelf();
    EXPECT_EQ(ret, AppMgrResultCode::ERROR_KILL_APPLICATION);
}

/**
 * @tc.name: AppMgrClient_AbilityAttachTimeOut_001
 * @tc.desc: ability attach timeout.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AbilityAttachTimeOut_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<IRemoteObject> token = nullptr;
    appMgrClient->AbilityAttachTimeOut(token);
}

/**
 * @tc.name: AppMgrClient_PrepareTerminate_001
 * @tc.desc: prepare terminate.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_PrepareTerminate_001, TestSize.Level0)
{
    sptr<IRemoteObject> token = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->PrepareTerminate(token);
}

/**
 * @tc.name: AppMgrClient_AddAbilityStageDone_001
 * @tc.desc: add ability stage done.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AddAbilityStageDone_001, TestSize.Level0)
{
    int32_t recordId = INIT_VALUE;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->AddAbilityStageDone(recordId);
}

/**
 * @tc.name: AppMgrClient_StartupResidentProcess_001
 * @tc.desc: startup resident process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartupResidentProcess_001, TestSize.Level0)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->StartupResidentProcess(bundleInfos);
}

/**
 * @tc.name: AppMgrClient_StartSpecifiedAbility_001
 * @tc.desc: start specified ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartSpecifiedAbility_001, TestSize.Level0)
{
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->StartSpecifiedAbility(want, abilityInfo);
}

/**
 * @tc.name: AppMgrClient_RegisterStartSpecifiedAbilityResponse_001
 * @tc.desc: register start specified ability response.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterStartSpecifiedAbilityResponse_001, TestSize.Level0)
{
    sptr<IStartSpecifiedAbilityResponse> response = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->RegisterStartSpecifiedAbilityResponse(response);
}

/**
 * @tc.name: AppMgrClient_ScheduleAcceptWantDone_001
 * @tc.desc: schedule accept want done.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ScheduleAcceptWantDone_001, TestSize.Level0)
{
    int32_t recordId = INIT_VALUE;
    AAFwk::Want want;
    std::string flag = EMPTY_STRING;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->ScheduleAcceptWantDone(recordId, want, flag);
}

/**
 * @tc.name: AppMgrClient_FinishUserTest_001
 * @tc.desc: finish user test.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_FinishUserTest_001, TestSize.Level0)
{
    std::string msg = EMPTY_STRING;
    int64_t resultCode = INIT_VALUE;
    std::string bundleName;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->FinishUserTest(msg, resultCode, bundleName);
    EXPECT_NE(ret, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_StartRenderProcess_001
 * @tc.desc: can not start render process with wrong param.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartRenderProcess_001, TestSize.Level0)
{
    std::string renderParam = EMPTY_STRING;
    pid_t renderPid;

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->StartRenderProcess(renderParam, INIT_VALUE, ERROR_PID, INIT_VALUE, renderPid);
    EXPECT_EQ(ret, ERROR_STATE);
}

#ifdef ABILITY_COMMAND_FOR_TEST
/**
 * @tc.name: AppMgrClient_BlockAppService_001
 * @tc.desc: block app service.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_BlockAppService_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int ret = appMgrClient->BlockAppService();
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
}
#endif

/**
 * @tc.name: AppMgrClient_SetCurrentUserId_001
 * @tc.desc: set current userId.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_SetCurrentUserId_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t userId = 0;
    appMgrClient->SetCurrentUserId(userId);
}

/**
 * @tc.name: AppMgrClient_UpdateApplicationInfoInstalled_001
 * @tc.desc: UpdateApplicationInfoInstalled.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UpdateApplicationInfoInstalled_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "";
    int uid = 1;
    appMgrClient->UpdateApplicationInfoInstalled(bundleName, uid);
}

/**
 * @tc.name: AppMgrClient_GetProcessRunningInformation_001
 * @tc.desc: GetProcessRunningInformation.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetProcessRunningInformation_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    AppExecFwk::RunningProcessInfo info;
    appMgrClient->GetProcessRunningInformation(info);
}

/**
 * @tc.name: AppMgrClient_DumpHeapMemory_001
 * @tc.desc: DumpHeapMemory.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DumpHeapMemory_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    int32_t pid = 1;
    OHOS::AppExecFwk::MallocInfo mallocInfo;
    appMgrClient->DumpHeapMemory(pid, mallocInfo);
}

/**
 * @tc.name: AppMgrClient_StartNativeProcessForDebugger_001
 * @tc.desc: StartNativeProcessForDebugger.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartNativeProcessForDebugger_001, TestSize.Level0)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    AAFwk::Want want;
    appMgrClient->StartNativeProcessForDebugger(want);
}

/**
 * @tc.name: AppMgrClient_GetBundleNameByPid_001
 * @tc.desc: GetBundleNameByPid.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_GetBundleNameByPid_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    int32_t pid = 0;
    std::string name = "test";
    int32_t uid = 0;
    auto ret = appMgrClient->GetBundleNameByPid(pid, name, uid);
    EXPECT_EQ(ret, NO_ERROR);
}

/**
 * @tc.name: AppMgrClient_NotifyAppFault_001
 * @tc.desc: NotifyAppFault.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyAppFault_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    FaultData faultData;
    auto resultCode = appMgrClient->NotifyAppFault(faultData);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_NotifyAppFaultBySA_001
 * @tc.desc: NotifyAppFaultBySA.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyAppFaultBySA_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    AppFaultDataBySA faultData;
    auto resultCode = appMgrClient->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(resultCode, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_ChangeAppGcState_001
 * @tc.desc: ChangeAppGcState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ChangeAppGcState_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);
    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    pid_t pid = 0;
    int32_t state = 0;
    auto resultCode = appMgrClient->ChangeAppGcState(pid, state);
    EXPECT_EQ(resultCode, NO_ERROR);
}

/**
 * @tc.name: AppMgrClient_RegisterAppDebugListener_001
 * @tc.desc: Register app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterAppDebugListener_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockAppDebugListenerStub> listener = new MockAppDebugListenerStub();
    EXPECT_NE(listener, nullptr);
    auto resultCode = appMgrClient->RegisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_OK);

    listener = nullptr;
    resultCode = appMgrClient->RegisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_UnregisterAppDebugListener_001
 * @tc.desc: Unregister app debug listener, check nullptr listener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterAppDebugListener_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    
    sptr<MockAppDebugListenerStub> listener = new MockAppDebugListenerStub();
    EXPECT_NE(listener, nullptr);
    auto resultCode = appMgrClient->UnregisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_OK);

    listener = nullptr;
    resultCode = appMgrClient->UnregisterAppDebugListener(listener);
    EXPECT_EQ(resultCode, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_RegisterAbilityDebugResponse_001
 * @tc.desc: Register ability debug response, check nullptr response.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterAbilityDebugResponse_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    sptr<MockAbilityDebugResponseStub> response = nullptr;
    auto resultCode = appMgrClient->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(resultCode, ERR_INVALID_DATA);

    response = new MockAbilityDebugResponseStub();
    EXPECT_NE(response, nullptr);
    resultCode = appMgrClient->RegisterAbilityDebugResponse(response);
    EXPECT_EQ(resultCode, NO_ERROR);
}

/**
 * @tc.name: AppMgrClient_AttachAppDebug_001
 * @tc.desc: Attach app, begin debug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AttachAppDebug_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
    
    std::string bundleName = "bundleName";
    auto resultCode = appMgrClient->AttachAppDebug(bundleName);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: AppMgrClient_DetachAppDebug_001
 * @tc.desc: Detach app, end debug.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_DetachAppDebug_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    std::string bundleName = "bundleName";
    auto resultCode = appMgrClient->DetachAppDebug(bundleName);
    EXPECT_EQ(resultCode, ERR_OK);
}

/**
 * @tc.name: AppMgrClient_RegisterApplicationStateObserver_001
 * @tc.desc: RegisterApplicationStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterApplicationStateObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<IApplicationStateObserver> observer = nullptr;
    std::vector<std::string> bundleNameList;
    auto result = appMgrClient->RegisterApplicationStateObserver(observer, bundleNameList);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_UnregisterApplicationStateObserver_001
 * @tc.desc: UnregisterApplicationStateObserver.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterApplicationStateObserver_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    sptr<IApplicationStateObserver> observer = nullptr;
    auto result = appMgrClient->UnregisterApplicationStateObserver(observer);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppMgrClient_NotifyPageShow_001
 * @tc.desc: NotifyPageShow.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyPageShow_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::RunningProcessInfo info;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    PageStateData pageStateData;
    auto result = appMgrClient->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_NotifyPageHide_001
 * @tc.desc: NotifyPageHide.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_NotifyPageHide_001, TestSize.Level1)
{
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    std::string deviceName = "device";
    std::string abilityName = "FirstAbility";
    std::string appName = "FirstApp";
    std::string bundleName = "com.ix.First.Test";
    AppExecFwk::RunningProcessInfo info;
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();
    PageStateData pageStateData;
    auto result = appMgrClient->NotifyPageHide(token, pageStateData);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_StartSpecifiedProcess_001
 * @tc.desc: StartSpecifiedProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartSpecifiedProcess_001, TestSize.Level1)
{
    AAFwk::Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    appMgrClient->StartSpecifiedProcess(want, abilityInfo);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_ScheduleNewProcessRequest_001
 * @tc.desc: schedule accept want done.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ScheduleNewProcessRequest_001, TestSize.Level0)
{
    int32_t recordId = INIT_VALUE;
    AAFwk::Want want;
    std::string flag = EMPTY_STRING;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    appMgrClient->ScheduleNewProcessRequest(recordId, want, flag);
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);
}

/**
 * @tc.name: AppMgrClient_RegisterAppRunningStatusListener_001
 * @tc.desc: RegisterAppRunningStatusListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterAppRunningStatusListener_001, TestSize.Level0)
{
    sptr<IRemoteObject> listener = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->RegisterAppRunningStatusListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_UnregisterAppRunningStatusListener_001
 * @tc.desc: UnregisterAppRunningStatusListener.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_UnregisterAppRunningStatusListener_001, TestSize.Level0)
{
    sptr<IRemoteObject> listener = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->UnregisterAppRunningStatusListener(listener);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: AppMgrClient_IsFinalAppProcess_001
 * @tc.desc: IsFinalAppProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_IsFinalAppProcess_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_IsFinalAppProcess_001 start");
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    auto ret = appMgrClient->IsFinalAppProcess();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrClient_IsFinalAppProcess_001 end");
}

/**
 * @tc.name: AppMgrClient_ClearProcessByToken_001
 * @tc.desc: ClearProcessByToken.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_ClearProcessByToken_001, TestSize.Level0)
{
    sptr<IRemoteObject> token = nullptr;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    appMgrClient->ClearProcessByToken(token);
    EXPECT_NE(appMgrClient, nullptr);
}
}  // namespace AppExecFwk
}  // namespace OHOS
