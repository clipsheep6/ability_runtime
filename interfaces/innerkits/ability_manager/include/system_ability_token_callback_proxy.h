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

#ifndef OHOS_AAFWK_SYSTEM_ABILITY_TOKEN_CALLBACK_PROXY_H
#define OHOS_AAFWK_SYSTEM_ABILITY_TOKEN_CALLBACK_PROXY_H

#include "system_ability_token_callback.h"
#include "iremote_proxy.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class SystemAbilityTokenCallbackProxy : public IRemoteProxy<ISystemAbilityTokenCallback> {
public:
    explicit SystemAbilityTokenCallbackProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<ISystemAbilityTokenCallback>(impl)
    {}
    virtual ~SystemAbilityTokenCallbackProxy() {}

    /**
     * @brief When the remote device send result back to system ability, AbilityMs notify the listener.
     *
     * @param want, want of caller.
     * @param callerUid, uid of caller.
     * @param deviceId, requestCode of caller.
     * @param deviceId, accessToken of caller .
     * @param deviceId, resultCode of caller.
     */
    virtual int32_t SendResult(OHOS::AAFwk::Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, int32_t resultCode) override;

private:
    static inline BrokerDelegator<SystemAbilityTokenCallbackProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_SYSTEM_ABILITY_TOKEN_CALLBACK_PROXY_H
