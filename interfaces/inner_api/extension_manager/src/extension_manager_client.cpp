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

#include "extension_manager_client.h"

#include "ability_manager_errors.h"
#include "extension_ability_info.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ABILITY_SERVICE_NOT_CONNECTED;        \
    }

ExtensionManagerClient& ExtensionManagerClient::GetInstance()
{
    static ExtensionManagerClient instance;
    return instance;
}

sptr<IExtensionManager> ExtensionManagerClient::GetExtensionManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        Connect();
    }

    return proxy_;
}

void ExtensionManagerClient::Connect()
{
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Fail to get SAMgr.");
        return;
    }
    auto remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Fail to connect ability manager service.");
        return;
    }

    deathRecipient_ = new ExtensionMgrDeathRecipient();
    if (remoteObj->IsProxyObject() && !remoteObj->AddDeathRecipient(deathRecipient_)) {
        HILOG_ERROR("Add death recipient to AbilityManagerService failed.");
        return;
    }

    proxy_ = iface_cast<IExtensionManager>(remoteObj);
    HILOG_DEBUG("Connect ability manager service success.");
}

void ExtensionManagerClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        HILOG_INFO("proxy_ is nullptr, no need reset.");
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote != nullptr && serviceRemote == remote.promote()) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void ExtensionManagerClient::ExtensionMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_INFO("ExtensionMgrDeathRecipient handle remote died.");
    ExtensionManagerClient::GetInstance().ResetProxy(remote);
}

ErrCode ExtensionManagerClient::ConnectServiceExtensionAbility(const Want &want,
    const sptr<IRemoteObject> &connect, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetExtensionManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect service failed, bundleName:%{public}s, abilityName:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    HILOG_INFO("name:%{public}s %{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(want, connect, nullptr, AppExecFwk::ExtensionAbilityType::SERVICE,
        userId, false);
}

ErrCode ExtensionManagerClient::ConnectServiceExtensionAbility(const Want &want,
    const sptr<IRemoteObject> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetExtensionManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect service failed, bundleName:%{public}s, abilityName:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    HILOG_INFO("name:%{public}s %{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(
        want, connect, callerToken, AppExecFwk::ExtensionAbilityType::SERVICE, userId, false);
}

ErrCode ExtensionManagerClient::ConnectEnterpriseAdminExtensionAbility(const Want &want,
    const sptr<IRemoteObject> &connect, const sptr<IRemoteObject> &callerToken, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetExtensionManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect service failed, bundleName:%{public}s, abilityName:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    HILOG_INFO("name:%{public}s %{public}s, userId:%{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(
        want, connect, callerToken, AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN, userId, true);
}

ErrCode ExtensionManagerClient::ConnectExtensionAbility(const Want &want, const sptr<IRemoteObject> &connect,
    int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetExtensionManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect failed, bundleName:%{public}s, abilityName:%{public}s",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    HILOG_INFO("bundleName: %{public}s, abilityName: %{public}s, userId: %{public}d.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    return abms->ConnectAbilityCommon(want, connect, nullptr, AppExecFwk::ExtensionAbilityType::UNSPECIFIED, userId);
}

ErrCode ExtensionManagerClient::DisconnectAbility(const sptr<IRemoteObject> &connect)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetExtensionManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("call");
    return abms->DisconnectAbility(connect);
}

}  // namespace AAFwk
}  // namespace OHOS
