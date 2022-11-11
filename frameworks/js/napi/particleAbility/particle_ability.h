/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PARTICLE_ABILITY_H
#define OHOS_ABILITY_RUNTIME_PARTICLE_ABILITY_H

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "js_runtime_utils.h"
#include "js_napi_common_ability.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief ParticleAbility NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
class JsParticleAbility : public JsNapiCommon {
public:
    JsParticleAbility() = default;
    ~JsParticleAbility() = default;
    Ability* GetAbility(napi_env env);
    static void Finalizer(NativeEngine *engine, void *data, void *hint);
    static NativeValue* PAConnectAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* PADisConnectAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* PAStartAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* PATerminateAbility(NativeEngine *engine, NativeCallbackInfo *info);
    static NativeValue* AcquireDataAbilityHelper(NativeEngine *engine, NativeCallbackInfo *info);
};

NativeValue* JsParticleAbilityInit(NativeEngine *engine, NativeValue *exportObj);
napi_value ParticleAbilityInit(napi_env env, napi_value exports);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_PARTICLE_ABILITY_H */
