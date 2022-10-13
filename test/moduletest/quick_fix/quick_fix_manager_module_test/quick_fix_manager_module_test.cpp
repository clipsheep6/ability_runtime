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
#include "mock_bundle_manager.h"
#include "mock_quick_fix_util.h"
#include "quick_fix_error_utils.h"
#define private public
#include "quick_fix_manager_client.h"
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

class QuickFixManagerModuleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
    sptr<QuickFixManagerService> quickFixService_ = nullptr;
    std::shared_ptr<QuickFixManagerClient> quickFixClient_ = nullptr;
};

void QuickFixManagerModuleTest::SetUpTestCase(void)
{}

void QuickFixManagerModuleTest::TearDownTestCase(void)
{}

void QuickFixManagerModuleTest::SetUp()
{
    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);

    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);

    quickFixService_ = QuickFixManagerService::GetInstance();
    ASSERT_NE(quickFixService_, nullptr);

    auto initRet = quickFixService_->Init();
    ASSERT_EQ(initRet, true);

    quickFixClient_ = std::make_shared<QuickFixManagerClient>();
    ASSERT_NE(quickFixClient_, nullptr);
    quickFixClient_->quickFixMgr_ = quickFixService_;
}

void QuickFixManagerModuleTest::TearDown()
{}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerModuleTest, ApplyQuickFix_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    std::vector<std::string> quickFixFiles;
    quickFixFiles.push_back("/data/storage/el2/base/entry1.hqf");
    quickFixFiles.push_back("/data/storage/el2/base/entry2.hqf");
    auto result = quickFixClient_->ApplyQuickFix(quickFixFiles);
    WaitUntilTaskDone(quickFixService_->eventHandler_);
    EXPECT_EQ(result, QUICK_FIX_OK);

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: GetApplyedQuickFixInfo
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerModuleTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    std::string bundleName = "com.ohos.quickfix";
    ApplicationQuickFixInfo quickFixInfo;
    auto result = quickFixClient_->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
    WaitUntilTaskDone(quickFixService_->eventHandler_);
    EXPECT_EQ(result, QUICK_FIX_OK);
    EXPECT_EQ(quickFixInfo.bundleName, "com.ohos.quickfix");
    EXPECT_EQ(quickFixInfo.bundleVersionCode, 1000);
    EXPECT_EQ(quickFixInfo.bundleVersionName, "1.0.0");

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS