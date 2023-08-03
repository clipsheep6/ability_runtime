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

#include "data_ability_manager_client.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
#define CHECK_POINTER_RETURN_NOT_CONNECTED(object)   \
    if (!object) {                                   \
        HILOG_ERROR("proxy is nullptr.");            \
        return ABILITY_SERVICE_NOT_CONNECTED;        \
    }

DataAbilityManagerClient& DataAbilityManagerClient::GetInstance()
{
    static DataAbilityManagerClient instance;
    return instance;
}

sptr<IDataAbilityManager> DataAbilityManagerClient::GetDataAbilityManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!proxy_) {
        (void)Connect();
    }
    return proxy_;
}

void DataAbilityManagerClient::DataAbilityMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("handle remote died.");
    DataAbilityManagerClient::GetInstance().ResetProxy(remote);
}

ErrCode DataAbilityManagerClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
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

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DataAbilityMgrDeathRecipient());
    if (deathRecipient_ == nullptr) {
        HILOG_ERROR("Failed to create DataAbilityMgrDeathRecipient!");
        return GET_ABILITY_SERVICE_FAILED;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        HILOG_ERROR("Add death recipient to AbilityManagerService failed.");
        return GET_ABILITY_SERVICE_FAILED;
    }

    proxy_ = iface_cast<IDataAbilityManager>(remoteObj);
    HILOG_DEBUG("Connect ability manager service success.");
    return ERR_OK;
}

void DataAbilityManagerClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!proxy_) {
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

sptr<IAbilityScheduler> DataAbilityManagerClient::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetDataAbilityManager();
    if (!abms) {
        return nullptr;
    }
    return abms->AcquireDataAbility(uri, tryBind, callerToken);
}

ErrCode DataAbilityManagerClient::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    auto abms = GetDataAbilityManager();
    CHECK_POINTER_RETURN_NOT_CONNECTED(abms);
    return abms->ReleaseDataAbility(dataAbilityScheduler, callerToken);
}
} // namespace AAFwk
} // namespace OHOS
