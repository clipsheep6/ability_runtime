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

#ifndef OHOS_ABILITY_RUNTIME_CONFIGURATION_OBSERVER_PROXY_H
#define OHOS_ABILITY_RUNTIME_CONFIGURATION_OBSERVER_PROXY_H

#include "iremote_proxy.h"
#include "app_mgr_constants.h"
#include "iconfiguration_observer.h"
namespace OHOS {
namespace AppExecFwk {
class ConfigurationObserverProxy : public IRemoteProxy<IConfigurationObserver> {
public:
    explicit ConfigurationObserverProxy(const sptr<IRemoteObject> &impl);
    virtual ~ConfigurationObserverProxy() = default;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    virtual void OnConfigurationUpdated(const Configuration& configuration) override;

private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<ConfigurationObserverProxy> delegator_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONFIGURATION_OBSERVER_PROXY_H
