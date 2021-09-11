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

#include "verify_io_sixth_ability.h"
#include "app_log_wrapper.h"
#include "skills.h"
namespace OHOS {
namespace AppExecFwk {
using IAbilityConnection = OHOS::AAFwk::IAbilityConnection;
using Skills = OHOS::AAFwk::Skills;
using PatternsMatcher = OHOS::AAFwk::PatternsMatcher;
using MatchType = OHOS::AAFwk::MatchType;
using WantParams = OHOS::AAFwk::WantParams;
void VerifyIoSixthAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyIoSixthAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyIoSixthAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyIoSixthAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyIoSixthAbility::OnBackground()
{
    Ability::OnBackground();
}

REGISTER_AA(VerifyIoSixthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS