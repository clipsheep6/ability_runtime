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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_JS_APPLICATION_CONTEXT_UTILS_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_JS_APPLICATION_CONTEXT_UTILS_H

#include <memory>

#include "native_engine/native_engine.h"
#include "context.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
class JsApplicationContextUtils {
public:
    JsApplicationContextUtils(std::weak_ptr<Context> &&context) : context_(std::move(context)) {}
    virtual ~JsApplicationContextUtils() = default;
    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value RegisterAbilityLifecycleCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterAbilityLifecycleCallback(napi_env env, napi_callback_info info);
    static napi_value RegisterEnvironmentCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterEnvironmentCallback(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value CreateBundleContext(napi_env env, napi_callback_info info);
    static napi_value SwitchArea(napi_env env, napi_callback_info info);
    static napi_value GetArea(napi_env env, napi_callback_info info);
    static napi_value CreateModuleContext(napi_env env, napi_callback_info info);
    static napi_value CreateModuleResourceManager(napi_env env, napi_callback_info info);
    static napi_value GetCacheDir(napi_env env, napi_callback_info info);
    static napi_value GetTempDir(napi_env env, napi_callback_info info);
    static napi_value GetFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDistributedFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDatabaseDir(napi_env env, napi_callback_info info);
    static napi_value GetPreferencesDir(napi_env env, napi_callback_info info);
    static napi_value GetBundleCodeDir(napi_env env, napi_callback_info info);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value KillProcessBySelf(napi_env env, napi_callback_info info);
    static napi_value GetRunningProcessInformation(napi_env env, napi_callback_info info);
    static napi_value CreateJsApplicationContext(napi_env env, const std::shared_ptr<Context> &context);

    napi_value OnGetCacheDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetTempDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetArea(napi_env env, NapiCallbackInfo& info);

private:
    napi_value OnSwitchArea(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo& info);
    static void BindNativeApplicationContext(napi_env env, napi_value object);

private:
    std::weak_ptr<Context> context_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULATOR_JS_APPLICATION_CONTEXT_UTILS_H
