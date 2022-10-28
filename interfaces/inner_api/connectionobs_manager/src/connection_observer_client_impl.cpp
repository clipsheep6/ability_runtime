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

#include "connection_observer_client_impl.h"

#include "connection_observer_errors.h"
#include "connection_observer_stub_impl.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t ConnectionObserverClientImpl::RegisterObserver(const std::shared_ptr<ConnectionObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("ConnectionObserverClientImpl::RegisterObserver invalid observer.");
        return ERR_INVALID_OBSERVER;
    }

    auto proxy = GetServiceProxy();

    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    if (!RegisterObserverToServiceLocked(proxy)) {
        HILOG_ERROR("register to service failed.");
        return ERR_REGISTER_FAILED;
    }

    return AddObserversLocked(observer);
}

int32_t ConnectionObserverClientImpl::UnregisterObserver(const std::shared_ptr<ConnectionObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("unregister, observer is invalid.");
        return ERR_INVALID_OBSERVER;
    }

    auto proxy = GetServiceProxy();

    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    auto ret = RemoveObserversLocked(observer);
    if (userObservers_.empty()) {
        UnregisterFromServiceLocked(proxy);
    }

    return ret;
}

int32_t ConnectionObserverClientImpl::GetDlpConnectionInfos(std::vector<DlpConnectionInfo> &infos)
{
    auto proxy = GetServiceProxy();
    if (!proxy) {
        HILOG_ERROR("unregister, observer is invalid.");
        return ERR_NO_PROXY;
    }

    return proxy->GetDlpConnectionInfos(infos);
}

void ConnectionObserverClientImpl::HandleExtensionConnected(const ConnectionData &data)
{
    auto observers = GetObservers();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        auto observer = *it;
        if (observer) {
            observer->OnExtensionConnected(data);
        }
    }
}

void ConnectionObserverClientImpl::HandleExtensionDisconnected(const ConnectionData &data)
{
    auto observers = GetObservers();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        auto observer = *it;
        if (observer) {
            observer->OnExtensionDisconnected(data);
        }
    }
}

void ConnectionObserverClientImpl::HandleDlpAbilityOpened(const DlpStateData &data)
{
    auto observers = GetObservers();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        auto observer = *it;
        if (observer) {
            observer->OnDlpAbilityOpened(data);
        }
    }
}

void ConnectionObserverClientImpl::HandleDlpAbilityClosed(const DlpStateData &data)
{
    auto observers = GetObservers();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        auto observer = *it;
        if (observer) {
            observer->OnDlpAbilityClosed(data);
        }
    }
}

bool ConnectionObserverClientImpl::RegisterObserverToServiceLocked(const std::shared_ptr<ServiceProxyAdapter> &proxy)
{
    if (isRegistered_) {
        return true;
    }

    if (!proxy) {
        HILOG_ERROR("fail to get service.");
        return false;
    }

    if (!observer_) {
        observer_ = sptr<IConnectionObserver>(new (std::nothrow) ConnectionObserverStubImpl(shared_from_this()));
        if (observer_ == nullptr) {
            HILOG_ERROR("observer is nullptr");
            return false;
        }
    }

    if (proxy->RegisterObserver(observer_) != ERR_OK) {
        HILOG_ERROR("register connection observer failed.");
        return false;
    }
    isRegistered_ = true;
    return true;
}

void ConnectionObserverClientImpl::UnregisterFromServiceLocked(const std::shared_ptr<ServiceProxyAdapter> &proxy)
{
    if (!isRegistered_ || !observer_) {
        return;
    }

    if (!proxy) {
        return;
    }

    if (proxy->UnregisterObserver(observer_) != ERR_OK) {
        HILOG_ERROR("unregister connection observer failed.");
        return;
    }
    isRegistered_ = false;
}

int32_t ConnectionObserverClientImpl::AddObserversLocked(const std::shared_ptr<ConnectionObserver> &observer)
{
    if (userObservers_.find(observer) != userObservers_.end()) {
        HILOG_ERROR("observer was already registered.");
        return ERR_OBSERVER_ALREADY_REGISTERED;
    }
    userObservers_.emplace(observer);
    return ERR_OK;
}

int32_t ConnectionObserverClientImpl::RemoveObserversLocked(const std::shared_ptr<ConnectionObserver> &observer)
{
    if (userObservers_.find(observer) == userObservers_.end()) {
        HILOG_ERROR("unregister no such observer.");
        return ERR_OBSERVER_NOT_REGISTERED;
    }
    userObservers_.erase(observer);
    return ERR_OK;
}

std::shared_ptr<ServiceProxyAdapter> ConnectionObserverClientImpl::GetServiceProxy()
{
    std::lock_guard<std::recursive_mutex> guard(proxyLock_);
    if (!serviceAdapter_) {
        ConnectLocked();
    }
    return serviceAdapter_;
}

void ConnectionObserverClientImpl::ConnectLocked()
{
    if (serviceAdapter_ != nullptr) {
        return;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Fail to get system ability registry.");
        return;
    }
    sptr<IRemoteObject> remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Fail to connect ability manager service.");
        return;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(
        new (std::nothrow) ServiceDeathRecipient(shared_from_this()));
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("Failed to create AbilityMgrDeathRecipient!");
        return;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("Add death recipient to AbilityManagerService failed.");
        return;
    }

    serviceAdapter_ = std::make_shared<ServiceProxyAdapter>(remoteObj);
    HILOG_INFO("Connect ability manager service success.");
}

void ConnectionObserverClientImpl::HandleRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (!ResetProxy(remote)) {
        return;
    }
    NotifyServiceDiedToObservers();
}

bool ConnectionObserverClientImpl::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::recursive_mutex> guard(proxyLock_);
    if (serviceAdapter_ == nullptr) {
        return false;
    }

    auto proxyObject = serviceAdapter_->GetProxyObject();
    if ((proxyObject != nullptr) && (proxyObject == remote.promote())) {
        proxyObject->RemoveDeathRecipient(deathRecipient_);
        serviceAdapter_ = nullptr;
        return true;
    }

    return false;
}

void ConnectionObserverClientImpl::ResetStatus()
{
    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    isRegistered_ = false;
    userObservers_.clear();
}

void ConnectionObserverClientImpl::NotifyServiceDiedToObservers()
{
    auto observers = GetObservers();
    ResetStatus();
    for (auto it = observers.begin(); it != observers.end(); ++it) {
        auto observer = *it;
        if (observer) {
            observer->OnServiceDied();
        }
    }
}

std::unordered_set<std::shared_ptr<ConnectionObserver>> ConnectionObserverClientImpl::GetObservers()
{
    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    return userObservers_;
}

void ConnectionObserverClientImpl::ServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("ServiceDeathRecipient handle remote abilityms died.");
    auto owner = owner_.lock();
    if (!owner) {
        HILOG_ERROR("ServiceDeathRecipient handle remote abilityms died.");
        return;
    }
    owner->HandleRemoteDied(remote);
}
} // namespace AbilityRuntime
} // namespace OHOS
