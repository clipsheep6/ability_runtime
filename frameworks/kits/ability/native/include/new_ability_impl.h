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

#ifndef FOUNDATION_APPEXECFWK_OHOS_NEW_ABILITY_IMPL_H
#define FOUNDATION_APPEXECFWK_OHOS_NEW_ABILITY_IMPL_H

#include "ability_impl.h"

namespace OHOS {
namespace AppExecFwk {
class Ability;
class AbilityHandler;
class AbilityLocalRecord;
class AbilityImpl;
class NewAbilityImpl final : public AbilityImpl {
public:
    /**
     * @brief Constructor.
     *
     */
    NewAbilityImpl() = default;

    /**
     * @brief Destructor.
     *
     */
    ~NewAbilityImpl() = default;

    /**
     * @brief Handling the life cycle switching of NewAbility.
     *
     * @param want Indicates the structure containing information about the ability.
     * @param targetState The life cycle state to switch to.
     *
     */
    void HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState);

    /**
     * @brief Handling the life cycle switching of NewAbility in switch.
     *
     * @param want Indicates the structure containing information about the ability.
     * @param targetState The life cycle state to switch to.
     *
     * @return return true if the lifecycle transaction successfully, otherwise return false.
     *
     */
    bool AbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_NEW_ABILITY_IMPL_H