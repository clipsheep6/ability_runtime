/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_ability_connect_callback_object.h"
#include "mock_ability_connect_callback_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
class CjAbilityConnectCallbackProxyTest : public testing::Test {
public:
    CjAbilityConnectCallbackProxyTest()
    {}
    ~CjAbilityConnectCallbackProxyTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<CJAbilityConnectCallback> proxy_{nullptr};
};

int g_cjRet = 0;
void (*g_registerFunc)(CJAbilityConnectCallbackFuncs *result) = [](CJAbilityConnectCallbackFuncs *result) {
    if (result != nullptr) {
        g_cjRet += 1;
    }
    result->onConnect =
        [](int64_t id, ElementNameHandle elementNameHandle, int64_t remoteObjectId, int32_t resultCode) {};
    result->onDisconnect = [](int64_t id, ElementNameHandle elementNameHandle, int32_t resultCode) {};
    result->release = [](int64_t id) { id++; };
};

void CjAbilityConnectCallbackProxyTest::SetUpTestCase()
{}

void CjAbilityConnectCallbackProxyTest::TearDownTestCase()
{}

void CjAbilityConnectCallbackProxyTest::SetUp()
{}

void CjAbilityConnectCallbackProxyTest::TearDown()
{}

HWTEST_F(CjAbilityConnectCallbackProxyTest, OnAbilityConnectDone_0100, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());
    sptr<CJAbilityConnectCallback> callback(new CJAbilityConnectCallback(0));
    AppExecFwk::ElementName element;
    EXPECT_CALL(*mockAbilityConnectStub, OnAbilityConnectDone(_, _, _)).Times(0);
    callback->OnAbilityConnectDone(element, mockAbilityConnectStub, 0);
    mockAbilityConnectStub->Wait();
}

HWTEST_F(CjAbilityConnectCallbackProxyTest, OnAbilityDisconnectDone_0100, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> mockAbilityConnectStub(new MockAbilityConnectCallback());
    sptr<CJAbilityConnectCallback> callback(new CJAbilityConnectCallback(0));
    AppExecFwk::ElementName element;
    EXPECT_CALL(*mockAbilityConnectStub, OnAbilityDisconnectDone(_, _)).Times(0);
    callback->OnAbilityDisconnectDone(element, 0);
    mockAbilityConnectStub->Wait();
}

HWTEST_F(CjAbilityConnectCallbackProxyTest, RegisterCJAbilityConnectCallbackFuncs_0100, TestSize.Level1)
{
    RegisterCJAbilityConnectCallbackFuncs(g_registerFunc);
    EXPECT_EQ(g_cjRet, 1);

    RegisterCJAbilityConnectCallbackFuncs(g_registerFunc);
    EXPECT_EQ(g_cjRet, 1);

    sptr<IRemoteObject> remoteObject = nullptr;
    sptr<CJAbilityConnectCallback> callback(new CJAbilityConnectCallback(0));
    AppExecFwk::ElementName element;

    callback->OnAbilityConnectDone(element, remoteObject, 0);
    callback->OnAbilityDisconnectDone(element, 0);
}

}  // namespace AbilityRuntime
}  // namespace OHOS