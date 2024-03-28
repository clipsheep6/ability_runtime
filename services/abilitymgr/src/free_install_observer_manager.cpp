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

#include <chrono>

#include "ability_event_handler.h"
#include "ability_manager_service.h"
#include "ability_manager_errors.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
FreeInstallObserverManager::FreeInstallObserverManager()
{}

FreeInstallObserverManager::~FreeInstallObserverManager()
{}

int32_t FreeInstallObserverManager::AddObserver(const sptr<IFreeInstallObserver> &observer)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "AddObserver begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "the observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> lock(observerLock_);
    if (ObserverExistLocked(observer)) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "Observer exist.");
        return ERR_INVALID_VALUE;
    }
    observerList_.emplace_back(observer);
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "observerList_ size:%{public}zu", observerList_.size());

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
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "AddDeathRecipient failed.");
    }

    return ERR_OK;
}

int32_t FreeInstallObserverManager::RemoveObserver(const sptr<IFreeInstallObserver> &observer)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "RemoveObserver begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "the observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> lock(observerLock_);
    auto it = std::find_if(observerList_.begin(), observerList_.end(),
        [&observer](const sptr<IFreeInstallObserver> &item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observerList_.end()) {
        observerList_.erase(it);
        TAG_LOGI(AAFwkTag::FREE_INSTALL, "observerList_ size:%{public}zu", observerList_.size());
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::FREE_INSTALL, "Observer not exist or has been removed.");
    return ERR_INVALID_VALUE;
}

void FreeInstallObserverManager::OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, const int &resultCode)
{
    auto task = [weak = weak_from_this(), bundleName, abilityName, startTime, resultCode]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::FREE_INSTALL, "self is nullptr, OnInstallFinished failed.");
            return;
        }
        TAG_LOGI(AAFwkTag::FREE_INSTALL, "OnInstallFinished come.");
        self->HandleOnInstallFinished(bundleName, abilityName, startTime, resultCode);
    };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(handler, "Fail to get Ability task handler.");
    handler->SubmitTask(task);
}

void FreeInstallObserverManager::HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "HandleOnInstallFinished begin.");
    for (auto it = observerList_.begin(); it != observerList_.end(); ++it) {
        if ((*it) == nullptr) {
            continue;
        }
        (*it)->OnInstallFinished(bundleName, abilityName, startTime, resultCode);
    }
}

bool FreeInstallObserverManager::ObserverExistLocked(const sptr<IFreeInstallObserver> &observer)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "ObserExist begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "The param observer is nullptr.");
        return false;
    }
    auto it = std::find_if(observerList_.begin(), observerList_.end(),
        [&observer](const sptr<IFreeInstallObserver> &item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    return it != observerList_.end();
}

void FreeInstallObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::FREE_INSTALL, "OnObserverDied begin.");
    auto remoteObj = remote.promote();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "observer is nullptr.");
        return;
    }
    remoteObj->RemoveDeathRecipient(deathRecipient_);

    std::lock_guard<ffrt::mutex> lock(observerLock_);
    auto it = std::find_if(observerList_.begin(), observerList_.end(), [&remoteObj]
        (const sptr<IFreeInstallObserver> item) {
        return (item && item->AsObject() == remoteObj);
    });
    if (it != observerList_.end()) {
        observerList_.erase(it);
    }
}

FreeInstallObserverRecipient::FreeInstallObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

FreeInstallObserverRecipient::~FreeInstallObserverRecipient()
{}

void FreeInstallObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGE(AAFwkTag::FREE_INSTALL, "FreeInstallObserverRecipient On remote died.");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AAFwk
} // namespace OHOS