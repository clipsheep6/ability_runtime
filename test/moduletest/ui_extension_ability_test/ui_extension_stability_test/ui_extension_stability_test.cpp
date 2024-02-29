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

#include "ability_manager_client.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "app_mgr_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "nativetoken_kit.h"
#include "session_info.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "session/host/include/extension_session.h"
#include "session_manager/include/extension_session_manager.h"
#include "token_setproc.h"
#include "ui_extension_connect_module_test_observer.h"
#include "want.h"
#include "want_params.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TARGET_BUNDLE_NAME = "com.ohos.uiextensionprovider";
const std::string TARGET_ABILITY_NAME = "UIExtensionProvider";
const std::string TARGET_MODULE_NAME = "entry";
const std::string TARGET_UIABILITY_NAME = "EntryAbility";

const std::string USER_BUNDLE_NAME = "com.ohos.uiextensionuser";
const std::string USER_ABILITY_NAME = "EntryAbility";
const std::string USER_MODULE_NAME = "entry";
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";

const uint32_t TEST_TIMES = 100;
const uint32_t TEST_TIMEOUT_MS = 5 * 1000;

static void SetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.RUNNING_STATE_OBSERVER";
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

using IApplicationStateObserver = AppExecFwk::IApplicationStateObserver;

class UIExtensionStabilityTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void WaitUntilProcessCreated(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void WaitUntilProcessDied(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void CheckProcessNotDied(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void WaitUntilAbilityForeground(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void WaitUntilAbilityBackground(const sptr<UIExtensionConnectModuleTestObserver> &observer);

    void RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);
    void UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);
    static sptr<AppExecFwk::IAppMgr> appMgr_;
};

sptr<AppExecFwk::IAppMgr> UIExtensionStabilityTest::appMgr_ = nullptr;

void UIExtensionStabilityTest::SetUpTestCase()
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Remote object is nullptr.");
        return;
    }

    sptr<AppExecFwk::IAppMgr> appMgr = iface_cast<AppExecFwk::IAppMgr>(remoteObj);
    if (appMgr == nullptr) {
        HILOG_ERROR("App mgr is nullptr.");
        return;
    }

    appMgr_ = appMgr;
}

void UIExtensionStabilityTest::TearDownTestCase()
{}

void UIExtensionStabilityTest::SetUp()
{}

void UIExtensionStabilityTest::TearDown()
{}

void UIExtensionStabilityTest::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    std::vector<std::string> bundleNameList;
    bundleNameList.push_back(TARGET_BUNDLE_NAME);
    bundleNameList.push_back(USER_BUNDLE_NAME);
    auto ret = appMgr_->RegisterApplicationStateObserver(observer, bundleNameList);
    if (ret != ERR_OK) {
        HILOG_ERROR("Register failed.");
    }
}

void UIExtensionStabilityTest::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    auto ret = appMgr_->UnregisterApplicationStateObserver(observer);
    if (ret != ERR_OK) {
        HILOG_ERROR("Unregister failed.");
    }
}

void UIExtensionStabilityTest::WaitUntilProcessCreated(const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(TEST_TIMEOUT_MS),
        [observer]() {
            return observer->processCreated_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processCreated_, true);
}

void UIExtensionStabilityTest::WaitUntilProcessDied(const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(TEST_TIMEOUT_MS),
        [observer]() {
            return observer->processDied_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processDied_, true);
}

void UIExtensionStabilityTest::CheckProcessNotDied(const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(TEST_TIMEOUT_MS),
        [observer]() {
            return observer->processDied_;
        });
    EXPECT_EQ(waitStatus, false);
    EXPECT_EQ(observer->processDied_, false);
}

void UIExtensionStabilityTest::WaitUntilAbilityForeground(
    const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(TEST_TIMEOUT_MS),
        [observer]() {
            return observer->processForegrounded_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processForegrounded_, true);
}

void UIExtensionStabilityTest::WaitUntilAbilityBackground(
    const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(TEST_TIMEOUT_MS),
        [observer]() {
            return observer->processBackgrounded_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processBackgrounded_, true);
}

/**
 * @tc.name: TerminateUIExtensionAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI8TYNB
 */
HWTEST_F(UIExtensionStabilityTest, TerminateUIExtensionAbility_0100, TestSize.Level1)
{
    HILOG_INFO("TerminateUIExtensionAbility_0100 start.");
    auto currentId = GetSelfTokenID();
    SetNativeToken();

    auto observer = sptr<UIExtensionConnectModuleTestObserver>::MakeSptr();
    RegisterApplicationStateObserver(observer);

    // start uiextension user firstly.
    Want userWant;
    AppExecFwk::ElementName userElement("0", USER_BUNDLE_NAME, USER_ABILITY_NAME, USER_MODULE_NAME);
    userWant.SetElement(userElement);
    EXPECT_EQ(AbilityManagerClient::GetInstance()->StartAbility(userWant), ERR_OK);
    WaitUntilAbilityForeground(observer);

    sptr<IRemoteObject> token = nullptr;
    auto ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
    int resultCode = 0;
    Want resultWant;
    ret = AbilityManagerClient::GetInstance()->TerminateAbility(token, resultCode, &resultWant);
    WaitUntilProcessDied(observer);

    UnregisterApplicationStateObserver(observer);
    HILOG_INFO("TerminateUIExtensionAbility_0100 finish.");
}

/**
 * @tc.name: MinimizeUIExtensionAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionStabilityTest, MinimizeUIExtensionAbility_0100, TestSize.Level1)
{
    HILOG_INFO("MinimizeUIExtensionAbility_0100 start.");
    auto currentId = GetSelfTokenID();
    SetNativeToken();

    auto observer = sptr<UIExtensionConnectModuleTestObserver>::MakeSptr();
    RegisterApplicationStateObserver(observer);

    // start uiextension user firstly.
    Want userWant;
    AppExecFwk::ElementName userElement("0", USER_BUNDLE_NAME, USER_ABILITY_NAME, USER_MODULE_NAME);
    userWant.SetElement(userElement);
    EXPECT_EQ(AbilityManagerClient::GetInstance()->StartAbility(userWant), ERR_OK);
    WaitUntilAbilityForeground(observer);

    Want uiAbilityWant;
    AppExecFwk::ElementName uiAbilityElement("0", TARGET_BUNDLE_NAME, TARGET_UIABILITY_NAME, TARGET_MODULE_NAME);
    uiAbilityWant.SetElement(uiAbilityElement);
    EXPECT_EQ(AbilityManagerClient::GetInstance()->StartAbility(uiAbilityWant), ERR_OK);
    WaitUntilAbilityForeground(observer);

    // start uiability and uiextension user repeatly.
    for (uint32_t i = 0; i < TEST_TIMES; i++) {
        AbilityManagerClient::GetInstance()->StartAbility(userWant);
        AbilityManagerClient::GetInstance()->StartAbility(uiAbilityWant);
    }

    // start uiability and destroy
    {
        auto ret = AbilityManagerClient::GetInstance()->StartAbility(uiAbilityWant);
        WaitUntilAbilityForeground(observer);
        sptr<IRemoteObject> token = nullptr;
        ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
        int resultCode = 0;
        Want resultWant;
        ret = AbilityManagerClient::GetInstance()->TerminateAbility(token, resultCode, &resultWant);
        WaitUntilAbilityForeground(observer);
    }

    // start ui extension user and destroy
    {
        auto ret = AbilityManagerClient::GetInstance()->StartAbility(userWant);
        WaitUntilAbilityForeground(observer);
        sptr<IRemoteObject> token = nullptr;
        ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
        int resultCode = 0;
        Want resultWant;
        ret = AbilityManagerClient::GetInstance()->TerminateAbility(token, resultCode, &resultWant);
        WaitUntilAbilityForeground(observer);
    }

    UnregisterApplicationStateObserver(observer);
    HILOG_INFO("MinimizeUIExtensionAbility_0100 finish.");
}
} // namespace AAFwk
} // namespace OHOS
