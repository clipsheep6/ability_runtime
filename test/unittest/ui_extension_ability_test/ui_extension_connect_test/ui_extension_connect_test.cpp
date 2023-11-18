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

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "ability_util.h"
#include "application_state_observer_stub.h"
#include "hilog_wrapper.h"
#include "nativetoken_kit.h"
#include "session_info.h"
#include "token_setproc.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int CONNECT_TIMEOUT_MS = 10 * 1000;
const int32_t MAIN_USER_ID = 100;

static void SetNativeToken()
{
    uint64_t tokenId;
    const char **perms = new const char *[1];
    perms[0] = "ohos.permission.CONNECT_UI_EXTENSION_ABILITY";
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
    delete[] perms;
}
} // namespace

class UIExtensionAbilityConnectTestObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    UIExtensionAbilityConnectTestObserver() = default;
    ~UIExtensionAbilityConnectTestObserver() = default;

    void ResetStatus();

    std::condition_variable observerCondation_;
    std::mutex observerMutex_;
    bool processCreated_ = false;
    bool processForegrounded_ = false;
    bool processBackgrounded_ = false;
    bool processDied_ = false;

private:
    void OnProcessCreated(const AppExecFwk::ProcessData &processData) override;
    void OnProcessStateChanged(const AppExecFwk::ProcessData &processData) override;
    void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
};

void UIExtensionAbilityConnectTestObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    HILOG_INFO("called");
    std::unique_lock<std::mutex> lock(observerMutex_);
    if (processData.bundleName == "com.ohos.uiextensionprovider") {
        processCreated_ = true;
    }
    observerCondation_.notify_one();
}

void UIExtensionAbilityConnectTestObserver::OnProcessStateChanged(const AppExecFwk::ProcessData &processData)
{
    HILOG_INFO("called");
    std::unique_lock<std::mutex> lock(observerMutex_);
    if (processData.bundleName == "com.ohos.uiextensionprovider") {
        if (processData.state == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            processForegrounded_ = true;
        } else if (processData.state == AppExecFwk::AppProcessState::APP_STATE_BACKGROUND) {
            processBackgrounded_ = true;
        }
    }
    observerCondation_.notify_one();
}

void UIExtensionAbilityConnectTestObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    HILOG_INFO("called");
    std::unique_lock<std::mutex> lock(observerMutex_);
    if (processData.bundleName == "com.ohos.uiextensionprovider") {
        processDied_ = true;
    }
    observerCondation_.notify_one();
}

void UIExtensionAbilityConnectTestObserver::ResetStatus()
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    bool processCreated_ = false;
    bool processForegrounded_ = false;
    bool processBackgrounded_ = false;
    bool processDied_ = false;
}

class UIExtensionAbilityConnectTest : public testing::Test,
                                      public AbilityConnectionStub {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<UIExtensionAbilityConnectTestObserver> observer_ = nullptr;
    std::condition_variable connectCondation_;
    std::mutex connectMutex_;
    bool connectFinished_ = false;
    bool disConnectFinished_ = false;

private:
    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;

    void ResetStatus();
};

void UIExtensionAbilityConnectTest::SetUpTestCase(void)
{}

void UIExtensionAbilityConnectTest::TearDownTestCase(void)
{}

void UIExtensionAbilityConnectTest::SetUp()
{
    ResetStatus();
    observer_ = std::make_unique<UIExtensionAbilityConnectTestObserver>();
    if (observer_ != nullptr) {
        observer_->ResetStatus();
    }
}

void UIExtensionAbilityConnectTest::TearDown()
{}

void UIExtensionAbilityConnectTest::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    HILOG_INFO("called");
    std::unique_lock<std::mutex> lock(connectMutex_);
    connectFinished_ = true;
    connectCondation_.notify_one();
}

void UIExtensionAbilityConnectTest::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode)
{
    HILOG_INFO("called");
    std::unique_lock<std::mutex> lock(connectMutex_);
    disConnectFinished_ = true;
    connectCondation_.notify_one();
}

void UIExtensionAbilityConnectTest::ResetStatus()
{
    std::unique_lock<std::mutex> lock(connectMutex_);
    connectFinished_ = false;
    disConnectFinished_ = false;
}

/**
 * @tc.name: PermissionCheck_0100
 * @tc.desc: permission check test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionAbilityConnectTest, PermissionCheck_0100, TestSize.Level1)
{
    HILOG_INFO("start.");
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->hostBundleName = "com.ohos.uiextensionuser";
    sessionInfo->uiExtensionAbilityId = 0;
    ASSERT_NE(sessionInfo, nullptr);

    auto ret = AbilityManagerClient::GetInstance()->ConnectUIExtensionAbility(providerWant, this, sessionInfo);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    HILOG_INFO("finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionAbilityConnectTest, ConnectUIExtensionAbility_0100, TestSize.Level1)
{
    HILOG_INFO("start.");

    auto currentId = GetSelfTokenID();
    SetNativeToken();

    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->hostBundleName = "com.ohos.uiextensionuser";
    sessionInfo->uiExtensionAbilityId = 0;
    ASSERT_NE(sessionInfo, nullptr);

    // Connect ui extension ability firstly.
    auto ret = AbilityManagerClient::GetInstance()->ConnectUIExtensionAbility(providerWant, this, sessionInfo);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until OnAbilityConnectDone has triggered.
    {
        std::unique_lock<std::mutex> lock(connectMutex_);
        auto waitStatus = connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return connectFinished_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(connectFinished_, true);
        EXPECT_NE(sessionInfo->uiExtensionAbilityId, 0);
    }

    // Even though sessionInfo has geneated, how to send to ui extension user?

    // start ui extension user
    Want userWant;
    AppExecFwk::ElementName userElement("0", "com.ohos.uiextensionuser", "EntryAbility", "entry");
    userWant.SetElement(userElement);
    ret = AbilityManagerClient::GetInstance()->StartAbility(userWant);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until ability has foregrounded
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processForegrounded_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processForegrounded_, true);
    }

    // Disconnect ui extension ability.
    // wish can't be destroyed, cause there exist ui extension component.
    ret = AbilityManagerClient::GetInstance()->DisconnectAbility(this);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until OnAbilityDisconnectDone has triggered.
    {
        std::unique_lock<std::mutex> lock(connectMutex_);
        auto waitStatus = connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return disConnectFinished_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(disConnectFinished_, true);
    }

    // Destroy ui extension user, this testcase should executed in screen-on.
    sptr<IRemoteObject> token = nullptr;
    ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
    EXPECT_EQ(ret, ERR_OK);

    int resultCode = 0;
    Want resultWant;
    ret = AbilityManagerClient::GetInstance()->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until ability has terminate
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processDied_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processDied_, true);
    }

    SetSelfTokenID(currentId);
    HILOG_INFO("finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionAbilityConnectTest, ConnectUIExtensionAbility_0200, TestSize.Level1)
{
    HILOG_INFO("start.");

    auto currentId = GetSelfTokenID();
    SetNativeToken();

    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->hostBundleName = "com.ohos.uiextensionuser";
    sessionInfo->uiExtensionAbilityId = 0;
    ASSERT_NE(sessionInfo, nullptr);

    // Connect ui extension ability firstly.
    auto ret = AbilityManagerClient::GetInstance()->ConnectUIExtensionAbility(providerWant, this, sessionInfo);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until OnAbilityConnectDone has triggered.
    {
        std::unique_lock<std::mutex> lock(connectMutex_);
        auto waitStatus = connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return connectFinished_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(connectFinished_, true);
        EXPECT_NE(sessionInfo->uiExtensionAbilityId, 0);
    }

    // start ui extension user
    Want userWant;
    AppExecFwk::ElementName userElement("0", "com.ohos.uiextensionuser", "EntryAbility", "entry");
    userWant.SetElement(userElement);
    ret = AbilityManagerClient::GetInstance()->StartAbility(userWant);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until ability has foregrounded
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processForegrounded_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processForegrounded_, true);
    }

    // Destroy ui extension user.
    sptr<IRemoteObject> token = nullptr;
    ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
    EXPECT_EQ(ret, ERR_OK);

    int resultCode = 0;
    Want resultWant;
    ret = AbilityManagerClient::GetInstance()->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until ability has background
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processBackgrounded_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processBackgrounded_, true);
    }

    // Disconnect ui extension ability.
    ret = AbilityManagerClient::GetInstance()->DisconnectAbility(this);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until OnAbilityDisconnectDone has triggered.
    {
        std::unique_lock<std::mutex> lock(connectMutex_);
        auto waitStatus = connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return disConnectFinished_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(disConnectFinished_, true);
    }

    SetSelfTokenID(currentId);
    HILOG_INFO("finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionAbilityConnectTest, ConnectUIExtensionAbility_0300, TestSize.Level1)
{
    HILOG_INFO("start.");

    // start ui extension user
    Want userWant;
    AppExecFwk::ElementName userElement("0", "com.ohos.uiextensionuser", "EntryAbility", "entry");
    userWant.SetElement(userElement);
    auto ret = AbilityManagerClient::GetInstance()->StartAbility(userWant);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until ability has foregrounded
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processForegrounded_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processForegrounded_, true);
    }

    // Destroy ui extension user.
    sptr<IRemoteObject> token = nullptr;
    ret = AbilityManagerClient::GetInstance()->GetTopAbility(token);
    EXPECT_EQ(ret, ERR_OK);

    int resultCode = 0;
    Want resultWant;
    ret = AbilityManagerClient::GetInstance()->TerminateAbility(token, resultCode, &resultWant);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until ability has terminate
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processDied_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processDied_, true);
    }

    HILOG_INFO("finish.");
}

/**
 * @tc.name: ConnectUIExtensionAbility_0400
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionAbilityConnectTest, ConnectUIExtensionAbility_0400, TestSize.Level1)
{
    HILOG_INFO("start.");

    auto currentId = GetSelfTokenID();
    SetNativeToken();

    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    sessionInfo->hostBundleName = "com.ohos.uiextensionuser";
    sessionInfo->uiExtensionAbilityId = 0;
    ASSERT_NE(sessionInfo, nullptr);

    // Connect ui extension ability firstly.
    auto ret = AbilityManagerClient::GetInstance()->ConnectUIExtensionAbility(providerWant, this, sessionInfo);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until OnAbilityConnectDone has triggered.
    {
        std::unique_lock<std::mutex> lock(connectMutex_);
        auto waitStatus = connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return connectFinished_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(connectFinished_, true);
        EXPECT_NE(sessionInfo->uiExtensionAbilityId, 0);
    }

    // Disconnect ui extension ability.
    // wish can't be destroyed, cause there exist ui extension component.
    ret = AbilityManagerClient::GetInstance()->DisconnectAbility(this);
    EXPECT_EQ(ret, ERR_OK);

    // Wait until OnAbilityDisconnectDone has triggered.
    {
        std::unique_lock<std::mutex> lock(connectMutex_);
        auto waitStatus = connectCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return disConnectFinished_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(disConnectFinished_, true);
    }

    // Wait until ability has terminate
    {
        std::unique_lock<std::mutex> lock(observer_->observerMutex_);
        auto waitStatus = observer_->observerCondation_.wait_for(lock, std::chrono::milliseconds(CONNECT_TIMEOUT_MS),
            [this]() {
                return observer_->processDied_;
            });
        EXPECT_EQ(waitStatus, true);
        EXPECT_EQ(observer_->processDied_, true);
    }

    SetSelfTokenID(currentId);
    HILOG_INFO("finish.");
}
} // namespace AAFwk
} // namespace OHOS