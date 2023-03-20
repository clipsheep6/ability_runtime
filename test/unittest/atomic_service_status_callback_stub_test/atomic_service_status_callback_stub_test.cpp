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
#define protected public
#include "mock_atomic_service_status_callback_stub.h"
#include "atomic_service_status_callback_interface.h"
#undef protected
#include "message_parcel.h"
#include "string_ex.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.IAtomicServiceStatusCallback";
}

class AtomicServiceStatusCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AtomicServiceStatusCallbackStubTest::SetUpTestCase(void)
{}
void AtomicServiceStatusCallbackStubTest::TearDownTestCase(void)
{}
void AtomicServiceStatusCallbackStubTest::SetUp()
{}
void AtomicServiceStatusCallbackStubTest::TearDown()
{}

/*
 * Feature: AtomicServiceStatusCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AtomicServiceStatusCallbackStub OnRemoteRequest
 * EnvConditions: want is nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AtomicServiceStatusCallbackStubTest, AtomicServiceStatusCallbackStub_OnInstallFinishedInner_001, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());

    int resultCode = 0;
    int32_t userId = 0;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteInt32(resultCode);
    data.WriteParcelable(nullptr);
    data.WriteInt32(userId);
    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnInstallFinished(_, _, _)).Times(0);
    int res = mockAtomicServiceStatusCallbackStub->OnRemoteRequest(
        IAtomicServiceStatusCallback::IAtomicServiceStatusCallbackCmd::ON_FREE_INSTALL_DONE, data, reply, option);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AtomicServiceStatusCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AtomicServiceStatusCallbackStub OnRemoteRequest
 * EnvConditions: want is not nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AtomicServiceStatusCallbackStubTest, AtomicServiceStatusCallbackStub_OnInstallFinishedInner_002, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());

    int resultCode = 0;
    Want want;
    int32_t userId = 0;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteInt32(resultCode);
    data.WriteParcelable(&want);
    data.WriteInt32(userId);
    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnInstallFinished(_, _, _)).Times(1);
    int res = mockAtomicServiceStatusCallbackStub->OnRemoteRequest(
        IAtomicServiceStatusCallback::IAtomicServiceStatusCallbackCmd::ON_FREE_INSTALL_DONE, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: AtomicServiceStatusCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AtomicServiceStatusCallbackStub OnRemoteRequest
 * EnvConditions: want is nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AtomicServiceStatusCallbackStubTest, AtomicServiceStatusCallbackStub_OnRemoteInstallFinishedInner_001, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());

    int resultCode = 0;
    Want want;
    int32_t userId = 0;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteInt32(resultCode);
    data.WriteParcelable(nullptr);
    data.WriteInt32(userId);
    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnRemoteInstallFinished(_, _, _)).Times(0);
    int res = mockAtomicServiceStatusCallbackStub->OnRemoteRequest(
        IAtomicServiceStatusCallback::IAtomicServiceStatusCallbackCmd::ON_REMOTE_FREE_INSTALL_DONE, data, reply, option);
    EXPECT_NE(res, NO_ERROR);
}

/*
 * Feature: AtomicServiceStatusCallbackStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AtomicServiceStatusCallbackStub OnRemoteRequest
 * EnvConditions: want is not nullptr
 * CaseDescription: Verify that on remote request is normal and abnormal
 */
HWTEST_F(AtomicServiceStatusCallbackStubTest, AtomicServiceStatusCallbackStub_OnRemoteInstallFinishedInner_002, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());

    int resultCode = 0;
    Want want;
    int32_t userId = 0;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteInt32(resultCode);
    data.WriteParcelable(&want);
    data.WriteInt32(userId);
    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnRemoteInstallFinished(_, _, _)).Times(1);
    int res = mockAtomicServiceStatusCallbackStub->OnRemoteRequest(
        IAtomicServiceStatusCallback::IAtomicServiceStatusCallbackCmd::ON_REMOTE_FREE_INSTALL_DONE, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: AtomicServiceStatusCallbackStub_OnRemoveTimeoutTaskInner_001
 * @tc.desc: OnRemoveTimeoutTaskInner
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(AtomicServiceStatusCallbackStubTest, AtomicServiceStatusCallbackStub_OnRemoveTimeoutTaskInner_001, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());
    
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteParcelable(nullptr);
    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnRemoveTimeoutTask(_)).Times(0);
    int res = mockAtomicServiceStatusCallbackStub->OnRemoteRequest(
        IAtomicServiceStatusCallback::IAtomicServiceStatusCallbackCmd::ON_REMOVE_TIMEOUT_TASK, data, reply, option);
    EXPECT_NE(res, NO_ERROR);
}

/**
 * @tc.name: AtomicServiceStatusCallbackStub_OnRemoveTimeoutTaskInner_002
 * @tc.desc: OnRemoveTimeoutTaskInner
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(AtomicServiceStatusCallbackStubTest, AtomicServiceStatusCallbackStub_OnRemoveTimeoutTaskInner_002, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());
    
    Want want;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);
    data.WriteParcelable(&want);
    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnRemoveTimeoutTask(_)).Times(1);
    int res = mockAtomicServiceStatusCallbackStub->OnRemoteRequest(
        IAtomicServiceStatusCallback::IAtomicServiceStatusCallbackCmd::ON_REMOVE_TIMEOUT_TASK, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}
}  // namespace AAFwk
}  // namespace OHOS
