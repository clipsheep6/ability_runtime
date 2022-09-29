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
#include "quick_fix_errno_def.h"
#include "quick_fix_manager_stub.h"
#include "mock_quick_fix_manager_stub.h"
#undef private
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AAFwk {
class QuickFixManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixManagerStub> mockQuickFixMgrService_ = nullptr;
};

void QuickFixManagerStubTest::SetUpTestCase(void)
{}

void QuickFixManagerStubTest::TearDownTestCase(void)
{}

void QuickFixManagerStubTest::SetUp()
{
    mockQuickFixMgrService_ = new MockQuickFixManagerStub();
    ASSERT_NE(mockQuickFixMgrService_, nullptr);
}

void QuickFixManagerStubTest::TearDown()
{}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerStubTest, ApplyQuickFix_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixManagerStub::GetDescriptor());
    std::vector<std::string> quickFixFiles;
    quickFixFiles.push_back("/data/storage/el2/base/entry1.hqf");
    quickFixFiles.push_back("/data/storage/el2/base/entry2.hqf");
    data.WriteStringVector(quickFixFiles);

    EXPECT_CALL(*mockQuickFixMgrService_, ApplyQuickFix(_)).Times(1);

    auto result = mockQuickFixMgrService_->OnRemoteRequest(
        IQuickFixManager::QuickFixMgrCmd::ON_APPLY_QUICK_FIX, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: GetApplyedQuickFixInfo
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerStubTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixManagerStub::GetDescriptor());
    std::string bundleName = "com.ohos.quickfix";
    data.WriteString(bundleName);

    EXPECT_CALL(*mockQuickFixMgrService_, GetApplyedQuickFixInfo(_, _)).Times(1);

    auto result = mockQuickFixMgrService_->OnRemoteRequest(
        IQuickFixManager::QuickFixMgrCmd::ON_GET_APPLYED_QUICK_FIX_INFO, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnRemoteRequest_0100
 * @tc.desc: OnRemoteRequest
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"fake_interface_token");
    std::string bundleName = "com.ohos.quickfix";
    data.WriteString(bundleName);

    auto result = mockQuickFixMgrService_->OnRemoteRequest(
        IQuickFixManager::QuickFixMgrCmd::ON_GET_APPLYED_QUICK_FIX_INFO, data, reply, option);
    EXPECT_EQ(result, QUICK_FIX_INVALID_PARAM);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnRemoteRequest_0200
 * @tc.desc: OnRemoteRequest
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerStubTest, OnRemoteRequest_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixManagerStub::GetDescriptor());
    std::string bundleName = "com.ohos.quickfix";
    data.WriteString(bundleName);

    uint32_t invalidCode = 10;
    auto result = mockQuickFixMgrService_->OnRemoteRequest(invalidCode, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnRemoteRequest_0300
 * @tc.desc: OnRemoteRequest
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerStubTest, OnRemoteRequest_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(QuickFixManagerStub::GetDescriptor());
    std::string bundleName = "com.ohos.quickfix";
    data.WriteString(bundleName);

    uint32_t invalidCode = 10;
    mockQuickFixMgrService_->requestFuncMap_[IQuickFixManager::ON_GET_APPLYED_QUICK_FIX_INFO] = nullptr;
    auto result = mockQuickFixMgrService_->OnRemoteRequest(invalidCode, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyQuickFixInner_0100
 * @tc.desc: ApplyQuickFixInner
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerStubTest, ApplyQuickFixInner_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    MessageParcel data;
    data.WriteInt32(-1);
    MessageParcel reply;

    auto result = mockQuickFixMgrService_->ApplyQuickFixInner(data, reply);
    EXPECT_EQ(result, QUICK_FIX_READ_PARCEL_FAILED);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfoInner_0100
 * @tc.desc: GetApplyedQuickFixInfoInner
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerStubTest, GetApplyedQuickFixInfoInner_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    MessageParcel data;
    MessageParcel reply;
    reply.allocator_ = nullptr;
    ApplicationQuickFixInfo expectQuickFixInfo;
    expectQuickFixInfo.asRemote_ = true;
    EXPECT_CALL(*mockQuickFixMgrService_, GetApplyedQuickFixInfo(_,_))
        .WillOnce(DoAll(SetArgReferee<1>(expectQuickFixInfo), Return(QUICK_FIX_OK)));
    auto result = mockQuickFixMgrService_->GetApplyedQuickFixInfoInner(data, reply);
    EXPECT_EQ(result, QUICK_FIX_WRITE_PARCEL_FAILED);
    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS