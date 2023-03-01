/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_connection.h"
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

ErrCode UIExtensionContext::StartUIExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    HILOG_DEBUG("StartUIExtensionAbility begin.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::UI);
    if (err != ERR_OK) {
        HILOG_ERROR("StartUIExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::ConnectExtensionAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_DEBUG("Connect ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode ret = ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_DEBUG("ConnectExtensionAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode UIExtensionContext::DisconnectExtensionAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_DEBUG("DisconnectExtensionAbility begin.");
    ErrCode ret = ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("DisconnectAbility error, ret=%{public}d", ret);
    }
    HILOG_DEBUG("DisconnectExtensionAbility end");
    return ret;
}

ErrCode UIExtensionContext::TerminateAbility()
{
    HILOG_DEBUG("TerminateAbility begin.");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        HILOG_ERROR("TerminateAbility is failed %{public}d", err);
    }
    HILOG_DEBUG("TerminateAbility end.");
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
