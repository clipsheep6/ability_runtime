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

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#define private public
#define protected public
#include "continuation_device_callback_proxy.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class ContinuationDeviceCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContinuationDeviceCallbackProxyTest::SetUpTestCase(void)
{
}

void ContinuationDeviceCallbackProxyTest::TearDownTestCase(void)
{
}

void ContinuationDeviceCallbackProxyTest::SetUp(void)
{
}

void ContinuationDeviceCallbackProxyTest::TearDown(void)
{
}

class MoclCallback : public IContinuationDeviceCallback {
public:
    MoclCallback() {};
    virtual ~MoclCallback() {};
    virtual void OnDeviceConnectDone(const std::string &deviceId, const std::string &deviceType) {};
    virtual void OnDeviceDisconnectDone(const std::string &deviceId) {};
};

/*
* @tc.number: AppExecFwk_ContinuationDeviceCallbackProxy_Connect_001
* @tc.name: Connect
* @tc.desc: Verify function Connect pointer callback normal
*/
HWTEST_F(ContinuationDeviceCallbackProxyTest, AppExecFwk_ContinuationDeviceCallbackProxy_Connect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Connect_001 start.";
    std::shared_ptr<MoclCallback> callback = std::make_shared<MoclCallback>();
    std::shared_ptr<IContinuationDeviceCallback> connectCallback = callback;
    sptr<ContinuationDeviceCallbackProxy> continuationDeviceCallbackProxy = new (std::nothrow)
        ContinuationDeviceCallbackProxy(connectCallback);
    EXPECT_TRUE(continuationDeviceCallbackProxy != nullptr);
    const std::string deviceId = "7001005458323933328a592135733900";
    const std::string deviceType = "rk3568";
    continuationDeviceCallbackProxy->Connect(deviceId, deviceType);
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Connect_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationDeviceCallbackProxy_Connect_002
* @tc.name: Connect
* @tc.desc: Verify function Connect pointer callback empty
*/
HWTEST_F(ContinuationDeviceCallbackProxyTest, AppExecFwk_ContinuationDeviceCallbackProxy_Connect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Connect_002 start.";
    std::shared_ptr<MoclCallback> callback = nullptr;
    std::shared_ptr<IContinuationDeviceCallback> connectCallback = callback;
    sptr<ContinuationDeviceCallbackProxy> continuationDeviceCallbackProxy = new (std::nothrow)
        ContinuationDeviceCallbackProxy(connectCallback);
    EXPECT_FALSE(continuationDeviceCallbackProxy == nullptr);
    const std::string deviceId = "7001005458323933328a592135733900";
    const std::string deviceType = "rk3568";
    continuationDeviceCallbackProxy->Connect(deviceId, deviceType);
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Connect_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_001
* @tc.name: Disconnect
* @tc.desc: Verify function Disconnect pointer callback normal
*/
HWTEST_F(
    ContinuationDeviceCallbackProxyTest, AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_001 start.";
    std::shared_ptr<MoclCallback> callback = std::make_shared<MoclCallback>();
    std::shared_ptr<IContinuationDeviceCallback> connectCallback = callback;
    sptr<ContinuationDeviceCallbackProxy> continuationDeviceCallbackProxy = new (std::nothrow)
        ContinuationDeviceCallbackProxy(connectCallback);
    EXPECT_TRUE(continuationDeviceCallbackProxy != nullptr);
    const std::string deviceId = "7001005458323933328a592135733900";
    continuationDeviceCallbackProxy->Disconnect(deviceId);
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_002
* @tc.name: Disconnect
* @tc.desc: Verify function Disconnect pointer callback empty
*/
HWTEST_F(
    ContinuationDeviceCallbackProxyTest, AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_002 start.";
    std::shared_ptr<MoclCallback> callback = nullptr;
    std::shared_ptr<IContinuationDeviceCallback> connectCallback = callback;
    sptr<ContinuationDeviceCallbackProxy> continuationDeviceCallbackProxy = new (std::nothrow)
        ContinuationDeviceCallbackProxy(connectCallback);
    EXPECT_FALSE(continuationDeviceCallbackProxy == nullptr);
    const std::string deviceId = "7001005458323933328a592135733900";
    continuationDeviceCallbackProxy->Disconnect(deviceId);
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationDeviceCallbackProxy_Disconnect_002 end.";
}
} // namespace AppExecFwk
} // namespace OHOS