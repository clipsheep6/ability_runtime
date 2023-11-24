/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_manager_service.h"
#include "mock_quick_fix_util.h"
#include "mock_system_ability_manager.h"
#include "permission_verification.h"
#include "quick_fix_error_utils.h"
#define private public
#include "iservice_registry.h"
#include "quick_fix_manager_service.h"
#undef private
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<AppExecFwk::BundleMgrService> mockBundleMgr = new (std::nothrow) AppExecFwk::BundleMgrService();
class QuickFixManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<QuickFixManagerService> quickFixMs_ = nullptr;
    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
};

void QuickFixManagerServiceTest::SetUpTestCase(void)
{}

void QuickFixManagerServiceTest::TearDownTestCase(void)
{}

void QuickFixManagerServiceTest::SetUp()
{
    quickFixMs_ = QuickFixManagerService::GetInstance();
    ASSERT_NE(quickFixMs_, nullptr);

    auto ret = quickFixMs_->Init();
    ASSERT_TRUE(ret);
    ASSERT_NE(quickFixMs_->eventRunner_, nullptr);
    ASSERT_NE(quickFixMs_->eventHandler_, nullptr);

    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);

    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void QuickFixManagerServiceTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyQuickFix_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [&](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return mockBundleMgr->AsObject();
        } else {
            return iSystemAbilityMgr_->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_))
        .WillOnce(testing::Invoke(mockGetSystemAbility))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));

    std::vector<std::string> quickFixFiles;
    auto ret = quickFixMs_->ApplyQuickFix(quickFixFiles);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerServiceTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [&](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return mockBundleMgr->AsObject();
        } else {
            return iSystemAbilityMgr_->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, GetSystemAbility(testing::_))
        .WillOnce(testing::Invoke(mockGetSystemAbility))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
    std::string bundleName = "com.ohos.quickfix";
    ApplicationQuickFixInfo quickFixInfo;
    auto ret = quickFixMs_->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
    EXPECT_EQ(ret, QUICK_FIX_OK);
    EXPECT_EQ(quickFixInfo.bundleName, "com.ohos.quickfix");
    EXPECT_EQ(quickFixInfo.bundleVersionCode, static_cast<uint32_t>(1000));
    EXPECT_EQ(quickFixInfo.bundleVersionName, "1.0.0");

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: ApplyAndRemoveTask_0100
 * @tc.desc: AddApplyTask and RemoveApplyTask
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerServiceTest, ApplyAndRemoveTask_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    EXPECT_NE(quickFixMs_, nullptr);
    quickFixMs_->RemoveApplyTask(nullptr);
    quickFixMs_->AddApplyTask(nullptr);

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, nullptr, nullptr, nullptr);
    quickFixMs_->RemoveApplyTask(applyTask);
    quickFixMs_->AddApplyTask(applyTask);
    quickFixMs_->AddApplyTask(applyTask);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0100
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, RevokeQuickFix_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    std::string bundleName = "com.ohos.quickfix";
    auto ret = quickFixMs_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0200
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, RevokeQuickFix_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    std::string bundleName = "com.ohos.quickfix";
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, nullptr, nullptr, nullptr);
    applyTask->InitRevokeTask(bundleName, true);

    auto ret = quickFixMs_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_DEPLOYING_TASK);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: RevokeQuickFix_0300
 * @tc.desc: RevokeQuickFix
 * @tc.type: FUNC
 */
HWTEST_F(QuickFixManagerServiceTest, RevokeQuickFix_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    std::string bundleName = "test.bundle.name";
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(nullptr, nullptr, nullptr, nullptr);
    applyTask->InitRevokeTask(bundleName, true);

    auto ret = quickFixMs_->RevokeQuickFix(bundleName);
    EXPECT_EQ(ret, QUICK_FIX_GET_BUNDLE_INFO_FAILED);

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS