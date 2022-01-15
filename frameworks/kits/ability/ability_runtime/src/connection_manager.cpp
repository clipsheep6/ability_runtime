/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "connection_manager.h"
#include "ability_connection.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
ConnectionManager& ConnectionManager::GetInstance()
{
    static ConnectionManager connectionManager;
    return connectionManager;
}

ErrCode ConnectionManager::ConnectAbility(const sptr<IRemoteObject> &connectCaller,
    const AAFwk::Want &want, const std::shared_ptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    AppExecFwk::ElementName connectReceiver = want.GetElement();
    HILOG_DEBUG("%{public}s begin, connectCaller: %{public}p, connectReceiver: %{public}s.",
        __func__, connectCaller.GetRefPtr(),
        (connectReceiver.GetBundleName() + ":" + connectReceiver.GetAbilityName()).c_str());

    sptr<AbilityConnection> abilityConnection;
    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](const std::map<ConnectionInfo,
            std::vector<std::shared_ptr<AbilityConnectCallback>>>::value_type &obj) {
                return connectCaller == obj.first.connectCaller &&
                    connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                    connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        std::vector<std::shared_ptr<AbilityConnectCallback>> callbacks = item->second;
        callbacks.push_back(connectCallback);
        abilityConnections_[item->first] = callbacks;
        abilityConnection = item->first.abilityConnection;
        abilityConnection->SetConnectCallback(connectCallback);
        HILOG_INFO("%{public}s end, find abilityConnection:%{public}p exist, callbackSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)callbacks.size());
        if (abilityConnection->GetResultCode() == ERR_OK) {
            return ERR_OK;
        } else {
            return AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, abilityConnection, connectCaller);
        }
    } else {
        abilityConnection = new AbilityConnection(connectCallback);
        ErrCode ret =
            AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(want, abilityConnection, connectCaller);
        if (ret == ERR_OK) {
            ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
            std::vector<std::shared_ptr<AbilityConnectCallback>> callbacks;
            callbacks.push_back(connectCallback);
            abilityConnections_[connectionInfo] = callbacks;
        }
        HILOG_DEBUG("%{public}s end, not find connection, connection: %{public}p, abilityConnectionsSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)abilityConnections_.size());
        return ret;
    }
}

ErrCode ConnectionManager::DisconnectAbility(const sptr<IRemoteObject> &connectCaller,
    const AppExecFwk::ElementName &connectReceiver, const std::shared_ptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("%{public}s begin, connectCaller: %{public}p, connectReceiver: %{public}s.",
        __func__, connectCaller.GetRefPtr(),
        (connectReceiver.GetBundleName() + ":" + connectReceiver.GetAbilityName()).c_str());

    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](
            const std::map<ConnectionInfo, std::vector<std::shared_ptr<AbilityConnectCallback>>>::value_type &obj) {
            return connectCaller == obj.first.connectCaller &&
                   connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                   connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        std::vector<std::shared_ptr<AbilityConnectCallback>> callbacks = item->second;
        HILOG_DEBUG("%{public}s begin remove callback, callbackSize:%{public}d.", __func__, (int32_t)callbacks.size());
        auto iter = callbacks.begin();
        while (iter != callbacks.end()) {
            if (*iter == connectCallback) {
                iter = callbacks.erase(iter);
            } else {
                iter++;
            }
        }
        abilityConnections_[item->first] = callbacks;
        sptr<AbilityConnection> abilityConnection;
        abilityConnection = item->first.abilityConnection;
        abilityConnection->SetConnectCallback(connectCallback);
        HILOG_INFO("%{public}s end, find abilityConnection:%{public}p exist, abilityConnectionsSize:%{public}d.",
            __func__, abilityConnection.GetRefPtr(), (int32_t)abilityConnections_.size());
        if (callbacks.size() == 0) {
            abilityConnections_.erase(item);
            HILOG_DEBUG("%{public}s disconnectAbility.", __func__);
            return AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
        } else {
            HILOG_DEBUG("%{public}s callbacks is not empty, do not need disconnectAbility.", __func__);
            return ERR_OK;
        }
    } else {
        HILOG_ERROR("%{public}s not find conn exist.", __func__);
        return ERR_INVALID_VALUE;
    }
}

bool ConnectionManager::DisconnectCaller(const sptr<IRemoteObject> &connectCaller)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (connectCaller == nullptr) {
        HILOG_ERROR("%{public}s end, connectCaller is nullptr.", __func__);
        return false;
    }

    HILOG_DEBUG("%{public}s, connectCaller:%{public}p, abilityConnectionsSize:%{public}d.",
        __func__, connectCaller.GetRefPtr(), (int32_t)abilityConnections_.size());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (IsConnectCallerEqual(connectionInfo.connectCaller, connectCaller)) {
            HILOG_DEBUG("%{public}s DisconnectAbility connection:%{public}p.",
                __func__, connectionInfo.abilityConnection.GetRefPtr());
            ErrCode ret =
                AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connectionInfo.abilityConnection);
            if (ret != ERR_OK) {
                HILOG_ERROR("%{public}s ams->DisconnectAbility error, ret=%{public}d", __func__, ret);
            }
            iter = abilityConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }

    HILOG_DEBUG("%{public}s end, abilityConnectionsSize:%{public}d.", __func__, (int32_t)abilityConnections_.size());
    return isDisconnect;
}

bool ConnectionManager::DisconnectReceiver(const AppExecFwk::ElementName &connectReceiver)
{
    HILOG_DEBUG("%{public}s begin, abilityConnectionsSize:%{public}d, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, (int32_t)abilityConnections_.size(), connectReceiver.GetBundleName().c_str(),
        connectReceiver.GetAbilityName().c_str());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (IsConnectReceiverEqual(connectionInfo.connectReceiver, connectReceiver)) {
            iter = abilityConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }

    HILOG_DEBUG("%{public}s end, abilityConnectionsSize:%{public}d.", __func__, (int32_t)abilityConnections_.size());
    return isDisconnect;
}

bool ConnectionManager::IsConnectCallerEqual(const sptr<IRemoteObject> &connectCaller,
    const sptr<IRemoteObject> &connectCallerOther)
{
    return connectCaller == connectCallerOther;
}

bool ConnectionManager::IsConnectReceiverEqual(const AppExecFwk::ElementName &connectReceiver,
    const AppExecFwk::ElementName &connectReceiverOther)
{
    return connectReceiver.GetBundleName() == connectReceiverOther.GetBundleName() &&
           connectReceiver.GetAbilityName() == connectReceiverOther.GetAbilityName();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
