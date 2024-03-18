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
#include "app_foreground_state_observer_stub.h"
#include "app_mgr_stub.h"
#undef private
#undef protected

#include "hilog_wrapper.h"
#include "mock_app_mgr_service.h"
#include "render_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
}  // namespace

class AppForegroundStateObserverMock : public AppForegroundStateObserverStub {
public:
    AppForegroundStateObserverMock() = default;
    virtual ~AppForegroundStateObserverMock() = default;
    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
};

class RenderStateObserverMock : public RenderStateObserverStub {
public:
    RenderStateObserverMock() = default;
    virtual ~RenderStateObserverMock() = default;
    void OnRenderStateChanged(pid_t renderPid, int32_t state) override
    {}
};

class AppMgrStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppMgrService> mockAppMgrService_;

    void WriteInterfaceToken(MessageParcel& data);
};

void AppMgrStubTest::SetUpTestCase(void)
{}

void AppMgrStubTest::TearDownTestCase(void)
{}

void AppMgrStubTest::SetUp()
{
    GTEST_LOG_(INFO) << "AppMgrStubTest::SetUp()";

    mockAppMgrService_ = new MockAppMgrService();
}

void AppMgrStubTest::TearDown()
{}

void AppMgrStubTest::WriteInterfaceToken(MessageParcel& data)
{
    GTEST_LOG_(INFO) << "AppMgrStubTest::WriteInterfaceToken()";

    data.WriteInterfaceToken(AppMgrStub::GetDescriptor());
}

/**
 * @tc.name: AppMgrStub_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrStubTest, AppMgrStub_GetProcessRunningInfosByUserId_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AppMgrStub_GetProcessRunningInfosByUserId_0100 start";

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteInt32(USER_ID);

    EXPECT_CALL(*mockAppMgrService_, GetProcessRunningInfosByUserId(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    GTEST_LOG_(INFO) << "AppMgrStub_GetProcessRunningInfosByUserId_0100 end";
}

/**
 * @tc.name: HandleGetAppRunningStateByBundleName_0100
 * @tc.desc: Handle get app running state by bundle name.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleGetAppRunningStateByBundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, GetAppRunningStateByBundleName(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_STATE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyLoadRepairPatch_0100
 * @tc.desc: Handle notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyLoadRepairPatch_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, NotifyLoadRepairPatch(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_LOAD_REPAIR_PATCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyHotReloadPage_0100
 * @tc.desc: Handle notify ace hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyHotReloadPage_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, NotifyHotReloadPage(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_HOT_RELOAD_PAGE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyUnLoadRepairPatch_0100
 * @tc.desc: Handle notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyUnLoadRepairPatch_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, NotifyUnLoadRepairPatch(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_UNLOAD_REPAIR_PATCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrStubTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    EXPECT_CALL(*mockAppMgrService_, PreStartNWebSpawnProcess()).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetProcessMemoryByPid_001
 * @tc.desc: Get memorySize by pid.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrStubTest, GetProcessMemoryByPid_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int32_t pid = 0;
    data.WriteInt32(pid);
    
    EXPECT_CALL(*mockAppMgrService_, GetProcessMemoryByPid(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_PROCESS_MEMORY_BY_PID), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunningProcessInformation_001
 * @tc.desc: Get pid list by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrStubTest, GetRunningProcessInformation_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    int32_t userId = 0;
    data.WriteString(bundleName);
    data.WriteInt32(userId);
    
    EXPECT_CALL(*mockAppMgrService_, GetRunningProcessInformation(_, _, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::GET_PIDS_BY_BUNDLENAME), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyFault_001
 * @tc.desc: Handle notify fault.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppMgrStubTest, HandleNotifyFault_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    FaultData faultData;
    faultData.errorObject.name = "testName";
    faultData.errorObject.message = "testMessage";
    faultData.errorObject.stack = "testStack";
    faultData.faultType = FaultDataType::UNKNOWN;
    data.WriteParcelable(&faultData);
    EXPECT_CALL(*mockAppMgrService_, NotifyAppFault(_)).Times(1);
    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleNotifyFaultBySA_001
 * @tc.desc: Handle notify fault by SA.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppMgrStubTest, HandleNotifyFaultBySA_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    AppFaultDataBySA faultData;
    faultData.errorObject.name = "testName";
    faultData.errorObject.message = "testMessage";
    faultData.errorObject.stack = "testStack";
    faultData.faultType = FaultDataType::UNKNOWN;
    faultData.pid = 24;
    data.WriteParcelable(&faultData);
    EXPECT_CALL(*mockAppMgrService_, NotifyAppFaultBySA(_)).Times(1);
    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleChangeAppGcState_001
 * @tc.desc: Handle change app Gc state.
 * @tc.type: FUNC
 * @tc.require: issuesI85VVU
 */
HWTEST_F(AppMgrStubTest, HandleChangeAppGcState_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);
    data.WriteInt32(0);
    data.WriteInt32(0);
    auto result = mockAppMgrService_->OnRemoteRequest(
            static_cast<uint32_t>(AppMgrInterfaceCode::CHANGE_APP_GC_STATE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: IsApplicationRunning_001
 * @tc.desc: On remote request to query the running status of the application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, IsApplicationRunning_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    bool isRunning = false;
    data.WriteString(bundleName);
    data.WriteBool(isRunning);

    EXPECT_CALL(*mockAppMgrService_, IsApplicationRunning(_, _)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(AppMgrInterfaceCode::IS_APPLICATION_RUNNING), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.number: HandleRegisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when write result success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = mockAppMgrService_->HandleRegisterAbilityForegroundStateObserver(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.number: HandleUnregisterAbilityForegroundStateObserver_0100
 * @tc.desc: Verify it when write result success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterAbilityForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = mockAppMgrService_->HandleUnregisterAbilityForegroundStateObserver(data, reply);
    EXPECT_EQ(result, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterAppForegroundStateObserver_0100
 * @tc.desc: Test when callback is not nullptr the return of writeInt32 is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) AppForegroundStateObserverMock();
    data.WriteRemoteObject(object);
    int32_t pid = 1;
    reply.WriteInt32(pid);
    auto res = mockAppMgrService_->HandleRegisterAppForegroundStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test when callback is not nullptr the return of writeInt32 is true.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) AppForegroundStateObserverMock();
    data.WriteRemoteObject(object);
    int32_t pid = 1;
    reply.WriteInt32(pid);
    auto res = mockAppMgrService_->HandleUnregisterAppForegroundStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleRegisterRenderStateObserver_0100
 * @tc.desc: Test register observer success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleRegisterRenderStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) RenderStateObserverMock();
    data.WriteRemoteObject(object);
    EXPECT_CALL(*mockAppMgrService_, RegisterRenderStateObserver(_)).Times(1);
    auto res = mockAppMgrService_->HandleRegisterRenderStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUnregisterRenderStateObserver_0100
 * @tc.desc: Test unregister observer success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUnregisterRenderStateObserver_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<IRemoteObject> object = new (std::nothrow) RenderStateObserverMock();
    data.WriteRemoteObject(object);
    EXPECT_CALL(*mockAppMgrService_, UnregisterRenderStateObserver(_)).Times(1);
    auto res = mockAppMgrService_->HandleUnregisterRenderStateObserver(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleUpdateRenderState_0100
 * @tc.desc: Test update render state success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleUpdateRenderState_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    pid_t renderPid = 0;
    data.WriteInt32(renderPid);
    int32_t state = 0;
    data.WriteInt32(state);
    EXPECT_CALL(*mockAppMgrService_, UpdateRenderState(_, _)).Times(1);
    auto res = mockAppMgrService_->HandleUpdateRenderState(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: HandleSignRestartAppFlag_0100
 * @tc.desc: Test sign restart app flag success.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrStubTest, HandleSignRestartAppFlag_0100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);
    auto res = mockAppMgrService_->HandleSignRestartAppFlag(data, reply);
    EXPECT_EQ(res, NO_ERROR);
}
} // namespace AppExecFwk
} // namespace OHOS
