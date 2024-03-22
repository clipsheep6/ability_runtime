/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "startup_manager.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"


namespace OHOS {
namespace AbilityRuntime {
StartupManager::StartupManager() = default;

StartupManager::~StartupManager() = default;

int32_t StartupManager::RegisterStartupTask(const std::string &name, const std::shared_ptr<StartupTask> &startupTask)
{
    auto result = startupTasks_.emplace(name, startupTask);
    if (!result.second) {
        HILOG_ERROR("Failed to register startup task, name: %{public}s already exist.", name.c_str());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t StartupManager::BuildAutoStartupTaskManager(std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
