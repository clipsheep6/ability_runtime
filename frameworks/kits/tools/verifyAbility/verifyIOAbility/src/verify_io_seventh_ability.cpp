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

#include "verify_io_seventh_ability.h"
#include "app_log_wrapper.h"
namespace OHOS {
namespace AppExecFwk {
void VerifyIoSeventhAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyIoSeventhAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyIoSeventhAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyIoSeventhAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyIoSeventhAbility::OnBackground()
{
    Ability::OnBackground();
}
REGISTER_AA(VerifyIoSeventhAbility)
}  // namespace AppExecFwk
}  // namespace OHOS