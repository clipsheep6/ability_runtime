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

#include "hilog_wrapper.h"
#include "js_startup_task_main_thread_executor.h"

namespace OHOS {
namespace AbilityRuntime {
JsStartupTask::JsStartupTask(const std::string &name, JsRuntime &jsRuntime,
    std::shared_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef)
    : StartupTask(name), jsRuntime_(jsRuntime), startupJsRef_(startupJsRef), contextJsRef_(contextJsRef) {}

JsStartupTask::~JsStartupTask() = default;

int32_t JsStartupTask::Init()
{
    // init dependencies_, isManualDispatch_, callCreateOnMainThread_, waitOnMainThread_, isAutoStartup_
    return ERR_OK;
}

int32_t JsStartupTask::RunTaskInit()
{
    HILOG_DEBUG("run startup task: %{public}s", name_.c_str());
    executor_ = std::make_shared<JsStartupTaskMainThreadExecutor>();
    executor_->Run(jsRuntime_);
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
