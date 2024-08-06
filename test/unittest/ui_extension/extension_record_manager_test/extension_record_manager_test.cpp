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

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_record.h"
#include "extension_record.h"
#define private public
#include "extension_record_manager.h"
#undef private
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ExtensionRecordManagerTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRecordManagerTest::SetUpTestCase(void)
{}

void ExtensionRecordManagerTest::TearDownTestCase(void)
{}

void ExtensionRecordManagerTest::SetUp()
{}

void ExtensionRecordManagerTest::TearDown()
{}

/**
 * @tc.name: IsFocused_0100
 * @tc.desc: check is focused.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsFocused_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    extRecordMgr->SetCachedFocusedCallerToken(extensionRecordId, token);

    bool isFocused = extRecordMgr->IsFocused(extensionRecordId, token);
    EXPECT_EQ(isFocused, true);

    auto focusedToken = extRecordMgr->GetCachedFocusedCallerToken(extensionRecordId);
    EXPECT_EQ(focusedToken, token);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerTokenList_0100
 * @tc.desc: get caller token list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetCallerTokenList_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(callerToken, nullptr);
    sessionInfo->callerToken = callerToken;
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);
    extRecord->SetFocusedCallerToken(callerToken);
    extRecord->GetFocusedCallerToken();

    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    std::list<sptr<IRemoteObject>> callerList;
    auto rootCallerToken = extRecordMgr->GetCallerTokenList(extensionRecordId, callerList);
    EXPECT_EQ(callerList.size(), 1);
    EXPECT_EQ(callerList.front(), callerToken);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AbilityRuntime
} // namespace OHOS
