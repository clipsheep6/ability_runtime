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

#include "assert_fault_callback.h"
#include "assert_fault_task_thread.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
AssertFaultCallback::AssertFaultCallback(const std::weak_ptr<AssertFaultTaskThread> &assertFaultThread)
{
    assertFaultThread_ = assertFaultThread;
    status_ = AAFwk::UserStatus::ASSERT_TERMINATE;
}
int32_t AssertFaultCallback::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AssertFaultCallback::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(MessageCode::NOTIFY_DEBUG_ASSERT_RESULT)) {
        auto status = static_cast<AAFwk::UserStatus>(data.ReadInt32());
        NotifyDebugAssertResult(status);
        return NO_ERROR;
    }

    HILOG_WARN("Unexpected event ID, for default handling.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

void AssertFaultCallback::NotifyDebugAssertResult(AAFwk::UserStatus status)
{
    HILOG_DEBUG("Notify user action result to assert fault thread.");
    status_ = status;
    auto assertThread = assertFaultThread_.lock();
    if (assertThread == nullptr) {
        HILOG_ERROR("Failed to obtain assert fault thread.");
        return;
    }
    assertThread->NotifyReleaseLongWaiting();
}

AAFwk::UserStatus AssertFaultCallback::GetAssertResult()
{
    return status_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
