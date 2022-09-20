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
#include "mock_quick_fix_manager_stub.h"
#include "quick_fix_errno_def.h"
#define private public
#include "quick_fix_manager_client.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class QuickFixManagerClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixManagerStub> mockQuickFixMgrService_ = nullptr;
    std::shared_ptr<QuickFixManagerClient> quickFixClient_ = nullptr;
};

void QuickFixManagerClientTest::SetUpTestCase(void)
{}

void QuickFixManagerClientTest::TearDownTestCase(void)
{}

void QuickFixManagerClientTest::SetUp()
{
    mockQuickFixMgrService_ = new MockQuickFixManagerStub();
    ASSERT_NE(mockQuickFixMgrService_, nullptr);

    quickFixClient_ = std::make_shared<QuickFixManagerClient>();
    ASSERT_NE(quickFixClient_, nullptr);
}

void QuickFixManagerClientTest::TearDown()
{}

/**
 * @tc.name: ApplyQuickFix_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, ApplyQuickFix_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    EXPECT_CALL(*mockQuickFixMgrService_, ApplyQuickFix(_)).Times(1);

    std::vector<std::string> quickfixFiles;
    auto ret = quickFixClient_->ApplyQuickFix(quickfixFiles);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @tc.name: GetApplyedQuickFixInfo_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5ODCD
 */
HWTEST_F(QuickFixManagerClientTest, GetApplyedQuickFixInfo_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start", __func__);

    quickFixClient_->quickFixMgr_ = mockQuickFixMgrService_;
    EXPECT_CALL(*mockQuickFixMgrService_, GetApplyedQuickFixInfo(_, _)).Times(1);

    std::string bundleName = "com.ohos.quickfix";
    ApplicationQuickFixInfo quickFixInfo;
    auto ret = quickFixClient_->GetApplyedQuickFixInfo(bundleName, quickFixInfo);
    EXPECT_EQ(ret, QUICK_FIX_OK);

    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @tc.name: ConnectQuickFixManagerService_0100
 * @tc.desc: ApplyQuickFix
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixManagerClientTest, ConnectQuickFixManagerService_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start", __func__);

    auto quickFixMgr = quickFixClient_->ConnectQuickFixManagerService();
    EXPECT_NE(quickFixMgr, nullptr);

    HILOG_INFO("%{public}s end", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS