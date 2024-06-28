/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <memory>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_data_ability_observer_stub.h"
#define private public
#include "dataobs_mgr_service.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
class DataObsMgrServiceTest : public testing::Test {
public:
    DataObsMgrServiceTest() = default;
    virtual ~DataObsMgrServiceTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DataObsMgrServiceTest::SetUpTestCase(void)
{}
void DataObsMgrServiceTest::TearDownTestCase(void)
{}
void DataObsMgrServiceTest::SetUp()
{}
void DataObsMgrServiceTest::TearDown()
{}

/*
 * Feature: DataObsMgrService
 * Function: QueryServiceState
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService QueryServiceState
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService could query service state.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_QueryServiceState_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_QueryServiceState_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_NOT_START;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_QueryServiceState_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: OnStart
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService OnStart
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService OnStart is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStart_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_RUNNING;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStart();
    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStart_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0200 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, nullptr));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_RegisterObserver_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();

    testing::Mock::AllowLeak(dataobsAbility);
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_RegisterObserver_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserver(*uri, dataobsAbility));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0200 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, nullptr));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_UnregisterObserver_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserver(*uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_UnregisterObserver_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0200 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->OnStart();

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChange is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_NotifyChange_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0400 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_TASK_LIMMIT);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->taskCount_ = 50;
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChange(*uri));
    dataObsMgrServer->taskCount_ = 0;

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_NotifyChange_0400 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_RegisterObserverExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, false));
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_RegisterObserverExt_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0200 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, nullptr, true));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService RegisterObserver is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_RegisterObserverExt_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInnerExt_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));
    dataObsMgrServer->dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_RegisterObserverExt_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(uri, dataobsAbility));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0200 start";
    const int testVal = static_cast<int>(NO_ERROR);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->RegisterObserverExt(uri, dataobsAbility, true));
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(dataobsAbility));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0300 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(uri, nullptr));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0400 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInnerExt_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(uri, dataobsAbility));
    dataObsMgrServer->dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0400 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0500 start";
    const int testVal = static_cast<int>(DATA_OBSERVER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(nullptr));

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0500 end";
}

/*
 * Feature: DataObsMgrService
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService UnregisterObserverExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_UnregisterObserverExt_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0600 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    const sptr<MockDataAbilityObserverStub> dataobsAbility(new (std::nothrow) MockDataAbilityObserverStub());
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInnerExt_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->UnregisterObserverExt(dataobsAbility));
    dataObsMgrServer->dataObsMgrInnerExt_ = std::make_shared<DataObsMgrInnerExt>();

    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_UnregisterObserverExt_0600 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is normal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_NotifyChangeExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0100 start";
    const int testVal = static_cast<int>(NO_ERROR);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_NotifyChangeExt_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0200 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_HANDLER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    dataObsMgrServer->OnStart();
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0200 end";
}

/*
 * Feature: DataObsMgrService
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService NotifyChangeExt is abnormal.
 */
HWTEST_F(DataObsMgrServiceTest, DataObsMgrServiceTest_NotifyChangeExt_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0300 start";
    const int testVal = static_cast<int>(DATAOBS_SERVICE_INNER_IS_NULL);
    Uri uri("dataobs://authority/com.domainname.dataability.persondata/ person/10");
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->dataObsMgrInner_.reset();
    EXPECT_EQ(testVal, dataObsMgrServer->NotifyChangeExt({ ChangeInfo::ChangeType::UPDATE, { uri } }));
    dataObsMgrServer->dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
    GTEST_LOG_(INFO) << "DataObsMgrServiceTest_NotifyChangeExt_0300 end";
}

/*
 * Feature: DataObsMgrService
 * Function: OnStop
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService OnStop
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService OnStop is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_OnStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStop_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_NOT_START;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    dataObsMgrServer->OnStop();
    EXPECT_EQ(testValue, dataObsMgrServer->QueryServiceState());

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_OnStop_0100 end";
}

/*
 * Feature: DataObsMgrService
 * Function: Dump
 * SubFunction: NA
 * FunctionPoints: DataObsMgrService Dump
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsMgrService Dump is normal.
 */
HWTEST_F(DataObsMgrServiceTest, AaFwk_DataObsMgrServiceTest_Dump_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_Dump_0100 start";
    const DataObsServiceRunningState testValue = DataObsServiceRunningState::STATE_RUNNING;
    auto dataObsMgrServer = DelayedSingleton<DataObsMgrService>::GetInstance();

    std::string fileName = "test.txt";
    std::vector<std::u16string> args;
    args.push_back(u"-h");
    FILE *fp = fopen(fileName.c_str(), "w");
    int ret = dataObsMgrServer->Dump(fileno(fp), args);
    fclose(fp);
    EXPECT_EQ(SUCCESS, ret);

    GTEST_LOG_(INFO) << "AaFwk_DataObsMgrServiceTest_Dump_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS
