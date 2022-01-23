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

#include "datashare_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t DataShareExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("DataShareExtensionContext"));
int DataShareExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode DataShareExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    HILOG_DEBUG("%{public}s. End calling StartAbility. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("DataShareExtensionContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode DataShareExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    HILOG_DEBUG("%{public}s. End calling StartAbility. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("DataShareExtensionContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

bool DataShareExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_INFO("DataShareExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
    return ret == ERR_OK;
}

ErrCode DataShareExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want.GetElement(), connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
    HILOG_INFO("%{public}s end DisconnectAbility", __func__);
    return ret;
}

ErrCode DataShareExtensionContext::TerminateAbility()
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        HILOG_ERROR("DataShareExtensionContext::TerminateAbility is failed %{public}d", err);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return err;
}

AppExecFwk::AbilityType DataShareExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("DataShareExtensionContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

std::shared_ptr<AppExecFwk::AbilityInfo> DataShareExtensionContext::GetAbilityInfo() const
{
    return abilityInfo_;
}

void DataShareExtensionContext::SetAbilityInfo(const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        HILOG_ERROR("DataShareExtensionContext::SetAbilityInfo Info == nullptr");
        return;
    }
    abilityInfo_ = abilityInfo;
}
}  // namespace AbilityRuntime
}  // namespace OHOS