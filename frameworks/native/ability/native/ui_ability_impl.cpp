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
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_ui_ability.h"
#include "ohos_application.h"
#include "scene_board_judgement.h"

namespace OHOS {
namespace AbilityRuntime {
void UIAbilityImpl::Init(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record, std::shared_ptr<UIAbility> &ability,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        ability == nullptr) {
        HILOG_ERROR("token or application or handler or record is nullptr");
        return;
    }
    token_ = record->GetToken();
    ability_ = ability;
    handler_ = handler;
    auto info = record->GetAbilityInfo();
#ifdef SUPPORT_GRAPHICS
    if (info && info->type == AppExecFwk::AbilityType::PAGE) {
        ability_->SetSceneListener(sptr<WindowLifeCycleImpl>(new WindowLifeCycleImpl(token_, shared_from_this())));
    }
#endif
    ability_->Init(record->GetAbilityInfo(), application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    abilityLifecycleCallbacks_ = application;
    HILOG_DEBUG("end");
}

void UIAbilityImpl::Start(const AAFwk::Want &want, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
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
    HILOG_DEBUG("end");
}

void UIAbilityImpl::Stop()
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    ability_->OnStop();
    StopCallback();
    HILOG_DEBUG("end");
}

void UIAbilityImpl::Stop(bool &isAsyncCallback)
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
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
            HILOG_ERROR("abilityImpl is nullptr");
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
    HILOG_DEBUG("end");
}

void UIAbilityImpl::StopCallback()
{
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
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
    HILOG_DEBUG("begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    return ability_->OnShare(wantParam);
}

void UIAbilityImpl::DispatchSaveAbilityState()
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    needSaveDate_ = true;
    HILOG_DEBUG("end");
}

void UIAbilityImpl::DispatchRestoreAbilityState(const AppExecFwk::PacMap &inState)
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }

    hasSaveData_ = true;
    restoreData_ = inState;
    HILOG_DEBUG("end");
}

void UIAbilityImpl::HandleAbilityTransaction(
    const AAFwk::Want &want, const AAFwk::LifeCycleStateInfo &targetState, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("called sourceState:%{public}d, targetState: %{public}d, isNewWant: %{public}d, sceneFlag: %{public}d",
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
        HILOG_ERROR("Org lifeCycleState equals to Dst lifeCycleState");
        return;
    }
#endif
    SetLifeCycleStateInfo(targetState);
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
    HILOG_DEBUG("called sourceState: %{public}d", lifecycleState_);
    WantParams wantParam;
    int32_t resultCode = Share(wantParam);
    HILOG_DEBUG("wantParam size: %{public}d", wantParam.Size());
    AAFwk::AbilityManagerClient::GetInstance()->ShareDataDone(token_, resultCode, uniqueId, wantParam);
}

void UIAbilityImpl::AbilityTransactionCallback(const AAFwk::AbilityLifeCycleState &state)
{
    HILOG_DEBUG("called");
    AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, state, GetRestoreData());
}

bool UIAbilityImpl::PrepareTerminateAbility()
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return false;
    }
    bool ret = ability_->OnPrepareTerminate();
    HILOG_DEBUG("end ret is %{public}d", ret);
    return ret;
}

void UIAbilityImpl::SendResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }

    ability_->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_DEBUG("end");
}

void UIAbilityImpl::NewWant(const AAFwk::Want &want)
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }
    ability_->SetWant(want);
    ability_->OnNewWant(want);
#ifdef SUPPORT_GRAPHICS
    ability_->ContinuationRestore(want);
#endif
    HILOG_DEBUG("end");
}

void UIAbilityImpl::SetLifeCycleStateInfo(const AAFwk::LifeCycleStateInfo &info)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }
    ability_->SetLifeCycleStateInfo(info);
}

bool UIAbilityImpl::CheckAndRestore()
{
    HILOG_DEBUG("begin");
    if (!hasSaveData_) {
        HILOG_ERROR("hasSaveData_ is false");
        return false;
    }

    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return false;
    }
    ability_->OnRestoreAbilityState(restoreData_);

    HILOG_DEBUG("end");
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
    HILOG_DEBUG("begin");
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        HILOG_DEBUG("ability name: [%{public}s]", ability_->GetAbilityName().c_str());
        ability_->OnConfigurationUpdatedNotify(config);
    }
    HILOG_DEBUG("end");
}

void UIAbilityImpl::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }
    ability_->ContinueAbilityWithStack(deviceId, versionCode);
}

void UIAbilityImpl::NotifyContinuationResult(int32_t result)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }
    ability_->OnCompleteContinuation(result);
}

void UIAbilityImpl::NotifyMemoryLevel(int32_t level)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("ability_ is nullptr");
        return;
    }
    ability_->OnMemoryLevel(level);
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
            return;
        }

        if (!impl->ability_ || !impl->ability_->GetAbilityInfo()) {
            HILOG_WARN("%{public}s failed", focuseMode ? "AfterFocused" : "AfterUnFocused");
            return;
        }

        auto abilityContext = impl->ability_->GetAbilityContext();
        if (abilityContext == nullptr) {
            return;
        }
        auto applicationContext = abilityContext->GetApplicationContext();
        if (applicationContext == nullptr || applicationContext->IsAbilityLifecycleCallbackEmpty()) {
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
    HILOG_DEBUG("end");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    auto owner = owner_.lock();
    if (owner == nullptr) {
        HILOG_ERROR("owner is nullptr");
        return;
    }
    bool needNotifyAMS = false;
    {
        std::lock_guard<std::mutex> lock(owner->notifyForegroundLock_);
        if (owner->notifyForegroundByAbility_) {
            owner->notifyForegroundByAbility_ = false;
            needNotifyAMS = true;
        } else {
            HILOG_DEBUG("Notify foreground by window, but client's foreground is running");
            owner->notifyForegroundByWindow_ = true;
        }
    }

    if (needNotifyAMS) {
        HILOG_DEBUG("Stage mode ability, window after foreground, notify ability manager service");
        AppExecFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_FOREGROUND_NEW, restoreData);
    }
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    AppExecFwk::PacMap restoreData;
    AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
        token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_NEW, restoreData);
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterFocused()
{
    HILOG_DEBUG("begin");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterFocused();
    }
    HILOG_DEBUG("end");
}

void UIAbilityImpl::WindowLifeCycleImpl::AfterUnfocused()
{
    HILOG_DEBUG("begin");
    auto owner = owner_.lock();
    if (owner) {
        owner->AfterUnFocused();
    }
    HILOG_DEBUG("end");
}

void UIAbilityImpl::WindowLifeCycleImpl::ForegroundFailed(int32_t type)
{
    HILOG_DEBUG("begin");
    AppExecFwk::PacMap restoreData;
    switch (type) {
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_OPERATION): {
            HILOG_DEBUG("window was freezed");
            AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
                token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_WINDOW_FREEZED, restoreData);
            break;
        }
        case static_cast<int32_t>(OHOS::Rosen::WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE): {
            HILOG_DEBUG("The ability is stage mode, schedule foreground invalid mode");
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
    HILOG_DEBUG("begin");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }

    HILOG_DEBUG("onForeground");
    ability_->OnForeground(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
    {
        std::lock_guard<std::mutex> lock(notifyForegroundLock_);
        notifyForegroundByAbility_ = true;
    }
    abilityLifecycleCallbacks_->OnAbilityForeground(ability_);
    HILOG_DEBUG("end");
}

void UIAbilityImpl::WindowLifeCycleImpl::BackgroundFailed(int32_t type)
{
    HILOG_DEBUG("begin");
    if (type == static_cast<int32_t>(OHOS::Rosen::WMError::WM_DO_NOTHING)) {
        AppExecFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(
            token_, AAFwk::AbilityLifeCycleState::ABILITY_STATE_BACKGROUND_FAILED, restoreData);
    }
}

void UIAbilityImpl::Background()
{
    HILOG_DEBUG("begin");
    if (ability_ == nullptr || ability_->GetAbilityInfo() == nullptr || abilityLifecycleCallbacks_ == nullptr) {
        HILOG_ERROR("ability_ or abilityLifecycleCallbacks_ is nullptr");
        return;
    }
    ability_->OnLeaveForeground();
    ability_->OnBackground();
    lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
    abilityLifecycleCallbacks_->OnAbilityBackground(ability_);
    HILOG_DEBUG("end");
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
            HandleForegroundNewState(want, ret);
#endif
            break;
        }
        case AAFwk::ABILITY_STATE_BACKGROUND_NEW: {
            if (lifecycleState_ != AAFwk::ABILITY_STATE_STARTED_NEW) {
                ret = false;
            }
#ifdef SUPPORT_GRAPHICS
            Background();
#endif
            break;
        }
        default: {
            ret = false;
            HILOG_ERROR("state error");
            break;
        }
    }
    HILOG_DEBUG("end retVal is %{public}d", static_cast<int>(ret));
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
#endif
} // namespace AbilityRuntime
} // namespace OHOS
