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

#ifndef OHOS_ABILITY_RUNTIME_SESSION_HANDLER_PROXY_H
#define OHOS_ABILITY_RUNTIME_SESSION_HANDLER_PROXY_H

#include "iremote_proxy.h"
#include "isession_handler_interface.h"

namespace OHOS {
namespace AAFwk {

class SessionHandlerProxy : public IRemoteProxy<ISessionHandler> {
public:
    explicit SessionHandlerProxy
        (const sptr<IRemoteObject> &impl) : IRemoteProxy<ISessionHandler>(impl) {}

    virtual ~SessionHandlerProxy() = default;

    virtual void OnSessionMovedToFront(int32_t sessionId) override;

private:
    static inline BrokerDelegator<SessionHandlerProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SESSION_HANDLER_PROXY_H