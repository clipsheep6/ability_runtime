/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include <sys/stat.h>

#include "ohos_application.h"

#include "ability.h"
#include "ability_record_mgr.h"
#include "ability_thread.h"
#include "app_loader.h"
#include "application_context.h"
#include "application_cleaner.h"
#include "application_impl.h"
#include "bundle_mgr_helper.h"
#include "configuration_utils.h"
#include "context_impl.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "runtime.h"
#include "system_ability_definition.h"
#include "ui_ability.h"
#ifdef SUPPORT_GRAPHICS
#include "window.h"
#endif

namespace OHOS {
namespace AppExecFwk {
REGISTER_APPLICATION(OHOSApplication, OHOSApplication)
constexpr int32_t APP_ENVIRONMENT_OVERWRITE = 1;

OHOSApplication::OHOSApplication()
{
    HILOG_DEBUG("called");
    abilityLifecycleCallbacks_.clear();
    elementsCallbacks_.clear();
}

OHOSApplication::~OHOSApplication()
{
}

/**
 *
 * @brief Called when Ability#onSaveAbilityState(PacMap) was called on an ability.
 *
 * @param outState Indicates the PacMap object passed to Ability#onSaveAbilityState(PacMap)
 * for storing user data and states. This parameter cannot be null.
 */

void OHOSApplication::DispatchAbilitySavedState(const PacMap &outState)
{
    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilitySaveState(outState);
        }
    }
}

/**
 *
 * @brief Will be called the application foregrounds
 *
 */
void OHOSApplication::OnForeground()
{
    HILOG_DEBUG("begin");
    if (abilityRuntimeContext_) {
        abilityRuntimeContext_->NotifyApplicationForeground();
    }

    if (runtime_ == nullptr) {
        HILOG_DEBUG("NotifyApplicationState, runtime_ is nullptr");
        return;
    }
    runtime_->NotifyApplicationState(false);
    HILOG_DEBUG("NotifyApplicationState::OnForeground end");
}

/**
 *
 * @brief Will be called the application backgrounds
 *
 */
void OHOSApplication::OnBackground()
{
    HILOG_DEBUG("begin");
    if (abilityRuntimeContext_) {
        abilityRuntimeContext_->NotifyApplicationBackground();
    }

    if (runtime_ == nullptr) {
        HILOG_DEBUG("NotifyApplicationState, runtime_ is nullptr");
        return;
    }
    runtime_->NotifyApplicationState(true);
    HILOG_DEBUG("NotifyApplicationState::OnBackground end");
}

void OHOSApplication::DumpApplication()
{
    HILOG_DEBUG("called");
    // create and initialize abilityInfos
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AbilityLocalRecord> record = nullptr;

    if (abilityRecordMgr_) {
        record = abilityRecordMgr_->GetAbilityItem(abilityRecordMgr_->GetToken());
    }

    if (record) {
        abilityInfo = record->GetAbilityInfo();
    }

    if (abilityInfo) {
        HILOG_DEBUG("==============AbilityInfo==============");
        HILOG_DEBUG("abilityInfo: package: %{public}s", abilityInfo->package.c_str());
        HILOG_DEBUG("abilityInfo: name: %{public}s", abilityInfo->name.c_str());
        HILOG_DEBUG("abilityInfo: label: %{public}s", abilityInfo->label.c_str());
        HILOG_DEBUG("abilityInfo: description: %{public}s", abilityInfo->description.c_str());
        HILOG_DEBUG("abilityInfo: iconPath: %{public}s", abilityInfo->iconPath.c_str());
        HILOG_DEBUG("abilityInfo: visible: %{public}d", abilityInfo->visible);
        HILOG_DEBUG("abilityInfo: kind: %{public}s", abilityInfo->kind.c_str());
        HILOG_DEBUG("abilityInfo: type: %{public}d", abilityInfo->type);
        HILOG_DEBUG("abilityInfo: orientation: %{public}d", abilityInfo->orientation);
        HILOG_DEBUG("abilityInfo: launchMode: %{public}d", abilityInfo->launchMode);
        for (auto permission : abilityInfo->permissions) {
            HILOG_DEBUG("abilityInfo: permission: %{public}s", permission.c_str());
        }
        HILOG_DEBUG("abilityInfo: bundleName: %{public}s", abilityInfo->bundleName.c_str());
        HILOG_DEBUG("abilityInfo: applicationName: %{public}s", abilityInfo->applicationName.c_str());
    }

    // create and initialize applicationInfo
    HILOG_DEBUG("==============applicationInfo==============");
    std::shared_ptr<ApplicationInfo> applicationInfoPtr = GetApplicationInfo();
    if (applicationInfoPtr != nullptr) {
        HILOG_DEBUG("applicationInfo: name: %{public}s", applicationInfoPtr->name.c_str());
        HILOG_DEBUG("applicationInfo: bundleName: %{public}s", applicationInfoPtr->bundleName.c_str());
        HILOG_DEBUG("applicationInfo: signatureKey: %{public}s", applicationInfoPtr->signatureKey.c_str());
    }
}

/**
 * @brief Set Runtime
 *
 * @param runtime Runtime instance.
 */
void OHOSApplication::SetRuntime(std::unique_ptr<AbilityRuntime::Runtime>&& runtime)
{
    HILOG_DEBUG("begin");
    if (runtime == nullptr) {
        HILOG_ERROR("OHOSApplication::SetRuntime failed, runtime is empty");
        return;
    }
    runtime_ = std::move(runtime);
    HILOG_DEBUG("end");
}

/**
 * @brief Set ApplicationContext
 *
 * @param abilityRuntimeContext ApplicationContext instance.
 */
void OHOSApplication::SetApplicationContext(
    const std::shared_ptr<AbilityRuntime::ApplicationContext> &abilityRuntimeContext)
{
    HILOG_DEBUG("called");
    if (abilityRuntimeContext == nullptr) {
        HILOG_ERROR("OHOSApplication::SetApplicationContext failed, context is empty");
        return;
    }
    abilityRuntimeContext_ = abilityRuntimeContext;
    auto application = std::static_pointer_cast<OHOSApplication>(shared_from_this());
    std::weak_ptr<OHOSApplication> applicationWptr = application;
    abilityRuntimeContext_->RegisterAppConfigUpdateObserver([applicationWptr](const Configuration &config) {
        std::shared_ptr<OHOSApplication> applicationSptr = applicationWptr.lock();
        if (applicationSptr == nullptr) {
            return;
        }
        applicationSptr->OnConfigurationUpdated(config);
    });
}

/**
 *
 * @brief Set the abilityRecordMgr to the OHOSApplication.
 *
 * @param abilityRecordMgr
 */
void OHOSApplication::SetAbilityRecordMgr(const std::shared_ptr<AbilityRecordMgr> &abilityRecordMgr)
{
    HILOG_DEBUG("called");
    if (abilityRecordMgr == nullptr) {
        HILOG_ERROR("ContextDeal::SetAbilityRecordMgr failed, abilityRecordMgr is nullptr");
        return;
    }
    abilityRecordMgr_ = abilityRecordMgr;
}

/**
 *
 * Register AbilityLifecycleCallbacks with OHOSApplication
 *
 * @param callBack callBack When the life cycle of the ability in the application changes,
 */
void OHOSApplication::RegisterAbilityLifecycleCallbacks(const std::shared_ptr<AbilityLifecycleCallbacks> &callBack)
{
    HILOG_DEBUG("called");

    if (callBack == nullptr) {
        HILOG_DEBUG("observer is null");
        return;
    }

    abilityLifecycleCallbacks_.emplace_back(callBack);
}

/**
 *
 * Unregister AbilityLifecycleCallbacks with OHOSApplication
 *
 * @param callBack RegisterAbilityLifecycleCallbacks`s callBack
 */
void OHOSApplication::UnregisterAbilityLifecycleCallbacks(const std::shared_ptr<AbilityLifecycleCallbacks> &callBack)
{
    HILOG_DEBUG("called");

    if (callBack == nullptr) {
        HILOG_DEBUG("observer is null");
        return;
    }

    abilityLifecycleCallbacks_.remove(callBack);
}

/**
 *
 * Will be called when the given ability calls Ability->onStart
 *
 * @param Ability Indicates the ability object that calls the onStart() method.
 */
void OHOSApplication::OnAbilityStart(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityStart(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onInactive
 *
 * @param Ability Indicates the Ability object that calls the onInactive() method.
 */
void OHOSApplication::OnAbilityInactive(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityInactive(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onBackground
 *
 * @param Ability Indicates the Ability object that calls the onBackground() method.
 */
void OHOSApplication::OnAbilityBackground(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onForeground
 *
 * @param Ability Indicates the Ability object that calls the onForeground() method.
 */
void OHOSApplication::OnAbilityForeground(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onActive
 *
 * @param Ability Indicates the Ability object that calls the onActive() method.
 */
void OHOSApplication::OnAbilityActive(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityActive(ability);
        }
    }
}

/**
 *
 * Will be called when the given ability calls Ability->onStop
 *
 * @param Ability Indicates the Ability object that calls the onStop() method.
 */
void OHOSApplication::OnAbilityStop(const std::shared_ptr<AbilityRuntime::UIAbility> &ability)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    HILOG_DEBUG("called");
    for (auto callback : abilityLifecycleCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityStop(ability);
        }
    }
}

/**
 *
 * @brief Register ElementsCallback with OHOSApplication
 *
 * @param callBack callBack when the system configuration of the device changes.
 */
void OHOSApplication::RegisterElementsCallbacks(const std::shared_ptr<ElementsCallback> &callback)
{
    HILOG_DEBUG("called");

    if (callback == nullptr) {
        HILOG_DEBUG("observer is null");
        return;
    }

    elementsCallbacks_.emplace_back(callback);
}

/**
 *
 * @brief Unregister ElementsCallback with OHOSApplication
 *
 * @param callback RegisterElementsCallbacks`s callback
 */
void OHOSApplication::UnregisterElementsCallbacks(const std::shared_ptr<ElementsCallback> &callback)
{
    HILOG_DEBUG("called");

    if (callback == nullptr) {
        HILOG_DEBUG("observer is null");
        return;
    }

    elementsCallbacks_.remove(callback);
}

/**
 *
 * @brief Will be Called when the system configuration of the device changes.
 *
 * @param config Indicates the new Configuration object.
 */
void OHOSApplication::OnConfigurationUpdated(const Configuration &config)
{
    HILOG_DEBUG("called");
    if (!abilityRecordMgr_ || !configuration_) {
        HILOG_DEBUG("abilityRecordMgr_ or configuration_ is null");
        return;
    }
    std::string language = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    std::string colorMode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    std::string languageIsSetByApp =
        config.GetItem(AAFwk::GlobalConfigurationKey::LANGUAGE_IS_SET_BY_APP);
    std::string colorModeIsSetByApp =
        config.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
    std::string globalColorMode =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    std::string globalLanguageIsSetByApp =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::LANGUAGE_IS_SET_BY_APP);
    std::string globalColorModeIsSetByApp =
        configuration_->GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
    if (colorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) == 0) {
        HILOG_DEBUG("colorMode is auto");
        configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_AUTO);
        return;
    }
    if (!colorMode.empty() && colorModeIsSetByApp.empty()) {
        if (!globalColorModeIsSetByApp.empty() && globalColorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) != 0) {
            HILOG_DEBUG("colormode has been set by app");
            return;
        }
    }
    if (!language.empty() && languageIsSetByApp.empty() && !globalLanguageIsSetByApp.empty()) {
        HILOG_DEBUG("language has been set by app");
        return;
    }
    std::vector<std::string> changeKeyV;
    configuration_->CompareDifferent(changeKeyV, config);
    configuration_->Merge(changeKeyV, config);
    if (globalColorMode.compare(ConfigurationInner::COLOR_MODE_AUTO) == 0 && colorModeIsSetByApp.empty()) {
        configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_AUTO);
    }

    // Update resConfig of resource manager, which belongs to application context.
    UpdateAppContextResMgr(config);

    // Notify all abilities
    HILOG_INFO(
        "recordCount: [%{public}d]", static_cast<int>(abilityRecordMgr_->GetRecordCount()));
    for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
        auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
        if (abilityRecord && abilityRecord->GetAbilityThread()) {
            abilityRecord->GetAbilityThread()->ScheduleUpdateConfiguration(config);
        }
    }

    // Notify AbilityStage
    HILOG_INFO("abilityStages size: [%{public}zu]", abilityStages_.size());
    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
        auto abilityStage = it->second;
        if (abilityStage) {
            abilityStage->OnConfigurationUpdated(config);
        }
    }

#ifdef SUPPORT_GRAPHICS
    // Notify Window
    HILOG_DEBUG("Update configuration for all window.");
    auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(config);
    Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
#endif

    for (auto callback : elementsCallbacks_) {
        if (callback != nullptr) {
            callback->OnConfigurationUpdated(nullptr, config);
        }
    }

    abilityRuntimeContext_->DispatchConfigurationUpdated(config);
}

/**
 *
 * @brief Called when the system has determined to trim the memory, for example,
 * when the ability is running in the background and there is no enough memory for
 * running as many background processes as possible.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void OHOSApplication::OnMemoryLevel(int level)
{
    HILOG_DEBUG("called");

    if (abilityRecordMgr_) {
        HILOG_DEBUG("Number of ability to be notified : [%{public}d]", abilityRecordMgr_->GetRecordCount());
        for (const auto &abilityToken : abilityRecordMgr_->GetAllTokens()) {
            auto abilityRecord = abilityRecordMgr_->GetAbilityItem(abilityToken);
            if (abilityRecord && abilityRecord->GetAbilityThread()) {
                abilityRecord->GetAbilityThread()->NotifyMemoryLevel(level);
            }
        }
    }

    HILOG_DEBUG("Number of abilityStage to be notified : [%{public}zu]", abilityStages_.size());
    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
        auto abilityStage = it->second;
        if (abilityStage) {
            abilityStage->OnMemoryLevel(level);
        }
    }

    HILOG_DEBUG("called");
    for (auto callback : elementsCallbacks_) {
        if (callback != nullptr) {
            callback->OnMemoryLevel(level);
        }
    }

    abilityRuntimeContext_->DispatchMemoryLevel(level);
}

/**
 *
 * @brief Will be called the application starts
 *
 */
void OHOSApplication::OnStart()
{
    HILOG_DEBUG("called");
}

/**
 *
 * @brief Will be called the application ends
 */
void OHOSApplication::OnTerminate()
{
    HILOG_DEBUG("called");
}

/**
 *
 * @brief Called when an ability calls Ability#onSaveAbilityState(PacMap).
 * You can implement your own logic in this method.
 * @param outState IIndicates the {@link PacMap} object passed to the onSaveAbilityState() callback.
 *
 */
void OHOSApplication::OnAbilitySaveState(const PacMap &outState)
{
    DispatchAbilitySavedState(outState);
}

void OHOSApplication::SetAppEnv(const std::vector<AppEnvironment>& appEnvironments)
{
    if (!appEnvironments.size()) {
        HILOG_INFO("appEnvironments empty.");
        return;
    }

    for (const auto &appEnvironment : appEnvironments) {
        if (setenv(appEnvironment.name.c_str(), appEnvironment.value.c_str(), APP_ENVIRONMENT_OVERWRITE)) {
            HILOG_ERROR("appEnvironment: %{public}s set failed.", appEnvironment.name.c_str());
            return;
        }
        HILOG_INFO("appEnvironment set successfully: %{public}s = %{public}s.", appEnvironment.name.c_str(),
                   appEnvironment.value.c_str());
    }
    return;
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::AddAbilityStage(
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return nullptr;
    }
    const std::shared_ptr<AbilityInfo> &abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return nullptr;
    }
    std::string moduleName = abilityInfo->moduleName;
    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator == abilityStages_.end()) {
        std::shared_ptr<AbilityRuntime::ContextImpl> stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
        stageContext->SetParentContext(abilityRuntimeContext_);
        stageContext->InitHapModuleInfo(abilityInfo);
        stageContext->SetConfiguration(GetConfiguration());
        std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo = stageContext->GetHapModuleInfo();
        if (hapModuleInfo == nullptr) {
            HILOG_ERROR("hapModuleInfo is nullptr");
            return nullptr;
        }
        SetAppEnv(hapModuleInfo->appEnvironments);

        if (abilityInfo->applicationInfo.multiProjects) {
            auto moduleContext = stageContext->CreateModuleContext(hapModuleInfo->moduleName);
            auto rm = moduleContext != nullptr ? moduleContext->GetResourceManager() : nullptr;
            stageContext->SetResourceManager(rm);
        }

        abilityStage = AbilityRuntime::AbilityStage::Create(runtime_, *hapModuleInfo);
        abilityStage->Init(stageContext);

        if (hapModuleInfo->moduleType == ModuleType::ENTRY){
            abilityStage->AutoStartup();
        }

        Want want;
        if (abilityRecord->GetWant()) {
            HILOG_DEBUG("want is ok, transport to abilityStage");
            want = *(abilityRecord->GetWant());
        }
        abilityStage->OnCreate(want);
        abilityStages_[moduleName] = abilityStage;
    } else {
        abilityStage = iterator->second;
    }
    const sptr<IRemoteObject> &token = abilityRecord->GetToken();
    if (token == nullptr) {
        HILOG_ERROR("token is null");
        return nullptr;
    }
    abilityStage->AddAbility(token, abilityRecord);
    return abilityStage->GetContext();
}

/**
 *
 * @brief update the application info after new module installed.
 *
 * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
 *
 */
void OHOSApplication::UpdateApplicationInfoInstalled(const AppExecFwk::ApplicationInfo &appInfo)
{
    HILOG_DEBUG("called");

    if (abilityRuntimeContext_ == nullptr) {
        HILOG_ERROR("abilityRuntimeContext_ is nullptr.");
        return;
    }

    abilityRuntimeContext_->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(appInfo));
}

bool OHOSApplication::AddAbilityStage(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    HILOG_DEBUG("called");
    if (abilityRuntimeContext_ == nullptr) {
        HILOG_ERROR("abilityRuntimeContext_ is nullptr.");
        return false;
    }

    if (runtime_ == nullptr) {
        HILOG_ERROR("runtime_ is nullptr.");
        return false;
    }

    if (abilityStages_.find(hapModuleInfo.moduleName) != abilityStages_.end()) {
        HILOG_ERROR("object already exists ");
        return false;
    }

    auto stageContext = std::make_shared<AbilityRuntime::ContextImpl>();
    stageContext->SetParentContext(abilityRuntimeContext_);
    stageContext->InitHapModuleInfo(hapModuleInfo);
    stageContext->SetConfiguration(GetConfiguration());
    auto moduleInfo = stageContext->GetHapModuleInfo();
    if (moduleInfo == nullptr) {
        HILOG_ERROR("moduleInfo is nullptr");
        return false;
    }

    if (abilityRuntimeContext_->GetApplicationInfo() && abilityRuntimeContext_->GetApplicationInfo()->multiProjects) {
        auto rm = stageContext->CreateModuleContext(hapModuleInfo.moduleName)->GetResourceManager();
        stageContext->SetResourceManager(rm);
    }

    auto abilityStage = AbilityRuntime::AbilityStage::Create(runtime_, *moduleInfo);
    abilityStage->Init(stageContext);
    Want want;
    abilityStage->OnCreate(want);
    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
    HILOG_ERROR("abilityStage insert and initialization");
    return true;
}

void OHOSApplication::CleanAbilityStage(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }
    if (token == nullptr) {
        HILOG_ERROR("token is nullptr");
        return;
    }
    std::string moduleName = abilityInfo->moduleName;
    auto iterator = abilityStages_.find(moduleName);
    if (iterator != abilityStages_.end()) {
        auto abilityStage = iterator->second;
        abilityStage->RemoveAbility(token);
        if (!abilityStage->ContainsAbility()) {
            abilityStage->OnDestroy();
            abilityStages_.erase(moduleName);
        }
        DoCleanWorkAfterStageCleaned(*abilityInfo);
    }
}

void OHOSApplication::DoCleanWorkAfterStageCleaned(const AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("language: %{public}s.", abilityInfo.srcLanguage.c_str());
    if (runtime_) {
        runtime_->DoCleanWorkAfterStageCleaned();
    }
}

std::shared_ptr<AbilityRuntime::Context> OHOSApplication::GetAppContext() const
{
    return abilityRuntimeContext_;
}

const std::unique_ptr<AbilityRuntime::Runtime>& OHOSApplication::GetRuntime()
{
    return runtime_;
}

void OHOSApplication::SetConfiguration(const Configuration &config)
{
    if (!configuration_) {
        configuration_ = std::make_shared<Configuration>(config);
    }
}

void OHOSApplication::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName, std::string &flag)
{
    HILOG_DEBUG("called");
    auto iter = abilityStages_.find(moduleName);
    if (iter != abilityStages_.end()) {
        auto abilityStage = iter->second;
        if (abilityStage) {
            flag = abilityStage->OnAcceptWant(want);
        }
    }
}

void OHOSApplication::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName,
    std::string &flag)
{
    HILOG_DEBUG("call.");
    if (abilityStages_.empty()) {
        HILOG_ERROR("abilityStages_ is empty.");
        return;
    }
    auto iter = abilityStages_.find(moduleName);
    if (iter == abilityStages_.end()) {
        HILOG_ERROR("%{public}s is not in abilityStage", moduleName.c_str());
        return;
    }
    auto abilityStage = iter->second;
    if (abilityStage) {
        flag = abilityStage->OnNewProcessRequest(want);
    }
}

std::shared_ptr<Configuration> OHOSApplication::GetConfiguration()
{
    return configuration_;
}

void OHOSApplication::SetExtensionTypeMap(std::map<int32_t, std::string> map)
{
    extensionTypeMap_ = map;
}

bool OHOSApplication::NotifyLoadRepairPatch(const std::string &hqfFile, const std::string &hapPath)
{
    if (runtime_ == nullptr) {
        HILOG_DEBUG("runtime is nullptr.");
        return true;
    }

    return runtime_->LoadRepairPatch(hqfFile, hapPath);
}

bool OHOSApplication::NotifyHotReloadPage()
{
    if (runtime_ == nullptr) {
        HILOG_DEBUG("runtime is nullptr.");
        return true;
    }

    return runtime_->NotifyHotReloadPage();
}

bool OHOSApplication::NotifyUnLoadRepairPatch(const std::string &hqfFile)
{
    if (runtime_ == nullptr) {
        HILOG_DEBUG("runtime is nullptr.");
        return true;
    }

    return runtime_->UnLoadRepairPatch(hqfFile);
}

void OHOSApplication::CleanAppTempData(bool isLastProcess)
{
    if (!isLastProcess) {
        HILOG_ERROR("failed");
        return;
    }
    if (abilityRuntimeContext_ == nullptr) {
        HILOG_ERROR("Context is nullptr.");
        return;
    }

    auto cleaner = ApplicationCleaner::GetInstance();
    if (cleaner) {
        cleaner->SetRuntimeContext(abilityRuntimeContext_);
        cleaner->RenameTempData();
    }
}

void OHOSApplication::CleanUselessTempData()
{
    if (abilityRuntimeContext_ == nullptr) {
        HILOG_ERROR("Context is nullptr.");
        return;
    }

    auto cleaner = ApplicationCleaner::GetInstance();
    if (cleaner) {
        cleaner->SetRuntimeContext(abilityRuntimeContext_);
        cleaner->ClearTempData();
    }
}

void OHOSApplication::UpdateAppContextResMgr(const Configuration &config)
{
    auto context = GetAppContext();
    if (context == nullptr) {
        HILOG_ERROR("Application context is nullptr.");
        return;
    }

    auto configUtils = std::make_shared<AbilityRuntime::ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(config, context->GetResourceManager());
}
}  // namespace AppExecFwk
}  // namespace OHOS
