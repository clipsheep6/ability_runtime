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
const std::string CONTEXT_DISTRIBUTEDFILES("distributedfiles");
const std::string CONTEXT_FILE_SEPARATOR("/");
const std::string CONTEXT_FILE_OPPOSITE_SEPARATOR("\\");
const std::string CONTEXT_BASE("base");
const std::string CONTEXT_CACHE("cache");
const std::string CONTEXT_PREFERENCES("preferences");
const std::string CONTEXT_DATABASE("database");
const std::string CONTEXT_TEMP("temp");
const std::string CONTEXT_FILES("files");
const std::string CONTEXT_HAPS("haps");
const std::string CONTEXT_PREVIEW(".preview");
const std::string CONTEXT_ASSET("asset");
const std::string CONTEXT_ELS[] = {"el1", "el2"};
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

void AbilityContext::SetOptions(const Options& options)
{
    options_ = options;

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
    CreateMutiDir(dir);
    return dir;
}

std::string AbilityContext::GetTempDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_TEMP;
    CreateMutiDir(dir);
    return dir;
}

std::string AbilityContext::GetFilesDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_FILES;
    CreateMutiDir(dir);
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
    CreateMutiDir(dir);
    return dir;
}

std::string AbilityContext::GetPreferencesDir()
{
    if (GetPreviewPath().empty()) {
        return "";
    }

    auto dir = GetBaseDir() + fileSeparator_ + CONTEXT_PREFERENCES;
    CreateMutiDir(dir);
    return dir;
}

std::string AbilityContext::GetDistributedFilesDir()
{
    auto preivewDir = GetPreviewPath();
    if (preivewDir.empty()) {
        return "";
    }

    auto dir = preivewDir + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_DISTRIBUTEDFILES;
    CreateMutiDir(dir);
    return dir;
}

void AbilityContext::SwitchArea(int mode)
{
    HILOG_DEBUG("ContextImpl::SwitchArea, mode:%{public}d.", mode);
    if (mode < 0 || mode >= (int)(sizeof(CONTEXT_ELS) / sizeof(CONTEXT_ELS[0]))) {
        HILOG_ERROR("ContextImpl::SwitchArea, mode is invalid.");
        return;
    }
    currArea_ = CONTEXT_ELS[mode];
    HILOG_DEBUG("ContextImpl::SwitchArea end, currArea:%{public}s.", currArea_.c_str());
}

int AbilityContext::GetArea()
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

std::string AbilityContext::GetBaseDir()
{
    return GetPreviewPath() + fileSeparator_ + currArea_ + fileSeparator_ + CONTEXT_BASE + fileSeparator_ +
        CONTEXT_HAPS + fileSeparator_ + options_.moduleName;
}

std::string AbilityContext::GetPreviewPath()
{
    std::string previewPath;
    auto pos = options_.modulePath.find(CONTEXT_PREVIEW);
    if (pos != std::string::npos) {
        previewPath = options_.modulePath.substr(0, pos + strlen(CONTEXT_PREVIEW.c_str()));
    }
    return previewPath;
}

bool AbilityContext::Access(const std::string& path)
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

void AbilityContext::Mkdir(const std::string& path)
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

bool AbilityContext::CreateMutiDir(const std::string &path)
{
    if (path.empty()) {
        HILOG_DEBUG("path is empty");
        return false;
    }

    std::string tempStr = path;
    tempStr += fileSeparator_;

    std::string::size_type pos = 0;
    std::string::size_type prePos = 0;
    std::string strFolderPath;
    std::string stdFolderPath;

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

void AbilityContext::fs_req_cleanup(uv_fs_t* req)
{
    uv_fs_req_cleanup(req);
    if (req) {
        delete req;
        req = nullptr;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
