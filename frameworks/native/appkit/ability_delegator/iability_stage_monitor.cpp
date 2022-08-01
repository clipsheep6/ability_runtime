/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <chrono>
#include "hilog_wrapper.h"
#include "iability_stage_monitor.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AppExecFwk {
IAbilityStageMonitor::IAbilityStageMonitor(const std::string &moduleName, const std::string &stagePath) 
    : moduleName_(moduleName), stagePath_(stagePath)
{}

bool IAbilityStageMonitor::Match(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage, bool isNotify)
{
    if (!abilityStage) {
        HILOG_ERROR("abilityStage param is null");
        return false;
    }

    if (moduleName_.compare(abilityStage->moduleName_) != 0 || stagePath_.compare(abilityStage->stagePath_) != 0) {
        HILOG_WARN("Different abilityStage");
        return false;
    }

    HILOG_INFO("Matched : abilityStage module name : %{public}s, stagePath : %{public}s, isNotify : %{public}s",
        moduleName_.c_str(), stagePath_.c_str(), (isNotify ? "true" : "false"));

    if (isNotify) {
        HILOG_INFO("Matched : notify abilityStage matched");
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
        HILOG_WARN("Timeout should be a positive number");
        realTime = MAX_TIME_OUT;
    }

    std::unique_lock<std::mutex> matchLock(mtxMatch_);

    auto condition = [this] { return this->matchedAbilityStage_ != nullptr; };
    if (!cvMatch_.wait_for(matchLock, realTime * 1ms, condition)) {
        HILOG_WARN("Wait ability timeout");
    }
    return matchedAbilityStage_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
