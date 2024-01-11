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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_CLEANER_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_CLEANER_H

#include <singleton.h>
#include <string>
#include <vector>

#include "application_context.h"

namespace OHOS {
namespace AppExecFwk {
class ApplicationCleaner final : public std::enable_shared_from_this<ApplicationCleaner>,
                                 public DelayedSingleton<ApplicationCleaner> {
public:
    ApplicationCleaner() {}
    ~ApplicationCleaner() {}

    void SetRuntimeContext(std::shared_ptr<AbilityRuntime::ApplicationContext> context)
    {
        context_ = context;
    }
    void RenameTempData();
    void ClearTempData();

private:
    int GetRootPath(std::vector<std::string> &rootPath);
    int GetObsoleteBundleTempPath(const std::vector<std::string> &rootPath, std::vector<std::string> &tempPath);
    bool RemoveDir(const std::string &tempPath);
    void TraverseObsoleteTempDirectory(const std::string &currentPath, std::vector<std::string> &tempDirs);

private:
    std::shared_ptr<AbilityRuntime::ApplicationContext> context_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APPLICATION_CLEANER_H
