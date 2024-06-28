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

#include <cstdlib>
#include <gtest/gtest.h>

#define private public
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "main_thread.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_overlay_manager.h"
#include "mock_system_ability_manager.h"
#include "ohos_application.h"
#include "process_info.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<BundleMgrService> mockBundleMgr = new (std::nothrow) BundleMgrService();

class MainThreadByMockBmsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MockBundleInstaller();
    sptr<MainThread> mainThread_ = nullptr;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
};

void MainThreadByMockBmsTest::SetUpTestCase() {}

void MainThreadByMockBmsTest::TearDownTestCase() {}

void MainThreadByMockBmsTest::SetUp()
{
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ASSERT_NE(runner, nullptr);
    mainThread_ = sptr<MainThread>(new (std::nothrow) MainThread());
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->Init(runner);
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void MainThreadByMockBmsTest::TearDown()
{
    mainThread_->applicationForDump_.reset();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void MainThreadByMockBmsTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        GTEST_LOG_(INFO) << "MockBundleInstaller systemAbilityId: " << systemAbilityId << " )";
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID || systemAbilityId == APP_MGR_SERVICE_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

/**
 * @tc.name: SetNativeLibPath_0100
 * @tc.desc: set native lib path.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadByMockBmsTest, SetNativeLibPath_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration config;
    AppLaunchData launchData;
    ProcessInfo processInfo("test_quickfix", 9999);
    ApplicationInfo appInfo;
    appInfo.name = "MainAbility";
    appInfo.bundleName = "com.ohos.quickfix";
    launchData.SetApplicationInfo(appInfo);
    launchData.SetProcessInfo(processInfo);
    MockBundleInstaller();
    // SetNativeLibPath is implemented in anonymous space, called by HandleLaunchApplication
    mainThread_->HandleLaunchApplication(launchData, config);
    ASSERT_NE(mainThread_->application_, nullptr);
    EXPECT_NE(mainThread_->application_->abilityRuntimeContext_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetOverlayModuleInfos_0100
 * @tc.desc: Get overlay paths form mock bms.
 * @tc.type: FUNC
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadByMockBmsTest, GetOverlayModuleInfos_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string moduleName = "entry";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.priority = 99;
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;
    MockBundleInstaller();
    int result = mainThread_->GetOverlayModuleInfos(bundleName, moduleName, overlayModuleInfos);
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(overlayModuleInfos.size() == 1);
}
} // namespace AppExecFwk
} // namespace OHOS