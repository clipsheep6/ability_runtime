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

#include "vpn_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t VpnExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("VpnExtensionContext"));
const std::string START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";

int32_t VpnExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode VpnExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StartAbilityAsCaller(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability as caller begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->
    StartAbilityAsCaller(want, token_, nullptr, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartAbilityAsCaller is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StartAbilityAsCaller(const AAFwk::Want &want,
    const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability as caller begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want, startOptions, token_, nullptr,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartAbilityAsCaller is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StartAbilityByCall(
    const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback, int32_t accountId)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        localCallContainer_ = std::make_shared<LocalCallContainer>();
        if (localCallContainer_ == nullptr) {
            HILOG_ERROR("%{public}s failed, localCallContainer_ is nullptr.", __func__);
            return ERR_INVALID_VALUE;
        }
    }
    return localCallContainer_->StartAbilityByCallInner(want, callback, token_, accountId);
}

ErrCode VpnExtensionContext::ReleaseCall(const std::shared_ptr<CallerCallBack> &callback) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        HILOG_ERROR("%{public}s failed, localCallContainer_ is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    return localCallContainer_->ReleaseCall(callback);
}

void VpnExtensionContext::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        HILOG_ERROR("%{public}s failed, localCallContainer_ is nullptr.", __func__);
        return;
    }
    localCallContainer_->ClearFailedCallConnection(callback);
}

ErrCode VpnExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("Connect ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    HILOG_INFO("VpnExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode VpnExtensionContext::StartAbilityWithAccount(const AAFwk::Want &want, int32_t accountId) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    HILOG_INFO("%{public}d accountId:", accountId);
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, token_, ILLEGAL_REQUEST_CODE, accountId);
    HILOG_DEBUG("%{public}s. End calling StartAbilityWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StartAbilityWithAccount(
    const AAFwk::Want &want, int32_t accountId, const AAFwk::StartOptions &startOptions) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE, accountId);
    HILOG_DEBUG("%{public}s. End calling StartAbilityWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StartVpnExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::UNSPECIFIED);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StartVpnExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::StopVpnExtensionAbility(const AAFwk::Want& want, int32_t accountId) const
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::UNSPECIFIED);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnContext::StopVpnExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode VpnExtensionContext::ConnectAbilityWithAccount(
    const AAFwk::Want &want, int32_t accountId, const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    HILOG_INFO("VpnExtensionContext::ConnectAbilityWithAccount ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode VpnExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
    HILOG_INFO("%{public}s end DisconnectAbility", __func__);
    return ret;
}

ErrCode VpnExtensionContext::TerminateAbility()
{
    HILOG_INFO("%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        HILOG_ERROR("VpnExtensionContext::TerminateAbility is failed %{public}d", err);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return err;
}

AppExecFwk::AbilityType VpnExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("VpnContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
