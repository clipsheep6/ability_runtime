/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityAppStateObserver::AbilityAppStateObserver(std::shared_ptr<AbilityRecord> abilityRecord)
    : abilityRecord_(abilityRecord) {}
void AbilityAppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    auto abilityRecord = abilityRecord_.lock();
    if (abilityRecord) {
        const auto &abilityInfo = abilityRecord->GetAbilityInfo();
        if (abilityInfo.bundleName == processData.bundleName &&
            processData.processType == AppExecFwk::ProcessType::NORMAL &&
            abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
            abilityRecord->OnProcessDied();
        }
        if (abilityRecord->IsSceneBoard() && abilityRecord->GetPid() == processData.pid) {
            abilityRecord->OnProcessDied();
        }
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "AbilityRecord null");
    }
}
} // namespace AAFwk
} // namespace OHOS