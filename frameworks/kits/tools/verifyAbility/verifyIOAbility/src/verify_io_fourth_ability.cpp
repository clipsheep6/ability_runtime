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

#include "verify_io_fourth_ability.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void VerifyIoFourthAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyIoFourthAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyIoFourthAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyIoFourthAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyIoFourthAbility::OnBackground()
{
    Ability::OnBackground();
}
void VerifyIoFourthAbility::OnForeground(const Want &want)
{
    Ability::OnForeground(want);
}

void SubAbilityTestTerminate ::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}

REGISTER_AA(VerifyIoFourthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS