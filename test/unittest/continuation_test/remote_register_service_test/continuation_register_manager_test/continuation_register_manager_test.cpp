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
#include <thread>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "continuation_register_manager_proxy.h"
#include "continuation_register_manager.h"
#include "connect_callback_stub.h"
#include "continuation_connector.h"
#undef private
#undef protected
#include "bundle_mgr_interface.h"
#include "ability_manager_interface.h"
#include "iability_controller.h"
#include "pixel_map.h"
#include "ability_info.h"
#include "ability.h"
#include "request_callback.h"

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class ContinuationRegisterManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContinuationRegisterManagerTest::SetUpTestCase(void)
{}
void ContinuationRegisterManagerTest::TearDownTestCase(void)
{}

void ContinuationRegisterManagerTest::SetUp(void)
{}

void ContinuationRegisterManagerTest::TearDown(void)
{}

class MockRequestCallback : public RequestCallback {
public:
    MockRequestCallback() {};
    virtual ~MockRequestCallback() {};

    virtual void OnResult(int result)
    {
        onResult_ = true;
    };

    bool onResult_ = false;
};

/*
 * @tc.number    : ContinuationRegisterManager_Register_001
 * @tc.name      : Register
 * @tc.desc      : Verify that the Register interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Register_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->Register(bundleName, parameter, deviceCallback, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_Register_002
 * @tc.name      : Register
 * @tc.desc      : Verify that the Register interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Register_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(nullptr);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->Register(bundleName, parameter, deviceCallback, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_Unregister_001
 * @tc.name      : Unregister
 * @tc.desc      : Verify that the Unregister interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Unregister_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    constexpr int32_t token = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->Unregister(token, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_Unregister_002
 * @tc.name      : Unregister
 * @tc.desc      : Verify that the Unregister interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Unregister_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    constexpr int32_t token = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(nullptr);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->Unregister(token, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_UpdateConnectStatus_001
 * @tc.name      : UpdateConnectStatus
 * @tc.desc      : Verify that the UpdateConnectStatus interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_UpdateConnectStatus_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->UpdateConnectStatus(token, deviceId, status, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_UpdateConnectStatus_002
 * @tc.name      : UpdateConnectStatus
 * @tc.desc      : Verify that the UpdateConnectStatus interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_UpdateConnectStatus_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(nullptr);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->UpdateConnectStatus(token, deviceId, status, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_ShowDeviceList_001
 * @tc.name      : ShowDeviceList
 * @tc.desc      : Verify that the ShowDeviceList interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_ShowDeviceList_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    constexpr int32_t token = 0;
    ExtraParams parameter;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->ShowDeviceList(token, parameter, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_ShowDeviceList_002
 * @tc.name      : ShowDeviceList
 * @tc.desc      : Verify that the ShowDeviceList interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_ShowDeviceList_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    constexpr int32_t token = 0;
    ExtraParams parameter;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    continuationRegisterManager->Init(nullptr);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManager->ShowDeviceList(token, parameter, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManager_Disconnect_001
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Disconnect_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManager->Init(continuationRegisterManagerProxy);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
    continuationRegisterManager->Disconnect();
    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
}

/*
 * @tc.number    : ContinuationRegisterManager_Disconnect_002
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Disconnect_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = nullptr;
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    continuationRegisterManager->Disconnect();
    EXPECT_TRUE(true);
}

/*
 * @tc.number    : ContinuationRegisterManager_Disconnect_003
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Disconnect_003, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);

    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(false);
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    continuationRegisterManager->Disconnect();
    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
}

/*
 * @tc.number    : ContinuationRegisterManager_Disconnect_004
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerTest, ContinuationRegisterManager_Disconnect_004, TestSize.Level1)
{
    auto continuationRegisterManager = std::make_shared<ContinuationRegisterManager>();
    std::shared_ptr<ContinuationRegisterManagerProxy> continuationRegisterManagerProxy;
    continuationRegisterManager->Init(continuationRegisterManagerProxy);

    continuationRegisterManager->Disconnect();
    EXPECT_TRUE(true);
}
}   // namespace AppExecFwk
}   // namespace OHOS