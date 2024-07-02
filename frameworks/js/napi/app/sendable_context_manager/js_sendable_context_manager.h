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

#ifndef OHOS_ABILITY_RUNTIME_JS_SENDABLE_CONTEXT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_SENDABLE_CONTEXT_MANAGER_H

#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class Context;
napi_value CreateJsSendableContextManager(napi_env env, napi_value exportObj);
napi_value CreateSendableContextObject(napi_env env, std::shared_ptr<Context> context);
napi_value CreateJsBaseContextFromSendable(napi_env env, void* wrapped);
napi_value CreateJsApplicationContextFromSendable(napi_env env, void* wrapped);
napi_value CreateJsAbilityStageContextFromSendable(napi_env env, void* wrapped);
napi_value CreateJsUIAbilityContextFromSendable(napi_env env, void* wrapped);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_SENDABLE_CONTEXT_MANAGER_H
