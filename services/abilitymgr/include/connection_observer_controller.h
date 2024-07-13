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

#ifndef OHOS_AAFWK_CONNECTION_OBSERVER_CONTROLLER_H
#define OHOS_AAFWK_CONNECTION_OBSERVER_CONTROLLER_H

#include <mutex>
#include <vector>
#include "cpp/mutex.h"

#include "iconnection_observer.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ConnectionObserverController
 * ConnectionObserverController manage connection observers.
 */
class ConnectionObserverController : public std::enable_shared_from_this<ConnectionObserverController> {
public:
    ConnectionObserverController() = default;
    ~ConnectionObserverController() = default;

    /**
     * add connection observer.
     *
     * @param observer the observer callback.
     * @return Returns ERR_OK on success, others on failure.
     */
    int AddObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    /**
     * delete a callback.
     *
     * @param observer the observer callback.
     */
    void RemoveObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer);

    /**
     * notify observers that extension was connected.
     *
     * @param data connection data.
     */
    void NotifyExtensionConnected(const AbilityRuntime::ConnectionData& data);

    /**
     * notify observers that extension was disconnected.
     *
     * @param data connection data.
     */
    void NotifyExtensionDisconnected(const AbilityRuntime::ConnectionData& data);

#ifdef WITH_DLP
    /**
     * notify observers that dlp ability was opened.
     *
     * @param data dlp state data.
     */
    void NotifyDlpAbilityOpened(const AbilityRuntime::DlpStateData& data);

    /**
     * notify observers that dlp ability was closed.
     *
     * @param data dlp state data.
     */
    void NotifyDlpAbilityClosed(const AbilityRuntime::DlpStateData& data);
#endif // WITH_DLP

private:
    std::vector<sptr<AbilityRuntime::IConnectionObserver>> GetObservers();
    void HandleRemoteDied(const wptr<IRemoteObject> &remote);

    template<typename F, typename... Args>
    void CallObservers(F func, Args&&... args)
    {
        auto observers = GetObservers();
        for (auto& observer : observers) {
            if (observer) {
                (observer->*func)(std::forward<Args>(args)...);
            }
        }
    }

    class ObserverDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        using ObserverDeathHandler = std::function<void(const wptr<IRemoteObject> &)>;
        explicit ObserverDeathRecipient(ObserverDeathHandler handler);
        ~ObserverDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) final;

    private:
        ObserverDeathHandler deathHandler_;
    };

private:
    ffrt::mutex observerLock_;
    std::vector<sptr<AbilityRuntime::IConnectionObserver>> observers_;
    sptr<IRemoteObject::DeathRecipient> observerDeathRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_CONNECTION_OBSERVER_CONTROLLER_H
