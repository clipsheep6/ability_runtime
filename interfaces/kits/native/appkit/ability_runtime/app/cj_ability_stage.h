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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_H

#include <utility>

#include "ability_stage.h"
#include "cj_ability_stage_proxy.h"

namespace OHOS {
namespace AbilityRuntime {
class CJAbilityStage : public AbilityStage {
public:
    static std::shared_ptr<CJAbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);
    explicit CJAbilityStage(std::shared_ptr<CJAbilityStageProxy> cjStage) : cjAbilityStageObject_(std::move(cjStage)) {}
    ~CJAbilityStage() override = default;

    void OnCreate(const AAFwk::Want& want) const override;
    std::string OnAcceptWant(const AAFwk::Want& want) override;
    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;
    void OnMemoryLevel(int level) override;

private:
    std::shared_ptr<CJAbilityStageProxy> cjAbilityStageObject_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_STAGE_H
