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

#include "context_impl.h"

#include <cerrno>
#include <regex>

#include "app_mgr_client.h"
#include "application_context.h"
#include "bundle_mgr_helper.h"
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
const mode_t GROUP_MODE = 02770;
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
const std::string ContextImpl::CONTEXT_ELS[] = {"el1", "el2", "el3", "el4"};
const std::string ContextImpl::CONTEXT_RESOURCE_END = "/resources/resfile";
Global::Resource::DeviceType ContextImpl::deviceType_ = Global::Resource::DeviceType::DEVICE_NOT_SET;
const std::string OVERLAY_STATE_CHANGED = "usual.event.OVERLAY_STATE_CHANGED";
const int32_t TYPE_RESERVE = 1;
const int32_t TYPE_OTHERS = 2;
const int32_t API11 = 11;
const int32_t API_VERSION_MOD = 100;
const int AREA2 = 2;
const int AREA3 = 3;

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
    HILOG_DEBUG("dir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetCacheDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_CACHE;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("dir:%{public}s", dir.c_str());
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

void ContextImpl::CreateDirIfNotExistWithCheck(const std::string &dirPath, const mode_t &mode, bool checkExist)
{
    if (checkExist) {
        CreateDirIfNotExist(dirPath, mode);
        return;
    }
    // Check if the dirPath exists on the first call
    std::lock_guard<std::mutex> lock(checkedDirSetLock_);
    if (checkedDirSet_.find(dirPath) != checkedDirSet_.end()) {
        return;
    }
    checkedDirSet_.emplace(dirPath);
    CreateDirIfNotExist(dirPath, mode);
}

int32_t ContextImpl::GetDatabaseDirWithCheck(bool checkExist, std::string &databaseDir)
{
    if (IsCreateBySystemApp()) {
        databaseDir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId())
                      + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        databaseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE;
    }
    if (parentContext_ != nullptr) {
        databaseDir = databaseDir + CONTEXT_FILE_SEPARATOR +
                      ((GetHapModuleInfo() == nullptr) ? "" : GetHapModuleInfo()->moduleName);
    }
    CreateDirIfNotExistWithCheck(databaseDir, 0, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetGroupDatabaseDirWithCheck(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    int32_t ret = GetGroupDirWithCheck(groupId, checkExist, databaseDir);
    if (ret != ERR_OK) {
        return ret;
    }
    databaseDir = databaseDir + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE;
    CreateDirIfNotExistWithCheck(databaseDir, GROUP_MODE, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    int32_t ret;
    if (groupId.empty()) {
        ret = GetDatabaseDirWithCheck(checkExist, databaseDir);
    } else {
        ret = GetGroupDatabaseDirWithCheck(groupId, checkExist, databaseDir);
    }
    HILOG_DEBUG("databaseDir: %{public}s", databaseDir.c_str());
    return ret;
}

std::string ContextImpl::GetDatabaseDir()
{
    std::string dir;
    GetDatabaseDirWithCheck(true, dir);
    HILOG_DEBUG("databaseDir: %{public}s", dir.c_str());
    return dir;
}

int32_t ContextImpl::GetPreferencesDirWithCheck(bool checkExist, std::string &preferencesDir)
{
    preferencesDir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
    CreateDirIfNotExistWithCheck(preferencesDir, MODE, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetGroupPreferencesDirWithCheck(const std::string &groupId, bool checkExist,
    std::string &preferencesDir)
{
    int32_t ret = GetGroupDirWithCheck(groupId, checkExist, preferencesDir);
    if (ret != ERR_OK) {
        return ret;
    }
    preferencesDir = preferencesDir + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
    CreateDirIfNotExistWithCheck(preferencesDir, GROUP_MODE, checkExist);
    return ERR_OK;
}

int32_t ContextImpl::GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir)
{
    int32_t ret;
    if (groupId.empty()) {
        ret = GetPreferencesDirWithCheck(checkExist, preferencesDir);
    } else {
        ret = GetGroupPreferencesDirWithCheck(groupId, checkExist, preferencesDir);
    }
    HILOG_DEBUG("preferencesDir: %{public}s", preferencesDir.c_str());
    return ret;
}

std::string ContextImpl::GetPreferencesDir()
{
    std::string dir;
    GetPreferencesDirWithCheck(true, dir);
    HILOG_DEBUG("preferencesDir: %{public}s", dir.c_str());
    return dir;
}

int32_t ContextImpl::GetGroupDirWithCheck(const std::string &groupId, bool checkExist, std::string &groupDir)
{
    if (currArea_ == CONTEXT_ELS[0]) {
        HILOG_ERROR("GroupDir currently can't supports the el1 level");
        return ERR_INVALID_VALUE;
    }
    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        HILOG_ERROR("failed, errCode: %{public}d.", errCode);
        return errCode;
    }
    std::string groupDirGet;
    bool ret = bundleMgr_->GetGroupDir(groupId, groupDirGet);
    if (!ret || groupDirGet.empty()) {
        HILOG_ERROR("GetGroupDir failed or groupDirGet is empty");
        return ERR_INVALID_VALUE;
    }
    std::string uuid = groupDirGet.substr(groupDirGet.rfind('/'));
    groupDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_GROUP + uuid;
    CreateDirIfNotExistWithCheck(groupDir, MODE, true);
    return ERR_OK;
}

std::string ContextImpl::GetGroupDir(std::string groupId)
{
    std::string dir;
    GetGroupDirWithCheck(groupId, true, dir);
    HILOG_DEBUG("GroupDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetTempDir()
{
    std::string dir = GetBaseDir() + CONTEXT_TEMP;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("dir:%{public}s", dir.c_str());
    return dir;
}

void ContextImpl::GetAllTempDir(std::vector<std::string> &tempPaths)
{
    // Application temp dir
    auto appTemp = GetTempDir();
    if (OHOS::FileExists(appTemp)) {
        tempPaths.push_back(appTemp);
    }
    // Module dir
    if (applicationInfo_ == nullptr) {
        HILOG_ERROR("The application info is empty");
        return;
    }

    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DATA_APP + currArea_ + CONTEXT_FILE_SEPARATOR + std::to_string(GetCurrentAccountId()) +
            CONTEXT_FILE_SEPARATOR + CONTEXT_BASE + CONTEXT_FILE_SEPARATOR + GetBundleName();
    } else {
        baseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_BASE;
    }
    for (const auto &moudleItem: applicationInfo_->moduleInfos) {
        auto moudleTemp = baseDir + CONTEXT_HAPS + CONTEXT_FILE_SEPARATOR + moudleItem.moduleName + CONTEXT_TEMP;
        if (!OHOS::FileExists(moudleTemp)) {
            HILOG_WARN("moudle[%{public}s] temp path not exists,path is %{public}s",
                moudleItem.moduleName.c_str(), moudleTemp.c_str());
            continue;
        }
        tempPaths.push_back(moudleTemp);
    }
}

std::string ContextImpl::GetResourceDir()
{
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfoPtr = GetHapModuleInfo();
    if (hapModuleInfoPtr == nullptr || hapModuleInfoPtr->moduleName.empty()) {
        return "";
    }
    std::string dir = std::string(LOCAL_CODE_PATH) + CONTEXT_FILE_SEPARATOR +
        hapModuleInfoPtr->moduleName + CONTEXT_RESOURCE_END;
    if (OHOS::FileExists(dir)) {
        return dir;
    }
    return "";
}

std::string ContextImpl::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_FILES;
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("dir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextImpl::GetDistributedFilesDir()
{
    HILOG_DEBUG("called");
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE + std::to_string(GetCurrentAccountId()) +
            CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE + GetBundleName();
    } else {
        if (currArea_ == CONTEXT_ELS[1] || currArea_ == CONTEXT_ELS[AREA2] || currArea_ == CONTEXT_ELS[AREA3]) {
            //when areamode swith to el3/el4, the distributedfiles dir should be always el2's distributedfilesdir dir
            dir = CONTEXT_DATA_STORAGE + CONTEXT_ELS[1] + CONTEXT_FILE_SEPARATOR + CONTEXT_DISTRIBUTEDFILES;
        } else {
            dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_DISTRIBUTEDFILES;
        }
    }
    CreateDirIfNotExist(dir, 0);
    HILOG_DEBUG("dir:%{public}s", dir.c_str());
    return dir;
}

void ContextImpl::SwitchArea(int mode)
{
    HILOG_DEBUG("mode:%{public}d.", mode);
    if (mode < 0 || mode >= (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0]))) {
        HILOG_ERROR("ContextImpl::SwitchArea, mode is invalid.");
        return;
    }
    currArea_ = CONTEXT_ELS[mode];
    HILOG_DEBUG("currArea:%{public}s.", currArea_.c_str());
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &moduleName)
{
    return CreateModuleContext(GetBundleName(), moduleName);
}

std::shared_ptr<Context> ContextImpl::CreateModuleContext(const std::string &bundleName, const std::string &moduleName)
{
    HILOG_DEBUG("begin.");
    if (bundleName.empty() || moduleName.empty()) {
        HILOG_ERROR("bundleName or moduleName is empty");
        return nullptr;
    }

    HILOG_DEBUG("length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());

    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }

    AppExecFwk::BundleInfo bundleInfo;
    GetBundleInfo(bundleName, bundleInfo, accountId);
    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("GetBundleInfo is error");
        ErrCode ret = bundleMgr_->GetDependentBundleInfo(bundleName, bundleInfo,
            AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
        if (ret != ERR_OK) {
            HILOG_ERROR("GetDependentBundleInfo failed:%{public}d", ret);
            return nullptr;
        }
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    if (bundleInfo.applicationInfo.codePath != std::to_string(TYPE_RESERVE) &&
        bundleInfo.applicationInfo.codePath != std::to_string(TYPE_OTHERS)) {
        auto info = std::find_if(bundleInfo.hapModuleInfos.begin(), bundleInfo.hapModuleInfos.end(),
            [&moduleName](const AppExecFwk::HapModuleInfo &hapModuleInfo) {
                return hapModuleInfo.moduleName == moduleName;
            });
        if (info == bundleInfo.hapModuleInfos.end()) {
            HILOG_ERROR("moduleName is error.");
            return nullptr;
        }
        appContext->InitHapModuleInfo(*info);
    }

    appContext->SetConfiguration(config_);
    InitResourceManager(bundleInfo, appContext, GetBundleName() == bundleName, moduleName);
    appContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    return appContext;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::CreateModuleResourceManager(
    const std::string &bundleName, const std::string &moduleName)
{
    HILOG_DEBUG("begin, bundleName: %{public}s, moduleName: %{public}s", bundleName.c_str(), moduleName.c_str());
    if (bundleName.empty() || moduleName.empty()) {
        HILOG_ERROR("bundleName is %{public}s, moduleName is %{public}s", bundleName.c_str(), moduleName.c_str());
        return nullptr;
    }

    AppExecFwk::BundleInfo bundleInfo;
    bool currentBundle = false;
    if (GetBundleInfo(bundleName, bundleInfo, currentBundle) != ERR_OK) {
        HILOG_ERROR("Failed to GetBundleInfo, bundleName: %{public}s", bundleName.c_str());
        return nullptr;
    }

    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitOthersResourceManagerInner(
            bundleInfo, currentBundle, moduleName);
        if (resourceManager == nullptr) {
            HILOG_ERROR("InitOthersResourceManagerInner create resourceManager failed");
        }
        return resourceManager;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitResourceManagerInner(
        bundleInfo, currentBundle, moduleName);
    if (resourceManager == nullptr) {
        HILOG_ERROR("InitResourceManagerInner create resourceManager failed");
        return nullptr;
    }
    UpdateResConfig(resourceManager);
    return resourceManager;
}

int32_t ContextImpl::GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo,
    bool &currentBundle)
{
    std::string currentBundleName;
    auto appContext = ApplicationContext::GetInstance();
    if (appContext != nullptr) {
        currentBundleName = appContext->GetBundleName();
    }
    currentBundle = bundleName == currentBundleName;

    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        HILOG_ERROR("errCode: %{public}d.", errCode);
        return errCode;
    }

    if (currentBundle) {
        bundleMgr_->GetBundleInfoForSelf((
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo);
    } else {
        int accountId = GetCurrentAccountId();
        if (accountId == 0) {
            accountId = GetCurrentActiveAccountId();
        }
        bundleMgr_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);
    }

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_WARN("bundleInfo is empty");
        ErrCode ret = bundleMgr_->GetUninstalledBundleInfo(bundleName, bundleInfo);
        if (ret != ERR_OK) {
            HILOG_ERROR("GetUninstalledBundleInfo failed:%{public}d", ret);
            return ret;
        }
    }
    return ERR_OK;
}

void ContextImpl::GetBundleInfo(const std::string &bundleName, AppExecFwk::BundleInfo &bundleInfo,
    const int &accountId)
{
    HILOG_DEBUG("begin");
    if (bundleMgr_ == nullptr) {
        int errCode = GetBundleManager();
        if (errCode != ERR_OK) {
            HILOG_ERROR("failed, errCode: %{public}d.", errCode);
            return;
        }
    }

    if (bundleName == GetBundleName()) {
        bundleMgr_->GetBundleInfoForSelf(
            (static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_DISABLE) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) +
            static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_METADATA)), bundleInfo);
    } else {
        bundleMgr_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);
    }
}

int ContextImpl::GetArea()
{
    HILOG_DEBUG("begin");
    int mode = -1;
    for (int i = 0; i < (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0])); i++) {
        if (currArea_ == CONTEXT_ELS[i]) {
            mode = i;
            break;
        }
    }
    if (mode == -1) {
        HILOG_ERROR("not find mode.");
        return EL_DEFAULT;
    }
    HILOG_DEBUG("end");
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

    HILOG_DEBUG("Dir:%{public}s", baseDir.c_str());
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
        HILOG_ERROR("no accounts.");
        return 0;
    }

    if (accountIds.size() > 1) {
        HILOG_ERROR("no current now.");
        return 0;
    }

    return accountIds[0];
}

std::shared_ptr<Context> ContextImpl::CreateBundleContext(const std::string &bundleName)
{
    HILOG_DEBUG("begin.");
    if (parentContext_ != nullptr) {
        return parentContext_->CreateBundleContext(bundleName);
    }

    if (bundleName.empty()) {
        HILOG_ERROR("bundleName is empty");
        return nullptr;
    }

    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        HILOG_ERROR("failed, errCode: %{public}d.", errCode);
        return nullptr;
    }

    AppExecFwk::BundleInfo bundleInfo;
    int accountId = GetCurrentAccountId();
    if (accountId == 0) {
        accountId = GetCurrentActiveAccountId();
    }
    HILOG_DEBUG("length: %{public}zu, bundleName: %{public}s",
        (size_t)bundleName.length(), bundleName.c_str());
    bundleMgr_->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, accountId);

    if (bundleInfo.name.empty() || bundleInfo.applicationInfo.name.empty()) {
        HILOG_ERROR("bundleInfo is empty");
        return nullptr;
    }

    std::shared_ptr<ContextImpl> appContext = std::make_shared<ContextImpl>();
    appContext->SetFlags(CONTEXT_CREATE_BY_SYSTEM_APP);
    appContext->SetConfiguration(config_);

    // init resourceManager.
    InitResourceManager(bundleInfo, appContext);
    appContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    return appContext;
}

void ContextImpl::InitResourceManager(const AppExecFwk::BundleInfo &bundleInfo,
    const std::shared_ptr<ContextImpl> &appContext, bool currentBundle, const std::string& moduleName)
{
    HILOG_DEBUG("begin, bundleName:%{public}s, moduleName:%{public}s",
        bundleInfo.name.c_str(), moduleName.c_str());

    if (appContext == nullptr) {
        HILOG_ERROR("InitResourceManager appContext is nullptr");
        return;
    }

    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE) ||
        bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitOthersResourceManagerInner(
            bundleInfo, currentBundle, moduleName);
        if (resourceManager == nullptr) {
            HILOG_ERROR("create resourceManager failed");
            return;
        }
        appContext->SetResourceManager(resourceManager);
        return;
    }

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitResourceManagerInner(
        bundleInfo, currentBundle, moduleName);
    if (resourceManager == nullptr) {
        return;
    }
    UpdateResConfig(resourceManager);
    appContext->SetResourceManager(resourceManager);
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::InitOthersResourceManagerInner(
    const AppExecFwk::BundleInfo &bundleInfo, bool currentBundle, const std::string& moduleName)
{
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    std::string hapPath;
    std::vector<std::string> overlayPaths;
    int32_t appType;
    if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_RESERVE)) {
        appType = TYPE_RESERVE;
    } else if (bundleInfo.applicationInfo.codePath == std::to_string(TYPE_OTHERS)) {
        appType = TYPE_OTHERS;
    } else {
        appType = 0;
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager(
        bundleInfo.name, moduleName, hapPath, overlayPaths, *resConfig, appType));
    return resourceManager;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::InitResourceManagerInner(
    const AppExecFwk::BundleInfo &bundleInfo, bool currentBundle, const std::string& moduleName)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = InitOthersResourceManagerInner(
        bundleInfo, currentBundle, moduleName);
    if (resourceManager == nullptr) {
        HILOG_ERROR("Create resourceManager failed");
        return resourceManager;
    }
    if (!moduleName.empty() || !bundleInfo.applicationInfo.multiProjects) {
        HILOG_DEBUG("hapModuleInfos count: %{public}zu", bundleInfo.hapModuleInfos.size());
        std::regex inner_pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + GetBundleName());
        std::regex outer_pattern(ABS_CODE_PATH);
        std::regex hsp_pattern(std::string(ABS_CODE_PATH) + FILE_SEPARATOR + bundleInfo.name + PATTERN_VERSION);
        std::string hsp_sandbox = std::string(LOCAL_CODE_PATH) + FILE_SEPARATOR + bundleInfo.name + FILE_SEPARATOR;
        for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
            HILOG_DEBUG("hapModuleInfo abilityInfo size: %{public}zu", hapModuleInfo.abilityInfos.size());
            if (!moduleName.empty() && hapModuleInfo.moduleName != moduleName) {
                continue;
            }
            std::string loadPath =  hapModuleInfo.hapPath.empty() ? hapModuleInfo.resourcePath : hapModuleInfo.hapPath;
            if (loadPath.empty()) {
                HILOG_DEBUG("loadPath is empty");
                continue;
            }
            if (currentBundle) {
                loadPath = std::regex_replace(loadPath, inner_pattern, LOCAL_CODE_PATH);
            } else if (bundleInfo.applicationInfo.bundleType == AppExecFwk::BundleType::SHARED) {
                loadPath = std::regex_replace(loadPath, hsp_pattern, hsp_sandbox);
            } else if (bundleInfo.applicationInfo.bundleType == AppExecFwk::BundleType::APP_SERVICE_FWK) {
                HILOG_DEBUG("System hsp path, not need translate.");
            } else {
                loadPath = std::regex_replace(loadPath, outer_pattern, LOCAL_BUNDLES);
            }

            HILOG_DEBUG("loadPath: %{public}s", loadPath.c_str());
            // getOverlayPath
            std::vector<AppExecFwk::OverlayModuleInfo> overlayModuleInfos;
            auto res = GetOverlayModuleInfos(bundleInfo.name, hapModuleInfo.moduleName, overlayModuleInfos);
            if (res != ERR_OK) {
                HILOG_DEBUG("Get overlay paths from bms failed.");
            }
            if (overlayModuleInfos.size() == 0) {
                if (!resourceManager->AddResource(loadPath.c_str())) {
                    HILOG_ERROR("AddResource fail, moduleResPath: %{public}s", loadPath.c_str());
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
                        HILOG_DEBUG("hapPath: %{public}s", it.hapPath.c_str());
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
                    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
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
    return resourceManager;
}

void ContextImpl::UpdateResConfig(std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("create ResConfig failed");
        return;
    }

    if (GetHapModuleInfo() != nullptr && GetApplicationInfo() != nullptr) {
        std::vector<AppExecFwk::Metadata> metadata = GetHapModuleInfo()->metadata;
        bool load = std::any_of(metadata.begin(), metadata.end(), [](const auto &metadataItem) {
            return metadataItem.name == "ContextResourceConfigLoadFromParentTemp" && metadataItem.value == "true";
        });
        if (load && GetApplicationInfo()->apiTargetVersion % API_VERSION_MOD >= API11) {
            std::shared_ptr<Global::Resource::ResourceManager> currentResMgr = GetResourceManager();
            if (currentResMgr != nullptr) {
                HILOG_DEBUG("apiVersion: %{public}d, load parent config.", GetApplicationInfo()->apiTargetVersion);
                currentResMgr->GetResConfig(*resConfig);
            }
        }
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
}

ErrCode ContextImpl::GetBundleManager()
{
    std::lock_guard<std::mutex> lock(bundleManagerMutex_);
    if (bundleMgr_ != nullptr && !resetFlag_) {
        return ERR_OK;
    }

    bundleMgr_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("The bundleMgr_ is nullptr.");
        return ERR_NULL_OBJECT;
    }

    HILOG_DEBUG("Success.");
    return ERR_OK;
}

void ContextImpl::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    if (info == nullptr) {
        HILOG_ERROR("info is empty");
        return;
    }
    applicationInfo_ = info;
}

void ContextImpl::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    HILOG_DEBUG("Start.");
    resourceManager_ = resourceManager;
    HILOG_DEBUG("End.");
}

std::shared_ptr<Global::Resource::ResourceManager> ContextImpl::GetResourceManager() const
{
    if (resourceManager_) {
        return resourceManager_;
    }

    return parentContext_ != nullptr ? parentContext_->GetResourceManager() : nullptr;
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
    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        HILOG_ERROR("failed, errCode: %{public}d.", errCode);
        return ;
    }

    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>();
    if (!bundleMgr_->GetHapModuleInfo(*abilityInfo.get(), *hapModuleInfo_)) {
        HILOG_ERROR("GetHapModuleInfo failed, will retval false value");
    }
}

void ContextImpl::InitHapModuleInfo(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    hapModuleInfo_ = std::make_shared<AppExecFwk::HapModuleInfo>(hapModuleInfo);
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ContextImpl::GetHapModuleInfo() const
{
    if (hapModuleInfo_ == nullptr) {
        HILOG_DEBUG("hapModuleInfo is empty");
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
        HILOG_DEBUG("application is nullptr");
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
    if (!OHOS::FileExists(dirPath)) {
        HILOG_DEBUG("ForceCreateDirectory, dir: %{public}s", dirPath.c_str());
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("create dir %{public}s failed, errno is %{public}d.", dirPath.c_str(), errno);
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
        HILOG_DEBUG("deviceType is %{public}s.", deviceType.c_str());
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
    int errCode = GetBundleManager();
    if (errCode != ERR_OK) {
        HILOG_ERROR("failed, errCode: %{public}d.", errCode);
        return errCode;
    }

    auto overlayMgrProxy = bundleMgr_->GetOverlayManagerProxy();
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
    HILOG_DEBUG("the size of overlay is: %{public}zu", overlayModuleInfos.size());
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
            HILOG_DEBUG("add path:%{public}s", iter->hapPath.c_str());
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
            HILOG_DEBUG("remove path:%{public}s", iter->hapPath.c_str());
            removePaths.emplace_back(iter->hapPath);
        }
    }

    return removePaths;
}

void ContextImpl::OnOverlayChanged(const EventFwk::CommonEventData &data,
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager, const std::string &bundleName,
    const std::string &moduleName, const std::string &loadPath)
{
    HILOG_DEBUG("begin.");
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
        HILOG_WARN("There is not any hapPath in overlayModuleInfo.");
    } else {
        if (isEnable) {
            std::vector<std::string> overlayPaths = GetAddOverlayPaths(overlayModuleInfos);
            if (!resourceManager->AddResource(loadPath, overlayPaths)) {
                HILOG_ERROR("AddResource error");
            }
        } else {
            std::vector<std::string> overlayPaths = GetRemoveOverlayPaths(overlayModuleInfos);
            if (!resourceManager->RemoveResource(loadPath, overlayPaths)) {
                HILOG_ERROR("RemoveResource error");
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

void ContextImpl::ClearUpApplicationData()
{
    int errCode = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->ClearUpApplicationDataBySelf();
    if (errCode != ERR_OK) {
        HILOG_ERROR("Delete bundle side user data by self is fail.");
        return;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
