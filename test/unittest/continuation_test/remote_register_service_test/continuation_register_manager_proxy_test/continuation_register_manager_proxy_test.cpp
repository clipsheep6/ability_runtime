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
class ContinuationRegisterManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContinuationRegisterManagerProxyTest::SetUpTestCase(void)
{}

void ContinuationRegisterManagerProxyTest::TearDownTestCase(void)
{}

void ContinuationRegisterManagerProxyTest::SetUp(void)
{}

void ContinuationRegisterManagerProxyTest::TearDown(void)
{}

class MoclConnectCallback : public RequestCallback {
public:
    MoclConnectCallback() {};
    virtual ~MoclConnectCallback() {};

    void OnResult(int result) override
    {
        onresult_ = true;
    };
    
    bool onresult_ = false;
};

class MoclRequest : public ContinuationRequest {
public:
    MoclRequest() {};
    virtual ~MoclRequest() {};

    void Execute() override
    {
        execute_ = true;
    };

    bool execute_ = false;
};

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
 * @tc.number    : ContinuationRegisterManagerProxy_Execute_001
 * @tc.name      : Register_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Execute_001, TestSize.Level1)
{
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback = nullptr;
    auto pContinuationRequestRegister =
        std::make_shared<ContinuationRequestRegister>(bundleName, parameter, deviceCallback);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestRegister->SetContinuationConnector(nullptr);
    pContinuationRequestRegister->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestRegister->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Execute_002
 * @tc.name      : Register_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Execute_002, TestSize.Level1)
{
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback = nullptr;
    auto pContinuationRequestRegister =
        std::make_shared<ContinuationRequestRegister>(bundleName, parameter, deviceCallback);
    pContinuationRequestRegister->SetContinuationConnector(continuatinConnector);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestRegister->SetRequestCallback(nullptr);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestRegister->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Execute_003
 * @tc.name      : Register_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Execute_003, TestSize.Level1)
{
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback = nullptr;
    auto pContinuationRequestRegister =
        std::make_shared<ContinuationRequestRegister>(bundleName, parameter, deviceCallback);
    pContinuationRequestRegister->SetContinuationConnector(continuatinConnector);
    pContinuationRequestRegister->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestRegister->Execute();
    EXPECT_TRUE(requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestUnRegister_Execute_001
 * @tc.name      : UnRegister_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestUnRegister_Execute_001, TestSize.Level1)
{
    constexpr int32_t token = 0;
    auto pContinuationRequestUnRegister = std::make_shared<ContinuationRequestUnRegister>(token);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestUnRegister->SetContinuationConnector(nullptr);
    pContinuationRequestUnRegister->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestUnRegister->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestUnRegister_Execute_002
 * @tc.name      : UnRegister_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestUnRegister_Execute_002, TestSize.Level1)
{
    constexpr int32_t token = 0;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    auto pContinuationRequestUnRegister = std::make_shared<ContinuationRequestUnRegister>(token);
    pContinuationRequestUnRegister->SetContinuationConnector(continuatinConnector);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestUnRegister->SetRequestCallback(nullptr);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestUnRegister->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestUnRegister_Execute_003
 * @tc.name      : UnRegister_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestUnRegister_Execute_003, TestSize.Level1)
{
    constexpr int32_t token = 0;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    auto pContinuationRequestUnRegister = std::make_shared<ContinuationRequestUnRegister>(token);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestUnRegister->SetContinuationConnector(continuatinConnector);
    pContinuationRequestUnRegister->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestUnRegister->Execute();
    EXPECT_TRUE(requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestUpdateConnectStatus_Execute_001
 * @tc.name      : UpdateConnectStatus_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestUpdateConnectStatus_Execute_001, TestSize.Level1)
{
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    auto pContinuationRequestUpdateConnectStatus =
        std::make_shared<ContinuationRequestUpdateConnectStatus>(token, deviceId, status);
    pContinuationRequestUpdateConnectStatus->SetContinuationConnector(nullptr);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestUpdateConnectStatus->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestUpdateConnectStatus->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestUpdateConnectStatus_Execute_002
 * @tc.name      : UpdateConnectStatus_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestUpdateConnectStatus_Execute_002, TestSize.Level1)
{
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    auto pContinuationRequestUpdateConnectStatus =
        std::make_shared<ContinuationRequestUpdateConnectStatus>(token, deviceId, status);
    pContinuationRequestUpdateConnectStatus->SetContinuationConnector(continuatinConnector);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestUpdateConnectStatus->SetRequestCallback(nullptr);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestUpdateConnectStatus->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestUpdateConnectStatus_Execute_003
 * @tc.name      : UpdateConnectStatus_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestUpdateConnectStatus_Execute_003, TestSize.Level1)
{
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    auto pContinuationRequestUpdateConnectStatus =
        std::make_shared<ContinuationRequestUpdateConnectStatus>(token, deviceId, status);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestUpdateConnectStatus->SetContinuationConnector(continuatinConnector);
    pContinuationRequestUpdateConnectStatus->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestUpdateConnectStatus->Execute();
    EXPECT_TRUE(requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestShowDeviceList_Execute_001
 * @tc.name      : ShowDeviceList_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestShowDeviceList_Execute_001, TestSize.Level1)
{
    constexpr int32_t token = 0;
    ExtraParams parameter;
    auto pContinuationRequestShowDeviceList = std::make_shared<ContinuationRequestShowDeviceList>(token, parameter);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestShowDeviceList->SetContinuationConnector(nullptr);
    pContinuationRequestShowDeviceList->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestShowDeviceList->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestShowDeviceList_Execute_002
 * @tc.name      : ShowDeviceList_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestShowDeviceList_Execute_002, TestSize.Level1)
{
    constexpr int32_t token = 0;
    ExtraParams parameter;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    auto pContinuationRequestShowDeviceList =
        std::make_shared<ContinuationRequestShowDeviceList>(token, parameter);
    pContinuationRequestShowDeviceList->SetContinuationConnector(continuatinConnector);
    pContinuationRequestShowDeviceList->SetRequestCallback(nullptr);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestShowDeviceList->Execute();
    EXPECT_TRUE(!requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRequestShowDeviceList_Execute_003
 * @tc.name      : ShowDeviceList_Execute
 * @tc.desc      : Verify that the Execute interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRequestShowDeviceList_Execute_003, TestSize.Level1)
{
    constexpr int32_t token = 0;
    ExtraParams parameter;
    std::shared_ptr<Context> context;
    sptr<ContinuationConnector> continuatinConnector = new (std::nothrow) ContinuationConnector(context);
    auto pContinuationRequestShowDeviceList =
        std::make_shared<ContinuationRequestShowDeviceList>(token, parameter);
    std::shared_ptr<MoclConnectCallback> requestCallback = std::make_shared<MoclConnectCallback>();
    pContinuationRequestShowDeviceList->SetContinuationConnector(continuatinConnector);
    pContinuationRequestShowDeviceList->SetRequestCallback(requestCallback);
    EXPECT_TRUE(!requestCallback->onresult_);
    pContinuationRequestShowDeviceList->Execute();
    EXPECT_TRUE(requestCallback->onresult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Constructor_001
 * @tc.name      : Constructor
 * @tc.desc      : Verify that the Constructor is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Constructor_001, TestSize.Level1)
{
    std::weak_ptr<Context> context ;
    auto continuatinConnector = std::make_shared<ContinuationRegisterManagerProxy>(context);
    std::shared_ptr<Context> applicationContext = continuatinConnector->applicationContext_.lock();
    EXPECT_TRUE(applicationContext == nullptr);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Register_001
 * @tc.name      : Register
 * @tc.desc      : Verify that the Register interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Register_001, TestSize.Level1)
{
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Context> context;
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->context_.lock() = nullptr;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->Register(bundleName, parameter, deviceCallback, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Register_002
 * @tc.name      : Register
 * @tc.desc      : Verify that the Register interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Register_002, TestSize.Level1)
{
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_.lock() = nullptr;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->Register(bundleName, parameter, deviceCallback, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Register_003
 * @tc.name      : Register
 * @tc.desc      : Verify that the Register interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Register_003, TestSize.Level1)
{
    const std::string bundleName = "";
    ExtraParams parameter;
    std::shared_ptr<IContinuationDeviceCallback> deviceCallback;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->context_ = context;
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->Register(bundleName, parameter, deviceCallback, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Unregister_001
 * @tc.name      : Unregister
 * @tc.desc      : Verify that the Unregister interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Unregister_001, TestSize.Level1)
{
    constexpr int32_t token = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    std::weak_ptr<Context> applicationContext;
    continuationRegisterManagerProxy->applicationContext_ = applicationContext;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->Unregister(token, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Unregister_002
 * @tc.name      : Unregister
 * @tc.desc      : Verify that the Unregister interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Unregister_002, TestSize.Level1)
{
    constexpr int32_t token = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->Unregister(token, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_UpdateConnect_001
 * @tc.name      : UpdateConnectStatus
 * @tc.desc      : Verify that the UpdateConnectStatus interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_UpdateConnect_001, TestSize.Level1)
{
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    std::weak_ptr<Context> applicationContext;
    continuationRegisterManagerProxy->applicationContext_ = applicationContext;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->UpdateConnectStatus(token, deviceId, status, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_UpdateConnect_002
 * @tc.name      : UpdateConnectStatus
 * @tc.desc      : Verify that the UpdateConnectStatus interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_UpdateConnect_002, TestSize.Level1)
{
    constexpr int32_t token = 0;
    const std::string deviceId = "";
    constexpr int32_t status = 0;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->UpdateConnectStatus(token, deviceId, status, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_ShowDeviceList_001
 * @tc.name      : ShowDeviceList
 * @tc.desc      : Verify that the ShowDeviceList interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_ShowDeviceList_001, TestSize.Level1)
{
    constexpr int32_t token = 0;
    ExtraParams parameter;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    std::weak_ptr<Context> applicationContext;
    continuationRegisterManagerProxy->applicationContext_ = applicationContext;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->ShowDeviceList(token, parameter, requestCallback);
    EXPECT_FALSE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_ShowDeviceList_002
 * @tc.name      : ShowDeviceList
 * @tc.desc      : Verify that the ShowDeviceList interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_ShowDeviceList_002, TestSize.Level1)
{
    constexpr int32_t token = 0;
    ExtraParams parameter;
    std::shared_ptr<MockRequestCallback> requestCallback = std::make_shared<MockRequestCallback>();
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->applicationContext_ = context;
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);

    EXPECT_FALSE(requestCallback->onResult_);
    continuationRegisterManagerProxy->ShowDeviceList(token, parameter, requestCallback);
    EXPECT_TRUE(requestCallback->onResult_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Disconnect_001
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Disconnect_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = nullptr;

    continuationRegisterManagerProxy->Disconnect();
    EXPECT_TRUE(true);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Disconnect_002
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Disconnect_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);

    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(false);

    continuationRegisterManagerProxy->Disconnect();
    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_Disconnect_003
 * @tc.name      : Disconnect
 * @tc.desc      : Verify that the Disconnect interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_Disconnect_003, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_ != nullptr);
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());

    continuationRegisterManagerProxy->Disconnect();
    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_SendRequest_001
 * @tc.name      : SendRequest
 * @tc.desc      : Verify that the SendRequest interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_SendRequest_001, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    std::shared_ptr<MoclRequest> request = std::make_shared<MoclRequest>();
    continuationRegisterManagerProxy->SendRequest(context, nullptr);

    EXPECT_TRUE(!request->execute_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_SendRequest_002
 * @tc.name      : SendRequest
 * @tc.desc      : Verify that the SendRequest interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_SendRequest_002, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    std::shared_ptr<MoclRequest> request = std::make_shared<MoclRequest>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = nullptr;
    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_ == nullptr);
    continuationRegisterManagerProxy->SendRequest(context, request);

    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_ != nullptr);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_SendRequest_003
 * @tc.name      : SendRequest
 * @tc.desc      : Verify that the SendRequest interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_SendRequest_003, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    std::shared_ptr<MoclRequest> request = std::make_shared<MoclRequest>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_ != nullptr);
    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
    continuationRegisterManagerProxy->SendRequest(context, request);

    EXPECT_TRUE(!request->execute_);
}

/*
 * @tc.number    : ContinuationRegisterManagerProxy_SendRequest_004
 * @tc.name      : SendRequest
 * @tc.desc      : Verify that the SendRequest interface is called normally
 */
HWTEST_F(ContinuationRegisterManagerProxyTest, ContinuationRegisterManagerProxy_SendRequest_004, TestSize.Level1)
{
    std::shared_ptr<Ability> context = std::make_shared<Ability>();
    std::shared_ptr<MoclRequest> request = std::make_shared<MoclRequest>();
    auto continuationRegisterManagerProxy = std::make_shared<ContinuationRegisterManagerProxy>(context);
    continuationRegisterManagerProxy->continuatinConnector_ = new (std::nothrow) ContinuationConnector(context);
    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_ != nullptr);
    EXPECT_FALSE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
    continuationRegisterManagerProxy->continuatinConnector_->isConnected_.store(true);
    EXPECT_TRUE(continuationRegisterManagerProxy->continuatinConnector_->isConnected_.load());
    EXPECT_TRUE(!request->execute_);
    continuationRegisterManagerProxy->SendRequest(context, request);
    EXPECT_TRUE(request->execute_);
}
}   // namespace AppExecFwk
}   // namespace OHOS