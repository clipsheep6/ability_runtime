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
#include <gmock/gmock.h>

#define private public
#define protected public

#include "ability_manager_errors.h"
#include "connection_data.h"
#include "connection_observer_client.h"
#include "connection_observer_client_impl.h"
#ifdef WITH_DLP
#include "dlp_state_data.h"
#endif // WITH_DLP
#include "hilog_tag_wrapper.h"
#include "mock_native_token.h"
#include "parcel.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const int32_t TEST_PID = 10001;
const int32_t TEST_UID = 10002;
const std::string TEST_BUNDLE_NAME = "com.ohos.connnection.test";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_ABILITY_NAME = "TestServiceExtension";
const int32_t TEST_CALLER_PID = 10003;
const int32_t TEST_CALLER_UID = 10004;
const std::string TEST_CALLER_NAME = "test_caller";

class MyConnectionObserver : public ConnectionObserver {
public:
    MyConnectionObserver() {}
    ~MyConnectionObserver() {}

    void OnExtensionConnected(const ConnectionData& data)
    {
        isExtensionConnected_ = true;
    }

    void OnExtensionDisconnected(const ConnectionData& data)
    {
        isExtensionDisconnected_ = true;
    }

    void OnDlpAbilityOpened(const DlpStateData& data)
    {
        isDlpAbilityOpened_ = true;
    }

    void OnDlpAbilityClosed(const DlpStateData& data)
    {
        isDlpAbilityClosed_ = true;
    }

    void OnServiceDied()
    {
        isServiceDied_ = true;
    }

    bool IsExtensionConnected() const
    {
        return isExtensionConnected_;
    }

    bool IsExtensionDisconnected() const
    {
        return isExtensionDisconnected_;
    }

    bool IsDlpAbilityOpened() const
    {
        return isDlpAbilityOpened_;
    }

    bool IsDlpAbilityClosed() const
    {
        return isDlpAbilityClosed_;
    }

    bool IsServiceDied() const
    {
        return isServiceDied_;
    }

private:
    bool isExtensionConnected_ = false;
    bool isExtensionDisconnected_ = false;
    bool isDlpAbilityOpened_ = false;
    bool isDlpAbilityClosed_ = false;
    bool isServiceDied_ = false;
};
}

class ConnectionObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConnectionObserverTest::SetUpTestCase(void) {}

void ConnectionObserverTest::TearDownTestCase(void) {}

void ConnectionObserverTest::SetUp(void) {}

void ConnectionObserverTest::TearDown(void) {}

/**
 * @tc.name: ConnectionObserver_Data_0100
 * @tc.desc: ConnectionData test.
 * @tc.type: FUNC
 * @tc.require: SR000H19UG
 */
HWTEST_F(ConnectionObserverTest, ConnectionObserver_Data_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Data_0100 start");

    ConnectionData connectionData;
    connectionData.extensionPid = TEST_PID;
    connectionData.extensionUid = TEST_UID;
    connectionData.extensionBundleName = TEST_BUNDLE_NAME;
    connectionData.extensionModuleName = TEST_MODULE_NAME;
    connectionData.extensionName = TEST_ABILITY_NAME;
    connectionData.extensionType = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    connectionData.callerPid = TEST_CALLER_PID;
    connectionData.callerUid = TEST_CALLER_UID;
    connectionData.callerName = TEST_CALLER_NAME;

    Parcel data;
    EXPECT_TRUE(connectionData.Marshalling(data));

    std::shared_ptr<ConnectionData> readedData(ConnectionData::Unmarshalling(data));
    EXPECT_TRUE(readedData);

    EXPECT_EQ(connectionData.extensionPid, readedData->extensionPid);
    EXPECT_EQ(connectionData.extensionUid, readedData->extensionUid);
    EXPECT_EQ(connectionData.extensionBundleName, readedData->extensionBundleName);
    EXPECT_EQ(connectionData.extensionModuleName, readedData->extensionModuleName);
    EXPECT_EQ(connectionData.extensionName, readedData->extensionName);
    EXPECT_EQ(connectionData.callerPid, readedData->callerPid);
    EXPECT_EQ(connectionData.callerUid, readedData->callerUid);
    EXPECT_EQ(connectionData.callerName, readedData->callerName);

    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Data_0100 end");
}

#ifdef WITH_DLP
/**
 * @tc.name: ConnectionObserver_Data_0200
 * @tc.desc: DlpState data test.
 * @tc.type: FUNC
 * @tc.require: AR000H1PGT
 */
HWTEST_F(ConnectionObserverTest, ConnectionObserver_Data_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Data_0200 start");

    DlpStateData dlpData;
    dlpData.targetPid = TEST_PID;
    dlpData.targetUid = TEST_UID;
    dlpData.targetBundleName = TEST_BUNDLE_NAME;
    dlpData.targetModuleName = TEST_MODULE_NAME;
    dlpData.targetAbilityName = TEST_ABILITY_NAME;
    dlpData.callerPid = TEST_CALLER_PID;
    dlpData.callerUid = TEST_CALLER_UID;
    dlpData.callerName = TEST_CALLER_NAME;

    Parcel data;
    EXPECT_TRUE(dlpData.Marshalling(data));

    std::shared_ptr<DlpStateData> readedData(DlpStateData::Unmarshalling(data));
    EXPECT_TRUE(readedData);

    EXPECT_EQ(dlpData.targetPid, readedData->targetPid);
    EXPECT_EQ(dlpData.targetUid, readedData->targetUid);
    EXPECT_EQ(dlpData.targetBundleName, readedData->targetBundleName);
    EXPECT_EQ(dlpData.targetModuleName, readedData->targetModuleName);
    EXPECT_EQ(dlpData.targetAbilityName, readedData->targetAbilityName);
    EXPECT_EQ(dlpData.callerUid, readedData->callerUid);
    EXPECT_EQ(dlpData.callerPid, readedData->callerPid);
    EXPECT_EQ(dlpData.callerName, readedData->callerName);

    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Data_0200 end");
}

/**
 * @tc.name: ConnectionObserver_Data_0300
 * @tc.desc: DlpState data test.
 * @tc.type: FUNC
 * @tc.require: issueI58213
 */
HWTEST_F(ConnectionObserverTest, ConnectionObserver_Data_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Data_0300 start");

    DlpConnectionInfo info;
    info.dlpUid = TEST_UID;
    info.openedAbilityCount = 1;

    Parcel data;
    EXPECT_TRUE(info.Marshalling(data));

    std::shared_ptr<DlpConnectionInfo> readedData(DlpConnectionInfo::Unmarshalling(data));
    EXPECT_TRUE(readedData);

    EXPECT_EQ(info.dlpUid, readedData->dlpUid);
    EXPECT_EQ(info.openedAbilityCount, readedData->openedAbilityCount);

    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Data_0300 end");
}

/**
 * @tc.name: ConnectionObserver_Observer_0100
 * @tc.desc: test observer callback.
 * @tc.type: FUNC
 * @tc.require: issueI58213
 */
HWTEST_F(ConnectionObserverTest, ConnectionObserver_Observer_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Observer_0100 start");

    auto clientImpl = ConnectionObserverClient::GetInstance().clientImpl_;
    EXPECT_TRUE(clientImpl);

    std::vector<DlpConnectionInfo> infos;
    auto result = ConnectionObserverClient::GetInstance().GetDlpConnectionInfos(infos);
    EXPECT_EQ(result, AAFwk::CHECK_PERMISSION_FAILED);

    std::vector<ConnectionData> connectionDatas;
    result = ConnectionObserverClient::GetInstance().GetConnectionData(connectionDatas);
    EXPECT_EQ(result, AAFwk::CHECK_PERMISSION_FAILED);

    std::shared_ptr<MyConnectionObserver> myObserver = std::make_shared<MyConnectionObserver>();
    clientImpl->userObservers_.emplace(myObserver);
    ConnectionObserverClient::GetInstance().RegisterObserver(myObserver);

    ConnectionData connectionData;
    clientImpl->HandleExtensionConnected(connectionData);
    EXPECT_TRUE(myObserver->IsExtensionConnected());

    clientImpl->HandleExtensionDisconnected(connectionData);
    EXPECT_TRUE(myObserver->IsExtensionDisconnected());

    DlpStateData dlpData;
    clientImpl->HandleDlpAbilityOpened(dlpData);
    EXPECT_TRUE(myObserver->IsDlpAbilityOpened());

    clientImpl->HandleDlpAbilityClosed(dlpData);
    EXPECT_TRUE(myObserver->IsDlpAbilityClosed());

    myObserver->OnServiceDied();
    ConnectionObserverClient::GetInstance().UnregisterObserver(myObserver);

    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Observer_0100 end");
}

/**
 * @tc.name: ConnectionObserver_Observer_0200
 * @tc.desc: test observer callback.
 * @tc.type: FUNC
 * @tc.require: issueI58213
 */
HWTEST_F(ConnectionObserverTest, ConnectionObserver_Observer_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Observer_0200 start");

    auto currentID = GetSelfTokenID();
    AppExecFwk::MockNativeToken::SetNativeToken();

    auto clientImpl = ConnectionObserverClient::GetInstance().clientImpl_;
    EXPECT_TRUE(clientImpl);

    std::vector<DlpConnectionInfo> infos;
    auto result = ConnectionObserverClient::GetInstance().GetDlpConnectionInfos(infos);
    EXPECT_EQ(result, ERR_OK);

    std::vector<ConnectionData> connectionDatas;
    result = ConnectionObserverClient::GetInstance().GetConnectionData(connectionDatas);
    EXPECT_EQ(result, ERR_OK);

    SetSelfTokenID(currentID);
    TAG_LOGI(AAFwkTag::TEST, "ConnectionObserver_Observer_0200 end");
}
#endif // WITH_DLP
}  // namespace AbilityRuntime
}  // namespace OHOS
