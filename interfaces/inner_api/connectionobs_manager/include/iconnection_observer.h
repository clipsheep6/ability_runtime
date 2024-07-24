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

#ifndef OHOS_ABILITYRUNTIME_ICONNECTION_OBSERVER_H
#define OHOS_ABILITYRUNTIME_ICONNECTION_OBSERVER_H

#include "connection_data.h"

#ifdef WITH_DLP
#include "dlp_state_data.h"
#endif // WITH_DLP

#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class IConnectionObserver
 * IConnectionObserver is used to notify connection relationship of extension component.
 */
class IConnectionObserver : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityruntime.connectionobserver");

    /**
     * called when extension was connected.
     *
     * @param data connection relationship data.
     */
    virtual void OnExtensionConnected(const ConnectionData &data) = 0;

    /**
     * called when extension was disconnected.
     *
     * @param data connection relationship data.
     */
    virtual void OnExtensionDisconnected(const ConnectionData &data) = 0;

#ifdef WITH_DLP
    /**
     * called when dlp ability was started.
     *
     * @param data dlp state data.
     */
    virtual void OnDlpAbilityOpened(const DlpStateData &data) = 0;

    /**
     * called when dlp ability was terminated.
     *
     * @param data dlp state data.
     */
    virtual void OnDlpAbilityClosed(const DlpStateData &data) = 0;
#endif // WITH_DLP

    enum ConnectionObserverCmd {
        // ipc id for OnExtensionConnected
        ON_EXTENSION_CONNECTED = 0,

        // ipc id for OnExtensionDisconnected
        ON_EXTENSION_DISCONNECTED,

#ifdef WITH_DLP
        // ipc id for OnDlpAbilityOpened
        ON_DLP_ABILITY_OPENED,

        // ipc id for OnExtensionDisconnected
        ON_DLP_ABILITY_CLOSED,
#endif // WITH_DLP

        // maximum of enum
        CMD_MAX
    };
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITYRUNTIME_ICONNECTION_OBSERVER_H
