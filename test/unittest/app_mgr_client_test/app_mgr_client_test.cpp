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
}  // namespace

class AppMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    AbilityRequest GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
        const std::string &appName, const std::string &bundleName);
};

void AppMgrClientTest::SetUpTestCase(void)
{}

void AppMgrClientTest::TearDownTestCase(void)
{}

void AppMgrClientTest::SetUp()
{}

void AppMgrClientTest::TearDown()
{}

AbilityRequest AppMgrClientTest::GenerateAbilityRequest(const std::string &deviceName, const std::string &abilityName,
    const std::string &appName, const std::string &bundleName)
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
    auto abilityReq = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName);
    auto record = AbilityRecord::CreateAbilityRecord(abilityReq);
    auto token = record->GetToken();

    auto appMgrClient = std::make_unique<AppMgrClient>();
    EXPECT_NE(appMgrClient, nullptr);

    auto result = appMgrClient->ConnectAppMgrService();
    EXPECT_EQ(result, AppMgrResultCode::RESULT_OK);

    appMgrClient->GetApplicationInfoByProcessID(ERROR_PID, application);
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
}  // namespace AppExecFwk
}  // namespace OHOS
