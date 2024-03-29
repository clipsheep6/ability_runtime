/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ability_stage_context.h"

#include <cstring>
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *CONTEXT_DISTRIBUTEDFILES("distributedfiles");
constexpr const char *CONTEXT_FILE_SEPARATOR("/");
constexpr const char *CONTEXT_FILE_OPPOSITE_SEPARATOR("\\");
constexpr const char *CONTEXT_BASE("base");
constexpr const char *CONTEXT_CACHE("cache");
constexpr const char *CONTEXT_PREFERENCES("preferences");
constexpr const char *CONTEXT_DATABASE("database");
constexpr const char *CONTEXT_TEMP("temp");
constexpr const char *CONTEXT_FILES("files");
constexpr const char *CONTEXT_HAPS("haps");
constexpr const char *CONTEXT_ASSET("asset");
constexpr const char *CONTEXT_ELS[] = {"el1", "el2", "el3", "el4"};
constexpr const char *CONTEXT_RESOURCE_BASE("/data/storage/el1/bundle");
constexpr const char *CONTEXT_RESOURCE_END("/resources/resfile");
constexpr int DIR_DEFAULT_PERM = 0770;
}
std::shared_ptr<AppExecFwk::Configuration> AbilityStageContext::GetConfiguration()
{
    return configuration_;
}

void AbilityStageContext::SetConfiguration(const std::shared_ptr<AppExecFwk::Configuration> &configuration)
{
    configuration_ = configuration;
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityStageContext::GetApplicationInfo() const
{
    return applicationInfo_;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityStageContext::GetHapModuleInfo() const
{
    return hapModuleInfo_;
}

void AbilityStageContext::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info)
{
    applicationInfo_ = info;
}

void AbilityStageContext::SetHapModuleInfo(const std::shared_ptr<AppExecFwk::HapModuleInfo> &info)
{
    hapModuleInfo_ = info;
}

Options AbilityStageContext::GetOptions()
{
    return options_;
}

void AbilityStageContext::SetOptions(const Options &options)
{
    options_ = options;

    auto pos = options_.previewPath.find(CONTEXT_FILE_SEPARATOR);
    if (pos == std::string::npos) {
        fileSeparator_ = CONTEXT_FILE_OPPOSITE_SEPARATOR;
    }
}

std::string AbilityStageContext::GetBundleName()
{
    return options_.bundleName;
}

std::string AbilityStageContext::GetBundleCodePath()
{
    std::string path;
    auto pos = options_.assetPath.find(CONTEXT_ASSET);
    if (pos != std::string::npos) {
        path = options_.assetPath.substr(0, pos);
    }
    return path;
}

std::string AbilityStageContext::GetBundleCodeDir()
{
    return GetPreviewPath();
}

std::string AbilityStageContext::GetCacheDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_CACHE;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityStageContext::GetTempDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_TEMP;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityStageContext::GetResourceDir()
{
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfoPtr = GetHapModuleInfo();
    if (hapModuleInfoPtr == nullptr || hapModuleInfoPtr->moduleName.empty()) {
        return "";
    }
    auto dir = std::string(CONTEXT_RESOURCE_BASE) +
        CONTEXT_FILE_SEPARATOR + hapModuleInfoPtr->moduleName + CONTEXT_RESOURCE_END;
    if (Access(dir)) {
        return dir;
    }
    return "";
}

std::string AbilityStageContext::GetFilesDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_FILES;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityStageContext::GetDatabaseDir()
{
    auto preivewDir = GetPreviewPath();
    if (preivewDir.empty()) {
        return "";
    }

    auto dir = preivewDir + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_DATABASE +
        fileSeparator_ + options_.moduleName;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityStageContext::GetPreferencesDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_PREFERENCES;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityStageContext::GetDistributedFilesDir()
{
    auto preivewDir = GetPreviewPath();
    if (preivewDir.empty()) {
        return "";
    }

    auto dir = preivewDir + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_DISTRIBUTEDFILES;
    CreateMultiDir(dir);
    return dir;
}

void AbilityStageContext::SwitchArea(int mode)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called, mode:%{public}d.", mode);
    if (mode < 0 || mode >= static_cast<int>(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0]))) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "mode is invalid.");
        return;
    }
    currArea_ = CONTEXT_ELS[mode];
}

int AbilityStageContext::GetArea()
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    int mode = -1;
    for (int i = 0; i < static_cast<int>(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0])); i++) {
        if (currArea_ == CONTEXT_ELS[i]) {
            mode = i;
            break;
        }
    }
    if (mode == -1) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Not find mode.");
        return EL_DEFAULT;
    }
    return mode;
}

std::string AbilityStageContext::GetBaseDir()
{
    auto previewPath = GetPreviewPath();
    if (previewPath.empty()) {
        return "";
    }

    return previewPath + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_BASE + fileSeparator_ +
        CONTEXT_HAPS + fileSeparator_ + options_.moduleName;
}

std::string AbilityStageContext::GetPreviewPath()
{
    return options_.previewPath;
}

bool AbilityStageContext::Access(const std::string &path)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Access: dir: %{public}s", path.c_str());
    std::unique_ptr<uv_fs_t, decltype(AbilityStageContext::FsReqCleanup)*> access_req = {
        new uv_fs_t, AbilityStageContext::FsReqCleanup };
    if (!access_req) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Failed to request heap memory.");
        return false;
    }

    return (uv_fs_access(nullptr, access_req.get(), path.c_str(), 0, nullptr) == 0);
}

void AbilityStageContext::Mkdir(const std::string &path)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "Mkdir: dir: %{public}s", path.c_str());
    std::unique_ptr<uv_fs_t, decltype(AbilityStageContext::FsReqCleanup)*> mkdir_req = {
        new uv_fs_t, AbilityStageContext::FsReqCleanup };
    if (!mkdir_req) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Failed to request heap memory.");
        return;
    }

    int ret = uv_fs_mkdir(nullptr, mkdir_req.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
    if (ret < 0) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Failed to create directory");
    }
}

bool AbilityStageContext::CreateMultiDir(const std::string &path)
{
    if (path.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "path is empty");
        return false;
    }

    if (Access(path)) {
        TAG_LOGD(AAFwkTag::ABILITY_SIM, "path is already exist");
        return true;
    }

    std::string tempStr = path;
    tempStr += fileSeparator_;

    std::string::size_type pos = 0;
    std::string::size_type prePos = 0;
    std::string strFolderPath;

    while ((pos = tempStr.find(fileSeparator_, pos)) != std::string::npos) {
        strFolderPath = tempStr.substr(0, pos);
        if (Access(strFolderPath)) {
            pos = pos + 1;
            prePos = pos;
            continue;
        }

        Mkdir(strFolderPath);
        pos = pos + 1;
        prePos = pos;
    }

    return Access(tempStr);
}

void AbilityStageContext::FsReqCleanup(uv_fs_t *req)
{
    uv_fs_req_cleanup(req);
    if (req) {
        delete req;
        req = nullptr;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
