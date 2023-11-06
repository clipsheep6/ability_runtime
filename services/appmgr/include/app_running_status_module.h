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

#ifndef OHOS_ABILITY_RUNTIME_APP_RUNNING_STAUS_MOUDLE_H
#define OHOS_ABILITY_RUNTIME_APP_RUNNING_STAUS_MOUDLE_H

#include <map>
#include <shared_mutex>

#include "app_running_status_listener_interface.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
class AppRunningStatusModule : public std::enable_shared_from_this<AppRunningStatusModule> {
public:
    AppRunningStatusModule() = default;
    virtual ~AppRunningStatusModule() = default;

    /**
     * Register listener.
     *
     * @param listener App running status listener object.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t RegisterListener(const sptr<AppRunningStatusListenerInterface> &listener);

    /**
     * Unregister listener.
     *
     * @param listener App running status listener object.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t UnregisterListener(const sptr<AppRunningStatusListenerInterface> &listener);

    /**
     * Notify the app running status event.
     *
     * @param bundle Bundle name in application record.
     * @param uid Uid of bundle.
     * @param runningStatus Running status.
     * @return
     */
    void NotifyAppRunningStatusEvent(const std::string &bundle, int32_t uid, RunningStatus runningStatus);

    /**
     * @class ClientDeathRecipient.
     * Notices IRemoteBroker died.
     */
    class ClientDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ClientDeathRecipient(const std::weak_ptr<AppRunningStatusModule> &weakPtr);
        virtual ~ClientDeathRecipient() = default;

        /**
         * Handle remote object died event.
         *
         * @param Remote Remote object.
         */
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::weak_ptr<AppRunningStatusModule> weakPtr_;
    };

private:
    void SetDeathRecipient(const sptr<AppRunningStatusListenerInterface> &listener,
        const sptr<IRemoteObject::DeathRecipient> &deathRecipient);
    int32_t RemoveListenerAndDeathRecipient(const wptr<IRemoteObject> &remote);

    mutable std::mutex listenerMutex_;
    std::map<sptr<AppRunningStatusListenerInterface>, sptr<IRemoteObject::DeathRecipient>> listeners_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_RUNNING_STAUS_MOUDLE_H
