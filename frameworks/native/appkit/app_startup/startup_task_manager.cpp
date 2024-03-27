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

#include "startup_task_manager.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

StartupTaskManager::StartupTaskManager(uint32_t startupTaskManagerId) : startupTaskManagerId_(startupTaskManagerId)
{}

StartupTaskManager::~StartupTaskManager() = default;

int32_t StartupTaskManager::AddTask(const std::shared_ptr<StartupTask> &task)
{
    if (task == nullptr) {
        HILOG_ERROR("Invalid task.");
        return ERR_INVALID_VALUE;
    }
    std::string name = task->GetName();
    auto result = tasks_.emplace(name, task);
    if (!result.second) {
        HILOG_ERROR("Failed to add task, name: %{public}s already exist.", name.c_str());
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t StartupTaskManager::SetConfig(const std::shared_ptr<StartupConfig> &config)
{
    config_ = config;
    return ERR_OK;
}

int32_t StartupTaskManager::Prepare()
{
    HILOG_DEBUG("id: %{public}u, task number: %{public}zu", startupTaskManagerId_, tasks_.size());
    // sort
    dispatcher_ = std::make_shared<StartupTaskDispatcher>();
    return ERR_OK;
}

int32_t StartupTaskManager::Run()
{
    HILOG_DEBUG("id: %{public}u, task number: %{public}zu", startupTaskManagerId_, tasks_.size());
    for (auto &iter : tasks_) {
        if (iter.second == nullptr) {
            continue;
        }
        std::unique_ptr<StartupTaskResultCallback> callback = std::make_unique<StartupTaskResultCallback>();
        iter.second->RunTaskInit(std::move(callback));
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
