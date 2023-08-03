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

#ifndef OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_INTERFACE_H

#include <iremote_broker.h>
#include "ability_connect_callback_interface.h"

namespace OHOS {
namespace AppExecFwk {
enum class ExtensionAbilityType;
}
namespace AAFwk {
namespace {
constexpr int DATA_SHARE_DEFAULT_INVALID_USER_ID = -1;
}
class Want;
/**
 * @class IDataShareManager
 * IDataShareManager interface is used to access ability manager services.
 */
class IDataShareManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.DataShareManager")

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
    virtual int ConnectAbilityCommon(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DATA_SHARE_DEFAULT_INVALID_USER_ID) = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_INTERFACE_H
