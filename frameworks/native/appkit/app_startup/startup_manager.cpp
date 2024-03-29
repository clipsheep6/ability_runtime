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
        return ERR_STARTUP_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t StartupManager::BuildAutoStartupTaskManager(std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    startupTaskManager = std::make_shared<StartupTaskManager>(startupTaskManagerId);
    for (auto &iter : startupTasks_) {
        if (iter.second->GetIsExcludeFromAutoStart()) {
            continue;
        }
        startupTaskManager->AddTask(iter.second);
    }
    startupTaskManager->SetConfig(defaultConfig_);
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    startupTaskManagerId++;
    return ERR_OK;
}

int32_t StartupManager::OnStartupTaskManagerComplete(uint32_t id)
{
    auto result = startupTaskManagerMap_.find(id);
    if (result == startupTaskManagerMap_.end()) {
        HILOG_ERROR("StartupTaskManager id: %{public}u not found.", id);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    HILOG_DEBUG("erase StartupTaskManager id: %{public}u", id);
    startupTaskManagerMap_.erase(result);
    return ERR_OK;
}

void StartupManager::SetDefaultConfig(const std::shared_ptr<StartupConfig> &config)
{
    defaultConfig_ = config;
}

std::shared_ptr<StartupConfig> StartupManager::GetDefaultConfig() const
{
    return defaultConfig_;
}

int32_t StartupManager::RemoveAllResult()
{
    HILOG_DEBUG("called.");
    for (auto &iter : startupTasks_) {
        if (iter.second != nullptr) {
            iter.second->RemoveResult();
        }
    }
    return ERR_OK;
}

int32_t StartupManager::RemoveResult(const std::string &name)
{
    HILOG_DEBUG("called, name: %{public}s", name.c_str());
    auto findResult = startupTasks_.find(name);
    if (findResult == startupTasks_.end() || findResult->second == nullptr) {
        HILOG_ERROR("name: %{public}s, not found", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    findResult->second->RemoveResult();
    return ERR_OK;
}

int32_t StartupManager::GetResult(const std::string &name, std::shared_ptr<StartupTaskResult> &result)
{
    HILOG_DEBUG("called, name: %{public}s", name.c_str());
    auto findResult = startupTasks_.find(name);
    if (findResult == startupTasks_.end() || findResult->second == nullptr) {
        HILOG_ERROR("name: %{public}s, not found", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    StartupTask::State state = findResult->second->GetState();
    if (state != StartupTask::State::INITIALIZED) {
        HILOG_ERROR("name: %{public}s, not initialized", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    result = findResult->second->GetResult();
    return ERR_OK;
}

int32_t StartupManager::IsInitialized(const std::string &name, bool &isInitialized)
{
    HILOG_DEBUG("called, name: %{public}s", name.c_str());
    auto findResult = startupTasks_.find(name);
    if (findResult == startupTasks_.end() || findResult->second == nullptr) {
        HILOG_ERROR("name: %{public}s, not found", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    StartupTask::State state = findResult->second->GetState();
    isInitialized = state == StartupTask::State::INITIALIZED;
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
