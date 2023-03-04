/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "mock_ability_manager_client_for_data_ability_observer.h"
#include "mock_ability_scheduler_for_observer.h"
#include "mock_ability_manager_client.h"
#include "context.h"
#include "ability_connect_manager.h"
#include "ability_context.h"
#include "data_ability_helper.h"

#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "values_bucket.h"
#include "data_ability_observer_interface.h"
#include "datashare_helper.h"
#include "mock_ability_runtime_context.h"
#include "session_info.h"
#undef private
#undef protected

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;

class DataAbilityHelperForObserverTest : public testing::Test {
public:
    DataAbilityHelperForObserverTest()
    {}
    virtual ~DataAbilityHelperForObserverTest()
    {}

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataAbilityHelperForObserverTest::SetUpTestCase(void)
{}

void DataAbilityHelperForObserverTest::TearDownTestCase(void)
{
    MockAbilitySchedulerTools::GetInstance()->SetMockStatus(false);
}

void DataAbilityHelperForObserverTest::SetUp(void)
{}

void DataAbilityHelperForObserverTest::TearDown(void)
{
    MockAbilitySchedulerTools::DestoryInstance();
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_0100
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetFileTypes_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    // Test to AbilityThread interface
    auto returnGetFileTypes = [&](const Uri &uri, const std::string &mimeTypeFilter) {
        std::vector<std::string> matchedMIMEs;
        matchedMIMEs.push_back("test1");
        matchedMIMEs.push_back("test2");
        matchedMIMEs.push_back("test3");
        return matchedMIMEs;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetFileTypes(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetFileTypes));

    dataAbilityHelper->GetFileTypes(*uri, mimeTypeFilter);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_0200
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetFileTypes_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    // Test to AbilityThread interface
    auto returnGetFileTypes = [&](const Uri &uri, const std::string &mimeTypeFilter) {
        std::vector<std::string> matchedMIMEs;
        matchedMIMEs.push_back("test1");
        matchedMIMEs.push_back("test2");
        matchedMIMEs.push_back("test3");
        return matchedMIMEs;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetFileTypes(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetFileTypes));

    dataAbilityHelper->GetFileTypes(*uri, mimeTypeFilter);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_0300
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetFileTypes_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnGetFileTypes = [&](const Uri &uri, const std::string &mimeTypeFilter) {
        std::vector<std::string> matchedMIMEs;
        matchedMIMEs.push_back("test1");
        matchedMIMEs.push_back("test2");
        matchedMIMEs.push_back("test3");
        return matchedMIMEs;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetFileTypes(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetFileTypes));

    dataAbilityHelper->GetFileTypes(*uri, mimeTypeFilter);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetFileTypes_0400
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetFileTypes_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0400 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mimeTypeFilter("mimeTypeFiltertest");
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    auto ret = dataAbilityHelper->GetFileTypes(*uri, mimeTypeFilter);
    EXPECT_TRUE(ret.empty());

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetFileTypes_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0100
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string mode("modetest");
    // Test to AbilityThread interface
    auto returnOpenFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenFile));

    dataAbilityHelper->OpenFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0200
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mode("modetest");
    // Test to AbilityThread interface
    auto returnOpenFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenFile));

    dataAbilityHelper->OpenFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0300
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mode("modetest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    dataAbilityHelper->callFromJs_ = true;
    // Test to AbilityThread interface
    auto returnOpenFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenFile));

    dataAbilityHelper->OpenFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0400
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0400 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mode("modetest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    dataAbilityHelper->callFromJs_ = false;
    // Test to AbilityThread interface
    auto returnOpenFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenFile));

    dataAbilityHelper->OpenFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenFile_0500
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenFile_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0500 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string mode("modetest");
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;

    EXPECT_EQ(dataAbilityHelper->OpenFile(*uri, mode), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenFile_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_0100
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Insert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    // Test to AbilityThread interface
    auto returnInsert = [&](const Uri &uri, const NativeRdb::ValuesBucket &val) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Insert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnInsert));

    dataAbilityHelper->Insert(*uri, val);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_0200
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Insert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    // Test to AbilityThread interface
    auto returnInsert = [&](const Uri &uri, const NativeRdb::ValuesBucket &val) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Insert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnInsert));

    dataAbilityHelper->Insert(*uri, val);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_0300
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Insert_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    // Test to AbilityThread interface
    auto returnInsert = [&](const Uri &uri, const NativeRdb::ValuesBucket &val) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Insert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnInsert));

    dataAbilityHelper->Insert(*uri, val);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Insert_0400
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Insert_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);

    EXPECT_EQ(dataAbilityHelper->Insert(*uri, val), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Insert_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_0100
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Update_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnUpdate = [&](
        const Uri &uri, const NativeRdb::ValuesBucket &val, const NativeRdb::DataAbilityPredicates &predicates) {
            int index = 1234;
            return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Update(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnUpdate));

    dataAbilityHelper->Update(*uri, val, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_0200
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Update_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnUpdate = [&](
        const Uri &uri, const NativeRdb::ValuesBucket &val, const NativeRdb::DataAbilityPredicates &predicates) {
            int index = 1234;
            return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Update(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnUpdate));

    dataAbilityHelper->Update(*uri, val, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_0300
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Update_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnUpdate = [&](
        const Uri &uri, const NativeRdb::ValuesBucket &val, const NativeRdb::DataAbilityPredicates &predicates) {
            int index = 1234;
            return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Update(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnUpdate));

    dataAbilityHelper->Update(*uri, val, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Update_0400
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Update_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0400 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    NativeRdb::ValuesBucket val;
    val.PutInt("valtest", 20);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");

    EXPECT_EQ(dataAbilityHelper->Update(*uri, val, predicates), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Update_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_0100
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Delete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnDelete = [&](const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Delete(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDelete));

    dataAbilityHelper->Delete(*uri, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_0200
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Delete_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnDelete = [&](const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates) {
        int index = 1234;
        return index;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Delete(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDelete));

    dataAbilityHelper->Delete(*uri, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_0300
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Delete_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0300 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;

    EXPECT_EQ(dataAbilityHelper->Delete(*uri, predicates), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Delete_0400
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Delete_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;

    EXPECT_EQ(dataAbilityHelper->Delete(*uri, predicates), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Delete_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0100
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnQuery =
        [&](const Uri &uri, const std::vector<std::string> &columns,
            const NativeRdb::DataAbilityPredicates &predicates) {
                std::shared_ptr<NativeRdb::AbsSharedResultSet> set = nullptr;
                return set;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Query(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnQuery));

    dataAbilityHelper->Query(*uri, columns, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0200
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    // Test to AbilityThread interface
    auto returnQuery =
        [&](const Uri &uri, const std::vector<std::string> &columns,
            const NativeRdb::DataAbilityPredicates &predicates) {
                std::shared_ptr<NativeRdb::AbsSharedResultSet> set = nullptr;
                return set;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Query(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnQuery));

    dataAbilityHelper->Query(*uri, columns, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0300
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnQuery =
        [&](const Uri &uri, const std::vector<std::string> &columns,
            const NativeRdb::DataAbilityPredicates &predicates) {
                std::shared_ptr<NativeRdb::AbsSharedResultSet> set = nullptr;
                return set;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Query(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnQuery));

    dataAbilityHelper->Query(*uri, columns, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0400
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0400 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnQuery =
        [&](const Uri &uri, const std::vector<std::string> &columns,
            const NativeRdb::DataAbilityPredicates &predicates) {
                std::shared_ptr<NativeRdb::AbsSharedResultSet> set = nullptr;
                return set;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Query(testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnQuery));

    dataAbilityHelper->Query(*uri, columns, predicates);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Query_0500
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Query_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0500 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::vector<std::string> columns;
    NativeRdb::DataAbilityPredicates predicates("predicatestest");
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;

    EXPECT_TRUE(dataAbilityHelper->Query(*uri, columns, predicates) == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Query_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    auto returnGetType = [&](const Uri &uri) {
        std::string type("Type1");
        return type;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetType(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetType));

    dataAbilityHelper->GetType(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_0200
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetType_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    auto returnGetType = [&](const Uri &uri) {
        std::string type("Type1");
        return type;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetType(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetType));

    dataAbilityHelper->GetType(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_0300
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetType_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnGetType = [&](const Uri &uri) {
        std::string type("Type1");
        return type;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), GetType(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnGetType));

    dataAbilityHelper->GetType(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_GetType_0400
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_GetType_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;

    EXPECT_TRUE(dataAbilityHelper->GetType(*uri).empty());

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_GetType_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_0100
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenRawFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    std::string mode("modetest");
    auto returnOpenRawFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenRawFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenRawFile));

    dataAbilityHelper->OpenRawFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_0200
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenRawFile_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    std::string mode("modetest");
    auto returnOpenRawFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenRawFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenRawFile));

    dataAbilityHelper->OpenRawFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_0300
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenRawFile_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    std::string mode("modetest");
    auto returnOpenRawFile = [&](const Uri &uri, const std::string &mode) {
        int fd = 1234;
        return fd;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), OpenRawFile(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnOpenRawFile));

    dataAbilityHelper->OpenRawFile(*uri, mode);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_OpenRawFile_0400
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_OpenRawFile_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0400 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    std::string mode("modetest");

    EXPECT_EQ(dataAbilityHelper->OpenRawFile(*uri, mode), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_OpenRawFile_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_0100
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Reload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    PacMap extras;
    auto returnReload = [&](const Uri &uri, const PacMap &extras) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Reload(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnReload));

    dataAbilityHelper->Reload(*uri, extras);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_0200
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Reload_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    PacMap extras;
    auto returnReload = [&](const Uri &uri, const PacMap &extras) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Reload(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnReload));

    dataAbilityHelper->Reload(*uri, extras);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_0300
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Reload_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    PacMap extras;
    auto returnReload = [&](const Uri &uri, const PacMap &extras) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), Reload(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnReload));

    dataAbilityHelper->Reload(*uri, extras);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Reload_0400
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Reload_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    PacMap extras;

    EXPECT_FALSE(dataAbilityHelper->Reload(*uri, extras));

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Reload_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_0100
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_BatchInsert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    std::vector<NativeRdb::ValuesBucket> values;
    auto returnBatchInsert = [&](const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), BatchInsert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnBatchInsert));

    dataAbilityHelper->BatchInsert(*uri, values);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_0200
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_BatchInsert_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    std::vector<NativeRdb::ValuesBucket> values;
    auto returnBatchInsert = [&](const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), BatchInsert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnBatchInsert));

    dataAbilityHelper->BatchInsert(*uri, values);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_0300
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_BatchInsert_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);;
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    std::vector<NativeRdb::ValuesBucket> values;
    NativeRdb::ValuesBucket value1;
    value1.PutInt("value1", 1);
    NativeRdb::ValuesBucket value2;
    value2.PutInt("value2", 2);
    NativeRdb::ValuesBucket value3;
    value3.PutInt("value3", 3);
    values.push_back(value1);
    values.push_back(value2);
    values.push_back(value3);
    auto returnBatchInsert = [&](const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values) { return true; };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), BatchInsert(testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnBatchInsert));

    dataAbilityHelper->BatchInsert(*uri, values);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_BatchInsert_0400
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_BatchInsert_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    // Test to AbilityThread interface
    std::vector<NativeRdb::ValuesBucket> values;

    EXPECT_EQ(dataAbilityHelper->BatchInsert(*uri, values), -1);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_BatchInsert_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_0100
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    auto returnNormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), NormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnNormalizeUri));

    dataAbilityHelper->NormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_0200
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NormalizeUri_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    auto returnNormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), NormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnNormalizeUri));

    dataAbilityHelper->NormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_0300
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NormalizeUri_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnNormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), NormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnNormalizeUri));

    dataAbilityHelper->NormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NormalizeUri_0400
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NormalizeUri_0400, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    Uri urivalue("");

    EXPECT_TRUE(urivalue.Equals(dataAbilityHelper->NormalizeUri(*uri)));

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_NormalizeUri_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_0100
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DenormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0100 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    // Test to AbilityThread interface
    auto returnDenormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), DenormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDenormalizeUri));

    dataAbilityHelper->DenormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_0200
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DenormalizeUri_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0200 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    // Test to AbilityThread interface
    auto returnDenormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), DenormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDenormalizeUri));

    dataAbilityHelper->DenormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_0300
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DenormalizeUri_0300, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0300 start";

    std::shared_ptr<MockAbilitySchedulerTools> mockTools = MockAbilitySchedulerTools::GetInstance();
    mockTools->SetMockStatus(true);
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    // Test to AbilityThread interface
    auto returnDenormalizeUri = [&](const Uri &uri) {
        Uri uriValue("dataability://device_id/com.domainname.dataability.");
        return uriValue;
    };
    EXPECT_CALL(*mockTools->GetMockAbilityScheduler(), DenormalizeUri(testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(returnDenormalizeUri));

    dataAbilityHelper->DenormalizeUri(*uri);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DenormalizeUri_0400
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DenormalizeUri_0400, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0400 start";

    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    Uri urivalue("");

    EXPECT_TRUE(urivalue.Equals(dataAbilityHelper->DenormalizeUri(*uri)));

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DenormalizeUri_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_SetCallFromJs_0100
 * @tc.name: SetCallFromJs
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_SetCallFromJs_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_SetCallFromJs_0100 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
    std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper->callFromJs_ == false);
    dataAbilityHelper->SetCallFromJs();
    EXPECT_TRUE(dataAbilityHelper->callFromJs_ == true);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_SetCallFromJs_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Release_0100
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Release_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0100 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    bool ret = false;
    EXPECT_TRUE(!ret);
    ret = dataAbilityHelper->Release();
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Release_0200
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Release_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0200 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    bool ret = false;
    EXPECT_TRUE(!ret);
    ret = dataAbilityHelper->Release();
    EXPECT_TRUE(!ret);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Release_0300
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Release_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0300 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    bool ret = false;
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ != nullptr);
    EXPECT_TRUE(!ret);
    ret = dataAbilityHelper->Release();
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Release_0400
 * @tc.name: Release
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Release_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0400 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    bool ret = true;
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    EXPECT_TRUE(ret);
    ret = dataAbilityHelper->Release();
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Release_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_RegisterObserver_0100
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_RegisterObserver_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string uriString;
    Uri uri1(uriString);
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    dataAbilityHelper->RegisterObserver(uri1, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_RegisterObserver_0200
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_RegisterObserver_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string uriString;
    Uri uri1(uriString);
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    dataAbilityHelper->RegisterObserver(uri1, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_RegisterObserver_0300
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_RegisterObserver_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    dataAbilityHelper->RegisterObserver(*uri, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ != nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_RegisterObserver_0400
 * @tc.name: RegisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_RegisterObserver_0400, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    dataAbilityHelper->RegisterObserver(*uri, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataAbilityHelperImpl_ == nullptr);
    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_UnregisterObserver_0100
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_UnregisterObserver_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    const sptr<AAFwk::IDataAbilityObserver> dataObserver;
    std::string uriString;
    Uri uri1(uriString);
    dataAbilityHelper->UnregisterObserver(uri1, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_UnregisterObserver_0200
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_UnregisterObserver_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    sptr<AAFwk::IDataAbilityObserver> dataObserver;
    std::string uriString;
    Uri uri1(uriString);
    dataAbilityHelper->UnregisterObserver(uri1, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_UnregisterObserver_0300
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_UnregisterObserver_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    sptr<AAFwk::IDataAbilityObserver> dataObserver;
    dataAbilityHelper->UnregisterObserver(*uri, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ != nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_UnregisterObserver_0400
 * @tc.name: UnregisterObserver
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_UnregisterObserver_0400, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    sptr<AAFwk::IDataAbilityObserver> dataObserver;
    dataAbilityHelper->UnregisterObserver(*uri, dataObserver);

    EXPECT_TRUE(dataAbilityHelper->dataAbilityHelperImpl_ == nullptr);
    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NotifyChange_0100
 * @tc.name: NotifyChange
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NotifyChange_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string uriString;
    Uri uri1(uriString);
    dataAbilityHelper->NotifyChange(uri1);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NotifyChange_0200
 * @tc.name: NotifyChange
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NotifyChange_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string uriString;
    Uri uri1(uriString);
    dataAbilityHelper->NotifyChange(uri1);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NotifyChange_0300
 * @tc.name: NotifyChange
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NotifyChange_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    dataAbilityHelper->NotifyChange(*uri);

    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ != nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_NotifyChange_0400
 * @tc.name: NotifyChange
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_NotifyChange_0400, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    dataAbilityHelper->NotifyChange(*uri);

    EXPECT_TRUE(dataAbilityHelper->dataAbilityHelperImpl_ == nullptr);
    EXPECT_TRUE(dataAbilityHelper->dataShareHelper_ == nullptr);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_ExecuteBatch_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string uriString;
    Uri uri1(uriString);
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    auto ret = dataAbilityHelper->ExecuteBatch(uri1, operations);

    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_ExecuteBatch_0200
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_ExecuteBatch_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string uriString;
    Uri uri1(uriString);
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    auto ret = dataAbilityHelper->ExecuteBatch(uri1, operations);

    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_ExecuteBatch_0300
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_ExecuteBatch_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    std::string uriString;
    Uri uri1(uriString);
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    auto ret = dataAbilityHelper->ExecuteBatch(uri1, operations);

    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_ExecuteBatch_0400
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_ExecuteBatch_0400, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    std::string uriString;
    Uri uri1(uriString);
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    auto ret = dataAbilityHelper->ExecuteBatch(uri1, operations);

    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_TransferScheme_0100
 * @tc.name: TransferScheme
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_TransferScheme_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    std::string uriString = "dataability";
    Uri uri1(uriString);
    Uri dataShareUri(uriString);
    bool ret = false;
    ret = dataAbilityHelper->TransferScheme(uri1, dataShareUri);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_TransferScheme_0200
 * @tc.name: TransferScheme
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_TransferScheme_0200, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string uriString = "datashare";
    Uri uri1(uriString);
    Uri dataShareUri(uriString);
    bool ret = false;
    ret = dataAbilityHelper->TransferScheme(uri1, dataShareUri);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_TransferScheme_0300
 * @tc.name: TransferScheme
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_TransferScheme_0300, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string uriString = "abc";
    Uri uri1(uriString);
    Uri dataShareUri(uriString);
    bool ret = false;
    ret = dataAbilityHelper->TransferScheme(uri1, dataShareUri);
    EXPECT_TRUE(!ret);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_TransferScheme_0400
 * @tc.name: TransferScheme
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_TransferScheme_0400, Function | MediumTest | Level3)
{
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("datashare://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context);
    std::string uriString = "abc";
    Uri dataShareUri(uriString);
    bool ret = false;
    ret = dataAbilityHelper->TransferScheme(*uri, dataShareUri);
    EXPECT_TRUE(ret);
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0100
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0100 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context = nullptr;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0200
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0200 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context =
        std::make_shared<AbilityRuntime::MockAbilityRuntimeContext>();
    std::shared_ptr<Uri> uri = nullptr;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0300
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0300 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context =
        std::make_shared<AbilityRuntime::MockAbilityRuntimeContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0400
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0400 start";
    std::shared_ptr<Context> context = nullptr;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0500
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0500 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();
    std::shared_ptr<Uri> uri = nullptr;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0600
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0600 start";
    std::shared_ptr<OHOS::AbilityRuntime::Context> context =
        std::make_shared<AbilityRuntime::MockAbilityRuntimeContext>();
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    bool tryBind = false;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri, tryBind);
    EXPECT_TRUE(dataAbilityHelper != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0600 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0700
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0700 start";
    sptr<IRemoteObject> remote = nullptr;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(remote);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0700 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0800
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0800, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0800 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    sptr<IRemoteObject> remote = nullptr;
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(remote, uri);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0800 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_0900
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_0900, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0900 start";
    std::shared_ptr<Uri> uri = nullptr;
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(remote, uri);
    EXPECT_TRUE(dataAbilityHelper == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_0900 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Creator_1000
 * @tc.name: Creator
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Creator_1000, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_1000 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(remote, uri);
    EXPECT_TRUE(dataAbilityHelper != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Creator_1000 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_DataAbilityHelper_0100
 * @tc.name: DataAbilityHelper
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(
    DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_DataAbilityHelper_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DataAbilityHelper_0100 start";
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    auto dataShareHelper = std::make_shared<DataShare::DataShareHelper>(remote, *uri);
    auto result = std::make_shared<DataAbilityHelper>(dataShareHelper);
    EXPECT_TRUE(result != nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_DataAbilityHelper_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Call_0100
 * @tc.name: Call
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Call_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Call_0100 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    std::weak_ptr<AbilityRecord> abilityRecord;
    sptr<IRemoteObject> remote = new OHOS::AAFwk::Token(abilityRecord);
    dataAbilityHelper->dataShareHelper_ = std::make_shared<DataShare::DataShareHelper>(remote, *uri);

    const std::string method;
    const std::string arg;
    AppExecFwk::PacMap pacMap;

    EXPECT_TRUE(dataAbilityHelper->Call(*uri, method, arg, pacMap) == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Call_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityHelper_Call_0200
 * @tc.name: Call
 * @tc.desc: Simulate successful test cases
 */
HWTEST_F(DataAbilityHelperForObserverTest, AaFwk_DataAbilityHelper_Call_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Call_0200 start";
    std::shared_ptr<Context> context = std::make_shared<AbilityContext>();;
    std::shared_ptr<Uri> uri =
        std::make_shared<Uri>("dataability://device_id/com.domainname.dataability.persondata/person/10");
    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = DataAbilityHelper::Creator(context, uri);
    EXPECT_TRUE(dataAbilityHelper != nullptr);
    dataAbilityHelper->dataAbilityHelperImpl_ = nullptr;
    dataAbilityHelper->dataShareHelper_ = nullptr;
    const std::string method;
    const std::string arg;
    AppExecFwk::PacMap pacMap;

    EXPECT_TRUE(dataAbilityHelper->Call(*uri, method, arg, pacMap) == nullptr);

    GTEST_LOG_(INFO) << "AaFwk_DataAbilityHelper_Call_0200 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS