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

#include <regex>

#include "bundle_mgr_helper.h"
#include "extension_context.h"
#include "constants.h"
#include "hilog_wrapper.h"
#include "file_ex.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AbilityBase::Constants;

const size_t ExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("ExtensionContext"));
const std::string ExtensionContext::CONTEXT_DATA_APP("/data/app/");
const std::string ExtensionContext::CONTEXT_BUNDLE("/bundle/");
const std::string ExtensionContext::CONTEXT_DISTRIBUTEDFILES_BASE_BEFORE("/mnt/hmdfs/");
const std::string ExtensionContext::CONTEXT_DISTRIBUTEDFILES_BASE_MIDDLE("/device_view/local/data/");
const std::string ExtensionContext::CONTEXT_DISTRIBUTEDFILES("distributedfiles");
const std::string ExtensionContext::CONTEXT_FILE_SEPARATOR("/");
const std::string ExtensionContext::CONTEXT_DATA("/data/");
const std::string ExtensionContext::CONTEXT_DATA_STORAGE("/data/storage/");
const std::string ExtensionContext::CONTEXT_BASE("base");
const std::string ExtensionContext::CONTEXT_CACHE("cache");
const std::string ExtensionContext::CONTEXT_PREFERENCES("preferences");
const std::string ExtensionContext::CONTEXT_GROUP("group");
const std::string ExtensionContext::CONTEXT_DATABASE("database");
const std::string ExtensionContext::CONTEXT_TEMP("/temp");
const std::string ExtensionContext::CONTEXT_FILES("/files");
const std::string ExtensionContext::CONTEXT_HAPS("/haps");
const std::string ExtensionContext::CONTEXT_ELS[] = {"el1", "el2", "el3", "el4"};
const std::string ExtensionContext::CONTEXT_RESOURCE_END = "/resources/resfile";
const std::string ExtensionContext::CONTEXT_EXTENSION("extension");
const mode_t MODE = 0770;
const int AREA2 = 2;
const int AREA3 = 3;
const std::string UNIQUE_ID("uniqueId");

void ExtensionContext::SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        HILOG_ERROR("ExtensionContext::SetAbilityInfo Info == nullptr");
        return;
    }
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> ExtensionContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

std::string ExtensionContext::GetDistributedFilesDir()
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
    dir += CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExist(dir, 0);
    HILOG_DEBUG("distributedFilesdir:%{public}s", dir.c_str());
    return dir;
}

std::string ExtensionContext::GetResourceDir()
{
    std::string resourceDir;
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfoPtr = GetHapModuleInfo();
    if (hapModuleInfoPtr != nullptr && !hapModuleInfoPtr->moduleName.empty()) {
        std::string dir = resourceDir = std::string(LOCAL_CODE_PATH) + CONTEXT_FILE_SEPARATOR + hapModuleInfoPtr->moduleName + CONTEXT_RESOURCE_END + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
        if (OHOS::FileExists(dir)) {
             resourceDir = dir;
        }
    }
    HILOG_DEBUG("resourceDir:%{public}s", resourceDir.c_str());
    return resourceDir;
}

std::string ExtensionContext::GetFilesDir()
{
    std::string filesDir = GetBaseDir() + CONTEXT_FILES + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExist(filesDir, MODE);
    HILOG_DEBUG("filesDir:%{public}s", filesDir.c_str());
    return filesDir;
}

std::string ExtensionContext::GetCacheDir()
{
    std::string cacheDir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_CACHE + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExist(cacheDir, MODE);
    HILOG_INFO("cacheDir:%{public}s", cacheDir.c_str());
    return cacheDir;
}

std::string ExtensionContext::GetTempDir()
{
    std::string tempDir = GetBaseDir() + CONTEXT_TEMP + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExist(tempDir, MODE);
    HILOG_DEBUG("tempDir:%{public}s", tempDir.c_str());
    return tempDir;
}

std::string ExtensionContext::GetDatabaseDir()
{
    std::string databaseDir;
    GetDatabaseDirWithCheck(true, databaseDir);
    HILOG_DEBUG("databaseDir: %{public}s", databaseDir.c_str());
    return databaseDir;
}

int32_t ExtensionContext::GetDatabaseDirWithCheck(bool checkExist, std::string &databaseDir)
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
    databaseDir += CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExistWithCheck(databaseDir, 0, checkExist);
    return ERR_OK;
}

std::string ExtensionContext::GetPreferencesDir()
{
    std::string dir;
    GetPreferencesDirWithCheck(true, dir);
    HILOG_DEBUG("preferencesDir: %{public}s", dir.c_str());
    return dir;
}

int32_t ExtensionContext::GetPreferencesDirWithCheck(bool checkExist, std::string &preferencesDir)
{
    preferencesDir = GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExistWithCheck(preferencesDir, MODE, checkExist);
    return ERR_OK;
}

std::string ExtensionContext::GetBundleCodeDir()
{
    auto appInfo = GetApplicationInfo();
    if (appInfo == nullptr) {
        return "";
    }

    std::string dir;
    if (IsCreateBySystemApp()) {
        dir = std::regex_replace(appInfo->codePath, std::regex(ABS_CODE_PATH), LOCAL_BUNDLES) + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    } else {
        dir = LOCAL_CODE_PATH + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    }
    CreateDirIfNotExist(dir, MODE);
    HILOG_DEBUG("bundleCodeDir:%{public}s", dir.c_str());
    return dir;
}

std::string ExtensionContext::GetGroupDir(std::string groupId)
{
    std::string dir;
    GetGroupDirWithCheck(groupId, true, dir);
    HILOG_DEBUG("groupDir:%{public}s", dir.c_str());
    return dir;
}

int32_t ExtensionContext::GetGroupDirWithCheck(const std::string &groupId, bool checkExist, std::string &groupDir)
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
    groupDir = CONTEXT_DATA_STORAGE + currArea_ + CONTEXT_FILE_SEPARATOR + CONTEXT_GROUP + uuid + CONTEXT_FILE_SEPARATOR + UNIQUE_ID;
    CreateDirIfNotExistWithCheck(groupDir, MODE, true);
    return ERR_OK;
}

std::string ExtensionContext::GetBaseDir() const
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
    HILOG_DEBUG("baseDir:%{public}s", baseDir.c_str());
    return baseDir;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
