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
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    static NativeValue *RegisterAutoStartupCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *UnregisterAutoStartupCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *SetApplicationAutoStartup(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *CancelApplicationAutoStartup(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *QueryAllAutoStartupApplications(NativeEngine *engine, NativeCallbackInfo *info);

private:
    NativeValue *OnRegisterAutoStartupCallback(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnUnregisterAutoStartupCallback(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnSetApplicationAutoStartup(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnCancelApplicationAutoStartup(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnQueryAllAutoStartupApplications(NativeEngine &engine, const NativeCallbackInfo &info);
    bool CheckCallerIsSystemApp();

    sptr<JsAbilityAutoStartupCallBack> jsAutoStartupCallback_;
};
NativeValue *JsAbilityAutoStartupManagerInit(NativeEngine *engine, NativeValue *exportObj);
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_H