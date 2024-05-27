/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_PROXY_H

#include "iremote_proxy.h"

#include "app_mgr_constants.h"
#include "bundle_info.h"
#include "iapp_state_callback.h"

namespace OHOS {
namespace AppExecFwk {
class AppStateCallbackProxy : public IRemoteProxy<IAppStateCallback> {
public:
    explicit AppStateCallbackProxy(const sptr<IRemoteObject> &impl);
    virtual ~AppStateCallbackProxy() = default;

    /**
     * AbilityMgr's request is done.
     *
     * @param token Ability token.
     * @param state Application state.
     */
    virtual void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const AbilityState state) override;

    /**
     * Application state changed callback.
     *
     * @param appProcessData Process data
     */
    virtual void OnAppStateChanged(const AppProcessData &appProcessData) override;

    /**
     * @brief Notify application update system environment changes.
     * @param config System environment change parameters.
     * @param userId userId Designation User ID.
     */
    virtual void NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) override;

    /**
     * @brief Notify abilityms start resident process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual void NotifyStartResidentProcess(std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * @brief Notify abilityms app process OnRemoteDied
     * @param abilityTokens abilities in died process.
     */
    virtual void OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;
    
private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<AppStateCallbackProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_PROXY_H
