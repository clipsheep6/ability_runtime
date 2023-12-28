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
#include "app_running_status_proxy.h"
#include "mock_app_running_status_module_test.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
namespace {
    const std::string BUNDLE_NAME = "bundleName";
    constexpr uint32_t UID = 1;
}
namespace OHOS {
namespace AbilityRuntime {
class AppRunningStatusProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<MockAppRunningStatusListenerInterface> stub_{ nullptr };
    sptr<AppRunningStatusProxy> proxy_{ nullptr };
};

void AppRunningStatusProxyTest::SetUpTestCase()
{}

void AppRunningStatusProxyTest::TearDownTestCase()
{}

void AppRunningStatusProxyTest::SetUp()
{
    stub_ = new MockAppRunningStatusListenerInterface();
    proxy_ = new AppRunningStatusProxy(stub_);
}

void AppRunningStatusProxyTest::TearDown()
{}

/**
 * @tc.name: NotifyAppRunningStatus_001
 * @tc.desc: Set AppRunningStatus by BundleName and uid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningStatusProxyTest, NotifyAppRunningStatus_001, TestSize.Level1)
{
    HILOG_INFO("NotifyAppRunningStatus_001 start");
    EXPECT_NE(proxy_, nullptr);
    EXPECT_CALL(*stub_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(stub_.GetRefPtr(), &MockAppRunningStatusListenerInterface::InvokeSendRequest));

    proxy_->NotifyAppRunningStatus(BUNDLE_NAME, UID, RunningStatus::APP_RUNNING_START);
    HILOG_INFO("NotifyAppRunningStatus_001 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
