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

#include "main_thread.h"

#include <malloc.h>
#include <new>
#include <regex>
#include <sys/prctl.h>
#include <unistd.h>

#include "constants.h"
#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "ability_loader.h"
#include "ability_thread.h"
#include "ability_util.h"
#include "app_loader.h"
#include "app_recovery.h"
#include "appfreeze_inner.h"
#include "application_data_manager.h"
#include "application_env_impl.h"
#include "bundle_mgr_proxy.h"
#include "hitrace_meter.h"
#include "configuration_convertor.h"
#include "common_event_manager.h"
#include "context_deal.h"
#include "context_impl.h"
#include "extension_ability_info.h"
#include "extension_module_loader.h"
#include "extension_plugin_info.h"
#include "extract_resource_manager.h"
#include "file_path_utils.h"
#include "hilog_wrapper.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#endif
#include "app_mgr_client.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "js_runtime.h"
#include "mix_stack_dumper.h"
#include "ohos_application.h"
#include "overlay_module_info.h"
#include "parameters.h"
#include "resource_manager.h"
#include "runtime.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "uncaught_exception_callback.h"
#include "hisysevent.h"
#include "js_runtime_utils.h"
#include "context/application_context.h"

#if defined(NWEB)
#include <thread>
#include "app_mgr_client.h"
#include "nweb_pre_dns_adapter.h"
#include "nweb_helper.h"
#endif

#ifdef IMAGE_PURGEABLE_PIXELMAP
#include "purgeable_resource_manager.h"
#endif

#if defined(ABILITY_LIBRARY_LOADER) || defined(APPLICATION_LIBRARY_LOADER)
#include <dirent.h>
#include <dlfcn.h>
#endif
namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityBase::Constants;
std::weak_ptr<OHOSApplication> MainThread::applicationForDump_;
std::shared_ptr<EventHandler> MainThread::signalHandler_ = nullptr;
std::shared_ptr<MainThread::MainHandler> MainThread::mainHandler_ = nullptr;
static std::shared_ptr<MixStackDumper> mixStackDumper_ = nullptr;
const std::string PERFCMD_PROFILE = "profile";
const std::string PERFCMD_DUMPHEAP = "dumpheap";
namespace {
#ifdef APP_USE_ARM
constexpr char FORM_RENDER_LIB_PATH[] = "/system/lib/libformrender.z.so";
#else
constexpr char FORM_RENDER_LIB_PATH[] = "/system/lib64/libformrender.z.so";
#endif

constexpr int32_t DELIVERY_TIME = 200;
constexpr int32_t DISTRIBUTE_TIME = 100;
constexpr int32_t UNSPECIFIED_USERID = -2;
constexpr int SIGNAL_JS_HEAP = 39;
constexpr int SIGNAL_JS_HEAP_PRIV = 40;

constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_VERSION[] = "VERSION";
constexpr char EVENT_KEY_TYPE[] = "TYPE";
constexpr char EVENT_KEY_HAPPEN_TIME[] = "HAPPEN_TIME";
constexpr char EVENT_KEY_REASON[] = "REASON";
constexpr char EVENT_KEY_JSVM[] = "JSVM";
constexpr char EVENT_KEY_SUMMARY[] = "SUMMARY";

const int32_t JSCRASH_TYPE = 3;
const std::string JSVM_TYPE = "ARK";
const std::string SIGNAL_HANDLER = "SignalHandler";

constexpr uint32_t CHECK_MAIN_THREAD_IS_ALIVE = 1;

const std::string OVERLAY_STATE_CHANGED = "usual.event.OVERLAY_STATE_CHANGED";

std::string GetLibPath(const std::string &hapPath, bool isPreInstallApp)
{
    std::string libPath = LOCAL_CODE_PATH;
    if (isPreInstallApp) {
        auto pos = hapPath.rfind("/");
        libPath = hapPath.substr(0, pos);
    }
    return libPath;
}

void GetHapSoPath(const HapModuleInfo &hapInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp)
{
    if (hapInfo.nativeLibraryPath.empty()) {
        HILOG_DEBUG("Lib path of %{public}s is empty, lib isn't isolated or compressed.", hapInfo.moduleName.c_str());
        return;
    }

    std::string appLibPathKey = hapInfo.bundleName + "/" + hapInfo.moduleName;
    std::string libPath = LOCAL_CODE_PATH;
    if (!hapInfo.compressNativeLibs) {
        HILOG_DEBUG("Lib of %{public}s will not be extracted from hap.", hapInfo.moduleName.c_str());
        libPath = GetLibPath(hapInfo.hapPath, isPreInstallApp);
    }

    libPath += (libPath.back() == '/') ? hapInfo.nativeLibraryPath : "/" + hapInfo.nativeLibraryPath;
    HILOG_INFO("appLibPathKey: %{private}s, lib path: %{private}s", appLibPathKey.c_str(), libPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(libPath);
}

void GetHspNativeLibPath(const BaseSharedBundleInfo &hspInfo, AppLibPathMap &appLibPaths, bool isPreInstallApp)
{
    if (hspInfo.nativeLibraryPath.empty()) {
        return;
    }

    std::string appLibPathKey = hspInfo.bundleName + "/" + hspInfo.moduleName;
    std::string libPath = LOCAL_CODE_PATH;
    if (!hspInfo.compressNativeLibs) {
        libPath = GetLibPath(hspInfo.hapPath, isPreInstallApp);
        libPath = libPath.back() == '/' ? libPath : libPath + "/";
        if (isPreInstallApp) {
            libPath += hspInfo.nativeLibraryPath;
        } else {
            libPath += hspInfo.bundleName + "/" + hspInfo.moduleName + "/" + hspInfo.nativeLibraryPath;
        }
    } else {
        libPath = libPath.back() == '/' ? libPath : libPath + "/";
        libPath += hspInfo.bundleName + "/" + hspInfo.nativeLibraryPath;
    }

    HILOG_DEBUG("appLibPathKey: %{private}s, libPath: %{private}s", appLibPathKey.c_str(), libPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(libPath);
}

void GetPatchNativeLibPath(const HapModuleInfo &hapInfo, std::string &patchNativeLibraryPath,
    AppLibPathMap &appLibPaths)
{
    if (hapInfo.isLibIsolated) {
        patchNativeLibraryPath = hapInfo.hqfInfo.nativeLibraryPath;
    }

    if (patchNativeLibraryPath.empty()) {
        HILOG_DEBUG("Patch lib path of %{public}s is empty.", hapInfo.moduleName.c_str());
        return;
    }

    if (hapInfo.compressNativeLibs && !hapInfo.isLibIsolated) {
        HILOG_DEBUG("Lib of %{public}s has compressed and isn't isolated, no need to set.", hapInfo.moduleName.c_str());
        return;
    }

    std::string appLibPathKey = hapInfo.bundleName + "/" + hapInfo.moduleName;
    std::string patchLibPath = LOCAL_CODE_PATH;
    patchLibPath += (patchLibPath.back() == '/') ? patchNativeLibraryPath : "/" + patchNativeLibraryPath;
    HILOG_INFO("appLibPathKey: %{public}s, patch lib path: %{private}s", appLibPathKey.c_str(), patchLibPath.c_str());
    appLibPaths[appLibPathKey].emplace_back(patchLibPath);
}
} // namespace

void MainThread::GetNativeLibPath(const BundleInfo &bundleInfo, const HspList &hspList, AppLibPathMap &appLibPaths)
{
    std::string patchNativeLibraryPath = bundleInfo.applicationInfo.appQuickFix.deployedAppqfInfo.nativeLibraryPath;
    if (!patchNativeLibraryPath.empty()) {
        // libraries in patch lib path has a higher priority when loading.
        std::string patchLibPath = LOCAL_CODE_PATH;
        patchLibPath += (patchLibPath.back() == '/') ? patchNativeLibraryPath : "/" + patchNativeLibraryPath;
        HILOG_INFO("napi patch lib path = %{private}s", patchLibPath.c_str());
        appLibPaths["default"].emplace_back(patchLibPath);
    }

    std::string nativeLibraryPath = bundleInfo.applicationInfo.nativeLibraryPath;
    if (!nativeLibraryPath.empty()) {
        if (nativeLibraryPath.back() == '/') {
            nativeLibraryPath.pop_back();
        }
        std::string libPath = LOCAL_CODE_PATH;
        libPath += (libPath.back() == '/') ? nativeLibraryPath : "/" + nativeLibraryPath;
        HILOG_INFO("napi lib path = %{private}s", libPath.c_str());
        appLibPaths["default"].emplace_back(libPath);
    }

    for (auto &hapInfo : bundleInfo.hapModuleInfos) {
        HILOG_DEBUG("moduleName: %{public}s, isLibIsolated: %{public}d, compressNativeLibs: %{public}d.",
            hapInfo.moduleName.c_str(), hapInfo.isLibIsolated, hapInfo.compressNativeLibs);
        GetPatchNativeLibPath(hapInfo, patchNativeLibraryPath, appLibPaths);
        GetHapSoPath(hapInfo, appLibPaths, hapInfo.hapPath.find(ABS_CODE_PATH));
    }

    for (auto &hspInfo : hspList) {
        HILOG_DEBUG("bundle:%s, module:%s, nativeLibraryPath:%s", hspInfo.bundleName.c_str(),
            hspInfo.moduleName.c_str(), hspInfo.nativeLibraryPath.c_str());
        GetHspNativeLibPath(hspInfo, appLibPaths, hspInfo.hapPath.find(ABS_CODE_PATH) != 0u);
    }
}

#define ACEABILITY_LIBRARY_LOADER
#ifdef ABILITY_LIBRARY_LOADER
    const std::string acelibdir("libace.z.so");
#endif

/**
 *
 * @brief Notify the AppMgrDeathRecipient that the remote is dead.
 *
 * @param remote The remote which is dead.
 */
void AppMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_ERROR("MainThread::AppMgrDeathRecipient remote died receive");
}

MainThread::MainThread()
{
    HILOG_INFO("MainThread");
#ifdef ABILITY_LIBRARY_LOADER
    fileEntries_.clear();
    nativeFileEntries_.clear();
    handleAbilityLib_.clear();
#endif  // ABILITY_LIBRARY_LOADER
}

MainThread::~MainThread()
{
    HILOG_INFO("~MainThread");
    if (watchdog_ != nullptr && !watchdog_->IsStopWatchdog()) {
        watchdog_->Stop();
        watchdog_ = nullptr;
    }
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread ConnectToAppMgr start.");
    auto object = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(APP_MGR_SERVICE_ID);
    if (object == nullptr) {
        HILOG_ERROR("failed to get app manager service");
        return false;
    }
    deathRecipient_ = new (std::nothrow) AppMgrDeathRecipient();
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("failed to new AppMgrDeathRecipient");
        return false;
    }

    if (!object->AddDeathRecipient(deathRecipient_)) {
        HILOG_ERROR("failed to AddDeathRecipient");
        return false;
    }

    appMgr_ = iface_cast<IAppMgr>(object);
    if (appMgr_ == nullptr) {
        HILOG_ERROR("failed to iface_cast object to appMgr_");
        return false;
    }
    appMgr_->AttachApplication(this);
    HILOG_DEBUG("MainThread::connectToAppMgr end");
    return true;
}

/**
 *
 * @brief Attach the mainthread to the AppMgr.
 *
 */
void MainThread::Attach()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Attach");
    if (!ConnectToAppMgr()) {
        HILOG_ERROR("attachApplication failed");
        return;
    }
    mainThreadState_ = MainThreadState::ATTACH;
}

/**
 *
 * @brief remove the deathRecipient from appMgr.
 *
 */
void MainThread::RemoveAppMgrDeathRecipient()
{
    HILOG_DEBUG("RemoveAppMgrDeathRecipient");
    if (appMgr_ == nullptr) {
        HILOG_ERROR("MainThread::RemoveAppMgrDeathRecipient failed");
        return;
    }

    sptr<IRemoteObject> object = appMgr_->AsObject();
    if (object != nullptr) {
        object->RemoveDeathRecipient(deathRecipient_);
    } else {
        HILOG_ERROR("appMgr_->AsObject() failed");
    }
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ScheduleForegroundApplication");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleForegroundApplication failed.");
            return;
        }
        appThread->HandleForegroundApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("PostTask task failed");
    }
    watchdog_->SetBackgroundStatus(false);
}

/**
 *
 * @brief Schedule the background lifecycle of application.
 *
 */
void MainThread::ScheduleBackgroundApplication()
{
    HILOG_DEBUG("ScheduleBackgroundApplication");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleBackgroundApplication failed.");
            return;
        }
        appThread->HandleBackgroundApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleBackgroundApplication PostTask task failed");
    }
    watchdog_->SetBackgroundStatus(true);
}

/**
 *
 * @brief Schedule the terminate lifecycle of application.
 *
 */
void MainThread::ScheduleTerminateApplication()
{
    HILOG_DEBUG("ScheduleTerminateApplication");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleTerminateApplication failed.");
            return;
        }
        appThread->HandleTerminateApplication();
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleTerminateApplication PostTask task failed");
    }
}

/**
 *
 * @brief Shrink the memory which used by application.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void MainThread::ScheduleShrinkMemory(const int level)
{
    HILOG_DEBUG("scheduleShrinkMemory level: %{public}d", level);
    wptr<MainThread> weak = this;
    auto task = [weak, level]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleShrinkMemory(level);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleShrinkMemory PostTask task failed");
    }
}

/**
 *
 * @brief Notify the memory level.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 */
void MainThread::ScheduleMemoryLevel(const int level)
{
    HILOG_DEBUG("ScheduleMemoryLevel level: %{public}d", level);
    wptr<MainThread> weak = this;
    auto task = [weak, level]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleMemoryLevel failed.");
            return;
        }
        appThread->HandleMemoryLevel(level);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleMemoryLevel PostTask task failed");
    }
}

/**
 *
 * @brief Get the application's memory allocation info.
 *
 * @param pid, pid input.
 * @param mallocInfo, dynamic storage information output.
 */
void MainThread::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    struct mallinfo mi = mallinfo();
    int usmblks = mi.usmblks; // 当前从分配器中分配的总的堆内存大小
    int uordblks = mi.uordblks; // 当前已释放给分配器，分配缓存了未释放给系统的内存大小
    int fordblks = mi.fordblks; // 当前未释放的大小
    HILOG_DEBUG("The pid of the app we want to dump memory allocation information is: %{public}i", pid);
    HILOG_DEBUG("usmblks: %{public}i, uordblks: %{public}i, fordblks: %{public}i", usmblks, uordblks, fordblks);
    mallocInfo.usmblks = usmblks;
    mallocInfo.uordblks = uordblks;
    mallocInfo.fordblks = fordblks;
}

/**
 *
 * @brief Schedule the application process exit safely.
 *
 */
void MainThread::ScheduleProcessSecurityExit()
{
    HILOG_DEBUG("ScheduleProcessSecurityExit called");
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, ScheduleProcessSecurityExit failed.");
            return;
        }
        appThread->HandleProcessSecurityExit();
    };
    bool result = mainHandler_->PostTask(task);
    if (!result) {
        HILOG_ERROR("ScheduleProcessSecurityExit post task failed");
    }
}

/**
 *
 * @brief Low the memory which used by application.
 *
 */
void MainThread::ScheduleLowMemory()
{
    HILOG_DEBUG("MainThread::scheduleLowMemory called");
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread schedule launch application start.");
    wptr<MainThread> weak = this;
    auto task = [weak, data, config]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleLaunchApplication failed.");
            return;
        }
        appThread->HandleLaunchApplication(data, config);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleLaunchApplication PostTask task failed");
    }
}

/**
 *
 * @brief update the application info after new module installed.
 *
 * @param appInfo The latest application info obtained from bms for update abilityRuntimeContext.
 *
 */
void MainThread::ScheduleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    HILOG_DEBUG("ScheduleUpdateApplicationInfoInstalled");
    wptr<MainThread> weak = this;
    auto task = [weak, appInfo]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleUpdateApplicationInfoInstalled failed.");
            return;
        }
        appThread->HandleUpdateApplicationInfoInstalled(appInfo);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleUpdateApplicationInfoInstalled PostTask task failed");
    }
}

void MainThread::ScheduleAbilityStage(const HapModuleInfo &abilityStage)
{
    HILOG_DEBUG("ScheduleAbilityStage");
    wptr<MainThread> weak = this;
    auto task = [weak, abilityStage]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleShrinkMemory failed.");
            return;
        }
        appThread->HandleAbilityStage(abilityStage);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleAbilityStageInfo PostTask task failed");
    }
}

void MainThread::ScheduleLaunchAbility(const AbilityInfo &info, const sptr<IRemoteObject> &token,
    const std::shared_ptr<AAFwk::Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("schedule launch ability %{public}s, type is %{public}d.", info.name.c_str(), info.type);

    AAFwk::Want newWant(*want);
    newWant.CloseAllFd();
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>(info);
    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(abilityInfo, token);
    abilityRecord->SetWant(want);

    wptr<MainThread> weak = this;
    auto task = [weak, abilityRecord]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleLaunchAbility failed.");
            return;
        }
        appThread->HandleLaunchAbility(abilityRecord);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleLaunchAbility PostTask task failed");
    }
}

/**
 *
 * @brief clean the ability by token.
 *
 * @param token The token belong to the ability which want to be cleaned.
 *
 */
void MainThread::ScheduleCleanAbility(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("Schedule clean ability called.");
    wptr<MainThread> weak = this;
    auto task = [weak, token]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleCleanAbility failed.");
            return;
        }
        appThread->HandleCleanAbility(token);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleCleanAbility PostTask task failed");
    }
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
    HILOG_DEBUG("MainThread::scheduleProfileChanged profile name: %{public}s", profile.GetName().c_str());
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
    HILOG_DEBUG("ScheduleConfigurationUpdated");
    wptr<MainThread> weak = this;
    auto task = [weak, config]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, HandleConfigurationUpdated failed.");
            return;
        }
        appThread->HandleConfigurationUpdated(config);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleConfigurationUpdated PostTask task failed");
    }
}

bool MainThread::CheckLaunchApplicationParam(const AppLaunchData &appLaunchData) const
{
    ApplicationInfo appInfo = appLaunchData.GetApplicationInfo();
    ProcessInfo processInfo = appLaunchData.GetProcessInfo();

    if (appInfo.name.empty()) {
        HILOG_ERROR("MainThread::checkLaunchApplicationParam applicationName is empty");
        return false;
    }

    if (processInfo.GetProcessName().empty()) {
        HILOG_ERROR("MainThread::checkLaunchApplicationParam processName is empty");
        return false;
    }
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
    if (record == nullptr) {
        HILOG_ERROR("MainThread::checkAbilityItem record is null");
        return false;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    sptr<IRemoteObject> token = record->GetToken();

    if (abilityInfo == nullptr) {
        HILOG_ERROR("MainThread::checkAbilityItem abilityInfo is null");
        return false;
    }

    if (token == nullptr) {
        HILOG_ERROR("MainThread::checkAbilityItem token is null");
        return false;
    }
    return true;
}

/**
 *
 * @brief Terminate the application but don't notify ams.
 *
 */
void MainThread::HandleTerminateApplicationLocal()
{
    HILOG_DEBUG("HandleTerminateApplicationLocal");
    if (application_ == nullptr) {
        HILOG_ERROR("MainThread::HandleTerminateApplicationLocal error!");
        return;
    }
    applicationImpl_->PerformTerminateStrong();

    std::shared_ptr<EventRunner> signalRunner = signalHandler_->GetEventRunner();
    if (signalRunner) {
        signalRunner->Stop();
    }

    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        HILOG_ERROR("MainThread::HandleTerminateApplicationLocal get manHandler error");
        return;
    }

    if (watchdog_ != nullptr && !watchdog_->IsStopWatchdog()) {
        watchdog_->Stop();
        watchdog_ = nullptr;
    }

    int ret = runner->Stop();
    if (ret != ERR_OK) {
        HILOG_ERROR("MainThread::HandleTerminateApplicationLocal failed. runner->Run failed ret = %{public}d", ret);
    }
    HILOG_DEBUG("runner is stopped");
    SetRunnerStarted(false);
}

/**
 *
 * @brief Schedule the application process exit safely.
 *
 */
void MainThread::HandleProcessSecurityExit()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("HandleProcessSecurityExit");
    if (abilityRecordMgr_ == nullptr) {
        HILOG_ERROR("MainThread::HandleProcessSecurityExit abilityRecordMgr_ is null");
        return;
    }

    std::vector<sptr<IRemoteObject>> tokens = (abilityRecordMgr_->GetAllTokens());

    for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
        HandleCleanAbilityLocal(*iter);
    }

    HandleTerminateApplicationLocal();
}

bool MainThread::InitCreate(
    std::shared_ptr<ContextDeal> &contextDeal, ApplicationInfo &appInfo, ProcessInfo &processInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    // get application shared point
    application_ = std::shared_ptr<OHOSApplication>(ApplicationLoader::GetInstance().GetApplicationByName());
    if (application_ == nullptr) {
        HILOG_ERROR("InitCreate application create failed");
        return false;
    }

    applicationInfo_ = std::make_shared<ApplicationInfo>(appInfo);
    if (applicationInfo_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create applicationInfo_ failed");
        return false;
    }

    processInfo_ = std::make_shared<ProcessInfo>(processInfo);
    if (processInfo_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create processInfo_ failed");
        return false;
    }

    applicationImpl_ = std::make_shared<ApplicationImpl>();
    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create applicationImpl_ failed");
        return false;
    }

    abilityRecordMgr_ = std::make_shared<AbilityRecordMgr>();
    if (abilityRecordMgr_ == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create AbilityRecordMgr failed");
        return false;
    }

    contextDeal = std::make_shared<ContextDeal>();
    if (contextDeal == nullptr) {
        HILOG_ERROR("MainThread::InitCreate create contextDeal failed");
        return false;
    }
    AppExecFwk::AppfreezeInner::GetInstance()->SetApplicationInfo(applicationInfo_);

    application_->SetProcessInfo(processInfo_);
    contextDeal->SetApplicationInfo(applicationInfo_);
    contextDeal->SetBundleCodePath(applicationInfo_->codePath);  // BMS need to add cpath

    return true;
}

bool MainThread::CheckForHandleLaunchApplication(const AppLaunchData &appLaunchData)
{
    if (application_ != nullptr) {
        HILOG_ERROR("MainThread::handleLaunchApplication already create application");
        return false;
    }

    if (!CheckLaunchApplicationParam(appLaunchData)) {
        HILOG_ERROR("MainThread::handleLaunchApplication appLaunchData invalid");
        return false;
    }
    return true;
}

bool MainThread::InitResourceManager(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager,
    const AppExecFwk::HapModuleInfo &entryHapModuleInfo, const std::string &bundleName,
    bool multiProjects, const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    bool isStageBased = entryHapModuleInfo.isStageBasedModel;
    if (isStageBased && multiProjects) {
        HILOG_INFO("MainThread::InitResourceManager for multiProjects.");
    } else {
        std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + bundleName);
        std::string loadPath =
            (!entryHapModuleInfo.hapPath.empty()) ? entryHapModuleInfo.hapPath : entryHapModuleInfo.resourcePath;
        if (!loadPath.empty()) {
            loadPath = std::regex_replace(loadPath, pattern, std::string(LOCAL_CODE_PATH));
            HILOG_DEBUG("ModuleResPath: %{public}s", loadPath.c_str());
            // getOverlayPath
            auto res = GetOverlayModuleInfos(bundleName, entryHapModuleInfo.moduleName, overlayModuleInfos_);
            if (res != ERR_OK) {
                HILOG_WARN("Get overlay paths from bms failed.");
            }
            if (overlayModuleInfos_.size() == 0) {
                if (!resourceManager->AddResource(loadPath.c_str())) {
                    HILOG_ERROR("AddResource failed");
                }
            } else {
                std::vector<std::string> overlayPaths;
                for (auto it : overlayModuleInfos_) {
                    if (std::regex_search(it.hapPath, std::regex(bundleName))) {
                        it.hapPath = std::regex_replace(it.hapPath, pattern, std::string(LOCAL_CODE_PATH));
                    } else {
                        it.hapPath = std::regex_replace(it.hapPath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
                    }
                    if (it.state == OverlayState::OVERLAY_ENABLE) {
                        HILOG_DEBUG("InitResourceManager hapPath: %{public}s", it.hapPath.c_str());
                        overlayPaths.emplace_back(it.hapPath);
                    }
                }
                HILOG_DEBUG("OverlayPaths size:%{public}zu.", overlayPaths.size());
                if (!resourceManager->AddResource(loadPath, overlayPaths)) {
                    HILOG_ERROR("AddResource failed");
                }
                // add listen overlay change
                EventFwk::MatchingSkills matchingSkills;
                matchingSkills.AddEvent(OVERLAY_STATE_CHANGED);
                EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
                subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
                wptr<MainThread> weak = this;
                auto callback = [weak, resourceManager, bundleName, moduleName = entryHapModuleInfo.moduleName,
                    loadPath](const EventFwk::CommonEventData &data) {
                    HILOG_DEBUG("On overlay changed.");
                    auto appThread = weak.promote();
                    if (appThread == nullptr) {
                        HILOG_ERROR("abilityThread is nullptr, SetRunnerStarted failed.");
                        return;
                    }
                    appThread->OnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
                };
                auto subscriber = std::make_shared<OverlayEventSubscriber>(subscribeInfo, callback);
                bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
                HILOG_DEBUG("Overlay event subscriber register result is %{public}d", subResult);
            }
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    const icu::Locale *localeInfo = resConfig->GetLocaleInfo();
    if (localeInfo != nullptr) {
        HILOG_INFO("Language: %{public}s, script: %{public}s, region: %{public}s",
            localeInfo->getLanguage(), localeInfo->getScript(), localeInfo->getCountry());
    }
#endif
    std::string colormode = config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    HILOG_DEBUG("Colormode is %{public}s.", colormode.c_str());
    resConfig->SetColorMode(ConvertColorMode(colormode));

    std::string hasPointerDevice = config.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    HILOG_DEBUG("HasPointerDevice is %{public}s.", hasPointerDevice.c_str());
    resConfig->SetInputDevice(ConvertHasPointerDevice(hasPointerDevice));

    std::string deviceType = config.GetItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE);
    HILOG_DEBUG("deviceType is %{public}s <---->  %{public}d.", deviceType.c_str(), ConvertDeviceType(deviceType));
    resConfig->SetDeviceType(ConvertDeviceType(deviceType));
    resourceManager->UpdateResConfig(*resConfig);
    return true;
}

void MainThread::OnOverlayChanged(const EventFwk::CommonEventData &data,
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
    const std::string &moduleName, const std::string &loadPath)
{
    HILOG_DEBUG("OnOverlayChanged begin.");
    if (mainHandler_ == nullptr) {
        HILOG_ERROR("mainHandler is nullptr.");
        return;
    }
    wptr<MainThread> weak = this;
    auto task = [weak, data, resourceManager, bundleName, moduleName, loadPath]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, SetRunnerStarted failed.");
            return;
        }
        appThread->HandleOnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::ScheduleConfigurationUpdated PostTask task failed");
    }
}

void MainThread::HandleOnOverlayChanged(const EventFwk::CommonEventData &data,
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
    const std::string &moduleName, const std::string &loadPath)
{
    HILOG_DEBUG("HandleOnOverlayChanged begin.");
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action != OVERLAY_STATE_CHANGED) {
        HILOG_DEBUG("Not this subscribe, action: %{public}s.", action.c_str());
        return;
    }
    bool isEnable = data.GetWant().GetBoolParam(Constants::OVERLAY_STATE, false);
    // 1.get overlay hapPath
    if (resourceManager == nullptr) {
        HILOG_ERROR("resourceManager is nullptr");
        return;
    }
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    auto res = GetOverlayModuleInfos(bundleName, moduleName, overlayModuleInfos);
    if (res != ERR_OK) {
        return;
    }

    // 2.add/remove overlay hapPath
    if (loadPath.empty() || overlayModuleInfos.size() == 0) {
        HILOG_WARN("There is not any hapPath in overlayModuleInfo");
    } else {
        if (isEnable) {
            std::vector<std::string> overlayPaths = GetAddOverlayPaths(overlayModuleInfos);
            if (!resourceManager->AddResource(loadPath, overlayPaths)) {
                HILOG_ERROR("AddResource failed");
            }
        } else {
            std::vector<std::string> overlayPaths = GetRemoveOverlayPaths(overlayModuleInfos);
            if (!resourceManager->RemoveResource(loadPath, overlayPaths)) {
                HILOG_ERROR("RemoveResource failed");
            }
        }
    }
}
[[maybe_unused]] static std::string GetNativeStrFromJsTaggedObj(NativeObject* obj, const char* key)
{
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get value from key:%{public}s, Null NativeObject", key);
        return "";
    }

    NativeValue* value = obj->GetProperty(key);
    NativeString* valueStr = AbilityRuntime::ConvertNativeValueTo<NativeString>(value);
    if (valueStr == nullptr) {
        HILOG_ERROR("Failed to convert value from key:%{public}s", key);
        return "";
    }
    size_t valueStrBufLength = valueStr->GetLength();
    size_t valueStrLength = 0;
    char* valueCStr = new (std::nothrow) char[valueStrBufLength + 1];
    if (valueCStr == nullptr) {
        HILOG_ERROR("Failed to new valueCStr");
        return "";
    }
    valueStr->GetCString(valueCStr, valueStrBufLength + 1, &valueStrLength);
    std::string ret(valueCStr, valueStrLength);
    delete []valueCStr;
    HILOG_DEBUG("GetNativeStrFromJsTaggedObj Success %{public}s:%{public}s", key, ret.c_str());
    return ret;
}

bool IsNeedLoadLibrary(const std::string &bundleName)
{
    std::vector<std::string> needLoadLibraryBundleNames{
        "com.ohos.contactsdataability",
        "com.ohos.medialibrary.medialibrarydata",
        "com.ohos.telephonydataability",
        "com.ohos.FusionSearch",
        "com.ohos.formrenderservice"
    };

    for (const auto &item : needLoadLibraryBundleNames) {
        if (item == bundleName) {
            return true;
        }
    }
    return false;
}

bool GetBundleForLaunchApplication(sptr<IBundleMgr> bundleMgr, const std::string &bundleName,
    int32_t appIndex, BundleInfo &bundleInfo)
{
    bool queryResult;
    if (appIndex != 0) {
        HILOG_INFO("bundleName = %{public}s", bundleName.c_str());
        queryResult = (bundleMgr->GetSandboxBundleInfo(bundleName,
            appIndex, UNSPECIFIED_USERID, bundleInfo) == 0);
    } else {
        HILOG_INFO("bundleName = %{public}s", bundleName.c_str());
        queryResult = (bundleMgr->GetBundleInfoForSelf(
            (static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) == ERR_OK);
    }
    return queryResult;
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread handle launch application start.");
    if (!CheckForHandleLaunchApplication(appLaunchData)) {
        HILOG_ERROR("MainThread::handleLaunchApplication CheckForHandleLaunchApplication failed");
        return;
    }

    auto appInfo = appLaunchData.GetApplicationInfo();
    ProcessInfo processInfo = appLaunchData.GetProcessInfo();
    HILOG_DEBUG("MainThread handle launch application, InitCreate Start.");
    std::shared_ptr<ContextDeal> contextDeal;
    if (!InitCreate(contextDeal, appInfo, processInfo)) {
        HILOG_ERROR("MainThread::handleLaunchApplication InitCreate failed");
        return;
    }
    sptr<IBundleMgr> bundleMgr = contextDeal->GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("MainThread::handleLaunchApplication GetBundleManager is nullptr");
        return;
    }

    auto bundleName = appInfo.bundleName;
    BundleInfo bundleInfo;
    if (!GetBundleForLaunchApplication(bundleMgr, bundleName, appLaunchData.GetAppIndex(), bundleInfo)) {
        HILOG_ERROR("HandleLaunchApplication GetBundleInfo failed!");
        return;
    }

    if (IsNeedLoadLibrary(bundleName)) {
        std::vector<std::string> localPaths;
        ChangeToLocalPath(bundleName, appInfo.moduleSourceDirs, localPaths);
        LoadAbilityLibrary(localPaths);
        LoadNativeLiabrary(bundleInfo, appInfo.nativeLibraryPath);
    }
    if (appInfo.needAppDetail) {
        HILOG_DEBUG("MainThread::handleLaunchApplication %{public}s need add app detail ability library path",
            bundleName.c_str());
        LoadAppDetailAbilityLibrary(appInfo.appDetailAbilityLibraryPath);
    }
    LoadAppLibrary();

    applicationForDump_ = application_;
    mixStackDumper_ = std::make_shared<MixStackDumper>();
    if (!mixStackDumper_->IsInstalled()) {
        mixStackDumper_->InstallDumpHandler(application_, signalHandler_);
    }

    bool moduelJson = false;
    bool isStageBased = false;
    bool findEntryHapModuleInfo = false;
    AppExecFwk::HapModuleInfo entryHapModuleInfo;
    if (!bundleInfo.hapModuleInfos.empty()) {
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (hapModuleInfo.moduleType == AppExecFwk::ModuleType::ENTRY) {
                findEntryHapModuleInfo = true;
                entryHapModuleInfo = hapModuleInfo;
                break;
            }
        }
        if (!findEntryHapModuleInfo) {
            HILOG_WARN("HandleLaunchApplication find entry hap module info failed!");
            entryHapModuleInfo = bundleInfo.hapModuleInfos.back();
        }
        moduelJson = entryHapModuleInfo.isModuleJson;
        isStageBased = entryHapModuleInfo.isStageBasedModel;
    }
    if (isStageBased) {
        AppRecovery::GetInstance().InitApplicationInfo(GetMainHandler(), GetApplicationInfo());
    }
    HILOG_DEBUG("stageBased:%{public}d moduleJson:%{public}d size:%{public}zu",
        isStageBased, moduelJson, bundleInfo.hapModuleInfos.size());

    // create contextImpl
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    contextImpl->SetApplicationInfo(std::make_shared<ApplicationInfo>(appInfo));
    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application_->SetApplicationContext(applicationContext);

    HspList hspList;
    ErrCode ret = bundleMgr->GetBaseSharedBundleInfos(appInfo.bundleName, hspList);
    if (ret != ERR_OK) {
        HILOG_ERROR("GetBaseSharedBundleInfos failed: %{public}d", ret);
    }
    AppLibPathMap appLibPaths {};
    GetNativeLibPath(bundleInfo, hspList, appLibPaths);
    bool isSystemApp = bundleInfo.applicationInfo.isSystemApp;
    HILOG_DEBUG("the application isSystemApp: %{public}d", isSystemApp);
    AbilityRuntime::JsRuntime::SetAppLibPath(appLibPaths, isSystemApp);

    if (isStageBased) {
        // Create runtime
        auto hapPath = entryHapModuleInfo.hapPath;
        auto moduleName = entryHapModuleInfo.moduleName;
        AbilityRuntime::Runtime::Options options;
        options.bundleName = appInfo.bundleName;
        options.codePath = LOCAL_CODE_PATH;
        options.hapPath = hapPath;
        options.moduleName = moduleName;
        options.eventRunner = mainHandler_->GetEventRunner();
        options.loadAce = true;
        options.isBundle = (entryHapModuleInfo.compileMode != AppExecFwk::CompileMode::ES_MODULE);
        options.isDebugVersion = bundleInfo.applicationInfo.debug;
        options.arkNativeFilePath = bundleInfo.applicationInfo.arkNativeFilePath;
        options.uid = bundleInfo.applicationInfo.uid;
        options.apiTargetVersion = appInfo.apiTargetVersion;
        auto runtime = AbilityRuntime::Runtime::Create(options);
        if (!runtime) {
            HILOG_ERROR("Failed to create runtime");
            return;
        }
        auto& jsEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNativeEngine();
        auto bundleName = appInfo.bundleName;
        auto versionCode = appInfo.versionCode;
        JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
        uncaughtExceptionInfo.hapPath = hapPath;
        wptr<MainThread> weak = this;
        uncaughtExceptionInfo.uncaughtTask = [weak, bundleName, versionCode]
            (std::string summary, const JsEnv::ErrorObject errorObj) {
            auto appThread = weak.promote();
            if (appThread == nullptr) {
                HILOG_ERROR("appThread is nullptr.");
                return;
            }
            time_t timet;
            time(&timet);
            HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK, "JS_ERROR",
                OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                EVENT_KEY_PACKAGE_NAME, bundleName,
                EVENT_KEY_VERSION, std::to_string(versionCode),
                EVENT_KEY_TYPE, JSCRASH_TYPE,
                EVENT_KEY_HAPPEN_TIME, timet,
                EVENT_KEY_REASON, errorObj.name,
                EVENT_KEY_JSVM, JSVM_TYPE,
                EVENT_KEY_SUMMARY, summary);
            ErrorObject appExecErrorObj = {
                .name = errorObj.name,
                .message = errorObj.message,
                .stack = errorObj.stack
            };
            FaultData faultData;
            faultData.faultType = FaultDataType::JS_ERROR;
            faultData.errorObject = appExecErrorObj;
            DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->NotifyAppFault(faultData);
            if (ApplicationDataManager::GetInstance().NotifyUnhandledException(summary) &&
                ApplicationDataManager::GetInstance().NotifyExceptionObject(appExecErrorObj)) {
                return;
            }
            // if app's callback has been registered, let app decide whether exit or not.
            HILOG_ERROR("\n%{public}s is about to exit due to RuntimeError\nError type:%{public}s\n%{public}s",
                bundleName.c_str(), errorObj.name.c_str(), summary.c_str());
            DelayedSingleton<AbilityManagerClient>::GetInstance()->RecordAppExitReason(REASON_JS_ERROR);
            appThread->ScheduleProcessSecurityExit();
        };
        (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
        application_->SetRuntime(std::move(runtime));

        std::weak_ptr<OHOSApplication> wpApplication = application_;
        AbilityLoader::GetInstance().RegisterUIAbility("UIAbility",
            [wpApplication]() -> AbilityRuntime::UIAbility* {
            auto app = wpApplication.lock();
            if (app != nullptr) {
                return AbilityRuntime::UIAbility::Create(app->GetRuntime());
            }
            HILOG_ERROR("AbilityLoader::GetAbilityByName failed.");
            return nullptr;
        });
        if (application_ != nullptr) {
            LoadAllExtensions(jsEngine);
        }

        IdleTimeCallback callback = [wpApplication](int32_t idleTime) {
            auto app = wpApplication.lock();
            if (app == nullptr) {
                HILOG_ERROR("app is nullptr.");
                return;
            }
            auto &runtime = app->GetRuntime();
            if (runtime == nullptr) {
                HILOG_ERROR("runtime is nullptr.");
                return;
            }
            auto& nativeEngine = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNativeEngine();
            nativeEngine.NotifyIdleTime(idleTime);
        };
        idleTime_ = std::make_shared<IdleTime>(mainHandler_, callback);
        idleTime_->Start();

        IdleNotifyStatusCallback cb = idleTime_->GetIdleNotifyFunc();
        jsEngine.NotifyIdleStatusControl(cb);
    }

    auto usertestInfo = appLaunchData.GetUserTestInfo();
    if (usertestInfo) {
        if (!PrepareAbilityDelegator(usertestInfo, isStageBased, entryHapModuleInfo)) {
            HILOG_ERROR("Failed to prepare ability delegator");
            return;
        }
    }

    // init resourceManager.
    HILOG_DEBUG("MainThread handle launch application, CreateResourceManager Start.");
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (resourceManager == nullptr) {
        HILOG_ERROR("MainThread::handleLaunchApplication create resourceManager failed");
        return;
    }

    if (!InitResourceManager(resourceManager, entryHapModuleInfo, bundleInfo.name,
        bundleInfo.applicationInfo.multiProjects, config)) {
        HILOG_ERROR("MainThread::handleLaunchApplication InitResourceManager failed");
        return;
    }
    contextImpl->SetResourceManager(resourceManager);
    AbilityBase::ExtractResourceManager::GetExtractResourceManager().SetGlobalObject(resourceManager);

    contextDeal->initResourceManager(resourceManager);
    contextDeal->SetApplicationContext(application_);
    application_->AttachBaseContext(contextDeal);
    application_->SetAbilityRecordMgr(abilityRecordMgr_);
    application_->SetConfiguration(config);
    contextImpl->SetConfiguration(application_->GetConfiguration());

    applicationImpl_->SetRecordId(appLaunchData.GetRecordId());
    applicationImpl_->SetApplication(application_);
    mainThreadState_ = MainThreadState::READY;
    if (!applicationImpl_->PerformAppReady()) {
        HILOG_ERROR("HandleLaunchApplication::application applicationImpl_->PerformAppReady failed");
        return;
    }
    // L1 needs to add corresponding interface
    ApplicationEnvImpl *pAppEvnIml = ApplicationEnvImpl::GetInstance();

    if (pAppEvnIml) {
        pAppEvnIml->SetAppInfo(*applicationInfo_.get());
    } else {
        HILOG_ERROR("HandleLaunchApplication::application pAppEvnIml is null");
    }

#if defined(NWEB)
    // pre dns for nweb
    std::thread(&OHOS::NWeb::PreDnsInThread).detach();

    // start nwebspawn process
    std::weak_ptr<OHOSApplication> weakApp = application_;
    wptr<IAppMgr> weakMgr = appMgr_;
    std::thread([weakApp, weakMgr] {
        auto app = weakApp.lock();
        auto appmgr = weakMgr.promote();
        if (app == nullptr || appmgr == nullptr) {
            HILOG_ERROR("HandleLaunchApplication app or appmgr is null");
            return;
        }

        if (prctl(PR_SET_NAME, "preStartNWeb") < 0) {
            HILOG_WARN("Set thread name failed with %{public}d", errno);
        }

        std::string nwebPath = app->GetAppContext()->GetCacheDir() + "/web";
        bool isFirstStartUpWeb = (access(nwebPath.c_str(), F_OK) != 0);
        if (!isFirstStartUpWeb) {
            appmgr->PreStartNWebSpawnProcess();
        }
        OHOS::NWeb::NWebHelper::TryPreReadLib(isFirstStartUpWeb, app->GetAppContext()->GetBundleCodeDir());
    }).detach();
#endif
}

#ifdef ABILITY_LIBRARY_LOADER
void MainThread::CalcNativeLiabraryEntries(const BundleInfo &bundleInfo, std::string &nativeLibraryPath)
{
    bool loadSoFromDir = bundleInfo.hapModuleInfos.empty();
    std::vector<std::string> nativeFileEntries;
    for (const auto &item: bundleInfo.hapModuleInfos) {
        if (!item.compressNativeLibs) {
            HILOG_DEBUG("handle entries for: %{public}s, with path: %{public}s", item.moduleName.c_str(),
                item.nativeLibraryPath.c_str());
            if (item.nativeLibraryPath.empty()) {
                HILOG_DEBUG("nativeLibraryPath empty: %{public}s", item.moduleName.c_str());
                continue;
            }
            std::string libPath = GetLibPath(item.hapPath, bundleInfo.isPreInstallApp);
            libPath += (libPath.back() == '/') ? item.nativeLibraryPath : "/" + item.nativeLibraryPath;
            HILOG_INFO("module lib path: %{public}s", libPath.c_str());
            if (libPath.back() != '/') {
                libPath.push_back('/');
            }
            for (const auto &entryName : item.nativeLibraryFileNames) {
                HILOG_DEBUG("add entry: %{public}s.", entryName.c_str());
                nativeFileEntries.emplace_back(libPath + entryName);
            }
        } else {
            HILOG_DEBUG("compressNativeLibs flag true for: %{public}s.", item.moduleName.c_str());
            loadSoFromDir = true;
        }
    }

    if (loadSoFromDir) {
        if (nativeLibraryPath.empty()) {
            HILOG_WARN("Native library path is empty.");
            return;
        }

        if (nativeLibraryPath.back() == '/') {
            nativeLibraryPath.pop_back();
        }
        std::string libPath = LOCAL_CODE_PATH;
        libPath += (libPath.back() == '/') ? nativeLibraryPath : "/" + nativeLibraryPath;
        HILOG_DEBUG("native library path = %{public}s", libPath.c_str());

        if (!ScanDir(libPath, nativeFileEntries_)) {
            HILOG_WARN("%{public}s scanDir %{public}s not exits", __func__, libPath.c_str());
        }
    }

    if (!nativeFileEntries.empty()) {
        nativeFileEntries_.insert(nativeFileEntries_.end(), nativeFileEntries.begin(), nativeFileEntries.end());
    }
}

void MainThread::LoadNativeLiabrary(const BundleInfo &bundleInfo, std::string &nativeLibraryPath)
{
    CalcNativeLiabraryEntries(bundleInfo, nativeLibraryPath);
    if (nativeFileEntries_.empty()) {
        HILOG_WARN("No native library");
        return;
    }

    void *handleAbilityLib = nullptr;
    for (auto fileEntry : nativeFileEntries_) {
        if (fileEntry.empty()) {
            continue;
        }
        handleAbilityLib = dlopen(fileEntry.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (handleAbilityLib == nullptr) {
            if (fileEntry.find("libformrender.z.so") == std::string::npos) {
                HILOG_ERROR("%{public}s Fail to dlopen %{public}s, [%{public}s]",
                    __func__, fileEntry.c_str(), dlerror());
                exit(-1);
            } else {
                HILOG_DEBUG("Load libformrender.z.so from native lib path.");
                handleAbilityLib = dlopen(FORM_RENDER_LIB_PATH, RTLD_NOW | RTLD_GLOBAL);
                if (handleAbilityLib == nullptr) {
                    HILOG_ERROR("%{public}s Fail to dlopen %{public}s, [%{public}s]",
                        __func__, FORM_RENDER_LIB_PATH, dlerror());
                    exit(-1);
                }
                fileEntry = FORM_RENDER_LIB_PATH;
            }
        }
        HILOG_DEBUG("%{public}s Success to dlopen %{public}s", __func__, fileEntry.c_str());
        handleAbilityLib_.emplace_back(handleAbilityLib);
    }
}
#endif

void MainThread::ChangeToLocalPath(const std::string &bundleName,
    const std::vector<std::string> &sourceDirs, std::vector<std::string> &localPath)
{
    std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + bundleName
        + std::string(FILE_SEPARATOR));
    for (auto item : sourceDirs) {
        if (item.empty()) {
            continue;
        }
        localPath.emplace_back(
            std::regex_replace(item, pattern, std::string(LOCAL_CODE_PATH) + std::string(FILE_SEPARATOR)));
    }
}

void MainThread::ChangeToLocalPath(const std::string &bundleName,
    const std::string &sourceDir, std::string &localPath)
{
    std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + bundleName);
    if (sourceDir.empty()) {
        return;
    }
    if (std::regex_search(localPath, std::regex(bundleName))) {
        localPath = std::regex_replace(localPath, pattern, std::string(LOCAL_CODE_PATH));
    } else {
        localPath = std::regex_replace(localPath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    }
}

void MainThread::HandleUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    HILOG_DEBUG("MainThread::HandleUpdateApplicationInfoInstalled");
    if (!application_) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    application_->UpdateApplicationInfoInstalled(appInfo);

    if (!appMgr_ || !applicationImpl_) {
        HILOG_ERROR("appMgr_ is nullptr");
        return;
    }
}

void MainThread::HandleAbilityStage(const HapModuleInfo &abilityStage)
{
    HILOG_DEBUG("MainThread::HandleAbilityStageInfo");
    if (!application_) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->AddAbilityStage(abilityStage);

    if (!appMgr_ || !applicationImpl_) {
        HILOG_ERROR("appMgr_ is nullptr");
        return;
    }

    appMgr_->AddAbilityStageDone(applicationImpl_->GetRecordId());
}

void MainThread::LoadAllExtensions(NativeEngine &nativeEngine)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("LoadAllExtensions.");
    if (!extensionConfigMgr_) {
        HILOG_ERROR("ExtensionConfigMgr is invalid");
        return;
    }

    auto extensionPlugins = AbilityRuntime::ExtensionPluginInfo::GetInstance().GetExtensionPlugins();
    if (extensionPlugins.empty()) {
        HILOG_ERROR("no extension type map.");
        return;
    }

    std::map<int32_t, std::string> extensionTypeMap;
    for (auto& item : extensionPlugins) {
        extensionTypeMap.insert(std::pair<int32_t, std::string>(item.extensionType, item.extensionName));
        AddExtensionBlockItem(item.extensionName, item.extensionType);

        std::string file = item.extensionLibFile;
        std::weak_ptr<OHOSApplication> wApp = application_;
        AbilityLoader::GetInstance().RegisterExtension(item.extensionName,
            [wApp, file]() -> AbilityRuntime::Extension* {
            auto app = wApp.lock();
            if (app != nullptr) {
                return AbilityRuntime::ExtensionModuleLoader::GetLoader(file.c_str()).Create(app->GetRuntime());
            }
            HILOG_ERROR("AbilityLoader::GetExtensionByName failed.");
            return nullptr;
        });
    }
    application_->SetExtensionTypeMap(extensionTypeMap);
}

bool MainThread::PrepareAbilityDelegator(const std::shared_ptr<UserTestRecord> &record, bool isStageBased,
    const AppExecFwk::HapModuleInfo &entryHapModuleInfo)
{
    HILOG_DEBUG("enter, isStageBased = %{public}d", isStageBased);
    if (!record) {
        HILOG_ERROR("Invalid UserTestRecord");
        return false;
    }
    auto args = std::make_shared<AbilityDelegatorArgs>(record->want);
    if (isStageBased) { // Stage model
        HILOG_DEBUG("PrepareAbilityDelegator for Stage model.");
        auto testRunner = TestRunner::Create(application_->GetRuntime(), args, false);
        auto delegator = std::make_shared<AbilityDelegator>(
            application_->GetAppContext(), std::move(testRunner), record->observer);
        AbilityDelegatorRegistry::RegisterInstance(delegator, args);
        delegator->Prepare();
    } else { // FA model
        HILOG_DEBUG("PrepareAbilityDelegator for FA model.");
        AbilityRuntime::Runtime::Options options;
        options.codePath = LOCAL_CODE_PATH;
        options.eventRunner = mainHandler_->GetEventRunner();
        options.hapPath = entryHapModuleInfo.hapPath;
        options.loadAce = false;
        options.isStageModel = false;
        options.isTestFramework = true;
        if (applicationInfo_) {
            options.apiTargetVersion = applicationInfo_->apiTargetVersion;
        }
        if (entryHapModuleInfo.abilityInfos.empty()) {
            HILOG_ERROR("Failed to abilityInfos");
            return false;
        }
        bool isFaJsModel = entryHapModuleInfo.abilityInfos.front().srcLanguage == "js" ? true : false;
        static auto runtime = AbilityRuntime::Runtime::Create(options);
        auto testRunner = TestRunner::Create(runtime, args, isFaJsModel);
        if (testRunner == nullptr) {
            HILOG_ERROR("Failed to Create testRunner");
            return false;
        }
        if (!testRunner->Initialize()) {
            HILOG_ERROR("Failed to Initialize testRunner");
            return false;
        }
        auto delegator = std::make_shared<AbilityDelegator>(
            application_->GetAppContext(), std::move(testRunner), record->observer);
        AbilityDelegatorRegistry::RegisterInstance(delegator, args);
        delegator->Prepare();
    }
    return true;
}

/**
 *
 * @brief launch the ability.
 *
 * @param abilityRecord The abilityRecord which belongs to the ability launched.
 *
 */
void MainThread::HandleLaunchAbility(const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    CHECK_POINTER_LOG(abilityRecord, "MainThread::HandleLaunchAbility parameter(abilityRecord) is null");
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector;
    if (abilityRecord->GetWant() != nullptr) {
        traceName += abilityRecord->GetWant()->GetElement().GetBundleName();
    } else {
        HILOG_ERROR("Want is nullptr, cant not get abilityName.");
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, traceName);
    HILOG_DEBUG("HandleLaunchAbility called start.");
    CHECK_POINTER_LOG(applicationImpl_, "MainThread::HandleLaunchAbility applicationImpl_ is null");
    CHECK_POINTER_LOG(abilityRecordMgr_, "MainThread::HandleLaunchAbility abilityRecordMgr_ is null");

    auto abilityToken = abilityRecord->GetToken();
    CHECK_POINTER_LOG(abilityToken, "MainThread::HandleLaunchAbility failed. abilityRecord->GetToken failed");

    abilityRecordMgr_->SetToken(abilityToken);
    abilityRecordMgr_->AddAbilityRecord(abilityToken, abilityRecord);

    if (!IsApplicationReady()) {
        HILOG_ERROR("MainThread::handleLaunchAbility not init OHOSApplication, should launch application first");
        return;
    }

    if (!CheckAbilityItem(abilityRecord)) {
        HILOG_ERROR("MainThread::handleLaunchAbility record is invalid");
        return;
    }

    auto& runtime = application_->GetRuntime();
    auto appInfo = application_->GetApplicationInfo();
    auto want = abilityRecord->GetWant();
    if (appInfo == nullptr) {
        HILOG_ERROR("appInfo is nullptr");
        return;
    }

    if (runtime && want && appInfo->debug) {
        auto perfCmd = want->GetStringParam("perfCmd");
        if (perfCmd.find(PERFCMD_PROFILE) != std::string::npos ||
            perfCmd.find(PERFCMD_DUMPHEAP) != std::string::npos) {
            HILOG_DEBUG("perfCmd is %{public}s", perfCmd.c_str());
            runtime->StartProfiler(perfCmd);
        } else {
            runtime->StartDebugMode(want->GetBoolParam("debugApp", false));
        }
    }

    std::vector<HqfInfo> hqfInfos = appInfo->appQuickFix.deployedAppqfInfo.hqfInfos;
    std::map<std::string, std::string> modulePaths;
    if (runtime && !hqfInfos.empty()) {
        for (auto it = hqfInfos.begin(); it != hqfInfos.end(); it++) {
            HILOG_INFO("moudelName: %{private}s, hqfFilePath: %{private}s.",
                it->moduleName.c_str(), it->hqfFilePath.c_str());
            modulePaths.insert(std::make_pair(it->moduleName, it->hqfFilePath));
        }
        runtime->RegisterQuickFixQueryFunc(modulePaths);
    }

    mainThreadState_ = MainThreadState::RUNNING;
    std::shared_ptr<AbilityRuntime::Context> stageContext = application_->AddAbilityStage(abilityRecord);
    SetProcessExtensionType(abilityRecord);
    UpdateRuntimeModuleChecker(runtime);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    AbilityThread::AbilityThreadMain(application_, abilityRecord, stageContext);
#else
    AbilityThread::AbilityThreadMain(application_, abilityRecord, mainHandler_->GetEventRunner(), stageContext);
#endif
}

/**
 *
 * @brief Clean the ability but don't notify ams.
 *
 * @param token The token which belongs to the ability launched.
 *
 */
void MainThread::HandleCleanAbilityLocal(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("start.");
    if (!IsApplicationReady()) {
        HILOG_ERROR("not init OHOSApplication, should launch application first");
        return;
    }

    if (token == nullptr) {
        HILOG_ERROR("token is null");
        return;
    }

    std::shared_ptr<AbilityLocalRecord> record = abilityRecordMgr_->GetAbilityItem(token);
    if (record == nullptr) {
        HILOG_ERROR("abilityRecord not found");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("record->GetAbilityInfo() failed");
        return;
    }
    HILOG_INFO("ability name: %{public}s", abilityInfo->name.c_str());

    abilityRecordMgr_->RemoveAbilityRecord(token);
    application_->CleanAbilityStage(token, abilityInfo);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    std::shared_ptr<EventRunner> runner = record->GetEventRunner();
    if (runner != nullptr) {
        int ret = runner->Stop();
        if (ret != ERR_OK) {
            HILOG_ERROR("MainThread::main failed. ability runner->Run failed ret = %{public}d", ret);
        }
        abilityRecordMgr_->RemoveAbilityRecord(token);
        application_->CleanAbilityStage(token, abilityInfo);
    } else {
        HILOG_WARN("runner not found");
    }
#endif
}

/**
 *
 * @brief Clean the ability.
 *
 * @param token The token which belongs to the ability launched.
 *
 */
void MainThread::HandleCleanAbility(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (applicationInfo_ == nullptr) {
        HILOG_ERROR("applicationInfo is null");
        return;
    }
    HILOG_DEBUG("Handle clean ability start, app is %{public}s.", applicationInfo_->name.c_str());

    if (!IsApplicationReady()) {
        HILOG_ERROR("not init OHOSApplication, should launch application first");
        return;
    }

    if (token == nullptr) {
        HILOG_ERROR("token is null");
        return;
    }

    std::shared_ptr<AbilityLocalRecord> record = abilityRecordMgr_->GetAbilityItem(token);
    if (record == nullptr) {
        HILOG_ERROR("abilityRecord not found");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("record->GetAbilityInfo() failed");
        return;
    }

#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->type == AbilityType::PAGE && abilityInfo->isStageBasedModel) {
        AppRecovery::GetInstance().RemoveAbility(token);
    }
#endif

    abilityRecordMgr_->RemoveAbilityRecord(token);
    application_->CleanAbilityStage(token, abilityInfo);
#ifdef APP_ABILITY_USE_TWO_RUNNER
    std::shared_ptr<EventRunner> runner = record->GetEventRunner();
    if (runner != nullptr) {
        int ret = runner->Stop();
        if (ret != ERR_OK) {
            HILOG_ERROR("MainThread::main failed. ability runner->Run failed ret = %{public}d", ret);
        }
        abilityRecordMgr_->RemoveAbilityRecord(token);
        application_->CleanAbilityStage(token, abilityInfo);
    } else {
        HILOG_WARN("runner not found");
    }
#endif
    appMgr_->AbilityCleaned(token);
    HILOG_INFO("Handle clean ability end, app: %{public}s, ability: %{public}s.",
        applicationInfo_->name.c_str(), abilityInfo->name.c_str());
}

/**
 *
 * @brief Foreground the application.
 *
 */
void MainThread::HandleForegroundApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread handle application to foreground called.");
    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!");
        return;
    }
#ifdef IMAGE_PURGEABLE_PIXELMAP
    PurgeableMem::PurgeableResourceManager::GetInstance().BeginAccessPurgeableMem();
#endif
    if (!applicationImpl_->PerformForeground()) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!, applicationImpl_->PerformForeground() failed");
        return;
    }

    HILOG_DEBUG("to foreground success, recordId is %{public}d", applicationImpl_->GetRecordId());
    appMgr_->ApplicationForegrounded(applicationImpl_->GetRecordId());
}

/**
 *
 * @brief Background the application.
 *
 */
void MainThread::HandleBackgroundApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread::handleBackgroundApplication called start.");

    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        HILOG_ERROR("MainThread::handleBackgroundApplication error!");
        return;
    }

    if (!applicationImpl_->PerformBackground()) {
        HILOG_ERROR("MainThread::handleForegroundApplication error!, applicationImpl_->PerformBackground() failed");
        return;
    }
    appMgr_->ApplicationBackgrounded(applicationImpl_->GetRecordId());
}

/**
 *
 * @brief Terminate the application.
 *
 */
void MainThread::HandleTerminateApplication()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread::handleTerminateApplication called start.");
    if ((application_ == nullptr) || (appMgr_ == nullptr)) {
        HILOG_ERROR("MainThread::handleTerminateApplication error!");
        return;
    }

    if (!applicationImpl_->PerformTerminate()) {
        HILOG_WARN("%{public}s: applicationImpl_->PerformTerminate() failed.", __func__);
    }

    std::shared_ptr<EventRunner> signalRunner = signalHandler_->GetEventRunner();
    if (signalRunner) {
        signalRunner->Stop();
    }

    std::shared_ptr<EventRunner> runner = mainHandler_->GetEventRunner();
    if (runner == nullptr) {
        HILOG_ERROR("MainThread::handleTerminateApplication get manHandler error");
        return;
    }

    if (watchdog_ != nullptr && !watchdog_->IsStopWatchdog()) {
        watchdog_->Stop();
        watchdog_ = nullptr;
    }

    int ret = runner->Stop();
    if (ret != ERR_OK) {
        HILOG_ERROR("MainThread::handleTerminateApplication failed. runner->Run failed ret = %{public}d", ret);
    }
    SetRunnerStarted(false);
    appMgr_->ApplicationTerminated(applicationImpl_->GetRecordId());
}

/**
 *
 * @brief Shrink the memory which used by application.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void MainThread::HandleShrinkMemory(const int level)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread::HandleShrinkMemory called start.");

    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::HandleShrinkMemory error! applicationImpl_ is null");
        return;
    }

    applicationImpl_->PerformMemoryLevel(level);
}

/**
 *
 * @brief Handle NotifyMemoryLevel.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void MainThread::HandleMemoryLevel(int level)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread::HandleMemoryLevel called start.");

    if (application_ == nullptr) {
        HILOG_ERROR("MainThread::HandleMemoryLevel error! application_ is null");
        return;
    }

    application_->OnMemoryLevel(level);
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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread::HandleConfigurationUpdated called start.");

    if (applicationImpl_ == nullptr) {
        HILOG_ERROR("MainThread::HandleConfigurationUpdated error! applicationImpl_ is null");
        return;
    }

    applicationImpl_->PerformConfigurationUpdated(config);
}

void MainThread::TaskTimeoutDetected(const std::shared_ptr<EventRunner> &runner)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread::TaskTimeoutDetected called start.");

    auto deliveryTimeoutCallback = []() {
        HILOG_DEBUG("MainThread::TaskTimeoutDetected delivery timeout");
    };
    auto distributeTimeoutCallback = []() {
        HILOG_DEBUG("MainThread::TaskTimeoutDetected distribute timeout");
    };

    if (runner !=nullptr && mainHandler_ != nullptr) {
        runner->SetDeliveryTimeout(DELIVERY_TIME);
        mainHandler_->SetDeliveryTimeoutCallback(deliveryTimeoutCallback);

        runner->SetDistributeTimeout(DISTRIBUTE_TIME);
        mainHandler_->SetDistributeTimeoutCallback(distributeTimeoutCallback);
    }
}

void MainThread::Init(const std::shared_ptr<EventRunner> &runner)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread:Init Start");
    mainHandler_ = std::make_shared<MainHandler>(runner, this);
    watchdog_ = std::make_shared<Watchdog>();
    signalHandler_ = std::make_shared<EventHandler>(EventRunner::Create(SIGNAL_HANDLER));
    extensionConfigMgr_ = std::make_unique<AbilityRuntime::ExtensionConfigMgr>();
    wptr<MainThread> weak = this;
    auto task = [weak]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, SetRunnerStarted failed.");
            return;
        }
        appThread->SetRunnerStarted(true);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("MainThread::Init PostTask task failed");
    }
    TaskTimeoutDetected(runner);

    watchdog_->Init(mainHandler_);
    AppExecFwk::AppfreezeInner::GetInstance()->SetMainHandler(mainHandler_);
    extensionConfigMgr_->Init();
}

void MainThread::HandleSignal(int signal)
{
    switch (signal) {
        case SIGNAL_JS_HEAP: {
            auto heapFunc = std::bind(&MainThread::HandleDumpHeap, false);
            signalHandler_->PostTask(heapFunc);
            break;
        }
        case SIGNAL_JS_HEAP_PRIV: {
            auto privateHeapFunc = std::bind(&MainThread::HandleDumpHeap, true);
            signalHandler_->PostTask(privateHeapFunc);
            break;
        }
        default:
            break;
    }
}

void MainThread::HandleDumpHeap(bool isPrivate)
{
    HILOG_DEBUG("Dump heap start.");
    if (mainHandler_ == nullptr) {
        HILOG_ERROR("HandleDumpHeap failed, mainHandler is nullptr");
        return;
    }

    auto task = [isPrivate] {
        auto app = applicationForDump_.lock();
        if (app == nullptr || app->GetRuntime() == nullptr) {
            HILOG_ERROR("runtime is nullptr.");
            return;
        }
        app->GetRuntime()->DumpHeapSnapshot(isPrivate);
    };
    mainHandler_->PostTask(task);
}

void MainThread::Start()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("MainThread start come.");
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        HILOG_ERROR("MainThread::main failed, runner is nullptr");
        return;
    }
    sptr<MainThread> thread = sptr<MainThread>(new (std::nothrow) MainThread());
    if (thread == nullptr) {
        HILOG_ERROR("MainThread::static failed. new MainThread failed");
        return;
    }

    struct sigaction sigAct;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = 0;
    sigAct.sa_handler = &MainThread::HandleSignal;
    sigaction(SIGUSR1, &sigAct, NULL);
    sigaction(SIGNAL_JS_HEAP, &sigAct, NULL);
    sigaction(SIGNAL_JS_HEAP_PRIV, &sigAct, NULL);

    thread->Init(runner);

    thread->Attach();

    int ret = runner->Run();
    if (ret != ERR_OK) {
        HILOG_ERROR("MainThread::main failed. runner->Run failed ret = %{public}d", ret);
    }

    thread->RemoveAppMgrDeathRecipient();
}

void MainThread::PreloadExtensionPlugin()
{
    AbilityRuntime::ExtensionPluginInfo::GetInstance().Preload();
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
    if (eventId == CHECK_MAIN_THREAD_IS_ALIVE) {
        auto mt = mainThreadObj_.promote();
        if (mt != nullptr) {
            mt->CheckMainThreadIsAlive();
        }
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
    HILOG_DEBUG("MainThread::IsApplicationReady called start");
    if (application_ == nullptr || applicationImpl_ == nullptr) {
        HILOG_WARN("MainThread::IsApplicationReady called. application_=null or applicationImpl_=null");
        return false;
    }

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
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef ABILITY_LIBRARY_LOADER
    HILOG_DEBUG("MainThread load ability library start.");
#ifdef ACEABILITY_LIBRARY_LOADER
    void *AceAbilityLib = nullptr;
    AceAbilityLib = dlopen(acelibdir.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (AceAbilityLib == nullptr) {
        HILOG_ERROR("Fail to dlopen %{public}s, [%{public}s]", acelibdir.c_str(), dlerror());
    } else {
        HILOG_DEBUG("Success to dlopen %{public}s", acelibdir.c_str());
        handleAbilityLib_.emplace_back(AceAbilityLib);
    }
#endif  // ACEABILITY_LIBRARY_LOADER
    size_t size = libraryPaths.size();
    for (size_t index = 0; index < size; index++) {
        std::string libraryPath = libraryPaths[index];
        HILOG_DEBUG("Try to scanDir %{public}s", libraryPath.c_str());
        if (!ScanDir(libraryPath, fileEntries_)) {
            HILOG_WARN("scanDir %{public}s not exits", libraryPath.c_str());
        }
        libraryPath = libraryPath + "/libs";
        if (!ScanDir(libraryPath, fileEntries_)) {
            HILOG_WARN("scanDir %{public}s not exits", libraryPath.c_str());
        }
    }

    if (fileEntries_.empty()) {
        HILOG_WARN("No ability library");
        return;
    }

    char resolvedPath[PATH_MAX] = {0};
    void *handleAbilityLib = nullptr;
    for (const auto& fileEntry : fileEntries_) {
        if (fileEntry.empty() || fileEntry.size() >= PATH_MAX) {
            continue;
        }
        if (realpath(fileEntry.c_str(), resolvedPath) == nullptr) {
            HILOG_ERROR("Failed to get realpath, errno = %{public}d", errno);
            continue;
        }

        handleAbilityLib = dlopen(resolvedPath, RTLD_NOW | RTLD_GLOBAL);
        if (handleAbilityLib == nullptr) {
            HILOG_ERROR("Fail to dlopen %{public}s, [%{public}s]",
                resolvedPath, dlerror());
            exit(-1);
        }
        HILOG_INFO("Success to dlopen %{public}s", fileEntry.c_str());
        handleAbilityLib_.emplace_back(handleAbilityLib);
    }
#endif  // ABILITY_LIBRARY_LOADER
}

void MainThread::LoadAppLibrary()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef APPLICATION_LIBRARY_LOADER
    std::string appPath = applicationLibraryPath;
    HILOG_INFO("calling dlopen. appPath=%{public}s", appPath.c_str());
    handleAppLib_ = dlopen(appPath.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (handleAppLib_ == nullptr) {
        HILOG_ERROR("Fail to dlopen %{public}s, [%{public}s]", appPath.c_str(), dlerror());
        exit(-1);
    }
#endif  // APPLICATION_LIBRARY_LOADER
}

void MainThread::LoadAppDetailAbilityLibrary(std::string &nativeLibraryPath)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
#ifdef ABILITY_LIBRARY_LOADER
    HILOG_DEBUG("LoadAppDetailAbilityLibrary try to scanDir %{public}s", nativeLibraryPath.c_str());
    std::vector<std::string> fileEntries;
    if (!ScanDir(nativeLibraryPath, fileEntries)) {
        HILOG_WARN("scanDir %{public}s not exits", nativeLibraryPath.c_str());
    }
    if (fileEntries.empty()) {
        HILOG_WARN("No ability library");
        return;
    }
    char resolvedPath[PATH_MAX] = {0};
    void *handleAbilityLib = nullptr;
    for (const auto& fileEntry : fileEntries) {
        if (fileEntry.empty() || fileEntry.size() >= PATH_MAX) {
            continue;
        }
        if (realpath(fileEntry.c_str(), resolvedPath) == nullptr) {
            HILOG_ERROR("Failed to get realpath, errno = %{public}d", errno);
            continue;
        }

        handleAbilityLib = dlopen(resolvedPath, RTLD_NOW | RTLD_GLOBAL);
        if (handleAbilityLib == nullptr) {
            HILOG_ERROR("Fail to dlopen %{public}s, [%{public}s]",
                resolvedPath, dlerror());
            exit(-1);
        }
        HILOG_INFO("Success to dlopen %{public}s", fileEntry.c_str());
        handleAbilityLib_.emplace_back(handleAbilityLib);
    }
#endif // ABILITY_LIBRARY_LOADER
}

bool MainThread::ScanDir(const std::string &dirPath, std::vector<std::string> &files)
{
    DIR *dirp = opendir(dirPath.c_str());
    if (dirp == nullptr) {
        HILOG_ERROR("MainThread::ScanDir open dir:%{public}s fail", dirPath.c_str());
        return false;
    }
    struct dirent *df = nullptr;
    for (;;) {
        df = readdir(dirp);
        if (df == nullptr) {
            break;
        }

        std::string currentName(df->d_name);
        if (currentName.compare(".") == 0 || currentName.compare("..") == 0) {
            continue;
        }

        if (CheckFileType(currentName, abilityLibraryType_)) {
            files.emplace_back(dirPath + pathSeparator_ + currentName);
        }
    }

    if (closedir(dirp) == -1) {
        HILOG_WARN("close dir fail");
    }
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
    HILOG_DEBUG("MainThread::CheckFileType path is %{public}s, support suffix is %{public}s",
        fileName.c_str(),
        extensionName.c_str());

    if (fileName.empty()) {
        HILOG_ERROR("the file name is empty");
        return false;
    }

    auto position = fileName.rfind('.');
    if (position == std::string::npos) {
        HILOG_WARN("filename no extension name");
        return false;
    }

    std::string suffixStr = fileName.substr(position);
    return LowerStr(suffixStr) == extensionName;
}

void MainThread::HandleScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    HILOG_DEBUG("MainThread::HandleScheduleAcceptWant");
    if (!application_) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    std::string specifiedFlag;
    application_->ScheduleAcceptWant(want, moduleName, specifiedFlag);

    if (!appMgr_ || !applicationImpl_) {
        HILOG_ERROR("appMgr_ is nullptr");
        return;
    }

    appMgr_->ScheduleAcceptWantDone(applicationImpl_->GetRecordId(), want, specifiedFlag);
}

void MainThread::ScheduleAcceptWant(const AAFwk::Want &want, const std::string &moduleName)
{
    HILOG_DEBUG("start");
    wptr<MainThread> weak = this;
    auto task = [weak, want, moduleName]() {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, HandleScheduleAcceptWant failed.");
            return;
        }
        appThread->HandleScheduleAcceptWant(want, moduleName);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("PostTask task failed");
    }
}

void MainThread::CheckMainThreadIsAlive()
{
    watchdog_->SetAppMainThreadState(true);
    watchdog_->AllowReportEvent();
}
#endif  // ABILITY_LIBRARY_LOADER

int32_t MainThread::ScheduleNotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ScheduleNotifyLoadRepairPatch function called.");
    wptr<MainThread> weak = this;
    auto task = [weak, bundleName, callback, recordId]() {
        auto appThread = weak.promote();
        if (appThread == nullptr || appThread->application_ == nullptr || callback == nullptr) {
            HILOG_ERROR("ScheduleNotifyLoadRepairPatch, parameter is nullptr.");
            return;
        }

        bool ret = true;
        std::vector<std::pair<std::string, std::string>> hqfFilePair;
        if (appThread->GetHqfFileAndHapPath(bundleName, hqfFilePair)) {
            for (auto it = hqfFilePair.begin(); it != hqfFilePair.end(); it++) {
                HILOG_INFO("ScheduleNotifyLoadRepairPatch, LoadPatch, hqfFile: %{private}s, hapPath: %{private}s.",
                    it->first.c_str(), it->second.c_str());
                ret = appThread->application_->NotifyLoadRepairPatch(it->first, it->second);
            }
        } else {
            HILOG_DEBUG("ScheduleNotifyLoadRepairPatch, There's no hqfFile need to load.");
        }

        callback->OnLoadPatchDone(ret ? NO_ERROR : ERR_INVALID_OPERATION, recordId);
    };
    if (mainHandler_ == nullptr || !mainHandler_->PostTask(task)) {
        HILOG_ERROR("ScheduleNotifyLoadRepairPatch, Post task failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t MainThread::ScheduleNotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    wptr<MainThread> weak = this;
    auto task = [weak, callback, recordId]() {
        auto appThread = weak.promote();
        if (appThread == nullptr || appThread->application_ == nullptr || callback == nullptr) {
            HILOG_ERROR("parameter is nullptr.");
            return;
        }
        auto ret = appThread->application_->NotifyHotReloadPage();
        callback->OnReloadPageDone(ret ? NO_ERROR : ERR_INVALID_OPERATION, recordId);
    };
    if (mainHandler_ == nullptr || !mainHandler_->PostTask(task)) {
        HILOG_ERROR("Post task failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

bool MainThread::GetHqfFileAndHapPath(const std::string &bundleName,
    std::vector<std::pair<std::string, std::string>> &fileMap)
{
    HILOG_DEBUG("function called.");
    auto bundleObj = DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("Failed to get bundle manager service.");
        return false;
    }

    sptr<IBundleMgr> bundleMgr = iface_cast<IBundleMgr>(bundleObj);
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Bundle manager is nullptr.");
        return false;
    }

    BundleInfo bundleInfo;
    if (bundleMgr->GetBundleInfoForSelf(
        (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo) != ERR_OK) {
        HILOG_ERROR("Get bundle info of %{public}s failed.", bundleName.c_str());
        return false;
    }

    for (auto hapInfo : bundleInfo.hapModuleInfos) {
        if ((processInfo_ != nullptr) && (processInfo_->GetProcessName() == hapInfo.process) &&
            (!hapInfo.hqfInfo.hqfFilePath.empty())) {
            std::string resolvedHapPath(AbilityBase::GetLoadPath(hapInfo.hapPath));
            std::string resolvedHqfFile(AbilityBase::GetLoadPath(hapInfo.hqfInfo.hqfFilePath));
            HILOG_INFO("bundleName: %{public}s, moduleName: %{public}s, processName: %{private}s, "
                "hqf file: %{private}s, hap path: %{private}s.", bundleName.c_str(), hapInfo.moduleName.c_str(),
                hapInfo.process.c_str(), resolvedHqfFile.c_str(), resolvedHapPath.c_str());
            fileMap.push_back(std::pair<std::string, std::string>(resolvedHqfFile, resolvedHapPath));
        }
    }

    return true;
}

int32_t MainThread::ScheduleNotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ScheduleNotifyUnLoadRepairPatch function called.");
    wptr<MainThread> weak = this;
    auto task = [weak, bundleName, callback, recordId]() {
        auto appThread = weak.promote();
        if (appThread == nullptr || appThread->application_ == nullptr || callback == nullptr) {
            HILOG_ERROR("ScheduleNotifyUnLoadRepairPatch, parameter is nullptr.");
            return;
        }

        bool ret = true;
        std::vector<std::pair<std::string, std::string>> hqfFilePair;
        if (appThread->GetHqfFileAndHapPath(bundleName, hqfFilePair)) {
            for (auto it = hqfFilePair.begin(); it != hqfFilePair.end(); it++) {
                HILOG_INFO("ScheduleNotifyUnLoadRepairPatch, UnloadPatch, hqfFile: %{private}s.", it->first.c_str());
                ret = appThread->application_->NotifyUnLoadRepairPatch(it->first);
            }
        } else {
            HILOG_DEBUG("ScheduleNotifyUnLoadRepairPatch, There's no hqfFile need to unload.");
        }

        callback->OnUnloadPatchDone(ret ? NO_ERROR : ERR_INVALID_OPERATION, recordId);
    };
    if (mainHandler_ == nullptr || !mainHandler_->PostTask(task)) {
        HILOG_ERROR("ScheduleNotifyUnLoadRepairPatch, Post task failed.");
        return ERR_INVALID_VALUE;
    }

    return NO_ERROR;
}

int32_t MainThread::ScheduleNotifyAppFault(const FaultData &faultData)
{
    if (mainHandler_ == nullptr) {
        HILOG_ERROR("mainHandler is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (faultData.faultType == FaultDataType::APP_FREEZE) {
        return AppExecFwk::AppfreezeInner::GetInstance()->AppfreezeHandle(faultData, false);
    }

    wptr<MainThread> weak = this;
    auto task = [weak, faultData] {
        auto appThread = weak.promote();
        if (appThread == nullptr) {
            HILOG_ERROR("appThread is nullptr, NotifyAppFault failed.");
            return;
        }
        appThread->NotifyAppFault(faultData);
    };
    mainHandler_->PostTask(task);
    return NO_ERROR;
}

void MainThread::NotifyAppFault(const FaultData &faultData)
{
    if (faultData.notifyApp) {
        ErrorObject faultErrorObj = {
            .name = faultData.errorObject.name,
            .message = faultData.errorObject.message,
            .stack = faultData.errorObject.stack
        };
        ApplicationDataManager::GetInstance().NotifyExceptionObject(faultErrorObj);
    }
}

void MainThread::SetProcessExtensionType(const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    if (!extensionConfigMgr_) {
        HILOG_ERROR("AddExtensionBlockItem failed, extensionConfigMgr_ is null");
        return;
    }
    if (!abilityRecord) {
        HILOG_ERROR("AddExtensionBlockItem failed, abilityRecord is null");
        return;
    }
    if (!abilityRecord->GetAbilityInfo()) {
        HILOG_ERROR("AddExtensionBlockItem failed, abilityInfo is null");
        return;
    }
    HILOG_INFO("SetProcessExtensionType, type = %{public}d",
        static_cast<int32_t>(abilityRecord->GetAbilityInfo()->extensionAbilityType));
    extensionConfigMgr_->SetProcessExtensionType(
        static_cast<int32_t>(abilityRecord->GetAbilityInfo()->extensionAbilityType));
}

void MainThread::AddExtensionBlockItem(const std::string &extensionName, int32_t type)
{
    if (!extensionConfigMgr_) {
        HILOG_ERROR("AddExtensionBlockItem failed, extensionConfigMgr_ is null");
        return;
    }
    extensionConfigMgr_->AddBlockListItem(extensionName, type);
}

void MainThread::UpdateRuntimeModuleChecker(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    if (!extensionConfigMgr_) {
        HILOG_ERROR("UpdateRuntimeModuleChecker failed, extensionConfigMgr_ is null");
        return;
    }
    extensionConfigMgr_->UpdateRuntimeModuleChecker(runtime);
}

int MainThread::GetOverlayModuleInfos(const std::string &bundleName, const std::string &moduleName,
    std::vector<OverlayModuleInfo> &overlayModuleInfos) const
{
    sptr<AppExecFwk::IBundleMgr> bundleMgr = AAFwk::AbilityUtil::GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContextImpl::CreateBundleContext GetBundleManager is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto overlayMgrProxy = bundleMgr->GetOverlayManagerProxy();
    if (overlayMgrProxy == nullptr) {
        HILOG_ERROR("GetOverlayManagerProxy failed.");
        return ERR_INVALID_VALUE;
    }

    auto ret = overlayMgrProxy->GetTargetOverlayModuleInfo(moduleName, overlayModuleInfos);
    if (ret != ERR_OK) {
        HILOG_ERROR("GetOverlayModuleInfo form bms failed.");
        return ret;
    }
    std::sort(overlayModuleInfos.begin(), overlayModuleInfos.end(),
        [](const OverlayModuleInfo& lhs, const OverlayModuleInfo& rhs) -> bool {
        return lhs.priority > rhs.priority;
    });
    HILOG_DEBUG("GetOverlayPath end, the size of overlay is: %{public}zu", overlayModuleInfos.size());
    return ERR_OK;
}

std::vector<std::string> MainThread::GetAddOverlayPaths(const std::vector<OverlayModuleInfo> &overlayModuleInfos)
{
    std::vector<std::string> addPaths;
    for (auto it : overlayModuleInfos) {
        auto iter = std::find_if(
            overlayModuleInfos_.begin(), overlayModuleInfos_.end(), [it](OverlayModuleInfo item) {
                return it.moduleName == item.moduleName;
            });
        if ((iter != overlayModuleInfos_.end()) && (it.state == AppExecFwk::OverlayState::OVERLAY_ENABLE)) {
            iter->state = it.state;
            ChangeToLocalPath(iter->bundleName, iter->hapPath, iter->hapPath);
            HILOG_DEBUG("add path:%{public}s.", iter->hapPath.c_str());
            addPaths.emplace_back(iter->hapPath);
        }
    }
    return addPaths;
}

std::vector<std::string> MainThread::GetRemoveOverlayPaths(const std::vector<OverlayModuleInfo> &overlayModuleInfos)
{
    std::vector<std::string> removePaths;
    for (auto it : overlayModuleInfos) {
        auto iter = std::find_if(
            overlayModuleInfos_.begin(), overlayModuleInfos_.end(), [it](OverlayModuleInfo item) {
                return it.moduleName == item.moduleName;
            });
        if ((iter != overlayModuleInfos_.end()) && (it.state != AppExecFwk::OverlayState::OVERLAY_ENABLE)) {
            iter->state = it.state;
            ChangeToLocalPath(iter->bundleName, iter->hapPath, iter->hapPath);
            HILOG_DEBUG("remove path:%{public}s.", iter->hapPath.c_str());
            removePaths.emplace_back(iter->hapPath);
        }
    }

    return removePaths;
}
}  // namespace AppExecFwk
}  // namespace OHOS
