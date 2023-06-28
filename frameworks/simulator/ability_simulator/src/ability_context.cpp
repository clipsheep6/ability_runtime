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

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string CONTEXT_DISTRIBUTEDFILES("distributedfiles");
const std::string CONTEXT_FILE_SEPARATOR("/");
const std::string CONTEXT_DATA_STORAGE("/data/storage/");
const std::string CONTEXT_BASE("base");
const std::string CONTEXT_CACHE("cache");
const std::string CONTEXT_PREFERENCES("preferences");
const std::string CONTEXT_DATABASE("database");
const std::string CONTEXT_TEMP("/temp");
const std::string CONTEXT_FILES("/files");
const std::string CONTEXT_HAPS("/haps");
const std::string CURRENT_AREA("el2");
constexpr const char* LOCAL_CODE_PATH = "/data/storage/el1/bundle";
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
}

std::string AbilityContext::GetBundleName()
{
    return options_.bundleName;
}

std::string AbilityContext::GetBundleCodePath()
{
    return options_.codePath;
}

std::string AbilityContext::GetBundleCodeDir()
{
    // /data/storage/el1/bundle
    return LOCAL_CODE_PATH;
}

std::string AbilityContext::GetCacheDir()
{
    // /data/storage/el2/base/haps/moduleName/cache
    return GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_CACHE;
}

std::string AbilityContext::GetTempDir()
{
    // /data/storage/el2/base/haps/moduleName/temp
    return GetBaseDir() + CONTEXT_TEMP;
}

std::string AbilityContext::GetFilesDir()
{
    // /data/storage/el2/base/haps/moduleName/files
    return GetBaseDir() + CONTEXT_FILES;
}

std::string AbilityContext::GetDatabaseDir()
{
    // /data/storage/el2/base/haps/moduleName
    return CONTEXT_DATA_STORAGE + CURRENT_AREA + CONTEXT_FILE_SEPARATOR + CONTEXT_DATABASE +
        CONTEXT_FILE_SEPARATOR + options_.moduleName;
}

std::string AbilityContext::GetPreferencesDir()
{
    // /data/storage/el2/base/haps/moduleName/preferences
    return GetBaseDir() + CONTEXT_FILE_SEPARATOR + CONTEXT_PREFERENCES;
}

std::string AbilityContext::GetDistributedFilesDir()
{
    // /data/storage/el2/distributedfiles
    return CONTEXT_DATA_STORAGE + CURRENT_AREA + CONTEXT_FILE_SEPARATOR + CONTEXT_DISTRIBUTEDFILES;
}

void AbilityContext::SwitchArea(int mode)
{
}

int AbilityContext::GetArea()
{
    return 0;
}

std::string AbilityContext::GetBaseDir()
{
    // /data/storage/el2/base/haps/moduleName
    return CONTEXT_DATA_STORAGE + CURRENT_AREA + CONTEXT_FILE_SEPARATOR + CONTEXT_BASE +
        CONTEXT_HAPS + CONTEXT_FILE_SEPARATOR + options_.moduleName;
}
} // namespace AbilityRuntime
} // namespace OHOS
