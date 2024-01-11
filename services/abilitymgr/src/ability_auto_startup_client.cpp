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

#include "ability_auto_startup_client.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {

std::shared_ptr<AbilityAutoStartupClient> AbilityAutoStartupClient::instance_ = nullptr;
std::recursive_mutex AbilityAutoStartupClient::mutex_;

#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ABILITY_SERVICE_NOT_CONNECTED;        \
    }

std::shared_ptr<AbilityAutoStartupClient> AbilityAutoStartupClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::recursive_mutex> lock_l(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AbilityAutoStartupClient>();
        }
    }
    return instance_;
}

AbilityAutoStartupClient::AbilityAutoStartupClient()
{}

AbilityAutoStartupClient::~AbilityAutoStartupClient()
{}

sptr<IAbilityManager> AbilityAutoStartupClient::GetAbilityManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        (void)Connect();
    }

    return proxy_;
}

ErrCode AbilityAutoStartupClient::Connect()
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

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new AbilityMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("Failed to create AbilityMgrDeathRecipient!");
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

ErrCode AbilityAutoStartupClient::SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetApplicationAutoStartupByEDM(info, flag);
}

ErrCode AbilityAutoStartupClient::CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CancelApplicationAutoStartupByEDM(info, flag);
}

ErrCode AbilityAutoStartupClient::SetApplicationAutoStartupByEDM(
    const std::vector<AutoStartupInfo> &infoList, bool flag)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->SetApplicationAutoStartupByEDM(infoList, flag);
}

ErrCode AbilityAutoStartupClient::CancelApplicationAutoStartupByEDM(
    const std::vector<AutoStartupInfo> &infoList, bool flag)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->CancelApplicationAutoStartupByEDM(infoList, flag);
}

ErrCode AbilityAutoStartupClient::QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
{
    HILOG_DEBUG("Called.");
    auto abms = GetAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->QueryAllAutoStartupApplications(infoList);
}

void AbilityAutoStartupClient::AbilityMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    HILOG_INFO("AbilityMgrDeathRecipient handle remote died.");
    AbilityAutoStartupClient::GetInstance()->ResetProxy(remote);
}

void AbilityAutoStartupClient::ResetProxy(wptr<IRemoteObject> remote)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        HILOG_DEBUG("To remove death recipient.");
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}
} // namespace AAFwk
} // namespace OHOS
