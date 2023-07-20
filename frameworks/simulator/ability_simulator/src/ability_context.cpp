/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_context.h"

#include <cstring>
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* CONTEXT_DISTRIBUTEDFILES("distributedfiles");
constexpr const char* CONTEXT_FILE_SEPARATOR("/");
constexpr const char* CONTEXT_FILE_OPPOSITE_SEPARATOR("\\");
constexpr const char* CONTEXT_BASE("base");
constexpr const char* CONTEXT_CACHE("cache");
constexpr const char* CONTEXT_PREFERENCES("preferences");
constexpr const char* CONTEXT_DATABASE("database");
constexpr const char* CONTEXT_TEMP("temp");
constexpr const char* CONTEXT_FILES("files");
constexpr const char* CONTEXT_HAPS("haps");
constexpr const char* CONTEXT_PREVIEW(".preview");
constexpr const char* CONTEXT_ASSET("asset");
constexpr const char* CONTEXT_ELS[] = {"el1", "el2"};
constexpr int DIR_DEFAULT_PERM = 0770;
}
std::shared_ptr<AppExecFwk::Configuration> AbilityContext::GetConfiguration()
{
    return nullptr;
}

Options AbilityContext::GetOptions()
{
    return options_;
}

void AbilityContext::SetOptions(const Options &options)
{
    options_ = options;
    HILOG_DEBUG("Options.bundleName: %{public}s", options.bundleName.c_str());
    HILOG_DEBUG("Options.moduleName: %{public}s", options.moduleName.c_str());
    HILOG_DEBUG("Options.modulePath: %{public}s", options.modulePath.c_str());
    HILOG_DEBUG("Options.resourcePath: %{public}s", options.resourcePath.c_str());
    HILOG_DEBUG("Options.debugPort: %{public}d", options.debugPort);
    HILOG_DEBUG("Options.assetPath: %{public}s", options.assetPath.c_str());
    HILOG_DEBUG("Options.systemResourcePath: %{public}s", options.systemResourcePath.c_str());
    HILOG_DEBUG("Options.appResourcePath: %{public}s", options.appResourcePath.c_str());
    HILOG_DEBUG("Options.containerSdkPath: %{public}s", options.containerSdkPath.c_str());
    HILOG_DEBUG("Options.url: %{public}s", options.url.c_str());
    HILOG_DEBUG("Options.language: %{public}s", options.language.c_str());
    HILOG_DEBUG("Options.region: %{public}s", options.region.c_str());
    HILOG_DEBUG("Options.script: %{public}s", options.script.c_str());
    HILOG_DEBUG("Options.themeId: %{public}d", options.themeId);
    HILOG_DEBUG("Options.deviceWidth: %{public}d", options.deviceWidth);
    HILOG_DEBUG("Options.deviceHeight: %{public}d", options.deviceHeight);
    HILOG_DEBUG("Options.isRound: %{public}d", options.themeId);
    HILOG_DEBUG("Options.compatibleVersion: %{public}d", options.compatibleVersion);
    HILOG_DEBUG("Options.installationFree: %{public}d", options.installationFree);
    HILOG_DEBUG("Options.labelId: %{public}d", options.labelId);
    HILOG_DEBUG("Options.compileMode: %{public}s", options.compileMode.c_str());
    HILOG_DEBUG("Options.pageProfile: %{public}s", options.pageProfile.c_str());
    HILOG_DEBUG("Options.targetVersion: %{public}d", options.targetVersion);
    HILOG_DEBUG("Options.releaseType: %{public}s", options.releaseType.c_str());
    HILOG_DEBUG("Options.enablePartialUpdate: %{public}d", options.enablePartialUpdate);

    auto pos = options_.modulePath.find(CONTEXT_FILE_SEPARATOR);
    if (pos == std::string::npos) {
        fileSeparator_ = CONTEXT_FILE_OPPOSITE_SEPARATOR;
    }
}

std::string AbilityContext::GetBundleName()
{
    return options_.bundleName;
}

std::string AbilityContext::GetBundleCodePath()
{
    std::string path;
    auto pos = options_.assetPath.find(CONTEXT_ASSET);
    if (pos != std::string::npos) {
        path = options_.assetPath.substr(0, pos);
    }
    return path;
}

std::string AbilityContext::GetBundleCodeDir()
{
    return GetPreviewPath();
}

std::string AbilityContext::GetCacheDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_CACHE;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityContext::GetTempDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_TEMP;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityContext::GetFilesDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_FILES;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityContext::GetDatabaseDir()
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

std::string AbilityContext::GetPreferencesDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_PREFERENCES;
    CreateMultiDir(dir);
    return dir;
}

std::string AbilityContext::GetDistributedFilesDir()
{
    auto preivewDir = GetPreviewPath();
    if (preivewDir.empty()) {
        return "";
    }

    auto dir = preivewDir + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_DISTRIBUTEDFILES;
    CreateMultiDir(dir);
    return dir;
}

void AbilityContext::SwitchArea(int mode)
{
    HILOG_DEBUG("called, mode:%{public}d.", mode);
    if (mode < 0 || mode >= (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0]))) {
        HILOG_ERROR("mode is invalid.");
        return;
    }
    currArea_ = CONTEXT_ELS[mode];
}

int AbilityContext::GetArea()
{
    HILOG_DEBUG("called");
    int mode = -1;
    for (int i = 0; i < static_cast<int>(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0])); i++) {
        if (currArea_ == CONTEXT_ELS[i]) {
            mode = i;
            break;
        }
    }
    if (mode == -1) {
        HILOG_ERROR("Not find mode.");
        return EL_DEFAULT;
    }
    return mode;
}

std::string AbilityContext::GetBaseDir()
{
    auto previewPath = GetPreviewPath();
    if (previewPath.empty()) {
        return "";
    }

    return previewPath + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_BASE + fileSeparator_ +
        CONTEXT_HAPS + fileSeparator_ + options_.moduleName;
}

std::string AbilityContext::GetPreviewPath()
{
    std::string previewPath;
    auto pos = options_.modulePath.find(CONTEXT_PREVIEW);
    if (pos != std::string::npos) {
        previewPath = options_.modulePath.substr(0, pos + strlen(CONTEXT_PREVIEW));
    }
    return previewPath;
}

bool AbilityContext::Access(const std::string &path)
{
    HILOG_DEBUG("Access: dir: %{public}s", path.c_str());
    std::unique_ptr<uv_fs_t, decltype(AbilityContext::fs_req_cleanup)*> access_req = {
        new uv_fs_t, AbilityContext::fs_req_cleanup };
    if (!access_req) {
        HILOG_ERROR("Failed to request heap memory.");
        return false;
    }

    int ret = uv_fs_access(nullptr, access_req.get(), path.c_str(), 0, nullptr);
    return ret == 0;
}

void AbilityContext::Mkdir(const std::string &path)
{
    HILOG_DEBUG("Mkdir: dir: %{public}s", path.c_str());
    std::unique_ptr<uv_fs_t, decltype(AbilityContext::fs_req_cleanup)*> mkdir_req = {
        new uv_fs_t, AbilityContext::fs_req_cleanup };
    if (!mkdir_req) {
        HILOG_ERROR("Failed to request heap memory.");
        return;
    }

    int ret = uv_fs_mkdir(nullptr, mkdir_req.get(), path.c_str(), DIR_DEFAULT_PERM, nullptr);
    if (ret < 0) {
        HILOG_ERROR("Failed to create directory");
    }
}

bool AbilityContext::CreateMultiDir(const std::string &path)
{
    if (path.empty()) {
        HILOG_DEBUG("path is empty");
        return false;
    }

    if (Access(path)) {
        HILOG_DEBUG("path is already exist");
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

void AbilityContext::fs_req_cleanup(uv_fs_t *req)
{
    uv_fs_req_cleanup(req);
    if (req) {
        delete req;
        req = nullptr;
    }
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContext::GetResourceManager() const
{
    return resourceMgr_;
}

void AbilityContext::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resMgr)
{
    resourceMgr_ = resMgr;
}
} // namespace AbilityRuntime
} // namespace OHOS
