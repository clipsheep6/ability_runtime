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

#include "assert_fault_proxy.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
AssertFaultProxy::AssertFaultProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IAssertFaultInterface>(impl)
{}

void AssertFaultProxy::NotifyUserActionResult(AAFwk::UserStatus status)
{
    HILOG_DEBUG("Notify user action result to assert fault application.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(AssertFaultProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(status))) {
        HILOG_ERROR("Write status failed.");
        return;
    }

    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Get remote failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (remote->SendRequest(static_cast<uint32_t>(MessageCode::NOTIFY_USER_ACTION_RESULT), data, reply, option)
        != NO_ERROR) {
        HILOG_ERROR("Remote send request failed.");
    }
}

AssertFaultRemoteDeathRecipient::AssertFaultRemoteDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

void AssertFaultRemoteDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("Callback is nullptr.");
        return;
    }
    handler_(remote);
}
} // namespace AbilityRuntime
} // namespace OHOS
