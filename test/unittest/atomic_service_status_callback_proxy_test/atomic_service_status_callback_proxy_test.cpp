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
#include "atomic_service_status_callback_proxy.h"
#include "atomic_service_status_callback.h"
#undef protected
#include "message_parcel.h"
#include "string_ex.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class AtomicServiceStatusCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AtomicServiceStatusCallbackProxy> proxy_{ nullptr };
};

void AtomicServiceStatusCallbackProxyTest::SetUpTestCase(void)
{}
void AtomicServiceStatusCallbackProxyTest::TearDownTestCase(void)
{}
void AtomicServiceStatusCallbackProxyTest::SetUp()
{}
void AtomicServiceStatusCallbackProxyTest::TearDown()
{}

/*
 * Feature: AAFwk
 * Function: AtomicServiceStatusCallbackProxy
 * SubFunction: IPC of client and server
 * FunctionPoints: OnInstallFinished
 * EnvConditions: NA
 * CaseDescription: verify OnInstallFinished IPC between client and server.
 */
HWTEST_F(AtomicServiceStatusCallbackProxyTest, AtomicServiceStatusCallbackProxy_IPC_001, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());
    sptr<AtomicServiceStatusCallbackProxy> callback(new AtomicServiceStatusCallbackProxy(mockAtomicServiceStatusCallbackStub));
    int resultCode = 0;
    Want want;
    AppExecFwk::ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    int32_t userId = 0;

    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnInstallFinished(_, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockAtomicServiceStatusCallbackStub.GetRefPtr(), &MockAtomicServiceStatusCallbackStub::PostVoid));
    callback->OnInstallFinished(resultCode, want, userId);
    mockAtomicServiceStatusCallbackStub->Wait();
}

/*
 * Feature: AAFwk
 * Function: AtomicServiceStatusCallbackProxy
 * SubFunction: IPC of client and server
 * FunctionPoints: OnRemoteInstallFinished
 * EnvConditions: NA
 * CaseDescription: verify OnRemoteInstallFinished IPC between client and server.
 */
HWTEST_F(AtomicServiceStatusCallbackProxyTest, AtomicServiceStatusCallbackProxy_IPC_002, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());
    sptr<AtomicServiceStatusCallbackProxy> callback(new AtomicServiceStatusCallbackProxy(mockAtomicServiceStatusCallbackStub));
    int resultCode = 0;
    Want want;
    AppExecFwk::ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    int32_t userId = 0;

    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnRemoteInstallFinished(_, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockAtomicServiceStatusCallbackStub.GetRefPtr(), &MockAtomicServiceStatusCallbackStub::PostVoid));
    callback->OnRemoteInstallFinished(resultCode, want, userId);
    mockAtomicServiceStatusCallbackStub->Wait();
}

/*
 * Feature: AAFwk
 * Function: AtomicServiceStatusCallbackProxy
 * SubFunction: IPC of client and server
 * FunctionPoints: instance
 * EnvConditions: NA
 * CaseDescription: instance is success.
 */
HWTEST_F(AtomicServiceStatusCallbackProxyTest, AtomicServiceStatusCallbackProxy_IPC_003, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());
    sptr<AtomicServiceStatusCallbackProxy> callback(new AtomicServiceStatusCallbackProxy(mockAtomicServiceStatusCallbackStub));
    EXPECT_NE(callback, nullptr);
}

/**
 * @tc.name: AtomicServiceStatusCallbackProxy_IPC_003
 * @tc.desc: OnRemoveTimeoutTask
 * @tc.type: FUNC
 * @tc.require: issueI6F3F6
 */
HWTEST_F(AtomicServiceStatusCallbackProxyTest, AtomicServiceStatusCallbackProxy_IPC_004, TestSize.Level1)
{
    sptr<MockAtomicServiceStatusCallbackStub> mockAtomicServiceStatusCallbackStub(
        new MockAtomicServiceStatusCallbackStub());
    sptr<AtomicServiceStatusCallbackProxy> callback(new AtomicServiceStatusCallbackProxy(mockAtomicServiceStatusCallbackStub));
    Want want;
    AppExecFwk::ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);

    EXPECT_CALL(*mockAtomicServiceStatusCallbackStub, OnRemoveTimeoutTask(_))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockAtomicServiceStatusCallbackStub.GetRefPtr(), &MockAtomicServiceStatusCallbackStub::PostVoid));
    callback->OnRemoveTimeoutTask(want);
    mockAtomicServiceStatusCallbackStub->Wait();
}
}  // namespace AAFwk
}  // namespace OHOS