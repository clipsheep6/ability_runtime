/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "iability_stage_monitor.h"

namespace OHOS {
namespace AppExecFwk {
IAbilityStageMonitor::IAbilityStageMonitor(const std::string &moduleName, const std::string &srcEntrance)
    : moduleName_(moduleName), srcEntrance_(srcEntrance)
{}

bool IAbilityStageMonitor::Match(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage, bool isNotify)
{
    if (!abilityStage) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityStage to match is null");
        return false;
    }
    if (moduleName_.compare(abilityStage->moduleName_) != 0 || srcEntrance_.compare(abilityStage->srcEntrance_) != 0) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "Different abilityStage");
        return false;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR,
        "Matched : abilityStage module name : %{public}s, srcEntrance : %{public}s, isNotify : %{public}s",
        moduleName_.c_str(), srcEntrance_.c_str(), (isNotify ? "true" : "false"));

    if (isNotify) {
        {
            std::lock_guard<std::mutex> matchLock(mtxMatch_);
            matchedAbilityStage_ = abilityStage;
        }
        cvMatch_.notify_one();
    }
    return true;
}

std::shared_ptr<DelegatorAbilityStageProperty> IAbilityStageMonitor::WaitForAbilityStage()
{
    return WaitForAbilityStage(MAX_TIME_OUT);
}

std::shared_ptr<DelegatorAbilityStageProperty> IAbilityStageMonitor::WaitForAbilityStage(const int64_t timeoutMs)
{
    auto realTime = timeoutMs;
    if (timeoutMs <= 0) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "Timeout should be a positive number");
        realTime = MAX_TIME_OUT;
    }

    std::unique_lock<std::mutex> matchLock(mtxMatch_);

    auto condition = [this] { return this->matchedAbilityStage_ != nullptr; };
    if (!cvMatch_.wait_for(matchLock, std::chrono::milliseconds(realTime), condition)) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "Wait abilityStage timeout");
    }
    return matchedAbilityStage_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
