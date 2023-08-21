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
#include "native_engine/native_engine.h"
#include "running_process_info.h"

namespace OHOS {
namespace AbilityRuntime {
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
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    static NativeValue* RegisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* UnregisterAbilityLifecycleCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* RegisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* UnregisterEnvironmentCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* On(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* Off(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* CreateBundleContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* SwitchArea(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetArea(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info);

    NativeValue* OnRegisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUnregisterAbilityLifecycleCallback(NativeEngine &engine, NativeCallbackInfo &info);

    NativeValue* OnRegisterEnvironmentCallback(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnUnregisterEnvironmentCallback(NativeEngine &engine, NativeCallbackInfo &info);

    NativeValue* OnOn(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnOff(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue* OnOnAbilityLifecycle(NativeEngine &engine, NativeCallbackInfo &info, bool isSync);
    NativeValue* OnOffAbilityLifecycle(NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId);
    NativeValue* OnOffAbilityLifecycleEventSync(NativeEngine &engine,
        const NativeCallbackInfo &info, int32_t callbackId);
    NativeValue* OnOnEnvironment(NativeEngine &engine, NativeCallbackInfo &info, bool isSync);
    NativeValue* OnOffEnvironment(NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId);
    NativeValue* OnOffEnvironmentEventSync(
        NativeEngine &engine, const NativeCallbackInfo &info, int32_t callbackId);
    NativeValue* OnOnApplicationStateChange(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnOffApplicationStateChange(NativeEngine &engine, const NativeCallbackInfo &info);

    NativeValue* OnGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetTempDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetDistributedFilesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetDatabaseDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetPreferencesDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetGroupDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetBundleCodeDir(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnKillProcessBySelf(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetRunningProcessInformation(NativeEngine &engine, NativeCallbackInfo &info);

    static NativeValue* GetCacheDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetTempDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetDistributedFilesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetDatabaseDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetPreferencesDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetGroupDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetBundleCodeDir(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetApplicationContext(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* KillProcessBySelf(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* GetRunningProcessInformation(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* CreateJsApplicationContext(NativeEngine &engine);

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    NativeValue* OnCreateBundleContext(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnSwitchArea(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue* OnGetArea(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);
    bool CheckCallerIsSystemApp();
    static void BindNativeApplicationContext(NativeEngine &engine, NativeObject* object);
    static JsAppProcessState ConvertToJsAppProcessState(
        const AppExecFwk::AppProcessState &appProcessState, const bool &isFocused);
    std::shared_ptr<JsAbilityLifecycleCallback> callback_;
    std::shared_ptr<JsEnvironmentCallback> envCallback_;
    std::shared_ptr<JsApplicationStateChangeCallback> applicationStateCallback_;
    std::mutex applicationStateCallbackLock_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_APPLICATION_CONTEXT_UTILS_H
