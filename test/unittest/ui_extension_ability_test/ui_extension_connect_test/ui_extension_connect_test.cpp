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

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "session_info.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionConnectTestConnection : public AbilityConnectionStub {
public:
    UIExtensionConnectTestConnection() = default;
    virtual ~UIExtensionConnectTestConnection() = default;

private:
    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;
};

void UIExtensionConnectTestConnection::OnAbilityConnectDone(const AppExecFwk::ElementName& element,
    const sptr<IRemoteObject>& remoteObject, int resultCode)
{
    HILOG_INFO("element: %{public}s", element.GetURI().c_str());
}

void UIExtensionConnectTestConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName& element,
    int resultCode)
{
    HILOG_INFO("element: %{public}s", element.GetURI().c_str());
}

class UIExtensionConnectTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionConnectTest::SetUpTestCase(void)
{}

void UIExtensionConnectTest::TearDownTestCase(void)
{}

void UIExtensionConnectTest::SetUp()
{}

void UIExtensionConnectTest::TearDown()
{}

/**
 * @tc.name: PermissionCheck_0100
 * @tc.desc: permission check test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(UIExtensionConnectTest, PermissionCheck_0100, TestSize.Level1)
{
    HILOG_INFO("start.");
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);

    auto connection = sptr<UIExtensionConnectTestConnection>::MakeSptr();
    ASSERT_NE(connection, nullptr);

    auto sessionInfo = sptr<SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);

    auto connectInfo = sptr<UIExtensionAbilityConnectInfo>::MakeSptr();
    ASSERT_NE(connectInfo, nullptr);

    auto ret = AbilityManagerClient::GetInstance()->ConnectUIExtensionAbility(providerWant, connection, sessionInfo,
        DEFAULT_INVAL_VALUE, connectInfo);
    EXPECT_NE(ret, ERR_OK);

    HILOG_INFO("finish.");
}
} // namespace AAFwk
} // namespace OHOS
