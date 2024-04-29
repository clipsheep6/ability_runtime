/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "app_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AppContext::AppContext()
{}
AppContext::~AppContext()
{}

const std::shared_ptr<AbilityInfo> AppContext::GetAbilityInfo()
{
    return nullptr;
}

ErrCode AppContext::StartAbility(const AAFwk::Want &want, int requestCode)
{
    return ERR_INVALID_VALUE;
}

ErrCode AppContext::StartAbility(const Want &want, int requestCode,
    const AbilityStartSetting &abilityStartSetting)
{
    return ERR_INVALID_VALUE;
}

ErrCode AppContext::TerminateAbility()
{
    return ERR_INVALID_VALUE;
}

std::string AppContext::GetCallingBundle()
{
    return "";
}

bool AppContext::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    return false;
}

ErrCode AppContext::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    return ERR_INVALID_VALUE;
}

bool AppContext::StopAbility(const AAFwk::Want &want)
{
    return false;
}

sptr<IRemoteObject> AppContext::GetToken()
{
    return nullptr;
}

void AppContext::StartAbilities(const std::vector<AAFwk::Want> &wants)
{}

int AppContext::GetMissionId()
{
    return -1;
}
}  // namespace AppExecFwk
}  // namespace OHOS
