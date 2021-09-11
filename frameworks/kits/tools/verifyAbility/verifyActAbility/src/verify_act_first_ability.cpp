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

#include "verify_act_first_ability.h"
#include "app_log_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
void AbilityConnectionActFirst::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{}

void AbilityConnectionActFirst::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{}
void VerifyIOAbilityLifecycleCallbacks::OnAbilityStart(const std::shared_ptr<Ability> &ability)
{}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityInactive(const std::shared_ptr<Ability> &ability)
{}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityBackground(const std::shared_ptr<Ability> &ability)
{}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityForeground(const std::shared_ptr<Ability> &ability)
{}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityActive(const std::shared_ptr<Ability> &ability)
{}

void VerifyIOAbilityLifecycleCallbacks::OnAbilityStop(const std::shared_ptr<Ability> &ability)
{}
void VerifyIOAbilityLifecycleCallbacks::OnAbilitySaveState(const PacMap &outState)
{}

void VerifyActFirstAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyActFirstAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyActFirstAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyActFirstAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyActFirstAbility::OnBackground()
{
    Ability::OnBackground();
}

void AbilityContextStartAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}

void ConnectServiceAbilityTest::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}
REGISTER_AA(VerifyActFirstAbility)
}  // namespace AppExecFwk
}  // namespace OHOS