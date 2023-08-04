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

#include "uiextension_manager_client.h"

#include "hilog_wrapper.h" 
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
std::recursive_mutex UIExtensionManagerClient::mutex_;
#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ABILITY_SERVICE_NOT_CONNECTED;        \
    }
UIExtensionManagerClient UIExtensionManagerClient::GetInstance()
{
    static UIExtensionManagerClient instance;
    return instance;
}

sptr<IAbilityManager> UIExtensionManagerClient::GetAbilityManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        (void)Connect();
    }

    return proxy_;
}

void UIExtensionManagerClient::UIExtensionMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("UIExtensionMgrDeathRecipient handle remote died.");
    UIExtensionManagerClient::GetInstance().ResetProxy(remote);
}

ErrCode UIExtensionManagerClient::Connect()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        HILOG_ERROR("Fail to get registry.");
        return GET_ABILITY_SERVICE_FAILED;
    }
    sptr<IRemoteObject> remoteObj = systemManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Fail to connect ability manager service.");
        return GET_ABILITY_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new UIExtensionMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("Failed to create DataAbilityMgrDeathRecipient!");
        return GET_ABILITY_SERVICE_FAILED;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("Add death recipient to AbilityManagerService failed.");
        return GET_ABILITY_SERVICE_FAILED;
    }

    proxy_ = iface_cast<IAbilityManager>(remoteObj);
    HILOG_DEBUG("Connect ability manager service success.");
    return ERR_OK;
}

void UIExtensionManagerClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

ErrCode UIExtensionManagerClient::StartUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    CHECK_POINTER_RETURN_NOT_CONNECTED(extensionSessionInfo);
    HILOG_INFO("name:%{public}s %{public}s, userId:%{public}d.",
        extensionSessionInfo->want.GetElement().GetAbilityName().c_str(),
        extensionSessionInfo->want.GetElement().GetBundleName().c_str(), userId);
    return abms->StartUIExtensionAbility(extensionSessionInfo, userId);
}

ErrCode UIExtensionManagerClient::TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
    int resultCode, const Want *resultWant)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("call");
    return abms->TerminateUIExtensionAbility(extensionSessionInfo, resultCode, resultWant);
}

ErrCode UIExtensionManagerClient::MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, bool fromUser)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    HILOG_INFO("fromUser:%{public}d.", fromUser);
    return abms->MinimizeUIExtensionAbility(extensionSessionInfo, fromUser);
}

ErrCode UIExtensionManagerClient::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abms = GetAbilityManager();
    if (abms == nullptr) {
        HILOG_ERROR("Connect failed, bundleName:%{public}s, abilityName:%{public}s, uri:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
            want.GetUriString().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    HILOG_INFO("name:%{public}s %{public}s, uri:%{public}s.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
        want.GetUriString().c_str());
    return abms->ConnectUIExtensionAbility(want, connect, sessionInfo, userId);
}

ErrCode UIExtensionManagerClient::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool &isFocused)
{
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CheckUIExtensionIsFocused(uiExtensionTokenId, isFocused);
}
} // namespace AAFwk
} // namespace OHOS