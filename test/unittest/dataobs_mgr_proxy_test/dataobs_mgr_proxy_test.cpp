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

#include "mock_data_obs_mgr_stub.h"

#include "dataobs_mgr_proxy.h"

namespace OHOS {
namespace AAFwk {
using namespace testing::ext;
using ::testing::_;

class DataObsMgrProxyTest : public testing::Test {
public:
    DataObsMgrProxyTest() = default;
    virtual ~DataObsMgrProxyTest() = default;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DataObsMgrProxyTest::SetUpTestCase(void)
{}
void DataObsMgrProxyTest::TearDownTestCase(void)
{}
void DataObsMgrProxyTest::SetUp()
{}
void DataObsMgrProxyTest::TearDown()
{}

/*
 * Feature: DataObsManagerProxy
 * Function: RegisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsManagerProxy RegisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerProxy RegisterObserver is normal.
 */
HWTEST_F(DataObsMgrProxyTest, DataObsMgrProxyTest_RegisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_RegisterObserver_0100 start";
    const int32_t testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());

    int32_t retVal = dataObsManagerProxy->RegisterObserver(*uri, dataObserver);
    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_RegisterObserver_0100 end";
}

/*
 * Feature: DataObsManagerProxy
 * Function: UnregisterObserver
 * SubFunction: NA
 * FunctionPoints: DataObsManagerProxy UnregisterObserver
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerProxy UnregisterObserver is normal.
 */
HWTEST_F(DataObsMgrProxyTest, DataObsMgrProxyTest_UnregisterObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_UnregisterObserver_0100 start";
    const int32_t testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());

    int32_t retVal = dataObsManagerProxy->UnregisterObserver(*uri, dataObserver);
    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_UnregisterObserver_0100 end";
}

/*
 * Feature: DataObsManagerProxy
 * Function: NotifyChange
 * SubFunction: NA
 * FunctionPoints: DataObsManagerProxy NotifyChange
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerProxy NotifyChange is normal.
 */
HWTEST_F(DataObsMgrProxyTest, DataObsMgrProxyTest_NotifyChange_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_NotifyChange_0100 start";
    const int32_t testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);

    int32_t retVal = dataObsManagerProxy->NotifyChange(*uri);
    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_NotifyChange_0100 end";
}

/*
 * Feature: DataObsManagerProxy
 * Function: RegisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsManagerProxy RegisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerProxy RegisterObserverExt is normal.
 */
HWTEST_F(DataObsMgrProxyTest, DataObsMgrProxyTest_RegisterObserverExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_RegisterObserverExt_0100 start";
    const int32_t testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("datashare://Authority/com.domainname.persondata");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());

    int32_t retVal = dataObsManagerProxy->RegisterObserverExt(*uri, dataObserver, true);
    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_RegisterObserverExt_0100 end";
}

/*
 * Feature: DataObsManagerProxy
 * Function: UnregisterObserverExt
 * SubFunction: NA
 * FunctionPoints: DataObsManagerProxy UnregisterObserverExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerProxy UnregisterObserverExt is normal.
 */
HWTEST_F(DataObsMgrProxyTest, DataObsMgrProxyTest_UnregisterObserverExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_UnregisterObserverExt_0100 start";
    const int32_t testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<MockDataObsMgrStub> dataobs = std::make_shared<MockDataObsMgrStub>();
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("datashare://Authority/com.domainname.persondata");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);
    sptr<AAFwk::IDataAbilityObserver> dataObserver(new (std::nothrow) MockDataAbilityObserverStub());

    int32_t retVal = dataObsManagerProxy->UnregisterObserverExt(*uri, dataObserver);
    EXPECT_EQ(testVal, retVal);
    retVal = dataObsManagerProxy->UnregisterObserverExt(dataObserver);
    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_UnregisterObserverExt_0100 end";
}

/*
 * Feature: DataObsManagerProxy
 * Function: NotifyChangeExt
 * SubFunction: NA
 * FunctionPoints: DataObsManagerProxy NotifyChangeExt
 * EnvConditions: NA
 * CaseDescription: Verify that the DataObsManagerProxy NotifyChangeExt is normal.
 */
HWTEST_F(DataObsMgrProxyTest, DataObsMgrProxyTest_NotifyChangeExt_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_NotifyChangeExt_0100 start";
    const int32_t testVal = static_cast<int>(TEST_RETVAL_ONREMOTEREQUEST);
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("datashare://Authority/com.domainname.persondata");
    sptr<MockDataObsMgrStub> mockDataobsMgrStub(new (std::nothrow) MockDataObsMgrStub());
    std::shared_ptr<DataObsManagerProxy> dataObsManagerProxy =
        std::make_shared<DataObsManagerProxy>(mockDataobsMgrStub);

    int32_t retVal = dataObsManagerProxy->NotifyChangeExt({ ChangeInfo::ChangeType::INSERT, { *uri } });
    EXPECT_EQ(testVal, retVal);

    GTEST_LOG_(INFO) << "DataObsMgrProxyTest_NotifyChangeExt_0100 end";
}
}  // namespace AAFwk
}  // namespace OHOS
