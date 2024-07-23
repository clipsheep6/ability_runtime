/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "recovery_info_timer.h"
#include "app_exit_reason_data_manager.h"
#include "app_recovery/default_recovery_config.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t HOURS_TO_SECOND = 60 * 60;

RecoveryInfoTimer& RecoveryInfoTimer::GetInstance()
{
    static RecoveryInfoTimer instance;
    return instance;
}
void RecoveryInfoTimer::SubmitSaveRecoveryInfo(RecoveryInfo recoveryInfo)
{
    std::lock_guard<std::mutex> lock(recoveryInfoQueueLock_);
    auto findByInfo = [&recoveryInfo](RecoveryInfo& item) {
        return item.abilityName == recoveryInfo.abilityName && item.bundleName == recoveryInfo.bundleName &&
            item.moduleName == recoveryInfo.moduleName;
    };
    auto i = find_if(recoveryInfoQueue_.begin(), recoveryInfoQueue_.end(), findByInfo);
    if (i != recoveryInfoQueue_.end()) {
        recoveryInfoQueue_.erase(i);
    }
    recoveryInfoQueue_.push_back(recoveryInfo);

    int64_t now = recoveryInfo.time;
    auto timeoutDeleteTime = AbilityRuntime::DefaultRecoveryConfig::GetInstance().
        GetTimeoutDeleteTime() * HOURS_TO_SECOND;
    auto reserveNumber = AbilityRuntime::DefaultRecoveryConfig::GetInstance().GetReserveNumber();
    int timeoutCount = 0;
    for (auto p = recoveryInfoQueue_.begin(); p != recoveryInfoQueue_.end(); p++) {
        if (now - p->time >= timeoutDeleteTime) {
            timeoutCount++;
        }
    }

    timeoutCount -= reserveNumber;
    for (; timeoutCount > 0; timeoutCount--) {
        auto recoveryInfo = recoveryInfoQueue_.begin();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "clearRecoveryInfo bundleName = %{public}s, abilityName = %{public}s",
            recoveryInfo->bundleName.c_str(), recoveryInfo->abilityName.c_str());
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            DeleteAbilityRecoverInfo(recoveryInfo->tokenId, recoveryInfo->moduleName, recoveryInfo->abilityName);
        recoveryInfoQueue_.pop_front();
    }
}
}
}