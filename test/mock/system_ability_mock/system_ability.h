/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_SYSTEM_ABILITY_H
#define MOCK_OHOS_ABILITY_RUNTIME_SYSTEM_ABILITY_H

#include "hilog/log.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"

namespace OHOS {
#define REGISTER_SYSTEM_ABILITY_BY_ID(a, b, c)
#define REGISTER_SYSTEM_ABILITY(abilityClassName, abilityId, runOnCreate)
#define DECLEAR_SYSTEM_ABILITY(className)

class SystemAbility {
public:
    static bool MakeAndRegisterAbility(SystemAbility*)
    {
        return true;
    }

    bool AddSystemAbilityListener(int32_t systemAbilityId);

protected:
    virtual void OnStart()
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility OnStart called");
    }

    virtual void OnStop()
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility OnStop called");
    }

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility OnAddSystemAbility called");
    }

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility OnRemoveSystemAbility called");
    }

    bool Publish(sptr<IRemoteObject> systemAbility)
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility Publish called");
        systemAbility.ForceSetRefPtr(nullptr);
        // For test just mock to return true
        return true;
    }

    SystemAbility(bool runOnCreate = false)
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility default Creator called %d", runOnCreate);
    }

    SystemAbility(const int32_t serviceId, bool runOnCreate = false)
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility Creator called %d", runOnCreate);
    }

    virtual ~SystemAbility()
    {
        TAG_LOGD(AAFwkTag::TEST, "Mock SystemAbility Destructor called");
    }
};
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_SYSTEM_ABILITY_H
