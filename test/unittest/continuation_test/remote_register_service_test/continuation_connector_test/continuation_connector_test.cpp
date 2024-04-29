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
#include "ability.h"
#include "continuation_connector.h"
#include "remote_register_service_stub.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;

namespace OHOS {
namespace AppExecFwk {
class ContinuationConnectorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContinuationConnectorTest::SetUpTestCase(void)
{}
void ContinuationConnectorTest::TearDownTestCase(void)
{}

void ContinuationConnectorTest::SetUp(void)
{}

void ContinuationConnectorTest::TearDown(void)
{}

class MockRegisterService : public RemoteRegisterServiceStub {
public:
    MockRegisterService() {};
    virtual ~MockRegisterService() {};

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        GTEST_LOG_(INFO) << "MockRegisterService::SendRequest called. return value " << returnCode_;
        reply.WriteInt32(ERR_NONE);
        return returnCode_;
    }

    int Register(const std::string &bundleName, const sptr<IRemoteObject> &token, const ExtraParams &extras,
        const sptr<IConnectCallback> &callback) override
    {
        return register_;
    };
    bool Unregister(int registerToken) override
    {
        return unregister_;
    };
    bool UpdateConnectStatus(int registerToken, const std::string &deviceId, int status) override
    {
        return updateConnectStatus_;
    };
    bool ShowDeviceList(int registerToken, const ExtraParams &extras) override
    {
        return showDeviceList_;
    };

    sptr<IRemoteBroker> AsInterface() override
    {
        if (!asInterface_) {
            return nullptr;
        }

        return this;
    }

    int32_t returnCode_ = ERR_NONE;
    int32_t register_ = ERR_NONE;
    bool unregister_ = true;
    bool updateConnectStatus_ = true;
    bool showDeviceList_ = true;
    bool asInterface_ = true;
};

class MockRequest : public ContinuationRequest {
public:
    MockRequest() {};
    virtual ~MockRequest() {};

    void Execute() override
    {
        flag = true;
    };

    bool flag = false;
};

class MockContext : public Ability {
public:
    MockContext() {};
    virtual ~MockContext() {};

    ErrCode DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn) override
    {
        GTEST_LOG_(INFO) << "Mock DisconnectAbility called.";
        return ERR_OK;
    }

    bool ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn) override
    {
        GTEST_LOG_(INFO) << "Mock ConnectAbility called.";
        return true;
    }
};

/*
* @tc.number: AppExecFwk_ContinuationConnector_GetInstance_001
* @tc.name: GetInstance
* @tc.desc: Verify function GetInstance normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_GetInstance_001 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    EXPECT_TRUE(ContinuationConnector::instance_ == nullptr);
    EXPECT_TRUE(ContinuationConnector::GetInstance(ability) != nullptr);
    EXPECT_TRUE(ContinuationConnector::instance_ != nullptr);
    ContinuationConnector::instance_.clear();
    ContinuationConnector::instance_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_GetInstance_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_GetInstance_002
* @tc.name: GetInstance
* @tc.desc: Verify function GetInstance normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_GetInstance_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_GetInstance_002 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    EXPECT_TRUE(ContinuationConnector::GetInstance(ability) != nullptr);
    EXPECT_TRUE(ContinuationConnector::instance_ != nullptr);
    EXPECT_TRUE(ContinuationConnector::GetInstance(ability) != nullptr);
    EXPECT_TRUE(ContinuationConnector::instance_ != nullptr);
    ContinuationConnector::instance_.clear();
    ContinuationConnector::instance_ = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_GetInstance_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_OnAbilityConnectDone_001
* @tc.name: OnAbilityConnectDone
* @tc.desc: Verify function OnAbilityConnectDone normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_OnAbilityConnectDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_001 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    auto mockRequest = std::make_shared<MockRequest>();
    EXPECT_TRUE(mockRequest != nullptr);
    EXPECT_FALSE(mockRequest->flag);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->continuationRequestList_.push_back(mockRequest);
    AppExecFwk::ElementName element;
    constexpr int32_t registerToken = 0;
    testConnector->OnAbilityConnectDone(element, object, registerToken);
    EXPECT_TRUE(mockRequest->flag);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_OnAbilityConnectDone_002
* @tc.name: OnAbilityConnectDone
* @tc.desc: Verify function OnAbilityConnectDone abnormal branch, parameter object is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_OnAbilityConnectDone_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_002 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<MockRegisterService> object = nullptr;
    EXPECT_TRUE(object == nullptr);
    auto mockRequest = std::make_shared<MockRequest>();
    EXPECT_TRUE(mockRequest != nullptr);
    EXPECT_FALSE(mockRequest->flag);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->continuationRequestList_.push_back(mockRequest);
    AppExecFwk::ElementName element;
    constexpr int32_t registerToken = 0;
    testConnector->OnAbilityConnectDone(element, object, registerToken);
    EXPECT_FALSE(mockRequest->flag);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_OnAbilityConnectDone_003
* @tc.name: OnAbilityConnectDone
* @tc.desc: Verify function OnAbilityConnectDone abnormal branch, conversion registerService is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_OnAbilityConnectDone_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_003 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    auto mockRequest = std::make_shared<MockRequest>();
    EXPECT_TRUE(mockRequest != nullptr);
    EXPECT_FALSE(mockRequest->flag);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->continuationRequestList_.push_back(mockRequest);
    AppExecFwk::ElementName element;
    constexpr int32_t registerToken = 0;
    object->asInterface_ = false;
    testConnector->OnAbilityConnectDone(element, object, registerToken);
    EXPECT_FALSE(mockRequest->flag);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_003 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_OnAbilityConnectDone_004
* @tc.name: OnAbilityConnectDone
* @tc.desc: Verify function OnAbilityConnectDone normal branch, member variable requestList is empty
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_OnAbilityConnectDone_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_004 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    AppExecFwk::ElementName element;
    constexpr int32_t registerToken = 0;
    EXPECT_TRUE(testConnector->continuationRequestList_.size() == 0);
    testConnector->OnAbilityConnectDone(element, object, registerToken);
    EXPECT_TRUE(testConnector->continuationRequestList_.size() == 0);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityConnectDone_004 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_001
* @tc.name: OnAbilityDisconnectDone
* @tc.desc: Verify function OnAbilityDisconnectDone normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_001 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    AppExecFwk::ElementName element;
    constexpr int32_t registerToken = 0;
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    testConnector->OnAbilityDisconnectDone(element, registerToken);
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_002
* @tc.name: OnAbilityDisconnectDone
* @tc.desc: Verify function OnAbilityDisconnectDone normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_002 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    AppExecFwk::ElementName element;
    constexpr int32_t registerToken = 0;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    testConnector->OnAbilityDisconnectDone(element, registerToken);
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_OnAbilityDisconnectDone_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_001
* @tc.name: BindRemoteRegisterAbility
* @tc.desc: Verify function BindRemoteRegisterAbility normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_001 start.";
    auto mockRequest = std::make_shared<MockRequest>();
    EXPECT_TRUE(mockRequest != nullptr);
    std::shared_ptr<MockContext> context = std::make_shared<MockContext>();
    EXPECT_TRUE(context != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(context);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->isConnected_.store(false);
    EXPECT_FALSE(testConnector->isConnected_.load());
    testConnector->BindRemoteRegisterAbility(mockRequest);
    EXPECT_TRUE(testConnector->continuationRequestList_.size() == 1);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_002
* @tc.name: BindRemoteRegisterAbility
* @tc.desc: Verify function BindRemoteRegisterAbility abnormal branch, member variable context_ is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_002 start.";
    auto mockRequest = std::make_shared<MockRequest>();
    EXPECT_TRUE(mockRequest != nullptr);
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->BindRemoteRegisterAbility(mockRequest);
    EXPECT_TRUE(testConnector->continuationRequestList_.size() == 0);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_003
* @tc.name: BindRemoteRegisterAbility
* @tc.desc: Verify function BindRemoteRegisterAbility abnormal branch, parameter request is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_003 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->BindRemoteRegisterAbility(nullptr);
    EXPECT_TRUE(testConnector->continuationRequestList_.size() == 0);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_003 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_004
* @tc.name: BindRemoteRegisterAbility
* @tc.desc: Verify function BindRemoteRegisterAbility normal branch, member variable isConnected is true
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_004 start.";
    auto mockRequest = std::make_shared<MockRequest>();
    EXPECT_TRUE(mockRequest != nullptr);
    EXPECT_FALSE(mockRequest->flag);
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->isConnected_.store(true);
    EXPECT_TRUE(testConnector->isConnected_.load());
    testConnector->BindRemoteRegisterAbility(mockRequest);
    EXPECT_TRUE(testConnector->continuationRequestList_.size() == 0);
    EXPECT_TRUE(mockRequest->flag);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_BindRemoteRegisterAbility_004 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_001
* @tc.name: UnbindRemoteRegisterAbility
* @tc.desc: Verify function UnbindRemoteRegisterAbility normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_001 start.";
    std::shared_ptr<MockContext> context = std::make_shared<MockContext>();
    EXPECT_TRUE(context != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(context);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->isConnected_.store(true);
    EXPECT_TRUE(testConnector->isConnected_.load());
    testConnector->UnbindRemoteRegisterAbility();
    EXPECT_FALSE(testConnector->isConnected_.load());
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_002
* @tc.name: UnbindRemoteRegisterAbility
* @tc.desc: Verify function UnbindRemoteRegisterAbility abnormal branch, member varable context_ is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_002 start.";
    std::shared_ptr<MockContext> context = nullptr;
    EXPECT_TRUE(context == nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(context);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->isConnected_.store(true);
    EXPECT_TRUE(testConnector->isConnected_.load());
    testConnector->UnbindRemoteRegisterAbility();
    EXPECT_TRUE(testConnector->isConnected_.load());
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UnbindRemoteRegisterAbility_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_IsAbilityConnected_001
* @tc.name: IsAbilityConnected
* @tc.desc: Verify function IsAbilityConnected normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_IsAbilityConnected_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_IsAbilityConnected_001 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->isConnected_.store(true);
    EXPECT_TRUE(testConnector->IsAbilityConnected());
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_IsAbilityConnected_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_IsAbilityConnected_002
* @tc.name: IsAbilityConnected
* @tc.desc: Verify function IsAbilityConnected normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_IsAbilityConnected_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_IsAbilityConnected_002 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    testConnector->isConnected_.store(false);
    EXPECT_FALSE(testConnector->IsAbilityConnected());
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_IsAbilityConnected_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Unregister_001
* @tc.name: Unregister
* @tc.desc: Verify function Unregister normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Unregister_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Unregister_001 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    constexpr int32_t registerToken = 100;
    object->unregister_ = true;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    EXPECT_TRUE(testConnector->Unregister(registerToken));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Unregister_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Unregister_002
* @tc.name: Unregister
* @tc.desc: Verify function Unregister normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Unregister_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Unregister_002 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    constexpr int32_t registerToken = 100;
    object->unregister_ = false;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    EXPECT_FALSE(testConnector->Unregister(registerToken));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Unregister_002 end.";
}

/*
 * Feature: AbilityManager
 * Function: ContinuationConnector
 * SubFunction: Unregister
 * FunctionPoints: The parameter of function Unregister.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function Unregister abnormal branch
 */
/*
* @tc.number: AppExecFwk_ContinuationConnector_Unregister_002
* @tc.name: Unregister
* @tc.desc: Verify function Unregister abnormal branch, member variable remoteRegisterService_ is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Unregister_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Unregister_003 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    EXPECT_FALSE(testConnector->Unregister(registerToken));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Unregister_003 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_UpdateConnectStatus_001
* @tc.name: UpdateConnectStatus
* @tc.desc: Verify function UpdateConnectStatus normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_UpdateConnectStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UpdateConnectStatus_001 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    constexpr int32_t stage = 1;
    const std::string deviceId = "7001005458323933328a592135733900";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    object->updateConnectStatus_ = true;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    EXPECT_TRUE(testConnector->UpdateConnectStatus(registerToken, deviceId, stage));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UpdateConnectStatus_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_UpdateConnectStatus_002
* @tc.name: UpdateConnectStatus
* @tc.desc: Verify function UpdateConnectStatus normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_UpdateConnectStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UpdateConnectStatus_002 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    constexpr int32_t stage = 1;
    const std::string deviceId = "7001005458323933328a592135733900";
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    object->updateConnectStatus_ = false;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    EXPECT_FALSE(testConnector->UpdateConnectStatus(registerToken, deviceId, stage));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UpdateConnectStatus_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_UpdateConnectStatus_002
* @tc.name: UpdateConnectStatus
* @tc.desc: Verify function UpdateConnectStatus abnormal branch, member variable remoteRegisterService_ is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_UpdateConnectStatus_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UpdateConnectStatus_003 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    constexpr int32_t stage = 1;
    const std::string deviceId = "7001005458323933328a592135733900";
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    EXPECT_FALSE(testConnector->UpdateConnectStatus(registerToken, deviceId, stage));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_UpdateConnectStatus_003 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_ShowDeviceList_001
* @tc.name: ShowDeviceList
* @tc.desc: Verify function ShowDeviceList normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_ShowDeviceList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_ShowDeviceList_001 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    const ExtraParams extra = {};
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    object->showDeviceList_ = true;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    EXPECT_TRUE(testConnector->ShowDeviceList(registerToken, extra));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_ShowDeviceList_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_ShowDeviceList_002
* @tc.name: ShowDeviceList
* @tc.desc: Verify function ShowDeviceList normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_ShowDeviceList_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_ShowDeviceList_002 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    const ExtraParams extra = {};
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    object->showDeviceList_ = false;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    EXPECT_FALSE(testConnector->ShowDeviceList(registerToken, extra));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_ShowDeviceList_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_ShowDeviceList_003
* @tc.name: ShowDeviceList
* @tc.desc: Verify function ShowDeviceList abnormal branch, member variable remoteRegisterService_ is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_ShowDeviceList_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_ShowDeviceList_003 start.";
    std::shared_ptr<Ability> ability = nullptr;
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    constexpr int32_t registerToken = 100;
    const ExtraParams extra = {};
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    EXPECT_FALSE(testConnector->ShowDeviceList(registerToken, extra));
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_ShowDeviceList_003 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Register_001
* @tc.name: Register
* @tc.desc: Verify function Register normal branch
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Register_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_001 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    const std::string bundleName = "ABC";
    const ExtraParams extra = {};
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    ability->token_ = object;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    std::weak_ptr<Context> context = ability;
    EXPECT_EQ(testConnector->Register(context, bundleName, extra, callback), ERR_NONE);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_001 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Register_002
* @tc.name: Register
* @tc.desc: Verify function Register abnormal branch, interface Register return error code
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Register_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_002 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    const std::string bundleName = "ABC";
    const ExtraParams extra = {};
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    ability->token_ = object;
    object->register_ = ERR_NONE - 1;
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    std::weak_ptr<Context> context = ability;
    EXPECT_EQ(testConnector->Register(context, bundleName, extra, callback), ERR_NONE - 1);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_002 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Register_003
* @tc.name: Register
* @tc.desc: Verify function Register abnormal branch, parameter context is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Register_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_003 start.";
    std::shared_ptr<Ability> ability = nullptr;
    EXPECT_TRUE(ability == nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    const std::string bundleName = "ABC";
    const ExtraParams extra = {};
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    std::weak_ptr<Context> context = ability;
    EXPECT_EQ(testConnector->Register(context, bundleName, extra, callback), -1);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_003 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Register_004
* @tc.name: Register
* @tc.desc: Verify function Register abnormal branch, member variable remoteRegisterService_ is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Register_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_004 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    const std::string bundleName = "ABC";
    const ExtraParams extra = {};
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    ability->token_ = object;
    EXPECT_TRUE(ability->token_ != nullptr);
    testConnector->remoteRegisterService_ = nullptr;
    EXPECT_TRUE(testConnector->remoteRegisterService_ == nullptr);
    std::weak_ptr<Context> context = ability;
    EXPECT_EQ(testConnector->Register(context, bundleName, extra, callback), -1);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_004 end.";
}

/*
* @tc.number: AppExecFwk_ContinuationConnector_Register_005
* @tc.name: Register
* @tc.desc: Verify function Register abnormal branch, obtained token is nullptr
*/
HWTEST_F(ContinuationConnectorTest, AppExecFwk_ContinuationConnector_Register_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_005 start.";
    std::shared_ptr<Ability> ability = std::make_shared<Ability>();
    EXPECT_TRUE(ability != nullptr);
    sptr<ContinuationConnector> testConnector = new (std::nothrow) ContinuationConnector(ability);
    EXPECT_TRUE(testConnector != nullptr);
    const std::string bundleName = "ABC";
    const ExtraParams extra = {};
    std::shared_ptr<IContinuationDeviceCallback> callback = nullptr;
    sptr<MockRegisterService> object = new (std::nothrow) MockRegisterService();
    EXPECT_TRUE(object != nullptr);
    EXPECT_TRUE(ability->token_ == nullptr);
    testConnector->remoteRegisterService_ = object;
    EXPECT_TRUE(testConnector->remoteRegisterService_ != nullptr);
    std::weak_ptr<Context> context = ability;
    EXPECT_EQ(testConnector->Register(context, bundleName, extra, callback), -1);
    testConnector.clear();
    testConnector = nullptr;
    GTEST_LOG_(INFO) << "AppExecFwk_ContinuationConnector_Register_005 end.";
}
}   // namespace AppExecFwk
}   // namespace OHOS