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

#include "simulate_feature_ability_sed.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void SimulateFeatureAbilitySed::OnStart(const Want &want)
{
    APP_LOGI("SimulateFeatureAbilitySed::onStart");
    Ability::OnStart(want);
}
void SimulateFeatureAbilitySed::OnStop()
{
    APP_LOGI("SimulateFeatureAbilitySed::onStop");
    Ability::OnStop();
}
void SimulateFeatureAbilitySed::OnActive()
{
    APP_LOGI("SimulateFeatureAbilitySed::OnActive");
    Ability::OnActive();
}
void SimulateFeatureAbilitySed::OnInactive()
{
    APP_LOGI("SimulateFeatureAbilitySed::OnInactive");
    Ability::OnInactive();
}
void SimulateFeatureAbilitySed::OnBackground()
{
    APP_LOGI("SimulateFeatureAbilitySed::OnBackground");
    Ability::OnBackground();
}
REGISTER_AA(SimulateFeatureAbilitySed)
}  // namespace AppExecFwk
}  // namespace OHOS