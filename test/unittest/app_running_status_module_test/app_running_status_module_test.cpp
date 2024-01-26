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

#include <algorithm>
#include <gtest/gtest.h>
#include <map>

#include "cpp/mutex.h"
#include "mock_app_running_status_module_test.h"
#include "hilog_wrapper.h"

#define private public
#include "app_running_status_module.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
}
class AppRunningStatusModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
void AppRunningStatusModuleTest::SetUpTestCase(void)
{}

void AppRunningStatusModuleTest::TearDownTestCase(void)
{}

void AppRunningStatusModuleTest::SetUp()
{}

void AppRunningStatusModuleTest::TearDown()
{}

/**
 * @tc.name: AppRunningStatusModuleTest_RegisterListener_0100
 * @tc.desc: Test the state of RegisterListener
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModule_RegisterListener_0100, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    sptr<AppRunningStatusListenerInterface> listener;
    ErrCode result = appRunningClient->RegisterListener(listener);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: AppRunningStatusModuleTest_RegisterListener_0200
 * @tc.desc: Test the state of RegisterListener
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModuleTest_RegisterListener_0200, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    sptr<AppRunningStatusListenerInterface> listener = new (std::nothrow) MockAppRunningStatusListenerInterface();
    ErrCode result = appRunningClient->RegisterListener(listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppRunningStatusModuleTest_UnregisterListener_0100
 * @tc.desc: Test the state of UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModule_UnregisterListener_0100, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    sptr<AppRunningStatusListenerInterface> listener;
    ErrCode result = appRunningClient->UnregisterListener(listener);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppRunningStatusModuleTest_UnregisterListener_0200
 * @tc.desc: Test the state of UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModuleTest_UnregisterListener_0200, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    sptr<AppRunningStatusListenerInterface> listener = new (std::nothrow) MockAppRunningStatusListenerInterface();
    ErrCode result = appRunningClient->UnregisterListener(listener);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: AppRunningStatusModuleTest_UnregisterListener_0300
 * @tc.desc: Test the state of UnregisterListener
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModuleTest_UnregisterListener_0300, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    sptr<AppRunningStatusListenerInterface> listener = new (std::nothrow) MockAppRunningStatusListenerInterface();
    sptr<AppRunningStatusModule::ClientDeathRecipient> deathRecipient
        = new (std::nothrow) AppRunningStatusModule::ClientDeathRecipient(appRunningClient);
    EXPECT_NE(deathRecipient, nullptr);
    appRunningClient->listeners_.emplace(listener, deathRecipient);
    ErrCode result = appRunningClient->UnregisterListener(listener);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppRunningStatusModuleTest_RemoveListenerAndDeathRecipient_0100
 * @tc.desc: Test the state of RemoveListenerAndDeathRecipient
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModuleTest_RemoveListenerAndDeathRecipient_0100, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    sptr<AppRunningStatusListenerInterface> listener;
    ErrCode result = appRunningClient->RemoveListenerAndDeathRecipient(listener);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AppRunningStatusModuleTest_RemoveListenerAndDeathRecipient_0100
 * @tc.desc: Test the state of RemoveListenerAndDeathRecipient
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusModuleTest, AppRunningStatusModuleTest_RemoveListenerAndDeathRecipient_0200, TestSize.Level1)
{
    auto appRunningClient = std::make_shared<AppRunningStatusModule>();
    auto listener = new (std::nothrow) MockAppRunningStatusListenerInterface();
    ErrCode result = appRunningClient->RemoveListenerAndDeathRecipient(listener);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}
} // namespace AppExecFwk
} // namespace OHOS
