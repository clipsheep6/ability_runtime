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

#include <set>

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
StartupManager::StartupManager()
{
    mainHandler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
}

StartupManager::~StartupManager() = default;

int32_t StartupManager::RegisterStartupTask(const std::string &name, const std::shared_ptr<StartupTask> &startupTask)
{
    auto result = startupTasks_.emplace(name, startupTask);
    if (!result.second) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to register startup task, name: %{public}s already exist.", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t StartupManager::BuildAutoStartupTaskManager(std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::map<std::string, std::shared_ptr<StartupTask>> autoStartupTasks;
    std::set<std::string> dependenciesSet;
    for (auto &iter : startupTasks_) {
        if (iter.second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task is null");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (iter.second->GetIsExcludeFromAutoStart()) {
            continue;
        }
        autoStartupTasks.emplace(iter.first, iter.second);
        auto dependencies = iter.second->GetDependencies();
        for (auto &dep : dependencies) {
            dependenciesSet.insert(dep);
        }
    }
    for (auto &dep : dependenciesSet) {
        if (autoStartupTasks.find(dep) != autoStartupTasks.end()) {
            continue;
        }
        TAG_LOGI(AAFwkTag::STARTUP, "try to add excludeFromAutoStart task: %{public}s", dep.c_str());
        AddStartupTask(dep, autoStartupTasks);
    }

    TAG_LOGD(AAFwkTag::STARTUP, "autoStartupTasksManager build, id: %{public}u, tasks num: %{public}zu",
        startupTaskManagerId, autoStartupTasks.size());
    startupTaskManager = std::make_shared<StartupTaskManager>(startupTaskManagerId, autoStartupTasks);
    startupTaskManager->SetConfig(defaultConfig_);
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    startupTaskManagerId++;
    return ERR_OK;
}

int32_t StartupManager::BuildStartupTaskManager(const std::vector<std::string> &inputDependencies,
    std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::map<std::string, std::shared_ptr<StartupTask>> currentStartupTasks;
    std::set<std::string> dependenciesSet;
    for (auto &iter : inputDependencies) {
        auto findResult = startupTasks_.find(iter);
        if (findResult == startupTasks_.end()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s not found", iter.c_str());
            return ERR_STARTUP_DEPENDENCY_NOT_FOUND;
        }
        if (findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s startup task is null", iter.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        currentStartupTasks.emplace(iter, findResult->second);
        auto dependencies = findResult->second->GetDependencies();
        for (auto &dep : dependencies) {
            dependenciesSet.insert(dep);
        }
    }

    for (auto &dep : dependenciesSet) {
        if (currentStartupTasks.find(dep) != currentStartupTasks.end()) {
            continue;
        }
        AddStartupTask(dep, currentStartupTasks);
    }
    TAG_LOGD(AAFwkTag::STARTUP, "startupTasksManager build, id: %{public}u, tasks num: %{public}zu",
        startupTaskManagerId, currentStartupTasks.size());
    startupTaskManager = std::make_shared<StartupTaskManager>(startupTaskManagerId, currentStartupTasks);
    startupTaskManager->SetConfig(defaultConfig_);
    startupTaskManagerMap_.emplace(startupTaskManagerId, startupTaskManager);
    startupTaskManagerId++;
    return ERR_OK;
}

int32_t StartupManager::OnStartupTaskManagerComplete(uint32_t id)
{
    auto result = startupTaskManagerMap_.find(id);
    if (result == startupTaskManagerMap_.end()) {
        TAG_LOGE(AAFwkTag::STARTUP, "StartupTaskManager id: %{public}u not found.", id);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "erase StartupTaskManager id: %{public}u", id);
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
    TAG_LOGD(AAFwkTag::STARTUP, "called.");
    for (auto &iter : startupTasks_) {
        if (iter.second != nullptr) {
            iter.second->RemoveResult();
        }
    }
    return ERR_OK;
}

int32_t StartupManager::RemoveResult(const std::string &name)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called, name: %{public}s", name.c_str());
    auto findResult = startupTasks_.find(name);
    if (findResult == startupTasks_.end() || findResult->second == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "name: %{public}s, not found", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    return findResult->second->RemoveResult();
}

int32_t StartupManager::GetResult(const std::string &name, std::shared_ptr<StartupTaskResult> &result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called, name: %{public}s", name.c_str());
    auto findResult = startupTasks_.find(name);
    if (findResult == startupTasks_.end() || findResult->second == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "name: %{public}s, not found", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    StartupTask::State state = findResult->second->GetState();
    if (state != StartupTask::State::INITIALIZED) {
        TAG_LOGE(AAFwkTag::STARTUP, "name: %{public}s, not initialized", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    result = findResult->second->GetResult();
    return ERR_OK;
}

int32_t StartupManager::IsInitialized(const std::string &name, bool &isInitialized)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called, name: %{public}s", name.c_str());
    auto findResult = startupTasks_.find(name);
    if (findResult == startupTasks_.end() || findResult->second == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "name: %{public}s, not found", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
    }
    StartupTask::State state = findResult->second->GetState();
    isInitialized = state == StartupTask::State::INITIALIZED;
    return ERR_OK;
}

int32_t StartupManager::PostMainThreadTask(const std::function<void()> &task)
{
    if (mainHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to get mainHandler_");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    mainHandler_->PostTask(task);
    return ERR_OK;
}

int32_t StartupManager::AddStartupTask(const std::string &name,
    std::map<std::string, std::shared_ptr<StartupTask>> &taskMap)
{
    auto isAdded = taskMap.find(name);
    if (isAdded != taskMap.end()) {
        // already added
        return ERR_OK;
    }
    std::stack<std::string> taskStack;
    taskStack.push(name);
    while (!taskStack.empty()) {
        auto taskName = taskStack.top();
        taskStack.pop();
        auto findResult = startupTasks_.find(taskName);
        if (findResult == startupTasks_.end()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task not found %{public}s", taskName.c_str());
            return ERR_STARTUP_DEPENDENCY_NOT_FOUND;
        }
        taskMap.emplace(taskName, findResult->second);
        if (findResult->second == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup task is null, %{public}s", taskName.c_str());
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        auto dependencies = findResult->second->GetDependencies();
        for (auto &dep : dependencies) {
            if (taskMap.find(dep) == taskMap.end()) {
                taskStack.push(dep);
            }
        }
    }
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
