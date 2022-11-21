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

#include "connection_state_item.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class ConnectedExtension
 * ConnectedExtension,This class is used to record a connected extension.
 */
class ConnectedExtension : public std::enable_shared_from_this<ConnectedExtension> {
public:
    static std::shared_ptr<ConnectedExtension> CreateConnectedExtension(const std::shared_ptr<ConnectionRecord> &record)
    {
        if (!record) {
            return nullptr;
        }

        auto targetExtension = record->GetAbilityRecord();
        if (!targetExtension) {
            return nullptr;
        }

        return std::make_shared<ConnectedExtension>(targetExtension);
    }

    ConnectedExtension()
    {
        extensionType_ = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    }

    explicit ConnectedExtension(const std::shared_ptr<AbilityRecord> &target)
    {
        if (!target) {
            return;
        }
        extensionPid_ = target->GetPid();
        extensionUid_ = target->GetUid();
        extensionBundleName_ = target->GetAbilityInfo().bundleName;
        extensionModuleName_ = target->GetAbilityInfo().moduleName;
        extensionName_ = target->GetAbilityInfo().name;
        extensionType_ = target->GetAbilityInfo().extensionAbilityType;
    }

    virtual ~ConnectedExtension() = default;

    bool AddConnection(const sptr<IRemoteObject> &connection)
    {
        if (!connection) {
            return false;
        }

        bool needNotify = connections_.empty();
        connections_.emplace(connection);

        return needNotify;
    }

    bool RemoveConnection(const sptr<IRemoteObject> &connection)
    {
        if (!connection) {
            return false;
        }

        connections_.erase(connection);
        return connections_.empty();
    }

    void GenerateExtensionInfo(AbilityRuntime::ConnectionData &data)
    {
        data.extensionPid = extensionPid_;
        data.extensionUid = extensionUid_;
        data.extensionBundleName = extensionBundleName_;
        data.extensionModuleName = extensionModuleName_;
        data.extensionName = extensionName_;
        data.extensionType = extensionType_;
    }

private:
    int32_t extensionPid_ = 0;
    int32_t extensionUid_ = 0;
    std::string extensionBundleName_;
    std::string extensionModuleName_;
    std::string extensionName_;
    AppExecFwk::ExtensionAbilityType extensionType_;
    std::set<sptr<IRemoteObject>> connections_; // remote object of IAbilityConnection
};

/**
 * @class ConnectedDataAbility
 * ConnectedDataAbility,This class is used to record a connected data ability.
 */
class ConnectedDataAbility : public std::enable_shared_from_this<ConnectedDataAbility> {
public:
    static std::shared_ptr<ConnectedDataAbility> CreateConnectedDataAbility(
        const std::shared_ptr<DataAbilityRecord> &record)
    {
        if (!record) {
            return nullptr;
        }

        auto targetAbility = record->GetAbilityRecord();
        if (!targetAbility) {
            return nullptr;
        }

        return std::make_shared<ConnectedDataAbility>(targetAbility);
    }

    ConnectedDataAbility() {}

    explicit ConnectedDataAbility(const std::shared_ptr<AbilityRecord> &target)
    {
        if (!target) {
            return;
        }

        dataAbilityPid_ = target->GetPid();
        dataAbilityUid_ = target->GetUid();
        bundleName_ = target->GetAbilityInfo().bundleName;
        moduleName_ = target->GetAbilityInfo().moduleName;
        abilityName_ = target->GetAbilityInfo().name;
    }

    virtual ~ConnectedDataAbility() = default;

    bool AddCaller(const DataAbilityCaller &caller)
    {
        if (!caller.isNotHap && !caller.callerToken) {
            return false;
        }

        bool needNotify = callers_.empty();
        auto it = find_if(callers_.begin(), callers_.end(), [&caller](const std::shared_ptr<CallerInfo> &info) {
            if (caller.isNotHap) {
                return info && info->IsNotHap() && info->GetCallerPid() == caller.callerPid;
            } else {
                return info && info->GetCallerToken() == caller.callerToken;
            }
        });
        if (it == callers_.end()) {
            callers_.emplace_back(std::make_shared<CallerInfo>(caller.isNotHap, caller.callerPid, caller.callerToken));
        }

        return needNotify;
    }

    bool RemoveCaller(const DataAbilityCaller &caller)
    {
        if (!caller.isNotHap && !caller.callerToken) {
            return false;
        }

        auto it = find_if(callers_.begin(), callers_.end(), [&caller](const std::shared_ptr<CallerInfo> &info) {
            if (caller.isNotHap) {
                return info && info->IsNotHap() && info->GetCallerPid() == caller.callerPid;
            } else {
                return info && info->GetCallerToken() == caller.callerToken;
            }
        });
        if (it != callers_.end()) {
            callers_.erase(it);
        }

        return callers_.empty();
    }

    void GenerateExtensionInfo(AbilityRuntime::ConnectionData &data)
    {
        data.extensionPid = dataAbilityPid_;
        data.extensionUid = dataAbilityUid_;
        data.extensionBundleName = bundleName_;
        data.extensionModuleName = moduleName_;
        data.extensionName = abilityName_;
        data.extensionType = AppExecFwk::ExtensionAbilityType::DATASHARE;
    }

private:
    class CallerInfo : public std::enable_shared_from_this<CallerInfo> {
    public:
        CallerInfo(bool isNotHap, int32_t callerPid, const sptr<IRemoteObject> &callerToken)
            : isNotHap_(isNotHap), callerPid_(callerPid), callerToken_(callerToken) {}

        bool IsNotHap() const
        {
            return isNotHap_;
        }

        int32_t GetCallerPid() const
        {
            return callerPid_;
        }

        sptr<IRemoteObject> GetCallerToken() const
        {
            return callerToken_;
        }

    private:
        bool isNotHap_ = false;
        int32_t callerPid_ = 0;
        sptr<IRemoteObject> callerToken_ = nullptr;
    };

    int32_t dataAbilityPid_ = 0;
    int32_t dataAbilityUid_ = 0;
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::list<std::shared_ptr<CallerInfo>> callers_; // caller infos of this data ability.
};

ConnectionStateItem::ConnectionStateItem(int32_t callerUid, int32_t callerPid, const std::string &callerName)
    : callerUid_(callerUid), callerPid_(callerPid), callerName_(callerName)
{
}

ConnectionStateItem::~ConnectionStateItem()
{}

std::shared_ptr<ConnectionStateItem> ConnectionStateItem::CreateConnectionStateItem(
    const std::shared_ptr<ConnectionRecord> &record)
{
    if (!record) {
        return nullptr;
    }

    return std::make_shared<ConnectionStateItem>(record->GetCallerUid(),
        record->GetCallerPid(), record->GetCallerName());
}

std::shared_ptr<ConnectionStateItem> ConnectionStateItem::CreateConnectionStateItem(
    const DataAbilityCaller &dataCaller)
{
    return std::make_shared<ConnectionStateItem>(dataCaller.callerUid,
        dataCaller.callerPid, dataCaller.callerName);
}

bool ConnectionStateItem::AddConnection(const std::shared_ptr<ConnectionRecord> &record,
    AbilityRuntime::ConnectionData &data)
{
    if (!record) {
        HILOG_ERROR("AddConnection, invalid connection record.");
        return false;
    }

    auto token = record->GetTargetToken();
    if (!token) {
        HILOG_ERROR("AddConnection, invalid token.");
        return false;
    }

    sptr<IRemoteObject> connectionObj = record->GetConnection();
    if (!connectionObj) {
        HILOG_ERROR("AddConnection, no connection callback for this connect.");
        return false;
    }

    std::shared_ptr<ConnectedExtension> connectedExtension = nullptr;
    auto it = connectionMap_.find(token);
    if (it == connectionMap_.end()) {
        connectedExtension = ConnectedExtension::CreateConnectedExtension(record);
        if (connectedExtension) {
            connectionMap_[token] = connectedExtension;
        }
    } else {
        connectedExtension = it->second;
    }

    if (!connectedExtension) {
        HILOG_ERROR("AddConnection, connectedExtension is invalid");
        return false;
    }

    bool needNotify = connectedExtension->AddConnection(connectionObj);
    if (needNotify) {
        GenerateConnectionData(connectedExtension, data);
    }

    return needNotify;
}

bool ConnectionStateItem::RemoveConnection(const std::shared_ptr<ConnectionRecord> &record,
    AbilityRuntime::ConnectionData &data)
{
    if (!record) {
        HILOG_ERROR("RemoveConnection, invalid connection record.");
        return false;
    }

    auto token = record->GetTargetToken();
    if (!token) {
        HILOG_ERROR("RemoveConnection, invalid token.");
        return false;
    }

    sptr<IRemoteObject> connectionObj = record->GetConnection();
    if (!connectionObj) {
        HILOG_ERROR("RemoveConnection, no connection callback for this connect.");
        return false;
    }

    auto it = connectionMap_.find(token);
    if (it == connectionMap_.end()) {
        HILOG_ERROR("RemoveConnection, no such connectedExtension.");
        return false;
    }

    auto connectedExtension = it->second;
    if (!connectedExtension) {
        HILOG_ERROR("RemoveConnection, can not find such connectedExtension");
        return false;
    }

    bool needNotify = connectedExtension->RemoveConnection(connectionObj);
    if (needNotify) {
        connectionMap_.erase(it);
        GenerateConnectionData(connectedExtension, data);
    }

    return needNotify;
}

bool ConnectionStateItem::AddDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &dataAbility, AbilityRuntime::ConnectionData &data)
{
    if (!dataAbility) {
        HILOG_ERROR("invalid dataAbility.");
        return false;
    }

    auto token = dataAbility->GetToken();
    if (!token) {
        HILOG_ERROR("invalid dataAbility token.");
        return false;
    }

    std::shared_ptr<ConnectedDataAbility> connectedAbility = nullptr;
    auto it = dataAbilityMap_.find(token);
    if (it == dataAbilityMap_.end()) {
        connectedAbility = ConnectedDataAbility::CreateConnectedDataAbility(dataAbility);
        if (connectedAbility) {
            dataAbilityMap_[token] = connectedAbility;
        }
    } else {
        connectedAbility = it->second;
    }

    if (!connectedAbility) {
        HILOG_ERROR("connectedAbility is invalid");
        return false;
    }

    bool needNotify = connectedAbility->AddCaller(caller);
    if (needNotify) {
        GenerateConnectionData(connectedAbility, data);
    }

    return needNotify;
}

bool ConnectionStateItem::RemoveDataAbilityConnection(const DataAbilityCaller &caller,
    const std::shared_ptr<DataAbilityRecord> &dataAbility, AbilityRuntime::ConnectionData &data)
{
    if (!dataAbility) {
        HILOG_ERROR("RemoveDataAbilityConnection, invalid data ability record.");
        return false;
    }

    auto token = dataAbility->GetToken();
    if (!token) {
        HILOG_ERROR("RemoveDataAbilityConnection, invalid data ability token.");
        return false;
    }

    auto it = dataAbilityMap_.find(token);
    if (it == dataAbilityMap_.end()) {
        HILOG_ERROR("RemoveDataAbilityConnection, no such connected data ability.");
        return false;
    }

    auto connectedDataAbility = it->second;
    if (!connectedDataAbility) {
        HILOG_ERROR("RemoveDataAbilityConnection, can not find such connectedDataAbility");
        return false;
    }

    bool needNotify = connectedDataAbility->RemoveCaller(caller);
    if (needNotify) {
        dataAbilityMap_.erase(it);
        GenerateConnectionData(connectedDataAbility, data);
    }

    return needNotify;
}

bool ConnectionStateItem::HandleDataAbilityDied(const sptr<IRemoteObject> &token,
    AbilityRuntime::ConnectionData &data)
{
    if (!token) {
        return false;
    }

    auto it = dataAbilityMap_.find(token);
    if (it == dataAbilityMap_.end()) {
        HILOG_ERROR("HandleDataAbilityDied, no such connected data ability.");
        return false;
    }

    auto connectedDataAbility = it->second;
    if (!connectedDataAbility) {
        HILOG_ERROR("HandleDataAbilityDied, can not find such connectedDataAbility");
        return false;
    }

    dataAbilityMap_.erase(it);
    GenerateConnectionData(connectedDataAbility, data);
    return true;
}

bool ConnectionStateItem::IsEmpty() const
{
    return connectionMap_.empty() && dataAbilityMap_.empty();
}

void ConnectionStateItem::GenerateAllConnectionData(std::vector<AbilityRuntime::ConnectionData> &datas)
{
    AbilityRuntime::ConnectionData data;
    for (auto it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
        GenerateConnectionData(it->second, data);
        datas.emplace_back(data);
    }

    for (auto it = dataAbilityMap_.begin(); it != dataAbilityMap_.end(); ++it) {
        GenerateConnectionData(it->second, data);
        datas.emplace_back(data);
    }
}

void ConnectionStateItem::GenerateConnectionData(
    const std::shared_ptr<ConnectedExtension> &connectedExtension, AbilityRuntime::ConnectionData &data)
{
    if (connectedExtension) {
        connectedExtension->GenerateExtensionInfo(data);
    }
    data.callerUid = callerUid_;
    data.callerPid = callerPid_;
    data.callerName = callerName_;
}

void ConnectionStateItem::GenerateConnectionData(const std::shared_ptr<ConnectedDataAbility> &connectedDataAbility,
    AbilityRuntime::ConnectionData &data)
{
    if (connectedDataAbility) {
        connectedDataAbility->GenerateExtensionInfo(data);
    }
    data.callerUid = callerUid_;
    data.callerPid = callerPid_;
    data.callerName = callerName_;
}
}  // namespace AAFwk
}  // namespace OHOS
