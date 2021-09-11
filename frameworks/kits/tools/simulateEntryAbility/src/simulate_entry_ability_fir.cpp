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

#include "simulate_entry_ability_fir.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void SimulateEntryAbilityFir::OnStart(const Want &want)
{
    APP_LOGI("SimulateEntryAbilityFir::onStart");
    Ability::OnStart(want);
}
void SimulateEntryAbilityFir::OnStop()
{
    APP_LOGI("SimulateEntryAbilityFir::onStop");
    Ability::OnStop();
}
void SimulateEntryAbilityFir::OnActive()
{
    APP_LOGI("SimulateEntryAbilityFir::OnActive");
    Ability::OnActive();
}
void SimulateEntryAbilityFir::OnInactive()
{
    APP_LOGI("SimulateEntryAbilityFir::OnInactive");
    Ability::OnInactive();
}
void SimulateEntryAbilityFir::OnBackground()
{
    APP_LOGI("SimulateEntryAbilityFir::OnBackground");
    Ability::OnBackground();
}
REGISTER_AA(SimulateEntryAbilityFir)
}  // namespace AppExecFwk
}  // namespace OHOS