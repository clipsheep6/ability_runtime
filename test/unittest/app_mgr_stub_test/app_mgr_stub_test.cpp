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

#include "app_mgr_stub.h"
#include "hilog_wrapper.h"
#include "mock_app_mgr_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t USER_ID = 100;
}  // namespace

class AppMgrStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppMgrService> mockAppMgrService_;

    void WriteInterfaceToken(MessageParcel &data);
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

void AppMgrStubTest::WriteInterfaceToken(MessageParcel &data)
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
        static_cast<uint32_t>(IAppMgr::Message::APP_GET_RUNNING_PROCESSES_BY_USER_ID), data, reply, option);
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
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, GetAppRunningStateByBundleName(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(IAppMgr::Message::GET_APP_RUNNING_STATE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyLoadRepairPatch_0100
 * @tc.desc: Handle notify load repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyLoadRepairPatch_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, NotifyLoadRepairPatch(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(IAppMgr::Message::NOTIFY_LOAD_REPAIR_PATCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyHotReloadPage_0100
 * @tc.desc: Handle notify ace hot reload page.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyHotReloadPage_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, NotifyHotReloadPage(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(IAppMgr::Message::NOTIFY_HOT_RELOAD_PAGE), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandleNotifyUnLoadRepairPatch_0100
 * @tc.desc: Handle notify unload repair patch.
 * @tc.type: FUNC
 * @tc.require: issueI581VW
 */
HWTEST_F(AppMgrStubTest, HandleNotifyUnLoadRepairPatch_0100, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    std::string bundleName = "testBundleName";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockAppMgrService_, NotifyUnLoadRepairPatch(_)).Times(1);

    auto result = mockAppMgrService_->OnRemoteRequest(
        static_cast<uint32_t>(IAppMgr::Message::NOTIFY_UNLOAD_REPAIR_PATCH), data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}
}  // namespace AppExecFwk
}  // namespace OHOS
