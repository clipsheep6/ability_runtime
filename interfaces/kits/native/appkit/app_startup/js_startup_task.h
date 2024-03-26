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

#ifndef OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_H
#define OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_H

#include "ability_manager_errors.h"
#include "js_runtime.h"
#include "js_startup_task_executor.h"
#include "startup_task.h"

namespace OHOS {
namespace AbilityRuntime {
class JsStartupTask : public StartupTask {
public:
    JsStartupTask(const std::string &name, JsRuntime &jsRuntime,
        std::shared_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef_);

    ~JsStartupTask() override;

    int32_t Init();

    int32_t RunTaskInit() override;

private:
    JsRuntime &jsRuntime_;
    std::shared_ptr<NativeReference> startupJsRef_;
    std::shared_ptr<NativeReference> contextJsRef_;
    std::shared_ptr<JsStartupTaskExecutor> executor_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_H
