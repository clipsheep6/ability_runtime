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

#include "ui_ability_impl.h"

#include "ability_handler.h"
#include "ability_manager_client.h"
#include "context/application_context.h"
#include "freeze_util.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_ui_ability.h"
#include "ohos_application.h"
#include "process_options.h"
#include "scene_board_judgement.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
void UIAbilityImpl::Init(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record, std::shared_ptr<UIAbility> &ability,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin.");
    if (token == nullptr || application == nullptr || handler == nullptr ||
        record == nullptr || ability == nullptr) {
        HILOG_ERROR("Token or application or handler or record is nullptr.");
        return;
    }
    token_ = record->GetToken();
    ability_ = ability;
    handler_ = handler;
#ifdef SUPPORT_GRAPHICS
    ability_->SetSceneListener(sptr<WindowLifeCycleImpl>(
        new (std::nothrow) WindowLifeCycleImpl(token_, shared_from_this())));
#endif
    ability_->Init(record, application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::Start(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    ability_->HandleCreateAsRecovery(want);
#endif
    ability_->OnStart(want, sessionInfo);

#ifdef SUPPORT_GRAPHICS
    lifecycleState_ = AAFwk::ABILITY_STATE_STARTED_NEW;
#else
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
#endif
    abilityLifecycleCallbacks_->OnAbilityStart(ability_);
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::Stop()
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }

    ability_->OnStop();
    StopCallback();
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::Stop(bool &isAsyncCallback)
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        isAsyncCallback = false;
        return;
    }

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    if (callbackInfo == nullptr) {
        ability_->OnStop();
        StopCallback();
        isAsyncCallback = false;
        return;
    }
    std::weak_ptr<UIAbilityImpl> weakPtr = shared_from_this();
    auto asyncCallback = [abilityImplWeakPtr = weakPtr, state = AAFwk::ABILITY_STATE_INITIAL]() {
        auto abilityImpl = abilityImplWeakPtr.lock();
        if (abilityImpl == nullptr) {
            HILOG_ERROR("abilityImpl is nullptr.");
            return;
        }
        abilityImpl->StopCallback();
        abilityImpl->AbilityTransactionCallback(state);
    };
    callbackInfo->Push(asyncCallback);
    ability_->OnStop(callbackInfo, isAsyncCallback);
    if (!isAsyncCallback) {
        StopCallback();
        AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    }
    // else: callbackInfo will be destroyed after the async callback
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::StopCallback()
{
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    lifecycleState_ = AAFwk::ABILITY_STATE_STOPED_NEW;
#else
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
#endif
    abilityLifecycleCallbacks_->OnAbilityStop(ability_);
    ability_->DestroyInstance(); // Release window and ability.
}

int32_t UIAbilityImpl::Share(AAFwk::WantParams &wantParam)
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return ability_->OnShare(wantParam);
}

void UIAbilityImpl::DispatchSaveAbilityState()
{
    HILOG_DEBUG("Called.");
    needSaveDate_ = true;
}

void UIAbilityImpl::DispatchRestoreAbilityState(const AppExecFwk::PacMap &inState)
{
    HILOG_DEBUG("Called.");
    hasSaveData_ = true;
    restoreData_ = inState;
}

void UIAbilityImpl::HandleAbilityTransaction(
    const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Lifecycle: srcState:%{public}d; targetState: %{public}d; isNewWant: %{public}d, sceneFlag: %{public}d",
        lifecycleState_, targetState.state, targetState.isNewWant, targetState.sceneFlag);
#ifdef SUPPORT_GRAPHICS
    if (ability_ != nullptr) {
        ability_->sceneFlag_ = targetState.sceneFlag;
    }
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        if (ability_ != nullptr && targetState.state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
            ability_->RequestFocus(want);
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, targetState.state, GetRestoreData());
        }
        HILOG_ERROR("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }
#endif
    SetLifeCycleStateInfo(targetState);
    UpdateSilentForeground(sessionInfo);
    if (ability_ != nullptr) {
        ability_->SetLaunchParam(targetState.launchParam);
        if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
            ability_->SetStartAbilitySetting(targetState.setting);
            Start(want, sessionInfo);
            CheckAndRestore();
        }
    }

    bool ret = false;
    ret = AbilityTransaction(want, targetState);
    if (ret) {
        AbilityTransactionCallback(targetState.state);
    }
}

void UIAbilityImpl::HandleShareData(int32_t uniqueId)
{
    HILOG_DEBUG("Called sourceState: %{public}d.", lifecycleState_);
    WantParams wantParam;
    int32_t resultCode = Share(wantParam);
    HILOG_DEBUG("WantParam size: %{public}d.", wantParam.Size());
    AAFwk::AbilityManagerClient::GetInstance()->ShareDataDone(token_, resultCode, uniqueId, wantParam);
}

void UIAbilityImpl::AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state)
{
    HILOG_DEBUG("Lifecycle: notify ability manager service.");
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; AbilityManagerClient::AbilityTransitionDone; the transaction start.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
    if (state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, state, GetRestoreData());
    if (ret == ERR_OK && state == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    }
}

void UIAbilityImpl::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
    HILOG_INFO("Notify execute done, intentId %{public}" PRIu64"", intentId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result);
    if (ret != ERR_OK) {
        HILOG_ERROR("Notify execute done faild.");
    }
}

bool UIAbilityImpl::PrepareTerminateAbility()
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return false;
    }
    bool ret = ability_->OnPrepareTerminate();
    HILOG_DEBUG("End ret is %{public}d.", ret);
    return ret;
}

void UIAbilityImpl::SendResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }

    ability_->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::NewWant(const AAFwk::Want &want)
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
#ifdef SUPPORT_GRAPHICS
    ability_->ContinuationRestore(want);
#endif
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }
    ability_->SetLifeCycleStateInfo(info);
}

bool UIAbilityImpl::CheckAndRestore()
{
    HILOG_DEBUG("Begin.");
    if (!hasSaveData_) {
        HILOG_ERROR("hasSaveData_ is false.");
        return false;
    }

    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return false;
    }
    ability_->OnRestoreAbilityState(restoreData_);
    HILOG_DEBUG("End.");
    return true;
}

AppExecFwk::PacMap &UIAbilityImpl::GetRestoreData()
{
    return restoreData_;
}

void UIAbilityImpl::SetCallingContext(const std::string &deviceId, const std::string &bundleName,
    const std::string &abilityName, const std::string &moduleName)
{
    if (ability_ != nullptr) {
        ability_->SetCallingContext(deviceId, bundleName, abilityName, moduleName);
    }
}

void UIAbilityImpl::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        HILOG_DEBUG("Ability name: [%{public}s].", ability_->GetAbilityName().c_str());
        ability_->OnConfigurationUpdatedNotify(config);
    }
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId, versionCode);
}

void UIAbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void UIAbilityImpl::NotifyMemoryLevel(int32_t level)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr.");
        return;
    }
    ability_->OnMemoryLevel(level);
}

void UIAbilityImpl::UpdateSilentForeground(sptr<AAFwk::SessionInfo> sessionInfo)
{
    if (ability_ == nullptr) {
        HILOG_DEBUG("ability_ is null");
        return;
    }
    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL &&
        sessionInfo && sessionInfo->processOptions &&
        AAFwk::ProcessOptions::IsNewProcessMode(sessionInfo->processOptions->processMode) &&
        sessionInfo->processOptions->startupVisibility == AAFwk::StartupVisibility::STARTUP_HIDE) {
        HILOG_INFO("Set IsSilentForeground to true.");
        ability_->SetIsSilentForeground(true);
        return;
    }
    ability_->SetIsSilentForeground(false);
}

#ifdef SUPPORT_GRAPHICS
void UIAbilityImpl::AfterUnFocused()
{
    AfterFocusedCommon(false);
}

void UIAbilityImpl::AfterFocused()
{
    AfterFocusedCommon(true);
}

void UIAbilityImpl::AfterFocusedCommon(bool isFocused)
{
    auto task = [abilityImpl = weak_from_this(), focuseMode = isFocused]() {
        auto impl = abilityImpl.lock();
        if (impl == nullptr) {
            HILOG_ERROR("impl is nullptr.");
            return;
        }

        if (!impl->ability_ || !impl->ability_->GetAbilityInfo()) {
            HILOG_ERROR("%{public}s failed.", focuseMode ? "AfterFocused" : "AfterUnFocused");
            return;
        }

        auto abilityContext = impl->ability_->GetAbilityContext();
        if (abilityContext == nullptr) {
            HILOG_ERROR("abilityContext is nullptr.");
            return;
        }
        auto applicationContext = abilityContext->GetApplicationContext();
        if (applicationContext == nullptr || applicationContext->IsAbilityLifecycleCallbackEmpty()) {
            HILOG_ERROR("applicationContext or lifecycleCallback is nullptr.");
            return;
        }
        auto &jsAbility = static_cast<JsUIAbility &>(*(impl->ability_));
        if (focuseMode) {
            applicationContext->DispatchWindowStageFocus(jsAbility.GetJsAbility(), jsAbility.GetJsWindowStage());
        } else {
            applicationContext->DispatchWindowStageUnfocus(jsAbility.GetJsAbility(), jsAbility.GetJsWindowStage());
        }
    };

    if (handler_) {
        handler_->PostTask(task);
    }
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Lifecycle: Call.");
    auto owner = owner_.lock();
    if (owner == nullptr) {
        HILOG_ERROR("Owner is nullptr.");
        return;
    }
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::FOREGROUND };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; UIAbilityImpl::WindowLifeCycleImpl::AfterForeground; the foreground lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);

    bool needNotifyAMS = false;
    {
        std::lock_guard<std::mutex> lock(owner->notifyForegroundLock_);
        if (owner->notifyForegroundByAbility_) {
            owner->notifyForegroundByAbility_ = false;
            needNotifyAMS = true;
        } else {
            HILOG_DEBUG("Notify foreground by window, but client's foreground is running.");
            owner->notifyForegroundByWindow_ = true;
        }
    }

    if (needNotifyAMS) {
        HILOG_INFO("Lifecycle: window notify ability manager service.");
        entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
            "; AbilityManagerClient::AbilityTransitionDone; the transaction start.";
        FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
        owner->lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
        AppExecFwk::PacMap restoreData;
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
        if (ret == ERR_OK) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
        }
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Called.");
    FreezeUtil::LifecycleFlow flow = { token_, FreezeUtil::TimeoutState::BACKGROUND };
    std::string entry = std::to_string(TimeUtil::SystemTimeMillisecond()) +
        "; UIAbilityImpl::WindowLifeCycleImpl::AfterBackground; the background lifecycle.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);

    HILOG_DEBUG("Lifecycle: window after background.");
    AppExecFwk::PacMap restoreData;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
        token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
    if (ret == ERR_OK) {
        FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    HILOG_DEBUG("Begin.");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    HILOG_DEBUG("Begin.");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::ForegroundFailed(int32_t type)
{
    HILOG_DEBUG("Begin.");
    AppExecFwk::PacMap restoreData;
    switch (type) {
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_OPERATION): {
            HILOG_DEBUG("Window was freezed.");
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_WINDOW_FREEZED, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE): {
            HILOG_DEBUG("The ability is stage mode, schedule foreground invalid mode.");
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_INVALID_WINDOW_MODE, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING): {
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_DO_NOTHING, restoreData);
            break;
        }
        default: {
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_FAILED, restoreData);
            break;
        }
    }
}

void UIAbilityImpl::Foreground(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }

    HILOG_DEBUG("Call onForeground.");
    ability_->OnForeground(want);
    if (ability_->CheckIsSilentForeground()) {
        HILOG_INFO("Is silent foreground.");
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = true;
        return;
    }
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }
    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);
    HILOG_DEBUG("End.");
}

void UIAbilityImpl::WindowLifeCycleImpl::BackgroundFailed(int32_t type)
{
    HILOG_DEBUG("Called.");
    if (type == static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING)) {
        AppExecFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_FAILED, restoreData);
    }
}

void UIAbilityImpl::Background()
{
    HILOG_DEBUG("Begin.");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr.");
        return;
    }
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    abilityLifecycleCallbacks_->OnAbilityBackground(ability_);
    HILOG_DEBUG("End.");
}
#endif

bool UIAbilityImpl::AbilityTransaction(const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    bool ret = true;
    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
#ifdef SUPPORT_GRAPHICS
            if (!Rosen::SceneBoardJudgement::IsSceneBoardEnabled() &&
                lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
                Background();
            }
#endif
            bool isAsyncCallback = false;
            Stop(isAsyncCallback);
            if (isAsyncCallback) {
                // AbilityManagerService will be notified after async callback
                ret = false;
            }
            break;
        }
        case AAFwk::ABILITY_STATE_FOREGROUND_NEW: {
            if (targetState.isNewWant) {
                NewWant(want);
            }
#ifdef SUPPORT_GRAPHICS
            if (!InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
                HandleForegroundNewState(want, ret);
            } else {
                HandleExecuteInsightIntentForeground(want, ret);
            }
#endif
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != AAFwk::ABILITY_STATE_STARTED_NEW) {
                ret = false;
            }
#ifdef SUPPORT_GRAPHICS
            if (!InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
                Background();
            } else {
                HILOG_DEBUG("HandleExecuteInsightIntentBackground");
                HandleExecuteInsightIntentBackground(want);
            }
#endif
            break;
        }
        default: {
            ret = false;
            HILOG_ERROR("State error.");
            break;
        }
    }
    HILOG_DEBUG("End retVal is %{public}d.", static_cast<int>(ret));
    return ret;
}

#ifdef SUPPORT_GRAPHICS
void UIAbilityImpl::HandleForegroundNewState(const AAFwk::Want &want, bool &bflag)
{
    if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        if (ability_) {
            ability_->RequestFocus(want);
        }
    } else {
        {
            std::lock_guard<std::mutex> lock(notifyForegroundLock_);
            notifyForegroundByWindow_ = false;
        }
        Foreground(want);
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        bflag = notifyForegroundByWindow_;
        if (bflag) {
            notifyForegroundByWindow_ = false;
            notifyForegroundByAbility_ = false;
        }
    }
}

void UIAbilityImpl::HandleExecuteInsightIntentForeground(const AAFwk::Want &want, bool &bflag)
{
    HILOG_INFO("Execute insight intent in foreground mode.");
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        HILOG_ERROR("Generate execute param failed.");
        HandleForegroundNewState(want, bflag);
        return;
    }

    HILOG_DEBUG("Insight intent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto intentCb = std::make_unique<InsightIntentExecutorAsyncCallback>();
    intentCb.reset(InsightIntentExecutorAsyncCallback::Create());
    if (intentCb == nullptr) {
        HILOG_ERROR("Create async callback failed.");
        HandleForegroundNewState(want, bflag);
        return;
    }

    if (lifecycleState_ == AAFwk::ABILITY_STATE_FOREGROUND_NEW) {
        ExecuteInsightIntentRepeateForeground(want, executeParam, std::move(intentCb));
    } else {
        ExecuteInsightIntentMoveToForeground(want, executeParam, std::move(intentCb));
    }

    bflag = false;
}

void UIAbilityImpl::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            HILOG_DEBUG("Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                HILOG_ERROR("Ability impl is nullptr.");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->AbilityTransactionCallback(AAFwk::ABILITY_STATE_FOREGROUND_NEW);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentRepeateForeground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");

    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByWindow_ = false;
    }

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            HILOG_DEBUG("Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                HILOG_ERROR("Ability impl is nullptr.");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
            abilityImpl->PostForegroundInsightIntent();
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentMoveToForeground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::PostForegroundInsightIntent()
{
    HILOG_DEBUG("called");
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("Ability params invalid.");
        return;
    }

    lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;

    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }

    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);

    bool flag = true;
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        flag = notifyForegroundByWindow_;
        if (flag) {
            notifyForegroundByWindow_ = false;
            notifyForegroundByAbility_ = false;
        }
    }

    if (flag) {
        AbilityTransactionCallback(AAFwk::ABILITY_STATE_FOREGROUND_NEW);
    }
}

void UIAbilityImpl::HandleExecuteInsightIntentBackground(const AAFwk::Want &want, bool onlyExecuteIntent)
{
    HILOG_INFO("Execute insight intent in background mode.");
    auto executeParam = std::make_shared<InsightIntentExecuteParam>();
    auto ret = InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret && !onlyExecuteIntent) {
        HILOG_ERROR("Generate execute param failed.");
        Background();
        return;
    }

    HILOG_DEBUG("Insight intent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);

    auto intentCb = std::make_unique<InsightIntentExecutorAsyncCallback>();
    intentCb.reset(InsightIntentExecutorAsyncCallback::Create());
    if (intentCb == nullptr && !onlyExecuteIntent) {
        HILOG_ERROR("Create async callback failed.");
        Background();
        return;
    }
    HILOG_DEBUG("lifecycleState_: %{public}d", lifecycleState_);
    if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL
        || lifecycleState_ == AAFwk::ABILITY_STATE_STARTED_NEW) {
        ExecuteInsightIntentBackgroundByColdBoot(want, executeParam, std::move(intentCb));
    } else {
        ExecuteInsightIntentBackgroundAlreadyStart(want, executeParam, std::move(intentCb));
    }
}

void UIAbilityImpl::ExecuteInsightIntentBackgroundByColdBoot(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");
    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            HILOG_DEBUG("Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                HILOG_ERROR("Ability impl is nullptr.");
                return;
            }
            abilityImpl->Background();
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentBackground(want, executeParam, std::move(callback));
}

void UIAbilityImpl::ExecuteInsightIntentBackgroundAlreadyStart(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");

    auto asyncCallback =
        [weak = weak_from_this(), intentId = executeParam->insightIntentId_](InsightIntentExecuteResult result) {
            HILOG_DEBUG("Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
            auto abilityImpl = weak.lock();
            if (abilityImpl == nullptr) {
                HILOG_ERROR("Ability impl is nullptr.");
                return;
            }
            abilityImpl->ExecuteInsightIntentDone(intentId, result);
        };
    callback->Push(asyncCallback);

    // private function, no need check ability_ validity.
    ability_->ExecuteInsightIntentBackground(want, executeParam, std::move(callback));
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
