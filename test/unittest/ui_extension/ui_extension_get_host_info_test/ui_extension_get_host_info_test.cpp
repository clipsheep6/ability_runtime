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
#include "hilog_wrapper.h"
#include "ability_manager_client.h"
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionGetHostInfoTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionGetHostInfoTest::SetUpTestCase(void)
{}

void UIExtensionGetHostInfoTest::TearDownTestCase(void)
{}

void UIExtensionGetHostInfoTest::SetUp()
{}

void UIExtensionGetHostInfoTest::TearDown()
{}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0100
 * @tc.desc: get ui extension root host info without permision.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(token, nullptr);
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0200
 * @tc.desc: get ui extension root host info without invalid param.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(nullptr, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0300
 * @tc.desc: basic function test of get ui extension root host info.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto currentID = GetSelfTokenID();
    AppExecFwk::MockNativeToken::SetNativeToken();

    sptr<IRemoteObject> token = nullptr;
    if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        auto ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
        EXPECT_EQ(ret, ERR_OK);

        UIExtensionHostInfo hostInfo;
        ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo);
        // cause top ability isn't a uiextension ability.
        EXPECT_EQ(ret, ERR_INVALID_VALUE);
        TAG_LOGI(AAFwkTag::TEST, "Get host info uri: %{public}s", hostInfo.elementName_.GetURI().c_str());
    }

    SetSelfTokenID(currentID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetUIExtensionRootHostInfo_0400
 * @tc.desc: basic function test of get ui extension root host info.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(UIExtensionGetHostInfoTest, GetUIExtensionRootHostInfo_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto currentID = GetSelfTokenID();
    AppExecFwk::MockNativeToken::SetNativeToken();

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(token, nullptr);

    UIExtensionHostInfo hostInfo;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "Get host info uri: %{public}s", hostInfo.elementName_.GetURI().c_str());

    SetSelfTokenID(currentID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AAFwk
} // namespace OHOS
