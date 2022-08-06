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

#ifndef ABILITY_RUNTIME_CONNECTION_OBSERVER_H
#define ABILITY_RUNTIME_CONNECTION_OBSERVER_H

#include "connection_data.h"
#include "dlp_state_data.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class ConnectionObserver
 * ConnectionObserver is used to notify connection relationship of extension component.
 */
class ConnectionObserver {
public:
    /**
     * @brief Constructor.
     *
     */
    ConnectionObserver() = default;

    /**
     * @brief Destructor.
     *
     */
    virtual ~ConnectionObserver() = default;

    /**
     * called when extension was connected.
     *
     * @param data connection relationship data.
     */
    virtual void OnExtensionConnected(const ConnectionData& data) = 0;

    /**
     * called when extension was disconnected.
     *
     * @param data connection relationship data.
     */
    virtual void OnExtensionDisconnected(const ConnectionData& data) = 0;

    /**
     * called when dlp ability was started.
     *
     * @param data dlp state data.
     */
    virtual void OnDlpAbilityOpened(const DlpStateData& data) = 0;

    /**
     * called when dlp ability was terminated.
     *
     * @param data dlp state data.
     */
    virtual void OnDlpAbilityClosed(const DlpStateData& data) = 0;

    /**
     * called when service was died.
     *
     */
    virtual void OnServiceDied() {}
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_CONNECTION_OBSERVER_H
