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

#include <regex>

#include "ability_constants.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "ipc_singleton.h"
#include "js_runtime_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#endif
#include "os_account_manager_wrapper.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"
#include "configuration_convertor.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AbilityRuntime::Constants;

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
const std::string ContextImpl::CONTEXT_DATABASE("database");
const std::string ContextImpl::CONTEXT_TEMP("/temp");
const std::string ContextImpl::CONTEXT_FILES("/files");
const std::string ContextImpl::CONTEXT_HAPS("/haps");
const std::string ContextImpl::CONTEXT_ELS[] = {"el1", "el2"};
Global::Resource::DeviceType ContextImpl::deviceType_ = Global::Resource::DeviceType::DEVICE_NOT_SET;

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
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("ContextImpl::GetBundleCodeDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetCacheDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_CACHE;
    CreateDirIfNotExist(dir);
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
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("ContextImpl::GetDatabaseDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetPreferencesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("ContextImpl::GetPreferencesDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetTempDir()
{
    std::string dir = GetBaseDir() + CONTEXT_TEMP;
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("ContextImpl::GetTempDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILES;
    CreateDirIfNotExist(dir);
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
    CreateDirIfNotExist(dir);
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

    AppExecFwk::BundleInfo bundleInfo;
    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }
    HILOG_DEBUG("ContextImpl::CreateModuleContext length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());
    bundleMgr->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, accountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("ContextImpl::CreateModuleContext GetBundleInfo is error");
        return nullptr;
    }

    std::vector<std::string> moduleResPaths;
    for (auto &info : bundleInfo.hapModuleInfos) {
        if (info.moduleName == moduleName) {
            moduleResPaths.emplace_back(info.resourcePath);
            break;
        }
    }

    if (moduleResPaths.empty()) {
        HILOG_ERROR("ContextImpl::CreateModuleContext hapModuleInfos is error.");
        return nullptr;
    }

    bundleInfo.moduleResPaths.swap(moduleResPaths);
    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->SetConfiguration(config_);
    InitResourceManager(bundleInfo, appContext, GetBundleName() == bundleName, true);
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
    const std::shared_ptr<ContextImpl> &appContext, bool currentBundle, bool moduleContext) const
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    if (appContext == nullptr || resourceManager == nullptr) {
        HILOG_ERROR("ContextImpl::InitResourceManager create resourceManager failed");
        return;
    }

    if (moduleContext || !bundleInfo.applicationInfo.multiProjects) {
        HILOG_DEBUG("ContextImpl::InitResourceManager moduleResPaths count: %{public}zu",
            bundleInfo.moduleResPaths.size());
        std::vector<std::string> moduleResPaths;
        std::string inner(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + GetBundleName());
        std::string outer(ABS_CODE_PATH);
        for (auto item : bundleInfo.moduleResPaths) {
            if (item.empty()) {
                continue;
            }
            if (currentBundle) {
                item.replace(0, inner.size(), LOCAL_CODE_PATH);
            } else {
                item.replace(0, outer.size(), LOCAL_BUNDLES);
            }
            moduleResPaths.emplace_back(item);
        }

        for (auto moduleResPath : moduleResPaths) {
            if (moduleResPath.empty()) {
                continue;
            }

            if (!resourceManager->AddResource(moduleResPath.c_str())) {
                HILOG_ERROR("ContextImpl::InitResourceManager AddResource fail, moduleResPath: %{public}s",
                    moduleResPath.c_str());
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
        HILOG_ERROR("ContextImpl::GetHapModuleInfo, hapModuleInfo is empty");
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

void ContextImpl::CreateDirIfNotExist(const std::string& dirPath) const
{
    HILOG_DEBUG("createDir: create directory if not exists.");
    if (!OHOS::FileExists(dirPath)) {
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("createDir: create dir %{public}s failed.", dirPath.c_str());
            return;
        }
        chmod(dirPath.c_str(), MODE);
    }
}

void ContextImpl::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    config_ = config;
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
}  // namespace AbilityRuntime
}  // namespace OHOS
