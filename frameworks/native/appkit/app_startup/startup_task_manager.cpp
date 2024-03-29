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

#include "hilog_wrapper.h"
#include "startup_manager.h"
#include "startup_topologysort.h"

namespace OHOS {
namespace AbilityRuntime {
StartupTaskManager::StartupTaskManager(uint32_t startupTaskManagerId) : startupTaskManagerId_(startupTaskManagerId)
{}

StartupTaskManager::~StartupTaskManager()
{
    HILOG_DEBUG("id: %{public}u deconstruct", startupTaskManagerId_);
}

int32_t StartupTaskManager::AddTask(const std::shared_ptr<StartupTask> &task)
{
    if (task == nullptr) {
        HILOG_ERROR("Invalid task.");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::string name = task->GetName();
    auto result = tasks_.emplace(name, task);
    if (!result.second) {
        HILOG_ERROR("Failed to add task, name: %{public}s already exist.", name.c_str());
        return ERR_STARTUP_INVALID_VALUE;
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
    std::shared_ptr<StartupSortResult> startupSortResult = nullptr;
    int32_t result = StartupTopologySort::Sort(tasks_, startupSortResult);
    if (result != ERR_OK) {
        CallListenerOnCompleted(result);
        return result;
    }
    if (startupSortResult == nullptr) {
        HILOG_ERROR("startupSortResult is nullptr.");
        CallListenerOnCompleted(ERR_STARTUP_INTERNAL_ERROR);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    dispatcher_ = std::make_shared<StartupTaskDispatcher>(tasks_, startupSortResult);
    return ERR_OK;
}

int32_t StartupTaskManager::Run(const std::shared_ptr<OnCompletedCallback> &mainThreadAwaitCallback)
{
    HILOG_DEBUG("id: %{public}u, task number: %{public}zu", startupTaskManagerId_, tasks_.size());
    if (dispatcher_ == nullptr) {
        HILOG_ERROR("dispatcher_ is nullptr.");
        CallListenerOnCompleted(ERR_STARTUP_INTERNAL_ERROR);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    AddAsyncTimeoutTimer();
    auto completedCallback = std::make_shared<OnCompletedCallback>(
        [weak = weak_from_this()](const std::shared_ptr<StartupTaskResult> &result) {
            auto startupTaskManager = weak.lock();
            if (startupTaskManager == nullptr) {
                HILOG_ERROR("startupTaskManager is nullptr.");
                return;
            }
            startupTaskManager->CancelAsyncTimeoutTimer();
            if (result == nullptr) {
                HILOG_ERROR("result is nullptr.");
                return;
            }
            startupTaskManager->CallListenerOnCompleted(result->GetResultCode(), result->GetResultMessage());
        });

    int32_t result = dispatcher_->Run(completedCallback, mainThreadAwaitCallback);
    if (result != ERR_OK) {
        CancelAsyncTimeoutTimer();
        if (!completedCallback->IsCalled()) {
            CallListenerOnCompleted(result);
        }
        return result;
    }
    return ERR_OK;
}

void StartupTaskManager::CallListenerOnCompleted(int32_t result, const std::string &resultMessage)
{
    if (config_ == nullptr) {
        HILOG_INFO("id: %{public}u, config is null, result: %{public}d", startupTaskManagerId_, result);
        return;
    }
    HILOG_DEBUG("id: %{public}u, complete, result: %{public}d", startupTaskManagerId_, result);
    if (resultMessage.empty()) {
        auto startupTaskResult = std::make_shared<StartupTaskResult>(result, StartupUtils::GetErrorMessage(result));
        config_->ListenerOnCompleted(startupTaskResult);
    } else {
        auto startupTaskResult = std::make_shared<StartupTaskResult>(result, resultMessage);
        config_->ListenerOnCompleted(startupTaskResult);
    }
    DelayedSingleton<StartupManager>::GetInstance()->OnStartupTaskManagerComplete(startupTaskManagerId_);
}

void StartupTaskManager::AddAsyncTimeoutTimer()
{
}

void StartupTaskManager::CancelAsyncTimeoutTimer()
{
}
} // namespace AbilityRuntime
} // namespace OHOS
