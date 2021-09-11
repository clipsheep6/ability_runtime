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

#include "verify_act_eighth_ability.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void SubscriberEighth::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}
void VerifyActEighthAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyActEighthAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyActEighthAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyActEighthAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyActEighthAbility::OnBackground()
{
    Ability::OnBackground();
}
REGISTER_AA(VerifyActEighthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS