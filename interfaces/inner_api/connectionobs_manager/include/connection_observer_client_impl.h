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

#ifndef ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_IMPL_H
#define ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_IMPL_H

#include <mutex>
#include <unordered_set>

#include "connection_observer.h"
#ifdef WITH_DLP
#include "dlp_connection_info.h"
#endif // WITH_DLP
#include "service_proxy_adapter.h"

namespace OHOS {
namespace AbilityRuntime {
/**
 * @class ConnectionObserverClientImpl
 * ConnectionObserverClientImpl is used to manage connection observer.
 */
class ConnectionObserverClientImpl : public std::enable_shared_from_this<ConnectionObserverClientImpl> {
public:
    ConnectionObserverClientImpl() = default;
    virtual ~ConnectionObserverClientImpl() = default;

    int32_t RegisterObserver(const std::shared_ptr<ConnectionObserver> &observer);
    int32_t UnregisterObserver(const std::shared_ptr<ConnectionObserver> &observer);
    int32_t GetConnectionData(std::vector<ConnectionData> &infos);
    void HandleExtensionConnected(const ConnectionData &data);
    void HandleExtensionDisconnected(const ConnectionData &data);
    void HandleRemoteDied(const wptr<IRemoteObject> &remote);
    
#ifdef WITH_DLP
    int32_t GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos);
    void HandleDlpAbilityOpened(const DlpStateData &data);
    void HandleDlpAbilityClosed(const DlpStateData &data);
#endif // WITH_DLP

private:
    class ServiceDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ServiceDeathRecipient(const std::shared_ptr<ConnectionObserverClientImpl>& owner) : owner_(owner) {}

        virtual ~ServiceDeathRecipient() = default;

        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;

    private:
        std::weak_ptr<ConnectionObserverClientImpl> owner_;
    };

    std::unordered_set<std::shared_ptr<ConnectionObserver>> GetObservers();
    std::shared_ptr<ServiceProxyAdapter> GetServiceProxy();
    void ConnectLocked();
    bool RegisterObserverToServiceLocked(const std::shared_ptr<ServiceProxyAdapter> &proxy);
    void UnregisterFromServiceLocked(const std::shared_ptr<ServiceProxyAdapter> &proxy);
    int32_t AddObserversLocked(const std::shared_ptr<ConnectionObserver> &observer);
    int32_t RemoveObserversLocked(const std::shared_ptr<ConnectionObserver> &observer);
    bool ResetProxy(const wptr<IRemoteObject> &remote);
    void ResetStatus();
    void NotifyServiceDiedToObservers();

    std::mutex observerLock_; // observer lock
    bool isRegistered_ = false; // mark whether register observer to abilityms.
    sptr<IConnectionObserver> observer_; // observer stub
    std::unordered_set<std::shared_ptr<ConnectionObserver>> userObservers_; // all registered observers.

    std::mutex proxyLock_; // proxy lock.
    std::shared_ptr<ServiceProxyAdapter> serviceAdapter_; // abilityms proxy adapter, send request code.
    sptr<IRemoteObject::DeathRecipient> deathRecipient_; // abilityms death recipient.
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // ABILITY_RUNTIME_CONNECTION_OBSERVER_CLIENT_IMPL_H
