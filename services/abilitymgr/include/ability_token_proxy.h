/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_ABILITY_TOKEN_PROXY_H
#define OHOS_AAFWK_ABILITY_TOKEN_PROXY_H

#include "ability_token_interface.h"

#include "iremote_proxy.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityTokenProxy
 * AbilityToken proxy.
 */
class AbilityTokenProxy : public IRemoteProxy<IAbilityToken> {
public:
    explicit AbilityTokenProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAbilityToken>(impl)
    {}
    virtual ~AbilityTokenProxy()
    {}

private:
    static inline BrokerDelegator<AbilityTokenProxy> delegator_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_ABILITY_TOKEN_PROXY_H
