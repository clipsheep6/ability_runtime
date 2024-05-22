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
#include <gtest/gtest.h>
#include <memory>
#include <algorithm>
#include <functional>
#include "uri.h"
#define private public
#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_errors.h"
#include "dataobs_mgr_inner.h"
#include "mock_data_ability_observer_stub.h"

using namespace OHOS;
using namespace testing::ext;
using namespace testing;


namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;
using ObsListType = std::list<sptr<IDataAbilityObserver>>;
using ObsRecipientMapType = OHOS::AAFwk::DataObsMgrInner::ObsRecipientMapType;
class DataObsMgrInnerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner_ = nullptr;
};
void DataObsMgrInnerTest::SetUpTestCase(void) {}
void DataObsMgrInnerTest::TearDownTestCase(void) {}
void DataObsMgrInnerTest::SetUp()
{
    dataObsMgrInner_ = std::make_shared<DataObsMgrInner>();
}
void DataObsMgrInnerTest::TearDown() {}

/*
 * Feature: DataObsMgrInner
 * Function: Register and unregister function test
 * SubFunction: HandleRegisterObserver/HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleRegisterObserver_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }

    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInner_->HandleRegisterObserver(uri, callback);

    EXPECT_EQ(dataObsMgrInner_->HaveRegistered(callback), true);
    dataObsMgrInner_->HandleUnregisterObserver(uri, callback);
    EXPECT_EQ(dataObsMgrInner_->HaveRegistered(callback), false);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register function test
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleRegisterObserver_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(callback);
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleRegisterObserver(uri, callback);
    EXPECT_EQ(res, OBS_EXIST);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register function test
 * SubFunction: HandleRegisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleRegisterObserver_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    const sptr<IDataAbilityObserver> callback1(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(callback1);
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleRegisterObserver(uri, callback);
    EXPECT_EQ(res, OBS_EXIST);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleUnregisterObserver_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInner->observers_.clear();
    int res = dataObsMgrInner->HandleUnregisterObserver(uri, callback);
    EXPECT_EQ(res, NO_OBS_FOR_URI);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleUnregisterObserver_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(callback);
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleUnregisterObserver(uri, callback);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleUnregisterObserver_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    const sptr<IDataAbilityObserver> callback2(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList;
    obsList.push_back(callback);
    obsList.push_back(callback2);
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    dataObsMgrInner->observers_.emplace("exit", obsList);
    int res = dataObsMgrInner->HandleUnregisterObserver(uri, callback);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInner
 * Function: Register and unregister function test
 * SubFunction: OnChange
 * FunctionPoints: When the data changes, call the OnChange function of the registered dataabilityobserver
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleNotifyChange_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());

    EXPECT_CALL(*mockDataAbilityObserverStub, OnChange()).Times(1);

    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    dataObsMgrInner_->HandleRegisterObserver(uri, callback);
    dataObsMgrInner_->HandleNotifyChange(uri);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleNotifyChange_0200, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    dataObsMgrInner->observers_.clear();
    int res = dataObsMgrInner->HandleNotifyChange(uri);
    EXPECT_EQ(res, NO_OBS_FOR_URI);
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister function test
 * SubFunction: HandleUnregisterObserver
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_HandleNotifyChange_0300, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    ObsListType obsList;
    obsList.push_back(nullptr);
    dataObsMgrInner->observers_.emplace(uri.ToString(), obsList);
    int res = dataObsMgrInner->HandleNotifyChange(uri);
    EXPECT_EQ(res, NO_ERROR);
}

/*
 * Feature: DataObsMgrInner
 * Function: GetObsListFromMap/RemoveObs/HaveRegistered function test
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_RemoveObs_HaveRegistered_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }
    Uri uri("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub));
    dataObsMgrInner_->HandleRegisterObserver(uri, callback);

    sptr<MockDataAbilityObserverStub> mockDataAbilityObserverStub2(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback2(
        new (std::nothrow) DataAbilityObserverProxy(mockDataAbilityObserverStub2));

    dataObsMgrInner_->HandleRegisterObserver(uri, callback2);
    auto obsPair = dataObsMgrInner_->observers_.find(uri.ToString());
    EXPECT_EQ((std::size_t)2, obsPair->second.size());
    EXPECT_EQ(true, dataObsMgrInner_->HaveRegistered(callback));
    EXPECT_EQ(true, dataObsMgrInner_->HaveRegistered(callback2));

    dataObsMgrInner_->RemoveObs(callback->AsObject());
    EXPECT_EQ(false, dataObsMgrInner_->HaveRegistered(callback));
    obsPair->second.clear();
    obsPair = dataObsMgrInner_->observers_.find(uri.ToString());
    EXPECT_EQ((std::size_t)0, obsPair->second.size());
    EXPECT_EQ(false, dataObsMgrInner_->HaveRegistered(callback));

    dataObsMgrInner_->RemoveObs(callback2->AsObject());
    EXPECT_EQ(false, dataObsMgrInner_->HaveRegistered(callback2));
    obsPair->second.clear();
    obsPair = dataObsMgrInner_->observers_.find(uri.ToString());
    EXPECT_EQ((std::size_t)0, obsPair->second.size());
    EXPECT_EQ(false, dataObsMgrInner_->HaveRegistered(callback2));
}

/*
 * Feature: DataObsMgrInner
 * Function: AddObsDeathRecipient/RemoveObsDeathRecipient function test
 * SubFunction: NA
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_AddRemove_ObsDeathRecipient_0100, TestSize.Level1)
{
    if (dataObsMgrInner_ == nullptr) {
        return;
    }

    sptr<IRemoteObject> observer(new (std::nothrow) MockDataAbilityObserverStub());
    sptr<IDataAbilityObserver> callback(new (std::nothrow) DataAbilityObserverProxy(observer));
    dataObsMgrInner_->AddObsDeathRecipient(callback);
    dataObsMgrInner_->AddObsDeathRecipient(nullptr);

    ObsRecipientMapType::const_iterator it;
    it = dataObsMgrInner_->obsRecipient_.find(observer);
    EXPECT_EQ(true, it != dataObsMgrInner_->obsRecipient_.end());

    dataObsMgrInner_->RemoveObsDeathRecipient(callback->AsObject());
    dataObsMgrInner_->RemoveObsDeathRecipient(nullptr);
    it = dataObsMgrInner_->obsRecipient_.find(observer);
    EXPECT_EQ(false, it != dataObsMgrInner_->obsRecipient_.end());

    dataObsMgrInner_->obsRecipient_.clear();
    dataObsMgrInner_->RemoveObsDeathRecipient(callback->AsObject());
}

/*
 * Feature: DataObsMgrInner
 * Function: Unregister function test
 * SubFunction: RemoveObs
 * FunctionPoints: NA
 * EnvConditions: NA
 * CaseDescription:NA
 */
HWTEST_F(DataObsMgrInnerTest, DataObsMgrInner_RemoveObs_0100, TestSize.Level1)
{
    std::shared_ptr<DataObsMgrInner> dataObsMgrInner = std::make_shared<DataObsMgrInner>();
    ASSERT_NE(dataObsMgrInner, nullptr);
    std::string uri1 = "uri1";
    std::string uri2 = "uri2";
    sptr<MockDataAbilityObserverStub> observer(new (std::nothrow) MockDataAbilityObserverStub());
    const sptr<IDataAbilityObserver> callback1(new (std::nothrow) DataAbilityObserverProxy(observer));
    const sptr<IDataAbilityObserver> callback2(new (std::nothrow) DataAbilityObserverProxy(observer));
    ObsListType obsList1;
    ObsListType obsList2;
    ObsListType obsList3;
    obsList1.push_back(callback1);
    obsList2.push_back(callback1);
    obsList2.push_back(callback2);
    dataObsMgrInner->observers_.emplace(uri1, obsList1);
    dataObsMgrInner->observers_.emplace(uri2, obsList2);
    dataObsMgrInner->RemoveObs(callback2->AsObject());
}
}  // namespace AAFwk
}  // namespace OHOS
