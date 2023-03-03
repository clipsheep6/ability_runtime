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
#include "gtest/gtest.h"
#include <memory>
#define private public
#include "dataobs_mgr_proxy.h"
#include "dataobs_mgr_client.h"
#include "mock_data_obs_manager_onchange_callback.h"
#include "mock_dataobs_mgr_service.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class DataObsMgrClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataObsMgrClientTest::SetUpTestCase(void)
{}
void DataObsMgrClientTest::TearDownTestCase(void)
{}
void DataObsMgrClientTest::SetUp()
{}
void DataObsMgrClientTest::TearDown()
{}

/*
 * Feature: DataObsMgrClient.
 * Function: The function of dataobsmgrservice was called.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_Call_Service_0100, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);
    Uri uri("datashare://device_id/com.domainname.dataability.persondata/person/25");
    client->RegisterObserver(uri, callBack);
    EXPECT_EQ(service->onChangeCall_, 1);

    client->UnregisterObserver(uri, callBack);
    EXPECT_EQ(service->onChangeCall_, 2);

    client->NotifyChange(uri);
    EXPECT_EQ(service->onChangeCall_, 3);

    client->RegisterObserverExt(uri, callBack, false);
    EXPECT_EQ(service->onChangeCall_, 4);

    client->UnregisterObserverExt(uri, callBack);
    EXPECT_EQ(service->onChangeCall_, 5);

    client->UnregisterObserverExt(callBack);
    EXPECT_EQ(service->onChangeCall_, 6);

    client->NotifyChangeExt({ ChangeInfo::ChangeType::INSERT, { uri } });
    EXPECT_EQ(service->onChangeCall_, 7);

    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: re-subscribe when service restart.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_ReregisterObserver_0100, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> callBack2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service1(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service1;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);
    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/25");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/26");

    EXPECT_EQ(client->RegisterObserver(uri1, callBack1), NO_ERROR);
    EXPECT_EQ(client->RegisterObserver(uri2, callBack2), NO_ERROR);
    EXPECT_EQ(service1->onChangeCall_, 2);

    sptr<MockDataObsMgrService> service2(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service2;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    client->ReRegister();
    EXPECT_EQ(service2->onChangeCall_, 2);
    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

/*
 * Feature: DataObsMgrClient.
 * Function: re-subscribe when service restart.
 * SubFunction: NA.
 * FunctionPoints: NA.
 * EnvConditions: NA.
 * CaseDescription: NA.
 */
HWTEST_F(DataObsMgrClientTest, DataObsMgrClient_ReregisterObserver_0200, TestSize.Level1)
{
    sptr<MockDataObsManagerOnChangeCallBack> callBack1(new (std::nothrow) MockDataObsManagerOnChangeCallBack());
    sptr<MockDataObsManagerOnChangeCallBack> callBack2(new (std::nothrow) MockDataObsManagerOnChangeCallBack());

    Uri uri1("datashare://device_id/com.domainname.dataability.persondata/person/1");
    Uri uri2("datashare://device_id/com.domainname.dataability.persondata/person/2");
    auto client = DataObsMgrClient::GetInstance();
    client->observers_.Clear();
    client->observerExts_.Clear();

    sptr<MockDataObsMgrService> service1(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service1;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    EXPECT_EQ(client->RegisterObserverExt(uri1, callBack1, false), SUCCESS);
    EXPECT_EQ(client->RegisterObserverExt(uri2, callBack2, true), SUCCESS);
    EXPECT_EQ(service1->onChangeCall_, 2);

    sptr<MockDataObsMgrService> service2(new (std::nothrow) MockDataObsMgrService());
    client->dataObsManger_ = service2;
    EXPECT_TRUE(client->dataObsManger_ != nullptr);

    client->ReRegister();
    EXPECT_EQ(service2->onChangeCall_, 2);
    testing::Mock::AllowLeak(DataObsMgrClient::GetInstance()->dataObsManger_);
}

}  // namespace AAFwk
}  // namespace OHOS
