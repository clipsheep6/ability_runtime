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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INFO_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INFO_CALLBACK_PROXY_H

#include "iability_info_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Transfer abilityInfo to the initiator.
 */
class AbilityInfoCallbackProxy : public IRemoteProxy<IAbilityInfoCallback> {
public:
    explicit AbilityInfoCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~AbilityInfoCallbackProxy() = default;

    /**
     * Notify the initiator of the ability token.
     *
     * @param token The token of ability.
     * @param want The want of ability to start.
     */
    virtual void NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want) override;

    /**
     * Notify to start specified ability.
     *
     * @param callerToken The token of caller.
     * @param want The want of ability to start.
     * @param requestCode The request code of start ability.
     * @param extraParam The extra param of ability to start.
     */
    virtual void NotifyStartSpecifiedAbility(const sptr<IRemoteObject> &callerToken, const Want &want, int requestCode,
        sptr<Want> &extraParam) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    void SetExtraParam(const sptr<Want> &want, sptr<Want> &extraParam);
    static inline BrokerDelegator<AbilityInfoCallbackProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
