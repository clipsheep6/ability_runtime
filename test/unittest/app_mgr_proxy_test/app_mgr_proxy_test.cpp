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

#include "mock_app_mgr_service.h"
#include "app_mgr_proxy.h"
#include "hilog_wrapper.h"
#include "quick_fix_callback_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
}  // namespace

class QuickFixCallbackImpl : public AppExecFwk::QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnUnloadPatchDone(int32_t resultCode) override
    {
        HILOG_DEBUG("function called.");
    }

    void OnReloadPageDone(int32_t resultCode) override
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
        mockAppMgrService_->code_, static_cast<uint32_t>(IAppMgr::Message::APP_GET_RUNNING_PROCESSES_BY_USER_ID));

    GTEST_LOG_(INFO) << "AppMgrProxy_GetProcessRunningInfosByUserId_0100 end";
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

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(IAppMgr::Message::GET_APP_RUNNING_STATE));

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

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(IAppMgr::Message::NOTIFY_LOAD_REPAIR_PATCH));

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

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(IAppMgr::Message::NOTIFY_HOT_RELOAD_PAGE));

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

    EXPECT_EQ(mockAppMgrService_->code_, static_cast<uint32_t>(IAppMgr::Message::NOTIFY_UNLOAD_REPAIR_PATCH));

    HILOG_INFO("%{public}s end.", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
