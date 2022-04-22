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

#include "ability_lifecycle_executor.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief While Ability's lifecycle changes, dispatch lifecycle state.
 *
 * @param state  Lifecycle state.
 */
void AbilityLifecycleExecutor::DispatchLifecycleState(const AbilityLifecycleExecutor::LifecycleState &state)
{
    state_ = state;
}

/**
 * @brief Obtains the int value of the ability lifecycle state represented by the
 * AbilityLifecycleExecutor.LifecycleState enum constant.
 *
 * @return return  Returns the int value of the ability lifecycle state represented
 * by the AbilityLifecycleExecutor.LifecycleState enum constant.
 */
int AbilityLifecycleExecutor::GetState()
{
    return state_;
}
}  // namespace AppExecFwk
}  // namespace OHOS