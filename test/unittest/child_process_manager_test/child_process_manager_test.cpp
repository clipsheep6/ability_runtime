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

#define private public
#include "app_utils.h"
#include "child_process_manager.h"
#include "mock_bundle_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ChildProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildProcessManagerTest::SetUpTestCase()
{
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    auto sysMgr = DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (sysMgr == nullptr) {
        GTEST_LOG_(ERROR) << "Failed to get ISystemAbilityManager.";
        return;
    }
    sysMgr->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void ChildProcessManagerTest::TearDownTestCase()
{}

void ChildProcessManagerTest::SetUp()
{}

void ChildProcessManagerTest::TearDown()
{}

/**
 * @tc.number: StartChildProcessBySelfFork_0100
 * @tc.desc: Test StartChildProcessBySelfFork return pid > 0
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessManagerTest, StartChildProcessBySelfFork_0100, TestSize.Level0)
{
    pid_t pid;
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isMultiProcesModelDevice_ = true;
    ChildProcessManager::GetInstance().StartChildProcessBySelfFork("./ets/process/DemoProcess.ts", pid);
    EXPECT_TRUE(pid > 0);
}
}  // namespace AAFwk
}  // namespace OHOS
