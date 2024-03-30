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


#include "startup_task.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
StartupTask::StartupTask(const std::string &name) : name_(name)
{
    state_ = State::CREATED;
}

StartupTask::~StartupTask() = default;

std::string StartupTask::GetName() const
{
    return name_;
}

std::vector<std::string> StartupTask::GetDependencies() const
{
    return dependencies_;
}

bool StartupTask::GetCallCreateOnMainThread() const
{
    return callCreateOnMainThread_;
}

bool StartupTask::GetWaitOnMainThread() const
{
    return waitOnMainThread_;
}

bool StartupTask::GetIsExcludeFromAutoStart() const
{
    return isExcludeFromAutoStart_;
}

void StartupTask::SetDependencies(const std::vector<std::string> &dependencies)
{
    dependencies_ = dependencies;
}

void StartupTask::SetCallCreateOnMainThread(bool callCreateOnMainThread)
{
    callCreateOnMainThread_ = callCreateOnMainThread;
}

void StartupTask::SetWaitOnMainThread(bool waitOnMainThread)
{
    waitOnMainThread_ = waitOnMainThread;
}

void StartupTask::SetIsExcludeFromAutoStart(bool excludeFromAutoStart)
{
    isExcludeFromAutoStart_ = excludeFromAutoStart;
}

void StartupTask::SaveResult(const std::shared_ptr<StartupTaskResult> &result)
{
    if (result == nullptr) {
        HILOG_ERROR("startup task: %{public}s, result is null", name_.c_str());
        return;
    }
    HILOG_DEBUG("startup task: %{public}s, result code: %{public}d", name_.c_str(), result->GetResultCode());
    result_ = result;
    if (result->GetResultCode() == ERR_OK) {
        state_ = State::INITIALIZED;
    } else {
        state_ = State::CREATED;
    }
}

int32_t StartupTask::RemoveResult()
{
    if (state_ != State::INITIALIZED) {
        HILOG_ERROR("%{public}s, the result is not init", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    result_ = nullptr;
    state_ = State::CREATED;
    return ERR_OK;
}

std::shared_ptr<StartupTaskResult> StartupTask::GetResult() const
{
    return result_;
}

StartupTask::State StartupTask::GetState() const
{
    return state_;
}

std::string StartupTask::DumpDependencies() const
{
    if (dependencies_.empty()) {
        return "";
    }
    bool isFirst = true;
    std::string dumpResult;
    for (auto &iter : dependencies_) {
        if (isFirst) {
            dumpResult = iter;
            isFirst = false;
        } else {
            dumpResult += ", " + iter;
        }
    }
    return dumpResult;
}

uint32_t StartupTask::getDependenciesCount() const
{
    return dependencies_.size();
}

int32_t StartupTask::AddExtraCallback(std::unique_ptr<StartupTaskResultCallback> callback)
{
    if (state_ != State::INITIALIZING) {
        HILOG_ERROR("state is not INITIALIZING");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    // extra callback will called while init done
    extraCallbacks_.emplace_back(std::move(callback));
    return ERR_OK;
}

void StartupTask::CallExtraCallback(const std::shared_ptr<StartupTaskResult> &result)
{
    for (auto &callback : extraCallbacks_) {
        if (callback != nullptr) {
            callback->Call(result);
        }
    }
    extraCallbacks_.clear();
}
} // namespace AbilityRuntime
} // namespace OHOS
