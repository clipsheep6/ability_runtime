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
class JsApplicationContextAutoStartupManager {
public:
    JsApplicationContextAutoStartupManager() = default;
    ~JsApplicationContextAutoStartupManager() = default;
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    static NativeValue *RegisterAutoStartupCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *UnregisterAutoStartupCallback(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *SetAutoStartup(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *CancelAutoStartup(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *IsAutoStartup(NativeEngine *engine, NativeCallbackInfo *info);

private:
    NativeValue *OnRegisterAutoStartupCallback(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnUnregisterAutoStartupCallback(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnSetAutoStartup(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnCancelAutoStartup(NativeEngine &engine, const NativeCallbackInfo &info);
    NativeValue *OnIsAutoStartup(NativeEngine &engine, const NativeCallbackInfo &info);

    sptr<JsAbilityAutoStartupCallBack> jsAutoStartupCallback_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_AUTO_STARTUP_MANAGER_H