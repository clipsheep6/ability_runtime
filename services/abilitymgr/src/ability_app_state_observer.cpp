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

#include "ability_app_state_observer.h"
#include "ability_record.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityAppStateObserver::AbilityAppStateObserver(std::shared_ptr<AbilityRecord> abilityRecord)
    : abilityRecord_(abilityRecord) {}
void AbilityAppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    auto abilityRecord = abilityRecord_.lock();
    if (abilityRecord && abilityRecord->GetAbilityInfo().bundleName == processData.bundleName) {
        abilityRecord->OnProcessDied();
    } else {
        HILOG_WARN("AbilityRecord null or bundleName not matched");
    }
}
} // namespace AAFwk
} // namespace OHOS