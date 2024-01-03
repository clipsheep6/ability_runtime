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

#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>

#include "ability_util.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_proxy.h"
#include "bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "ui_extension_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
static void SetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "SetUpTestCase";
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
    delete[] perms;
}
} // namespace

class UIExtensionInfoModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<AppExecFwk::IBundleMgr> bundleMgr_;
};

sptr<AppExecFwk::IBundleMgr> UIExtensionInfoModuleTest::bundleMgr_ = nullptr;

void UIExtensionInfoModuleTest::SetUpTestCase(void)
{
    HILOG_INFO("start.");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObj);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Bundle mgr is nullptr.");
        return;
    }

    bundleMgr_ = bundleMgr;
    SetNativeToken();
}

void UIExtensionInfoModuleTest::TearDownTestCase(void)
{}

void UIExtensionInfoModuleTest::SetUp()
{}

void UIExtensionInfoModuleTest::TearDown()
{}

/**
 * @tc.name: QueryUIExtensionAbilityInfos_0100
 * @tc.desc: Query SHARE extension ability infos function test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UIExtensionInfoModuleTest, QueryUIExtensionAbilityInfos_0100, TestSize.Level1)
{
    HILOG_INFO("start.");
    ASSERT_NE(bundleMgr_, nullptr);

    for (auto &type : UIExtensionUtils::UI_EXTENSION_SET) {
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        bool queryResult = bundleMgr_->QueryExtensionAbilityInfos(type, AppExecFwk::Constants::ALL_USERID,
            extensionInfos);
        for (auto &item : extensionInfos) {
            HILOG_INFO("UIExtensionAbility: type: %{public}s, bundleName: %{public}s, moduleName: %{public}s, "
                "abilityName: %{public}s.", ConvertToExtensionTypeName(type).c_str(),
                item.bundleName.c_str(), item.moduleName.c_str(), item.name.c_str());
            EXPECT_EQ(item.type, type);
            // Get apl of bundle, and output xml format.
        }
    }

    HILOG_INFO("finish.");
}
} // namespace AAFwk
} // namespace OHOS
