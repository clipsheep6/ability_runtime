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
#ifdef SUPPORT_HICHECKER
#include "hichecker.h"
#endif
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::HiviewDFX;
ConnectionManager& ConnectionManager::GetInstance()
{
    static ConnectionManager connectionManager;
    return connectionManager;
}

ErrCode ConnectionManager::ConnectAbility(const sptr<IRemoteObject>& connectCaller,
    const AAFwk::Want& want, const sptr<AbilityConnectCallback>& connectCallback)
{
    return ConnectAbilityInner(connectCaller, want, AAFwk::DEFAULT_INVAL_VALUE, connectCallback);
}

ErrCode ConnectionManager::ConnectAbilityWithAccount(const sptr<IRemoteObject>& connectCaller,
    const AAFwk::Want& want, int accountId, const sptr<AbilityConnectCallback>& connectCallback)
{
    return ConnectAbilityInner(connectCaller, want, accountId, connectCallback);
}

ErrCode ConnectionManager::ConnectAbilityInner(const sptr<IRemoteObject>& connectCaller,
    const AAFwk::Want& want, int accountId, const sptr<AbilityConnectCallback>& connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("connectCaller or connectCallback is nullptr.");
        return AAFwk::ERR_INVALID_CALLER;
    }

    AppExecFwk::ElementName connectReceiver = want.GetElement();
    HILOG_DEBUG("connectReceiver: %{public}s.",
        (connectReceiver.GetBundleName() + ":" + connectReceiver.GetAbilityName()).c_str());

    sptr<AbilityConnection> abilityConnection;
    std::lock_guard<std::recursive_mutex> lock(connectionsLock_);
    auto connectionIter = abilityConnections_.begin();
    for (; connectionIter != abilityConnections_.end(); ++connectionIter) {
        if (MatchConnection(connectCaller, want, *connectionIter)) {
            break;
        }
    }
    HILOG_DEBUG("abilityConnectionsSize: %{public}zu.", abilityConnections_.size());
    if (connectionIter != abilityConnections_.end()) {
        std::vector<sptr<AbilityConnectCallback>>& callbacks = connectionIter->second;
        callbacks.push_back(connectCallback);
        abilityConnection = connectionIter->first.abilityConnection;
        abilityConnection->AddConnectCallback(connectCallback);
        HILOG_INFO("find abilityConnection exist, callbackSize:%{public}zu.", callbacks.size());
        if (abilityConnection->GetConnectionState() == CONNECTION_STATE_CONNECTED) {
            connectCallback->OnAbilityConnectDone(connectReceiver, abilityConnection->GetRemoteObject(),
                abilityConnection->GetResultCode());
            return ERR_OK;
        } else if (abilityConnection->GetConnectionState() == CONNECTION_STATE_CONNECTING) {
            return ERR_OK;
        } else {
            HILOG_ERROR("AbilityConnection has disconnected, erase it and reconnect.");
            abilityConnections_.erase(connectionIter);
            return CreateConnection(connectCaller, want, accountId, connectCallback);
        }
    } else {
        return CreateConnection(connectCaller, want, accountId, connectCallback);
    }
}

bool ConnectionManager::MatchConnection(
    const sptr<IRemoteObject>& connectCaller, const AAFwk::Want& connectReceiver,
    const std::map<ConnectionInfo, std::vector<sptr<AbilityConnectCallback>>>::value_type& connection)
{
    if (!connectReceiver.GetElement().GetAbilityName().empty()) {
        return connectCaller == connection.first.connectCaller &&
            connectReceiver.GetElement().GetBundleName() == connection.first.connectReceiver.GetBundleName() &&
            connectReceiver.GetElement().GetModuleName() == connection.first.connectReceiver.GetModuleName() &&
            connectReceiver.GetElement().GetAbilityName() == connection.first.connectReceiver.GetAbilityName();
    } else {
        // ImplicitConnect
        return connectCaller == connection.first.connectCaller &&
            connectReceiver.GetElement().GetBundleName() == connection.first.connectReceiver.GetBundleName() &&
            connectReceiver.GetOperation() == connection.first.connectReceiver;
    }
}

ErrCode ConnectionManager::CreateConnection(const sptr<IRemoteObject>& connectCaller,
    const AAFwk::Want& want, int accountId, const sptr<AbilityConnectCallback>& connectCallback)
{
    HILOG_INFO("Can not find connection, CreateConnection");
    sptr<AbilityConnection> abilityConnection = new AbilityConnection();
    if (abilityConnection == nullptr) {
        HILOG_ERROR("create connection failed.");
        return AAFwk::ERR_INVALID_CALLER;
    }
    abilityConnection->AddConnectCallback(connectCallback);
    abilityConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbility(
        want, abilityConnection, connectCaller, accountId);
    std::lock_guard<std::recursive_mutex> lock(connectionsLock_);
    if (ret == ERR_OK) {
        ConnectionInfo connectionInfo(connectCaller, want.GetOperation(), abilityConnection);
        std::vector<sptr<AbilityConnectCallback>> callbacks;
        callbacks.push_back(connectCallback);
        abilityConnections_[connectionInfo] = callbacks;
    } else {
        HILOG_ERROR("Call AbilityManagerService's ConnectAbility error:%{public}d", ret);
    }
    return ret;
}

ErrCode ConnectionManager::DisconnectAbility(const sptr<IRemoteObject>& connectCaller,
    const AppExecFwk::ElementName& connectReceiver, const sptr<AbilityConnectCallback>& connectCallback)
{
    AAFwk::Want wantReceiver;
    wantReceiver.SetElement(connectReceiver);
    return DisconnectAbility(connectCaller, wantReceiver, connectCallback);
}

ErrCode ConnectionManager::DisconnectAbility(const sptr<IRemoteObject>& connectCaller,
    const AAFwk::Want& connectReceiver, const sptr<AbilityConnectCallback>& connectCallback)
{
    if (connectCaller == nullptr || connectCallback == nullptr) {
        HILOG_ERROR("connectCaller or connectCallback is nullptr.");
        return AAFwk::ERR_INVALID_CALLER;
    }

    auto element = connectReceiver.GetElement();
    HILOG_DEBUG("connectReceiver: %{public}s.",
        (element.GetBundleName() + ":" + element.GetAbilityName()).c_str());
    std::lock_guard<std::recursive_mutex> lock(connectionsLock_);
    auto item = std::find_if(abilityConnections_.begin(), abilityConnections_.end(),
        [&connectCaller, &connectReceiver, this](const auto& obj) {
                return MatchConnection(connectCaller, connectReceiver, obj);
        });
    if (item != abilityConnections_.end()) {
        HILOG_DEBUG("remove callback, Size:%{public}zu.", item->second.size());
        auto iter = item->second.begin();
        while (iter != item->second.end()) {
            if (*iter == connectCallback) {
                iter = item->second.erase(iter);
            } else {
                iter++;
            }
        }

        sptr<AbilityConnection> abilityConnection = item->first.abilityConnection;

        HILOG_INFO("find abilityConnection exist, abilityConnectionsSize:%{public}zu.",
            abilityConnections_.size());
        if (item->second.empty()) {
            abilityConnections_.erase(item);
            HILOG_DEBUG("no callback left, so disconnectAbility.");
            return AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
        } else {
            connectCallback->OnAbilityDisconnectDone(element, ERR_OK);
            abilityConnection->RemoveConnectCallback(connectCallback);
            HILOG_DEBUG("callbacks is not empty, do not need disconnectAbility.");
            return ERR_OK;
        }
    } else {
        HILOG_ERROR("not find conn exist.");
        return AAFwk::CONNECTION_NOT_EXIST;
    }
}

bool ConnectionManager::DisconnectCaller(const sptr<IRemoteObject>& connectCaller)
{
    HILOG_DEBUG("call");
    if (connectCaller == nullptr) {
        HILOG_ERROR("connectCaller is nullptr.");
        return false;
    }
    std::lock_guard<std::recursive_mutex> lock(connectionsLock_);
    HILOG_DEBUG("abilityConnectionsSize:%{public}zu.", abilityConnections_.size());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (IsConnectCallerEqual(connectionInfo.connectCaller, connectCaller)) {
            HILOG_DEBUG("DisconnectAbility.");
            ErrCode ret =
                AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connectionInfo.abilityConnection);
            if (ret != ERR_OK) {
                HILOG_ERROR("ams->DisconnectAbility error, ret=%{public}d", ret);
            }
            iter = abilityConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }

    HILOG_DEBUG("abilityConnectionsSize:%{public}zu.", abilityConnections_.size());
    return isDisconnect;
}

bool ConnectionManager::RemoveConnection(const sptr<AbilityConnection> connection)
{
    std::lock_guard<std::recursive_mutex> lock(connectionsLock_);
    HILOG_DEBUG("abilityConnectionsSize: %{public}zu", abilityConnections_.size());

    bool isDisconnect = false;
    auto iter = abilityConnections_.begin();
    while (iter != abilityConnections_.end()) {
        ConnectionInfo connectionInfo = iter->first;
        if (connectionInfo.abilityConnection == connection) {
            HILOG_DEBUG("Remove connection.");
            iter = abilityConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }
    return isDisconnect;
}

void ConnectionManager::ReportConnectionLeakEvent(const int pid, const int tid)
{
    HILOG_DEBUG("pid:%{public}d, tid:%{public}d.", pid, tid);
#ifdef SUPPORT_HICHECKER
    if (HiChecker::Contains(Rule::RULE_CHECK_ABILITY_CONNECTION_LEAK)) {
        DfxDumpCatcher dumpLog;
        std::string stackTrace;
        bool ret = dumpLog.DumpCatch(pid, tid, stackTrace);
        if (ret) {
            std::string cautionMsg = "TriggerRule:RULE_CHECK_ABILITY_CONNECTION_LEAK-pid=" +
                std::to_string(pid) + "-tid=" + std::to_string(tid) + ", has leaked connection" +
                ", Are you missing a call to DisconnectAbility()";
            HILOG_DEBUG("cautionMsg:%{public}s.", cautionMsg.c_str());
            Caution caution(Rule::RULE_CHECK_ABILITY_CONNECTION_LEAK, cautionMsg, stackTrace);
            HiChecker::NotifyAbilityConnectionLeak(caution);
        } else {
            HILOG_ERROR("dumpCatch stackTrace failed.");
        }
    }
#endif
}

bool ConnectionManager::IsConnectCallerEqual(const sptr<IRemoteObject>& connectCaller,
    const sptr<IRemoteObject>& connectCallerOther)
{
    return connectCaller == connectCallerOther;
}

bool ConnectionManager::IsConnectReceiverEqual(AAFwk::Operation& connectReceiver,
    const AppExecFwk::ElementName& connectReceiverOther)
{
    return connectReceiver.GetBundleName() == connectReceiverOther.GetBundleName() &&
        connectReceiver.GetAbilityName() == connectReceiverOther.GetAbilityName();
}
} // namespace AbilityRuntime
} // namespace OHOS
