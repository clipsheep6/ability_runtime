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

#include "app_mgr_proxy.h"
#include "hilog_wrapper.h"
#include "mock_app_mgr_service.h"
#include "quick_fix_callback_stub.h"
#include "app_foreground_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
} // namespace

class AppForegroundStateObserverMock : public AppForegroundStateObserverStub {
public:
    AppForegroundStateObserverMock() = default;
    virtual ~AppForegroundStateObserverMock() = default;

    void OnAppStateChanged(const AppStateData &appStateData) override
    {}
};

class QuickFixCallbackImpl : public AppExecFwk::QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode, int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnUnloadPatchDone(int32_t resultCode, int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnReloadPageDone(int32_t resultCode, int32_t recordId) override
    {
        HILOG_DEBUG("function called.");
    }
};

class AppMgrProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppMgrService> mockAppMgrService_;
    sptr<AppMgrProxy> appMgrProxy_;
};

void AppMgrProxyTest::SetUpTestCase(void)
{}

void AppMgrProxyTest::TearDownTestCase(void)
{}

void AppMgrProxyTest::SetUp()
{
    GTEST_LOG_(INFO) << "AppMgrProxyTest::SetUp()";

    mockAppMgrService_ = new MockAppMgrService();
    appMgrProxy_ = new AppMgrProxy(mockAppMgrService_);
}

void AppMgrProxyTest::TearDown()
{}

/**
 * @tc.name: AppMgrProxy_GetProcessRunningInfosByUserId_0100
 * @tc.desc: GetProcessRunningInfosByUserId
 * @tc.type: FUNC
 * @tc.require: SR000GH1GO
 */
HWTEST_F(AppMgrProxyTest, AppMgrProxy_GetProcessRunningInfosByUserId_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AppMgrProxy_GetProcessRunningInfosByUserId_0100 start";

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::vector<RunningProcessInfo> info;
    appMgrProxy_->GetProcessRunningInfosByUserId(info, USER_ID);

    EXPECT_EQ(
        mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID));

    GTEST_LOG_(INFO) << "AppMgrProxy_GetProcessRunningInfosByUserId_0100 end";
}

/**
 * @tc.name: AppMgrProxy_GetAllRenderProcesses_0100
 * @tc.desc: GetAllRenderProcesses
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrProxyTest, AppMgrProxy_GetAllRenderProcesses_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AppMgrProxy_GetAllRenderProcesses_0100 start";

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::vector<RenderProcessInfo> info;
    appMgrProxy_->GetAllRenderProcesses(info);

    EXPECT_EQ(
        mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RENDER_PROCESSES));

    GTEST_LOG_(INFO) << "AppMgrProxy_GetAllRenderProcesses_0100 end";
}

/**
 * @tc.name: GetAppRunningStateByBundleName_0100
 * @tc.desc: Get app running state by bundle name.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrProxyTest, GetAppRunningStateByBundleName_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::string bundleName = "testBundleName";
    appMgrProxy_->GetAppRunningStateByBundleName(bundleName);

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_STATE));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyLoadRepairPatch_0100
 * @tc.desc: Notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrProxyTest, NotifyLoadRepairPatch_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::string bundleName = "testBundleName";
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    appMgrProxy_->NotifyLoadRepairPatch(bundleName, callback);

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_LOAD_REPAIR_PATCH));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyHotReloadPage_0100
 * @tc.desc: Notify ace execute hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrProxyTest, NotifyHotReloadPage_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::string bundleName = "testBundleName";
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    appMgrProxy_->NotifyHotReloadPage(bundleName, callback);

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_HOT_RELOAD_PAGE));

    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @tc.name: NotifyUnLoadRepairPatch_0100
 * @tc.desc: Notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrProxyTest, NotifyUnLoadRepairPatch_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::string bundleName = "testBundleName";
    sptr<IQuickFixCallback> callback = new QuickFixCallbackImpl();
    appMgrProxy_->NotifyUnLoadRepairPatch(bundleName, callback);

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_UNLOAD_REPAIR_PATCH));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrProxyTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    appMgrProxy_->PreStartNWebSpawnProcess();
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetProcessMemoryByPid_001
 * @tc.desc: Get memorySize by pid.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrProxyTest, GetProcessMemoryByPid_001, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    int32_t pid = 0;
    int32_t memorySize = 0;
    appMgrProxy_->GetProcessMemoryByPid(pid, memorySize);
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::GET_PROCESS_MEMORY_BY_PID));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetRunningProcessInformation_001
 * @tc.desc: Get application processes information list by bundleName.
 * @tc.type: FUNC
 * @tc.require: issueI76JBF
 */
HWTEST_F(AppMgrProxyTest, GetRunningProcessInformation_001, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::string bundleName = "testBundleName";
    int32_t userId = USER_ID;
    std::vector<RunningProcessInfo> info;
    appMgrProxy_->GetRunningProcessInformation(bundleName, userId, info);
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::GET_PIDS_BY_BUNDLENAME));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyAppFault_001
 * @tc.desc: Notify app fault.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppMgrProxyTest, NotifyAppFault_001, TestSize.Level1)
{
    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));
    FaultData faultData;
    appMgrProxy_->NotifyAppFault(faultData);
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT));
}

/**
 * @tc.name: NotifyAppFaultBySA_001
 * @tc.desc: Notify app fault by SA.
 * @tc.type: FUNC
 * @tc.require: issueI79RY8
 */
HWTEST_F(AppMgrProxyTest, NotifyAppFaultBySA_001, TestSize.Level1)
{
    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));
    AppFaultDataBySA faultData;
    appMgrProxy_->NotifyAppFaultBySA(faultData);
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA));
}

/**
 * @tc.name: ChangeAppGcState_001
 * @tc.desc: Change app Gc state.
 * @tc.type: FUNC
 * @tc.require: issuesI85VVU
 */
HWTEST_F(AppMgrProxyTest, ChangeAppGcState_001, TestSize.Level1)
{
    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));
    int32_t pid = 0;
    int32_t state = 0;
    appMgrProxy_->ChangeAppGcState(pid, state);
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::CHANGE_APP_GC_STATE));
}

/**
 * @tc.name: IsApplicationRunning_001
 * @tc.desc: Send request to query the running status of the application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrProxyTest, IsApplicationRunning_001, TestSize.Level1)
{
    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockAppMgrService_.GetRefPtr(), &MockAppMgrService::InvokeSendRequest));

    std::string bundleName = "testBundleName";
    bool isRunning = false;
    appMgrProxy_->IsApplicationRunning(bundleName, isRunning);
    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(AppMgrInterfaceCode::IS_APPLICATION_RUNNING));
}

/**
 * @tc.name: RegisterAppForegroundStateObserver_0100
 * @tc.desc: Test when all condition not met.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrProxyTest, RegisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _)).Times(1);        
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserverMock();
    auto res = appMgrProxy_->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: UnregisterAppForegroundStateObserver_0100
 * @tc.desc: Test when all condition not met.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrProxyTest, UnregisterAppForegroundStateObserver_0100, TestSize.Level1)
{
    EXPECT_CALL(*mockAppMgrService_, SendRequest(_, _, _, _)).Times(1);     
    sptr<IAppForegroundStateObserver> observer = new (std::nothrow) AppForegroundStateObserverMock();
    auto res = appMgrProxy_->RegisterAppForegroundStateObserver(observer);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AppExecFwk
}  // namespace OHOS
