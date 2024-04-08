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
#include "assert_fault_callback_death_mgr.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultCallbackDeathMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AssertFaultCallbackDeathMgrTest::SetUpTestCase()
{}

void AssertFaultCallbackDeathMgrTest::TearDownTestCase()
{}

void AssertFaultCallbackDeathMgrTest::SetUp()
{}

void AssertFaultCallbackDeathMgrTest::TearDown()
{}

/*
 * Feature: AssertFaultCallbackDeathMgrTest
 * Function: AddAssertFaultCallback
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallbackDeathMgrTest AddAssertFaultCallback
 * EnvConditions: NA
 * CaseDescription: Verify AddAssertFaultCallback
 */
HWTEST_F(AssertFaultCallbackDeathMgrTest, AddAssertFaultCallback_0100, TestSize.Level1)
{
    auto assertFaultCallbackDeathMgr = DelayedSingleton<AbilityRuntime::AssertFaultCallbackDeathMgr>::GetInstance();
    sptr<IRemoteObject> remote;
    assertFaultCallbackDeathMgr->AddAssertFaultCallback(remote);
}

/*
 * Feature: AssertFaultCallbackDeathMgrTest
 * Function: RemoveAssertFaultCallback
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallbackDeathMgrTest RemoveAssertFaultCallback
 * EnvConditions: NA
 * CaseDescription: Verify RemoveAssertFaultCallback
 */
HWTEST_F(AssertFaultCallbackDeathMgrTest, RemoveAssertFaultCallback_0100, TestSize.Level1)
{
    auto assertFaultCallbackDeathMgr = DelayedSingleton<AbilityRuntime::AssertFaultCallbackDeathMgr>::GetInstance();
    sptr<IRemoteObject> remoteObject;
    wptr<IRemoteObject> remote(remoteObject);
    assertFaultCallbackDeathMgr->RemoveAssertFaultCallback(remote);
}

/*
 * Feature: AssertFaultCallbackDeathMgrTest
 * Function: CallAssertFaultCallback
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallbackDeathMgrTest CallAssertFaultCallback
 * EnvConditions: NA
 * CaseDescription: Verify CallAssertFaultCallback
 */
HWTEST_F(AssertFaultCallbackDeathMgrTest, CallAssertFaultCallback_0100, TestSize.Level1)
{
    auto assertFaultCallbackDeathMgr = DelayedSingleton<AbilityRuntime::AssertFaultCallbackDeathMgr>::GetInstance();
    uint64_t assertFaultSessionId = 1;
    assertFaultCallbackDeathMgr->CallAssertFaultCallback(assertFaultSessionId, AAFwk::UserStatus::ASSERT_TERMINATE);
}
}
}