/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "session_info.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionPreloadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionPreloadTest::SetUpTestCase(void)
{}

void UIExtensionPreloadTest::TearDownTestCase(void)
{}

void UIExtensionPreloadTest::SetUp()
{}

void UIExtensionPreloadTest::TearDown()
{}

/**
 * @tc.name: PermissionCheck_0100
 * @tc.desc: permission check test.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto ret = AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(providerWant, hostBundleName,
        DEFAULT_INVAL_VALUE);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}
} // namespace AAFwk
} // namespace OHOS
