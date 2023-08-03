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

#ifndef OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_CLIENT_H

#include <mutex>

#include "data_share_manager_interface.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class DataShareManagerClient
 * DataShareManagerClient is used to access ability manager services.
 */
class DataShareManagerClient {
public:
    DataShareManagerClient() = default;
    ~DataShareManagerClient() = default;

    static DataShareManagerClient& GetInstance();

    /**
     * Connect data share extension ability.
     *
     * @param want, special want for the data share extension ability.
     * @param connect, callback used to notify caller the result of connecting or disconnecting.
     * @param userId, the extension runs in.
     * @return Returns ERR_OK on success, others on failure.
     */
    ErrCode ConnectDataShareExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        int32_t userId = DATA_SHARE_DEFAULT_INVALID_USER_ID);

private:
    class DataShareMgrDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DataShareMgrDeathRecipient() = default;
        virtual ~DataShareMgrDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    private:
        DISALLOW_COPY_AND_MOVE(DataShareMgrDeathRecipient);
    };

    sptr<IDataShareManager> GetDataShareManager();
    void Connect();
    void ResetProxy(const wptr<IRemoteObject> &remote);

    std::mutex mutex_;
    sptr<IDataShareManager> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_CLIENT_H
