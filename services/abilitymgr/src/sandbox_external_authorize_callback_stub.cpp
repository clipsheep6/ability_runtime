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

#include "sandbox_external_authorize_callback_stub.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
int32_t SandboxExternalAuthorizeCallbackStub::GetAuthorizationInner(MessageParcel& data, MessageParcel& reply)
{
    int32_t sandboxUid;
    if (!data.ReadInt32(sandboxUid)) {
        HILOG_ERROR("Read int32 fail");
        return ERR_INVALID_STATE;
    }
    Want *resultWant = data.ReadParcelable<Want>();
    if (resultWant == nullptr) {
        HILOG_ERROR("Read Want fail");
        return ERR_INVALID_STATE;
    }

    AuthorType auth = this->GetAuthorization(sandboxUid, *resultWant);
    reply.WriteInt32(auth);

    return NO_ERROR;
}

int SandboxExternalAuthorizeCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    std::u16string descriptor = SandboxExternalAuthorizeCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_INFO("Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<int32_t>(GET_SANDBOX_EXTERNAL_AUTH)) {
        return GetAuthorizationInner(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

AuthorType SandboxExternalAuthorizeCallbackStub::GetAuthorization(int sandboxUid, const Want& want)
{
    return DENY_START_ABILITY;
}
}  // namespace AAFwk
}  // namespace OHOS
