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


#ifndef OHOS_ABILITY_RUNTIME_ABILITY_BACKGROUND_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_ABILITY_BACKGROUND_CONNECTION_H

#include "ability_connect_callback_interface.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityBackgroundConnection
 * Ability Background Connection Stub.
 */
class AbilityBackgroundConnection : public IRemoteStub<IAbilityConnection> {
public:
    AbilityBackgroundConnection() = default;
    virtual ~AbilityBackgroundConnection() = default;

    /**
     * @brief OnAbilityConnectDone, AbilityMs notify caller ability the result of connect.
     * @param element service ability's ElementName.
     * @param remoteObject the session proxy of service ability.
     * @param resultCode ERR_OK on success, others on failure.
     */
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;

    /**
     * OnAbilityDisconnectDone, AbilityMs notify caller ability the result of disconnect.
     *
     * @param element, service ability's ElementName.
     * @param resultCode, ERR_OK on success, others on failure.
     */
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    DISALLOW_COPY_AND_MOVE(AbilityBackgroundConnection);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif // ABILITY_BACKGROUND_CONNECTION_H
