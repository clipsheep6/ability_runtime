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
#include "mock_free_install_observer_stub.h"
#include "free_install_observer_proxy.h"
#undef pr
#include "message_parcel.h"
#include "string_ex.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AbilityRuntime {
class FreeInstallObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<FreeInstallObserverProxy> proxy_{ nullptr };
};

void FreeInstallObserverProxyTest::SetUpTestCase(void)
{}
void FreeInstallObserverProxyTest::TearDownTestCase(void)
{}
void FreeInstallObserverProxyTest::SetUp()
{}
void FreeInstallObserverProxyTest::TearDown()
{}

/*
 * Feature: AAFwk
 * Function: AtomicServiceStatusCallbackProxy
 * SubFunction: IPC of client and server
 * FunctionPoints: OnInstallFinished
 * EnvConditions: NA
 * CaseDescription: verify OnInstallFinished IPC between client and server.
 */
HWTEST_F(FreeInstallObserverProxyTest, FreeInstallObserverProxy_IPC_001, TestSize.Level1)
{
    sptr<MockFreeInstallObserverStub> mockFreeInstallObserverStub(
        new MockFreeInstallObserverStub());
    sptr<FreeInstallObserverProxy> callback(new FreeInstallObserverProxy(mockFreeInstallObserverStub));
    std::string bundleName = "";
    std::string abilityName = "";
    std::string startTime = "";
    int resultCode = 0;

    EXPECT_CALL(*mockFreeInstallObserverStub, OnInstallFinished(_, _, _, _))
        .Times(1)
        .WillOnce(InvokeWithoutArgs(mockFreeInstallObserverStub.GetRefPtr(), &MockFreeInstallObserverStub::PostVoid));
    callback->OnInstallFinished(bundleName, abilityName, startTime, resultCode);
    mockFreeInstallObserverStub->Wait();
}
}  // namespace AAFwk
}  // namespace OHOS