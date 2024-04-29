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
#define protected public
#include "ability_impl.h"
#include "ability_local_record.h"
#include "ability_thread.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "mock_ability_token.h"
#include "parcel.h"
#undef protected
#undef private

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace testing;
using namespace testing::ext;

class AbilityLocalRecordTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityLocalRecordTest::SetUpTestCase()
{}

void AbilityLocalRecordTest::TearDownTestCase()
{}

void AbilityLocalRecordTest::SetUp()
{}

void AbilityLocalRecordTest::TearDown()
{}

/**
 * @tc.number: GetEventHandler_0100
 * @tc.name: GetEventHandler
 * @tc.desc: GetEventHandler Test, return is not nullptr.
 */
HWTEST_F(AbilityLocalRecordTest, GetEventHandler_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetEventHandler_0100 start";
    auto record = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    auto handler = std::make_shared<EventHandler>(EventRunner::Create());
    record->SetEventHandler(handler);
    EXPECT_TRUE(record->GetEventHandler() != nullptr);
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetEventHandler_0100 end";
}

/**
 * @tc.number: GetEventRunner_0100
 * @tc.name: GetEventRunner
 * @tc.desc: GetEventRunner Test, return is not nullptr.
 */
HWTEST_F(AbilityLocalRecordTest, GetEventRunner_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest SetEventHandler_0100 start";
    auto record = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    record->SetEventRunner(runner);
    EXPECT_TRUE(record->GetEventRunner() != nullptr);
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest SetEventHandler_0100 end";
}

/**
 * @tc.number: GetAbilityThread_0100
 * @tc.name: GetAbilityThread
 * @tc.desc: GetAbilityThread Test, return is nullptr.
 */
HWTEST_F(AbilityLocalRecordTest, GetAbilityThread_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetAbilityThread_0100 start";
    auto record = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    auto abilityThread = sptr<AbilityThread>(nullptr);
    record->SetAbilityThread(abilityThread);
    EXPECT_TRUE(record->GetAbilityThread() == nullptr);
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetAbilityThread_0100 end";
}

/**
 * @tc.number: GetWant_0100
 * @tc.name: GetWant
 * @tc.desc: GetWant Test, return is nullptr.
 */
HWTEST_F(AbilityLocalRecordTest, GetWant_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetWant_0100 start";
    auto record = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(nullptr, nullptr);
    std::shared_ptr<AAFwk::Want> want;
    record->SetWant(want);
    EXPECT_TRUE(record->GetWant() == nullptr);
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetWant_0100 end";
}

/**
 * @tc.number: GetAbilityInfo_0100
 * @tc.name: GetAbilityInfo
 * @tc.desc: GetAbilityInfo Test, return is not nullptr.
 */
HWTEST_F(AbilityLocalRecordTest, GetAbilityInfo_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetAbilityInfo_0100 start";
    auto info = std::make_shared<OHOS::AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto record = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(info, token);
    EXPECT_TRUE(record->GetAbilityInfo() != nullptr);
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetAbilityInfo_0100 end";
}

/**
 * @tc.number: GetToken_0100
 * @tc.name: GetToken
 * @tc.desc: GetToken Test, return is not nullptr.
 */
HWTEST_F(AbilityLocalRecordTest, GetToken_0100, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetToken_0100 start";
    auto info = std::make_shared<OHOS::AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto record = std::make_shared<OHOS::AppExecFwk::AbilityLocalRecord>(info, token);
    EXPECT_TRUE(record->GetToken() != nullptr);
    GTEST_LOG_(INFO) << "AbilityLocalRecordTest GetToken_0100 end";
}