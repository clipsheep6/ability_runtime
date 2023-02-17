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

#ifndef OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_FREE_INSTALL_OBSERVER_STUB_H

#include "free_install_observer_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AbilityRuntime {
class FreeInstallObserverProxy : public IRemoteProxy<IFreeInstallObserver>
{
public:
    explicit FreeInstallObserverProxy(const sptr<IRemoteObject> &impl);
    virtual ~FreeInstallObserverProxy() = default;

    /**
     * OnInstallFinished, AMS return free install result.
     * 
     * @param bundleName, free install bundleName
     * @param abilityName, free install abilityName
     * @param startTime, free install start request time
     */
    virtual void OnInstallFinished(const std::string bundleName, const std::string abilityName, const std::string startTime, int resultCode) override;
private:
    bool WriteInterfaceToken(MessageParcel &data);
    static inline BrokerDelegator<FreeInstallObserverProxy> delegator_;
};
}
}

#endif