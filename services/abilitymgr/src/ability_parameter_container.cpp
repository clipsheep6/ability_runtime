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

#include "ability_parameter_container.h"

namespace OHOS {
namespace AAFwk {

void AbilityParameterContaier::AddParameter(int abilityRecordId, const AbilityRequest &abilityRequest)
{
    std::lock_guard<std::mutex> guard(lock_);
    parameterContaier_.emplace(abilityRecordId, abilityRequest);
}

void AbilityParameterContaier::RemoveParameterByID(int abilityRecordId)
{
    std::lock_guard<std::mutex> guard(lock_);
    parameterContaier_.erase(abilityRecordId);
    HILOG_INFO("parameterContaier_ size %{public}zu", parameterContaier_.size());
}

AbilityRequest AbilityParameterContaier::GetAbilityRequestFromContaier(int abilityRecordId)
{
    std::lock_guard<std::mutex> guard(lock_);
    AbilityRequest abilityRequest;
    auto iter = parameterContaier_.find(abilityRecordId);
    if (iter != parameterContaier_.end()) {
        return iter->second;
    }
    return abilityRequest;
}

bool AbilityParameterContaier::IsExist(int abilityRecordId)
{
    std::lock_guard<std::mutex> guard(lock_);
    return (parameterContaier_.find(abilityRecordId) != parameterContaier_.end());
}

}  // namespace AAFwk
}  // namespace OHOS
