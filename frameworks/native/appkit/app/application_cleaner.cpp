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

#include "application_cleaner.h"

#include <cstring>
#include <dirent.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "directory_ex.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "os_account_manager_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
static const std::string MARK_SYMBOL{ "_useless" };
static const std::string CONTEXT_DATA_APP{ "/data/app/" };
static const std::vector<std::string> CONTEXT_ELS{ "el1", "el2", "el3", "el4" };
static const std::string PATH_SEPARATOR = { "/" };
static const char FILE_SEPARATOR_CHAR = '/';
static const std::string CONTEXT_BASE{ "/base/" };
static const std::string MARK_TEMP_DIR{ "temp_useless" };
static const std::string CONTEXT_HAPS{ "/haps" };

static const size_t MARK_TEMP_LEN = 12;
static const int PATH_MAX_SIZE = 256;

const mode_t MODE = 0777;
static const int RESULT_OK = 0;
static const int RESULT_ERR = -1;
} // namespace
void ApplicationCleaner::RenameTempData()
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Context is null.");
        return;
    }
    std::vector<std::string> tempdir{};
    context_->GetAllTempDir(tempdir);
    if (tempdir.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Get app temp path list is empty.");
        return;
    }
    int64_t now =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    std::ostringstream stream;
    stream << std::hex << now;
    for (const auto &path : tempdir) {
        auto newPath = path + MARK_SYMBOL + stream.str();
        if (rename(path.c_str(), newPath.c_str()) != 0) {
            TAG_LOGE(AAFwkTag::APPKIT, "Rename temp dir failed, msg is %{public}s", strerror(errno));
        }
    }
}

void ApplicationCleaner::ClearTempData()
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    auto cleanTemp = [self = shared_from_this()]() {
        if (self == nullptr || self->context_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid shared pointer");
            return;
        }

        std::vector<std::string> rootDir;
        std::vector<std::string> temps;

        if (self->GetRootPath(rootDir) != RESULT_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Get root dir error.");
            return;
        }

        if (self->GetObsoleteBundleTempPath(rootDir, temps) != RESULT_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "Get bundle temp file list is false.");
            return;
        }

        for (const auto &temp : temps) {
            if (self->RemoveDir(temp) == false) {
                TAG_LOGE(AAFwkTag::APPKIT, "Clean bundle data dir failed, path: %{private}s", temp.c_str());
            }
        }
    };
    ffrt::submit(cleanTemp);
}

int ApplicationCleaner::GetRootPath(std::vector<std::string> &rootPath)
{
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid context pointer");
        return RESULT_ERR;
    }

    auto instance = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance();
    if (instance == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get OsAccountManager instance.");
        return RESULT_ERR;
    }

    int userId = -1;
    if (instance->GetOsAccountLocalIdFromProcess(userId) != RESULT_OK) {
        return RESULT_ERR;
    }

    rootPath.clear();
    auto baseDir = context_->GetBaseDir();
    auto infos = context_->GetApplicationInfo();
    if (infos == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Input param is invalid");
        return RESULT_ERR;
    }

    rootPath.emplace_back(baseDir);
    for (const auto &moudle : infos->moduleInfos) {
        auto moudleDir = baseDir + CONTEXT_HAPS + PATH_SEPARATOR + moudle.moduleName;
        if (access(moudleDir.c_str(), F_OK) != 0) {
            continue;
        }
        rootPath.emplace_back(moudleDir);
    }
    return RESULT_OK;
}

ErrCode ApplicationCleaner::GetObsoleteBundleTempPath(
    const std::vector<std::string> &rootPath, std::vector<std::string> &tempPath)
{
    if (rootPath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Input param is invalid");
        return RESULT_ERR;
    }

    for (const auto &dir : rootPath) {
        if (dir.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "Input param is invalid");
            continue;
        }
        std::vector<std::string> temp;
        TraverseObsoleteTempDirectory(dir, temp);
        std::copy(temp.begin(), temp.end(), std::back_inserter(tempPath));
    }
    return RESULT_OK;
}

void ApplicationCleaner::TraverseObsoleteTempDirectory(
    const std::string &currentPath, std::vector<std::string> &tempDirs)
{
    if (currentPath.empty() || (currentPath.size() > PATH_MAX_SIZE)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Traverse temp directory current path invaild");
        return;
    }

    std::string filePath = currentPath;
    DIR *dir = opendir(filePath.c_str());
    if (dir == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Open dir error. %{public}s", currentPath.c_str());
        return;
    }
    if (filePath.back() != FILE_SEPARATOR_CHAR) {
        filePath.push_back(FILE_SEPARATOR_CHAR);
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        if (ptr->d_type == DT_DIR && strncmp(ptr->d_name, MARK_TEMP_DIR.c_str(), MARK_TEMP_LEN) == 0) {
            std::string tempDir = filePath + std::string(ptr->d_name);
            tempDirs.emplace_back(tempDir);
            continue;
        }
        if (ptr->d_type == DT_DIR) {
            std::string currentDir = filePath + std::string(ptr->d_name);
            TraverseObsoleteTempDirectory(currentDir, tempDirs);
        }
    }
    closedir(dir);
}

bool ApplicationCleaner::RemoveDir(const std::string &tempPath)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called");
    if (tempPath.empty()) {
        return false;
    }
    struct stat buf = {};
    if (stat(tempPath.c_str(), &buf) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to obtain file properties");
        return false;
    }

    if (S_ISREG(buf.st_mode)) {
        return OHOS::RemoveFile(tempPath);
    }

    if (S_ISDIR(buf.st_mode)) {
        return OHOS::ForceRemoveDirectory(tempPath);
    }

    return false;
}

} // namespace AppExecFwk
} // namespace OHOS
