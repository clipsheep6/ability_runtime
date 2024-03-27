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

#include "js_startup_task.h"

#include "ability_runtime_error_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
JsStartupTask::JsStartupTask(const std::string &name, JsRuntime &jsRuntime,
    std::shared_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef)
    : StartupTask(name), jsRuntime_(jsRuntime), startupJsRef_(startupJsRef), contextJsRef_(contextJsRef) {}

JsStartupTask::~JsStartupTask() = default;

int32_t JsStartupTask::Init()
{
    // init dependencies_, isManualDispatch_, callCreateOnMainThread_, waitOnMainThread_, isAutoStartup_
    HILOG_DEBUG("%{public}s, dump: %{public}d%{public}d%{public}d%{public}d, dep: %{public}s", name_.c_str(),
        isManualDispatch_, callCreateOnMainThread_, waitOnMainThread_, isAutoStartup_, DumpDependencies().c_str());
    return ERR_OK;
}

int32_t JsStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    if (state_ != State::CREATED) {
        HILOG_ERROR("%{public}s, state is wrong %{public}d", name_.c_str(), static_cast<int32_t>(state_));
        return ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR;
    }
    state_ = State::INITIALIZING;
    callback->Push([weak = weak_from_this()](const std::shared_ptr<StartupTaskResult> &result) {
        auto startupTask = weak.lock();
        if (startupTask == nullptr) {
            HILOG_ERROR("startupTask is nullptr.");
            return;
        }
        startupTask->SaveResult(result);
    });
    HILOG_DEBUG("%{public}s, RunOnMainThread", name_.c_str());
    return JsStartupTaskExecutor::RunOnMainThread(jsRuntime_, startupJsRef_, contextJsRef_, std::move(callback));
}
} // namespace AbilityRuntime
} // namespace OHOS
