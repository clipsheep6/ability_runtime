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

#ifndef OHOS_ABILITY_RUNTIME_EXTENSION_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_EXTENSION_MANAGER_INTERFACE_H

#include <iremote_broker.h>

namespace OHOS {
namespace AppExecFwk {
enum class ExtensionAbilityType;
}
namespace AAFwk {
namespace {
constexpr int DEFAULT_INVALID_USER_ID = -1;
}
class Want;
/**
 * @class IExtensionManager
 * IExtensionManager interface is used to access ability manager services.
 */
class IExtensionManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.ExtensionManager")

    /**
     * Connect ability common method.
     *
     * @param want, special want for service type's ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param extensionType, type of the extension.
     * @param userId, the service user ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int ConnectAbilityCommon(const Want &want, sptr<IRemoteObject> connect,
        sptr<IRemoteObject> callerToken, AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVALID_USER_ID, bool isQuerryExtensionOnly = true)
    {
        return 0;
    }

    /**
     * Disconnect session with extension ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(sptr<IRemoteObject> connect) = 0;

    /**
     * Start extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType ExtensionAbilityType of which you want to start.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartExtensionAbility(const Want &want, sptr<IRemoteObject> callerToken,
        int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
    {
        return 0;
    }

    /**
     * Stop extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to stop.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType ExtensionAbilityType of which you want to stop.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopExtensionAbility(const Want& want, sptr<IRemoteObject> callerToken,
        int32_t userId, AppExecFwk::ExtensionAbilityType extensionType)
    {
        return 0;
    }

    /**
     * Destroys this Service ability by Want.
     *
     * @param want, Special want for service type's ability.
     * @param token ability's token.
     * @return Returns true if this Service ability will be destroyed; returns false otherwise.
     */
    virtual int StopServiceAbility(const Want &want, int32_t userId, sptr<IRemoteObject> token) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EXTENSION_MANAGER_INTERFACE_H
