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
#define private public
#include "mock_quick_fix_callback_stub.h"
#include "quick_fix_callback_proxy.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class QuickFixCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockQuickFixCallbackStub> mockCallbackService_ = nullptr;
    sptr<QuickFixCallbackProxy> quickFixMgrProxy_ = nullptr;
};

void QuickFixCallbackProxyTest::SetUpTestCase(void)
{}

void QuickFixCallbackProxyTest::TearDownTestCase(void)
{}

void QuickFixCallbackProxyTest::SetUp()
{
    mockCallbackService_ = new MockQuickFixCallbackStub();
    ASSERT_NE(mockCallbackService_, nullptr);

    quickFixMgrProxy_ = new QuickFixCallbackProxy(mockCallbackService_);
    ASSERT_NE(quickFixMgrProxy_, nullptr);
}

void QuickFixCallbackProxyTest::TearDown()
{}

/**
 * @tc.name: OnLoadPatchDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackProxyTest, OnLoadPatchDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockCallbackService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockCallbackService_.GetRefPtr(), &MockQuickFixCallbackStub::InvokeSendRequest));

    int32_t resultCode = 0;
    quickFixMgrProxy_->OnLoadPatchDone(resultCode);

    EXPECT_EQ(mockCallbackService_->code_,
        static_cast<uint32_t>(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_LOAD_PATCH));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnUnloadPatchDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackProxyTest, OnUnloadPatchDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockCallbackService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockCallbackService_.GetRefPtr(), &MockQuickFixCallbackStub::InvokeSendRequest));

    int32_t resultCode = 0;
    quickFixMgrProxy_->OnUnloadPatchDone(resultCode);

    EXPECT_EQ(mockCallbackService_->code_,
        static_cast<uint32_t>(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_UNLOAD_PATCH));

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @tc.name: OnReloadPageDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 * @tc.require: issueI5OD2E
 */
HWTEST_F(QuickFixCallbackProxyTest, OnReloadPageDone_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);

    EXPECT_CALL(*mockCallbackService_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mockCallbackService_.GetRefPtr(), &MockQuickFixCallbackStub::InvokeSendRequest));

    int32_t resultCode = 0;
    quickFixMgrProxy_->OnReloadPageDone(resultCode);

    EXPECT_EQ(mockCallbackService_->code_,
        static_cast<uint32_t>(IQuickFixCallback::QuickFixCallbackCmd::ON_NOTIFY_RELOAD_PAGE));

    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS