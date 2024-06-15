/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "application_context.h"

#include <algorithm>

#include "ability_manager_errors.h"
#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "running_process_info.h"

namespace OHOS {
namespace AbilityRuntime {
std::vector<std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::vector<std::shared_ptr<EnvironmentCallback>> ApplicationContext::envCallbacks_;
std::vector<std::weak_ptr<ApplicationStateChangeCallback>> ApplicationContext::applicationStateCallback_;

std::shared_ptr<ApplicationContext> ApplicationContext::GetInstance()
{
    if (applicationContext_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(Context::contextMutex_);
        if (applicationContext_ == nullptr) {
            applicationContext_ = std::make_shared<ApplicationContext>();
        }
    }
    return applicationContext_;
}

void ApplicationContext::AttachContextImpl(const std::shared_ptr<ContextImpl> &contextImpl)
{
    contextImpl_ = contextImpl;
}

void ApplicationContext::RegisterAbilityLifecycleCallback(
    const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ApplicationContext RegisterAbilityLifecycleCallback");
    if (abilityLifecycleCallback == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    callbacks_.push_back(abilityLifecycleCallback);
}

void ApplicationContext::UnregisterAbilityLifecycleCallback(
    const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ApplicationContext UnregisterAbilityLifecycleCallback");
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    auto it = std::find(callbacks_.begin(), callbacks_.end(), abilityLifecycleCallback);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

bool ApplicationContext::IsAbilityLifecycleCallbackEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    return callbacks_.empty();
}

void ApplicationContext::RegisterEnvironmentCallback(
    const std::shared_ptr<EnvironmentCallback> &environmentCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ApplicationContext RegisterEnvironmentCallback");
    if (environmentCallback == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    envCallbacks_.push_back(environmentCallback);
}

void ApplicationContext::UnregisterEnvironmentCallback(
    const std::shared_ptr<EnvironmentCallback> &environmentCallback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ApplicationContext UnregisterEnvironmentCallback");
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    auto it = std::find(envCallbacks_.begin(), envCallbacks_.end(), environmentCallback);
    if (it != envCallbacks_.end()) {
        envCallbacks_.erase(it);
    }
}

void ApplicationContext::RegisterApplicationStateChangeCallback(
    const std::weak_ptr<ApplicationStateChangeCallback> &applicationStateChangeCallback)
{
    std::lock_guard<std::recursive_mutex> lock(applicationStateCallbackLock_);
    applicationStateCallback_.push_back(applicationStateChangeCallback);
}

void ApplicationContext::DispatchOnAbilityCreate(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityCreate(ability);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageCreate(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageDestroy(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchWindowStageFocus(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s start.", __func__);
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is null");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageActive(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchWindowStageUnfocus(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageInactive(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityDestroy(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityForeground(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityBackground(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityContinue(const std::shared_ptr<NativeReference> &ability)
{
    if (!ability) {
        TAG_LOGE(AAFwkTag::APPKIT, "ability is nullptr");
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityContinue(ability);
        }
    }
}

void ApplicationContext::DispatchConfigurationUpdated(const AppExecFwk::Configuration &config)
{
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    for (auto envCallback : envCallbacks_) {
        if (envCallback != nullptr) {
            envCallback->OnConfigurationUpdated(config);
        }
    }
}

void ApplicationContext::DispatchMemoryLevel(const int level)
{
    std::lock_guard<std::recursive_mutex> lock(envCallbacksLock_);
    for (auto envCallback : envCallbacks_) {
        if (envCallback != nullptr) {
            envCallback->OnMemoryLevel(level);
        }
    }
}

void ApplicationContext::NotifyApplicationForeground()
{
    std::lock_guard<std::recursive_mutex> lock(applicationStateCallbackLock_);
    for (auto callback : applicationStateCallback_) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifyApplicationForeground();
        }
    }
}

void ApplicationContext::NotifyApplicationBackground()
{
    std::lock_guard<std::recursive_mutex> lock(applicationStateCallbackLock_);
    for (auto callback : applicationStateCallback_) {
        auto callbackSptr = callback.lock();
        if (callbackSptr != nullptr) {
            callbackSptr->NotifyApplicationBackground();
        }
    }
}

std::string ApplicationContext::GetBundleName() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleName() : "";
}

std::shared_ptr<Context> ApplicationContext::CreateBundleContext(const std::string &bundleName)
{
    return (contextImpl_ != nullptr) ? contextImpl_->CreateBundleContext(bundleName) : nullptr;
}

std::shared_ptr<Context> ApplicationContext::CreateModuleContext(const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleContext(moduleName) : nullptr;
}

std::shared_ptr<Context> ApplicationContext::CreateModuleContext(const std::string &bundleName,
                                                                 const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleContext(bundleName, moduleName) : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    return contextImpl_ ? contextImpl_->CreateModuleResourceManager(bundleName, moduleName) : nullptr;
}

int32_t ApplicationContext::CreateSystemHspModuleResourceManager(const std::string &bundleName,
    const std::string &moduleName, std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    return contextImpl_ ?
        contextImpl_->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager) : ERR_INVALID_VALUE;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ApplicationContext::GetApplicationInfo() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetApplicationInfo() : nullptr;
}

void ApplicationContext::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetApplicationInfo(info);
    }
    applicationInfoUpdateFlag_ = true;
}

bool ApplicationContext::GetApplicationInfoUpdateFlag() const
{
    return applicationInfoUpdateFlag_;
}

void ApplicationContext::SetApplicationInfoUpdateFlag(bool flag)
{
    applicationInfoUpdateFlag_ = flag;
}

std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::GetResourceManager() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetResourceManager() : nullptr;
}

std::string ApplicationContext::GetBundleCodePath() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return nullptr;
}

std::string ApplicationContext::GetBundleCodeDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBundleCodeDir() : "";
}

std::string ApplicationContext::GetCacheDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetCacheDir() : "";
}

std::string ApplicationContext::GetTempDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetTempDir() : "";
}

void ApplicationContext::GetAllTempDir(std::vector<std::string> &tempPaths)
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "The contextimpl is nullptr");
        return;
    }
    contextImpl_->GetAllTempDir(tempPaths);
}

std::string ApplicationContext::GetResourceDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetResourceDir() : "";
}

std::string ApplicationContext::GetFilesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetFilesDir() : "";
}

void ApplicationContext::KillProcessBySelf()
{
    if (contextImpl_ != nullptr) {
        contextImpl_->KillProcessBySelf();
    }
}

int32_t ApplicationContext::GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info)
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetProcessRunningInformation(info) : -1;
}

bool ApplicationContext::IsUpdatingConfigurations()
{
    return (contextImpl_ != nullptr) ? contextImpl_->IsUpdatingConfigurations() : false;
}

bool ApplicationContext::PrintDrawnCompleted()
{
    return (contextImpl_ != nullptr) ? contextImpl_->PrintDrawnCompleted() : false;
}

std::string ApplicationContext::GetDatabaseDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDatabaseDir() : "";
}

std::string ApplicationContext::GetPreferencesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetPreferencesDir() : "";
}

int32_t ApplicationContext::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    return contextImpl_ ?
        contextImpl_->GetSystemDatabaseDir(groupId, checkExist, databaseDir) : ERR_INVALID_VALUE;
}

int32_t ApplicationContext::GetSystemPreferencesDir(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    return contextImpl_ ?
        contextImpl_->GetSystemPreferencesDir(groupId, checkExist, preferencesDir) : ERR_INVALID_VALUE;
}

std::string ApplicationContext::GetGroupDir(std::string groupId)
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetGroupDir(groupId) : "";
}

int32_t ApplicationContext::RestartApp(const AAFwk::Want& want)
{
    std::string abilityName = want.GetElement().GetAbilityName();
    if (abilityName == "") {
        TAG_LOGE(AAFwkTag::APPKIT, "abilityName is empty.");
        return ERR_INVALID_VALUE;
    }
    std::string bundleName = GetBundleName();
    const_cast<AAFwk::Want &>(want).SetBundle(bundleName);
    return (contextImpl_ != nullptr) ? contextImpl_->RestartApp(want) : ERR_INVALID_VALUE;
}

std::string ApplicationContext::GetDistributedFilesDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDistributedFilesDir() : "";
}

std::string ApplicationContext::GetCloudFileDir()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetCloudFileDir() : "";
}

sptr<IRemoteObject> ApplicationContext::GetToken()
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetToken() : nullptr;
}

void ApplicationContext::SetToken(const sptr<IRemoteObject> &token)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SetToken(token);
    }
}

void ApplicationContext::SwitchArea(int mode)
{
    if (contextImpl_ != nullptr) {
        contextImpl_->SwitchArea(mode);
    }
}

void ApplicationContext::SetColorMode(int32_t colorMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "colorMode:%{public}d.", colorMode);
    if (colorMode < -1 || colorMode > 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "colorMode is invalid.");
        return;
    }
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::GetColorModeStr(colorMode));
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
        AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::SetLanguage(const std::string &language)
{
    TAG_LOGD(AAFwkTag::APPKIT, "language:%{public}s.", language.c_str());
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
    config.AddItem(AAFwk::GlobalConfigurationKey::LANGUAGE_IS_SET_BY_APP,
        AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::ClearUpApplicationData()
{
    if (contextImpl_ != nullptr) {
        contextImpl_->ClearUpApplicationData();
    }
}

int ApplicationContext::GetArea()
{
    if (contextImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "AbilityContext::contextImpl is nullptr.");
        return ContextImpl::EL_DEFAULT;
    }
    return contextImpl_->GetArea();
}

std::shared_ptr<AppExecFwk::Configuration> ApplicationContext::GetConfiguration() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetConfiguration() : nullptr;
}

std::string ApplicationContext::GetBaseDir() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetBaseDir() : nullptr;
}

Global::Resource::DeviceType ApplicationContext::GetDeviceType() const
{
    return (contextImpl_ != nullptr) ? contextImpl_->GetDeviceType() : Global::Resource::DeviceType::DEVICE_PHONE;
}

void ApplicationContext::RegisterAppConfigUpdateObserver(AppConfigUpdateCallback appConfigChangeCallback)
{
    appConfigChangeCallback_ = appConfigChangeCallback;
}

std::string ApplicationContext::GetAppRunningUniqueId() const
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetAppRunningUniqueId is %{public}s.", appRunningUniqueId_.c_str());
    return appRunningUniqueId_;
}

void ApplicationContext::SetAppRunningUniqueId(const std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetAppRunningUniqueId is %{public}s.", appRunningUniqueId.c_str());
    appRunningUniqueId_ = appRunningUniqueId;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
