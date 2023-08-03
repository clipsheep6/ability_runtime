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

#ifndef OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_PROXY_H
#define OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_PROXY_H

#include "data_share_manager_interface.h"

#include "iremote_proxy.h"

namespace OHOS {
class MessageParcel;
namespace AAFwk {
enum class AbilityManagerInterfaceCode;
/**
 * @class DataShareManagerProxy
 * DataShareManagerProxy.
 */
class DataShareManagerProxy : public IRemoteProxy<IDataShareManager> {
public:
    explicit DataShareManagerProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDataShareManager>(impl) {}
    virtual ~DataShareManagerProxy() = default;

    /**
     * ConnectAbility, connect session with service ability.
     *
     * @param want, Special want for service type's ability.
     * @param connect, Callback used to notify caller the result of connecting or disconnecting.
     * @param callerToken, caller ability token.
     * @param userId, Designation User ID.
     * @return Returns ERR_OK on success, others on failure.
     */
    int ConnectAbilityCommon(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DATA_SHARE_DEFAULT_INVALID_USER_ID) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    ErrCode SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data, MessageParcel &reply,
        MessageOption& option);

private:
    static inline BrokerDelegator<DataShareManagerProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DATA_SHARE_MANAGER_PROXY_H
