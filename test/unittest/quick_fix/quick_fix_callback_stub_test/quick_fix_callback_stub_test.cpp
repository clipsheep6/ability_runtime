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
#include "quick_fix_callback_stub.h"
#include "mock_quick_fix_callback_stub.h"
#undef private
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixCallbackStub> mockQuickFixCallback_ = nullptr;
};

void QuickFixCallbackStubTest::SetUpTestCase(void)
{}

void QuickFixCallbackStubTest::TearDownTestCase(void)
{}

void QuickFixCallbackStubTest::SetUp()
{
    mockQuickFixCallback_ = new MockQuickFixCallbackStub();
    ASSERT_NE(mockQuickFixCallback_, nullptr);
}

void QuickFixCallbackStubTest::TearDown()
{}

/**
 * @tc.name: OnLoadPatchDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackStubTest, OnLoadPatchDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixCallbackStub::GetDescriptor());
    int32_t resultCode = 0;
    data.WriteInt32(resultCode);

    EXPECT_CALL(*mockQuickFixCallback_, OnLoadPatchDone(_)).Times(1);

    auto result = mockQuickFixCallback_->OnRemoteRequest(
        IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_LOAD_PATCH, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnUnloadPatchDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackStubTest, OnUnloadPatchDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixCallbackStub::GetDescriptor());
    int32_t resultCode = 0;
    data.WriteInt32(resultCode);

    EXPECT_CALL(*mockQuickFixCallback_, OnUnloadPatchDone(_)).Times(1);

    auto result = mockQuickFixCallback_->OnRemoteRequest(
        IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_UNLOAD_PATCH, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnReloadPageDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackStubTest, OnReloadPageDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixCallbackStub::GetDescriptor());
    int32_t resultCode = 0;
    data.WriteInt32(resultCode);

    EXPECT_CALL(*mockQuickFixCallback_, OnReloadPageDone(_)).Times(1);

    auto result = mockQuickFixCallback_->OnRemoteRequest(
        IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_RELOAD_PAGE, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnRemoteRequest_0100
 * @tc.desc: OnRemoteRequest
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"fake_interface_token");
    int32_t resultCode = 0;
    data.WriteInt32(resultCode);

    auto result = mockQuickFixCallback_->OnRemoteRequest(
        IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_LOAD_PATCH, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnRemoteRequest_0200
 * @tc.desc: OnRemoteRequest
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixCallbackStub::GetDescriptor());
    std::string bundleName = "com.ohos.quickfix";
    data.WriteString(bundleName);

    uint32_t invalidCode = 10;
    auto result = mockQuickFixCallback_->OnRemoteRequest(invalidCode, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS