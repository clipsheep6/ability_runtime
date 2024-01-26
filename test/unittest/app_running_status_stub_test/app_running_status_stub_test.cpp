/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#define private public
#include "app_running_status_stub.h"
#include "app_running_status_listener_interface.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace {
    const std::string BUNDLE_NAME = "bundleName";
    constexpr uint32_t UNKNOWN_CODE = 3;
    constexpr int32_t UID = 1;
}
namespace OHOS {
namespace AbilityRuntime {

class MockAppRunningStatusStub : public AppRunningStatusStub {
public:
    MOCK_METHOD3(NotifyAppRunningStatus,
        void(const std::string &bundle, int32_t uid, RunningStatus runningStatus));
};

class AppRunningStatusStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<MockAppRunningStatusStub> stub_;
};

void AppRunningStatusStubTest::SetUpTestCase()
{}

void AppRunningStatusStubTest::TearDownTestCase()
{}

void AppRunningStatusStubTest::SetUp()
{
    stub_ = new MockAppRunningStatusStub();
}

void AppRunningStatusStubTest::TearDown()
{}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: handle AppRunningStatus request
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusStubTest, OnRemoteRequest_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(stub_->GetDescriptor());

    EXPECT_CALL(*stub_, NotifyAppRunningStatus(_, _, _)).Times(1);
    auto runningCode = static_cast<uint32_t>(AppRunningStatusListenerInterface::MessageCode::APP_RUNNING_STATUS);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = stub_->OnRemoteRequest(runningCode, data, reply, option);
    EXPECT_EQ(ret, NO_ERROR);
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: check null interface descriptor
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusStubTest, OnRemoteRequest_002, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    EXPECT_CALL(*stub_, NotifyAppRunningStatus(_, _, _)).Times(0);
    auto runningCode = static_cast<uint32_t>(AppRunningStatusListenerInterface::MessageCode::APP_RUNNING_STATUS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto ret = stub_->OnRemoteRequest(runningCode, data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
}

/**
 * @tc.name: OnRemoteRequest_003
 * @tc.desc: Check unknown request code
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusStubTest, OnRemoteRequest_003, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);
    MessageParcel data;
    data.WriteInterfaceToken(stub_->GetDescriptor());
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    EXPECT_CALL(*stub_, NotifyAppRunningStatus(_, _, _)).Times(0);
    auto ret = stub_->OnRemoteRequest(UNKNOWN_CODE, data, reply, option);
    EXPECT_EQ(ret, IPC_STUB_UNKNOW_TRANS_ERR);
}

/**
 * @tc.name: HandleAppRunningStatus_001
 * @tc.desc: handle AppRunningStatus
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusStubTest, HandleAppRunningStatus_001, TestSize.Level1)
{
    EXPECT_NE(stub_, nullptr);

    MessageParcel data;
    data.WriteString(BUNDLE_NAME);
    data.WriteInt32(UID);
    data.WriteInt32(static_cast<int32_t>(RunningStatus::APP_RUNNING_START));

    MessageParcel reply;
    EXPECT_CALL(*stub_, NotifyAppRunningStatus(_, _, _)).Times(1);
    auto ret = stub_->HandleAppRunningStatus(data, reply);
    EXPECT_EQ(ret, NO_ERROR);
}
} // namespace AbilityRuntime
} // namespace OHOS
