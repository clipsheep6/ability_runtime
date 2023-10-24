/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_JS_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_JS_ABILITY_CONTEXT_H

#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>
#include "ability_context.h"
#include "configuration.h"
#include "js_runtime_utils.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsAbilityContext final {
public:
    explicit JsAbilityContext(const std::shared_ptr<AbilityContext> &context) : context_(context) {}
    ~JsAbilityContext() = default;

    static void Finalizer(napi_env env, void *data, void *hint);

    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info);
    static napi_value StartRecentAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByCall(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value StartAbilityForResultWithAccount(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value RestoreWindowStage(napi_env env, napi_callback_info info);
    static napi_value RequestDialogService(napi_env env, napi_callback_info info);
    static napi_value IsTerminating(napi_env env, napi_callback_info info);

    static void ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference> &jsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config);

private:
    napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo &info);
    napi_value OnIsTerminating(napi_env env, NapiCallbackInfo &info);
    napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo &info);

    std::weak_ptr<AbilityContext> context_;
};
napi_value CreateJsAbilityContext(napi_env env, const std::shared_ptr<AbilityContext> &context);
napi_value CreateJsErrorByNativeErr(napi_env env, int32_t err, const std::string &permission = "");
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_JS_ABILITY_CONTEXT_H
