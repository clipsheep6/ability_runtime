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

#ifndef OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_EXECUTOR_H

#include "ability_manager_errors.h"
#include "js_runtime.h"
#include "js_startup_task_result.h"

namespace OHOS {
namespace AbilityRuntime {
class JsStartupTaskExecutor {
public:
    static int32_t RunOnMainThread(JsRuntime &jsRuntime,
        const std::shared_ptr<NativeReference> &startup, const std::shared_ptr<NativeReference> &context,
        std::unique_ptr<StartupTaskResultCallback> callback);

    static int32_t RunOnTaskPool(JsRuntime &jsRuntime,
        const std::shared_ptr<NativeReference> &startup, const std::shared_ptr<NativeReference> &context,
        std::unique_ptr<StartupTaskResultCallback> callback);

private:
    static int32_t CallStartupInit(napi_env env, const std::shared_ptr<NativeReference> &startup,
        const std::shared_ptr<NativeReference> &context, std::unique_ptr<StartupTaskResultCallback> &callback,
        napi_value &returnVal);

    static int32_t HandleReturnVal(napi_env env, napi_value returnVal,
        std::unique_ptr<StartupTaskResultCallback> &callback);

    static napi_value ResolveResultCallback(napi_env env, napi_callback_info info);

    static napi_value RejectResultCallback(napi_env env, napi_callback_info info);

    static void ReplyFailed(StartupTaskResultCallback *callback,
        int32_t resultCode, const std::string &resultMessage = "");

    static void ReplyFailed(std::unique_ptr<StartupTaskResultCallback> callback,
        int32_t resultCode, const std::string &resultMessage = "");

    static void ReplySucceeded(StartupTaskResultCallback *callback,
        const std::shared_ptr<NativeReference> &resultRef);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_STARTUP_TASK_EXECUTOR_H
