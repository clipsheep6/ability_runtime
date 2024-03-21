/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ability_loader.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AbilityLoader &AbilityLoader::GetInstance()
{
    static AbilityLoader abilityLoader;
    return abilityLoader;
}

void AbilityLoader::RegisterAbility(const std::string &abilityName, const CreateAblity &createFunc)
{
    abilities_.insert_or_assign(abilityName, createFunc);
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityLoader::RegisterAbility:%{public}s", abilityName.c_str());
}

void AbilityLoader::RegisterExtension(const std::string &abilityName, const CreateExtension &createFunc)
{
    extensions_.emplace(abilityName, createFunc);
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityLoader::RegisterExtension:%{public}s", abilityName.c_str());
}

void AbilityLoader::RegisterUIAbility(const std::string &abilityName, const CreateUIAbility &createFunc)
{
    TAG_LOGD(AAFwkTag::ABILITY, "RegisterUIAbility: %{public}s", abilityName.c_str());
    uiAbilities_.emplace(abilityName, createFunc);
}

Ability *AbilityLoader::GetAbilityByName(const std::string &abilityName)
{
    auto it = abilities_.find(abilityName);
    if (it != abilities_.end()) {
        return it->second();
    }
    TAG_LOGE(AAFwkTag::ABILITY, "AbilityLoader::GetAbilityByName failed:%{public}s", abilityName.c_str());
    return nullptr;
}

AbilityRuntime::Extension *AbilityLoader::GetExtensionByName(const std::string &abilityName)
{
    auto it = extensions_.find(abilityName);
    if (it != extensions_.end()) {
        return it->second();
    }
    TAG_LOGE(AAFwkTag::ABILITY, "AbilityLoader::GetExtensionByName failed:%{public}s", abilityName.c_str());
    return nullptr;
}

AbilityRuntime::UIAbility *AbilityLoader::GetUIAbilityByName(const std::string &abilityName)
{
    auto it = uiAbilities_.find(abilityName);
    if (it != uiAbilities_.end()) {
        return it->second();
    }
    TAG_LOGE(AAFwkTag::ABILITY, "GetAbilityByName failed: %{public}s", abilityName.c_str());
    return nullptr;
}

#ifdef ABILITY_WINDOW_SUPPORT
void AbilityLoader::RegisterAbilitySlice(const std::string &sliceName, const CreateSlice &createFunc)
{
    slices_.emplace(sliceName, createFunc);
    TAG_LOGD(AAFwkTag::ABILITY, HILOG_MODULE_APP, "RegisterAbilitySlice %s", sliceName.c_str());
}

AbilitySlice *AbilityLoader::GetAbilitySliceByName(const std::string &sliceName)
{
    auto it = slices_.find(sliceName);
    if (it != slices_.end()) {
        return it->second();
    }
    TAG_LOGE(AAFwkTag::ABILITY, HILOG_MODULE_APP, "GetAbilitySliceByName failed: %s", sliceName.c_str());
    return nullptr;
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
