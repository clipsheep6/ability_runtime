/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "session_handler_stub.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
SessionHandlerStub::SessionHandlerStub()
{
    vecMemberFunc_.resize(ISessionHandler::CODE_MAX);
    vecMemberFunc_[ON_SESSION_MOVED_TO_FRONT] = &SessionHandlerStub::OnSessionMovedToFrontInner;
}

int32_t SessionHandlerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = SessionHandlerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::DEFAULT, "local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code < ISessionHandler::CODE_MAX) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SessionHandlerStub::OnSessionMovedToFrontInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t sessionId = data.ReadInt32();
    OnSessionMovedToFront(sessionId);
    return NO_ERROR;
}

void SessionHandlerStub::OnSessionMovedToFront(int32_t sessionId)
{
    TAG_LOGI(AAFwkTag::DEFAULT, "call, sessionId:%{public}d", sessionId);
}
}
}