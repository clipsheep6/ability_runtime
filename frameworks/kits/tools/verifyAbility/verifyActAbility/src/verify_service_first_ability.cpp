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

#include "verify_service_first_ability.h"
#include "app_log_wrapper.h"
#include "bundle_installer_proxy.h"
#include "status_receiver_impl.h"

namespace OHOS {
namespace AppExecFwk {
void VerifyServiceFirstAbility::OnStart(const Want &want)
{
    Ability::OnStart(want);
}
void VerifyServiceFirstAbility::OnStop()
{
    Ability::OnStop();
}
void VerifyServiceFirstAbility::OnActive()
{
    Ability::OnActive();
}
void VerifyServiceFirstAbility::OnInactive()
{
    Ability::OnInactive();
}
void VerifyServiceFirstAbility::OnBackground()
{
    Ability::OnBackground();
}
void VerifyServiceFirstAbility::OnForeground(const Want &want)
{
    Ability::OnForeground(want);
}

void VerifyServiceFirstAbility::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Ability::OnCommand(want, restart, startId);
}
sptr<IRemoteObject> VerifyServiceFirstAbility::OnConnect(const Want &want)
{
    Ability::OnConnect(want);
    return Ability::GetToken();
}
void VerifyServiceFirstAbility::OnDisconnect(const Want &want)
{
    Ability::OnDisconnect(want);
}
REGISTER_AA(VerifyServiceFirstAbility)
}  // namespace AppExecFwk
}  // namespace OHOS