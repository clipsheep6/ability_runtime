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

#include "context_deal.h"

#include <regex>

#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_context.h"
#include "bundle_mgr_helper.h"
#include "constants.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

#define MODE 0771
namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AbilityBase::Constants;

const std::string ContextDeal::CONTEXT_DEAL_FILE_SEPARATOR("/");
const std::string ContextDeal::CONTEXT_DEAL_Files("files");
const int64_t ContextDeal::CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
const std::string ContextDeal::CONTEXT_DATA_STORAGE("/data/storage/");
const std::string ContextDeal::CONTEXT_DEAL_DATA_APP("/data/app/");
const std::string ContextDeal::CONTEXT_DEAL_BASE("base");
const std::string ContextDeal::CONTEXT_DEAL_DATABASE("database");
const std::string ContextDeal::CONTEXT_DEAL_PREFERENCES("preferences");
const std::string ContextDeal::CONTEXT_DEAL_DATA("data");

ContextDeal::ContextDeal(bool isCreateBySystemApp) : isCreateBySystemApp_(isCreateBySystemApp)
{}

std::shared_ptr<ApplicationInfo> ContextDeal::GetApplicationInfo() const
{
    return applicationInfo_;
}

void ContextDeal::SetApplicationInfo(const std::shared_ptr<ApplicationInfo> &info)
{
    if (info == nullptr) {
        HILOG_ERROR("SetApplicationInfo failed, info is empty");
        return;
    }
    applicationInfo_ = info;
}

std::shared_ptr<Context> ContextDeal::GetApplicationContext() const
{
    return appContext_;
}

void ContextDeal::SetApplicationContext(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        HILOG_ERROR("SetApplicationContext failed, context is empty");
        return;
    }
    appContext_ = context;
}

std::string ContextDeal::GetBundleCodePath()
{
    if (applicationInfo_ == nullptr) {
        return "";
    }

    std::string dir;
    if (isCreateBySystemApp_) {
        dir = std::regex_replace(applicationInfo_->codePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    } else {
        dir = LOCAL_CODE_PATH;
    }

    return dir;
}

void ContextDeal::SetBundleCodePath(std::string &path)
{
    path_ = path;
}

const std::shared_ptr<AbilityInfo> ContextDeal::GetAbilityInfo()
{
    return abilityInfo_;
}

void ContextDeal::SetAbilityInfo(const std::shared_ptr<AbilityInfo> &info)
{
    if (info == nullptr) {
        HILOG_ERROR("SetAbilityInfo failed, info is empty");
        return;
    }
    abilityInfo_ = info;
}

std::shared_ptr<Context> ContextDeal::GetContext()
{
    return abilityContext_;
}

void ContextDeal::SetContext(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        HILOG_ERROR("The context is empty.");
        return;
    }
    abilityContext_ = context;
}

std::shared_ptr<BundleMgrHelper> ContextDeal::GetBundleManager() const
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("Failed to get bundle manager service.");
        return nullptr;
    }
    return bundleMgrHelper;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextDeal::GetResourceManager() const
{
    return resourceManager_;
}

std::string ContextDeal::GetDatabaseDir()
{
    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = CONTEXT_DEAL_DATA_APP + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR + std::to_string(GetCurrentAccountId())
            + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DATABASE + CONTEXT_DEAL_FILE_SEPARATOR + GetBundleName();
    } else {
        dir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DATABASE;
    }
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("GetDatabaseDir:%{public}s", dir.c_str());
    return dir;
}

std::string ContextDeal::GetDataDir()
{
    std::string dir = GetBaseDir() + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_DATA;
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("GetDataDir dir = %{public}s", dir.c_str());
    return dir;
}

std::string ContextDeal::GetDir(const std::string &name, int mode)
{
    if (applicationInfo_ == nullptr) {
        HILOG_ERROR("GetDir failed, applicationInfo_ == nullptr");
        return "";
    }
    std::string dir = applicationInfo_->dataDir + CONTEXT_DEAL_FILE_SEPARATOR + name;
    if (!OHOS::FileExists(dir)) {
        HILOG_INFO("GetDir File is not exits");
        OHOS::ForceCreateDirectory(dir);
        OHOS::ChangeModeDirectory(dir, mode);
    }
    return dir;
}

std::string ContextDeal::GetFilesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_Files;
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("GetFilesDir dir = %{public}s", dir.c_str());
    return dir;
}

std::string ContextDeal::GetBundleName() const
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::string ContextDeal::GetBundleResourcePath()
{
    if (abilityInfo_ == nullptr) {
        return "";
    }

    std::string dir;
    if (isCreateBySystemApp_) {
        dir = std::regex_replace(abilityInfo_->resourcePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES);
    } else {
        std::regex pattern(std::string(ABS_CODE_PATH) + std::string(FILE_SEPARATOR) + abilityInfo_->bundleName);
        dir = std::regex_replace(abilityInfo_->resourcePath, pattern, LOCAL_CODE_PATH);
    }
    return dir;
}

sptr<AAFwk::IAbilityManager> ContextDeal::GetAbilityManager()
{
    auto remoteObject = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("Failed to get ability manager service.");
        return nullptr;
    }
    sptr<AAFwk::IAbilityManager> ams = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    return ams;
}

std::string ContextDeal::GetAppType()
{
    auto bundleMgrHelper = GetBundleManager();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("GetAppType failed to get bundle manager service");
        return "";
    }
    std::string retString = bundleMgrHelper->GetAppType(applicationInfo_->bundleName);
    return retString;
}

bool ContextDeal::IsCreateBySystemApp() const
{
    return (static_cast<uint64_t>(flags_) & static_cast<uint64_t>(CONTEXT_CREATE_BY_SYSTEM_APP)) == 1;
}

int ContextDeal::GetCurrentAccountId() const
{
    int userId = 0;
    DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->GetOsAccountLocalIdFromProcess(userId);
    return userId;
}

void ContextDeal::CreateDirIfNotExist(const std::string &dirPath) const
{
    if (!OHOS::FileExists(dirPath)) {
        HILOG_DEBUG("CreateDirIfNotExist File is not exits");
        bool createDir = OHOS::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_INFO("CreateDirIfNotExist: create dir %{public}s failed.", dirPath.c_str());
            return;
        }
    }
}

void ContextDeal::SetPattern(int patternId)
{
    if (resourceManager_ != nullptr) {
        if (!pattern_.empty()) {
            pattern_.clear();
        }
        OHOS::Global::Resource::RState errval = resourceManager_->GetPatternById(patternId, pattern_);
        if (errval != OHOS::Global::Resource::RState::SUCCESS) {
            HILOG_ERROR("SetPattern GetPatternById(patternId:%d) retval is %u", patternId, errval);
        }
    } else {
        HILOG_ERROR("SetPattern resourceManager_ is nullptr");
    }
}

std::shared_ptr<HapModuleInfo> ContextDeal::GetHapModuleInfo()
{
    // fix set HapModuleInfoLocal data failed, request only once
    if (hapModuleInfoLocal_ == nullptr) {
        HapModuleInfoRequestInit();
        if (hapModuleInfoLocal_ == nullptr) {
            HILOG_ERROR("hapModuleInfoLocal_ is nullptr");
            return nullptr;
        }
    }
    return hapModuleInfoLocal_;
}

void ContextDeal::initResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resourceManager)
{
    resourceManager_ = resourceManager;
}

std::string ContextDeal::GetString(int resId)
{
    if (resourceManager_ == nullptr) {
        HILOG_ERROR("GetString resourceManager_ is nullptr");
        return "";
    }

    std::string ret;
    OHOS::Global::Resource::RState errval = resourceManager_->GetStringById(resId, ret);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return ret;
    } else {
        HILOG_ERROR("GetString GetStringById(resId:%d) retval is %u", resId, errval);
        return "";
    }
}

std::vector<std::string> ContextDeal::GetStringArray(int resId)
{
    if (resourceManager_ == nullptr) {
        HILOG_ERROR("GetStringArray resourceManager_ is nullptr");
        return std::vector<std::string>();
    }

    std::vector<std::string> retv;
    OHOS::Global::Resource::RState errval = resourceManager_->GetStringArrayById(resId, retv);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return retv;
    } else {
        HILOG_ERROR("GetStringArray GetStringArrayById(resId:%d) retval is %u", resId, errval);
        return std::vector<std::string>();
    }
}

std::vector<int> ContextDeal::GetIntArray(int resId)
{
    if (resourceManager_ == nullptr) {
        HILOG_ERROR("GetIntArray resourceManager_ is nullptr");
        return std::vector<int>();
    }

    std::vector<int> retv;
    OHOS::Global::Resource::RState errval = resourceManager_->GetIntArrayById(resId, retv);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return retv;
    } else {
        HILOG_ERROR("GetIntArray GetIntArrayById(resId:%d) retval is %u", resId, errval);
        return std::vector<int>();
    }
}

std::map<std::string, std::string> ContextDeal::GetTheme()
{
    if (theme_.empty()) {
        SetTheme(GetThemeId());
    }
    return theme_;
}

void ContextDeal::SetTheme(int themeId)
{
    if (resourceManager_ == nullptr) {
        HILOG_ERROR("SetTheme resourceManager_ is nullptr");
        return;
    }

    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        HILOG_ERROR("SetTheme hapModInfo is nullptr");
        return;
    }

    if (!theme_.empty()) {
        theme_.clear();
    }
    OHOS::Global::Resource::RState errval = resourceManager_->GetThemeById(themeId, theme_);
    if (errval != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_ERROR("SetTheme GetThemeById(themeId:%d) retval is %u", themeId, errval);
    }
}

std::map<std::string, std::string> ContextDeal::GetPattern()
{
    if (!pattern_.empty()) {
        return pattern_;
    } else {
        HILOG_ERROR("GetPattern pattern_ is empty");
        return std::map<std::string, std::string>();
    }
}

int ContextDeal::GetColor(int resId)
{
    if (resourceManager_ == nullptr) {
        HILOG_ERROR("GetColor resourceManager_ is nullptr");
        return INVALID_RESOURCE_VALUE;
    }

    uint32_t ret = INVALID_RESOURCE_VALUE;
    OHOS::Global::Resource::RState errval = resourceManager_->GetColorById(resId, ret);
    if (errval == OHOS::Global::Resource::RState::SUCCESS) {
        return ret;
    } else {
        HILOG_ERROR("GetColor GetColorById(resId:%d) retval is %u", resId, errval);
        return INVALID_RESOURCE_VALUE;
    }
}

int ContextDeal::GetThemeId()
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo != nullptr) {
        return -1;
    } else {
        HILOG_ERROR("GetThemeId hapModInfo is nullptr");
        return -1;
    }
}

int ContextDeal::GetDisplayOrientation()
{
    if (abilityInfo_ != nullptr) {
        HILOG_DEBUG("GetDisplayOrientation end");
        return static_cast<int>(abilityInfo_->orientation);
    } else {
        HILOG_ERROR("GetDisplayOrientation abilityInfo_ is nullptr");
        return static_cast<int>(DisplayOrientation::UNSPECIFIED);
    }
}

std::string ContextDeal::GetPreferencesDir()
{
    std::string dir = GetBaseDir() + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_PREFERENCES;
    CreateDirIfNotExist(dir);
    HILOG_DEBUG("GetPreferencesDir:%{public}s", dir.c_str());
    return dir;
}

void ContextDeal::SetColorMode(int mode)
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        HILOG_ERROR("SetColorMode hapModInfo is nullptr");
        return;
    }

    if (mode == static_cast<int>(ModuleColorMode::DARK)) {
        hapModInfo->colorMode = ModuleColorMode::DARK;
    } else if (mode == static_cast<int>(ModuleColorMode::LIGHT)) {
        hapModInfo->colorMode = ModuleColorMode::LIGHT;
    } else {  // default use AUTO
        hapModInfo->colorMode = ModuleColorMode::AUTO;
    }
}

int ContextDeal::GetColorMode()
{
    auto hapModInfo = GetHapModuleInfo();
    if (hapModInfo == nullptr) {
        HILOG_ERROR("GetColorMode hapModInfo is nullptr");
        return -1;
    }
    return static_cast<int>(hapModInfo->colorMode);
}


bool ContextDeal::HapModuleInfoRequestInit()
{
    auto bundleMgrHelper = GetBundleManager();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("Failed to get bundle manager service.");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("The abilityInfo_ is nullptr.");
        return false;
    }

    hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    if (!bundleMgrHelper->GetHapModuleInfo(*abilityInfo_.get(), *hapModuleInfoLocal_)) {
        HILOG_ERROR("Failed, will retval false value.");
        return false;
    }
    return true;
}

std::string ContextDeal::GetBaseDir() const
{
    std::string baseDir;
    if (IsCreateBySystemApp()) {
        baseDir = CONTEXT_DEAL_DATA_APP + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR +
            std::to_string(GetCurrentAccountId()) + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_BASE +
            CONTEXT_DEAL_FILE_SEPARATOR + GetBundleName();
    } else {
        baseDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_DEAL_FILE_SEPARATOR + CONTEXT_DEAL_BASE;
    }

    HILOG_DEBUG("GetBaseDir:%{public}s", baseDir.c_str());
    return baseDir;
}
}  // namespace AppExecFwk
}  // namespace OHOS
