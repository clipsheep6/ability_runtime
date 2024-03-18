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
#include <singleton.h>
#define private public
#include "ability.h"
#include "ability_context.h"
#include "context_container.h"
#include "context_deal.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_manager_service.h"
#include "mock_system_ability_manager.h"
#include "ohos_application.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();

class ContextContainerByMockBmsTest : public testing::Test {
public:
    ContextContainerByMockBmsTest() : context_(nullptr), contextDeal_(nullptr)
    {}
    ~ContextContainerByMockBmsTest()
    {}
    std::shared_ptr<AbilityContext> context_ = nullptr;
    std::shared_ptr<ContextDeal> contextDeal_ = nullptr;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void MockBundleInstaller();
};

void ContextContainerByMockBmsTest::SetUpTestCase(void)
{}

void ContextContainerByMockBmsTest::TearDownTestCase(void)
{}

void ContextContainerByMockBmsTest::SetUp(void)
{
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    context_ = std::make_shared<AbilityContext>();
    contextDeal_ = std::make_shared<ContextDeal>();
}

void ContextContainerByMockBmsTest::TearDown(void)
{
    context_ = nullptr;
    contextDeal_ = nullptr;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void ContextContainerByMockBmsTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_)).WillOnce(testing::Invoke(mockGetSystemAbility));
}

/**
 * @tc.number: AppExecFwk_ContextContainer_GetAppType_0100
 * @tc.name: GetAppType
 * @tc.desc: Test whether AttachBaseContext is called normally,
 *           and verify whether the return value of GetAppType is correct.
 */
HWTEST_F(ContextContainerByMockBmsTest, AppExecFwk_ContextContainer_GetAppType_0100, Function | MediumTest | Level1)
{
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleName = "hello";
    contextDeal_->SetApplicationInfo(info);
    context_->AttachBaseContext(contextDeal_);
    MockBundleInstaller();
    EXPECT_CALL(*mockBundleMgr, GetAppType(testing::_))
        .WillOnce(testing::Return("system"))
        .WillRepeatedly(testing::Return("system"));
    std::string path = context_->GetAppType();
    std::string appType = "system";

    EXPECT_STREQ(context_->GetAppType().c_str(), appType.c_str());
}
}  // namespace AppExecFwk
}  // namespace OHOS