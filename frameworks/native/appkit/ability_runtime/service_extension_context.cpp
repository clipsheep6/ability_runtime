/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "service_extension_context.h"

#include "ability_connection.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t ServiceExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("ServiceExtensionContext"));
const std::string START_ABILITY_TYPE = "ABILITY_INNER_START_WITH_ACCOUNT";

int32_t ServiceExtensionContext::ILLEGAL_REQUEST_CODE(-1);

ErrCode ServiceExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "Start ability begin, ability:%{public}s.", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityAsCaller(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "Start ability as caller begin, ability:%{public}s.",
        want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->
    StartAbilityAsCaller(want, token_, nullptr, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartAbilityAsCaller is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityAsCaller(const AAFwk::Want &want,
    const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "Start ability as caller begin, ability:%{public}s.",
        want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityAsCaller(want, startOptions, token_, nullptr,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartAbilityAsCaller is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityByCall(
    const AAFwk::Want& want, const std::shared_ptr<CallerCallBack> &callback, int32_t accountId)
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        localCallContainer_ = std::make_shared<LocalCallContainer>();
        if (localCallContainer_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "%{public}s failed, localCallContainer_ is nullptr.", __func__);
            return ERR_INVALID_VALUE;
        }
    }
    return localCallContainer_->StartAbilityByCallInner(want, callback, token_, accountId);
}

ErrCode ServiceExtensionContext::ReleaseCall(const std::shared_ptr<CallerCallBack> &callback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s failed, localCallContainer_ is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }
    return localCallContainer_->ReleaseCall(callback);
}

void ServiceExtensionContext::ClearFailedCallConnection(const std::shared_ptr<CallerCallBack> &callback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    if (localCallContainer_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s failed, localCallContainer_ is nullptr.", __func__);
        return;
    }
    localCallContainer_->ClearFailedCallConnection(callback);
}

ErrCode ServiceExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "Connect ability begin, ability:%{public}s.",
        want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    TAG_LOGD(AAFwkTag::APPKIT, "ServiceExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode ServiceExtensionContext::StartAbilityWithAccount(const AAFwk::Want &want, int32_t accountId) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}d accountId:", accountId);
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(
        want, token_, ILLEGAL_REQUEST_CODE, accountId);
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s. End calling StartAbilityWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartAbilityWithAccount(
    const AAFwk::Want &want, int32_t accountId, const AAFwk::StartOptions &startOptions) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    (const_cast<Want &>(want)).SetParam(START_ABILITY_TYPE, true);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE, accountId);
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s. End calling StartAbilityWithAccount. ret=%{public}d", __func__, err);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartAbilityWithAccount is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StartServiceExtensionAbility(const AAFwk::Want &want, int32_t accountId) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StartServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::StopServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StopExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::StopServiceExtensionAbility is failed %{public}d", err);
    }
    return err;
}

ErrCode ServiceExtensionContext::ConnectAbilityWithAccount(
    const AAFwk::Want &want, int32_t accountId, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbilityWithAccount(token_, want, accountId, connectCallback);
    TAG_LOGI(AAFwkTag::APPKIT, "ServiceExtensionContext::ConnectAbilityWithAccount ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode ServiceExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "begin.");
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s end DisconnectAbility error, ret=%{public}d", __func__, ret);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "end");
    return ret;
}

ErrCode ServiceExtensionContext::TerminateAbility()
{
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceExtensionContext::TerminateAbility is failed %{public}d", err);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s end.", __func__);
    return err;
}

ErrCode ServiceExtensionContext::RequestModalUIExtension(const Want &want)
{
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s begin.", __func__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->RequestModalUIExtension(want);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceExtensionContext::RequestModalUIExtension is failed %{public}d", err);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s end.", __func__);
    return err;
}

AppExecFwk::AbilityType ServiceExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ServiceContext::GetAbilityInfoType info == nullptr");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
