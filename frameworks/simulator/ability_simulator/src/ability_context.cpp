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
    return GetBaseDir() + fileSeparator_ + CONTEXT_CACHE;
}

std::string AbilityContext::GetTempDir()
{
    return GetBaseDir() + fileSeparator_ + CONTEXT_TEMP;
}

std::string AbilityContext::GetFilesDir()
{
    return GetBaseDir() + fileSeparator_ + CONTEXT_FILES;
}

std::string AbilityContext::GetDatabaseDir()
{
    return GetPreviewPath() + currArea_ + fileSeparator_ + CONTEXT_DATABASE +
        fileSeparator_ + options_.moduleName;
}

std::string AbilityContext::GetPreferencesDir()
{
    return GetBaseDir() + fileSeparator_ + CONTEXT_PREFERENCES;
}

std::string AbilityContext::GetDistributedFilesDir()
{
    return GetPreviewPath() + currArea_ + fileSeparator_ + CONTEXT_DISTRIBUTEDFILES;
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
    return GetPreviewPath() + currArea_ + fileSeparator_ + CONTEXT_BASE + fileSeparator_ +
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
} // namespace AbilityRuntime
} // namespace OHOS
