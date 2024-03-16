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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "nativetoken_kit.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "session_info.h"
#include "system_ability_definition.h"
#include "token_setproc.h"
#include "ui_extension_connect_module_test_connection.h"
#include "ui_extension_connect_module_test_observer.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int CONNECT_TIMEOUT_MS = 5 * 1000;
const int32_t MAIN_USER_ID = 100;

const std::string TARGET_BUNDLE_NAME = "com.ohos.uiextensionprovider";
const std::string TARGET_ABILITY_NAME = "UIExtensionProvider";
const std::string TARGET_MODULE_NAME = "entry";

const std::string USER_BUNDLE_NAME = "com.ohos.uiextensionuser";
const std::string USER_ABILITY_NAME = "EntryAbility";
const std::string USER_MODULE_NAME = "entry";
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";


static void SetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[2];
    perms[0] = "ohos.permission.CONNECT_UI_EXTENSION_ABILITY";
    perms[1] = "ohos.permission.RUNNING_STATE_OBSERVER";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
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

using IAbilityConnection = AppExecFwk::IAbilityConnection;
using IApplicationStateObserver = AppExecFwk::IApplicationStateObserver;

class UIExtensionConnectModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void WaitUntilConnectDone(const sptr<UIExtensionConnectModuleTestConnection> &connection);
    void WaitUntilDisConnectDone(const sptr<UIExtensionConnectModuleTestConnection> &connection);
    void WaitUntilProcessCreated(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void WaitUntilProcessDied(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void CheckProcessNotDied(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void WaitUntilAbilityForeground(const sptr<UIExtensionConnectModuleTestObserver> &observer);
    void WaitUntilAbilityBackground(const sptr<UIExtensionConnectModuleTestObserver> &observer);

    void RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);
    void UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer);
    static sptr<AppExecFwk::IAppMgr> appMgr_;
};

sptr<AppExecFwk::IAppMgr> UIExtensionConnectModuleTest::appMgr_ = nullptr;

void UIExtensionConnectModuleTest::SetUpTestCase(void)
{
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "Failed to get SystemAbilityManager.");
        return;
    }

    auto remoteObj = systemAbilityMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "Remote object is nullptr.");
        return;
    }

    sptr<AppExecFwk::IAppMgr> appMgr = iface_cast<AppExecFwk::IAppMgr>(remoteObj);
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "App mgr is nullptr.");
        return;
    }

    appMgr_ = appMgr;
}

void UIExtensionConnectModuleTest::TearDownTestCase(void)
{}

void UIExtensionConnectModuleTest::SetUp()
{}

void UIExtensionConnectModuleTest::TearDown()
{}

void UIExtensionConnectModuleTest::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    std::vector<std::string> bundleNameList;
    bundleNameList.push_back(TARGET_BUNDLE_NAME);
    bundleNameList.push_back(USER_BUNDLE_NAME);
    auto ret = appMgr_->RegisterApplicationStateObserver(observer, bundleNameList);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::TEST, "Register failed.");
    }
}

void UIExtensionConnectModuleTest::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    auto ret = appMgr_->UnregisterApplicationStateObserver(observer);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::TEST, "Unregister failed.");
    }
}

void UIExtensionConnectModuleTest::WaitUntilConnectDone(const sptr<UIExtensionConnectModuleTestConnection> &connection)
{
    std::unique_lock<std::mutex> lock(connection->connectMutex_);
    auto waitStatus = connection->connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [connection]() {
            return connection->connectFinished_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(connection->connectFinished_, true);
}

void UIExtensionConnectModuleTest::WaitUntilDisConnectDone(
    const sptr<UIExtensionConnectModuleTestConnection> &connection)
{
    std::unique_lock<std::mutex> lock(connection->connectMutex_);
    auto waitStatus = connection->connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [connection]() {
            return connection->disConnectFinished_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(connection->disConnectFinished_, true);
}

void UIExtensionConnectModuleTest::WaitUntilProcessCreated(const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [observer]() {
            return observer->processCreated_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processCreated_, true);
}

void UIExtensionConnectModuleTest::WaitUntilProcessDied(const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [observer]() {
            return observer->processDied_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processDied_, true);
}

void UIExtensionConnectModuleTest::CheckProcessNotDied(const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [observer]() {
            return observer->processDied_;
        });
    EXPECT_EQ(waitStatus, false);
    EXPECT_EQ(observer->processDied_, false);
}

void UIExtensionConnectModuleTest::WaitUntilAbilityForeground(
    const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [observer]() {
            return observer->processForegrounded_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processForegrounded_, true);
}

void UIExtensionConnectModuleTest::WaitUntilAbilityBackground(
    const sptr<UIExtensionConnectModuleTestObserver> &observer)
{
    std::unique_lock<std::mutex> lock(observer->observerMutex_);
    auto waitStatus = observer->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
        [observer]() {
            return observer->processBackgrounded_;
        });
    EXPECT_EQ(waitStatus, true);
    EXPECT_EQ(observer->processBackgrounded_, true);
}

/**
 * @tc.name: ConnectUIExtensionAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionConnectModuleTest, ConnectUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0100 start.");
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    auto client_ = AbilityManagerClient::GetInstance();
    EXPECT_NE(client_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId), ERR_OK);

    Want userWant;
    AppExecFwk::ElementName userElement("0", USER_BUNDLE_NAME, USER_ABILITY_NAME, USER_MODULE_NAME);
    userWant.SetElement(userElement);
    auto ret = client_->StartAbility(userWant);
    EXPECT_EQ(ret, ERR_OK);

    ret = client_->DisconnectAbility(connect);
    EXPECT_NE(ret, ERR_OK);

    sptr<IRemoteObject> token = nullptr;
    ret = client_->GetTopAbility(token);
    EXPECT_NE(ret, ERR_OK);

    int resultCode = 0;
    Want resultWant;
    ret = client_->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0100 finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionConnectModuleTest, ConnectUIExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0200 start.");
    auto client_ = AbilityManagerClient::GetInstance();
    Want userWant;
    AppExecFwk::ElementName userElement("0", USER_BUNDLE_NAME, USER_ABILITY_NAME, USER_MODULE_NAME);
    userWant.SetElement(userElement);
    auto ret = client_->StartAbility(userWant);
    EXPECT_EQ(ret, ERR_OK);

    sptr<IRemoteObject> token = nullptr;
    ret = client_->GetTopAbility(token);
    EXPECT_NE(ret, ERR_OK);

    ret = client_->MinimizeAbility(token);
    EXPECT_NE(ret, ERR_OK);

    int resultCode = 0;
    Want resultWant;
    ret = client_->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0200 finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0400
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionConnectModuleTest, ConnectUIExtensionAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0300 start.");
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    auto client_ = AbilityManagerClient::GetInstance();
    EXPECT_NE(client_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId), ERR_OK);

    auto ret = client_->DisconnectAbility(connect);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0300 finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0500
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionConnectModuleTest, ConnectUIExtensionAbility_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0400 start.");
    Want want;
    sptr<IAbilityConnection> connect = nullptr;
    sptr<SessionInfo> sessionInfo = nullptr;
    int32_t userId = 1;
    auto client_ = AbilityManagerClient::GetInstance();
    EXPECT_NE(client_->ConnectUIExtensionAbility(want, connect, sessionInfo, userId), ERR_OK);

    auto ret = client_->DisconnectAbility(connect);
    EXPECT_NE(ret, ERR_OK);

    Want userWant;
    AppExecFwk::ElementName userElement("0", USER_BUNDLE_NAME, USER_ABILITY_NAME, USER_MODULE_NAME);
    userWant.SetElement(userElement);
    ret = client_->StartAbility(userWant);
    EXPECT_EQ(ret, ERR_OK);

    ret = client_->DisconnectAbility(connect);
    EXPECT_NE(ret, ERR_OK);

    sptr<IRemoteObject> token = nullptr;
    ret = client_->GetTopAbility(token);
    EXPECT_NE(ret, ERR_OK);

    ret = client_->MinimizeAbility(token);
    EXPECT_NE(ret, ERR_OK);

    int resultCode = 0;
    Want resultWant;
    ret = client_->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ConnectUIExtensionAbility_0400 finish.");
}
} // namespace AAFwk
} // namespace OHOS
