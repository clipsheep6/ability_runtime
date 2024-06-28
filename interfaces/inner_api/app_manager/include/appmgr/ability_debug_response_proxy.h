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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_RESPONSE_PROXY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_RESPONSE_PROXY_H

#include "ability_debug_response_interface.h"
#include "iremote_object.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDebugResponseProxy : public IRemoteProxy<IAbilityDebugResponse> {
public:
    explicit AbilityDebugResponseProxy(const sptr<IRemoteObject> &impl);
    virtual ~AbilityDebugResponseProxy() = default;
    
    /**
     * @brief Set ability attach debug flag through proxy project.
     * @param tokens The token of ability token.
     */
    void OnAbilitysDebugStarted(const std::vector<sptr<IRemoteObject>> &tokens) override;

    /**
     * @brief Cancel ability attach debug flag through proxy project.
     * @param tokens The token of ability token.
     */
    void OnAbilitysDebugStoped(const std::vector<sptr<IRemoteObject>> &tokens) override;

    /**
     * @brief Change ability assert debug flag.
     * @param tokens The token of ability records.
     * @param isAssertDebug Assert debug flag.
     */
    void OnAbilitysAssertDebugChange(const std::vector<sptr<IRemoteObject>> &tokens, bool isAssertDebug) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    void SendRequest(const IAbilityDebugResponse::Message &message, const std::vector<sptr<IRemoteObject>> &tokens);
    static inline BrokerDelegator<AbilityDebugResponseProxy> delegator_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_DEBUG_RESPONSE_PROXY_H
