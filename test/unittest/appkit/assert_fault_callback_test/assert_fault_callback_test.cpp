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

#include <cstdlib>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "app_mgr_proxy.h"
#include "app_mgr_stub.h"
#include "main_thread.h"
#include "assert_fault_callback.h"
#include "assert_fault_task_thread.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "mock_bundle_manager.h"
#include "process_info.h"
#include "quick_fix_callback_stub.h"
#include "system_ability_definition.h"
#include "sys_mgr_client.h"
#include "ohos_application.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultCallbackTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<AssertFaultCallback> assertFaultCallback_ = nullptr;
};

void AssertFaultCallbackTest::SetUpTestCase()
{}

void AssertFaultCallbackTest::TearDownTestCase()
{}

void AssertFaultCallbackTest::SetUp()
{
    assertFaultCallback_ = sptr<AssertFaultCallback>(
        new AssertFaultCallback(DelayedSingleton<AbilityRuntime::AssertFaultTaskThread>::GetInstance()));
}

void AssertFaultCallbackTest::TearDown()
{}

/*
 * Feature: AssertFaultCallback
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallback OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify OnRemoteRequest
 */
HWTEST_F(AssertFaultCallbackTest, OnRemoteRequest_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    uint32_t code = static_cast<uint32_t>(AssertFaultCallback::MessageCode::NOTIFY_DEBUG_ASSERT_RESULT);
    std::u16string str = AssertFaultCallback::GetDescriptor();
    str.at(0) -= 1;
    MessageParcel data;
    data.WriteInterfaceToken(str);
    MessageParcel reply;
    MessageOption option;
    auto result = assertFaultCallback_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: AssertFaultCallback
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallback OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify OnRemoteRequest
 */
HWTEST_F(AssertFaultCallbackTest, OnRemoteRequest_0200, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    uint32_t code = static_cast<uint32_t>(AssertFaultCallback::MessageCode::NOTIFY_DEBUG_ASSERT_RESULT);
    MessageParcel data;
    data.WriteInterfaceToken(AssertFaultCallback::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    auto result = assertFaultCallback_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, NO_ERROR);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: AssertFaultCallback
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallback OnRemoteRequest
 * EnvConditions: NA
 * CaseDescription: Verify OnRemoteRequest
 */
HWTEST_F(AssertFaultCallbackTest, OnRemoteRequest_0300, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    uint32_t code = static_cast<uint32_t>(AssertFaultCallback::MessageCode::NOTIFY_DEBUG_ASSERT_RESULT+1);
    MessageParcel data;
    data.WriteInterfaceToken(AssertFaultCallback::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    auto result = assertFaultCallback_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(result, IPC_STUB_UNKNOW_TRANS_ERR);
    HILOG_INFO("%{public}s end.", __func__);
}

/*
 * Feature: AssertFaultCallback
 * Function: NotifyDebugAssertResult
 * SubFunction: NA
 * FunctionPoints: AssertFaultCallback NotifyDebugAssertResult
 * EnvConditions: NA
 * CaseDescription: Verify NotifyDebugAssertResult
 */
HWTEST_F(AssertFaultCallbackTest, NotifyDebugAssertResult_0100, TestSize.Level1)
{
    HILOG_INFO("%{public}s start.", __func__);
    assertFaultCallback_->NotifyDebugAssertResult(AAFwk::ASSERT_RETRY);
    HILOG_INFO("%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS