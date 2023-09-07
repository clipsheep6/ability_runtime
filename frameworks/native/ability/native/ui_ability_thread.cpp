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

#include "ui_ability_thread.h"

#include <chrono>

#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "context_deal.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
namespace {
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "UIAbility";
#endif
const int32_t PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS = 3000;
}
UIAbilityThread::UIAbilityThread() : abilityImpl_(nullptr), currentAbility_(nullptr) {}

UIAbilityThread::~UIAbilityThread()
{
    if (currentAbility_) {
        currentAbility_->DetachBaseContext();
        currentAbility_.reset();
    }
}

std::string UIAbilityThread::CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    std::string abilityName;
    if (abilityRecord == nullptr) {
        HILOG_ERROR("abilityRecord is nullptr");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        HILOG_DEBUG("AbilityInfo name is %{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    abilityName = ABILITY_NAME;
#else
    abilityName = abilityInfo->name;
#endif
    HILOG_DEBUG("ability name is %{public}s", abilityName.c_str());
    return abilityName;
}

std::shared_ptr<AppExecFwk::ContextDeal> UIAbilityThread::CreateAndInitContextDeal(
    std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject)
{
    HILOG_DEBUG("called");
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal = nullptr;
    if ((application == nullptr) || (abilityRecord == nullptr) || (abilityObject == nullptr)) {
        HILOG_ERROR("application or abilityRecord or abilityObject is nullptr");
        return contextDeal;
    }

    contextDeal = std::make_shared<AppExecFwk::ContextDeal>();
    if (contextDeal == nullptr) {
        HILOG_ERROR("contextDeal is nullptr");
        return contextDeal;
    }
	
    auto abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr){
        HILOG_ERROR("contextDeal is nullptr");
        return nullptr;
    }
	
    contextDeal->SetAbilityInfo(abilityInfo);
    contextDeal->SetApplicationInfo(application->GetApplicationInfo());
    abilityObject->SetProcessInfo(application->GetProcessInfo());
    std::shared_ptr<AppExecFwk::Context> tmpContext = application->GetApplicationContext();
    contextDeal->SetApplicationContext(tmpContext);
    contextDeal->SetBundleCodePath(abilityInfo->codePath);
    contextDeal->SetContext(abilityObject);
    return contextDeal;
}

void UIAbilityThread::Attach(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("application or abilityRecord or mainRunner is nullptr");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    if (abilityName.empty()) {
        HILOG_ERROR("abilityName is empty");
        return;
    }
    HILOG_DEBUG("begin ability: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(
        BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_, stageContext));

    AttachInner(application, abilityRecord, stageContext);
}

void UIAbilityThread::AttachInner(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    // new abilityImpl
    abilityImpl_ = std::make_shared<UIAbilityImpl>();
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);

    // ability attach : ipc
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("err is %{public}d", err);
        return;
    }
}

void UIAbilityThread::Attach(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("application or abilityRecord is nullptr");
        return;
    }
    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("runner is nullptr");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(
        BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_, stageContext));

    AttachInner(application, abilityRecord, stageContext);
    HILOG_DEBUG("end");
}

void UIAbilityThread::HandleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    HILOG_DEBUG("begin, name is %{public}s", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId, lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName, lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    HILOG_DEBUG("end");
}

void UIAbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
    HILOG_DEBUG("end");
}

void UIAbilityThread::ScheduleSaveAbilityState()
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
    HILOG_DEBUG("end");
}

void UIAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("end");
}

void UIAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }

        abilityThread->HandleUpdateConfiguration(config);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void UIAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("end");
}

void UIAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("name: %{public}s,targeState: %{public}d,isNewWant: %{public}d",
        want.GetElement().GetAbilityName().c_str(), lifeCycleStateInfo.state, lifeCycleStateInfo.isNewWant);

    if (token_ == nullptr) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }

        abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
}

void UIAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!token_) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (!abilityThread) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };

    if (!abilityHandler_) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("postTask error");
    }
}

bool UIAbilityThread::SchedulePrepareTerminateAbility()
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return true;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        HILOG_DEBUG("end ret is %{public}d", ret);
        return ret;
    } else {
        wptr<UIAbilityThread> weak = this;
        auto task = [weak]() {
            auto abilityThread = weak.promote();
            if (abilityThread == nullptr) {
                HILOG_ERROR("abilityThread is nullptr");
                return;
            }
            abilityThread->HandlePrepareTermianteAbility();
        };

        if (abilityHandler_ == nullptr) {
            HILOG_ERROR("abilityHandler_ is nullptr");
            return false;
        }

        bool ret = abilityHandler_->PostTask(task);
        if (!ret) {
            HILOG_ERROR("PostTask error");
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        auto condition = [this] {
            if (this->isPrepareTerminateAbilityDone_) {
                return true;
            } else {
                return false;
            }
        };
        if (!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
            HILOG_WARN("Wait timeout");
        }
        HILOG_DEBUG("end ret is %{public}d", isPrepareTerminate_);
        return isPrepareTerminate_;
    }
}

void UIAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    HILOG_DEBUG("begin");
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (requestCode == -1) {
            HILOG_ERROR("requestCode is -1");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        HILOG_ERROR("abilityImpl_ is nullptr");
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void UIAbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);
}

void UIAbilityThread::NotifyContinuationResult(int32_t result)
{
    HILOG_DEBUG("result: %{public}d", result);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

void UIAbilityThread::NotifyMemoryLevel(int32_t level)
{
    HILOG_DEBUG("result: %{public}d", level);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->NotifyMemoryLevel(level);
}

std::shared_ptr<AbilityContext> UIAbilityThread::BuildAbilityContext(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application, const sptr<IRemoteObject> &token,
    const std::shared_ptr<Context> &stageContext)
{
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    if (abilityContextImpl == nullptr){
        HILOG_ERROR("abilityContextImpl is nullptr");
        return abilityContextImpl;
    }
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetToken(token);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    abilityContextImpl->SetConfiguration(application->GetConfiguration());
    return abilityContextImpl;
}

void UIAbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("begin");
    if (token_ == nullptr) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            HILOG_ERROR("failed err is %{public}d", err);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    abilityHandler_->PostTask(task);
}

#ifdef SUPPORT_GRAPHICS
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("begin");
    if (currentAbility_ == nullptr) {
        HILOG_DEBUG("currentAbility_ is nullptr");
        return;
    }
    auto scene = currentAbility_->GetScene();
    if (scene == nullptr) {
        HILOG_ERROR("scene is nullptr");
        return;
    }

    auto window = scene->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr");
        return;
    }
    window->DumpInfo(params, info);
    currentAbility_->Dump(params, info);
    if (params.empty()) {
        DumpOtherInfo(info);
        return;
    }
    HILOG_DEBUG("end");
}
#else
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("called");
    if (currentAbility_ != nullptr) {
        currentAbility_->Dump(params, info);
    }

    DumpOtherInfo(info);
}
#endif

void UIAbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (!abilityHandler_) {
        HILOG_DEBUG("abilityHandler_ is nullptr");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (!runner) {
        HILOG_DEBUG("runner is nullptr");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (!ablityContext) {
            HILOG_DEBUG("ablityContext is nullptr");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (!localCallContainer) {
            HILOG_DEBUG("localCallContainer is nullptr");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

void UIAbilityThread::CallRequest()
{
    HILOG_DEBUG("begin");
    if (!currentAbility_) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<UIAbility> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval]() {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            HILOG_ERROR("ability is nullptr");
            return;
        }

        retval = currentAbility->CallRequest();
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }
    abilityHandler_->PostSyncTask(syncTask);
    AAFwk::AbilityManagerClient::GetInstance()->CallRequestDone(token_, retval);
    HILOG_DEBUG("end");
}

void UIAbilityThread::HandlePrepareTermianteAbility()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    isPrepareTerminate_ = abilityImpl_->PrepareTerminateAbility();
    HILOG_DEBUG("end ret is %{public}d", isPrepareTerminate_);
    isPrepareTerminateAbilityDone_.store(true);
    cv_.notify_all();
}
} // namespace AbilityRuntime
} // namespace OHOS
