/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "context_impl.h"

#include <cerrno>
#include <regex>

#include "app_mgr_client.h"
#include "bundle_mgr_proxy.h"
#include "common_event_manager.h"
#include "configuration_convertor.h"
#include "constants.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "ipc_singleton.h"
#include "js_runtime_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#endif
#include "os_account_manager_wrapper.h"
#include "overlay_event_subscriber.h"
#include "overlay_module_info.h"
#include "parameters.h"
#include "running_process_info.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AbilityBase::Constants;

const std::string PATTERN_VERSION = std::string(FILE_SEPARATOR) + "v\\d+" + FILE_SEPARATOR;

const size_t Context::CONTEXT_TYPE_ID(std::hash<const char*> {} ("Context"));
const int64_t ContextImpl::CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
const mode_t MODE = 0770;
const std::string ContextImpl::CONTEXT_DATA_APP("/data/app/");
const std::string ContextImpl::CONTEXT_BUNDLE("/bundle/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE("/mnt/hmdfs/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE("/device_view/local/data/");
const std::string ContextImpl::CONTEXT_DISTRIBUTEDFILES("distributedfiles");
const std::string ContextImpl::CONTEXT_FILE_SEPARATOR("/");
const std::string ContextImpl::CONTEXT_DATA("/data/");
const std::string ContextImpl::CONTEXT_DATA_STORAGE("/data/storage/");
const std::string ContextImpl::CONTEXT_BASE("base");
const std::string ContextImpl::CONTEXT_CACHE("cache");
const std::string ContextImpl::CONTEXT_PREFERENCES("preferences");
const std::string ContextImpl::CONTEXT_GROUP("group");
const std::string ContextImpl::CONTEXT_DATABASE("database");
const std::string ContextImpl::CONTEXT_TEMP("/temp");
const std::string ContextImpl::CONTEXT_FILES("/files");
const std::string ContextImpl::CONTEXT_HAPS("/haps");
const std::string ContextImpl::CONTEXT_ELS[] = {"el1", "el2"};
Global::Resource::DeviceType ContextImpl::deviceType_ = Global::Resource::DeviceType::DEVICE_NOT_SET;
const std::string OVERLAY_STATE_CHANGED = "usual.event.OVERLAY_STATE_CHANGED";

std::string ContextImpl::GetBundleName() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetBundleName();
    }
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::string ContextImpl::GetBundleCodeDir()
{
    auto appInfo = GetApplicationInfo();
    if (appInfo == nullptr) {
        return "";
    }

    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = std::regex_replace(appInfo->codePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    } else {
        dir = LOCAL_CODE_PATH;
    }
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("ContextImpl::GetBundleCodeDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetCacheDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_CACHE;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("ContextImpl::GetCacheDir:%{public}s", dir.c_str());
    return dir;
}

bool ContextImpl::IsUpdatingConfigurations()
{
    return false;
}

bool ContextImpl::PrintDrawnCompleted()
{
    return false;
}

std::string ContextImpl::GetDatabaseDir()
{
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId())
            + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE;
    }
    if (parentContext_ != nullptr) {
        dir = dir + CONTEXT_FILE_SEPARATOR + ((GetHapModuleInfo() == nullptr) ? "" : GetHapModuleInfo()->moduleName);
    }
    CreateDirIfNotExist(dir, 0);
    HILOG_DEBUG("ContextImpl::GetDatabaseDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetPreferencesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("ContextImpl::GetPreferencesDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetGroupDir(std::string groupId)
{
    std::string dir = "";
    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("GetBundleManager is nullptr");
        return dir;
    }

    std::string groupDir;
    bool ret = bundleMgr->GetGroupDir(groupId, groupDir);
    if (!ret || groupDir.empty()) {
        HILOG_ERROR("GetGroupDir failed or groupDir is empty");
        return dir;
    }
    std::string uuid = groupDir.substr(groupDir.rfind("/"));
    dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_GROUP + uuid;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("GroupDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetTempDir()
{
    std::string dir = GetBaseDir() + CONTEXT_TEMP;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("ContextImpl::GetTempDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILES;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("ContextImpl::GetFilesDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetDistributedFilesDir()
{
    HILOG_DEBUG("ContextImpl::GetDistributedFilesDir");
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE + std::to_string(GetCurrentAccountId()) +
            CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE + GetBundleName();
    } else {
        dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_DISTRIBUTEDFILES;
    }
    CreateDirIfNotExist(dir, 0);
    HILOG_DEBUG("ContextImpl::GetDistributedFilesDir:%{public}s", dir.c_str());
    return dir;
}

void ContextImpl::SwitchArea(int mode)
{
    HILOG_DEBUG("ContextImpl::SwitchArea, mode:%{public}d.", mode);
    if (mode < 0 || mode >= (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0]))) {
        HILOG_ERROR("ContextImpl::SwitchArea, mode is invalid.");
        return;
    }
    currArea_ = CONTEXT_ELS[mode];
    HILOG_DEBUG("ContextImpl::SwitchArea end, currArea:%{public}s.", currArea_.c_str());
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &moduleName)
{
    return CreateModuleContext(GetBundleName(), moduleName);
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &bundleName, const std::string &moduleName)
{
    HILOG_DEBUG("CreateModuleContext begin.");
    if (bundleName.empty()) {
        HILOG_ERROR("ContextImpl::CreateModuleContext bundleName is empty");
        return nullptr;
    }

    if (moduleName.empty()) {
        HILOG_ERROR("ContextImpl::CreateModuleContext moduleName is empty");
        return nullptr;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContextImpl::CreateModuleContext GetBundleManager is nullptr");
        return nullptr;
    }

    HILOG_DEBUG("ContextImpl::CreateModuleContext length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());

    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (bundleName == GetBundleName()) {
        bundleMgr->GetBundleInfoForSelf(
            (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo);
    } else {
        bundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, accountId);
    }

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("ContextImpl::CreateModuleContext GetBundleInfo is error");
        ErrCode ret = bundleMgr->GetDependentBundleInfo(bundleName, bundleInfo);
        if (ret != ERR_OK) {
            HILOG_ERROR("ContextImpl::CreateModuleContext GetDependentBundleInfo failed:%d", ret);
            return nullptr;
        }
    }

    auto info = std::find_if(bundleInfo.hapModuleInfos.begin(), bundleInfo.hapModuleInfos.end(),
        [&moduleName](const AppExecFwk::HapModuleInfo &hapModuleInfo) {
            return hapModuleInfo.moduleName == moduleName;
        });
    if (info == bundleInfo.hapModuleInfos.end()) {
        HILOG_ERROR("ContextImpl::CreateModuleContext moduleName is error.");
        return nullptr;
    }
    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->InitHapModuleInfo(*info);
    appContext->SetConfiguration(config_);
    InitResourceManager(bundleInfo, appContext, GetBundleName() == bundleName, moduleName);
    appContext->SetApplicationInfo(GetApplicationInfo());
    return appContext;
}

int ContextImpl::GetArea()
{
    HILOG_DEBUG("ContextImpl::GetArea begin");
    int mode = -1;
    for (int i = 0; i < (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0])); i++) {
        if (currArea_ == CONTEXT_ELS[i]) {
            mode = i;
            break;
        }
    }
    if (mode == -1) {
        HILOG_ERROR("ContextImpl::GetArea not find mode.");
        return EL_DEFAULT;
    }
    HILOG_DEBUG("ContextImpl::GetArea end");
    return mode;
}

std::string ContextImpl::GetBaseDir() const
{
    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId()) +
            CONTEXT_FILE_SEPARATOR + CONTEXT_BASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        baseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_BASE;
    }
    if (parentContext_ != nullptr) {
        baseDir = baseDir + CONTEXT_HAPS + CONTEXT_FILE_SEPARATOR +
            ((GetHapModuleInfo() == nullptr) ? "" : GetHapModuleInfo()->moduleName);
    }

    HILOG_DEBUG("ContextImpl::GetBaseDir:%{public}s", baseDir.c_str());
    return baseDir;
}

int ContextImpl::GetCurrentAccountId() const
{
    int userId = 0;
    auto instance = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
    if (instance == nullptr) {
        HILOG_ERROR("Failed to get OsAccountManager instance.");
        return userId;
    }
    instance->GetOsAccountLocalIdFromProcess(userId);
    return userId;
}

int ContextImpl::GetCurrentActiveAccountId() const
{
    std::vector<int> accountIds;
    auto instance = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
    if (instance == nullptr) {
        HILOG_ERROR("Failed to get OsAccountManager instance.");
        return 0;
    }
    ErrCode ret = instance->QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK) {
        HILOG_ERROR("ContextImpl::GetCurrentActiveAccountId error.");
        return 0;
    }

    if (accountIds.size() == 0) {
        HILOG_ERROR("ContextImpl::GetCurrentActiveAccountId error, no accounts.");
        return 0;
    }

    if (accountIds.size() > 1) {
        HILOG_ERROR("ContextImpl::GetCurrentActiveAccountId error, no current now.");
        return 0;
    }

    return accountIds[0];
}

std::shared_ptr<Context> ContextImpl::CreateBundleContext(const std::string &bundleName)
{
    HILOG_DEBUG("CreateBundleContext begin.");
    if (parentContext_ != nullptr) {
        return parentContext_->CreateBundleContext(bundleName);
    }

    if (bundleName.empty()) {
        HILOG_ERROR("ContextImpl::CreateBundleContext bundleName is empty");
        return nullptr;
    }

    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContextImpl::CreateBundleContext GetBundleManager is nullptr");
        return nullptr;
    }

    AppExecFwk::BundleInfo bundleInfo;
    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }
    HILOG_DEBUG("ContextImpl::CreateBundleContext length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());
    bundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("ContextImpl::CreateBundleContext GetBundleInfo is error");
        return nullptr;
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    appContext->SetConfiguration(config_);

    // init resourceManager.
    InitResourceManager(bundleInfo, appContext);
    appContext->SetApplicationInfo(GetApplicationInfo());
    return appContext;
}

void ContextImpl::InitResourceManager(const AppExecFwk::BundleInfo &bundleInfo,
    const std::shared_ptr<ContextImpl> &appContext, bool currentBundle, const std::string& moduleName)
{
    HILOG_DEBUG("InitResourceManager begin, bundleName:%{public}s, moduleName:%{public}s",
        bundleInfo.name.c_str(), moduleName.c_str());
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (appContext == nullptr || resourceManager == nullptr) {
        HILOG_ERROR("InitResourceManager create resourceManager failed");
        return;
    }
    if (!moduleName.empty() || !bundleInfo.applicationInfo.multiProjects) {
        HILOG_DEBUG("InitResourceManager hapModuleInfos count: %{public}zu", bundleInfo.hapModuleInfos.size());
        std::regex inner_pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + GetBundleName());
        std::regex outer_pattern(ABS_CODE_PATH);
        std::regex hsp_pattern(std::string(ABS_CODE_PATH) + FILE_SEPARATOR + bundleInfo.name + PATTERN_VERSION);
        std::string hsp_sandbox = std::string(LOCAL_CODE_PATH) + FILE_SEPARATOR + bundleInfo.name + FILE_SEPARATOR;
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            if (!moduleName.empty() && hapModuleInfo.moduleName != moduleName) {
                continue;
            }
            std::string loadPath =  hapModuleInfo.hapPath.empty() ? hapModuleInfo.resourcePath : hapModuleInfo.hapPath;
            if (loadPath.empty()) {
                continue;
            }
            if (currentBundle) {
                loadPath = std::regex_replace(loadPath, inner_pattern, LOCAL_CODE_PATH);
            } else if (bundleInfo.applicationInfo.bundleType == AppExecFwk::BundleType::SHARED) {
                loadPath = std::regex_replace(loadPath, hsp_pattern, hsp_sandbox);
            } else {
                loadPath = std::regex_replace(loadPath, outer_pattern, LOCAL_BUNDLES);
            }

            HILOG_DEBUG("ContextImpl::InitResourceManager loadPath: %{public}s", loadPath.c_str());
            // getOverlayPath
            std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos;
            auto res = GetOverlayModuleInfos(bundleInfo.name, hapModuleInfo.moduleName, overlayModuleInfos);
            if (res != ERR_OK) {
                HILOG_DEBUG("Get overlay paths from bms failed.");
            }
            if (overlayModuleInfos.size() == 0) {
                if (!resourceManager->AddResource(loadPath.c_str())) {
                    HILOG_ERROR("InitResourceManager AddResource fail, moduleResPath: %{public}s", loadPath.c_str());
                }
            } else {
                std::vector<std::string> overlayPaths;
                for (auto it : overlayModuleInfos) {
                    if (std::regex_search(it.hapPath, std::regex(GetBundleName()))) {
                        it.hapPath = std::regex_replace(it.hapPath, inner_pattern, LOCAL_CODE_PATH);
                    } else {
                        it.hapPath = std::regex_replace(it.hapPath, outer_pattern, LOCAL_BUNDLES);
                    }
                    if (it.state == AppExecFwk::OverlayState::OVERLAY_ENABLE) {
                        HILOG_DEBUG("ContextImpl::InitResourceManager hapPath: %{public}s", it.hapPath.c_str());
                        overlayPaths.emplace_back(it.hapPath);
                    }
                }
                HILOG_DEBUG("OverlayPaths size:%{public}zu.", overlayPaths.size());
                if (!resourceManager->AddResource(loadPath, overlayPaths)) {
                    HILOG_ERROR("AddResource failed");
                }

                if (currentBundle) {
                    // add listen overlay change
                    overlayModuleInfos_ = overlayModuleInfos;
                    EventFwk::MatchingSkills matchingSkills;
                    matchingSkills.AddEvent(OVERLAY_STATE_CHANGED);
                    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
                    auto callback = [this, resourceManager, bundleName = bundleInfo.name, moduleName =
                        hapModuleInfo.moduleName, loadPath](const EventFwk::CommonEventData &data) {
                        HILOG_INFO("On overlay changed.");
                        this->OnOverlayChanged(data, resourceManager, bundleName, moduleName, loadPath);
                    };
                    auto subscriber = std::make_shared<AppExecFwk::OverlayEventSubscriber>(subscribeInfo, callback);
                    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber);
                    HILOG_INFO("Overlay event subscriber register result is %{public}d", subResult);
                }
            }
        }
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("ContextImpl::InitResourceManager create ResConfig failed");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    if (resConfig->GetLocaleInfo() != nullptr) {
        HILOG_DEBUG("ContextImpl::InitResourceManager language: %{public}s, script: %{public}s, region: %{public}s,",
            resConfig->GetLocaleInfo()->getLanguage(), resConfig->GetLocaleInfo()->getScript(),
            resConfig->GetLocaleInfo()->getCountry());
    } else {
        HILOG_ERROR("ContextImpl::InitResourceManager language: GetLocaleInfo is null.");
    }
#endif
    resConfig->SetDeviceType(GetDeviceType());
    resourceManager->UpdateResConfig(*resConfig);
    appContext->SetResourceManager(resourceManager);
}

sptr<AppExecFwk::IBundleMgr> ContextImpl::GetBundleManager() const
{
    HILOG_DEBUG("ContextImpl::GetBundleManager");
    auto instance = OHOS::DelayedSingleton<AppExecFwk::SysMrgClient>::GetInstance();
    if (instance == nullptr) {
        HILOG_ERROR("failed to get SysMrgClient instance");
        return nullptr;
    }
    auto bundleObj = instance->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bms = iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
    return bms;
}

void ContextImpl::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    if (info == nullptr) {
        HILOG_ERROR("ContextImpl::SetApplicationInfo failed, info is empty");
        return;
    }
    applicationInfo_ = info;
}

void ContextImpl::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    HILOG_DEBUG("ContextImpl::initResourceManager. Start.");
    resourceManager_ = resourceManager;
    HILOG_DEBUG("ContextImpl::initResourceManager. End.");
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::GetResourceManager() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetResourceManager();
    }

    return resourceManager_;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ContextImpl::GetApplicationInfo() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetApplicationInfo();
    }

    return applicationInfo_;
}

void ContextImpl::SetParentContext(const std::shared_ptr<Context> &context)
{
    parentContext_ = context;
}

std::string ContextImpl::GetBundleCodePath() const
{
    if (parentContext_ != nullptr) {
        return parentContext_->GetBundleCodePath();
    }
    return (applicationInfo_ != nullptr) ? applicationInfo_->codePath : "";
}

void ContextImpl::InitHapModuleInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (hapModuleInfo_ != nullptr || abilityInfo == nullptr) {
        return;
    }
    sptr<AppExecFwk::IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        HILOG_ERROR("InitHapModuleInfo: failed to get bundle manager service");
        return;
    }

    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    if (!ptr->GetHapModuleInfo(*abilityInfo.get(), *hapModuleInfo_)) {
        HILOG_ERROR("InitHapModuleInfo: GetHapModuleInfo failed, will retval false value");
    }
}

void ContextImpl::InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>(hapModuleInfo);
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ContextImpl::GetHapModuleInfo() const
{
    if (hapModuleInfo_ == nullptr) {
        HILOG_DEBUG("ContextImpl::GetHapModuleInfo, hapModuleInfo is empty");
    }
    return hapModuleInfo_;
}

void ContextImpl::SetFlags(int64_t flags)
{
    flags_ = static_cast<uint64_t>(flags_) | static_cast<uint64_t>(CONTEXT_CREATE_BY_SYSTEM_APP);
}

bool ContextImpl::IsCreateBySystemApp() const
{
    return (static_cast<uint64_t>(flags_) & static_cast<uint64_t>(CONTEXT_CREATE_BY_SYSTEM_APP)) == 1;
}

std::shared_ptr<ApplicationContext> Context::applicationContext_ = nullptr;
std::mutex Context::contextMutex_;

std::shared_ptr<ApplicationContext> Context::GetApplicationContext()
{
    std::lock_guard<std::mutex> lock(contextMutex_);
    return applicationContext_;
}

void ContextImpl::SetToken(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        HILOG_DEBUG("ContextImpl::SetToken failed, application is nullptr");
        return;
    }
    token_ = token;
}

sptr<IRemoteObject> ContextImpl::GetToken()
{
    return token_;
}

void ContextImpl::CreateDirIfNotExist(const std::string& dirPath, const mode_t& mode) const
{
    HILOG_DEBUG("createDir: create directory if not exists.");
    if (!OHOS::FileExists(dirPath)) {
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("createDir: create dir %{public}s failed, errno is %{public}d.", dirPath.c_str(), errno);
            return;
        }
        if (mode != 0) {
            chmod(dirPath.c_str(), mode);
        }
    }
}

void ContextImpl::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    config_ = config;
}

void ContextImpl::KillProcessBySelf()
{
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    appMgrClient->KillApplicationSelf();
}

int32_t ContextImpl::GetProcessRunningInformation(AppExecFwk::RunningProcessInfo &info)
{
    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    auto result = appMgrClient->GetProcessRunningInformation(info);
    HILOG_DEBUG("result is %{public}d.", result);
    return result;
}

std::shared_ptr<AppExecFwk::Configuration> ContextImpl::GetConfiguration() const
{
    return config_;
}

Global::Resource::DeviceType ContextImpl::GetDeviceType() const
{
    if (deviceType_ != Global::Resource::DeviceType::DEVICE_NOT_SET) {
        return deviceType_;
    }

    auto config = GetConfiguration();
    if (config != nullptr) {
        auto deviceType = config->GetItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE);
        HILOG_INFO("deviceType is %{public}s.", deviceType.c_str());
        deviceType_ = AppExecFwk::ConvertDeviceType(deviceType);
    }

    if (deviceType_ == Global::Resource::DeviceType::DEVICE_NOT_SET) {
        deviceType_ = Global::Resource::DeviceType::DEVICE_PHONE;
    }
    HILOG_DEBUG("deviceType is %{public}d.", deviceType_);
    return deviceType_;
}

int ContextImpl::GetOverlayModuleInfos(const std::string &bundleName, const std::string &moduleName,
    std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos)
{
    sptr<AppExecFwk::IBundleMgr> bundleMgr = GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("ContextImpl::CreateBundleContext GetBundleManager is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto overlayMgrProxy = bundleMgr->GetOverlayManagerProxy();
    if (overlayMgrProxy ==  nullptr) {
        HILOG_ERROR("GetOverlayManagerProxy failed.");
        return ERR_INVALID_VALUE;
    }

    auto ret = overlayMgrProxy->GetTargetOverlayModuleInfo(moduleName, overlayModuleInfos);
    if (ret != ERR_OK) {
        HILOG_DEBUG("GetOverlayModuleInfo form bms failed.");
        return ret;
    }
    std::sort(overlayModuleInfos.begin(), overlayModuleInfos.end(),
        [](const AppExecFwk::OverlayModuleInfo& lhs, const AppExecFwk::OverlayModuleInfo& rhs) -> bool {
        return lhs.priority > rhs.priority;
    });
    HILOG_DEBUG("GetOverlayPath end, the size of overlay is: %{public}zu", overlayModuleInfos.size());
    return ERR_OK;
}

std::vector<std::string> ContextImpl::GetAddOverlayPaths(
    const std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos)
{
    std::vector<std::string> addPaths;
    for (auto it : overlayModuleInfos) {
        auto iter = std::find_if(
            overlayModuleInfos_.begin(), overlayModuleInfos_.end(), [it](AppExecFwk::OverlayModuleInfo item) {
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

std::vector<std::string> ContextImpl::GetRemoveOverlayPaths(
    const std::vector<AppExecFwk::OverlayModuleInfo> &overlayModuleInfos)
{
    std::vector<std::string> removePaths;
    for (auto it : overlayModuleInfos) {
        auto iter = std::find_if(
            overlayModuleInfos_.begin(), overlayModuleInfos_.end(), [it](AppExecFwk::OverlayModuleInfo item) {
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

void ContextImpl::OnOverlayChanged(const EventFwk::CommonEventData &data,
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
    const std::string &moduleName, const std::string &loadPath)
{
    HILOG_DEBUG("OnOverlayChanged begin.");
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action != OVERLAY_STATE_CHANGED) {
        HILOG_DEBUG("Not this subscribe, action: %{public}s.", action.c_str());
        return;
    }
    if (GetBundleName() != bundleName) {
        HILOG_DEBUG("Not this app, bundleName: %{public}s.", bundleName.c_str());
        return;
    }
    bool isEnable = data.GetWant().GetBoolParam(AppExecFwk::Constants::OVERLAY_STATE, false);
    // 1.get overlay hapPath
    if (resourceManager == nullptr) {
        HILOG_ERROR("resourceManager is nullptr.");
        return;
    }
    if (overlayModuleInfos_.size() == 0) {
        HILOG_ERROR("overlayModuleInfos is empty.");
        return;
    }
    std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos;
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

void ContextImpl::ChangeToLocalPath(const std::string &bundleName,
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
}  // namespace AbilityRuntime
}  // namespace OHOS
