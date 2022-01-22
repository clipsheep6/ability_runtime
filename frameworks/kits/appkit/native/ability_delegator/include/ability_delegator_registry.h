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

#ifndef FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_REGISTRY_H
#define FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_REGISTRY_H

#include <memory>
#include "ability_delegator.h"
#include "ability_delegator_args.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegatorRegistry
{
public:
    static std::shared_ptr<AbilityDelegator> GetAbilityDelegator();
    static std::shared_ptr<AbilityDelegatorArgs> GetArguments();
    static void RegisterInstance(
        const std::shared_ptr<AbilityDelegator> &delegator, const std::shared_ptr<AbilityDelegatorArgs> &args);

private:
    static std::shared_ptr<AbilityDelegator> abilityDelegator_;
    static std::shared_ptr<AbilityDelegatorArgs> abilityDelegatorArgs_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_ABILITY_DELEGATOR_REGISTRY_H
