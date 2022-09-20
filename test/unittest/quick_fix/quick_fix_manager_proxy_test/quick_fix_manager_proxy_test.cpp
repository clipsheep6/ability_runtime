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
#define private public
#include "mock_quick_fix_manager_stub.h"
#include "mock_quick_fix_util.h"
#include "quick_fix_manager_proxy.h"
#undef private
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixManagerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixManagerStub> mockQuickFixMgrService_ = nullptr;
    sptr<AAFwk::QuickFixManagerProxy> quickFixMgrProxy_ = nullptr;
    std::shared_ptr<QuickFixUtil> quickFixUtil_ = nullptr;
};

void QuickFixManagerProxyTest::SetUpTestCase(void)
{}

void QuickFixManagerProxyTest::TearDownTestCase(void)
{}

void QuickFixManagerProxyTest::SetUp()
{
    mockQuickFixMgrService_ = new MockQuickFixManagerStub();
    ASSERT_NE(mockQuickFixMgrService_, nullptr);

    quickFixMgrProxy_ = new QuickFixManagerProxy(mockQuickFixMgrService_);
    ASSERT_NE(quickFixMgrProxy_, nullptr);

    quickFixUtil_ = std::make_shared<QuickFixUtil>();
    ASSERT_NE(quickFixUtil_, nullptr);

    sptr<IRemoteObject> bundleObject = new (std::nothrow) AppExecFwk::BundleMgrService();
    quickFixUtil_->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, bundleObject);
}

void QuickFixManagerProxyTest::TearDown()
{}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: apply quick fix.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerProxyTest, ApplyQuickFix_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockQuickFixMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockQuickFixMgrService_.GetRefPtr(), &MockQuickFixManagerStub::InvokeSendRequest));

    std::vector<std::string> quickFixFiles;
    quickFixFiles.push_back("/data/storage/el2/base/entry1.hqf");
    quickFixFiles.push_back("/data/storage/el2/base/entry2.hqf");
    quickFixMgrProxy_->ApplyQuickFix(quickFixFiles);

    EXPECT_EQ(mockQuickFixMgrService_->code_,
        static_cast<uint32_t>(IQuickFixManager::QuickFixMgrCmd::ON_APPLY_QUICK_FIX));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: Get applyed quick fix info.
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerProxyTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockQuickFixMgrService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockQuickFixMgrService_.GetRefPtr(), &MockQuickFixManagerStub::InvokeSendRequest));

    std::string bundleName = "com.ohos.quickfix";
    ApplicationQuickFixInfo quickFixInfo;
    quickFixMgrProxy_->GetApplyedQuickFixInfo(bundleName, quickFixInfo);

    EXPECT_EQ(mockQuickFixMgrService_->code_,
        static_cast<uint32_t>(IQuickFixManager::QuickFixMgrCmd::ON_GET_APPLYED_QUICK_FIX_INFO));

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS