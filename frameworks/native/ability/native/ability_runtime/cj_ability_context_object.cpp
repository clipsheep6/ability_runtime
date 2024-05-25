/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "cj_ability_context_object.h"

#include "hilog_wrapper.h"

// g_cjAbilityCallbacks is used to save cangjie functions.
// It is assigned by the global variable REGISTER_ABILITY_CONTEXT_CALLBACK on the cangjie side which invokes
// RegisterCJAbilityCallbacks. And it is never released.
CJAbilityCallbacks* g_cjAbilityCallbacks = nullptr;

void RegisterCJAbilityCallbacks(void (*registerFunc)(CJAbilityCallbacks*))
{
    if (g_cjAbilityCallbacks != nullptr) {
        HILOG_ERROR("Repeated registration for cangjie functions of CJAbilityStage.");
        return;
    }

    if (registerFunc == nullptr) {
        HILOG_ERROR("RegisterCJAbilityCallbacks failed, registerFunc is nullptr.");
        return;
    }

    g_cjAbilityCallbacks = new CJAbilityCallbacks();
    registerFunc(g_cjAbilityCallbacks);
}

namespace OHOS {
namespace AbilityRuntime {
CJAbilityCallbacks* GetGlobalCJAbilityCallbacks()
{
    return g_cjAbilityCallbacks;
}
} // namespace AbilityRuntime
} // namespace OHOS
