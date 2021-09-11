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

#include "verify_act_third_ability.h"
#include "app_log_wrapper.h"
#include "bundle_installer_proxy.h"
#include "status_receiver_impl.h"

namespace OHOS {
namespace AppExecFwk {
void VerifyActThirdAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyActThirdAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyActThirdAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyActThirdAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyActThirdAbility::OnBackground()
{
    Ability::OnBackground();
}
void HapManageDemo::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}

REGISTER_AA(VerifyActThirdAbility)
}  // namespace AppExecFwk
}  // namespace OHOS