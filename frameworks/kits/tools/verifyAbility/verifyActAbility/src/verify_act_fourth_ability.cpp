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

#include "verify_act_fourth_ability.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void VerifyActFourthAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyActFourthAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyActFourthAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyActFourthAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyActFourthAbility::OnBackground()
{
    Ability::OnBackground();
}

void VerifyActFourthAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{}

void CommentAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}
REGISTER_AA(VerifyActFourthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS