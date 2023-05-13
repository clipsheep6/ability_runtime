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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_BASE_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_NAPI_BASE_CONTEXT_H

#include <memory>

#include "napi/native_api.h"

namespace OHOS {
namespace AppExecFwk {
class Ability;
}

namespace AbilityRuntime {
    /**
     * @brief Get "stageMode" value of object.
     *
     * @param env NAPI environment.
     * @param object Native value contains "stageMode" object.
     * @param stageMode The value of "stageMode" object.
     * @return napi_status
     */
    napi_status IsStageContext(napi_env env, napi_value object, bool& stageMode);

    /**
     * @brief Get stage mode context.
     *
     * @param env NAPI environment.
     * @param object Native value of context.
     * @return The stage mode context.
     */
    class Context;
    std::shared_ptr<Context> GetStageModeContext(napi_env env, napi_value object);

    /**
     * @brief Get current ability.
     *
     * @param env NAPI environment.
     * @return The pointer of current ability.
     */
    AppExecFwk::Ability* GetCurrentAbility(napi_env env);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_BASE_CONTEXT_H
