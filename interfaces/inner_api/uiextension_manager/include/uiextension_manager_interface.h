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

#ifndef OHOS_ABILITY_RUNTIME_UIEXTENSION_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_UIEXTENSION_MANAGER_INTERFACE_H

#include <iremote_broker.h>

#include "ability_manager_interface.h"
#include "session_info.h"

namespace OHOS {
namespace AAFwk {
class IUIExtensionManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.UIExtensionManager")

    /**
     * Start ui extension ability with extension session info, send extension session info to ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to start.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode StartUIExtensionAbility(
        const sptr<SessionInfo> &extensionSessionInfo, int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * TerminateUIExtensionAbility with want, return want from ability manager service.
     *
     * @param extensionSessionInfo the extension session info of the ability to terminate.
     * @param resultCode resultCode.
     * @param Want Ability want returned.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        int resultCode = DEFAULT_INVAL_VALUE, const Want *resultWant = nullptr) = 0;

    /**
     * MinimizeUIExtensionAbility, minimize the special ui extension ability.
     *
     * @param extensionSessionInfo the extension session info of the ability to minimize.
     * @param fromUser mark the minimize operation source.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        bool fromUser = false) = 0;

    /**
     * Connect ui extension ability.
     *
     * @param want, special want for the ui extension ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param sessionInfo the extension session info of the ability to connect.
     * @param userId, the extension runs in.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<SessionInfo> &sessionInfo, int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    /**
     * Check ui extension is focused.
     * @param uiExtensionTokenId ui extension tokenId.
     * @param isFocused is focused.
     * @return Returns ERR_OK on success, others on failure.
     */
    virtual ErrCode CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool &isFocused) = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UIEXTENSION_MANAGER_INTERFACE_H