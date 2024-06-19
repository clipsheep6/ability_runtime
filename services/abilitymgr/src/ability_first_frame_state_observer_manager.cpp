/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifdef SUPPORT_GRAPHICS
#include "ability_first_frame_state_observer_manager.h"

#include "ability_first_frame_state_data.h"
#include "ability_first_frame_state_observer_stub.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "hilog_tag_wrapper.h"
#include "permission_verification.h"

namespace OHOS {
namespace AppExecFwk {
AbilityFirstFrameStateObserverSet::AbilityFirstFrameStateObserverSet(
    std::weak_ptr<AbilityFirstFrameStateObserverManager> managerPtr, bool isNotifyAllBundles)
    : abilityFirstFrameStateObserverManager_(managerPtr), isNotifyAllBundles_(isNotifyAllBundles) {}

int32_t AbilityFirstFrameStateObserverSet::AddAbilityFirstFrameStateObserver(
    const sptr<IAbilityFirstFrameStateObserver> &observer, const std::string &targetBundleName)
{
    if (observer == nullptr || observer->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param observer or observer->AsObject is nullptr.");
        return ERR_INVALID_VALUE;
    }
    {
        std::lock_guard lockRegister(observerLock_);
        for (auto it : observerMap_) {
            if (it.first->AsObject() == observer->AsObject()) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Observer exist.");
                return ERR_OK;
            }
        }
        observerMap_.emplace(observer, targetBundleName);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "observerMap_ size:%{public}zu", observerMap_.size());
        AddObserverDeathRecipient(observer);
    }
    return ERR_OK;
}

void AbilityFirstFrameStateObserverSet::AddObserverDeathRecipient(const sptr<IRemoteBroker> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (observer == nullptr || observer->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param observer or observer->AsObject is nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "This death recipient has been added.");
        return;
    }
    auto deathRecipientFunc = [weak = abilityFirstFrameStateObserverManager_](const wptr<IRemoteObject> &remote) {
        auto manager = weak.lock();
        auto object = remote.promote();
        if (manager == nullptr || object == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "manager or object is nullptr.");
            return;
        }
        sptr<IAbilityFirstFrameStateObserver> observer = iface_cast<IAbilityFirstFrameStateObserver>(object);
        manager->UnregisterAbilityFirstFrameStateObserver(observer);
    };
    sptr<IRemoteObject::DeathRecipient> deathRecipient =
        new (std::nothrow) AbilityFirstFrameStateObserverRecipient(deathRecipientFunc);
    if (deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "deathRecipient is nullptr.");
        return;
    }
    if (!observer->AsObject()->AddDeathRecipient(deathRecipient)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient failed.");
    }
    recipientMap_.emplace(observer->AsObject(), deathRecipient);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observerMap_ size:%{public}zu", recipientMap_.size());
}

int32_t AbilityFirstFrameStateObserverSet::RemoveAbilityFirstFrameStateObserver(
    const sptr<IAbilityFirstFrameStateObserver> &observer)
{
    std::lock_guard lockRegister(observerLock_);
    for (auto it = observerMap_.begin(); it != observerMap_.end(); ++it) {
        if (it->first->AsObject() == observer->AsObject()) {
            observerMap_.erase(it);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "observerMap_ size:%{public}zu", observerMap_.size());
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Remove observer failed.");
    return ERR_INVALID_VALUE;
}

void AbilityFirstFrameStateObserverSet::RemoveObserverDeathRecipient(const sptr<IRemoteBroker> &observer)
{
    if (observer == nullptr || observer->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param observer is nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        recipientMap_.erase(it);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "recipientMap_ size:%{public}zu", recipientMap_.size());
}

void AbilityFirstFrameStateObserverSet::OnAbilityFirstFrameState(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The param abilityRecord is nullptr.");
        return;
    }
    const AppExecFwk::AbilityInfo abilityInfo = abilityRecord->GetAbilityInfo();
    AbilityFirstFrameStateData abilityFirstFrameStateData;
    abilityFirstFrameStateData.bundleName = abilityInfo.bundleName;
    abilityFirstFrameStateData.moduleName = abilityInfo.moduleName;
    abilityFirstFrameStateData.abilityName = abilityInfo.name;
    abilityFirstFrameStateData.appIndex = abilityRecord->GetAppIndex();
    abilityFirstFrameStateData.coldStart = abilityRecord->GetColdStartFlag();
    std::lock_guard lockRegister(observerLock_);
    for (auto it : observerMap_) {
        if (!isNotifyAllBundles_) {
            if (it.second == abilityInfo.bundleName) {
                it.first->OnAbilityFirstFrameState(abilityFirstFrameStateData);
            }
        } else {
            it.first->OnAbilityFirstFrameState(abilityFirstFrameStateData);
        }
    }
}

AbilityFirstFrameStateObserverManager::AbilityFirstFrameStateObserverManager()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityFirstFrameStateObserverManager instance is created");
}

AbilityFirstFrameStateObserverManager::~AbilityFirstFrameStateObserverManager()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityFirstFrameStateObserverManager instance is destroyed");
}

void AbilityFirstFrameStateObserverManager::Init()
{
    if (!stateObserverSetForBundleName_) {
        stateObserverSetForBundleName_ = std::make_unique<AbilityFirstFrameStateObserverSet>(shared_from_this());
    }
    if (!stateObserverSetForAllBundles_) {
        stateObserverSetForAllBundles_ = std::make_unique<AbilityFirstFrameStateObserverSet>(shared_from_this(), true);
    }
}

int32_t AbilityFirstFrameStateObserverManager::RegisterAbilityFirstFrameStateObserver(
    const sptr<IAbilityFirstFrameStateObserver> &observer, const std::string &targetBundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (!PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    // verify permissions.
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    if (targetBundleName.empty()) {
        return stateObserverSetForAllBundles_->AddAbilityFirstFrameStateObserver(observer, "");
    } else {
        return stateObserverSetForBundleName_->AddAbilityFirstFrameStateObserver(observer, targetBundleName);
    }
}

int32_t AbilityFirstFrameStateObserverManager::UnregisterAbilityFirstFrameStateObserver(
    const sptr<IAbilityFirstFrameStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    if (!PermissionVerification::GetInstance()->IsSystemAppCall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    // verify permissions.
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    if (stateObserverSetForBundleName_->RemoveAbilityFirstFrameStateObserver(observer) == ERR_OK) {
        return ERR_OK;
    }
    stateObserverSetForAllBundles_->RemoveAbilityFirstFrameStateObserver(observer);
    return ERR_OK;
}

void AbilityFirstFrameStateObserverManager::HandleOnFirstFrameState(
    const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    stateObserverSetForBundleName_->OnAbilityFirstFrameState(abilityRecord);
    stateObserverSetForAllBundles_->OnAbilityFirstFrameState(abilityRecord);
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif // SUPPORT_GRAPHICS