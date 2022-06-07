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

#ifndef FOUNDATION_ABILITY_RUNTIME_INTERFACES_INNERKITS_APP_MANAGER_INCLUDE_APPMGR_ICONFIGURATION_OBSERVER_H
#define FOUNDATION_ABILITY_RUNTIME_INTERFACES_INNERKITS_APP_MANAGER_INCLUDE_APPMGR_ICONFIGURATION_OBSERVER_H

#include "iremote_broker.h"
#include "iremote_object.h"
#include "configuration.h"

namespace OHOS {
namespace AppExecFwk {
class IConfigurationObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityruntime.IConfigurationObserver");

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    virtual void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) = 0;

    enum class Message {
        TRANSACT_ON_CONFIGURATION_UPDATED = 0,
    };
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_ABILITY_RUNTIME_INTERFACES_INNERKITS_APP_MANAGER_INCLUDE_APPMGR_ICONFIGURATION_OBSERVER_H
