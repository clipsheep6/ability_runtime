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

#include "verify_act_ninth_ability.h"
#include "app_log_wrapper.h"
#include <thread>
namespace OHOS {
namespace AppExecFwk {
void SubscriberNinth::OnReceiveEvent(const EventFwk::CommonEventData &data)
{}
void SubscriberNinth::AsyncProcess(const std::shared_ptr<EventFwk::AsyncCommonEventResult> &result, int code)
{}

void VerifyActNinthAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyActNinthAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyActNinthAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyActNinthAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyActNinthAbility::OnBackground()
{
    Ability::OnBackground();
}
REGISTER_AA(VerifyActNinthAbility)
}  // namespace AppExecFwk
}  // namespace OHOS