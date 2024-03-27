/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "user_callback_stub.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
UserCallbackStub::UserCallbackStub()
{
    vecMemberFunc_.resize(UserCallbackCmd::CMD_MAX);
    vecMemberFunc_[UserCallbackCmd::ON_STOP_USER_DONE] = &UserCallbackStub::OnStopUserDoneInner;
    vecMemberFunc_[UserCallbackCmd::ON_START_USER_DONE] = &UserCallbackStub::OnStartUserDoneInner;
}

int UserCallbackStub::OnStopUserDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto accountId = data.ReadInt32();
    auto errCode = data.ReadInt32();
    OnStopUserDone(accountId, errCode);
    return NO_ERROR;
}

int UserCallbackStub::OnStartUserDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto accountId = data.ReadInt32();
    auto errCode = data.ReadInt32();
    OnStartUserDone(accountId, errCode);
    return NO_ERROR;
}

int UserCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = UserCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    if (code < UserCallbackCmd::CMD_MAX && code >= 0) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS
