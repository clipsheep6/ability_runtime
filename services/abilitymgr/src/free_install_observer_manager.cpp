/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "free_install_observer_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
FreeInstallObserverManager::FreeInstallObserverManager()
{}

FreeInstallObserverManager::~FreeInstallObserverManager()
{}

int32_t FreeInstallObserverManager::AddObserver(int32_t recordId, const sptr<IFreeInstallObserver> &observer)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "begin");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "observer is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> lock(observerLock_);
    observerMap_[recordId] = observer;

    if (!deathRecipient_) {
        std::weak_ptr<FreeInstallObserverManager> thisWeakPtr(shared_from_this());
        // add death recipient
        deathRecipient_ =
            new FreeInstallObserverRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto freeInstallObserverManager = thisWeakPtr.lock();
                if (freeInstallObserverManager) {
                    freeInstallObserverManager->OnObserverDied(remote);
                }
            });
    }
    
    auto observerObj = observer->AsObject();
    if (!observerObj || !observerObj->AddDeathRecipient(deathRecipient_)) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "AddDeathRecipient failed");
    }

    return ERR_OK;
}

int32_t FreeInstallObserverManager::RemoveObserver(const sptr<IFreeInstallObserver> &observer)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "begin");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> lock(observerLock_);
    for (auto &item : observerMap_) {
        if (item.second && item.second->AsObject() == observer->AsObject()) {
            observerMap_.erase(item.first);
            return ERR_OK;
        }
    }
    TAG_LOGE(AAFwkTag::FREE_INSTALL, "Observer not exist or has been removed");
    return ERR_INVALID_VALUE;
}

void FreeInstallObserverManager::OnInstallFinished(int32_t recordId, const std::string &bundleName,
    const std::string &abilityName, const std::string &startTime, const int &resultCode)
{
    auto task = [weak = weak_from_this(), recordId, bundleName, abilityName, startTime, resultCode]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::FREE_INSTALL, "self is nullptr");
            return;
        }
        self->HandleOnInstallFinished(recordId, bundleName, abilityName, startTime, resultCode);
    };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(handler, "Fail to get Ability task handler.");
    handler->SubmitTask(task);
}

void FreeInstallObserverManager::OnInstallFinishedByUrl(int32_t recordId, const std::string &startTime,
    const std::string &url, const int &resultCode)
{
    auto task = [weak = weak_from_this(), recordId, startTime, url, resultCode]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::FREE_INSTALL, "self is nullptr");
            return;
        }
        self->HandleOnInstallFinishedByUrl(recordId, startTime, url, resultCode);
    };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(handler, "Fail to get Ability task handler.");
    handler->SubmitTask(task);
}

void FreeInstallObserverManager::HandleOnInstallFinished(int32_t recordId, const std::string &bundleName,
    const std::string &abilityName, const std::string &startTime, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "begin");
    auto iter = observerMap_.find(recordId);
    if (iter != observerMap_.end() && iter->second != nullptr) {
        (iter->second)->OnInstallFinished(bundleName, abilityName, startTime, resultCode);
    }
}

void FreeInstallObserverManager::HandleOnInstallFinishedByUrl(int32_t recordId, const std::string &startTime,
    const std::string &url, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "begin");
    auto iter = observerMap_.find(recordId);
    if (iter != observerMap_.end() && iter->second != nullptr) {
        (iter->second)->OnInstallFinishedByUrl(startTime, url, resultCode);
    }
}

void FreeInstallObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    auto remoteObj = remote.promote();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "observer is nullptr");
        return;
    }
    remoteObj->RemoveDeathRecipient(deathRecipient_);

    std::lock_guard<ffrt::mutex> lock(observerLock_);
    for (auto &item : observerMap_) {
        if (item.second && item.second->AsObject() == remoteObj) {
            observerMap_.erase(item.first);
            return;
        }
    }
}

FreeInstallObserverRecipient::FreeInstallObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

FreeInstallObserverRecipient::~FreeInstallObserverRecipient()
{}

void FreeInstallObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AAFwk
} // namespace OHOS