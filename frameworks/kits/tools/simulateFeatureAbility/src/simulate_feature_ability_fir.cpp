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

#include "simulate_feature_ability_fir.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void SimulateFeatureAbilityFir::OnStart(const Want &want)
{
    APP_LOGI("SimulateFeatureAbilityFir::onStart");
    Ability::OnStart(want);
}
void SimulateFeatureAbilityFir::OnStop()
{
    APP_LOGI("SimulateFeatureAbilityFir::onStop");
    Ability::OnStop();
}
void SimulateFeatureAbilityFir::OnActive()
{
    APP_LOGI("SimulateFeatureAbilityFir::OnActive");
    Ability::OnActive();
}
void SimulateFeatureAbilityFir::OnInactive()
{
    APP_LOGI("SimulateFeatureAbilityFir::OnInactive");
    Ability::OnInactive();
}
void SimulateFeatureAbilityFir::OnBackground()
{
    APP_LOGI("SimulateFeatureAbilityFir::OnBackground");
    Ability::OnBackground();
}
REGISTER_AA(SimulateFeatureAbilityFir)
}  // namespace AppExecFwk
}  // namespace OHOS