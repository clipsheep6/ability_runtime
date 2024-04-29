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

#ifndef FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_CONTEXT_H
#define FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_CONTEXT_H

#include <memory>
#include <mutex>

#include "application_info.h"
#include "configuration.h"
#include "hap_module_info.h"
#include "options.h"

namespace OHOS {
namespace AbilityRuntime {
class Context {
public:
    Context() = default;
    ~Context() = default;

    virtual std::string GetBundleName() = 0;

    virtual std::string GetBundleCodePath() = 0;

    virtual std::string GetBundleCodeDir() = 0;

    virtual std::string GetCacheDir() = 0;

    virtual std::string GetTempDir() = 0;

    virtual std::string GetResourceDir() = 0;

    virtual std::string GetFilesDir() = 0;

    virtual std::string GetDatabaseDir() = 0;

    virtual std::string GetPreferencesDir() = 0;

    virtual std::string GetDistributedFilesDir() = 0;

    virtual std::string GetCloudFileDir() = 0;

    virtual void SwitchArea(int mode) = 0;

    virtual int GetArea() = 0;

    virtual std::string GetBaseDir() = 0;

    virtual std::shared_ptr<AppExecFwk::Configuration> GetConfiguration() = 0;

    virtual Options GetOptions() = 0;

    virtual void SetOptions(const Options &options) = 0;

    virtual std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const = 0;

    virtual std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_CONTEXT_H
