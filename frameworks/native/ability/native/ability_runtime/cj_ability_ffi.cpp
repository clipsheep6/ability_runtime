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

#include "ability_runtime/cj_ability_ffi.h"

#include "ability_runtime/cj_ui_ability.h"
#include "ability_runtime/cj_ability_context.h"
#include "cj_common_ffi.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
extern "C" {
int64_t FFIAbilityGetAbilityContext(AbilityHandle abilityHandle)
{
    if (abilityHandle == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability handle is null");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto ability = static_cast<CJUIAbility*>(abilityHandle);
    auto context = ability->GetAbilityContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext failed, abilityContext is null");
        return ERR_INVALID_INSTANCE_CODE;
    }
    auto cjContext = FFI::FFIData::Create<CJAbilityContext>(context);
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetAbilityContext failed, abilityContext is null");
        return ERR_INVALID_INSTANCE_CODE;
    }
    return cjContext->GetID();
}

void FFIAbilityContextGetFilesDir(int64_t id, void(*accept)(const char*))
{
    auto cjContext = FFI::FFIData::GetData<CJAbilityContext>(id);
    if (cjContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj abilityContext is null");
        return;
    }
    auto context = cjContext->GetAbilityContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "context is null");
        return;
    }
    auto filesDir = context->GetFilesDir();
    accept(filesDir.c_str());
}
}
} // namespace AbilityRuntime
} // namespace OHOS
