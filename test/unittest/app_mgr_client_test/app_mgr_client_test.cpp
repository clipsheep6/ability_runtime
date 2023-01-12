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
#define protected public
#include "app_scheduler.h"
#include "app_mgr_client.h"
#include "ability_record.h"
#include "app_mgr_constants.h"
#include "hilog_wrapper.h"
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
{}

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
    int32_t ret = 0;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->PrepareTerminate(token);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: AppMgrClient_AddAbilityStageDone_001
 * @tc.desc: add ability stage done.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_AddAbilityStageDone_001, TestSize.Level0)
{
    int32_t recordId = INIT_VALUE;
    int32_t ret = 0;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->AddAbilityStageDone(recordId);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: AppMgrClient_StartupResidentProcess_001
 * @tc.desc: startup resident process.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_StartupResidentProcess_001, TestSize.Level0)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t ret = 0;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->StartupResidentProcess(bundleInfos);
    EXPECT_EQ(0, ret);
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
    int32_t ret = 0;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->StartSpecifiedAbility(want, abilityInfo);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name: AppMgrClient_RegisterStartSpecifiedAbilityResponse_001
 * @tc.desc: register start specified ability response.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrClientTest, AppMgrClient_RegisterStartSpecifiedAbilityResponse_001, TestSize.Level0)
{
    sptr<IStartSpecifiedAbilityResponse> response = nullptr;
    int32_t ret = 0;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->RegisterStartSpecifiedAbilityResponse(response);
    EXPECT_EQ(0, ret);
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
    int32_t ret = 0;
    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->ScheduleAcceptWantDone(recordId, want, flag);
    EXPECT_EQ(0, ret);
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
    EXPECT_EQ(ret, AppMgrResultCode::RESULT_OK);
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

    int ret = appMgrClient->StartRenderProcess(renderParam, INIT_VALUE, ERROR_PID, renderPid);
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

}  // namespace AppExecFwk
}  // namespace OHOS
