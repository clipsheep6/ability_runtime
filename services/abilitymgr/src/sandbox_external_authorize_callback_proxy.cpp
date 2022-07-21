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

#include "sandbox_external_authorize_callback_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
AuthorType SandboxExternalAuthorizeCallbackProxy::GetAuthorization(int sandboxUid, const Want& want)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(SandboxExternalAuthorizeCallbackProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return DENY_START_ABILITY;
    }
    if (!data.WriteInt32(sandboxUid)) {
        HILOG_ERROR("Write int32 failed.");
        return DENY_START_ABILITY;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write want failed.");
        return DENY_START_ABILITY;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote service is null.");
        return DENY_START_ABILITY;
    }

    int32_t requestResult = remote->SendRequest(
        static_cast<uint32_t>(GET_SANDBOX_EXTERNAL_AUTH),
        data, reply, option);
    if (requestResult != 0) {
        HILOG_ERROR("SendRequest fail, result: %{public}d.", requestResult);
        return DENY_START_ABILITY;
    }
    int ret = reply.ReadInt32();
    if (ret < DENY_START_ABILITY || ret > ALLOW_START_ABILITY) {
        return DENY_START_ABILITY;
    }
    return static_cast<AuthorType>(ret);
}
}  // namespace AAFwk
}  // namespace OHOS
