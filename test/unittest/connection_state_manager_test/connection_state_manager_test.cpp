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
#include "connection_state_manager.h"
#undef private
#include "ability_connection.h"
#include "ability_record.h"
#include "connection_observer_errors.h"
#include "data_ability_record.h"
#include "dlp_state_item.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class ConnectionStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<ConnectionStateManager> manager_ {nullptr};
    std::shared_ptr<AbilityRecord> abilityRecord_ {nullptr};
    std::shared_ptr<DataAbilityRecord> dataAbilityRecord_ {nullptr};
    sptr<IAbilityConnection> callback_ {nullptr};

    class AbilityConnectionMock : public IAbilityConnection {
    public:
        AbilityConnectionMock() = default;
        virtual ~AbilityConnectionMock() = default;
        void OnAbilityConnectDone(
            const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override
        {}
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
        {}
        sptr<IRemoteObject> AsObject() override
        {
            return {};
        }
    };
};

void ConnectionStateManagerTest::SetUpTestCase(void)
{}
void ConnectionStateManagerTest::TearDownTestCase(void)
{}
void ConnectionStateManagerTest::TearDown(void)
{}
void ConnectionStateManagerTest::SetUp()
{
    Want want;
    AbilityInfo abilityInfo;
    ApplicationInfo applicationInfo;
    AbilityRequest abilityRequest;
    abilityRecord_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    dataAbilityRecord_ = std::make_shared<DataAbilityRecord>(abilityRequest);
    manager_ = std::make_shared<ConnectionStateManager>();
    callback_ = new AbilityConnectionMock();
}

/*
 * Feature: ConnectionStateManager
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterObserver
 */
HWTEST_F(ConnectionStateManagerTest, RegisterObserver_001, TestSize.Level1)
{
    sptr<IConnectionObserver> observer = nullptr;
    int res = manager_->RegisterObserver(observer);
    EXPECT_EQ(res, ERR_SERVICE_NOT_INIT);
}

/*
 * Feature: ConnectionStateManager
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify RegisterObserver
 */
HWTEST_F(ConnectionStateManagerTest, RegisterObserver_002, TestSize.Level1)
{
    sptr<IConnectionObserver> observer = nullptr;
    manager_->Init();
    int res = manager_->RegisterObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OBSERVER);
}

/*
 * Feature: ConnectionStateManager
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterObserver
 */
HWTEST_F(ConnectionStateManagerTest, UnregisterObserver_001, TestSize.Level1)
{
    sptr<IConnectionObserver> observer = nullptr;
    int res = manager_->UnregisterObserver(observer);
    EXPECT_EQ(res, ERR_SERVICE_NOT_INIT);
}

/*
 * Feature: ConnectionStateManager
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify UnregisterObserver
 */
HWTEST_F(ConnectionStateManagerTest, UnregisterObserver_002, TestSize.Level1)
{
    sptr<IConnectionObserver> observer = nullptr;
    manager_->Init();
    int res = manager_->UnregisterObserver(observer);
    EXPECT_EQ(res, 0);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateManagerTest, AddConnection_001, TestSize.Level1)
{
    std::shared_ptr<ConnectionRecord> connectionRecord = nullptr;
    manager_->Init();
    manager_->AddConnection(connectionRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddConnection
 */
HWTEST_F(ConnectionStateManagerTest, AddConnection_002, TestSize.Level1)
{
    auto connectionRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback_);
    manager_->Init();
    manager_->AddConnection(connectionRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateManagerTest, RemoveConnection_001, TestSize.Level1)
{
    std::shared_ptr<ConnectionRecord> connectionRecord = nullptr;
    bool isCallerDied = false;
    manager_->RemoveConnection(connectionRecord, isCallerDied);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateManagerTest, RemoveConnection_002, TestSize.Level1)
{
    std::shared_ptr<ConnectionRecord> connectionRecord = nullptr;
    bool isCallerDied = false;
    manager_->Init();
    manager_->RemoveConnection(connectionRecord, isCallerDied);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateManagerTest, RemoveConnection_003, TestSize.Level1)
{
    auto connectionRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback_);
    bool isCallerDied = true;
    manager_->Init();
    manager_->RemoveConnection(connectionRecord, isCallerDied);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnection
 */
HWTEST_F(ConnectionStateManagerTest, RemoveConnection_004, TestSize.Level1)
{
    auto connectionRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback_);
    bool isCallerDied = false;
    manager_->Init();
    manager_->RemoveConnection(connectionRecord, isCallerDied);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnection
 */
HWTEST_F(ConnectionStateManagerTest, AddDataAbilityConnection_001, TestSize.Level1)
{
    DataAbilityCaller caller;
    caller.callerPid = 1;
    manager_->Init();
    manager_->AddDataAbilityConnection(caller, dataAbilityRecord_);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveDataAbilityConnection
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveDataAbilityConnection
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnection
 */
HWTEST_F(ConnectionStateManagerTest, RemoveDataAbilityConnection_001, TestSize.Level1)
{
    DataAbilityCaller caller;
    caller.callerPid = 1;
    manager_->Init();
    manager_->RemoveDataAbilityConnection(caller, dataAbilityRecord_);
}

/*
 * Feature: ConnectionStateManager
 * Function: CheckDataAbilityConnectionParams
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager CheckDataAbilityConnectionParams
 * EnvConditions: NA
 * CaseDescription: Verify CheckDataAbilityConnectionParams
 */
HWTEST_F(ConnectionStateManagerTest, CheckDataAbilityConnectionParams_001, TestSize.Level1)
{
    DataAbilityCaller caller;
    bool res = manager_->CheckDataAbilityConnectionParams(caller, dataAbilityRecord_);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: CheckDataAbilityConnectionParams
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager CheckDataAbilityConnectionParams
 * EnvConditions: NA
 * CaseDescription: Verify CheckDataAbilityConnectionParams
 */
HWTEST_F(ConnectionStateManagerTest, CheckDataAbilityConnectionParams_002, TestSize.Level1)
{
    DataAbilityCaller caller;
    std::shared_ptr<DataAbilityRecord> dataAbilityRecord = nullptr;
    manager_->Init();
    bool res = manager_->CheckDataAbilityConnectionParams(caller, dataAbilityRecord);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: CheckDataAbilityConnectionParams
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager CheckDataAbilityConnectionParams
 * EnvConditions: NA
 * CaseDescription: Verify CheckDataAbilityConnectionParams
 */
HWTEST_F(ConnectionStateManagerTest, CheckDataAbilityConnectionParams_003, TestSize.Level1)
{
    DataAbilityCaller caller;
    caller.callerPid = 0;
    manager_->Init();
    bool res = manager_->CheckDataAbilityConnectionParams(caller, dataAbilityRecord_);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: CheckDataAbilityConnectionParams
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager CheckDataAbilityConnectionParams
 * EnvConditions: NA
 * CaseDescription: Verify CheckDataAbilityConnectionParams
 */
HWTEST_F(ConnectionStateManagerTest, CheckDataAbilityConnectionParams_004, TestSize.Level1)
{
    DataAbilityCaller caller;
    caller.callerPid = 1;
    manager_->Init();
    bool res = manager_->CheckDataAbilityConnectionParams(caller, dataAbilityRecord_);
    EXPECT_TRUE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleDataAbilityDied_001, TestSize.Level1)
{
    std::shared_ptr<DataAbilityRecord> dataAbilityRecord = nullptr;
    manager_->Init();
    manager_->HandleDataAbilityDied(dataAbilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleDataAbilityDied_002, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<DataAbilityRecord> dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = nullptr;
    manager_->Init();
    manager_->HandleDataAbilityDied(dataAbilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDataAbilityDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDataAbilityDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleDataAbilityDied_003, TestSize.Level1)
{
    AbilityRequest abilityRequest;
    std::shared_ptr<DataAbilityRecord> dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord_;
    manager_->Init();
    manager_->HandleDataAbilityDied(dataAbilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDataAbilityCallerDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDataAbilityCallerDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityCallerDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleDataAbilityCallerDied_001, TestSize.Level1)
{
    int32_t callerPid = 0;
    manager_->HandleDataAbilityCallerDied(callerPid);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDataAbilityCallerDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDataAbilityCallerDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityCallerDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleDataAbilityCallerDied_002, TestSize.Level1)
{
    int32_t callerPid = 1;
    manager_->HandleDataAbilityCallerDied(callerPid);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDlpManager
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDlpManager
 * EnvConditions: NA
 * CaseDescription: Verify AddDlpManager
 */
HWTEST_F(ConnectionStateManagerTest, AddDlpManager_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpManger = nullptr;
    manager_->AddDlpManager(dlpManger);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDlpManager
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDlpManager
 * EnvConditions: NA
 * CaseDescription: Verify AddDlpManager
 */
HWTEST_F(ConnectionStateManagerTest, AddDlpManager_002, TestSize.Level1)
{
    manager_->AddDlpManager(abilityRecord_);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDlpManager
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDlpManager
 * EnvConditions: NA
 * CaseDescription: Verify AddDlpManager
 */
HWTEST_F(ConnectionStateManagerTest, AddDlpManager_003, TestSize.Level1)
{
    auto abilityRecord = abilityRecord_;
    abilityRecord->ownerMissionUserId_ = 1;
    manager_->dlpItems_[abilityRecord->ownerMissionUserId_] = nullptr;
    manager_->AddDlpManager(abilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveDlpManager
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveDlpManager
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDlpManager
 */
HWTEST_F(ConnectionStateManagerTest, RemoveDlpManager_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    manager_->RemoveDlpManager(abilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveDlpManager
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveDlpManager
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDlpManager
 */
HWTEST_F(ConnectionStateManagerTest, RemoveDlpManager_002, TestSize.Level1)
{
    manager_->RemoveDlpManager(abilityRecord_);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDlpAbility
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDlpAbility
 * EnvConditions: NA
 * CaseDescription: Verify AddDlpAbility
 */
HWTEST_F(ConnectionStateManagerTest, AddDlpAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    manager_->AddDlpAbility(abilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDlpAbility
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDlpAbility
 * EnvConditions: NA
 * CaseDescription: Verify AddDlpAbility
 */
HWTEST_F(ConnectionStateManagerTest, AddDlpAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    manager_->Init();
    manager_->AddDlpAbility(abilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveDlpAbility
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveDlpAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDlpAbility
 */
HWTEST_F(ConnectionStateManagerTest, RemoveDlpAbility_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    manager_->RemoveDlpAbility(abilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveDlpAbility
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveDlpAbility
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDlpAbility
 */
HWTEST_F(ConnectionStateManagerTest, RemoveDlpAbility_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    manager_->Init();
    manager_->RemoveDlpAbility(abilityRecord);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleAppDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleAppDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleAppDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleAppDied_001, TestSize.Level1)
{
    int32_t pid = 0;
    manager_->HandleAppDied(pid);
}

/*
 * Feature: ConnectionStateManager
 * Function: GetDlpConnectionInfos
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager GetDlpConnectionInfos
 * EnvConditions: NA
 * CaseDescription: Verify GetDlpConnectionInfos
 */
HWTEST_F(ConnectionStateManagerTest, GetDlpConnectionInfos_001, TestSize.Level1)
{
    std::vector<DlpConnectionInfo> infos;
    manager_->dlpItems_[0] = nullptr;
    manager_->dlpItems_[1] = nullptr;
    manager_->GetDlpConnectionInfos(infos);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddConnectionInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddConnectionInner
 * EnvConditions: NA
 * CaseDescription: Verify AddConnectionInner
 */
HWTEST_F(ConnectionStateManagerTest, AddConnectionInner_001, TestSize.Level1)
{
    std::shared_ptr<ConnectionRecord> connectionRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback_);
    ConnectionData data;
    connectionRecord->callerPid_ = 0;
    manager_->connectionStates_[0] = nullptr;
    bool res = manager_->AddConnectionInner(connectionRecord, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveConnectionInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveConnectionInner
 * EnvConditions: NA
 * CaseDescription: Verify RemoveConnectionInner
 */
HWTEST_F(ConnectionStateManagerTest, RemoveConnectionInner_001, TestSize.Level1)
{
    std::shared_ptr<ConnectionRecord> connectionRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback_);
    ConnectionData data;
    connectionRecord->callerPid_ = 0;
    manager_->connectionStates_[0] = nullptr;
    bool res = manager_->RemoveConnectionInner(connectionRecord, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleCallerDied
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleCallerDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleCallerDied
 */
HWTEST_F(ConnectionStateManagerTest, HandleCallerDied_001, TestSize.Level1)
{
    std::shared_ptr<ConnectionRecord> connectionRecord =
        ConnectionRecord::CreateConnectionRecord(abilityRecord_->GetToken(), abilityRecord_, callback_);
    int32_t callerUid = 0;
    int32_t callerPid = 0;
    std::string callerName = "callerName";
    manager_->connectionStates_[0] = std::make_shared<ConnectionStateItem>(callerUid, callerPid, callerName);
    manager_->HandleCallerDied(callerPid);
}

/*
 * Feature: ConnectionStateManager
 * Function: AddDataAbilityConnectionInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager AddDataAbilityConnectionInner
 * EnvConditions: NA
 * CaseDescription: Verify AddDataAbilityConnectionInner
 */
HWTEST_F(ConnectionStateManagerTest, AddDataAbilityConnectionInner_001, TestSize.Level1)
{
    DataAbilityCaller caller;
    ConnectionData data;
    caller.callerPid = 0;
    manager_->connectionStates_[0] = nullptr;
    bool res = manager_->AddDataAbilityConnectionInner(caller, dataAbilityRecord_, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: RemoveDataAbilityConnectionInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager RemoveDataAbilityConnectionInner
 * EnvConditions: NA
 * CaseDescription: Verify RemoveDataAbilityConnectionInner
 */
HWTEST_F(ConnectionStateManagerTest, RemoveDataAbilityConnectionInner_001, TestSize.Level1)
{
    DataAbilityCaller caller;
    ConnectionData data;
    caller.callerPid = 0;
    manager_->connectionStates_[0] = nullptr;
    bool res = manager_->RemoveDataAbilityConnectionInner(caller, dataAbilityRecord_, data);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDataAbilityDiedInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDataAbilityDiedInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDataAbilityDiedInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDataAbilityDiedInner_001, TestSize.Level1)
{
    sptr<IRemoteObject> abilityToken;
    std::vector<AbilityRuntime::ConnectionData> allData;
    manager_->connectionStates_[0] = nullptr;
    manager_->HandleDataAbilityDiedInner(abilityToken, allData);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDlpAbilityInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDlpAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDlpAbilityInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDlpAbilityInner_001, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpAbility = nullptr;
    bool isAdd = true;
    DlpStateData dlpData;
    bool res = manager_->HandleDlpAbilityInner(dlpAbility, isAdd, dlpData);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDlpAbilityInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDlpAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDlpAbilityInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDlpAbilityInner_002, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpAbility = abilityRecord_;
    bool isAdd = true;
    DlpStateData dlpData;
    dlpAbility->appIndex_ = 0;
    bool res = manager_->HandleDlpAbilityInner(dlpAbility, isAdd, dlpData);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDlpAbilityInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDlpAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDlpAbilityInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDlpAbilityInner_003, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpAbility = abilityRecord_;
    bool isAdd = true;
    DlpStateData dlpData;
    dlpAbility->appIndex_ = 1;
    bool res = manager_->HandleDlpAbilityInner(dlpAbility, isAdd, dlpData);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDlpAbilityInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDlpAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDlpAbilityInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDlpAbilityInner_004, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpAbility = abilityRecord_;
    bool isAdd = true;
    DlpStateData dlpData;
    dlpAbility->appIndex_ = 1;
    manager_->dlpItems_[1] = nullptr;
    bool res = manager_->HandleDlpAbilityInner(dlpAbility, isAdd, dlpData);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDlpAbilityInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDlpAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDlpAbilityInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDlpAbilityInner_005, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpAbility = abilityRecord_;
    bool isAdd = false;
    DlpStateData dlpData;
    int32_t dlpUid = 0;
    int32_t dlpPid = 0;
    std::shared_ptr<DlpStateItem> item = std::make_shared<DlpStateItem>(dlpUid, dlpPid);
    dlpAbility->appIndex_ = 1;
    manager_->dlpItems_[1] = item;
    bool res = manager_->HandleDlpAbilityInner(dlpAbility, isAdd, dlpData);
    EXPECT_FALSE(res);
}

/*
 * Feature: ConnectionStateManager
 * Function: HandleDlpAbilityInner
 * SubFunction: NA
 * FunctionPoints: ConnectionStateManager HandleDlpAbilityInner
 * EnvConditions: NA
 * CaseDescription: Verify HandleDlpAbilityInner
 */
HWTEST_F(ConnectionStateManagerTest, HandleDlpAbilityInner_006, TestSize.Level1)
{
    std::shared_ptr<AbilityRecord> dlpAbility = abilityRecord_;
    bool isAdd = true;
    DlpStateData dlpData;
    int32_t dlpUid = 0;
    int32_t dlpPid = 0;
    std::shared_ptr<DlpStateItem> item = std::make_shared<DlpStateItem>(dlpUid, dlpPid);
    dlpAbility->appIndex_ = 1;
    manager_->dlpItems_[1] = item;
    bool res = manager_->HandleDlpAbilityInner(dlpAbility, isAdd, dlpData);
    EXPECT_FALSE(res);
}
}  // namespace AAFwk
}  // namespace OHOS
