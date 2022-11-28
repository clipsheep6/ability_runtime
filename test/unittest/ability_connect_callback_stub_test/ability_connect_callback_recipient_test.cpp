/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "iremote_proxy.h"
#define private public
#define protected public
#include "mock_ability_connect_callback_stub.h"
#include "ability_connect_callback_proxy.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS;
using namespace testing;

class AbilityConnectCallbackRecipientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AbilityConnectCallbackRecipientTest::SetUpTestCase(void)
{}
void AbilityConnectCallbackRecipientTest::TearDownTestCase(void)
{}
void AbilityConnectCallbackRecipientTest::SetUp()
{}
void AbilityConnectCallbackRecipientTest::TearDown()
{}

/**
 * @tc.name: AbilityConnectCallbackRecipientTest_001
 * @tc.desc: OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(AbilityConnectCallbackRecipientTest, AbilityConnectCallbackRecipientTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityConnectCallbackRecipientTest_001 start";
    sptr<IRemoteObject> remoteObject;
    wptr<IRemoteObject> remote(remoteObject);
    OHOS::AAFwk::AbilityConnectCallbackRecipient::RemoteDiedHandler handler;
    std::shared_ptr<AbilityConnectCallbackRecipient> recipient =
        std::make_shared<AbilityConnectCallbackRecipient>(handler);
    recipient->OnRemoteDied(remote);

    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "AbilityConnectCallbackRecipientTest_001 end";
}
