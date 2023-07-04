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

#ifndef OHOS_ABILITY_RUNTIME_SIMULAOTR_JS_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_SIMULAOTR_JS_ABILITY_CONTEXT_H

#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>
#include "ability_context.h"

class NativeObject;
class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class JsAbilityContext final {
public:
    JsAbilityContext() {}
    ~JsAbilityContext() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint);

    static NativeValue *StartAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartAbilityAsCaller(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartRecentAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartAbilityByCall(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartAbilityForResult(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartAbilityForResultWithAccount(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartServiceExtensionAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StartServiceExtensionAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StopServiceExtensionAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *StopServiceExtensionAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *ConnectAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *ConnectAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *DisconnectAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *TerminateSelf(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *TerminateSelfWithResult(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *RestoreWindowStage(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *RequestDialogService(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue *IsTerminating(NativeEngine *engine, NativeCallbackInfo *info);
};
NativeValue *CreateJsAbilityContext(NativeEngine &engine, const std::shared_ptr<AbilityContext> &context);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULAOTR_JS_ABILITY_CONTEXT_H
