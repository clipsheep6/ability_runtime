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

#ifndef OHOS_ABILITY_RUNTIME_COMPONENT_INTERCEPTION_H
#define OHOS_ABILITY_RUNTIME_COMPONENT_INTERCEPTION_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::Want;

/**
 * @brief Interface to monitor what is happening in component manager.
 */
class IComponentInterception : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.IComponentInterception");

    /**
     * The system is trying to start an component.
     *
     * @param want The want of component to start.
     * @param callerToken Caller component token.
     * @param requestCode the requestCode of the component to start.
     * @param componentStatus the status of component.
     * @param extraParam The extra param of component to start.
     * @return Return true to allow component to start, or false to reject.
     */
    virtual bool AllowComponentStart(const Want &want, const sptr<IRemoteObject> &callerToken,
        int requestCode, int componentStatus, sptr<Want> &extraParam) = 0;

    /**
     * The system is trying to move ability to foreground/background.
     *
     * @param abilityToken Ability token.
     * @param opCode the operation code of the ability.
     */
    virtual void NotifyHandleAbilityStateChange(const sptr<IRemoteObject> &abilityToken, int opCode) = 0;

    enum class Message {
        TRANSACT_ON_ALLOW_COMPONENT_START = 1,
        TRANSACT_ON_HANDLE_MOVE_ABILITY,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_COMPONENT_INTERCEPTION_H
