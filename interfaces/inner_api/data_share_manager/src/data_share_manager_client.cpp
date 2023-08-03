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

#include "data_share_manager_client.h"

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

DataShareManagerClient& DataShareManagerClient::GetInstance()
{
    static DataShareManagerClient instance;
    return instance;
}

sptr<IDataShareManager> DataShareManagerClient::GetDataShareManager()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        Connect();
    }

    return proxy_;
}

void DataShareManagerClient::Connect()
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

    deathRecipient_ = new DataShareMgrDeathRecipient();
    if (remoteObj->IsProxyObject() && !remoteObj->AddDeathRecipient(deathRecipient_)) {
        HILOG_ERROR("Add death recipient to AbilityManagerService failed.");
        return;
    }

    proxy_ = iface_cast<IDataShareManager>(remoteObj);
    HILOG_DEBUG("Connect ability manager service success.");
}

void DataShareManagerClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr) {
        HILOG_ERROR("proxy_ is nullptr, no need reset.");
        return;
    }

    auto serviceRemote = proxy_->AsObject();
    if (serviceRemote != nullptr && serviceRemote == remote.promote()) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

void DataShareManagerClient::DataShareMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("handle remote died.");
    DataShareManagerClient::GetInstance().ResetProxy(remote);
}

ErrCode DataShareManagerClient::ConnectDataShareExtensionAbility(const Want &want,
    const sptr<IAbilityConnection> &connect, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityMgr = GetDataShareManager();
    if (abilityMgr == nullptr) {
        HILOG_ERROR("Connect failed, bundleName:%{public}s, abilityName:%{public}s, uri:%{public}s.",
            want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
            want.GetUriString().c_str());
        return ABILITY_SERVICE_NOT_CONNECTED;
    }

    HILOG_DEBUG("name:%{public}s %{public}s, uri:%{public}s.",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(),
        want.GetUriString().c_str());
    return abilityMgr->ConnectAbilityCommon(
        want, connect, nullptr, AppExecFwk::ExtensionAbilityType::DATASHARE, userId);
}
} // namespace AAFwk
} // namespace OHOS
