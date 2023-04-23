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

#ifndef OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_INCLUDE_ROUTER_DEATH_RECIPIENT_H
#define OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_INCLUDE_ROUTER_DEATH_RECIPIENT_H

#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
class ServiceRouterDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    ServiceRouterDeathRecipient()
    {}
    virtual ~ServiceRouterDeathRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SERVICE_ROUTER_FRAMEWORK_INCLUDE_SERVICE_ROUTER_DEATH_RECIPIENT_H