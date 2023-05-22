/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ui_extension_context.h"

#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t UIExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("UIExtensionContext"));
int UIExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::TerminateSelf()
{
    HILOG_DEBUG("TerminateSelf begin.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        HILOG_ERROR("TerminateSelf is failed %{public}d", err);
    }
    HILOG_DEBUG("TerminateSelf end.");
    return err;
}

AppExecFwk::AbilityType UIExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_WARN("GetAbilityInfoType info is nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
