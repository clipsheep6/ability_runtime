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

#include "ability_delegator.h"

#include "hilog_wrapper.h"
#include "ohos_application.h"
#include "ability_manager_client.h"
#include "ability_delegator_registry.h"
#include "itest_observer.h"

namespace OHOS {
namespace AppExecFwk {
AbilityDelegator::AbilityDelegator(const std::shared_ptr<AbilityRuntime::Context> &context,
    std::unique_ptr<TestRunner> runner, const sptr<IRemoteObject> &observer)
    : appContext_(context), testRunner_(std::move(runner)), observer_(observer)
{}

AbilityDelegator::~AbilityDelegator()
{}

void AbilityDelegator::AddAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Monitor has been added");
        return;
    }

    abilityMonitors_.emplace_back(monitor);
}

void AbilityDelegator::AddAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    auto pos = std::find(abilityStageMonitors_.begin(), abilityStageMonitors_.end(), monitor);
    if (pos != abilityStageMonitors_.end()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Stage monitor has been added");
        return;
    }
    abilityStageMonitors_.emplace_back(monitor);
}

void AbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        abilityMonitors_.erase(pos);
    }
}

void AbilityDelegator::RemoveAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    auto pos = std::find(abilityStageMonitors_.begin(), abilityStageMonitors_.end(), monitor);
    if (pos != abilityStageMonitors_.end()) {
        abilityStageMonitors_.erase(pos);
    }
}

void AbilityDelegator::ClearAllMonitors()
{
    std::unique_lock<std::mutex> lck(mutexMonitor_);
    abilityMonitors_.clear();
}

size_t AbilityDelegator::GetMonitorsNum()
{
    std::unique_lock<std::mutex> lck(mutexMonitor_);
    return abilityMonitors_.size();
}

size_t AbilityDelegator::GetStageMonitorsNum()
{
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    return abilityStageMonitors_.size();
}


std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->WaitForAbility();
    if (!obtainedAbility) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid obtained ability");
        return {};
    }

    return obtainedAbility;
}

std::shared_ptr<DelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid monitor for waiting abilityStage");
        return nullptr;
    }

    AddAbilityStageMonitor(monitor);
    auto stage = monitor->WaitForAbilityStage();
    if (!stage) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid obtained abilityStage");
        return nullptr;
    }
    return stage;
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return {};
    }

    AddAbilityMonitor(monitor);

    auto obtainedAbility = monitor->WaitForAbility(timeoutMs);
    if (!obtainedAbility) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid obtained ability");
        return {};
    }

    return obtainedAbility;
}

std::shared_ptr<DelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor, const int64_t timeoutMs)
{
    if (!monitor) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid monitor for waiting abilityStage with timeout");
        return nullptr;
    }
    AddAbilityStageMonitor(monitor);
    auto stage = monitor->WaitForAbilityStage(timeoutMs);
    if (!stage) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid obtained abilityStage");
        return nullptr;
    }
    return stage;
}

std::shared_ptr<AbilityRuntime::Context> AbilityDelegator::GetAppContext() const
{
    return appContext_;
}

AbilityDelegator::AbilityState AbilityDelegator::GetAbilityState(const sptr<IRemoteObject> &token)
{
    if (!token) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(token);
    if (!existedProperty) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Unknown ability token");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    return ConvertAbilityState(existedProperty->lifecycleState_);
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::GetCurrentTopAbility()
{
    AppExecFwk::ElementName elementName = AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility();
    std::string bundleName = elementName.GetBundleName();
    std::string abilityName = elementName.GetAbilityName();
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Failed to get top ability");
        return {};
    }

    if (!bundleName.empty()) {
        std::string::size_type pos = abilityName.find(bundleName);
        if (pos == std::string::npos || pos != 0) {
            abilityName = bundleName + "." + abilityName;
        }
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByName(abilityName);
    if (!existedProperty) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Unknown ability name");
        return {};
    }

    return existedProperty;
}

std::string AbilityDelegator::GetThreadName() const
{
    return {};
}

void AbilityDelegator::Prepare()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!testRunner_) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid TestRunner");
        return;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "Call TestRunner::Prepare()");
    testRunner_->Prepare();

    if (!delegatorThread_) {
        delegatorThread_ = std::make_unique<DelegatorThread>(true);
        if (!delegatorThread_) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Create delegatorThread failed");
            return;
        }
    }

    auto runTask = [this]() { this->OnRun(); };
    if (!delegatorThread_->Run(runTask)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Run task on delegatorThread failed");
    }
}

void AbilityDelegator::OnRun()
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!testRunner_) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid TestRunner");
        return;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "Call TestRunner::Run(), Start run");
    testRunner_->Run();
    TAG_LOGI(AAFwkTag::DELEGATOR, "Run finished");
}

ErrCode AbilityDelegator::StartAbility(const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    auto realWant(want);
    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (delegatorArgs && delegatorArgs->FindDebugFlag()) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "Set Debug to the want which used for starting the ability");
        realWant.SetParam("debugApp", true);
    }
    realWant.SetParam(IS_DELEGATOR_CALL, true);

    return AbilityManagerClient::GetInstance()->StartAbility(realWant);
}

bool AbilityDelegator::DoAbilityForeground(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!token) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityForeground(token);
    if (ret) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Failed to call DelegatorDoAbilityForeground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

bool AbilityDelegator::DoAbilityBackground(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!token) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return false;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DelegatorDoAbilityBackground(token);
    if (ret) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Failed to call DelegatorDoAbilityBackground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

std::unique_ptr<ShellCmdResult> AbilityDelegator::ExecuteShellCommand(const std::string &cmd, const int64_t timeoutSec)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "command : %{public}s, timeout : %{public}" PRId64, cmd.data(), timeoutSec);

    if (cmd.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Invalid cmd");
        return {};
    }

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid testObserver");
        return {};
    }

    auto result = testObserver->ExecuteShellCommand(cmd, timeoutSec);
    return std::make_unique<ShellCmdResult>(result);
}

void AbilityDelegator::Print(const std::string &msg)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    auto testObserver = iface_cast<ITestObserver>(observer_);
    if (!testObserver) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid testObserver");
        return;
    }

    auto realMsg(msg);
    if (realMsg.length() > INFORMATION_MAX_LENGTH) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Too long message");
        realMsg.resize(INFORMATION_MAX_LENGTH);
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "message to print : %{public}s", realMsg.data());

    testObserver->TestStatus(realMsg, 0);
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability, true)) {
            monitor->OnAbilityStart(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformStageStart(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!abilityStage) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    if (abilityStageMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Empty abilityStageMonitors");
        return;
    }

    for (auto &monitor : abilityStageMonitors_) {
        if (!monitor) {
            continue;
        }
        monitor->Match(abilityStage, true);
    }
}

void AbilityDelegator::PostPerformScenceCreated(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Empty abilityMonitors.");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageCreate(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformScenceRestored(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "abilityMonitors is Empty");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageRestore(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformScenceDestroyed(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "abilityMonitors is empty");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageDestroy(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformForeground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "abilityMonitors is Empty.");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityForeground(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformBackground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "abilityMonitors is empty.");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityBackground(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformStop(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Empty abilityMonitors");
        return;
    }

    for (auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityStop(ability->object_);
        }
    }

    RemoveAbilityProperty(ability);
    CallClearFunc(ability);
}

AbilityDelegator::AbilityState AbilityDelegator::ConvertAbilityState(
    const AbilityLifecycleExecutor::LifecycleState lifecycleState)
{
    AbilityDelegator::AbilityState abilityState {AbilityDelegator::AbilityState::UNINITIALIZED};
    switch (lifecycleState) {
        case AbilityLifecycleExecutor::LifecycleState::STARTED_NEW:
            abilityState = AbilityDelegator::AbilityState::STARTED;
            break;
        case AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::FOREGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::BACKGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::STOPED_NEW:
            abilityState = AbilityDelegator::AbilityState::STOPPED;
            break;
        default:
            TAG_LOGE(AAFwkTag::DELEGATOR, "Unknown lifecycleState");
            break;
    }

    return abilityState;
}

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid ability property");
        return;
    }

   TAG_LOGW(AAFwkTag::DELEGATOR, "ability property : name : %{public}s, state : %{public}d",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByToken(ability->token_);
    if (existedProperty) {
        // update
        existedProperty->lifecycleState_ = ability->lifecycleState_;
        return;
    }

    abilityProperties_.emplace_back(ability);
}

void AbilityDelegator::RemoveAbilityProperty(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!ability) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid ability property");
        return;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "ability property { name : %{public}s, state : %{public}d }",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    abilityProperties_.remove_if([ability](const auto &properties) {
        return ability->fullName_ == properties->fullName_;
    });
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::FindPropertyByToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");

    if (!token) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return {};
    }

    for (const auto &it : abilityProperties_) {
        if (!it) {
           TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid ability property");
            continue;
        }

        if (token == it->token_) {
            TAG_LOGI(AAFwkTag::DELEGATOR, "Property exists");
            return it;
        }
    }

    return {};
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::FindPropertyByName(const std::string &name)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Find property by %{public}s.", name.c_str());

    if (name.empty()) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid input parameter");
        return {};
    }

    for (const auto &it : abilityProperties_) {
        if (!it) {
           TAG_LOGW(AAFwkTag::DELEGATOR, "Invalid ability property");
            continue;
        }

        if (name == it->fullName_) {
            TAG_LOGI(AAFwkTag::DELEGATOR, "Property exists");
            return it;
        }
    }

    return {};
}

void AbilityDelegator::FinishUserTest(const std::string &msg, const int64_t resultCode)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter, msg : %{public}s, code : %{public}" PRId64, msg.data(), resultCode);

    if (!observer_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Invalid observer");
        return;
    }

    auto delegatorArgs = AbilityDelegatorRegistry::GetArguments();
    if (!delegatorArgs) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Invalid delegator args");
        return;
    }

    auto realMsg(msg);
    if (realMsg.length() > INFORMATION_MAX_LENGTH) {
       TAG_LOGW(AAFwkTag::DELEGATOR, "Too long message");
        realMsg.resize(INFORMATION_MAX_LENGTH);
    }

    const auto &bundleName = delegatorArgs->GetTestBundleName();
    auto err = AAFwk::AbilityManagerClient::GetInstance()->FinishUserTest(realMsg, resultCode, bundleName);
    if (err) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Failed to call FinishUserTest : %{public}d", err);
    }
}

void AbilityDelegator::RegisterClearFunc(ClearFunc func)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (!func) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Invalid func");
        return;
    }

    clearFunc_ = func;
}

inline void AbilityDelegator::CallClearFunc(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "Enter");
    if (clearFunc_) {
        clearFunc_(ability);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
