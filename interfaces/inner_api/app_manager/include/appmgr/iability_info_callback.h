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

#ifndef OHOS_ABILITY_RUNTIME_IABILITY_INFO_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_IABILITY_INFO_CALLBACK_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::Want;
/**
 * @brief Transfer abilityInfo to the initiator.
 */
class IAbilityInfoCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IAbilityInfoCallback");

    /**
     * Notify the initiator of the ability token.
     *
     * @param token The token of ability.
     * @param want The want of ability to start.
     */
    virtual void NotifyAbilityToken(const sptr<IRemoteObject> token, const Want &want) = 0;

    /**
     * Notify to start specified ability.
     *
     * @param callerToken The token of caller.
     * @param want The want of ability to start.
     * @param requestCode The request code of start ability.
     * @param extraParam The extra param of ability to start.
     */
    virtual void NotifyStartSpecifiedAbility(const sptr<IRemoteObject> &callerToken, const Want &want, int requestCode,
        sptr<Want> &extraParam) = 0;

    enum {
        Notify_ABILITY_TOKEN = 1,
        Notify_START_SPECIFIED_ABILITY,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IABILITY_INFO_CALLBACK_H
