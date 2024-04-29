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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_H

#include "js_ability_auto_startup_callback.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace AbilityRuntime {
class JsAbilityAutoStartupManager {
public:
    JsAbilityAutoStartupManager() = default;
    ~JsAbilityAutoStartupManager() = default;
    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value RegisterAutoStartupCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterAutoStartupCallback(napi_env env, napi_callback_info info);
    static napi_value SetApplicationAutoStartup(napi_env env, napi_callback_info info);
    static napi_value CancelApplicationAutoStartup(napi_env env, napi_callback_info info);
    static napi_value QueryAllAutoStartupApplications(napi_env env, napi_callback_info info);

private:
    napi_value OnRegisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info);
    napi_value OnUnregisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info);
    napi_value OnSetApplicationAutoStartup(napi_env env, NapiCallbackInfo &info);
    napi_value OnCancelApplicationAutoStartup(napi_env env, NapiCallbackInfo &info);
    napi_value OnQueryAllAutoStartupApplications(napi_env env, const NapiCallbackInfo &info);
    static bool CheckCallerIsSystemApp();

    sptr<JsAbilityAutoStartupCallBack> jsAutoStartupCallback_;
};
napi_value JsAbilityAutoStartupManagerInit(napi_env env, napi_value exportObj);
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_H