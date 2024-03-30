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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_MANAGER_H

#include <map>

#include "js_startup_manager.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "startup_config.h"
#include "startup_manager.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace AbilityRuntime {
class JsStartupManager {
public:
    JsStartupManager() = default;

    ~JsStartupManager() = default;

    static void Finalizer(napi_env env, void *data, void *hint);

    static napi_value Run(napi_env env, napi_callback_info info);

    static napi_value RemoveAllResult(napi_env env, napi_callback_info info);

    static napi_value GetResult(napi_env env, napi_callback_info info);

    static napi_value IsInitialized(napi_env env, napi_callback_info info);

    static napi_value RemoveResult(napi_env env, napi_callback_info info);

private:
    napi_value OnRun(napi_env env, NapiCallbackInfo &info);

    napi_value OnRemoveAllResult(napi_env env, NapiCallbackInfo &info);

    napi_value OnGetResult(napi_env env, NapiCallbackInfo &info);

    napi_value OnIsInitialized(napi_env env, NapiCallbackInfo &info);

    napi_value OnRemoveResult(napi_env env, NapiCallbackInfo &info);

    int32_t GetDependencies(napi_env env, napi_value value, std::vector<std::string> &dependencies);

    int32_t GetConfig(napi_env env, napi_value value, std::shared_ptr<StartupConfig> &config);

    int32_t RunStartupTask(napi_env env, NapiCallbackInfo &info,
        std::shared_ptr<StartupTaskManager> &startupTaskManager);
};

napi_value JsStartupManagerInit(napi_env env, napi_value exportObj);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_MANAGER_H
