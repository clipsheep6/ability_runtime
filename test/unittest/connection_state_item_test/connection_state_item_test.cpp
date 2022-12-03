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
#include "ability_record.h"
#include "connection_state_item.h"
#include "connection_record.h"
#undef private
#include "connection_observer_errors.h"
#include "ability_connect_callback_interface.h"
#include "ability_connect_callback_stub.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class MyAbilityConnection : public AbilityConnectionStub {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {}
};
class ConnectionStateItemTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<ConnectionStateItem> Init();
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
    std::shared_ptr<DataAbilityRecord> InitDataAbilityRecord();
};

void ConnectionStateItemTest::SetUpTestCase(void)
{}
void ConnectionStateItemTest::TearDownTestCase(void)
{}
void ConnectionStateItemTest::SetUp(void)
{}
void ConnectionStateItemTest::TearDown(void)
{}
std::shared_ptr<ConnectionStateItem> ConnectionStateItemTest::Init()
{
    int32_t callerUid = 0;
    int32_t callerPid = 0;
    std::string callerName = "callerName";
    return std::make_shared<ConnectionStateItem>(callerUid, callerPid, callerName);
}
std::shared_ptr<AbilityRecord> ConnectionStateItemTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}
std::shared_ptr<DataAbilityRecord> ConnectionStateItemTest::InitDataAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    return std::make_shared<DataAbilityRecord>(abilityRequest);
}

/*
 * Feature: ConnectionStateItem
 * Function: CreateConnectionStateItem
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem CreateConnectionStateItem
 * EnvConditions: NA
 * CaseDescription: Verify CreateConnectionStateItem
 */
HWTEST_F(ConnectionStateItemTest, CreateConnectionStateItem_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<ConnectionRecord> record = nullptr;
    auto res = connectionStateItem->CreateConnectionStateItem(record);
    EXPECT_EQ(res, nullptr);
}

/*
 * Feature: ConnectionStateItem
 * Function: CreateConnectionStateItem
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem CreateConnectionStateItem
 * EnvConditions: NA
 * CaseDescription: Verify CreateConnectionStateItem
 */
HWTEST_F(ConnectionStateItemTest, CreateConnectionStateItem_002, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback;
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    auto res = connectionStateItem->CreateConnectionStateItem(record);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: ConnectionStateItem
 * Function: CreateConnectionStateItem
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem CreateConnectionStateItem
 * EnvConditions: NA
 * CaseDescription: Verify CreateConnectionStateItem
 */
HWTEST_F(ConnectionStateItemTest, CreateConnectionStateItem_003, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller dataCaller;
    auto res = connectionStateItem->CreateConnectionStateItem(dataCaller);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateItemTest, AddConnection_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<ConnectionRecord> record = nullptr;
    ConnectionData data;
    auto res = connectionStateItem->AddConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateItemTest, AddConnection_002, TestSize.Level1)
{
    auto connectionStateItem = Init();
    sptr<IRemoteObject> callerToken;
    std::shared_ptr<AbilityRecord> abilityRecord;
    sptr<IAbilityConnection> connCallback;
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        callerToken, abilityRecord, connCallback);
    ConnectionData data;
    auto res = connectionStateItem->AddConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateItemTest, AddConnection_003, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback;
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    auto res = connectionStateItem->AddConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateItemTest, AddConnection_004, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback = new MyAbilityConnection();
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    auto res = connectionStateItem->AddConnection(record, data);
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateItemTest, AddConnection_005, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback = new MyAbilityConnection();
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    connectionStateItem->connectionMap_[abilityRecord->GetToken()] = nullptr;
    auto res = connectionStateItem->AddConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveConnection_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<ConnectionRecord> record = nullptr;
    ConnectionData data;
    auto res = connectionStateItem->RemoveConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveConnection_002, TestSize.Level1)
{
    auto connectionStateItem = Init();
    sptr<IRemoteObject> callerToken;
    std::shared_ptr<AbilityRecord> abilityRecord;
    sptr<IAbilityConnection> connCallback;
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        callerToken, abilityRecord, connCallback);
    ConnectionData data;
    auto res = connectionStateItem->RemoveConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveConnection_003, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback;
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    auto res = connectionStateItem->RemoveConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveConnection_004, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback = new MyAbilityConnection();
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    auto res = connectionStateItem->RemoveConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveConnection_005, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback = new MyAbilityConnection();
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    connectionStateItem->connectionMap_[abilityRecord->GetToken()] = nullptr;
    auto res = connectionStateItem->RemoveConnection(record, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveConnection_006, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback = new MyAbilityConnection();
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    connectionStateItem->AddConnection(record, data);
    auto res = connectionStateItem->RemoveConnection(record, data);
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, AddDataAbilityConnection_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility;
    ConnectionData data;
    auto res = connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, AddDataAbilityConnection_002, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    ConnectionData data;
    auto res = connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, AddDataAbilityConnection_003, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    caller.isNotHap = true;
    caller.callerToken = abilityRecord->GetToken();
    dataAbility->ability_ = abilityRecord;
    ConnectionData data;
    auto res = connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, AddDataAbilityConnection_004, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    dataAbility->ability_ = abilityRecord;
    connectionStateItem->dataAbilityMap_.emplace(abilityRecord->GetToken(), nullptr);
    ConnectionData data;
    auto res = connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: AddDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem AddDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, AddDataAbilityConnection_005, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    caller.isNotHap = true;
    caller.callerToken = abilityRecord->GetToken();
    dataAbility->ability_ = abilityRecord;
    ConnectionData data;
    connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    caller.isNotHap = false;
    auto res1 = connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res1);
    caller.callerToken = nullptr;
    auto res2 = connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res2);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveDataAbilityConnection_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility;
    ConnectionData data;
    auto res = connectionStateItem->RemoveDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveDataAbilityConnection_002, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    ConnectionData data;
    auto res = connectionStateItem->RemoveDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveDataAbilityConnection_003, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    dataAbility->ability_ = abilityRecord;
    ConnectionData data;
    auto res = connectionStateItem->RemoveDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveDataAbilityConnection_004, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    dataAbility->ability_ = abilityRecord;
    connectionStateItem->dataAbilityMap_.emplace(abilityRecord->GetToken(), nullptr);
    ConnectionData data;
    auto res = connectionStateItem->RemoveDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveDataAbilityConnection_005, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    caller.isNotHap = false;
    caller.callerToken = nullptr;
    dataAbility->ability_ = abilityRecord;
    ConnectionData data;
    connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    auto res = connectionStateItem->RemoveDataAbilityConnection(caller, dataAbility, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateItemTest, RemoveDataAbilityConnection_006, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    caller.isNotHap = true;
    caller.callerToken = abilityRecord->GetToken();
    dataAbility->ability_ = abilityRecord;
    ConnectionData data;
    connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    auto res = connectionStateItem->RemoveDataAbilityConnection(caller, dataAbility, data);
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateItemTest, HandleDataAbilityDied_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    sptr<IRemoteObject> token;
    ConnectionData data;
    auto res = connectionStateItem->HandleDataAbilityDied(token, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateItemTest, HandleDataAbilityDied_002, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    ConnectionData data;
    auto res = connectionStateItem->HandleDataAbilityDied(token, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateItemTest, HandleDataAbilityDied_003, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    connectionStateItem->dataAbilityMap_.emplace(abilityRecord->GetToken(), nullptr);
    ConnectionData data;
    auto res = connectionStateItem->HandleDataAbilityDied(token, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateItemTest, HandleDataAbilityDied_004, TestSize.Level1)
{
    auto connectionStateItem = Init();
    DataAbilityCaller caller;
    ConnectionData data;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    dataAbility->ability_ = abilityRecord;
    connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    auto res = connectionStateItem->HandleDataAbilityDied(token, data);
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: IsEmpty
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem IsEmpty
 * EnvConditions: NA
 * CaseDescription: Verify IsEmpty
 */
HWTEST_F(ConnectionStateItemTest, IsEmpty_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    auto res = connectionStateItem->IsEmpty();
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateItem
 * Function: GenerateAllConnectionData
 * SubFunction: NA
 * FunctionPoints: ConnectionStateItem GenerateAllConnectionData
 * EnvConditions: NA
 * CaseDescription: Verify GenerateAllConnectionData
 */
HWTEST_F(ConnectionStateItemTest, GenerateAllConnectionData_001, TestSize.Level1)
{
    auto connectionStateItem = Init();
    std::vector<AbilityRuntime::ConnectionData> datas;
    std::shared_ptr<AbilityRecord> abilityRecord = InitAbilityRecord();
    sptr<IAbilityConnection> connCallback = new MyAbilityConnection();
    std::shared_ptr<ConnectionRecord> record = std::make_shared<ConnectionRecord>(
        abilityRecord->GetToken(), abilityRecord, connCallback);
    ConnectionData data;
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbility = InitDataAbilityRecord();
    caller.isNotHap = true;
    caller.callerToken = abilityRecord->GetToken();
    dataAbility->ability_ = abilityRecord;
    connectionStateItem->AddConnection(record, data);
    connectionStateItem->AddDataAbilityConnection(caller, dataAbility, data);
    connectionStateItem->GenerateAllConnectionData(datas);
}
}  // namespace AAFwk
}  // namespace OHOS
