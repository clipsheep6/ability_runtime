/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SESSION_HANDLER_STUB_H
#define OHOS_ABILITY_RUNTIME_SESSION_HANDLER_STUB_H

#include "isession_handler_interface.h"
#include <iremote_stub.h>

namespace OHOS {
namespace AAFwk {

class SessionHandlerStub : public IRemoteStub<ISessionHandler> {
public:
    SessionHandlerStub();
    virtual ~SessionHandlerStub() = default;
    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    virtual void OnSessionMovedToFront(int32_t sessionId) override;

private:
    DISALLOW_COPY_AND_MOVE(SessionHandlerStub);
    virtual int32_t OnSessionMovedToFrontInner(MessageParcel &data, MessageParcel &reply);
    using StubFunc = int (SessionHandlerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::vector<StubFunc> vecMemberFunc_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SESSION_HANDLER_STUB_H