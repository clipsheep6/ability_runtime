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

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "mock_quick_fix_util.h"
#include "quick_fix_error_utils.h"
#define private public
#include "quick_fix_manager_service.h"
#undef private
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
template<typename F>
static void WaitUntilTaskCalled(const F &f, const std::shared_ptr<AppExecFwk::EventHandler> &handler,
    std::atomic<bool> &taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->PostTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            // if delay more than 1 second, break
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}
} // namespace

class QuickFixManagerApplyTaskTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<QuickFixManagerService> quickFixMs_ = nullptr;
    sptr<AppExecFwk::QuickFixManagerHostImpl> bundleQfMgr_ = nullptr;
    sptr<AppExecFwk::IAppMgr> appMgr_ = nullptr;
    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
};

void QuickFixManagerApplyTaskTest::SetUpTestCase(void)
{}

void QuickFixManagerApplyTaskTest::TearDownTestCase(void)
{}

void QuickFixManagerApplyTaskTest::SetUp()
{
    quickFixMs_ = QuickFixManagerService::GetInstance();
    ASSERT_NE(quickFixMs_, nullptr);

    auto ret = quickFixMs_->Init();
    EXPECT_TRUE(ret);
    EXPECT_NE(quickFixMs_->eventRunner_, nullptr);
    EXPECT_NE(quickFixMs_->eventHandler_, nullptr);

    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);

    bundleQfMgr_ = new (std::nothrow) AppExecFwk::QuickFixManagerHostImpl();
    ASSERT_NE(bundleQfMgr_, nullptr);

    appMgr_ = QuickFixUtil::GetAppManagerProxy();
    ASSERT_NE(appMgr_, nullptr);
}

void QuickFixManagerApplyTaskTest::TearDown()
{}

/**
 * @tc.name: Run_0100
 * @tc.desc: run quick fix task.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskTest, Run_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);

    EXPECT_CALL(*bundleQfMgr_, DeployQuickFix(_, _)).Times(1);
    std::vector<std::string> quickFixFiles;
    applyTask->Run(quickFixFiles);
    WaitUntilTaskDone(quickFixMs_->eventHandler_);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchDeployed_0100
 * @tc.desc: handle patch deployed.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskTest, HandlePatchDeployed_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);

    EXPECT_CALL(*bundleQfMgr_, SwitchQuickFix(_, _, _)).Times(1);
    applyTask->HandlePatchDeployed();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchSwitched_0100
 * @tc.desc: handle patch switched.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskTest, HandlePatchSwitched_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);

    EXPECT_CALL(*bundleQfMgr_, DeleteQuickFix(_, _)).Times(1);
    applyTask->HandlePatchSwitched();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: HandlePatchDeleted_0100
 * @tc.desc: handle patch deleted.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskTest, HandlePatchDeleted_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);

    applyTask->HandlePatchDeleted();
    WaitUntilTaskDone(quickFixMs_->eventHandler_);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: NotifyApplyStatus_0100
 * @tc.desc: handle patch deleted.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerApplyTaskTest, NotifyApplyStatus_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    applyTask->bundleName_ = "testBundleName";
    applyTask->bundleVersionCode_ = 1;
    applyTask->patchVersionCode_ = 100;
    applyTask->NotifyApplyStatus(QUICK_FIX_OK);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: SetQuickFixInfo_0400
 * @tc.desc: Set quick fix info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(QuickFixManagerApplyTaskTest, SetQuickFixInfo_0400, TestSize.Level1)
{
    HILOG_INFO("testcase start.");
    auto applyTask = std::make_shared<QuickFixManagerApplyTask>(bundleQfMgr_, appMgr_,
        quickFixMs_->eventHandler_, quickFixMs_);
    ASSERT_NE(applyTask, nullptr);
    auto quickFixRes = std::make_shared<AppExecFwk::DeployQuickFixResult>();
    quickFixRes->resultCode = 0;
    quickFixRes->bundleName = "com.ohos.quickfix";
    quickFixRes->bundleVersionCode = 100000;
    quickFixRes->patchVersionCode = 100001;
    quickFixRes->isSoContained = true;
    quickFixRes->type = AppExecFwk::QuickFixType::PATCH;
    quickFixRes->moduleNames.emplace_back("entry");
    quickFixRes->moduleNames.emplace_back("feature");
    bool res = applyTask->SetQuickFixInfo(quickFixRes);
    ASSERT_EQ(res, true);
    EXPECT_EQ(applyTask->bundleName_, "com.ohos.quickfix");
    EXPECT_EQ(applyTask->bundleVersionCode_, 100000);
    EXPECT_EQ(applyTask->patchVersionCode_, 100001);
    EXPECT_EQ(applyTask->isSoContained_, true);
    EXPECT_EQ(applyTask->type_, AppExecFwk::QuickFixType::PATCH);
    ASSERT_EQ(applyTask->moduleNames_.size(), 2);
    EXPECT_EQ(applyTask->moduleNames_[0], "entry");
    EXPECT_EQ(applyTask->moduleNames_[1], "feature");
    HILOG_INFO("testcase end.");
}
} // namespace AppExecFwk
} // namespace OHOS
