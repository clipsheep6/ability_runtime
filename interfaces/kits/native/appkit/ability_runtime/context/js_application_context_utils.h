/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_UTILS_H

#include <memory>
#include <mutex>

#include "ability_lifecycle_callback.h"
#include "application_context.h"
#include "application_state_change_callback.h"
#include "js_ability_auto_startup_callback.h"
#include "native_engine/native_engine.h"
#include "running_process_info.h"

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;
namespace {
enum JsAppProcessState {
    STATE_CREATE,
    STATE_FOREGROUND,
    STATE_ACTIVE,
    STATE_BACKGROUND,
    STATE_DESTROY
};
}
class JsApplicationContextUtils {
public:
    explicit JsApplicationContextUtils(std::weak_ptr<ApplicationContext> &&applicationContext)
        : applicationContext_(std::move(applicationContext))
    {
    }
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
    static napi_value SetAutoStartup(napi_env env, napi_callback_info info);
    static napi_value CancelAutoStartup(napi_env env, napi_callback_info info);
    static napi_value IsAutoStartup(napi_env env, napi_callback_info info);

    napi_value OnRegisterAbilityLifecycleCallback(napi_env env, NapiCallbackInfo& info);
    napi_value OnUnregisterAbilityLifecycleCallback(napi_env env, NapiCallbackInfo& info);

    napi_value OnRegisterEnvironmentCallback(napi_env env, NapiCallbackInfo& info);
    napi_value OnUnregisterEnvironmentCallback(napi_env env, NapiCallbackInfo& info);

    napi_value OnOn(napi_env env, NapiCallbackInfo& info);
    napi_value OnOff(napi_env env, NapiCallbackInfo& info);
    napi_value OnOnAbilityLifecycle(napi_env env, NapiCallbackInfo& info, bool isSync);
    napi_value OnOffAbilityLifecycle(napi_env env, NapiCallbackInfo& info, int32_t callbackId);
    napi_value OnOffAbilityLifecycleEventSync(napi_env env, NapiCallbackInfo& info, int32_t callbackId);
    napi_value OnOnEnvironment(napi_env env, NapiCallbackInfo& info, bool isSync);
    napi_value OnOffEnvironment(napi_env env, NapiCallbackInfo& info, int32_t callbackId);
    napi_value OnOffEnvironmentEventSync(
        napi_env env, NapiCallbackInfo& info, int32_t callbackId);
    napi_value OnOnApplicationStateChange(napi_env env, NapiCallbackInfo& info);
    napi_value OnOffApplicationStateChange(napi_env env, NapiCallbackInfo& info);

    napi_value OnGetCacheDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetTempDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetResourceDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDistributedFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetGroupDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnKillProcessBySelf(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetRunningProcessInformation(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetColorMode(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetLanguage(napi_env env, NapiCallbackInfo& info);
    napi_value OnRegisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info);
    napi_value OnUnregisterAutoStartupCallback(napi_env env, NapiCallbackInfo &info);
    napi_value OnSetAutoStartup(napi_env env, NapiCallbackInfo &info);
    napi_value OnCancelAutoStartup(napi_env env, NapiCallbackInfo &info);
    napi_value OnIsAutoStartup(napi_env env, NapiCallbackInfo &info);
    napi_value OnClearUpApplicationData(napi_env env, NapiCallbackInfo& info);

    static napi_value GetCacheDir(napi_env env, napi_callback_info info);
    static napi_value GetTempDir(napi_env env, napi_callback_info info);
    static napi_value GetResourceDir(napi_env env, napi_callback_info info);
    static napi_value GetFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDistributedFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDatabaseDir(napi_env env, napi_callback_info info);
    static napi_value GetPreferencesDir(napi_env env, napi_callback_info info);
    static napi_value GetGroupDir(napi_env env, napi_callback_info info);
    static napi_value GetBundleCodeDir(napi_env env, napi_callback_info info);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value KillProcessBySelf(napi_env env, napi_callback_info info);
    static napi_value SetColorMode(napi_env env, napi_callback_info info);
    static napi_value SetLanguage(napi_env env, napi_callback_info info);
    static napi_value ClearUpApplicationData(napi_env env, napi_callback_info info);
    static napi_value GetRunningProcessInformation(napi_env env, napi_callback_info info);
    static napi_value CreateJsApplicationContext(napi_env env);

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    napi_value OnCreateBundleContext(napi_env env, NapiCallbackInfo& info);
    napi_value OnCreateModuleResourceManager(napi_env env, NapiCallbackInfo& info);
    napi_value OnSwitchArea(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetArea(napi_env env, NapiCallbackInfo& info);
    napi_value OnCreateModuleContext(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo& info);
    bool CheckCallerIsSystemApp();
    static void BindNativeApplicationContext(napi_env env, napi_value object);
    static JsAppProcessState ConvertToJsAppProcessState(
        const AppExecFwk::AppProcessState &appProcessState, const bool &isFocused);
    std::shared_ptr<JsAbilityLifecycleCallback> callback_;
    std::shared_ptr<JsEnvironmentCallback> envCallback_;
    std::shared_ptr<JsApplicationStateChangeCallback> applicationStateCallback_;
    std::mutex applicationStateCallbackLock_;
    sptr<JsAbilityAutoStartupCallBack> jsAutoStartupCallback_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_UTILS_H
