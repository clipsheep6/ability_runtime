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


namespace OHOS {
namespace AbilityRuntime {
StartupTask::StartupTask(const std::string &name) : name_(name)
{}

StartupTask::~StartupTask() = default;

std::string StartupTask::GetName() const
{
    return name_;
}

std::vector<std::string> StartupTask::GetDependencies() const
{
    return dependencies_;
}

bool StartupTask::GetIsManualDispatch() const
{
    return isManualDispatch_;
}

bool StartupTask::GetCallCreateOnMainThread() const
{
    return callCreateOnMainThread_;
}

bool StartupTask::GetWaitOnMainThread() const
{
    return waitOnMainThread_;
}

bool StartupTask::GetIsAutoStartup() const
{
    return isAutoStartup_;
}

void StartupTask::SetDependencies(const std::vector<std::string> &dependencies)
{
    dependencies_ = dependencies;
}

void StartupTask::SetIsManualDispatch(bool isManualDispatch)
{
    isManualDispatch_ = isManualDispatch;
}

void StartupTask::SetCallCreateOnMainThread(bool callCreateOnMainThread)
{
    callCreateOnMainThread_ = callCreateOnMainThread;
}

void StartupTask::SetWaitOnMainThread(bool waitOnMainThread)
{
    waitOnMainThread_ = waitOnMainThread;
}

void StartupTask::SetIsAutoStartup(bool isAutoStartup)
{
    isAutoStartup = isAutoStartup_;
}
} // namespace AbilityRuntime
} // namespace OHOS
