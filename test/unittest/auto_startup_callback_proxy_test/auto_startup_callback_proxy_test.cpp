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

#define private public
#include "auto_startup_callback_proxy.h"
#undef private

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

class AutoStartupCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AutoStartupCallbackProxyTest::SetUpTestCase() {}

void AutoStartupCallbackProxyTest::TearDownTestCase() {}

void AutoStartupCallbackProxyTest::SetUp() {}

void AutoStartupCallbackProxyTest::TearDown() {}

/*
 * Feature: AutoStartupCallbackProxyTest
 * Function: OnAutoStartupOn
 * SubFunction: NA
 * FunctionPoints: AutoStartupCallbackProxyTest OnAutoStartupOn
 */
HWTEST_F(AutoStartupCallbackProxyTest, OnAutoStartupOn_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoStartupCallBackProxy OnAutoStartupOn_001 start";
    sptr<IRemoteObject> impl;
    auto autoStartupCallBackProxy = std::make_shared<AutoStartupCallBackProxy>(impl);
    EXPECT_NE(autoStartupCallBackProxy, nullptr);
    AutoStartupInfo info;
    autoStartupCallBackProxy->OnAutoStartupOn(info);
    GTEST_LOG_(INFO) << "AutoStartupCallBackProxy OnAutoStartupOn_001 end";
}

/*
 * Feature: AutoStartupCallbackProxyTest
 * Function: OnAutoStartupOff
 * SubFunction: NA
 * FunctionPoints: AutoStartupCallbackProxyTest OnAutoStartupOff
 */
HWTEST_F(AutoStartupCallbackProxyTest, OnAutoStartupOff_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoStartupCallBackProxy OnAutoStartupOff_001 start";
    sptr<IRemoteObject> impl;
    auto autoStartupCallBackProxy = std::make_shared<AutoStartupCallBackProxy>(impl);
    EXPECT_NE(autoStartupCallBackProxy, nullptr);
    AutoStartupInfo info;
    autoStartupCallBackProxy->OnAutoStartupOff(info);
    GTEST_LOG_(INFO) << "AutoStartupCallBackProxy OnAutoStartupOff_001 end";
}

/*
 * Feature: AutoStartupCallbackProxyTest
 * Function: SendRequest
 * SubFunction: NA
 * FunctionPoints: AutoStartupCallbackProxyTest SendRequest
 */
HWTEST_F(AutoStartupCallbackProxyTest, SendRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AutoStartupCallBackProxy SendRequest_001 start";
    sptr<IRemoteObject> impl;
    auto autoStartupCallBackProxy = std::make_shared<AutoStartupCallBackProxy>(impl);
    AbilityManagerInterfaceCode code = AbilityManagerInterfaceCode::ON_AUTO_STARTUP_OFF;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto result = autoStartupCallBackProxy->SendRequest(code, data, reply, option);
    EXPECT_EQ(result, INNER_ERR);
    GTEST_LOG_(INFO) << "AutoStartupCallBackProxy SendRequest_001 end";
}
} // namespace AAFwk
} // namespace OHOS
