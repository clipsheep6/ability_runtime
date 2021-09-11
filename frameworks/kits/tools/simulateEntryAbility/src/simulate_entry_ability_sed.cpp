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

#include "simulate_entry_ability_sed.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void SimulateEntryAbilitySed::OnStart(const Want &want)
{
    APP_LOGI("SimulateEntryAbilitySed::onStart");
    Ability::OnStart(want);
}
void SimulateEntryAbilitySed::OnStop()
{
    APP_LOGI("SimulateEntryAbilitySed::onStop");
    Ability::OnStop();
}
void SimulateEntryAbilitySed::OnActive()
{
    APP_LOGI("SimulateEntryAbilitySed::OnActive");
    Ability::OnActive();
}
void SimulateEntryAbilitySed::OnInactive()
{
    APP_LOGI("SimulateEntryAbilitySed::OnInactive");
    Ability::OnInactive();
}
void SimulateEntryAbilitySed::OnBackground()
{
    APP_LOGI("SimulateEntryAbilitySed::OnBackground");
    Ability::OnBackground();
}
REGISTER_AA(SimulateEntryAbilitySed)
}  // namespace AppExecFwk
}  // namespace OHOS