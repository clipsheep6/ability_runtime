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
#include "extension_ability_info.h"

namespace OHOS {
namespace AppExecFwk {
}
namespace AAFwk {
namespace {
constexpr int DEFAULT_INVALID_USER_ID = -1;
const int DEFAULT_INVAL_VALUE = -1;
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
    virtual int ConnectAbilityCommon(const Want &want, const sptr<IRemoteObject> &connect,
        const sptr<IRemoteObject> &callerToken, AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVALID_USER_ID)
    {
        return 0;
    }

    /**
     * DisconnectAbility, disconnect session with service ability.
     *
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int DisconnectAbility(const sptr<IRemoteObject> &connect) = 0;

    /**
     * Start extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to start.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be started.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED)
    {
        return 0;
    }

    /**
     * Stop extension ability with want, send want to ability manager service.
     *
     * @param want, the want of the ability to stop.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @param extensionType If an ExtensionAbilityType is set, only extension of that type can be stopped.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int StopExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED)
    {
        return 0;
    }

    /**
     * Request dialog service with want, send want to ability manager service.
     *
     * @param want, the want of the dialog service to start.
     * @param callerToken, caller ability token.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual int32_t RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_EXTENSION_MANAGER_INTERFACE_H
