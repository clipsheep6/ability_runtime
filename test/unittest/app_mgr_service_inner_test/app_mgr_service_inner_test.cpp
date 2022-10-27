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
#include "app_mgr_service_inner.h"
#undef private
#include "hilog_wrapper.h"
#include "mock_native_token.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceInnerTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceInnerTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerTest::SetUp()
{}

void AppMgrServiceInnerTest::TearDown()
{}

/**
 * @tc.name: PointerDeviceCallback_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceCallback_0100, TestSize.Level1)
{
    HILOG_INFO("PointerDeviceCallback_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(appMgrServiceInner);
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    // invalid parameter value
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "false", nullptr);
    appMgrServiceInner->PointerDeviceEventCallback("invalid_key", "false", context);
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "invalid", context);

    // set "input.pointer.device" to false
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "false", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "false");

    // set "input.pointer.device" to true
    appMgrServiceInner->PointerDeviceEventCallback(key.c_str(), "true", context);
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "true");

    HILOG_INFO("PointerDeviceCallback_0100 end");
}

/**
 * @tc.name: PointerDeviceWatchParameter_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceWatchParameter_0100, TestSize.Level1)
{
    HILOG_INFO("PointerDeviceWatchParameter_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string key = AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE;
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    std::string value;

    appMgrServiceInner->AddWatchParameter();
    sleep(1);

    // invalid parameter value
    system::SetParameter(key.c_str(), "invalid");
    sleep(1);

    // set "input.pointer.device" to false
    system::SetParameter(key.c_str(), "false");
    sleep(2); // sleep 2s, wait until UpdateConfiguration finished.
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "false");

    // set "input.pointer.device" to true
    system::SetParameter(key.c_str(), "true");
    sleep(2); // sleep 2s, wait until UpdateConfiguration finished.
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_EQ(value, "true");

    HILOG_INFO("PointerDeviceWatchParameter_0100 end");
}

/**
 * @tc.name: PointerDeviceUpdateConfig_0100
 * @tc.desc: set parameter, expect config update
 * @tc.type: FUNC
 * @tc.require: I581UL
 */
HWTEST_F(AppMgrServiceInnerTest, PointerDeviceUpdateConfig_0100, TestSize.Level1)
{
    HILOG_INFO("PointerDeviceUpdateConfig_0100 start");

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::shared_ptr<AppExecFwk::Configuration> config;
    std::string value;
    int32_t result;

    appMgrServiceInner->InitGlobalConfiguration();
    config = appMgrServiceInner->GetConfiguration();
    EXPECT_NE(config, nullptr);
    value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    EXPECT_TRUE((value == "true") || (value == "false"));

    // config didn't change
    result = appMgrServiceInner->UpdateConfiguration(*config);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    Configuration changeConfig;
    if (value == "true") {
        changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
        result = appMgrServiceInner->UpdateConfiguration(changeConfig);
        EXPECT_EQ(result, 0);
        config = appMgrServiceInner->GetConfiguration();
        EXPECT_NE(config, nullptr);
        value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        EXPECT_EQ(value, "false");
    } else {
        changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
        result = appMgrServiceInner->UpdateConfiguration(changeConfig);
        EXPECT_EQ(result, 0);
        config = appMgrServiceInner->GetConfiguration();
        EXPECT_NE(config, nullptr);
        value = config->GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        EXPECT_EQ(value, "true");
    }

    HILOG_INFO("PointerDeviceUpdateConfig_0100 end");
}

/**
 * @tc.name: PreStartNWebSpawnProcess_001
 * @tc.desc: prestart nwebspawn process.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerTest, PreStartNWebSpawnProcess_001, TestSize.Level0)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    int callingPid = 1;
    appMgrServiceInner->PreStartNWebSpawnProcess(callingPid);
    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS