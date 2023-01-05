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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "ability_loader.h"
#include "data_ability_impl.h"
#include "hilog_wrapper.h"
#include "mock_ability_token.h"
#include "mock_data_ability.h"
#include "base/account/os_account/services/accountmgr/test/mock/app_account/accesstoken_kit.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/ability/native/data_ability_operation.h"
#undef private
#undef protected
namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

REGISTER_AA(MockDataAbility)

class DataAbilityImplTest : public testing::Test {
public:
    DataAbilityImplTest() : dataabilityimpl(nullptr)
    {}
    ~DataAbilityImplTest()
    {
        dataabilityimpl = nullptr;
    }
    DataAbilityImpl* dataabilityimpl;

    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataAbilityImplTest::SetUpTestCase(void)
{}

void DataAbilityImplTest::TearDownTestCase(void)
{}

void DataAbilityImplTest::SetUp(void)
{}

void DataAbilityImplTest::TearDown(void)
{}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Insert_0100
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Insert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Insert_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->isNativeAbility = true;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();

    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    EXPECT_NE(dataabilityimpl, nullptr);
    Uri uri("\nullptr");
    int number = 1;
    int result = 0;
    NativeRdb::ValuesBucket value;
    result = dataabilityimpl->Insert(uri, value);

    EXPECT_EQ(number, result);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Insert_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Insert_0200
 * @tc.name: Insert
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Insert_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Insert_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    int number = -1;
    NativeRdb::ValuesBucket value;

    EXPECT_EQ(number, dataabilityimpl->Insert(uri, value));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Insert_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Insert_0300
 * @tc.name: Insert
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Insert_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Insert_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->isNativeAbility = true;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    EXPECT_NE(dataabilityimpl, nullptr);
    Uri uri("\nullptr");
    constexpr int32_t number = -1;
    NativeRdb::ValuesBucket value;
    auto result = dataabilityimpl->Insert(uri, value);

    EXPECT_EQ(number, result);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Insert_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Update_0100
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Update_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Update_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    int number = 1;
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_EQ(number, dataabilityimpl->Update(uri, value, predicates));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Update_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Update_0200
 * @tc.name: Update
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Update_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Update_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    int number = -1;
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_EQ(number, dataabilityimpl->Update(uri, value, predicates));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Update_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Update_0300
 * @tc.name: Update
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Update_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Update_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    constexpr int32_t number = -1;
    NativeRdb::ValuesBucket value;
    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_EQ(number, dataabilityimpl->Update(uri, value, predicates));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Update_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Delete_0100
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Delete_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Delete_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    int number = 1;

    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_EQ(number, dataabilityimpl->Delete(uri, predicates));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Delete_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Delete_0200
 * @tc.name: Delete
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Delete_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Delete_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    int number = -1;
    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_EQ(number, dataabilityimpl->Delete(uri, predicates));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Delete_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Delete_0300
 * @tc.name: Delete
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Delete_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Delete_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    constexpr int32_t number = -1;

    NativeRdb::DataAbilityPredicates predicates;

    EXPECT_EQ(number, dataabilityimpl->Delete(uri, predicates));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Delete_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Query_0100
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Query_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Query_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    Uri uri("\nullptr");
    std::vector<std::string> columns;
    columns.push_back("string1");
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> set = dataabilityimpl->Query(uri, columns, predicates);

    EXPECT_TRUE(set != nullptr);
    dataabilityimpl.reset();
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Query_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Query_0200
 * @tc.name: Query
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Query_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Query_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    std::vector<std::string> columns;
    columns.push_back("string1");
    NativeRdb::DataAbilityPredicates predicates;

    std::shared_ptr<NativeRdb::AbsSharedResultSet> set = dataabilityimpl->Query(uri, columns, predicates);
    EXPECT_EQ(nullptr, set);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Query_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Query_0300
 * @tc.name: Query
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Query_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Query_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    std::vector<std::string> columns;
    columns.push_back("string1");
    NativeRdb::DataAbilityPredicates predicates;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> set = dataabilityimpl->Query(uri, columns, predicates);

    EXPECT_TRUE(set == nullptr);
    dataabilityimpl.reset();
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Query_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_OpenFile_0100
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_OpenFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenFile_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    Uri uri("\nullptr");
    constexpr int32_t number = 1;
    const std::string mode = "abc";

    EXPECT_EQ(number, dataabilityimpl->OpenFile(uri, mode));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenFile_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_OpenFile_0200
 * @tc.name: OpenFile
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_OpenFile_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenFile_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    constexpr int32_t number = -1;
    const std::string mode = "abc";

    EXPECT_EQ(number, dataabilityimpl->OpenFile(uri, mode));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenFile_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_OpenFile_0300
 * @tc.name: OpenFile
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_OpenFile_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenFile_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    Uri uri("\nullptr");
    constexpr int32_t number = -1;
    const std::string mode = "r";

    EXPECT_EQ(number, dataabilityimpl->OpenFile(uri, mode));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenFile_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_OpenRawFile_0100
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_OpenRawFile_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenRawFile_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    Uri uri("\nullptr");
    constexpr int32_t number = 1;
    const std::string mode = "abc";

    EXPECT_EQ(number, dataabilityimpl->OpenRawFile(uri, mode));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenRawFile_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_OpenRawFile_0200
 * @tc.name: OpenRawFile
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_OpenRawFile_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenRawFile_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    constexpr int32_t number = -1;
    const std::string mode = "abc";

    EXPECT_EQ(number, dataabilityimpl->OpenRawFile(uri, mode));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenRawFile_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_OpenRawFile_0300
 * @tc.name: OpenRawFile
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_OpenRawFile_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenRawFile_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);
    Uri uri("\nullptr");
    constexpr int32_t number = -1;
    const std::string mode = "r";

    EXPECT_EQ(number, dataabilityimpl->OpenRawFile(uri, mode));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_OpenRawFile_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Call_0100
 * @tc.name: Call
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Call_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Call_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    Uri uri("\nullptr");
    const std::string method = "abc";
    const std::string arg = "abc";
    AppExecFwk::PacMap pacMap;
    std::shared_ptr<AppExecFwk::PacMap> set = dataabilityimpl->Call(uri, method, arg, pacMap);

    EXPECT_TRUE(set != nullptr);
    dataabilityimpl.reset();
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Call_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Call_0200
 * @tc.name: Call
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Call_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Call_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    const std::string method = "abc";
    const std::string arg = "abc";
    AppExecFwk::PacMap pacMap;

    std::shared_ptr<AppExecFwk::PacMap> set = dataabilityimpl->Call(uri, method, arg, pacMap);
    EXPECT_EQ(nullptr, set);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Call_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetFileTypes_0100
 * @tc.name: GetFileTypes
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetFileTypes_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetFileTypes_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    std::vector<std::string> value;
    const std::string mimeTypeFilter = "abc";

    value = dataabilityimpl->GetFileTypes(uri, mimeTypeFilter);
    std::string ret = value.back();
    EXPECT_EQ(ret, mimeTypeFilter);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetFileTypes_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetFileTypes_0200
 * @tc.name: GetFileTypes
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetFileTypes_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetFileTypes_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    std::vector<std::string> value;
    const std::string mimeTypeFilter = "abc";

    dataabilityimpl->GetFileTypes(uri, mimeTypeFilter);
    EXPECT_EQ(value,dataabilityimpl->GetFileTypes(uri, mimeTypeFilter));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetFileTypes_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetType_0100
 * @tc.name: GetType
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetType_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetType_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    const std::string value("Type1");

    EXPECT_EQ(value, dataabilityimpl->GetType(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetType_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetType_0200
 * @tc.name: GetType
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetType_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetType_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    const std::string value;

    EXPECT_EQ(value, dataabilityimpl->GetType(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetType_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Reload_0100
 * @tc.name: Reload
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Reload_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Reload_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    PacMap extras;

    EXPECT_TRUE(dataabilityimpl->Reload(uri, extras));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Reload_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_Reload_0200
 * @tc.name: Reload
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_Reload_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Reload_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    PacMap extras;

    EXPECT_FALSE(dataabilityimpl->Reload(uri, extras));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_Reload_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_BatchInsert_0100
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_BatchInsert_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_BatchInsert_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    std::vector<NativeRdb::ValuesBucket> values;
    constexpr int32_t number = 1;

    EXPECT_EQ(number, dataabilityimpl->BatchInsert(uri, values));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_BatchInsert_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_BatchInsert_0200
 * @tc.name: BatchInsert
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_BatchInsert_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_BatchInsert_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    std::vector<NativeRdb::ValuesBucket> values;
    constexpr int32_t number = -1;

    EXPECT_EQ(number, dataabilityimpl->BatchInsert(uri, values));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_BatchInsert_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_BatchInsert_0300
 * @tc.name: BatchInsert
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_BatchInsert_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_BatchInsert_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    std::vector<NativeRdb::ValuesBucket> values;
    constexpr int32_t number = -1;

    EXPECT_EQ(number, dataabilityimpl->BatchInsert(uri, values));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_BatchInsert_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_NormalizeUri_0100
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_NormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_NormalizeUri_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    Uri urivalue("UriTest");

    EXPECT_EQ(urivalue, dataabilityimpl->NormalizeUri(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_NormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_NormalizeUri_0200
 * @tc.name: NormalizeUri
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_NormalizeUri_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_NormalizeUri_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    Uri urivalue("");

    EXPECT_EQ(urivalue, dataabilityimpl->NormalizeUri(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_NormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_NormalizeUri_0300
 * @tc.name: NormalizeUri
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_NormalizeUri_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_NormalizeUri_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    Uri urivalue("");

    EXPECT_EQ(urivalue, dataabilityimpl->NormalizeUri(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_NormalizeUri_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_DenormalizeUri_0100
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_DenormalizeUri_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_DenormalizeUri_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    Uri urivalue("UriTest");

    EXPECT_EQ(urivalue, dataabilityimpl->DenormalizeUri(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_DenormalizeUri_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_DenormalizeUri_0200
 * @tc.name: DenormalizeUri
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_DenormalizeUri_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_DenormalizeUri_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Uri uri("\nullptr");
    Uri urivalue("");

    EXPECT_EQ(urivalue, dataabilityimpl->DenormalizeUri(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_DenormalizeUri_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_DenormalizeUri_0300
 * @tc.name: DenormalizeUri
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_DenormalizeUri_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_DenormalizeUri_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    Uri urivalue("");

    EXPECT_EQ(urivalue, dataabilityimpl->DenormalizeUri(uri));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_DenormalizeUri_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetPermissionInfo_0100
 * @tc.name: GetPermissionInfo
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetPermissionInfo_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "r";

    EXPECT_TRUE(abilityInfo->readPermission == dataabilityimpl->GetPermissionInfo(permissionType));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetPermissionInfo_0200
 * @tc.name: GetPermissionInfo
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetPermissionInfo_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "w";

    EXPECT_TRUE(abilityInfo->writePermission == dataabilityimpl->GetPermissionInfo(permissionType));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetPermissionInfo_0300
 * @tc.name: GetPermissionInfo
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetPermissionInfo_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "a";

    EXPECT_TRUE("" == dataabilityimpl->GetPermissionInfo(permissionType));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetPermissionInfo_0400
 * @tc.name: GetPermissionInfo
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetPermissionInfo_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0400 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "r";

    EXPECT_TRUE("" == dataabilityimpl->GetPermissionInfo(permissionType));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_GetPermissionInfo_0500
 * @tc.name: GetPermissionInfo
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_GetPermissionInfo_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0500 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "w";

    EXPECT_TRUE("" == dataabilityimpl->GetPermissionInfo(permissionType));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_GetPermissionInfo_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0100
 * @tc.name: CheckReadAndWritePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "a";
    bool ret = false;
    ret = dataabilityimpl->CheckReadAndWritePermission(permissionType);

    EXPECT_TRUE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0200
 * @tc.name: CheckReadAndWritePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "r";
    bool ret = false;
    ret = dataabilityimpl->CheckReadAndWritePermission(permissionType);

    EXPECT_TRUE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0300
 * @tc.name: CheckReadAndWritePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "w";
    bool ret = false;
    ret = dataabilityimpl->CheckReadAndWritePermission(permissionType);

    EXPECT_TRUE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0400
 * @tc.name: CheckReadAndWritePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0400 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "r";
    bool ret = true;
    ret = dataabilityimpl->CheckReadAndWritePermission(permissionType);

    EXPECT_FALSE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0500
 * @tc.name: CheckReadAndWritePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0500 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string permissionType = "w";
    bool ret = true;
    ret = dataabilityimpl->CheckReadAndWritePermission(permissionType);

    EXPECT_FALSE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckReadAndWritePermission_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0100
 * @tc.name: CheckOpenFilePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string mode = "r";
    bool ret = true;
    ret = dataabilityimpl->CheckOpenFilePermission(mode);

    EXPECT_FALSE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0200
 * @tc.name: CheckOpenFilePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string mode = "w";
    bool ret = true;
    ret = dataabilityimpl->CheckOpenFilePermission(mode);

    EXPECT_FALSE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0300
 * @tc.name: CheckOpenFilePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string mode = "ar";
    bool ret = true;
    ret = dataabilityimpl->CheckOpenFilePermission(mode);

    EXPECT_FALSE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0400
 * @tc.name: CheckOpenFilePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0400 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string mode = "aw";
    bool ret = true;
    ret = dataabilityimpl->CheckOpenFilePermission(mode);

    EXPECT_FALSE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0500
 * @tc.name: CheckOpenFilePermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0500 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    Uri uri("\nullptr");
    const std::string mode = "abc";
    bool ret = false;
    ret = dataabilityimpl->CheckOpenFilePermission(mode);

    EXPECT_TRUE(ret);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckOpenFilePermission_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_ExecuteBatch_0100
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_ExecuteBatch_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    results.clear();

    EXPECT_EQ(results, dataabilityimpl->ExecuteBatch(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_ExecuteBatch_0200
 * @tc.name: ExecuteBatch
 * @tc.desc: Validate when normally entering a string.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_ExecuteBatch_0200, Function | MediumTest | Level3)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    results.clear();

    EXPECT_EQ(results, dataabilityimpl->ExecuteBatch(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_ExecuteBatch_0300
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_ExecuteBatch_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    operation->type_ = 1;
    operations.push_back(operation);
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    results.clear();

    EXPECT_EQ(results, dataabilityimpl->ExecuteBatch(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_ExecuteBatch_0400
 * @tc.name: ExecuteBatch
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_ExecuteBatch_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0400 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    dataabilityimpl->ExecuteBatch(operations);
    EXPECT_TRUE(true);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_ExecuteBatch_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0100
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;

    EXPECT_TRUE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0200
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation;
    operations.push_back(operation);

    EXPECT_FALSE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0300
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    operation->type_ = 1;
    operations.push_back(operation);

    EXPECT_FALSE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0400
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0400 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    operation->type_ = 2;
    operations.push_back(operation);

    EXPECT_FALSE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0400 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0500
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0500 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    operation->type_ = 3;
    operations.push_back(operation);

    EXPECT_FALSE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0500 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0600
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0600, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0600 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation = std::make_shared<DataAbilityOperation>();
    operation->type_ = 4;
    operations.push_back(operation);

    EXPECT_FALSE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0600 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0700
 * @tc.name: CheckExecuteBatchPermission
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(
    DataAbilityImplTest, AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0700, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0700 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->name = "MockDataAbility";
    abilityInfo->type = AbilityType::DATA;
    abilityInfo->readPermission = "r";
    abilityInfo->writePermission = "w";
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    std::shared_ptr<AbilityLocalRecord> record = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    std::shared_ptr<EventRunner> eventRunner = EventRunner::Create(abilityInfo->name);
    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(eventRunner);
    std::shared_ptr<Ability> ability = std::make_shared<MockDataAbility>();
    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    dataabilityimpl->Init(application, record, ability, handler, token, contextDeal);
    contextDeal->SetAbilityInfo(abilityInfo);
    ability->AttachBaseContext(contextDeal);

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    std::shared_ptr<DataAbilityOperation> operation1 = std::make_shared<DataAbilityOperation>();
    std::shared_ptr<DataAbilityOperation> operation2 = std::make_shared<DataAbilityOperation>();
    operation1->type_ = 4;
    operation2->type_ = 1;
    operations.push_back(operation1);
    operations.push_back(operation2);

    EXPECT_FALSE(dataabilityimpl->CheckExecuteBatchPermission(operations));
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_CheckExecuteBatchPermission_0700 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0100
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0100 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    dataabilityimpl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    targetState.state = AbilityLifeCycleState::ABILITY_STATE_INITIAL;
    targetState.isNewWant = false;
    dataabilityimpl->HandleAbilityTransaction(want, targetState);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0100 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0200
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0200 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    dataabilityimpl->lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    targetState.state = AAFwk::ABILITY_STATE_ACTIVE;
    targetState.isNewWant = true;
    dataabilityimpl->HandleAbilityTransaction(want, targetState);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0200 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0300
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0300 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    dataabilityimpl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    targetState.state = AAFwk::ABILITY_STATE_ACTIVE;
    targetState.isNewWant = true;
    dataabilityimpl->HandleAbilityTransaction(want, targetState);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0300 end";
}

/**
 * @tc.number: AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0400
 * @tc.name: HandleAbilityTransaction
 * @tc.desc: Simulate successful test cases.
 */
HWTEST_F(DataAbilityImplTest, AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0400 start";
    std::shared_ptr<DataAbilityImpl> dataabilityimpl = std::make_shared<DataAbilityImpl>();
    Want want;
    AAFwk::LifeCycleStateInfo targetState;
    dataabilityimpl->lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    targetState.state = AAFwk::ABILITY_STATE_BACKGROUND;
    targetState.isNewWant = true;
    dataabilityimpl->HandleAbilityTransaction(want, targetState);
    sleep(1);
    GTEST_LOG_(INFO) << "AaFwk_DataAbilityImplTest_HandleAbilityTransaction_0400 end";
}
}  // namespace AppExecFwk
}  // namespace OHOS
