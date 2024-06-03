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

#include "cj_ability_stage.h"

#include "cj_runtime.h"
#include "context_impl.h"
#include "hilog_wrapper.h"

using namespace OHOS::AbilityRuntime;

std::shared_ptr<CJAbilityStage> CJAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (!runtime) {
        HILOG_ERROR("Runtime does not exist.");
        return nullptr;
    }
    auto& cjRuntime = static_cast<CJRuntime&>(*runtime);
    // Load cj app library.
    if (!cjRuntime.IsAppLibLoaded()) {
        HILOG_ERROR("Failed to create CJAbilityStage, applib not loaded.");
        return nullptr;
    }

    auto cjAbilityStageObject = CJAbilityStageObject::LoadModule(hapModuleInfo.moduleName);
    if (cjAbilityStageObject == nullptr) {
        cjRuntime.UnLoadCJAppLibrary();
        HILOG_ERROR("Failed to create CJAbilityStage.");
        return nullptr;
    }

    return std::make_shared<CJAbilityStage>(cjAbilityStageObject);
}

void CJAbilityStage::OnCreate(const AAFwk::Want& want) const
{
    AbilityStage::OnCreate(want);
    if (!cjAbilityStageObject_) {
        HILOG_ERROR("CJAbilityStage is not loaded.");
        return;
    }
    HILOG_DEBUG("CJAbilityStage::OnCreate");
    cjAbilityStageObject_->OnCreate();
}

std::string CJAbilityStage::OnAcceptWant(const AAFwk::Want& want)
{
    AbilityStage::OnAcceptWant(want);
    if (!cjAbilityStageObject_) {
        HILOG_ERROR("CJAbilityStage is not loaded.");
        return "";
    }
    return cjAbilityStageObject_->OnAcceptWant(want);
}

void CJAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdated(configuration);
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    if (!cjAbilityStageObject_) {
        HILOG_ERROR("CJAbilityStage is not loaded.");
        return;
    }
    cjAbilityStageObject_->OnConfigurationUpdated(fullConfig);
}

void CJAbilityStage::OnMemoryLevel(int level)
{
    AbilityStage::OnMemoryLevel(level);
    if (!cjAbilityStageObject_) {
        HILOG_ERROR("CJAbilityStage is not loaded.");
        return;
    }
    cjAbilityStageObject_->OnMemoryLevel(level);
}
