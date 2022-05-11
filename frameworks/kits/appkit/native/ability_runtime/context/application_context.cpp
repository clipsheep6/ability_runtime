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

#include "application_context.h"

#include <algorithm>

#include "configuration.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
std::map<int64_t, std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::shared_ptr<ApplicationContext> ApplicationContext::instance_ = nullptr;
std::mutex ApplicationContext::mutex_;

std::shared_ptr<ApplicationContext> ApplicationContext::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<ApplicationContext>();
        }
    }
    return instance_;
}

void ApplicationContext::RegisterAbilityLifecycleCallback(
    const int64_t callbackId, const std::shared_ptr<AbilityLifecycleCallback> &abilityLifecycleCallback)
{
    HILOG_INFO("ApplicationContext RegisterAbilityLifecycleCallback");
    callbacks_.emplace(callbackId, abilityLifecycleCallback);
}

void ApplicationContext::UnregisterAbilityLifecycleCallback(const int64_t callbackId)
{
    HILOG_INFO("ApplicationContext UnregisterAbilityLifecycleCallback");
    auto iter = callbacks_.find(callbackId);
    if (iter != callbacks_.end()) {
        HILOG_INFO("callbackId exist");
        callbacks_.erase(iter);
    }
}

void ApplicationContext::DispatchOnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityCreate(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityWindowStageCreate(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityWindowStageDestroy(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityDestroy(abilityObj);
    }
}

void ApplicationContext::DispatchOnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityForeground(abilityObj);
    }
}
void ApplicationContext::DispatchOnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityBackground(abilityObj);
    }
}
void ApplicationContext::DispatchOnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj)
{
    for (auto callback : callbacks_) {
        callback.second->OnAbilityContinue(abilityObj);
    }
}

void ApplicationContext::SetStageContext(const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    stageContext_ = stageContext;
}

std::string ApplicationContext::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}
std::shared_ptr<Context> ApplicationContext::CreateBundleContext(const std::string &bundleName)
{
    return stageContext_ ? stageContext_->CreateBundleContext(bundleName) : nullptr;
}
std::shared_ptr<AppExecFwk::ApplicationInfo> ApplicationContext::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}
std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}
std::string ApplicationContext::GetBundleCodePath() const
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}
std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}
std::string ApplicationContext::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}
std::string ApplicationContext::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}
std::string ApplicationContext::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}
std::string ApplicationContext::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}
bool ApplicationContext::IsUpdatingConfigurations()
{
    return stageContext_ ? stageContext_->IsUpdatingConfigurations() : false;
}
bool ApplicationContext::PrintDrawnCompleted()
{
    return stageContext_ ? stageContext_->PrintDrawnCompleted() : false;
}
std::string ApplicationContext::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}
std::string ApplicationContext::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}
std::string ApplicationContext::GetDistributedFilesDir()
{
    return stageContext_ ? stageContext_->GetDistributedFilesDir() : "";
}
sptr<IRemoteObject> ApplicationContext::GetToken()
{
    return token_;
}
void ApplicationContext::SetToken(const sptr<IRemoteObject> &token)
{
    token_ = token;
}
void ApplicationContext::SwitchArea(int mode)
{
    HILOG_DEBUG("ApplicationContext::SwitchArea.");
    if (stageContext_ != nullptr) {
        stageContext_->SwitchArea(mode);
    }
}

void ApplicationContext::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    config_ = config;
}

std::shared_ptr<AppExecFwk::Configuration> ApplicationContext::GetConfiguration() const
{
    return config_;
}
std::string ApplicationContext::GetBaseDir() const
{
    return stageContext_ ? stageContext_->GetBaseDir() : "";
}
}  // namespace AbilityRuntime
}  // namespace OHOS