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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_RDB_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_RDB_H

#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "cpp/mutex.h"

#include "data_ability_observer_interface.h"

namespace OHOS {
namespace AAFwk {
class DataObsMgrInnerRdb : public std::enable_shared_from_this<DataObsMgrInnerRdb> {
public:
    using ObsMapType = std::map<std::string, std::list<sptr<IDataAbilityObserver>>>;
    using ObsRecipientMapType = std::map<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>;

    DataObsMgrInnerRdb();
    virtual ~DataObsMgrInnerRdb();

    int HandleRegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver);
    int HandleUnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver);
    int HandleNotifyChange(const Uri &uri);
    void OnCallBackDied(const wptr<IRemoteObject> &remote);

private:
    void AddObsDeathRecipient(sptr<IDataAbilityObserver> dataObserver);
    void RemoveObsDeathRecipient(sptr<IRemoteObject> dataObserver);
    void RemoveObs(sptr<IRemoteObject> dataObserver);
    bool HaveRegistered(sptr<IDataAbilityObserver> dataObserver);

    static constexpr uint32_t OBS_NUM_MAX = 50;
    ffrt::mutex innerRdbMutex_;
    ObsMapType observers_;
    ObsRecipientMapType obsRecipient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_RDB_H
