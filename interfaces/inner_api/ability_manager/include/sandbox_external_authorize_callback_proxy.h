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

#ifndef OHOS_ABILITY_RUNTIME_SANDBOX_EXTERNAL_AUTHORIZE_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_SANDBOX_EXTERNAL_AUTHORIZE_CALLBACK_PROXY_H

#include <string>
#include "iremote_proxy.h"
#include "i_sandbox_external_authorize_callback.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * interface for SandboxExternalAuthorizeCallbackProxy.
 */
class SandboxExternalAuthorizeCallbackProxy : public IRemoteProxy<ISandboxExternalAuthorizeCallback> {
public:
    explicit SandboxExternalAuthorizeCallbackProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<ISandboxExternalAuthorizeCallback>(impl) {}
    ~SandboxExternalAuthorizeCallbackProxy() = default;

    /**
     * @brief GetAuthorization.
     *
     * @param sandboxUid sandbox uid.
     * @param Want external request want.
     * @return AuthorType authorize type.
     */
    virtual AuthorType GetAuthorization(int sandboxUid, const Want& want) override;
private:
    static inline BrokerDelegator<SandboxExternalAuthorizeCallbackProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_SANDBOX_EXTERNAL_AUTHORIZE_CALLBACK_PROXY_H
