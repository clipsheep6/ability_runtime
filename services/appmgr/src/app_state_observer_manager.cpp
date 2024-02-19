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

#include "ability_foreground_state_observer_stub.h"
#include "app_foreground_state_observer_stub.h"
#include "application_state_observer_stub.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "remote_client_manager.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string THREAD_NAME = "AppStateObserverManager";
const int BUNDLE_NAME_LIST_MAX_SIZE = 128;
} // namespace
AppStateObserverManager::AppStateObserverManager()
{
    HILOG_DEBUG("AppStateObserverManager instance is created");
}

AppStateObserverManager::~AppStateObserverManager()
{
    HILOG_DEBUG("AppStateObserverManager instance is destroyed");
}

void AppStateObserverManager::Init()
{
    if (!handler_) {
        handler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("app_state_task_queue");
    }
}

int32_t AppStateObserverManager::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    HILOG_DEBUG("called");
    if (bundleNameList.size() > BUNDLE_NAME_LIST_MAX_SIZE) {
        HILOG_ERROR("the bundleNameList passed in is too long");
        return ERR_INVALID_VALUE;
    }
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (ObserverExist(observer)) {
        HILOG_ERROR("Observer exist.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<ffrt::mutex> lockRegister(observerLock_);
    appStateObserverMap_.emplace(observer, bundleNameList);
    HILOG_DEBUG("appStateObserverMap_ size:%{public}zu", appStateObserverMap_.size());
    AddObserverDeathRecipient(observer, ObserverType::APPLICATION_STATE_OBSERVER);
    return ERR_OK;
}

int32_t AppStateObserverManager::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_DEBUG("called");
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<ffrt::mutex> lockUnregister(observerLock_);
    if (observer == nullptr) {
        HILOG_ERROR("Observer nullptr");
        return ERR_INVALID_VALUE;
    }
    std::map<sptr<IApplicationStateObserver>, std::vector<std::string>>::iterator it;
    for (it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        if (it->first->AsObject() == observer->AsObject()) {
            appStateObserverMap_.erase(it);
            HILOG_DEBUG("appStateObserverMap_ size:%{public}zu", appStateObserverMap_.size());
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    HILOG_ERROR("Observer not exist.");
    return ERR_INVALID_VALUE;
}

int32_t AppStateObserverManager::RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    if (IsAppForegroundObserverExist(observer)) {
        HILOG_ERROR("Observer exist.");
        return ERR_INVALID_VALUE;
    }

    std::lock_guard<ffrt::mutex> lockRegister(appForegroundObserverLock_);
    appForegroundStateObserverSet_.emplace(observer);
    AddObserverDeathRecipient(observer, ObserverType::APP_FOREGROUND_STATE_OBSERVER);
    return ERR_OK;
}

int32_t AppStateObserverManager::UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (observer == nullptr) {
        HILOG_ERROR("Observer nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<ffrt::mutex> lockUnregister(appForegroundObserverLock_);
    for (auto &it : appForegroundStateObserverSet_) {
        if (it != nullptr && it->AsObject() == observer->AsObject()) {
            appForegroundStateObserverSet_.erase(it);
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    return ERR_INVALID_VALUE;
}

int32_t AppStateObserverManager::RegisterAbilityForegroundStateObserver(
    const sptr<IAbilityForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    if (IsAbilityForegroundObserverExist(observer)) {
        HILOG_DEBUG("Observer exist.");
        return ERR_OK;
    }

    std::lock_guard<ffrt::mutex> lockRegister(abilityforegroundObserverLock_);
    abilityforegroundObserverSet_.emplace(observer);
    AddObserverDeathRecipient(observer, ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER);
    return ERR_OK;
}

int32_t AppStateObserverManager::UnregisterAbilityForegroundStateObserver(
    const sptr<IAbilityForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (observer == nullptr) {
        HILOG_ERROR("Observer nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (AAFwk::PermissionVerification::GetInstance()->VerifyAppStateObserverPermission() == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    std::lock_guard<ffrt::mutex> lockUnregister(abilityforegroundObserverLock_);
    for (auto &it : abilityforegroundObserverSet_) {
        if (it != nullptr && it->AsObject() == observer->AsObject()) {
            abilityforegroundObserverSet_.erase(it);
            RemoveObserverDeathRecipient(observer);
            return ERR_OK;
        }
    }
    return ERR_INVALID_VALUE;
}

void AppStateObserverManager::OnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnAppStarted failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnAppStarted failed.");
            return;
        }
        HILOG_DEBUG("OnAppStarted come.");
        self->HandleOnAppStarted(appRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnAppStopped failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnAppStopped failed.");
            return;
        }
        HILOG_DEBUG("OnAppStopped come.");
        self->HandleOnAppStopped(appRecord);
    };
    handler_->SubmitTask(task);
}


void AppStateObserverManager::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state,
    bool needNotifyApp,
    bool isFromWindowFocusChanged)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnAppStateChanged failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord, state, needNotifyApp, isFromWindowFocusChanged]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnAppStateChanged failed.");
            return;
        }
        HILOG_DEBUG("OnAppStateChanged come.");
        self->dummyCode_ = __LINE__;
        self->HandleAppStateChanged(appRecord, state, needNotifyApp, isFromWindowFocusChanged);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnProcessDied failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessDied failed.");
            return;
        }
        HILOG_DEBUG("OnProcessDied come.");
    self->HandleOnAppProcessDied(appRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnRenderProcessDied(const std::shared_ptr<RenderRecord> &renderRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnRenderProcessDied failed.");
        return;
    }

    auto task = [weak = weak_from_this(), renderRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnRenderProcessDied failed.");
            return;
        }
        HILOG_DEBUG("OnRenderProcessDied come.");
        self->HandleOnRenderProcessDied(renderRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnProcessStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnProcessStateChanged failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessStateChanged failed.");
            return;
        }
        HILOG_DEBUG("OnProcessStateChanged come.");
        self->HandleOnProcessStateChanged(appRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnProcessCreated failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessCreated failed.");
            return;
        }
        self->HandleOnAppProcessCreated(appRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnProcessReused(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnProcessReused failed.");
        return;
    }

    auto task = [weak = weak_from_this(), appRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessReused failed.");
            return;
        }
        HILOG_DEBUG("OnProcessReused come.");
        self->HandleOnProcessResued(appRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnRenderProcessCreated(const std::shared_ptr<RenderRecord> &renderRecord)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnRenderProcessCreated failed.");
        return;
    }

    auto task = [weak = weak_from_this(), renderRecord]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnRenderProcessCreated failed.");
            return;
        }
        HILOG_DEBUG("OnRenderProcessCreated come.");
        self->HandleOnRenderProcessCreated(renderRecord);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::StateChangedNotifyObserver(
    const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged)
{
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, StateChangedNotifyObserver failed.");
        return;
    }

    auto task = [weak = weak_from_this(), abilityStateData, isAbility, isFromWindowFocusChanged]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, StateChangedNotifyObserver failed.");
            return;
        }
        HILOG_DEBUG("StateChangedNotifyObserver come.");
        self->HandleStateChangedNotifyObserver(abilityStateData, isAbility, isFromWindowFocusChanged);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::HandleOnAppStarted(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        HILOG_ERROR("app record is null");
        return;
    }

    AppStateData data = WrapAppStateData(appRecord, ApplicationState::APP_STATE_CREATE);
    HILOG_DEBUG("HandleOnAppStarted, bundle:%{public}s, uid:%{public}d, state:%{public}d",
        data.bundleName.c_str(), data.uid, data.state);
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnAppStarted(data);
        }
    }
}

void AppStateObserverManager::HandleOnAppStopped(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (appRecord == nullptr) {
        HILOG_ERROR("app record is null");
        return;
    }

    AppStateData data = WrapAppStateData(appRecord, ApplicationState::APP_STATE_TERMINATED);
    HILOG_DEBUG("HandleOnAppStopped, bundle:%{public}s, uid:%{public}d, state:%{public}d",
        data.bundleName.c_str(), data.uid, data.state);
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnAppStopped(data);
        }
    }
}

void AppStateObserverManager::HandleAppStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state, bool needNotifyApp, bool isFromWindowFocusChanged)
{
    if (appRecord == nullptr) {
        return;
    }
    dummyCode_ = __LINE__;
    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_BACKGROUND) {
        if (needNotifyApp && !isFromWindowFocusChanged) {
            AppStateData data = WrapAppStateData(appRecord, state);
            appRecord->GetSplitModeAndFloatingMode(data.isSplitScreenMode, data.isFloatingWindowMode);
            dummyCode_ = __LINE__;
            auto appForegroundStateObserverSetCopy = GetAppForegroundStateObserverSetCopy();
            for (auto it : appForegroundStateObserverSetCopy) {
                if (it != nullptr) {
                    it->OnAppStateChanged(data);
                }
            }
        }
        dummyCode_ = __LINE__;
        if (!AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType()) &&
            !AAFwk::UIExtensionUtils::IsWindowExtension(appRecord->GetExtensionType())) {
            AppStateData data = WrapAppStateData(appRecord, state);
            HILOG_DEBUG("HandleAppStateChanged, name:%{public}s, uid:%{public}d, state:%{public}d, notify:%{public}d",
                data.bundleName.c_str(), data.uid, data.state, needNotifyApp);
            dummyCode_ = __LINE__;
            auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
            for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
                std::vector<std::string>::iterator iter =
                    std::find(it->second.begin(), it->second.end(), data.bundleName);
                bool valid = (it->second.empty() || iter != it->second.end()) && it->first != nullptr;
                if (valid) {
                    it->first->OnForegroundApplicationChanged(data);
                }
                if (valid && needNotifyApp) {
                    it->first->OnAppStateChanged(data);
                }
            }
        }
    }
    dummyCode_ = __LINE__;
    if (state == ApplicationState::APP_STATE_CREATE || state == ApplicationState::APP_STATE_TERMINATED) {
        AppStateData data = WrapAppStateData(appRecord, state);
        HILOG_DEBUG("OnApplicationStateChanged, name:%{public}s, uid:%{public}d, state:%{public}d",
            data.bundleName.c_str(), data.uid, data.state);
        dummyCode_ = __LINE__;
        auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
        for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
            std::vector<std::string>::iterator iter = std::find(it->second.begin(),
                it->second.end(), data.bundleName);
            if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
                it->first->OnApplicationStateChanged(data);
            }
        }
    }
}

void AppStateObserverManager::HandleStateChangedNotifyObserver(
    const AbilityStateData abilityStateData, bool isAbility, bool isFromWindowFocusChanged)
{
    HILOG_DEBUG("Handle state change, module:%{public}s, bundle:%{public}s, ability:%{public}s, state:%{public}d,"
        "pid:%{public}d ,uid:%{public}d, abilityType:%{public}d, isAbility:%{public}d, callerBundleName:%{public}s,"
        "callerAbilityName:%{public}s",
        abilityStateData.moduleName.c_str(), abilityStateData.bundleName.c_str(),
        abilityStateData.abilityName.c_str(), abilityStateData.abilityState,
        abilityStateData.pid, abilityStateData.uid, abilityStateData.abilityType, isAbility,
        abilityStateData.callerBundleName.c_str(), abilityStateData.callerAbilityName.c_str());
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
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

    if ((abilityStateData.abilityState == static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND) ||
            abilityStateData.abilityState == static_cast<int32_t>(AbilityState::ABILITY_STATE_BACKGROUND)) &&
        isAbility && !isFromWindowFocusChanged) {
        auto abilityforegroundObserverSetCopy = GetAbilityforegroundObserverSetCopy();
        for (auto &it : abilityforegroundObserverSetCopy) {
            if (it != nullptr) {
                it->OnAbilityStateChanged(abilityStateData);
            }
        }
    }
}

void AppStateObserverManager::HandleOnAppProcessCreated(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    HILOG_INFO("Process Create, bundle:%{public}s, pid:%{public}d, uid:%{public}d, processType:%{public}d, "
        "extensionType:%{public}d, processName:%{public}s, renderUid:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, data.processType, data.extensionType, data.processName.c_str(),
        data.renderUid);
    HandleOnProcessCreated(data);
}

void AppStateObserverManager::HandleOnProcessResued(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    HILOG_DEBUG("Process Resued, bundle:%{public}s, pid:%{public}d, uid:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid);

    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessReused(data);
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
    HILOG_DEBUG("RenderProcess Create, bundle:%{public}s, pid:%{public}d, uid:%{public}d, processType:%{public}d, "
        "processName:%{public}s, renderUid:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, data.processType, data.processName.c_str(), data.renderUid);
    HandleOnProcessCreated(data);
}

void AppStateObserverManager::HandleOnProcessCreated(const ProcessData &data)
{
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessCreated(data);
        }
    }
}

void AppStateObserverManager::HandleOnProcessStateChanged(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    HILOG_DEBUG("bundle:%{public}s pid:%{public}d uid:%{public}d state:%{public}d isContinuousTask:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, data.state, data.isContinuousTask);
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), data.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnProcessStateChanged(data);
        }
    }
}

void AppStateObserverManager::HandleOnAppProcessDied(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("app record is null");
        return;
    }
    ProcessData data = WrapProcessData(appRecord);
    HILOG_DEBUG("Process died, bundle:%{public}s, pid:%{public}d, uid:%{public}d, renderUid:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, data.renderUid);
    HandleOnProcessDied(data);
}

void AppStateObserverManager::HandleOnRenderProcessDied(const std::shared_ptr<RenderRecord> &renderRecord)
{
    if (!renderRecord) {
        HILOG_ERROR("render record is null");
        return;
    }
    ProcessData data = WrapRenderProcessData(renderRecord);
    HILOG_DEBUG("Render Process died, bundle:%{public}s, pid:%{public}d, uid:%{public}d, renderUid:%{public}d",
        data.bundleName.c_str(), data.pid, data.uid, data.renderUid);
    HandleOnProcessDied(data);
}

void AppStateObserverManager::HandleOnProcessDied(const ProcessData &data)
{
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
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
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (applicationInfo) {
        processData.accessTokenId = applicationInfo->accessTokenId;
    }
    processData.state = static_cast<AppProcessState>(appRecord->GetState());
    processData.isContinuousTask = appRecord->IsContinuousTask();
    processData.isKeepAlive = appRecord->IsKeepAliveApp();
    processData.isFocused = appRecord->GetFocusFlag();
    processData.requestProcCode = appRecord->GetRequestProcCode();
    processData.processChangeReason = static_cast<int32_t>(appRecord->GetProcessChangeReason());
    processData.processName = appRecord->GetProcessName();
    processData.extensionType = appRecord->GetExtensionType();
    processData.processType = appRecord->GetProcessType();
    return processData;
}

ProcessData AppStateObserverManager::WrapRenderProcessData(const std::shared_ptr<RenderRecord> &renderRecord)
{
    ProcessData processData;
    processData.bundleName = renderRecord->GetHostBundleName();
    processData.pid = renderRecord->GetPid();
    processData.uid = renderRecord->GetHostUid();
    processData.renderUid = renderRecord->GetUid();
    processData.processName = renderRecord->GetProcessName();
    processData.processType = renderRecord->GetProcessType();
    return processData;
}

bool AppStateObserverManager::ObserverExist(const sptr<IRemoteBroker> &observer)
{
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return false;
    }
    std::lock_guard<ffrt::mutex> lockRegister(observerLock_);
    for (auto it = appStateObserverMap_.begin(); it != appStateObserverMap_.end(); ++it) {
        if (it->first->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

bool AppStateObserverManager::IsAbilityForegroundObserverExist(const sptr<IRemoteBroker> &observer)
{
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return false;
    }
    std::lock_guard<ffrt::mutex> lockRegister(abilityforegroundObserverLock_);
    for (auto &it : abilityforegroundObserverSet_) {
        if (it != nullptr && it->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

bool AppStateObserverManager::IsAppForegroundObserverExist(const sptr<IRemoteBroker> &observer)
{
    if (observer == nullptr) {
        HILOG_ERROR("The param observer is nullptr.");
        return false;
    }
    std::lock_guard<ffrt::mutex> lockRegister(appForegroundObserverLock_);
    for (auto &it : appForegroundStateObserverSet_) {
        if (it != nullptr && it->AsObject() == observer->AsObject()) {
            return true;
        }
    }
    return false;
}

void AppStateObserverManager::AddObserverDeathRecipient(const sptr<IRemoteBroker> &observer, const ObserverType &type)
{
    HILOG_DEBUG("Add observer death recipient begin.");
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
        sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
        auto deathRecipientFunc = [thisWeakPtr, type](const wptr<IRemoteObject> &remote) {
            auto appStateObserverManager = thisWeakPtr.lock();
            if (appStateObserverManager) {
                appStateObserverManager->OnObserverDied(remote, type);
            }
        };
        if (type == ObserverType::APPLICATION_STATE_OBSERVER) {
            deathRecipient = new (std::nothrow) ApplicationStateObserverRecipient(deathRecipientFunc);
        } else if (type == ObserverType::APP_FOREGROUND_STATE_OBSERVER) {
            deathRecipient = new (std::nothrow) AppForegroundStateObserverRecipient(deathRecipientFunc);
        } else if (type == ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER) {
            deathRecipient = new (std::nothrow) AbilityForegroundStateObserverRecipient(deathRecipientFunc);
        } else {
            HILOG_WARN("ObserverType is not exists");
            return;
        }
        if (deathRecipient == nullptr) {
            HILOG_ERROR("deathRecipient is nullptr.");
            return;
        }
        if (!observer->AsObject()->AddDeathRecipient(deathRecipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
        recipientMap_.emplace(observer->AsObject(), deathRecipient);
    }
}

void AppStateObserverManager::RemoveObserverDeathRecipient(const sptr<IRemoteBroker> &observer)
{
    HILOG_DEBUG("Remove observer death recipient begin.");
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

AppStateObserverMap AppStateObserverManager::GetAppStateObserverMapCopy()
{
    std::lock_guard<ffrt::mutex> lock(observerLock_);
    return appStateObserverMap_;
}

AppForegroundStateObserverSet AppStateObserverManager::GetAppForegroundStateObserverSetCopy()
{
    std::lock_guard<ffrt::mutex> lock(appForegroundObserverLock_);
    return appForegroundStateObserverSet_;
}

AbilityforegroundObserverSet AppStateObserverManager::GetAbilityforegroundObserverSetCopy()
{
    std::lock_guard<ffrt::mutex> lock(abilityforegroundObserverLock_);
    return abilityforegroundObserverSet_;
}

void AppStateObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote, const ObserverType &type)
{
    HILOG_INFO("OnObserverDied");
    auto object = remote.promote();
    if (object == nullptr) {
        HILOG_ERROR("observer nullptr.");
        return;
    }

    if (type == ObserverType::APPLICATION_STATE_OBSERVER) {
        sptr<IApplicationStateObserver> observer = iface_cast<IApplicationStateObserver>(object);
        UnregisterApplicationStateObserver(observer);
    } else if (type == ObserverType::ABILITY_FOREGROUND_STATE_OBSERVER) {
        sptr<IAbilityForegroundStateObserver> observer = iface_cast<IAbilityForegroundStateObserver>(object);
        UnregisterAbilityForegroundStateObserver(observer);
    } else if (type == ObserverType::APP_FOREGROUND_STATE_OBSERVER) {
        sptr<IAppForegroundStateObserver> observer = iface_cast<IAppForegroundStateObserver>(object);
        UnregisterAppForegroundStateObserver(observer);
    } else {
        HILOG_WARN("ObserverType is not exists");
        return;
    }
}

AppStateData AppStateObserverManager::WrapAppStateData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppStateData appStateData;
    appStateData.pid = appRecord->GetPriorityObject()->GetPid();
    appStateData.bundleName = appRecord->GetBundleName();
    appStateData.state = static_cast<int32_t>(state);
    appStateData.uid = appRecord->GetUid();
    appStateData.extensionType = appRecord->GetExtensionType();
    if (appRecord->GetApplicationInfo() != nullptr) {
        appStateData.accessTokenId = static_cast<int32_t>(appRecord->GetApplicationInfo()->accessTokenId);
    }
    appStateData.isFocused = appRecord->GetFocusFlag();
    auto renderRecordMap = appRecord->GetRenderRecordMap();
    if (!renderRecordMap.empty()) {
        for (auto iter : renderRecordMap) {
            auto renderRecord = iter.second;
            if (renderRecord != nullptr) {
                appStateData.renderPids.emplace_back(renderRecord->GetPid());
            }
        }
    }
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    auto bundleMgr = remoteClientManager->GetBundleManagerHelper();
    std::string callerBundleName;
    if (bundleMgr != nullptr &&
        IN_PROCESS_CALL(bundleMgr->GetNameForUid(appRecord->GetCallerUid(), callerBundleName)) == ERR_OK) {
        appStateData.callerBundleName = callerBundleName;
    } else {
        appStateData.callerBundleName = "";
    }
    HILOG_DEBUG("Handle state change, bundle:%{public}s, state:%{public}d,"
        "pid:%{public}d ,uid:%{public}d, isFocused:%{public}d, callerBUndleName: %{public}s",
        appStateData.bundleName.c_str(), appStateData.state,
        appStateData.pid, appStateData.uid, appStateData.isFocused, appStateData.callerBundleName.c_str());
    return appStateData;
}

void AppStateObserverManager::OnPageShow(const PageStateData pageStateData)
{
    HILOG_DEBUG("call");
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnProcessCreated failed.");
        return;
    }

    auto task = [weak = weak_from_this(), pageStateData]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessCreated failed.");
            return;
        }
        HILOG_DEBUG("OnProcessCreated come.");
        self->HandleOnPageShow(pageStateData);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::OnPageHide(const PageStateData pageStateData)
{
    HILOG_DEBUG("call");
    if (handler_ == nullptr) {
        HILOG_ERROR("handler is nullptr, OnProcessCreated failed.");
        return;
    }

    auto task = [weak = weak_from_this(), pageStateData]() {
        auto self = weak.lock();
        if (self == nullptr) {
            HILOG_ERROR("self is nullptr, OnProcessCreated failed.");
            return;
        }
        HILOG_DEBUG("OnProcessCreated come.");
        self->HandleOnPageHide(pageStateData);
    };
    handler_->SubmitTask(task);
}

void AppStateObserverManager::HandleOnPageShow(const PageStateData pageStateData)
{
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), pageStateData.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnPageShow(pageStateData);
        }
    }
}

void AppStateObserverManager::HandleOnPageHide(const PageStateData pageStateData)
{
    auto appStateObserverMapCopy = GetAppStateObserverMapCopy();
    for (auto it = appStateObserverMapCopy.begin(); it != appStateObserverMapCopy.end(); ++it) {
        std::vector<std::string>::iterator iter = std::find(it->second.begin(),
            it->second.end(), pageStateData.bundleName);
        if ((it->second.empty() || iter != it->second.end()) && it->first != nullptr) {
            it->first->OnPageHide(pageStateData);
        }
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
