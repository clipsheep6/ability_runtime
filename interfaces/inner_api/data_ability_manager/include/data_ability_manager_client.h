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

#ifndef OHOS_ABILITY_DATA_ABILITY_MANAGER_CLIENT_H
#define OHOS_ABILITY_DATA_ABILITY_MANAGER_CLIENT_H

#include <mutex>

#include "data_ability_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class DataAbilityManagerClient {
public:
    DataAbilityManagerClient() = default;
    ~DataAbilityManagerClient() = default;

    static DataAbilityManagerClient& GetInstance();

    /**
     * Connect ability manager service.
     *
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode Connect();

    /**
     * AcquireDataAbility, acquire a data ability by its authority, if it not existed,
     * AMS loads it synchronously.
     *
     * @param uri, data ability uri.
     * @param tryBind, true: when a data ability is died, ams will kill this client, or do nothing.
     * @param callerToken, specifies the caller ability token.
     * @return returns the data ability ipc object, or nullptr for failed.
     */
    sptr<IAbilityScheduler> AcquireDataAbility(const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken);

    /**
     * ReleaseDataAbility, release the data ability that referenced by 'dataAbilityToken'.
     *
     * @param dataAbilityToken, specifies the data ability that will be released.
     * @param callerToken, specifies the caller ability token.
     * @return returns ERR_OK if succeeded, or error codes for failed.
     */
    ErrCode ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken);

private:
    class DataAbilityMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DataAbilityMgrDeathRecipient() = default;
        ~DataAbilityMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        DISALLOW_COPY_AND_MOVE(DataAbilityMgrDeathRecipient);
    };

    sptr<IDataAbilityManager> GetDataAbilityManager();
    void ResetProxy(const wptr<IRemoteObject> &remote);

    std::mutex mutex_;
    sptr<IDataAbilityManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_DATA_ABILITY_MANAGER_CLIENT_H
