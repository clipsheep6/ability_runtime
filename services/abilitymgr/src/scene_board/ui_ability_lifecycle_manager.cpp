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

#include "scene_board/ui_ability_lifecycle_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iability_info_callback.h"

namespace OHOS {
namespace AAFwk {
int UIAbilityLifecycleManager::StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    HILOG_DEBUG("Call.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("sessionInfo is invalid.");
        return ERR_INVALID_VALUE;
    }
    auto descriptor = Str16ToStr8(sessionInfo->sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        HILOG_ERROR("token's Descriptor: %{public}s", descriptor.c_str());
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AbilityRecord> uiAbilityRecord = nullptr;
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end()) {
        uiAbilityRecord = iter->second;
    } else {
        uiAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest, sessionInfo);
    }

    if (uiAbilityRecord == nullptr) {
        HILOG_ERROR("Failed to get ability record.");
        return ERR_INVALID_VALUE;
    }

    if (uiAbilityRecord->IsTerminating()) {
        HILOG_ERROR("%{public}s is terminating.", uiAbilityRecord->GetAbilityInfo().name.c_str());
        return ERR_INVALID_VALUE;
    }

    if (uiAbilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        HILOG_DEBUG("pending state is FOREGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        if (iter == sessionAbilityMap_.end()) {
            sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
        }
        return ERR_OK;
    } else {
        HILOG_DEBUG("pending state is not FOREGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }

    UpdateAbilityRecordLaunchReason(abilityRequest, uiAbilityRecord);
    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback>(abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest.want);
    }

    uiAbilityRecord->ProcessForegroundAbility();
    if (iter == sessionAbilityMap_.end()) {
        sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    if (!IsContainsAbility(token)) {
        return ERR_INVALID_VALUE;
    }
    auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    HILOG_DEBUG("AbilityMS attach abilityThread, name is %{public}s.", abilityRecord->GetAbilityInfo().name.c_str());

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());

    abilityRecord->SetScheduler(scheduler);
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) const
{
    HILOG_DEBUG("Ability request state %{public}d done.", state);
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetWant().GetElement().GetURI();
        HILOG_DEBUG("Ability is %{public}s, start to foreground.", element.c_str());
        abilityRecord->ForegroundAbility();
    }
}

int UIAbilityLifecycleManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state,
    const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    HILOG_INFO("AbilityTransactionDone, state: %{public}s.", abilityState.c_str());

    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int UIAbilityLifecycleManager::DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    switch (state) {
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord);
        }
        case AbilityState::BACKGROUND: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::FOREGROUND: {
            return DispatchForeground(abilityRecord, true);
        }
        case AbilityState::FOREGROUND_FAILED:
        case AbilityState::FOREGROUND_INVALID_MODE:
        case AbilityState::FOREGROUND_WINDOW_FREEZED: {
            return DispatchForeground(abilityRecord, false, static_cast<AbilityState>(state));
        }
        default: {
            HILOG_WARN("Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int UIAbilityLifecycleManager::DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
    AbilityState state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("DispatchForeground Ability transition life state error. expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING,
            abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    handler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    auto self(weak_from_this());
    if (success) {
        HILOG_INFO("foreground succeeded.");
        auto task = [self, abilityRecord]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                HILOG_WARN("mgr is invalid.");
                return;
            }
            selfObj->CompleteForegroundSuccess(abilityRecord);
        };
        handler->PostTask(task);
    } else {
        auto task = [self, abilityRecord, state]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                HILOG_WARN("Mission list mgr is invalid.");
                return;
            }
            if (state == AbilityState::FOREGROUND_WINDOW_FREEZED) {
                HILOG_INFO("Window was freezed.");
                if (abilityRecord != nullptr) {
                    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
                    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
                }
                return;
            }
            selfObj->CompleteForegroundFailed(abilityRecord, state);
        };
        handler->PostTask(task);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    return 0;
}

int UIAbilityLifecycleManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    return 0;
}

void UIAbilityLifecycleManager::CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);

    CHECK_POINTER(abilityRecord);
    // ability do not save window mode
    abilityRecord->RemoveWindowMode();
    std::string element = abilityRecord->GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability: %{public}s", element.c_str());
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);

    if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        abilityRecord->SetMinimizeReason(true);
        MoveToBackgroundTask(abilityRecord);
    } else if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        HILOG_DEBUG("not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
}

void UIAbilityLifecycleManager::CompleteForegroundFailed(const std::shared_ptr<AbilityRecord> &abilityRecord,
    AbilityState state)
{
    HILOG_DEBUG("CompleteForegroundFailed come, state: %{public}d.", static_cast<int32_t>(state));
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        HILOG_ERROR("CompleteForegroundFailed, ability is nullptr.");
        return;
    }

    HandleForegroundTimeout(abilityRecord, state);
}

void UIAbilityLifecycleManager::HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &ability,
    AbilityState state)
{
    if (ability == nullptr) {
        HILOG_ERROR("ability record is nullptr.");
        return;
    }

    if (!ability->IsAbilityState(AbilityState::FOREGROUNDING)) {
        HILOG_ERROR("this ability is not foregrounding state.");
        return;
    }

    // notify SCB the ability to fail to foreground

    // root launcher load timeout, notify appMs force terminate the ability and restart immediately.
    if (ability->IsLauncherAbility() && ability->IsLauncherRoot()) {
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
        HILOG_INFO("Launcher root load timeout, restart.");
        // notify SCB to delay to start launcher
        return;
    }

    // other
    HandleTimeoutAndResumeAbility(ability, state);
}

void UIAbilityLifecycleManager::HandleTimeoutAndResumeAbility(const std::shared_ptr<AbilityRecord> &ability,
    AbilityState state)
{
    HILOG_DEBUG("Call");
    if (ability == nullptr) {
        HILOG_ERROR("LoadAndForeGroundCommon: ability is nullptr.");
        return;
    }
    EraseAbilityRecord(ability);

    // load and foreground timeout, notify appMs force terminate the ability.
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());

    // caller not exist or caller is service or timeout ability is launcher, back to launcher
    auto callerAbility = ability->GetCallerRecord();
    if ((callerAbility == nullptr) ||
        (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::SERVICE) ||
        (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::EXTENSION)) {
        HILOG_DEBUG("ability timeout, back to launcher.");
        // notify SCB to delay to start launcher
        return;
    }
}

void UIAbilityLifecycleManager::MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord) {}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    const
{
    if (token == nullptr) {
        HILOG_ERROR("nullptr.");
        return nullptr;
    }

    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return iter->second;
        }
    }
    return nullptr;
}

bool UIAbilityLifecycleManager::IsContainsAbility(const sptr<IRemoteObject> &token) const
{
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return true;
        }
    }
    return false;
}

void UIAbilityLifecycleManager::EraseAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject()) {
            sessionAbilityMap_.erase(iter);
            break;
        }
    }
}

void UIAbilityLifecycleManager::UpdateAbilityRecordLaunchReason(
    const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    if (abilityRecord == nullptr) {
        HILOG_ERROR("input record is nullptr.");
        return;
    }

    if (abilityRequest.IsContinuation()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CONTINUATION);
        return;
    }

    if (abilityRequest.IsAppRecovery() || abilityRecord->GetRecoveryInfo()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_APP_RECOVERY);
        return;
    }

    abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    return;
}
}  // namespace AAFwk
}  // namespace OHOS