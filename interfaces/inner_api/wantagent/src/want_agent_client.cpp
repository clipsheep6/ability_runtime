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

#include "want_agent_client.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
WantAgentClient &WantAgentClient::GetInstance()
{
    static WantAgentClient client;
    return client;
}

WantAgentClient::WantAgentClient() {}

WantAgentClient::~WantAgentClient() {}

sptr<IWantSender> WantAgentClient::GetWantSender(
    const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return nullptr;
    }
    return abms->GetWantSender(wantSenderInfo, callerToken);
}

ErrCode WantAgentClient::SendWantSender(const sptr<IWantSender> &target, const SenderInfo &senderInfo)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    return abms->SendWantSender(target, senderInfo);
}

void WantAgentClient::CancelWantSender(const sptr<IWantSender> &sender)
{
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return;
    }
    abms->CancelWantSender(sender);
}

ErrCode WantAgentClient::GetPendingWantUid(const sptr<IWantSender> &target, int32_t &uid)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    uid = abms->GetPendingWantUid(target);
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantUserId(const sptr<IWantSender> &target, int32_t &userId)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    userId = abms->GetPendingWantUserId(target);
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantBundleName(const sptr<IWantSender> &target, std::string &bundleName)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    bundleName = abms->GetPendingWantBundleName(target);
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantCode(const sptr<IWantSender> &target, int32_t &code)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    code = abms->GetPendingWantCode(target);
    return ERR_OK;
}

ErrCode WantAgentClient::GetPendingWantType(const sptr<IWantSender> &target, int32_t &type)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    type = abms->GetPendingWantType(target);
    type < 0 ? type = 0 : type;
    return ERR_OK;
}

void WantAgentClient::RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return;
    }
    if (recevier == nullptr) {
        HILOG_ERROR("recevier is nullptr.");
        return;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return;
    }
    abms->RegisterCancelListener(sender, recevier);
}

void WantAgentClient::UnregisterCancelListener(
    const sptr<IWantSender> &sender, const sptr<IWantReceiver> &recevier)
{
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr.");
        return;
    }
    if (recevier == nullptr) {
        HILOG_ERROR("recevier is nullptr.");
        return;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return;
    }
    abms->UnregisterCancelListener(sender, recevier);
}

ErrCode WantAgentClient::GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    return abms->GetPendingRequestWant(target, want);
}

ErrCode WantAgentClient::GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info)
{
    if (target == nullptr) {
        HILOG_ERROR("target is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    if (info == nullptr) {
        HILOG_ERROR("info is nullptr.");
        return INVALID_PARAMETERS_ERR;
    }
    auto abms = GetAbilityManager();
    if (!abms) {
        HILOG_ERROR("ability proxy is nullptr.");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    return abms->GetWantSenderInfo(target, info);
}

sptr<IAbilityManager> WantAgentClient::GetAbilityManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager == nullptr) {
            HILOG_ERROR("Fail to get registry.");
            return nullptr;
        }
        auto remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
        if (remoteObj == nullptr) {
            HILOG_ERROR("Fail to connect ability manager service.");
            return nullptr;
        }

        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) AbilityMgrDeathRecipient());
        if (deathRecipient_ == nullptr) {
            HILOG_ERROR("%{public}s :Failed to create AbilityMgrDeathRecipient!", __func__);
            return nullptr;
        }
        if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
            HILOG_ERROR("%{public}s :Add death recipient to AbilityManagerService failed.", __func__);
            return nullptr;
        }
        proxy_ = iface_cast<IAbilityManager>(remoteObj);
    }

    return proxy_;
}

void WantAgentClient::AbilityMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_INFO("AbilityMgrDeathRecipient handle remote died.");
    WantAgentClient::GetInstance().ResetProxy(remote);
}

void WantAgentClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
    }
    proxy_ = nullptr;
}
}  // namespace AAFwk
}  // namespace OHOS
