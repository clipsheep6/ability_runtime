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

#include "constants.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_context.h"
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
const std::string ContextDeal::CONTEXT_DATA_STORAGE("/data/storage/");
const std::string ContextDeal::CONTEXT_DEAL_DATA_APP("/data/app/");
const std::string ContextDeal::CONTEXT_DEAL_BASE("base");
const std::string ContextDeal::CONTEXT_DEAL_DATABASE("database");
const std::string ContextDeal::CONTEXT_DEAL_PREFERENCES("preferences");
const std::string ContextDeal::CONTEXT_DEAL_DATA("data");

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
        HILOG_ERROR("SetContext failed, context is empty");
        return;
    }
    abilityContext_ = context;
}

sptr<IBundleMgr> ContextDeal::GetBundleManager() const
{
    auto bundleObj =
        OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    sptr<IBundleMgr> bms = iface_cast<IBundleMgr>(bundleObj);
    return bms;
}

std::shared_ptr<Global::Resource::ResourceManager> ContextDeal::GetResourceManager() const
{
    return resourceManager_;
}

std::string ContextDeal::GetDatabaseDir()
{
    std::string dir;
    if (isCreateBySystemApp_) {
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

sptr<AAFwk::IAbilityManager> ContextDeal::GetAbilityManager()
{
    auto remoteObject = OHOS::DelayedSingleton<SysMrgClient>::GetInstance()->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("failed to get ability manager service");
        return nullptr;
    }
    sptr<AAFwk::IAbilityManager> ams = iface_cast<AAFwk::IAbilityManager>(remoteObject);
    return ams;
}

std::string ContextDeal::GetAppType()
{
    sptr<IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        HILOG_ERROR("GetAppType failed to get bundle manager service");
        return "";
    }
    std::string retString = ptr->GetAppType(applicationInfo_->bundleName);
    return retString;
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

void ContextDeal::SetSystemAppFlag(bool isCreateBySystemApp)
{
    isCreateBySystemApp_ = isCreateBySystemApp;
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

bool ContextDeal::HapModuleInfoRequestInit()
{
    sptr<IBundleMgr> ptr = GetBundleManager();
    if (ptr == nullptr) {
        HILOG_ERROR("GetHapModuleInfo failed to get bundle manager service");
        return false;
    }

    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("GetHapModuleInfo failed for abilityInfo_ is nullptr");
        return false;
    }

    hapModuleInfoLocal_ = std::make_shared<HapModuleInfo>();
    if (!ptr->GetHapModuleInfo(*abilityInfo_.get(), *hapModuleInfoLocal_)) {
        HILOG_ERROR("IBundleMgr::GetHapModuleInfo failed, will retval false value");
        return false;
    }
    return true;
}

std::string ContextDeal::GetBaseDir() const
{
    std::string baseDir;
    if (isCreateBySystemApp_) {
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
