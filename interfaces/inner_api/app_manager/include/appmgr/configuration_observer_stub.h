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

#ifndef OHOS_ABILITY_RUNTIME_CONFIGURATION_OBSERVER_STUB_H
#define OHOS_ABILITY_RUNTIME_CONFIGURATION_OBSERVER_STUB_H

#include <map>

#include "iremote_stub.h"
#include "app_mgr_constants.h"
#include "iconfiguration_observer.h"
#include "nocopyable.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
class ConfigurationObserverStub : public IRemoteStub<IConfigurationObserver> {
public:
    ConfigurationObserverStub();
    virtual ~ConfigurationObserverStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    virtual void OnConfigurationUpdated(const Configuration& configuration) override;

private:
    int32_t HandleOnConfigurationUpdated(MessageParcel &data, MessageParcel &reply);

    using ConfigurationObserverFunc = int32_t (ConfigurationObserverStub::*)(MessageParcel &data,
        MessageParcel &reply);
    std::map<uint32_t, ConfigurationObserverFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(ConfigurationObserverStub);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONFIGURATION_OBSERVER_STUB_H
