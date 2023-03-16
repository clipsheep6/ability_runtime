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

#include "connection_observer_controller.h"

#include "connection_observer_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AbilityRuntime;
int ConnectionObserverController::AddObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("observer is invalid");
        return AbilityRuntime::ERR_INVALID_OBSERVER;
    }

    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    auto it = std::find_if(observers_.begin(), observers_.end(), [&observer](const sptr<IConnectionObserver> &item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observers_.end()) {
        HILOG_WARN("observer was already added, do not add again");
        return 0;
    }

    if (!observerDeathRecipient_) {
        std::weak_ptr<ConnectionObserverController> thisWeakPtr(shared_from_this());
        observerDeathRecipient_ =
            new ObserverDeathRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto controller = thisWeakPtr.lock();
                if (controller) {
                    controller->HandleRemoteDied(remote);
                }
            });
    }
    auto observerObj = observer->AsObject();
    if (!observerObj || !observerObj->AddDeathRecipient(observerDeathRecipient_)) {
        HILOG_ERROR("AddDeathRecipient failed.");
    }
    observers_.emplace_back(observer);

    return 0;
}

void ConnectionObserverController::RemoveObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("observer is invalid");
        return;
    }

    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    auto it = std::find_if(observers_.begin(), observers_.end(), [&observer](const sptr<IConnectionObserver> item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void ConnectionObserverController::NotifyExtensionConnected(const AbilityRuntime::ConnectionData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnExtensionConnected, data);
}

void ConnectionObserverController::NotifyExtensionDisconnected(const AbilityRuntime::ConnectionData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnExtensionDisconnected, data);
}

void ConnectionObserverController::NotifyDlpAbilityOpened(const AbilityRuntime::DlpStateData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnDlpAbilityOpened, data);
}

void ConnectionObserverController::NotifyDlpAbilityClosed(const AbilityRuntime::DlpStateData& data)
{
    CallObservers(&AbilityRuntime::IConnectionObserver::OnDlpAbilityClosed, data);
}

std::vector<sptr<AbilityRuntime::IConnectionObserver>> ConnectionObserverController::GetObservers()
{
    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    return observers_;
}

void ConnectionObserverController::HandleRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("remote connection oberver was died.");
    auto remoteObj = remote.promote();
    if (!remoteObj) {
        HILOG_DEBUG("invalid remote object.");
        return;
    }
    remoteObj->RemoveDeathRecipient(observerDeathRecipient_);

    std::lock_guard<std::recursive_mutex> guard(observerLock_);
    auto it = std::find_if(observers_.begin(), observers_.end(), [&remoteObj](const sptr<IConnectionObserver> item) {
        return (item && item->AsObject() == remoteObj);
    });
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

ConnectionObserverController::ObserverDeathRecipient::ObserverDeathRecipient(ObserverDeathHandler handler)
    : deathHandler_(handler)
{}

void ConnectionObserverController::ObserverDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (deathHandler_) {
        deathHandler_(remote);
    }
}
} // namespace AAFwk
} // namespace OHOS
