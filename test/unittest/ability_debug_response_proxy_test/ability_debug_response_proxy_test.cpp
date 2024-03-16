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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ability_debug_response_proxy.h"
#include "mock_ability_debug_response_stub.h"
#include "mock_ability_token.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class AbilityDebugResponseProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void WriteInterfaceToken(MessageParcel& data);
};

void AbilityDebugResponseProxyTest::SetUpTestCase(void)
{}

void AbilityDebugResponseProxyTest::TearDownTestCase(void)
{}

void AbilityDebugResponseProxyTest::SetUp()
{}

void AbilityDebugResponseProxyTest::TearDown()
{}

void AbilityDebugResponseProxyTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(GetDescriptor());
}

/**
 * @tc.name: AbilityDebugResponseProxyTest_OnAbilitysDebugStarted_0100
 * @tc.desc: Verify the OnAbilitysDebugStarted calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugResponseProxyTest, OnAbilitysDebugStarted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilitysDebugStarted_0100 start";
    auto mockStub = new (std::nothrow) MockAbilityDebugResponseStub();
    auto proxy = new AbilityDebugResponseProxy(mockStub);
    EXPECT_TRUE(proxy);

    std::vector<sptr<IRemoteObject>> tokens;
    sptr<MockAbilityToken> token = new (std::nothrow) MockAbilityToken();
    EXPECT_TRUE(token);
    tokens.push_back(token);
    EXPECT_CALL(*mockStub, OnAbilitysDebugStarted(_)).Times(1);
    proxy->OnAbilitysDebugStarted(tokens);
    
    GTEST_LOG_(INFO) << "OnAbilitysDebugStarted_0100 end";
}

/**
 * @tc.name: AbilityDebugResponseProxyTest_OnAbilitysDebugStoped_0100
 * @tc.desc: Verify the OnAbilitysDebugStoped calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugResponseProxyTest, OnAbilitysDebugStoped_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAbilitysDebugStoped_0100 start";
    auto mockStub = new (std::nothrow) MockAbilityDebugResponseStub();
    auto proxy = new (std::nothrow) AbilityDebugResponseProxy(mockStub);
    EXPECT_TRUE(proxy);

    std::vector<sptr<IRemoteObject>> tokens;
    sptr<MockAbilityToken> token = new (std::nothrow) MockAbilityToken();
    EXPECT_TRUE(token);
    tokens.push_back(token);

    EXPECT_CALL(*mockStub, OnAbilitysDebugStoped(_)).Times(1);
    proxy->OnAbilitysDebugStarted(tokens);
    GTEST_LOG_(INFO) << "OnAbilitysDebugStoped_0100 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS