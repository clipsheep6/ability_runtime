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

#ifndef OHOS_AAFWK_CONNECTION_STATE_ITME_H
#define OHOS_AAFWK_CONNECTION_STATE_ITME_H

#include <string>
#include <map>
#include <vector>

#include "connection_record.h"
#include "extension_ability_info.h"
#include "connection_data.h"
#include "data_ability_record.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DataAbilityCaller
 * DataAbilityCaller,This class is used to record data ability caller info.
 */
struct DataAbilityCaller {
    bool isNotHap = false;
    int32_t callerPid = 0;
    int32_t callerUid = 0;
    std::string callerName;
    sptr<IRemoteObject> callerToken = nullptr;
};

/**
 * @class ConnectionStateItem
 * ConnectionStateItem,This class is used to record connection state of a process.
 */
class ConnectedExtension;
class ConnectedDataAbility;
class ConnectionStateItem : public std::enable_shared_from_this<ConnectionStateItem> {
public:
    static std::shared_ptr<ConnectionStateItem> CreateConnectionStateItem(
        const std::shared_ptr<ConnectionRecord> &record);

    static std::shared_ptr<ConnectionStateItem> CreateConnectionStateItem(
        const DataAbilityCaller &dataCaller);

    ConnectionStateItem(int32_t callerUid, int32_t callerPid, const std::string &callerName);
    virtual ~ConnectionStateItem();

    /**
     * add a connection to target extension.
     *
     * @param record the connection record which mark an connection.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool AddConnection(const std::shared_ptr<ConnectionRecord> &record, AbilityRuntime::ConnectionData &data);

    /**
     * remove a connection to target extension.
     *
     * @param record the connection record which mark an connection.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool RemoveConnection(const std::shared_ptr<ConnectionRecord> &record, AbilityRuntime::ConnectionData &data);

    /**
     * add a connection to target data ability.
     *
     * @param caller the caller of this data ability.
     * @param dataAbility data ability that acquired.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool AddDataAbilityConnection(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &dataAbility, AbilityRuntime::ConnectionData &data);

    /**
     * remove a connection to target data ability.
     *
     * @param caller the caller of this data ability.
     * @param dataAbility data ability that acquired.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool RemoveDataAbilityConnection(const DataAbilityCaller &caller,
        const std::shared_ptr<DataAbilityRecord> &dataAbility, AbilityRuntime::ConnectionData &data);

    /**
     * handle died of data ability.
     *
     * @param token target token of data ability.
     * @param data output relationship data.
     * @return Returns true if need report relationship.
     */
    bool HandleDataAbilityDied(const sptr<IRemoteObject> &token, AbilityRuntime::ConnectionData &data);

    /**
     * generate all relationship data of this item.
     *
     * @param datas output relationship data.
     */
    void GenerateAllConnectionData(std::vector<AbilityRuntime::ConnectionData> &datas);

    /**
     * check if connections is empty.
     *
     * @return true if no connections.
     */
    bool IsEmpty() const;

private:
    DISALLOW_COPY_AND_MOVE(ConnectionStateItem);

    void GenerateConnectionData(const std::shared_ptr<ConnectedExtension> &connectedExtension,
        AbilityRuntime::ConnectionData &data);

    void GenerateConnectionData(const std::shared_ptr<ConnectedDataAbility> &connectedDataAbility,
        AbilityRuntime::ConnectionData &data);

    int32_t callerUid_ = 0;
    int32_t callerPid_ = 0;
    std::string callerName_;
    std::map<sptr<IRemoteObject>, std::shared_ptr<ConnectedExtension>> connectionMap_; // key:targetExtension token
    std::map<sptr<IRemoteObject>, std::shared_ptr<ConnectedDataAbility>> dataAbilityMap_; // key:targetDatability token
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_CONNECTION_STATE_ITME_H
