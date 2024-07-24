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
#include "connection_observer_controller.h"
#undef private
#include "connection_observer_errors.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
class ConnectionObserverControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConnectionObserverControllerTest::SetUpTestCase(void)
{}
void ConnectionObserverControllerTest::TearDownTestCase(void)
{}
void ConnectionObserverControllerTest::SetUp(void)
{}
void ConnectionObserverControllerTest::TearDown(void)
{}

/*
 * Feature: ConnectionObserverController
 * Function: AddObserver
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController AddObserver
 * EnvConditions: NA
 * CaseDescription: Verify AddObserver
 */
HWTEST_F(ConnectionObserverControllerTest, AddObserver_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    sptr<IConnectionObserver> observer = nullptr;
    auto res = connectionObserverController->AddObserver(observer);
    EXPECT_EQ(res, ERR_INVALID_OBSERVER);
}

/*
 * Feature: ConnectionObserverController
 * Function: RemoveObserver
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController RemoveObserver
 * EnvConditions: NA
 * CaseDescription: Verify RemoveObserver
 */
HWTEST_F(ConnectionObserverControllerTest, RemoveObserver_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    sptr<IConnectionObserver> observer = nullptr;
    connectionObserverController->RemoveObserver(observer);
}

/*
 * Feature: ConnectionObserverController
 * Function: NotifyExtensionConnected
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController NotifyExtensionConnected
 * EnvConditions: NA
 * CaseDescription: Verify NotifyExtensionConnected
 */
HWTEST_F(ConnectionObserverControllerTest, NotifyExtensionConnected_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    ConnectionData data;
    connectionObserverController->NotifyExtensionConnected(data);
}

/*
 * Feature: ConnectionObserverController
 * Function: NotifyExtensionDisconnected
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController NotifyExtensionDisconnected
 * EnvConditions: NA
 * CaseDescription: Verify NotifyExtensionDisconnected
 */
HWTEST_F(ConnectionObserverControllerTest, NotifyExtensionDisconnected_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    ConnectionData data;
    connectionObserverController->NotifyExtensionDisconnected(data);
}

#ifdef WITH_DLP
/*
 * Feature: ConnectionObserverController
 * Function: NotifyDlpAbilityOpened
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController NotifyDlpAbilityOpened
 * EnvConditions: NA
 * CaseDescription: Verify NotifyDlpAbilityOpened
 */
HWTEST_F(ConnectionObserverControllerTest, NotifyDlpAbilityOpened_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    DlpStateData data;
    connectionObserverController->NotifyDlpAbilityOpened(data);
}

/*
 * Feature: ConnectionObserverController
 * Function: NotifyDlpAbilityClosed
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController NotifyDlpAbilityClosed
 * EnvConditions: NA
 * CaseDescription: Verify NotifyDlpAbilityClosed
 */
HWTEST_F(ConnectionObserverControllerTest, NotifyDlpAbilityClosed_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    DlpStateData data;
    connectionObserverController->NotifyDlpAbilityClosed(data);
}
#endif // WITH_DLP

/*
 * Feature: ConnectionObserverController
 * Function: GetObservers
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController GetObservers
 * EnvConditions: NA
 * CaseDescription: Verify GetObservers
 */
HWTEST_F(ConnectionObserverControllerTest, GetObservers_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    std::vector<sptr<AbilityRuntime::IConnectionObserver>> observers;
    auto res = connectionObserverController->GetObservers();
    EXPECT_EQ(res, observers);
}

/*
 * Feature: ConnectionObserverController
 * Function: HandleRemoteDied
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController HandleRemoteDied
 * EnvConditions: NA
 * CaseDescription: Verify HandleRemoteDied
 */
HWTEST_F(ConnectionObserverControllerTest, HandleRemoteDied_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    wptr<IRemoteObject> remote;
    connectionObserverController->HandleRemoteDied(remote);
}

/*
 * Feature: ObserverDeathRecipient
 * Function: OnRemoteDied
 * SubFunction: NA
 * FunctionPoints: ConnectionObserverController OnRemoteDied
 * EnvConditions: NA
 * CaseDescription: Verify OnRemoteDied
 */
HWTEST_F(ConnectionObserverControllerTest, OnRemoteDied_001, TestSize.Level1)
{
    auto connectionObserverController = std::make_shared<ConnectionObserverController>();
    ASSERT_NE(connectionObserverController, nullptr);
    ConnectionObserverController::ObserverDeathRecipient::ObserverDeathHandler handler;
    auto observerDeathRecipient = std::make_shared<ConnectionObserverController::ObserverDeathRecipient>(handler);
    wptr<IRemoteObject> remote;
    observerDeathRecipient->OnRemoteDied(remote);
}
}  // namespace AAFwk
}  // namespace OHOS
