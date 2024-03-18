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
#include <chrono>
#include "hilog_wrapper.h"
#include "iability_monitor.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AppExecFwk {
IAbilityMonitor::IAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

IAbilityMonitor::IAbilityMonitor(const std::string &abilityName,
    const std::string &moduleName) : abilityName_(abilityName), moduleName_(moduleName)
{}

bool IAbilityMonitor::Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify)
{
    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid ability property");
        return false;
    }

    const auto &aName = ability->name_;

    if (abilityName_.empty() || aName.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid name");
        return false;
    }

    if (abilityName_.compare(aName)) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Different name");
        return false;
    }

    const auto &aModuleName = ability->moduleName_;

    if (!moduleName_.empty() && moduleName_.compare(aModuleName) != 0) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Different moduleName, %{public}s and %{public}s.",
            moduleName_.c_str(), aModuleName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "Matched : ability name : %{public}s, isNotify : %{public}s",
        abilityName_.data(), (isNotify ? "true" : "false"));

    if (isNotify) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "Matched : notify ability matched");
        {
            std::lock_guard<std::mutex> matchLock(mMatch_);
            matchedAbility_ = ability;
        }
        cvMatch_.notify_one();
    }

    return true;
}

std::shared_ptr<ADelegatorAbilityProperty> IAbilityMonitor::WaitForAbility()
{
    return WaitForAbility(MAX_TIME_OUT);
}

std::shared_ptr<ADelegatorAbilityProperty> IAbilityMonitor::WaitForAbility(const int64_t timeoutMs)
{
    auto realTime = timeoutMs;
    if (timeoutMs <= 0) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Timeout should be a positive number");
        realTime = MAX_TIME_OUT;
    }

    std::unique_lock<std::mutex> matchLock(mMatch_);

    auto condition = [this] { return this->matchedAbility_ != nullptr; };
    if (!cvMatch_.wait_for(matchLock, realTime * 1ms, condition)) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Wait ability timeout");
    }

    return matchedAbility_;
}

void IAbilityMonitor::OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{}
}  // namespace AppExecFwk
}  // namespace OHOS
