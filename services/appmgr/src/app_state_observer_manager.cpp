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

#include "app_state_observer_manager.h"
#include "application_state_observer_stub.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
const std::string THREAD_NAME = "AppStateObserverManager";
const int BUNDLENAMELIST_MAX_SIZE = 128;
AppStateObserverManager::AppStateObserverManager()
{
    HILOG_INFO("AppStateObserverManager instance is created");
}

AppStateObserverManager::~AppStateObserverManager()
{
    HILOG_INFO("AppStateObserverManager instance is destroyed");
}

void AppStateObserverManager::Init()
{
    if (!handler_) {
        handler_ = std::make_shared<EventHandler>(EventRunner::Create(THREAD_NAME));
    }
}

int32_t AppStateObserverManager::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    HILOG_INFO("Register applicationStateObserver begin.");
    if (bundleNameList.size() > BUNDLENAMELIST_MAX_SIZE) {
        HILOG_ERROR("the bundleNameList passed in is too long");
        return ERR_INVALID_VALUE;
    }
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::recursive_mutex> lockRegister(observerLock_);
    if (ObserverExist(observer)) {
        HILOG_ERROR("Observer exist.");
        return ERR_INVALID_VALUE;
    }
    appStateObserverMap_.emplace(observer, bundleNameList);
    HILOG_DEBUG("%{public}s appStateObserverMap_ size:%{public}d", __func__, (int32_t)appStateObserverMap_.size());
    AddObserverDeathRecipient(observer);
    return ERR_OK;
}

int32_t AppStateObserverManager::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_INFO("%{public}s begin", __func__);
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<std::recursive_mutex> lockUnregister(observerLock_);
    if (observer == nullptr) {
        HILOG_ERROR("Observer nullptr");
        return ERR_INVALID_VALUE;
    }
    std::map<sptr<IApplicationStateObserver>, std::vector<std::string>>::iterator it;
    for (it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        if (it->first->AsObject() == observer->AsObject()) {
            appStateObserverMap_.erase(it);
            HILOG_INFO("%{public}s appStateObserverMap_ size:%{public}d", __func__,
                (int32_t)appStateObserverMap_.size());
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    HILOG_ERROR("Observer not exist.");
    return ERR_INVALID_VALUE;
}

void AppStateObserverManager::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state)
{
    auto task = [weak = weak_from_this(), appRecord, state]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnAppStateChanged failed.");
            return;
        }
        HILOG_INFO("OnAppStateChanged come.");
        self->HandleAppStateChanged(appRecord, state);
    };
    handler_->PostTask(task);
}

void AppStateObserverManager::OnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessDied failed.");
            return;
        }
        HILOG_INFO("OnProcessDied come.");
        self->HandleOnProcessDied(appRecord);
    };
    handler_->PostTask(task);
}

void AppStateObserverManager::OnRenderProcessDied(const std::shared_ptr<RenderRecord> &renderRecord)
{
    auto task = [weak = weak_from_this(), renderRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnRenderProcessDied failed.");
            return;
        }
        HILOG_INFO("OnRenderProcessDied come.");
        self->HandleOnRenderProcessDied(renderRecord);
    };
    handler_->PostTask(task);
}

void AppStateObserverManager::OnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessCreated failed.");
            return;
        }
        HILOG_INFO("OnProcessCreated come.");
        self->HandleOnProcessCreated(appRecord);
    };
    handler_->PostTask(task);
}

void AppStateObserverManager::OnRenderProcessCreated(const std::shared_ptr<RenderRecord> &renderRecord)
{
    auto task = [weak = weak_from_this(), renderRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnRenderProcessCreated failed.");
            return;
        }
        HILOG_INFO("OnRenderProcessCreated come.");
        self->HandleOnRenderProcessCreated(renderRecord);
    };
    handler_->PostTask(task);
}

void AppStateObserverManager::StateChangedNotifyObserver(const AbilityStateData abilityStateData, bool isAbility)
{
    auto task = [weak = weak_from_this(), abilityStateData, isAbility]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, StateChangedNotifyObserver failed.");
            return;
        }
        HILOG_INFO("StateChangedNotifyObserver come.");
        self->HandleStateChangedNotifyObserver(abilityStateData, isAbility);
    };
    handler_->PostTask(task);
}

void AppStateObserverManager::HandleAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_BACKGROUND) {
        AppStateData data = WrapAppStateData(appRecord, state);
        HILOG_DEBUG("OnForegroundApplicationChanged, name:%{public}s, uid:%{public}d, state:%{public}d",
            data.bundleName.c_str(), data.uid, data.state);
        std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
        for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
            std::vector<std::string>::iterator iter = std::find(it->second.begin(),
                it->second.end(), data.bundleName);
            if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
                it->first->OnForegroundApplicationChanged(data);
            }
        }
    }

    if (state == ApplicationState::APP_STATE_CREATE || state == ApplicationState::APP_STATE_TERMINATED) {
        AppStateData data = WrapAppStateData(appRecord, state);
        HILOG_INFO("OnApplicationStateChanged, name:%{public}s, uid:%{public}d, state:%{public}d",
            data.bundleName.c_str(), data.uid, data.state);
        std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
        for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
            std::vector<std::string>::iterator iter = std::find(it->second.begin(),
                it->second.end(), data.bundleName);
            if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
                it->first->OnApplicationStateChanged(data);
            }
        }
    }
}

void AppStateObserverManager::HandleStateChangedNotifyObserver(const AbilityStateData abilityStateData, bool isAbility)
{
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    HILOG_DEBUG("Handle state change, module:%{public}s, bundle:%{public}s, ability:%{public}s, state:%{public}d,"
        "pid:%{public}d ,uid:%{public}d, abilityType:%{public}d, isAbility:%{public}d",
        abilityStateData.moduleName.c_str(), abilityStateData.bundleName.c_str(),
        abilityStateData.abilityName.c_str(), abilityStateData.abilityState,
        abilityStateData.pid, abilityStateData.uid, abilityStateData.abilityType, isAbility);
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), abilityStateData.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            if (isAbility) {
                it->first->OnAbilityStateChanged(abilityStateData);
            } else {
                it->first->OnExtensionStateChanged(abilityStateData);
            }
        }
    }
}

void AppStateObserverManager::HandleOnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    HILOG_DEBUG("Process Create, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, (int32_t)appStateObserverMap_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessCreated(data);
        }
    }
}

void AppStateObserverManager::HandleOnRenderProcessCreated(const std::shared_ptr<RenderRecord> &renderRecord)
{
    if (!renderRecord) {
        HILOG_ERROR("render record is null");
        return;
    }
    ProcessData data = WrapRenderProcessData(renderRecord);
    HILOG_DEBUG("RenderProcess Create, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, (int32_t)appStateObserverMap_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessCreated(data);
        }
    }
}

void AppStateObserverManager::HandleOnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    HILOG_DEBUG("Process died, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d.",
        data.bundleName.c_str(), data.pid, data.uid, (int32_t)appStateObserverMap_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessDied(data);
        }
    }
}

void AppStateObserverManager::HandleOnRenderProcessDied(const std::shared_ptr<RenderRecord> &renderRecord)
{
    if (!renderRecord) {
        HILOG_ERROR("render record is null");
        return;
    }
    ProcessData data = WrapRenderProcessData(renderRecord);
    HILOG_DEBUG("Render Process died, bundle:%{public}s, pid:%{public}d, uid:%{public}d, size:%{public}d.",
        data.bundleName.c_str(), data.pid, data.uid, (int32_t)appStateObserverMap_.size());
    std::lock_guard<std::recursive_mutex> lockNotify(observerLock_);
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessDied(data);
        }
    }
}

ProcessData AppStateObserverManager::WrapProcessData(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    ProcessData processData;
    processData.bundleName = appRecord->GetBundleName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.uid = appRecord->GetUid();
    return processData;
}

ProcessData AppStateObserverManager::WrapRenderProcessData(const std::shared_ptr<RenderRecord> &renderRecord)
{
    ProcessData processData;
    processData.bundleName = renderRecord->GetHostBundleName();
    processData.pid = renderRecord->GetPid();
    processData.uid = renderRecord->GetHostUid();
    return processData;
}

bool AppStateObserverManager::ObserverExist(const sptr<IApplicationStateObserver> &observer)
{
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return false;
    }
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        if (it->first->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

void AppStateObserverManager::AddObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_INFO("Add observer death recipient begin.");
    if (observer == nullptr || observer->AsObject() == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        HILOG_ERROR("This death recipient has been added.");
        return;
    } else {
        std::weak_ptr<AppStateObserverManager> thisWeakPtr(shared_from_this());
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new ApplicationStateObserverRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto appStateObserverManager = thisWeakPtr.lock();
                if (appStateObserverManager) {
                    appStateObserverManager->OnObserverDied(remote);
                }
            });
        observer->AsObject()->AddDeathRecipient(deathRecipient);
        recipientMap_.emplace(observer->AsObject(), deathRecipient);
    }
}

void AppStateObserverManager::RemoveObserverDeathRecipient(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_INFO("Remove observer death recipient begin.");
    if (observer == nullptr || observer->AsObject() == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return;
    }
    auto it = recipientMap_.find(observer->AsObject());
    if (it != recipientMap_.end()) {
        it->first->RemoveDeathRecipient(it->second);
        recipientMap_.erase(it);
        return;
    }
}

void AppStateObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("%{public}s begin", __func__);
    auto object = remote.promote();
    if (object == nullptr) {
        HILOG_ERROR("observer nullptr.");
        return;
    }
    sptr<IApplicationStateObserver> observer = iface_cast<IApplicationStateObserver>(object);
    UnregisterApplicationStateObserver(observer);
}

AppStateData AppStateObserverManager::WrapAppStateData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppStateData appStateData;
    appStateData.pid = appRecord->GetPriorityObject()->GetPid();
    appStateData.bundleName = appRecord->GetBundleName();
    appStateData.state = static_cast<int32_t>(state);
    appStateData.uid = appRecord->GetUid();
    appStateData.accessTokenId = static_cast<int32_t>(appRecord->GetApplicationInfo()->accessTokenId);
    return appStateData;
}
}  // namespace AppExecFwk
}  // namespace OHOS
