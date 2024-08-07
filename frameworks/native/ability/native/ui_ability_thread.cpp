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

#include "ui_ability_thread.h"

#include <chrono>

#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "context_deal.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "time_util.h"

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
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityRecord");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo name: %{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    abilityName = ABILITY_NAME;
#else
    abilityName = abilityInfo->name;
#endif
    TAG_LOGD(AAFwkTag::UIABILITY, "ability name: %{public}s", abilityName.c_str());
    return abilityName;
}

std::shared_ptr<AppExecFwk::ContextDeal> UIAbilityThread::CreateAndInitContextDeal(
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal = nullptr;
    if (application == nullptr || abilityRecord == nullptr || abilityObject == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or abilityRecord or abilityObject");
        return contextDeal;
    }

    contextDeal = std::make_shared<AppExecFwk::ContextDeal>();
    if (contextDeal == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null contextDeal");
        return contextDeal;
    }

    auto abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityInfo");
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

void UIAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or abilityRecord or mainRunner");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "empty AabilityName");
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "ability: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(abilityName);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
        return;
    }
    ability->SetAbilityRecordId(abilityRecord->GetAbilityRecordId());
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
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(),
        application, token_, stageContext, abilityRecord->GetAbilityRecordId()));

    AttachInner(application, abilityRecord, stageContext);
}

void UIAbilityThread::AttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    // new abilityImpl
    abilityImpl_ = std::make_shared<UIAbilityImpl>();
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);

    // ability attach : ipc
    TAG_LOGI(AAFwkTag::UIABILITY, "attach uiability");
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::LOAD };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; AbilityThread::Attach start; the load lifecycle";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "err: %{public}d", err);
        return;
    }
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
}

void UIAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null application or abilityRecord");
        return;
    }
    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runner");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetUIAbilityByName(abilityName);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
        return;
    }
    ability->SetAbilityRecordId(abilityRecord->GetAbilityRecordId());
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
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(),
        application, token_, stageContext, abilityRecord->GetAbilityRecordId()));

    AttachInner(application, abilityRecord, stageContext);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::HandleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    TAG_LOGD(AAFwkTag::UIABILITY, "name: %{public}s", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    std::string methodName = "HandleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId, lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName, lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::AddLifecycleEvent(uint32_t state, std::string &methodName) const
{
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
        std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
            "; AbilityThread::" + methodName + "; the foreground lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }
    if (state == AAFwk::ABILITY_STATE_BACKGROUND_NEW) {
        FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::BACKGROUND };
        std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
            "; AbilityThread::" + methodName + "; the background lifecycle.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    }
}

void UIAbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
}

void UIAbilityThread::ScheduleSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
}

void UIAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
}

void UIAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    HandleUpdateConfiguration(config);
}

void UIAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
}

void UIAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UIABILITY, "name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);
    std::string methodName = "ScheduleAbilityTransaction";
    AddLifecycleEvent(lifeCycleStateInfo.state, methodName);

    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }

        abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:AbilityTransaction");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
    }
}

void UIAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:ShareData");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
    }
}

bool UIAbilityThread::SchedulePrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return true;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return false;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        TAG_LOGD(AAFwkTag::UIABILITY, "ret: %{public}d", ret);
        return ret;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        abilityThread->HandlePrepareTermianteAbility();
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:PrepareTerminateAbility");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    auto condition = [weak] {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return false;
        }
        return abilityThread->isPrepareTerminateAbilityDone_.load();
    };
    if (!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
        TAG_LOGW(AAFwkTag::UIABILITY, "wait timeout");
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "ret: %{public}d", isPrepareTerminate_);
    return isPrepareTerminate_;
}

void UIAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityHandler_ == nullptr || requestCode == -1) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_ or requestCode is -1");
        return;
    }

    wptr<UIAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }

        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
    };
    bool ret = abilityHandler_->PostTask(task, "UIAbilityThread:SendResult");
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "postTask error");
    }
}

void UIAbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);;
}

void UIAbilityThread::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called, result: %{public}d", result);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

void UIAbilityThread::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called, result: %{public}d", level);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    abilityImpl_->NotifyMemoryLevel(level);
}

std::shared_ptr<AbilityContext> UIAbilityThread::BuildAbilityContext(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application, const sptr<IRemoteObject> &token,
    const std::shared_ptr<Context> &stageContext, int32_t abilityRecordId)
{
    auto abilityContextImpl = std::make_shared<AbilityContextImpl>();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContextImpl");
        return abilityContextImpl;
    }
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetToken(token);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    abilityContextImpl->SetConfiguration(application->GetConfiguration());
    abilityContextImpl->SetAbilityRecordId(abilityRecordId);
    return abilityContextImpl;
}

void UIAbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null token_");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    wptr<UIAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityThread");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "failed err: %{public}d", err);
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:DumpAbilityInfo");
}

#ifdef SUPPORT_SCREEN
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null currentAbility_");
        return;
    }
    auto scene = currentAbility_->GetScene();
    if (scene == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null scene");
        return;
    }

    auto window = scene->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null Window");
        return;
    }
    window->DumpInfo(params, info);
    currentAbility_->Dump(params, info);
    if (params.empty()) {
        DumpOtherInfo(info);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}
#else
void UIAbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
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
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null runner");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (ablityContext == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null Ablitycontext");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (localCallContainer == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null localCallContainer");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

void UIAbilityThread::CallRequest()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<UIAbility> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval]() {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null ability");
            return;
        }

        retval = currentAbility->CallRequest();
    };
    abilityHandler_->PostSyncTask(syncTask, "UIAbilityThread:CallRequest");
    AbilityManagerClient::GetInstance()->CallRequestDone(token_, retval);
    TAG_LOGD(AAFwkTag::UIABILITY, "end");
}

void UIAbilityThread::OnExecuteIntent(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }

    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityHandler_");
        return;
    }

    wptr<UIAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null AbilityThread");
            return;
        }
        if (abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->HandleExecuteInsightIntentBackground(want, true);
            return;
        }
    };
    abilityHandler_->PostTask(task, "UIAbilityThread:OnExecuteIntent");
}

void UIAbilityThread::HandlePrepareTermianteAbility()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (abilityImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null abilityImpl_");
        return;
    }
    isPrepareTerminate_ = abilityImpl_->PrepareTerminateAbility();
    TAG_LOGD(AAFwkTag::UIABILITY, "end ret: %{public}d", isPrepareTerminate_);
    isPrepareTerminateAbilityDone_.store(true);
    cv_.notify_all();
}
#ifdef SUPPORT_SCREEN
int UIAbilityThread::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null current ability");
        return ERR_INVALID_VALUE;
    }
    return currentAbility_->CreateModalUIExtension(want);
}
#endif //SUPPORT_SCREEN
void UIAbilityThread::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    if (currentAbility_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null current ability");
        return;
    }
#ifdef SUPPORT_SCREEN
    currentAbility_->UpdateSessionToken(sessionToken);
#endif //SUPPORT_SCREEN
}
} // namespace AbilityRuntime
} // namespace OHOS
