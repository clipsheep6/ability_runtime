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

#ifndef OHOS_ABILITY_RUNTIME_JS_STARTUP_CONFIG_H
#define OHOS_ABILITY_RUNTIME_JS_STARTUP_CONFIG_H

#include <memory>

#include "js_runtime.h"
#include "startup_config.h"
#include "startup_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class JsStartupConfig : public StartupConfig {
public:
    JsStartupConfig(napi_env env);

    ~JsStartupConfig() override;

    int32_t Init(std::unique_ptr<NativeReference> &configEntryJsRef);

    int32_t Init(napi_value config);

    static napi_value GetConfigFromEntry(napi_env env, std::unique_ptr<NativeReference> &configEntryJsRef);

    static napi_value BuildResult(napi_env env, const std::shared_ptr<StartupTaskResult> &result);

private:
    napi_env env_;

    void InitAwaitTimeout(napi_env env, napi_value config);
    void InitListener(napi_env env, napi_value config);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_STARTUP_CONFIG_H
