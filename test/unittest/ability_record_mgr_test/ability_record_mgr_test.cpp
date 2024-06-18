/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "ability_record_mgr.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_stub.h"
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
class IAbilityMgrToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.AbilityMgrToken");
};

class AbilityMgrToken : public IRemoteStub<IAbilityMgrToken> {
public:
    AbilityMgrToken() = default;
    virtual ~AbilityMgrToken() = default;

    virtual int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }

private:
    DISALLOW_COPY_AND_MOVE(AbilityMgrToken);
};

class AbilityRecordMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityRecordMgrTest::SetUpTestCase()
{}

void AbilityRecordMgrTest::TearDownTestCase()
{}

void AbilityRecordMgrTest::SetUp()
{}

void AbilityRecordMgrTest::TearDown()
{}

/**
 * @tc.number: GetToken_0100
 * @tc.name: GetToken
 * @tc.desc: GetToken Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, GetEventHandler_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetToken_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    recordMgr->tokens_ = nullptr;
    auto token = recordMgr->GetToken();
    EXPECT_EQ(token, nullptr);
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetToken_0100 end";
}

/**
 * @tc.number: SetToken_0100
 * @tc.name: SetToken
 * @tc.desc: SetToken Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, SetToken_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest SetToken_0100 start";
    sptr<IRemoteObject> token = new (std::nothrow) AbilityMgrToken();
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    recordMgr->SetToken(token);
    EXPECT_NE(recordMgr->GetToken(), nullptr);
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest SetToken_0100 end";
}

/**
 * @tc.number: SetEventRunner_0100
 * @tc.name: SetEventRunner
 * @tc.desc: SetEventRunner Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, SetEventRunner_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest SetEventRunner_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    EXPECT_NE(recordMgr, nullptr);
    recordMgr->SetEventRunner(nullptr);

    std::shared_ptr<AppExecFwk::EventRunner> runner = EventRunner::GetMainEventRunner();
    EXPECT_NE(runner, nullptr);
    recordMgr->tokens_ = nullptr;
    recordMgr->SetEventRunner(runner);

    sptr<IRemoteObject> token = new (std::nothrow) AbilityMgrToken();
    recordMgr->SetToken(token);
    recordMgr->SetEventRunner(runner);
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest SetEventRunner_0100 end";
}

/**
 * @tc.number: AddAbilityRecord_0100
 * @tc.name: AddAbilityRecord
 * @tc.desc: AddAbilityRecord Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, AddAbilityRecord_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest AddAbilityRecord_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    EXPECT_NE(recordMgr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    recordMgr->AddAbilityRecord(token, nullptr);

    token = new (std::nothrow) AbilityMgrToken();
    recordMgr->AddAbilityRecord(token, nullptr);

    auto abilityRecord = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    EXPECT_NE(abilityRecord, nullptr);

    recordMgr->AddAbilityRecord(token, abilityRecord);
    EXPECT_EQ(recordMgr->abilityRecords_.size(), 1);
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest AddAbilityRecord_0100 end";
}

/**
 * @tc.number: RemoveAbilityRecord_0100
 * @tc.name: RemoveAbilityRecord
 * @tc.desc: RemoveAbilityRecord Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, RemoveAbilityRecord_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest RemoveAbilityRecord_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    EXPECT_NE(recordMgr, nullptr);
    sptr<IRemoteObject> token = nullptr;
    recordMgr->RemoveAbilityRecord(token);

    token = new (std::nothrow) AbilityMgrToken();
    auto abilityRecord = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    recordMgr->AddAbilityRecord(token, abilityRecord);
    EXPECT_EQ(recordMgr->abilityRecords_.size(), 1);

    recordMgr->RemoveAbilityRecord(token);
    EXPECT_EQ(recordMgr->abilityRecords_.size(), 0);
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest RemoveAbilityRecord_0100 end";
}

/**
 * @tc.number: GetRecordCount_0100
 * @tc.name: GetRecordCount
 * @tc.desc: GetRecordCount Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, GetRecordCount_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetRecordCount_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    EXPECT_EQ(recordMgr->GetRecordCount(), 0);

    sptr<IRemoteObject> token = new (std::nothrow) AbilityMgrToken();
    auto abilityRecord = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    recordMgr->AddAbilityRecord(token, abilityRecord);
    EXPECT_EQ(recordMgr->GetRecordCount(), 1);
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetRecordCount_0100 end";
}

/**
 * @tc.number: GetAbilityItem_0100
 * @tc.name: GetAbilityItem
 * @tc.desc: GetAbilityItem Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, GetAbilityItem_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetAbilityItem_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();
    sptr<IRemoteObject> token = nullptr;
    EXPECT_EQ(recordMgr->GetAbilityItem(token), nullptr);

    token = new (std::nothrow) AbilityMgrToken();
    auto abilityRecord = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    recordMgr->AddAbilityRecord(token, abilityRecord);
    EXPECT_NE(recordMgr->GetAbilityItem(token), nullptr);

    sptr<IRemoteObject> token2 = new (std::nothrow) AbilityMgrToken();
    EXPECT_EQ(recordMgr->GetAbilityItem(token2), nullptr);

    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetAbilityItem_0100 end";
}

/**
 * @tc.number: GetAllTokens_0100
 * @tc.name: GetAllTokens
 * @tc.desc: GetAllTokens Test, return is not nullptr.
 */
HWTEST_F(AbilityRecordMgrTest, GetAllTokens_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetAllTokens_0100 start";
    auto recordMgr = std::make_shared<OHOS::AppExecFwk::AbilityRecordMgr>();

    sptr<IRemoteObject> token = new (std::nothrow) AbilityMgrToken();
    auto abilityRecord = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    recordMgr->AddAbilityRecord(token, abilityRecord);
    EXPECT_EQ(recordMgr->GetAllTokens().size(), 1);

    GTEST_LOG_(INFO) << "AbilityRecordMgrTest GetAllTokens_0100 end";
}
}
}
