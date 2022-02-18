/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "main_thread.h"

#include <new>
#include <regex>

#include "ability_constants.h"
#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "ability_loader.h"
#include "ability_thread.h"
#include "app_loader.h"
#include "app_log_wrapper.h"
#include "application_env_impl.h"
#include "bytrace.h"
#include "context_deal.h"
#include "context_impl.h"
#include "extension_module_loader.h"
#include "form_extension.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "js_runtime.h"
#include "locale_config.h"
#include "ohos_application.h"
#include "resource_manager.h"
#include "runtime.h"
#include "service_extension.h"
#include "static_subscriber_extension.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "faultloggerd_client.h"
#include "dfx_dump_catcher.h"

#if defined(ABILITY_LIBRARY_LOADER) || defined(APPLICATION_LIBRARY_LOADER)
#include <dirent.h>
#include <dlfcn.h>
#endif

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityRuntime::Constants;
std::shared_ptr<OHOSApplication> MainThread::applicationForAnr_ = nullptr;
namespace {
constexpr int32_t DELIVERY_TIME = 200;
constexpr int32_t DISTRIBUTE_TIME = 100;
constexpr int32_t UNSPECIFIED_USERID = -2;
}

#define ACEABILITY_LIBRARY_LOADER
#ifdef ABILITY_LIBRARY_LOADER
#endif

/**
 *
 * @brief Notify the AppMgrDeathRecipient that the remote is dead.
 *
 * @param remote The remote whitch is dead.
 */
void AppMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    APP_LOGE("MainThread::AppMgrDeathRecipient remote died receive");
}

MainThread::MainThread()
{
#ifdef ABILITY_LIBRARY_LOADER
    fileEntries_.clear();
    handleAbilityLib_.clear();
#endif  // ABILITY_LIBRARY_LOADER
}

MainThread::~MainThread()
{
#ifdef ABILITY_LIBRARY_LOADER
    CloseAbilityLibrary();
#endif  // ABILITY_LIBRARY_LOADER
}

/**
 *
 * @brief Get the current MainThreadState.
 *
 * @return Returns the current MainThreadState.
 */
MainThreadState MainThread::GetMainThreadState() const
{
    return mainThreadState_;
}

/**
 *
 * @brief Set the runner state of mainthread.
 *
 * @param runnerStart whether the runner is started.
 */
void MainThread::SetRunnerStarted(bool runnerStart)
{
    isRunnerStarted_ = runnerStart;
}

/**
 *
 * @brief Get the runner state of mainthread.
 *
 * @return Returns the runner state of mainthread.
 */
bool MainThread::GetRunnerStarted() const
{
    return isRunnerStarted_;
}

/**
 *
 * @brief Get the newThreadId.
 *
 * @return Returns the newThreadId.
 */
int MainThread::GetNewThreadId()
{
    return newThreadId_++;
}

/**
 *
 * @brief Get the application.
 *
 * @return Returns the application.
 */
std::shared_ptr<OHOSApplication> MainThread::GetApplication() const
{
    return application_;
}

/**
 *
 * @brief Get the applicationInfo.
 *
 * @return Returns the applicationInfo.
 */
std::shared_ptr<ApplicationInfo> MainThread::GetApplicationInfo() const
{
    return applicationInfo_;
}

/**
 *
 * @brief Get the applicationImpl.
 *
 * @return Returns the applicationImpl.
 */
std::shared_ptr<ApplicationImpl> MainThread::GetApplicationImpl()
{
    return applicationImpl_;
}

/**
 *
 * @brief Connect the mainthread to the AppMgr.
 *
 */
bool MainThread::ConnectToAppMgr()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::ConnectToAppMgr start");
    auto object = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        APP_LOGE("failed to get app manager service");
        return false;
    }
    deathRecipient_ = new (std::nothrow) AppMgrDeathRecipient();
    if (deathRecipient_ == nullptr) {
        APP_LOGE("failed to new AppMgrDeathRecipient");
        return false;
    }

    APP_LOGI("%{public}s, Start calling AddDeathRecipient.", __func__);
    if (!object->AddDeathRecipient(deathRecipient_)) {
        APP_LOGE("failed to AddDeathRecipient");
        return false;
    }
    APP_LOGI("%{public}s, End calling AddDeathRecipient.", __func__);

    appMgr_ = iface_cast<IAppMgr>(object);
    if (appMgr_ == nullptr) {
        APP_LOGE("failed to iface_cast object to appMgr_");
        return false;
    }
    APP_LOGI("MainThread::connectToAppMgr before AttachApplication");
    appMgr_->AttachApplication(this);
    APP_LOGI("MainThread::connectToAppMgr after AttachApplication");
    return true;
}

/**
 *
 * @brief Attach the mainthread to the AppMgr.
 *
 */
void MainThread::Attach()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::attach called");
    if (!ConnectToAppMgr()) {
        APP_LOGE("attachApplication failed");
        return;
    }
    mainThreadState_ = MainThreadState::ATTACH;
    APP_LOGI("MainThread::attach mainThreadState: %{public}d", mainThreadState_);
}

/**
 *
 * @brief remove the deathRecipient from appMgr.
 *
 */
void MainThread::RemoveAppMgrDeathRecipient()
{
    APP_LOGI("MainThread::RemoveAppMgrDeathRecipient called begin");
    if (appMgr_ == nullptr) {
        APP_LOGE("MainThread::RemoveAppMgrDeathRecipient failed");
        return;
    }

    sptr<IRemoteObject> object = appMgr_->AsObject();
    if (object != nullptr) {
        APP_LOGI("%{public}s called. Start calling  RemoveDeathRecipient.", __func__);
        object->RemoveDeathRecipient(deathRecipient_);
        APP_LOGI("%{public}s called. End calling  RemoveDeathRecipient.", __func__);
    } else {
        APP_LOGE("appMgr_->AsObject() failed");
    }
    APP_LOGI("%{public}s called end.", __func__);
}

/**
 *
 * @brief Get the eventHandler of mainthread.
 *
 * @return Returns the eventHandler of mainthread.
 */
std::shared_ptr<EventHandler> MainThread::GetMainHandler() const
{
    return mainHandler_;
}

/**
 *
 * @brief Schedule the foreground lifecycle of application.
 *
 */
void MainThread::ScheduleForegroundApplication()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::scheduleForegroundApplication called begin");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleForegroundApplication failed.");
            return;
        }
        appThread->HandleForegroundApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleForegroundApplication called end.");
}

/**
 *
 * @brief Schedule the background lifecycle of application.
 *
 */
void MainThread::ScheduleBackgroundApplication()
{
    APP_LOGI("MainThread::scheduleBackgroundApplication called begin");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleBackgroundApplication failed.");
            return;
        }
        appThread->HandleBackgroundApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleBackgroundApplication PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleBackgroundApplication called end.");
}

/**
 *
 * @brief Schedule the terminate lifecycle of application.
 *
 */
void MainThread::ScheduleTerminateApplication()
{
    APP_LOGI("MainThread::scheduleTerminateApplication called begin");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleTerminateApplication failed.");
            return;
        }
        appThread->HandleTerminateApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleTerminateApplication PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleTerminateApplication called.");
}

/**
 *
 * @brief Shrink the memory whitch used by application.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void MainThread::ScheduleShrinkMemory(const int level)
{
    APP_LOGI("MainThread::scheduleShrinkMemory level: %{public}d", level);
    wptr<MainThread> weak = this;
    auto task = [weak, level]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleShrinkMemory(level);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleShrinkMemory PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleShrinkMemory level: %{public}d end.", level);
}

/**
 *
 * @brief Schedule the application process exit safely.
 *
 */
void MainThread::ScheduleProcessSecurityExit()
{
    APP_LOGI("MainThread::ScheduleProcessSecurityExit called start");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleProcessSecurityExit();
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleProcessSecurityExit PostTask task failed");
    }
    APP_LOGI("MainThread::ScheduleProcessSecurityExit called end");
}

/**
 *
 * @brief Low the memory whitch used by application.
 *
 */
void MainThread::ScheduleLowMemory()
{
    APP_LOGI("MainThread::scheduleLowMemory called");
}

/**
 *
 * @brief Launch the application.
 *
 * @param data The launchdata of the application witch launced.
 *
 */
void MainThread::ScheduleLaunchApplication(const AppLaunchData &data, const Configuration &config)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::scheduleLaunchApplication start");
    wptr<MainThread> weak = this;
    auto task = [weak, data, config]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleLaunchApplication failed.");
            return;
        }
        appThread->HandleLaunchApplication(data, config);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleLaunchApplication PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleLaunchApplication end.");
}

void MainThread::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    APP_LOGI("MainThread::ScheduleAbilityStageInfo start");
    wptr<MainThread> weak = this;
    auto task = [weak, abilityStage]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleAbilityStage(abilityStage);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleAbilityStageInfo PostTask task failed");
    }
    APP_LOGI("MainThread::ScheduleAbilityStageInfo end.");
}

/**
 *
 * @brief launch the application.
 *
 * @param info The launchdata of the application witch launced.
 * @param token The launchdata of the application witch launced.
 *
 */
void MainThread::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token)
{
    APP_LOGI("MainThread::scheduleLaunchAbility called start.");
    APP_LOGI(
        "MainThread::scheduleLaunchAbility AbilityInfo name:%{public}s type:%{public}d", info.name.c_str(), info.type);

    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>(info);
    if (abilityInfo == nullptr) {
        APP_LOGE("MainThread::ScheduleLaunchAbility abilityInfo is nullptr");
        return;
    }
    sptr<IRemoteObject> abilityToken = token;
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, abilityToken);

    std::shared_ptr<ContextDeal> contextDeal = std::make_shared<ContextDeal>();
    sptr<IBundleMgr> bundleMgr = contextDeal->GetBundleManager();
    if (bundleMgr == nullptr) {
        APP_LOGE("MainThread::ScheduleLaunchAbility GetBundleManager is nullptr");
    } else {
        BundleInfo bundleInfo;
        bundleMgr->GetBundleInfo(abilityInfo->bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo);
        abilityRecord->SetCompatibleVersion(bundleInfo.compatibleVersion);
        APP_LOGI("MainThread::ScheduleLaunchAbility compatibleVersion:%{public}d", bundleInfo.compatibleVersion);
    }

    wptr<MainThread> weak = this;
    auto task = [weak, abilityRecord]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleLaunchAbility failed.");
            return;
        }
        appThread->HandleLaunchAbility(abilityRecord);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleLaunchAbility PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleLaunchAbility called end.");
}

/**
 *
 * @brief clean the ability by token.
 *
 * @param token The token belong to the ability whitch want to be cleaned.
 *
 */
void MainThread::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    APP_LOGI("MainThread::scheduleCleanAbility called start.");
    wptr<MainThread> weak = this;
    auto task = [weak, token]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleCleanAbility failed.");
            return;
        }
        appThread->HandleCleanAbility(token);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleCleanAbility PostTask task failed");
    }
    APP_LOGI("MainThread::scheduleCleanAbility called end.");
}

/**
 *
 * @brief send the new profile.
 *
 * @param profile The updated profile.
 *
 */
void MainThread::ScheduleProfileChanged(const Profile &profile)
{
    APP_LOGI("MainThread::scheduleProfileChanged profile name: %{public}s", profile.GetName().c_str());
}

/**
 *
 * @brief send the new config to the application.
 *
 * @param config The updated config.
 *
 */
void MainThread::ScheduleConfigurationUpdated(const Configuration &config)
{
    APP_LOGI("MainThread::ScheduleConfigurationUpdated called start.");
    wptr<MainThread> weak = this;
    auto task = [weak, config]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("appThread is nullptr, HandleConfigurationUpdated failed.");
            return;
        }
        appThread->HandleConfigurationUpdated(config);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleConfigurationUpdated PostTask task failed");
    }
    APP_LOGI("MainThread::ScheduleConfigurationUpdated called end.");
}

/**
 *
 * @brief Check whether the appLaunchData is legal.
 *
 * @param appLaunchData The appLaunchData should be checked.
 *
 * @return if the appLaunchData is legal, return true. else return false.
 */
bool MainThread::CheckLaunchApplicationParam(const AppLaunchData &appLaunchData) const
{
    APP_LOGI("MainThread::CheckLaunchApplicationParam called start.");

    ApplicationInfo appInfo = appLaunchData.GetApplicationInfo();
    ProcessInfo processInfo = appLaunchData.GetProcessInfo();

    if (appInfo.name.empty()) {
        APP_LOGE("MainThread::checkLaunchApplicationParam applicationName is empty");
        return false;
    }

    if (processInfo.GetProcessName().empty()) {
        APP_LOGE("MainThread::checkLaunchApplicationParam processName is empty");
        return false;
    }

    APP_LOGI("MainThread::CheckLaunchApplicationParam called end.");
    return true;
}

/**
 *
 * @brief Check whether the record is legal.
 *
 * @param record The record should be checked.
 *
 * @return if the record is legal, return true. else return false.
 */
bool MainThread::CheckAbilityItem(const std::shared_ptr<AbilityLocalRecord> &record) const
{
    APP_LOGI("MainThread::CheckAbilityItem called start.");
    if (record == nullptr) {
        APP_LOGE("MainThread::checkAbilityItem record is null");
        return false;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    sptr<IRemoteObject> token = record->GetToken();

    if (abilityInfo == nullptr) {
        APP_LOGE("MainThread::checkAbilityItem abilityInfo is null");
        return false;
    }

    if (token == nullptr) {
        APP_LOGE("MainThread::checkAbilityItem token is null");
        return false;
    }

    APP_LOGI("MainThread::CheckAbilityItem called end.");
    return true;
}

/**
 *
 * @brief Terminate the application but don't notify ams.
 *
 */
void MainThread::HandleTerminateApplicationLocal()
{
    APP_LOGI("MainThread::HandleTerminateApplicationLocal called start.");
    if (application_ == nullptr) {
        APP_LOGE("MainThread::HandleTerminateApplicationLocal error!");
        return;
    }
    APP_LOGI("MainThread::HandleTerminateApplicationLocal before PerformTerminateStrong.");
    applicationImpl_->PerformTerminateStrong();
    APP_LOGI("MainThread::HandleTerminateApplicationLocal after PerformTerminateStrong.");
    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        APP_LOGE("MainThread::HandleTerminateApplicationLocal get manHandler error");
        return;
    }

    if (watchDogHandler_ != nullptr) {
        watchDogHandler_->Stop();
    }
    int ret = runner->Stop();
    if (ret != ERR_OK) {
        APP_LOGE("MainThread::HandleTerminateApplicationLocal failed. runner->Run failed ret = %{public}d", ret);
    }
    APP_LOGI("runner is stopped");
    SetRunnerStarted(false);

#ifdef ABILITY_LIBRARY_LOADER
    APP_LOGI("MainThread::HandleTerminateApplicationLocal called. Start calling CloseAbilityLibrary.");
    CloseAbilityLibrary();
    APP_LOGI("MainThread::HandleTerminateApplicationLocal called. End calling CloseAbilityLibrary.");
#endif  // ABILITY_LIBRARY_LOADER
#ifdef APPLICATION_LIBRARY_LOADER
    if (handleAppLib_ != nullptr) {
        APP_LOGI("MainThread::HandleTerminateApplicationLocal called. Start calling dlclose.");
        dlclose(handleAppLib_);
        APP_LOGI("MainThread::HandleTerminateApplicationLocal called. End calling dlclose.");
        handleAppLib_ = nullptr;
    }
#endif  // APPLICATION_LIBRARY_LOADER
    APP_LOGI("MainThread::HandleTerminateApplicationLocal called end.");
}

/**
 *
 * @brief Schedule the application process exit safely.
 *
 */
void MainThread::HandleProcessSecurityExit()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::HandleProcessSecurityExit called start.");
    if (abilityRecordMgr_ == nullptr) {
        APP_LOGE("MainThread::HandleProcessSecurityExit abilityRecordMgr_ is null");
        return;
    }

    std::vector<sptr<IRemoteObject>> tokens = (abilityRecordMgr_->GetAllTokens());

    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
        HandleCleanAbilityLocal(*iter);
    }

    HandleTerminateApplicationLocal();
    APP_LOGI("MainThread::HandleProcessSecurityExit called end.");
}

bool MainThread::InitCreate(
    std::shared_ptr<ContextDeal> &contextDeal, ApplicationInfo &appInfo, ProcessInfo &processInfo, Profile &appProfile)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    applicationInfo_ = std::make_shared<ApplicationInfo>(appInfo);
    if (applicationInfo_ == nullptr) {
        APP_LOGE("MainThread::InitCreate create applicationInfo_ failed");
        return false;
    }

    processInfo_ = std::make_shared<ProcessInfo>(processInfo);
    if (processInfo_ == nullptr) {
        APP_LOGE("MainThread::InitCreate create processInfo_ failed");
        return false;
    }

    appProfile_ = std::make_shared<Profile>(appProfile);
    if (appProfile_ == nullptr) {
        APP_LOGE("MainThread::InitCreate create appProfile_ failed");
        return false;
    }

    applicationImpl_ = std::make_shared<ApplicationImpl>();
    if (applicationImpl_ == nullptr) {
        APP_LOGE("MainThread::InitCreate create applicationImpl_ failed");
        return false;
    }

    abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    if (abilityRecordMgr_ == nullptr) {
        APP_LOGE("MainThread::InitCreate create AbilityRecordMgr failed");
        return false;
    }

    contextDeal = std::make_shared<ContextDeal>();
    if (contextDeal == nullptr) {
        APP_LOGE("MainThread::InitCreate create contextDeal failed");
        return false;
    }

    contextDeal->SetProcessInfo(processInfo_);
    contextDeal->SetApplicationInfo(applicationInfo_);
    contextDeal->SetProfile(appProfile_);
    contextDeal->SetBundleCodePath(applicationInfo_->codePath);  // BMS need to add cpath

    return true;
}

bool MainThread::CheckForHandleLaunchApplication(const AppLaunchData &appLaunchData)
{
    if (application_ != nullptr) {
        APP_LOGE("MainThread::handleLaunchApplication already create application");
        return false;
    }

    if (!CheckLaunchApplicationParam(appLaunchData)) {
        APP_LOGE("MainThread::handleLaunchApplication appLaunchData invalid");
        return false;
    }
    return true;
}

bool MainThread::InitResourceManager(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    std::shared_ptr<ContextDeal> &contextDeal, ApplicationInfo &appInfo, BundleInfo& bundleInfo)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleLaunchApplication moduleResPaths count: %{public}zu start",
        bundleInfo.moduleResPaths.size());
    std::vector<std::string> resPaths;
    ChangeToLocalPath(bundleInfo.name, bundleInfo.moduleResPaths, resPaths);
    for (auto moduleResPath : resPaths) {
        if (!moduleResPath.empty()) {
            APP_LOGI("MainThread::handleLaunchApplication length: %{public}zu, moduleResPath: %{public}s",
                moduleResPath.length(),
                moduleResPath.c_str());
            APP_LOGI("MainThread::handleLaunchApplication. before resourceManager->AddResource.");
            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                APP_LOGE("MainThread::handleLaunchApplication AddResource failed");
            }
            APP_LOGI("MainThread::handleLaunchApplication. after resourceManager->AddResource.");
        }
    }
    APP_LOGI("MainThread::handleLaunchApplication moduleResPaths end.");
    APP_LOGI("MainThread::handleLaunchApplication before Resource::CreateResConfig.");
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    APP_LOGI("MainThread::handleLaunchApplication after Resource::CreateResConfig.");
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    const icu::Locale *localeInfo = resConfig->GetLocaleInfo();
    if (localeInfo != nullptr) {
        APP_LOGI("MainThread::handleLaunchApplication language: %{public}s, script: %{public}s, region: %{public}s,",
            localeInfo->getLanguage(),
            localeInfo->getScript(),
            localeInfo->getCountry());
    } else {
        APP_LOGI("MainThread::handleLaunchApplication localeInfo is nullptr.");
    }

    APP_LOGI("MainThread::handleLaunchApplication. Start calling UpdateResConfig.");
    resourceManager->UpdateResConfig(*resConfig);
    APP_LOGI("MainThread::handleLaunchApplication. End calling UpdateResConfig.");
    return true;
}
/**
 *
 * @brief Launch the application.
 *
 * @param appLaunchData The launchdata of the application witch launced.
 *
 */
void MainThread::HandleLaunchApplication(const AppLaunchData &appLaunchData, const Configuration &config)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleLaunchApplication called start.");
    if (!CheckForHandleLaunchApplication(appLaunchData)) {
        APP_LOGE("MainThread::handleLaunchApplication CheckForHandleLaunchApplication failed");
        return;
    }
    std::vector<std::string> localPaths;
    ChangeToLocalPath(appLaunchData.GetApplicationInfo().bundleName,
        appLaunchData.GetApplicationInfo().moduleSourceDirs, localPaths);
    LoadAbilityLibrary(localPaths);
    LoadAppLibrary();

    ApplicationInfo appInfo = appLaunchData.GetApplicationInfo();
    ProcessInfo processInfo = appLaunchData.GetProcessInfo();
    Profile appProfile = appLaunchData.GetProfile();

    std::shared_ptr<ContextDeal> contextDeal = nullptr;
    if (!InitCreate(contextDeal, appInfo, processInfo, appProfile)) {
        APP_LOGE("MainThread::handleLaunchApplication InitCreate failed");
        return;
    }

    // get application shared point
    application_ = std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName());
    if (application_ == nullptr) {
        APP_LOGE("HandleLaunchApplication::application launch failed");
        return;
    }
    applicationForAnr_ = application_;

    // init resourceManager.
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        APP_LOGE("MainThread::handleLaunchApplication create resourceManager failed");
        return;
    }

    APP_LOGI("MainThread::handleLaunchApplication. Start calling GetBundleManager.");
    sptr<IBundleMgr> bundleMgr = contextDeal->GetBundleManager();
    if (bundleMgr == nullptr) {
        APP_LOGE("MainThread::handleLaunchApplication GetBundleManager is nullptr");
        return;
    }
    APP_LOGI("MainThread::handleLaunchApplication. End calling GetBundleManager.");

    BundleInfo bundleInfo;
    APP_LOGI("MainThread::handleLaunchApplication length: %{public}zu, bundleName: %{public}s",
        appInfo.bundleName.length(), appInfo.bundleName.c_str());
    if (!bundleMgr->GetBundleInfo(appInfo.bundleName, BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, UNSPECIFIED_USERID)) {
        APP_LOGD("MainThread::handleLaunchApplication GetBundleInfo fail.");
    }

    if (!InitResourceManager(resourceManager, contextDeal, appInfo, bundleInfo)) {
        APP_LOGE("MainThread::handleLaunchApplication InitResourceManager failed");
        return;
    }

    bool moduelJson = false;
    bool isStageBased = false;
    if (!bundleInfo.hapModuleInfos.empty()) {
        moduelJson = bundleInfo.hapModuleInfos.back().isModuleJson;
        isStageBased = bundleInfo.hapModuleInfos.back().isStageBasedModel;
    }
    APP_LOGI("stageBased:%{public}d moduleJson:%{public}d size:%{public}d",
        isStageBased, moduelJson, (int32_t)bundleInfo.hapModuleInfos.size());
    if (isStageBased) {
        // Create runtime
        AbilityRuntime::Runtime::Options options;
        options.codePath = LOCAL_CODE_PATH;
        options.eventRunner = mainHandler_->GetEventRunner();
        auto runtime = AbilityRuntime::Runtime::Create(options);
        if (!runtime) {
            APP_LOGE("OHOSApplication::OHOSApplication: Failed to create runtime");
            return;
        }

        application_->SetRuntime(std::move(runtime));
        auto usertestInfo = appLaunchData.GetUserTestInfo();
        if (usertestInfo.observer) {
            if (!AbilityDelegatorPrepare(usertestInfo)) {
                return;
            }
        }
        AbilityLoader::GetInstance().RegisterAbility("Ability", [application = application_]() {
            return Ability::Create(application->GetRuntime());
        });
        AbilityLoader::GetInstance().RegisterExtension("FormExtension", [application = application_]() {
            return AbilityRuntime::FormExtension::Create(application->GetRuntime());
        });
        AbilityLoader::GetInstance().RegisterExtension("StaticSubscriberExtension", [application = application_]() {
            return AbilityRuntime::StaticSubscriberExtension::Create(application->GetRuntime());
        });
        LoadAndRegisterExtension("system/lib/libservice_extension_module.z.so", "ServiceExtension",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib/libdatashare_ext_ability_module.z.so", "DataShareExtAbility",
            application_->GetRuntime());
        LoadAndRegisterExtension("system/lib/libworkschedextension.z.so", "WorkSchedulerExtension",
            application_->GetRuntime());
    }

    contextDeal->initResourceManager(resourceManager);
    contextDeal->SetApplicationContext(application_);
    application_->AttachBaseContext(contextDeal);
    application_->SetAbilityRecordMgr(abilityRecordMgr_);
    application_->SetConfiguration(config);

    // create contextImpl
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetResourceManager(resourceManager);
    contextImpl->SetApplicationInfo(std::make_shared<ApplicationInfo>(appInfo));
    contextImpl->InitAppContext();
    application_->SetApplicationContext(contextImpl);

    applicationImpl_->SetRecordId(appLaunchData.GetRecordId());
    applicationImpl_->SetApplication(application_);
    mainThreadState_ = MainThreadState::READY;
    APP_LOGI("MainThread::handleLaunchApplication before PerformAppReady.");
    if (!applicationImpl_->PerformAppReady()) {
        APP_LOGE("HandleLaunchApplication::application applicationImpl_->PerformAppReady failed");
        return;
    }
    APP_LOGI("MainThread::handleLaunchApplication after PerformAppReady.");
    // L1 needs to add corresponding interface
    ApplicationEnvImpl *pAppEvnIml = ApplicationEnvImpl::GetInstance();

    if (pAppEvnIml) {
        pAppEvnIml->SetAppInfo(*applicationInfo_.get());
    } else {
        APP_LOGE("HandleLaunchApplication::application pAppEvnIml is null");
    }

    APP_LOGI("MainThread::handleLaunchApplication called end.");
}

void MainThread::ChangeToLocalPath(const std::string &bundleName,
    const std::vector<std::string> &sourceDirs, std::vector<std::string> &localPath)
{
    for (auto item : sourceDirs) {
        if (item.empty()) {
            continue;
        }
        std::regex pattern(ABS_CODE_PATH + FILE_SEPARATOR + bundleName + FILE_SEPARATOR);
        localPath.emplace_back(
            std::regex_replace(item, pattern, LOCAL_CODE_PATH + FILE_SEPARATOR));
    }
}

void MainThread::HandleAbilityStage(const HapModuleInfo &abilityStage)
{
    APP_LOGI("MainThread::HandleAbilityStageInfo");
    if (!application_) {
        APP_LOGE("application_ is nullptr");
        return;
    }

    application_->AddAbilityStage(abilityStage);

    if (!appMgr_ || !applicationImpl_) {
        APP_LOGE("appMgr_ is nullptr");
        return;
    }

    if (!appMgr_ || !applicationImpl_) {
        APP_LOGE("appMgr_ is nullptr");
        return;
    }
    appMgr_->AddAbilityStageDone(applicationImpl_->GetRecordId());
}

void MainThread::LoadAndRegisterExtension(const std::string &libName,
    const std::string &extensionName, const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    APP_LOGI("MainThread::LoadAndRegisterExtension.libName:%{public}s,extensionName:%{public}s,",
        libName.c_str(), extensionName.c_str());
    if (application_ == nullptr) {
        APP_LOGE("LoadAndRegisterExtension::application launch failed");
        return;
    }
    APP_LOGI("MainThread::LoadAndRegisterExtension load success.");
    AbilityLoader::GetInstance().RegisterExtension(extensionName, [application = application_, libName]() {
        return AbilityRuntime::ExtensionModuleLoader::GetLoader(libName.c_str()).Create(application->GetRuntime());
    });
}

bool MainThread::AbilityDelegatorPrepare(const UserTestRecord &record)
{
    APP_LOGI("MainThread::AbilityDelegatorPrepare");
    auto args = std::make_shared<AbilityDelegatorArgs>(record.want);
    if (!args) {
        APP_LOGE("args is null");
        return false;
    }
    auto testRunner = TestRunner::Create(application_->GetRuntime(), args);
    if (!testRunner) {
        APP_LOGE("testRunner is null");
        return false;
    }

    auto delegator = std::make_shared<AbilityDelegator>(this, std::move(testRunner), record.observer);
    if (!delegator) {
        APP_LOGE("delegator is null");
        return false;
    }
    delegator->Init();
    AbilityDelegatorRegistry::RegisterInstance(delegator, args);

    delegator->Prepare();
    return true;
}

int MainThread::FinishUserTest(const std::string &msg, const int &resultCode,
    const std::string &bundleName, const sptr<IRemoteObject> &observer)
{
    APP_LOGI("MainThread::FinishUserTest");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->FinishUserTest(
        msg, resultCode, bundleName, observer);
    if (err != ERR_OK) {
        APP_LOGE("MainThread::FinishUserTest is failed %{public}d", err);
    }
    return ERR_OK;
}

/**
 *
 * @brief launch the ability.
 *
 * @param abilityRecord The abilityRecord whitch belongs to the ability launched.
 *
 */
void MainThread::HandleLaunchAbility(const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleLaunchAbility called start.");

    if (applicationImpl_ == nullptr) {
        APP_LOGE("MainThread::HandleLaunchAbility applicationImpl_ is null");
        return;
    }

    if (abilityRecordMgr_ == nullptr) {
        APP_LOGE("MainThread::HandleLaunchAbility abilityRecordMgr_ is null");
        return;
    }

    if (abilityRecord == nullptr) {
        APP_LOGE("MainThread::HandleLaunchAbility parameter(abilityRecord) is null");
        return;
    }

    auto abilityToken = abilityRecord->GetToken();
    if (abilityToken == nullptr) {
        APP_LOGE("MainThread::HandleLaunchAbility failed. abilityRecord->GetToken failed");
        return;
    }

    abilityRecordMgr_->SetToken(abilityToken);
    abilityRecordMgr_->AddAbilityRecord(abilityToken, abilityRecord);

    if (!IsApplicationReady()) {
        APP_LOGE("MainThread::handleLaunchAbility not init OHOSApplication, should launch application first");
        return;
    }

    if (!CheckAbilityItem(abilityRecord)) {
        APP_LOGE("MainThread::handleLaunchAbility record is invalid");
        return;
    }

    mainThreadState_ = MainThreadState::RUNNING;
    std::shared_ptr<AbilityRuntime::Context> stageContext = application_->AddAbilityStage(abilityRecord);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    APP_LOGI("MainThread::handleLaunchAbility. Start calling AbilityThreadMain start.");
    AbilityThread::AbilityThreadMain(application_, abilityRecord, stageContext);
    APP_LOGI("MainThread::handleLaunchAbility. Start calling AbilityThreadMain end.");
#else
    APP_LOGI("MainThread::handleLaunchAbility. Start calling 2 AbilityThreadMain start.");
    AbilityThread::AbilityThreadMain(application_, abilityRecord, mainHandler_->GetEventRunner(), stageContext);
    APP_LOGI("MainThread::handleLaunchAbility. Start calling 2 AbilityThreadMain end.");
#endif
    APP_LOGI("MainThread::handleLaunchAbility called end.");
}

/**
 *
 * @brief Clean the ability but don't notify ams.
 *
 * @param token The token whitch belongs to the ability launched.
 *
 */
void MainThread::HandleCleanAbilityLocal(const sptr<IRemoteObject> &token)
{
    APP_LOGI("MainThread::HandleCleanAbilityLocal called start.");
    if (!IsApplicationReady()) {
        APP_LOGE("MainThread::HandleCleanAbilityLocal not init OHOSApplication, should launch application first");
        return;
    }

    if (token == nullptr) {
        APP_LOGE("MainThread::HandleCleanAbilityLocal token is null");
        return;
    }

    std::shared_ptr<AbilityLocalRecord> record = abilityRecordMgr_->GetAbilityItem(token);
    if (record == nullptr) {
        APP_LOGE("MainThread::HandleCleanAbilityLocal abilityRecord not found");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        APP_LOGE("MainThread::HandleCleanAbilityLocal record->GetAbilityInfo() failed");
        return;
    }
    APP_LOGI("MainThread::HandleCleanAbilityLocal ability name: %{public}s", abilityInfo->name.c_str());

    abilityRecordMgr_->RemoveAbilityRecord(token);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    std::shared_ptr<EventRunner> runner = record->GetEventRunner();
    if (runner != nullptr) {
        int ret = runner->Stop();
        if (ret != ERR_OK) {
            APP_LOGE("MainThread::main failed. ability runner->Run failed ret = %{public}d", ret);
        }
        abilityRecordMgr_->RemoveAbilityRecord(token);
    } else {
        APP_LOGW("MainThread::HandleCleanAbilityLocal runner not found");
    }
#endif
    APP_LOGI("MainThread::HandleCleanAbilityLocal called end.");
}

/**
 *
 * @brief Clean the ability.
 *
 * @param token The token whitch belongs to the ability launched.
 *
 */
void MainThread::HandleCleanAbility(const sptr<IRemoteObject> &token)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleCleanAbility called start.");
    if (!IsApplicationReady()) {
        APP_LOGE("MainThread::handleCleanAbility not init OHOSApplication, should launch application first");
        return;
    }

    if (token == nullptr) {
        APP_LOGE("MainThread::handleCleanAbility token is null");
        return;
    }

    std::shared_ptr<AbilityLocalRecord> record = abilityRecordMgr_->GetAbilityItem(token);
    if (record == nullptr) {
        APP_LOGE("MainThread::handleCleanAbility abilityRecord not found");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        APP_LOGE("MainThread::handleCleanAbility record->GetAbilityInfo() failed");
        return;
    }
    APP_LOGI("MainThread::handleCleanAbility ability name: %{public}s", abilityInfo->name.c_str());

    abilityRecordMgr_->RemoveAbilityRecord(token);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    std::shared_ptr<EventRunner> runner = record->GetEventRunner();
    if (runner != nullptr) {
        int ret = runner->Stop();
        if (ret != ERR_OK) {
            APP_LOGE("MainThread::main failed. ability runner->Run failed ret = %{public}d", ret);
        }
        abilityRecordMgr_->RemoveAbilityRecord(token);
    } else {
        APP_LOGW("MainThread::handleCleanAbility runner not found");
    }
#endif
    APP_LOGI("MainThread::handleCleanAbility before AbilityCleaned.");
    appMgr_->AbilityCleaned(token);
    APP_LOGI("MainThread::handleCleanAbility after AbilityCleaned.");
    APP_LOGI("MainThread::handleCleanAbility called end.");
}

/**
 *
 * @brief Foreground the application.
 *
 */
void MainThread::HandleForegroundApplication()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleForegroundApplication called start.");
    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        APP_LOGE("MainThread::handleForegroundApplication error!");
        return;
    }

    if (!applicationImpl_->PerformForeground()) {
        APP_LOGE("MainThread::handleForegroundApplication error!, applicationImpl_->PerformForeground() failed");
        return;
    }

    APP_LOGI("MainThread::handleForegroundApplication before ApplicationForegrounded");
    appMgr_->ApplicationForegrounded(applicationImpl_->GetRecordId());
    APP_LOGI("MainThread::handleForegroundApplication after ApplicationForegrounded");
    APP_LOGI("MainThread::handleForegroundApplication called end");
}

/**
 *
 * @brief Background the application.
 *
 */
void MainThread::HandleBackgroundApplication()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleBackgroundApplication called start.");

    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        APP_LOGE("MainThread::handleBackgroundApplication error!");
        return;
    }

    if (!applicationImpl_->PerformBackground()) {
        APP_LOGE("MainThread::handleForegroundApplication error!, applicationImpl_->PerformBackground() failed");
        return;
    }
    APP_LOGI("MainThread::handleBackgroundApplication before ApplicationBackgrounded");
    appMgr_->ApplicationBackgrounded(applicationImpl_->GetRecordId());
    APP_LOGI("MainThread::handleBackgroundApplication after ApplicationBackgrounded");

    APP_LOGI("MainThread::handleBackgroundApplication called end");
}

/**
 *
 * @brief Terminate the application.
 *
 */
void MainThread::HandleTerminateApplication()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::handleTerminateApplication called start.");
    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        APP_LOGE("MainThread::handleTerminateApplication error!");
        return;
    }

    APP_LOGI("MainThread::handleTerminateApplication before PerformTerminate");
    if (!applicationImpl_->PerformTerminate()) {
        APP_LOGE("MainThread::handleForegroundApplication error!, applicationImpl_->PerformTerminate() failed");
        return;
    }
    APP_LOGI("MainThread::handleTerminateApplication after PerformTerminate");

    APP_LOGI("MainThread::handleTerminateApplication before ApplicationTerminated");
    appMgr_->ApplicationTerminated(applicationImpl_->GetRecordId());
    APP_LOGI("MainThread::handleTerminateApplication after ApplicationTerminated");
    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        APP_LOGE("MainThread::handleTerminateApplication get manHandler error");
        return;
    }

    if (watchDogHandler_ != nullptr) {
        watchDogHandler_->Stop();
    }
    APP_LOGI("MainThread::handleTerminateApplication before stop runner");
    int ret = runner->Stop();
    APP_LOGI("MainThread::handleTerminateApplication after stop runner");
    if (ret != ERR_OK) {
        APP_LOGE("MainThread::handleTerminateApplication failed. runner->Run failed ret = %{public}d", ret);
    }
    SetRunnerStarted(false);

#ifdef ABILITY_LIBRARY_LOADER
    APP_LOGI("MainThread::handleTerminateApplication. Start callint CloseAbilityLibrary.");
    CloseAbilityLibrary();
    APP_LOGI("MainThread::handleTerminateApplication. End callint CloseAbilityLibrary.");
#endif  // ABILITY_LIBRARY_LOADER
#ifdef APPLICATION_LIBRARY_LOADER
    if (handleAppLib_ != nullptr) {
        dlclose(handleAppLib_);
        handleAppLib_ = nullptr;
    }
#endif  // APPLICATION_LIBRARY_LOADER
    APP_LOGI("MainThread::handleTerminateApplication called end.");
}

/**
 *
 * @brief Shrink the memory whitch used by application.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void MainThread::HandleShrinkMemory(const int level)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::HandleShrinkMemory called start.");

    if (applicationImpl_ == nullptr) {
        APP_LOGE("MainThread::HandleShrinkMemory error! applicationImpl_ is null");
        return;
    }

    applicationImpl_->PerformMemoryLevel(level);
    APP_LOGI("MainThread::HandleShrinkMemory called end.");
}

/**
 *
 * @brief send the new config to the application.
 *
 * @param config The updated config.
 *
 */
void MainThread::HandleConfigurationUpdated(const Configuration &config)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::HandleConfigurationUpdated called start.");

    if (applicationImpl_ == nullptr) {
        APP_LOGE("MainThread::HandleConfigurationUpdated error! applicationImpl_ is null");
        return;
    }

    applicationImpl_->PerformConfigurationUpdated(config);
    APP_LOGI("MainThread::HandleConfigurationUpdated called end.");
}

void MainThread::TaskTimeoutDetected(const std::shared_ptr<EventRunner> &runner)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::TaskTimeoutDetected called start.");

    auto deliveryTimeoutCallback = []() {
        APP_LOGI("MainThread::TaskTimeoutDetected delivery timeout");
    };
    auto distributeTimeoutCallback = []() {
        APP_LOGI("MainThread::TaskTimeoutDetected distribute timeout");
    };

    if (runner !=nullptr && mainHandler_ != nullptr) {
        runner->SetDeliveryTimeout(DELIVERY_TIME);
        mainHandler_->SetDeliveryTimeoutCallback(deliveryTimeoutCallback);

        runner->SetDistributeTimeout(DISTRIBUTE_TIME);
        mainHandler_->SetDistributeTimeoutCallback(distributeTimeoutCallback);
    }
    APP_LOGI("MainThread::TaskTimeoutDetected called end.");
}

void MainThread::Init(const std::shared_ptr<EventRunner> &runner, const std::shared_ptr<EventRunner> &watchDogRunner)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread:Init Start");
    mainHandler_ = std::make_shared<MainHandler>(runner, this);
    watchDogHandler_ = std::make_shared<WatchDog>(watchDogRunner);
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("abilityThread is nullptr, SetRunnerStarted failed.");
            return;
        }
        APP_LOGI("MainThread:MainHandler Start");
        appThread->SetRunnerStarted(true);
    };
    auto taskWatchDog = []() {
        APP_LOGI("MainThread:WatchDogHandler Start");
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::Init PostTask task failed");
    }
    if (!watchDogHandler_->PostTask(taskWatchDog)) {
        APP_LOGE("MainThread::Init WatchDog postTask task failed");
    }
    TaskTimeoutDetected(runner);
    // watchDogHandler_->Init(mainHandler_, watchDogHandler_)
    // APP_LOGI("MainThread:Init before CreateRunner.")
    TaskHandlerClient::GetInstance()->CreateRunner();
    APP_LOGI("MainThread:Init after CreateRunner.");
    APP_LOGI("MainThread:Init end.");
}

void MainThread::HandleANRProcess(int sigMessage)
{
    if (sigMessage == SIGUSR1) {
        int rFD = -1;
        std::string mainThreadStackInfo;
        if ((rFD = RequestFileDescriptor(int32_t(FaultLoggerType::CPP_CRASH))) < 0) {
            APP_LOGE("MainThread::HandleANRProcess request file eescriptor failed");
            return;
        }
        auto jsRuntime = std::move((std::unique_ptr<AbilityRuntime::JsRuntime>&)applicationForAnr_->GetRuntime());
        if (jsRuntime != nullptr) {
            mainThreadStackInfo= jsRuntime->BuildNativeAndJsBackStackTrace();
            if (write(rFD, mainThreadStackInfo.c_str(), mainThreadStackInfo.size()) != mainThreadStackInfo.size()) {
                APP_LOGE("MainThread::HandleANRProcess write main thread stack info failed");
            }
        }
        OHOS::HiviewDFX::DfxDumpCatcher dumplog;
        std::string proStackInfo;
        if (dumplog.DumpCatch(getpid(), 0, proStackInfo) == false) {
            APP_LOGE("MainThread::HandleANRProcess get process stack info failed");
        }
        if (write(rFD, proStackInfo.c_str(), proStackInfo.size()) != proStackInfo.size()) {
            APP_LOGE("MainThread::HandleANRProcess write process stack info failed");
        }
        if (rFD != -1) {
            close(rFD);
        }
    }
}

void MainThread::Start()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
    APP_LOGI("MainThread::main called start");
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        APP_LOGE("MainThread::main failed, runner is nullptr");
        return;
    }
    std::shared_ptr<EventRunner> runnerWatchDog = EventRunner::Create("WatchDogRunner");
    if (runnerWatchDog == nullptr) {
        APP_LOGE("MainThread::Start runnerWatchDog is nullptr");
        return;
    }
    sptr<MainThread> thread = sptr<MainThread>(new (std::nothrow) MainThread());
    if (thread == nullptr) {
        APP_LOGE("MainThread::static failed. new MainThread failed");
        return;
    }

    APP_LOGI("MainThread::main Register sig handle start");
    struct sigaction sigAct;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = 0;
    sigAct.sa_handler = &MainThread::HandleANRProcess;
    sigaction(SIGUSR1, &sigAct, NULL);
    APP_LOGI("MainThread::main Register sig handle end");

    APP_LOGI("MainThread::main called start Init");
    thread->Init(runner, runnerWatchDog);
    APP_LOGI("MainThread::main called end Init");

    APP_LOGI("MainThread::main called start Attach");
    thread->Attach();
    APP_LOGI("MainThread::main called end Attach");

    int ret = runner->Run();
    if (ret != ERR_OK) {
        APP_LOGE("MainThread::main failed. runner->Run failed ret = %{public}d", ret);
    }

    thread->RemoveAppMgrDeathRecipient();
    APP_LOGI("MainThread::main runner stopped");
}

MainThread::MainHandler::MainHandler(const std::shared_ptr<EventRunner> &runner, const sptr<MainThread> &thread)
    : AppExecFwk::EventHandler(runner), mainThreadObj_(thread)
{}

/**
 *
 * @brief Process the event.
 *
 * @param event the event want to be processed.
 *
 */
void MainThread::MainHandler::ProcessEvent(const OHOS::AppExecFwk::InnerEvent::Pointer &event)
{
    auto eventId = event->GetInnerEventId();
    if (eventId == MAIN_THREAD_IS_ALIVE) {
        WatchDog::GetCurrentHandler()->SendEvent(MAIN_THREAD_IS_ALIVE);
    }
}

/**
 *
 * @brief Check whether the OHOSApplication is ready.
 *
 * @return if the record is legal, return true. else return false.
 *
 */
bool MainThread::IsApplicationReady() const
{
    APP_LOGI("MainThread::IsApplicationReady called start");
    if (application_ == nullptr || applicationImpl_ == nullptr) {
        APP_LOGI("MainThread::IsApplicationReady called. application_=null or applicationImpl_=null");
        return false;
    }

    APP_LOGI("MainThread::IsApplicationReady called end");
    return true;
}

#ifdef ABILITY_LIBRARY_LOADER
/**
 *
 * @brief Load the ability library.
 *
 * @param libraryPaths the library paths.
 *
 */
void MainThread::LoadAbilityLibrary(const std::vector<std::string> &libraryPaths)
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef ABILITY_LIBRARY_LOADER
    APP_LOGI("MainThread::LoadAbilityLibrary called start");
#ifdef ACEABILITY_LIBRARY_LOADER
#ifdef _ARM64_
    std::string acelibdir("/system/lib64/libace.z.so");
#else
    std::string acelibdir("/system/lib/libace.z.so");
#endif
    void *AceAbilityLib = nullptr;
    APP_LOGI("MainThread::LoadAbilityLibrary. Start calling dlopen acelibdir.");
    AceAbilityLib = dlopen(acelibdir.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (AceAbilityLib == nullptr) {
        APP_LOGE("Fail to dlopen %{public}s, [%{public}s]", acelibdir.c_str(), dlerror());
    } else {
        APP_LOGI("Success to dlopen %{public}s", acelibdir.c_str());
        handleAbilityLib_.emplace_back(AceAbilityLib);
    }
    APP_LOGI("MainThread::LoadAbilityLibrary. End calling dlopen.");
#endif  // ACEABILITY_LIBRARY_LOADER
    int size = libraryPaths.size();
    APP_LOGI("MainThread::LoadAbilityLibrary. size=%{public}d.", size);
    for (int index = 0; index < size; index++) {
        std::string libraryPath = libraryPaths[index];
        APP_LOGI("MainThread::LoadAbilityLibrary Try to scanDir %{public}s", libraryPath.c_str());
        if (!ScanDir(libraryPath)) {
            APP_LOGE("MainThread::LoadAbilityLibrary scanDir %{public}s not exits", libraryPath.c_str());
        }
        libraryPath = libraryPath + "/libs";
        if (!ScanDir(libraryPath)) {
            APP_LOGE("MainThread::LoadAbilityLibrary scanDir %{public}s not exits", libraryPath.c_str());
        }
    }

    if (fileEntries_.empty()) {
        APP_LOGE("No ability library");
        return;
    }

    void *handleAbilityLib = nullptr;
    for (auto fileEntry : fileEntries_) {
        if (!fileEntry.empty()) {
            APP_LOGI("MainThread::LoadAbilityLibrary. Start calling dlopen fileEntry.");
            handleAbilityLib = dlopen(fileEntry.c_str(), RTLD_NOW | RTLD_GLOBAL);
            APP_LOGI("MainThread::LoadAbilityLibrary. End calling dlopen fileEntry.");
            if (handleAbilityLib == nullptr) {
                APP_LOGE("MainThread::LoadAbilityLibrary Fail to dlopen %{public}s, [%{public}s]",
                    fileEntry.c_str(),
                    dlerror());
                exit(-1);
            } else {
                APP_LOGI("MainThread::LoadAbilityLibrary Success to dlopen %{public}s", fileEntry.c_str());
            }
            handleAbilityLib_.emplace_back(handleAbilityLib);
        }
    }
    APP_LOGI("MainThread::LoadAbilityLibrary called end.");
#endif  // ABILITY_LIBRARY_LOADER
}

void MainThread::LoadAppLibrary()
{
    BYTRACE_NAME(BYTRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef APPLICATION_LIBRARY_LOADER
    std::string appPath = applicationLibraryPath;
    APP_LOGI("MainThread::handleLaunchApplication Start calling dlopen. appPath=%{public}s", appPath.c_str());
    handleAppLib_ = dlopen(appPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (handleAppLib_ == nullptr) {
        APP_LOGE("Fail to dlopen %{public}s, [%{public}s]", appPath.c_str(), dlerror());
        exit(-1);
    }
    APP_LOGI("MainThread::handleLaunchApplication End calling dlopen.";
#endif  // APPLICATION_LIBRARY_LOADER
}

/**
 *
 * @brief Close the ability library loaded.
 *
 */
void MainThread::CloseAbilityLibrary()
{
    APP_LOGI("MainThread::CloseAbilityLibrary called start");
    for (auto iter : handleAbilityLib_) {
        if (iter != nullptr) {
            APP_LOGI("MainThread::CloseAbilityLibrary before dlclose");
            dlclose(iter);
            APP_LOGI("MainThread::CloseAbilityLibrary after dlclose");
            iter = nullptr;
        }
    }
    handleAbilityLib_.clear();
    fileEntries_.clear();
    APP_LOGI("MainThread::CloseAbilityLibrary called end");
}

/**
 *
 * @brief Scan the dir ability library loaded.
 *
 * @param dirPath the the path should be scan.
 *
 */
bool MainThread::ScanDir(const std::string &dirPath)
{
    APP_LOGI("MainThread::ScanDir called start. dirPath:  %{public}s.", dirPath.c_str());
    APP_LOGI("MainThread::ScanDir before opendir.");
    DIR *dirp = opendir(dirPath.c_str());
    if (dirp == nullptr) {
        APP_LOGE("MainThread::ScanDir open dir:%{public}s fail", dirPath.c_str());
        return false;
    }
    APP_LOGI("MainThread::ScanDir after opendir.");
    struct dirent *df = nullptr;
    for (;;) {
        APP_LOGI("MainThread::ScanDir before readdir.");
        df = readdir(dirp);
        APP_LOGI("MainThread::ScanDir after readdir.");
        if (df == nullptr) {
            break;
        }

        std::string currentName(df->d_name);
        APP_LOGD("folder found:'%{public}s'", df->d_name);
        if (currentName.compare(".") == 0 || currentName.compare("..") == 0) {
            continue;
        }

        if (CheckFileType(currentName, abilityLibraryType_)) {
            APP_LOGI("MainThread::ScanDir CheckFileType == true.");
            fileEntries_.emplace_back(dirPath + pathSeparator_ + currentName);
        }
    }

    APP_LOGI("MainThread::ScanDir before closedir.");
    if (closedir(dirp) == -1) {
        APP_LOGW("close dir fail");
    }
    APP_LOGI("MainThread::ScanDir after closedir.");
    APP_LOGI("MainThread::ScanDir called end.");
    return true;
}

/**
 *
 * @brief Check the fileType.
 *
 * @param fileName The fileName of the lib.
 * @param extensionName The extensionName of the lib.
 *
 * @return if the FileType is legal, return true. else return false.
 *
 */
bool MainThread::CheckFileType(const std::string &fileName, const std::string &extensionName)
{
    APP_LOGD("MainThread::CheckFileType path is %{public}s, support suffix is %{public}s",
        fileName.c_str(),
        extensionName.c_str());

    if (fileName.empty()) {
        APP_LOGE("the file name is empty");
        return false;
    }

    auto position = fileName.rfind('.');
    if (position == std::string::npos) {
        APP_LOGE("filename no extension name");
        return false;
    }

    std::string suffixStr = fileName.substr(position);
    APP_LOGD("MainThread::CheckFileType end.");
    return LowerStr(suffixStr) == extensionName;
}

void MainThread::HandleScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    APP_LOGI("MainThread::HandleScheduleAcceptWant");
    if (!application_) {
        APP_LOGE("application_ is nullptr");
        return;
    }

    std::string specifiedFlag;
    application_->ScheduleAcceptWant(want, moduleName, specifiedFlag);

    if (!appMgr_ || !applicationImpl_) {
        APP_LOGE("appMgr_ is nullptr");
        return;
    }

    appMgr_->ScheduleAcceptWantDone(applicationImpl_->GetRecordId(), want, specifiedFlag);
}

void MainThread::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    APP_LOGI("MainThread::ScheduleAcceptWant start");
    wptr<MainThread> weak = this;
    auto task = [weak, want, moduleName]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            APP_LOGE("abilityThread is nullptr, HandleScheduleAcceptWant failed.");
            return;
        }
        appThread->HandleScheduleAcceptWant(want, moduleName);
    };
    if (!mainHandler_->PostTask(task)) {
        APP_LOGE("MainThread::ScheduleAcceptWant PostTask task failed");
    }
    APP_LOGI("MainThread::ScheduleAcceptWant end.");
}
#endif  // ABILITY_LIBRARY_LOADER
}  // namespace AppExecFwk
}  // namespace OHOS
