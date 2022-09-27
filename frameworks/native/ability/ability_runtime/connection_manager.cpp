/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "dfx_dump_catcher.h"
#include "hichecker.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::HiviewDFX;
ConnectionManager& ConnectionManager::GetInstance()
{
    static ConnectionManager connectionManager;
    return connectionManager;
}

ErrCode ConnectionManager::ConnectAbility(const sptr<IRemoteObject> &connectCaller,
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback)
{
    return ConnectAbilityInner(connectCaller, want, AAFwk::DEFAULT_INVAL_VALUE, connectCallback);
}

ErrCode ConnectionManager::ConnectAbilityWithAccount(const sptr<IRemoteObject> &connectCaller,
    const AAFwk::Want &want, int accountId, const sptr<AbilityConnectCallback> &connectCallback)
{
    return ConnectAbilityInner(connectCaller, want, accountId, connectCallback);
}

ErrCode ConnectionManager::ConnectAbilityInner(const sptr<IRemoteObject> &connectCaller,
    const AAFwk::Want &want, int accountId, const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    AppExecFwk::ElementName connectReceiver = want.GetElement();
    HILOG_DEBUG("%{public}s begin, connectReceiver: %{public}s.", __func__, connectReceiver.GetURI().c_str());

    sptr<AbilityConnection> abilityConnection;
    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](const std::map<ConnectionInfo,
            std::vector<sptr<AbilityConnectCallback>>>::value_type &obj) {
                return connectCaller == obj.first.connectCaller &&
                    connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                    connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        std::vector<sptr<AbilityConnectCallback>> callbacks = item->second;
        callbacks.push_back(connectCallback);
        abilityConnections_[item->first] = callbacks;
        abilityConnection = item->first.abilityConnection;
        abilityConnection->AddConnectCallback(connectCallback);
        HILOG_INFO("find abilityConnection exist, callbackSize:%{public}zu.", callbacks.size());
        if (abilityConnection->GetConnectionState() == CONNECTION_STATE_CONNECTED) {
            connectCallback->OnAbilityConnectDone(connectReceiver, abilityConnection->GetRemoteObject(),
                abilityConnection->GetResultCode());
            return ERR_OK;
        } else if (abilityConnection->GetConnectionState() == CONNECTION_STATE_CONNECTING) {
            return ERR_OK;
        } else {
            abilityConnections_.erase(item);
            HILOG_DEBUG("not find connection, abilityConnectionsSize:%{public}zu.", abilityConnections_.size());
            return ERR_INVALID_VALUE;
        }
    } else {
        abilityConnection = new AbilityConnection();
        if (abilityConnection == nullptr) {
            HILOG_ERROR("abilityConnection == nullptr");
            return ERR_INVALID_VALUE;
        }
        abilityConnection->AddConnectCallback(connectCallback);
        abilityConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);
        ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(
            want, abilityConnection, connectCaller, accountId);
        if (ret == ERR_OK) {
            ConnectionInfo connectionInfo(connectCaller, connectReceiver, abilityConnection);
            std::vector<sptr<AbilityConnectCallback>> callbacks;
            callbacks.push_back(connectCallback);
            abilityConnections_[connectionInfo] = callbacks;
        } else {
            HILOG_ERROR("%{public}s, Call AbilityManagerService's ConnectAbility error:%{public}d", __func__, ret);
        }
        HILOG_DEBUG("not find connection, abilityConnectionsSize:%{public}zu.", abilityConnections_.size());
        return ret;
    }
}

ErrCode ConnectionManager::DisconnectAbility(const sptr<IRemoteObject> &connectCaller,
    const AppExecFwk::ElementName &connectReceiver, const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("%{public}s, connectCaller or connectCallback is nullptr.", __func__);
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("%{public}s begin,, connectReceiver: %{public}s.", __func__, connectReceiver.GetURI().c_str());

    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver](
            const std::map<ConnectionInfo, std::vector<sptr<AbilityConnectCallback>>>::value_type &obj) {
            return connectCaller == obj.first.connectCaller &&
                   connectReceiver.GetBundleName() == obj.first.connectReceiver.GetBundleName() &&
                   connectReceiver.GetAbilityName() == obj.first.connectReceiver.GetAbilityName();
        });
    if (item != abilityConnections_.end()) {
        HILOG_DEBUG("%{public}s begin remove callback, Size:%{public}zu.", __func__, item->second.size());
        auto iter = item->second.begin();
        while (iter != item->second.end()) {
            if (*iter == connectCallback) {
                iter = item->second.erase(iter);
            } else {
                iter++;
            }
        }

        sptr<AbilityConnection> abilityConnection = item->first.abilityConnection;

        HILOG_INFO("find abilityConnection exist, abilityConnectionsSize:%{public}zu.", abilityConnections_.size());
        if (item->second.empty()) {
            abilityConnections_.erase(item);
            HILOG_DEBUG("%{public}s no callback left, so disconnectAbility.", __func__);
            return AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
        } else {
            connectCallback->OnAbilityDisconnectDone(connectReceiver, ERR_OK);
            abilityConnection->RemoveConnectCallback(connectCallback);
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
        HILOG_ERROR("%{public}s failed, connectCaller is nullptr.", __func__);
        return false;
    }

    HILOG_DEBUG("%{public}s, abilityConnectionsSize:%{public}zu.",
        __func__, abilityConnections_.size());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (IsConnectCallerEqual(connectionInfo.connectCaller, connectCaller)) {
            HILOG_DEBUG("%{public}s DisconnectAbility.", __func__);
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

    HILOG_DEBUG("%{public}s end, abilityConnectionsSize:%{public}zu.", __func__, abilityConnections_.size());
    return isDisconnect;
}

bool ConnectionManager::DisconnectReceiver(const AppExecFwk::ElementName &connectReceiver)
{
    HILOG_DEBUG("%{public}s begin, abilityConnectionsSize:%{public}zu, bundleName:%{public}s, abilityName:%{public}s.",
        __func__, abilityConnections_.size(), connectReceiver.GetBundleName().c_str(),
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

    HILOG_DEBUG("%{public}s end, abilityConnectionsSize:%{public}zu.", __func__, abilityConnections_.size());
    return isDisconnect;
}

void ConnectionManager::ReportConnectionLeakEvent(const int pid, const int tid)
{
    HILOG_DEBUG("%{public}s begin, pid:%{public}d, tid:%{public}d.", __func__, pid, tid);
    if (HiChecker::Contains(Rule::RULE_CHECK_ABILITY_CONNECTION_LEAK)) {
        DfxDumpCatcher dumpLog;
        std::string stackTrace;
        bool ret = dumpLog.DumpCatch(pid, tid, stackTrace);
        if (ret) {
            std::string cautionMsg = "TriggerRule:RULE_CHECK_ABILITY_CONNECTION_LEAK-pid=" +
                std::to_string(pid) + "-tid=" + std::to_string(tid) + ", has leaked connection" +
                ", Are you missing a call to DisconnectAbility()";
            HILOG_DEBUG("%{public}s cautionMsg:%{public}s.", __func__, cautionMsg.c_str());
            Caution caution(Rule::RULE_CHECK_ABILITY_CONNECTION_LEAK, cautionMsg, stackTrace);
            HiChecker::NotifyAbilityConnectionLeak(caution);
        } else {
            HILOG_ERROR("%{public}s dumpCatch stackTrace failed.", __func__);
        }
    }
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
} // namespace AbilityRuntime
} // namespace OHOS
