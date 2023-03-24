/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ui_ability_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "errors.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iability_info_callback.h"

namespace OHOS {
namespace AAFwk {
void UIAbilityManager::Init()
{
}

// 所有的ability都是从合一服务调过来的，还需要callerAccessTokenId吗？
int UIAbilityManager::StartAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo)
{
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);

    abilityRequest.callerAccessTokenId = IPCSkeleton::GetCallingTokenID();
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueWaitingAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return 0;
    }

    return StartAbilityLocked(abilityRequest, sessionInfo);
}

// 还需要再abilityRecord中记录callerAbility吗？
int UIAbilityManager::StartAbilityLocked(const AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Start ability locked.");
    std::shared_ptr<AbilityRecord> targetAbilityRecord = nullptr;
    auto iter = sessionItems_.find(sessionInfo);
    if (iter != sessionItems_.end()) {
        targetAbilityRecord = iter->second;
    } else {
        targetAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest, sessionInfo);
    }

    if (targetAbilityRecord == nullptr) {
        HILOG_ERROR("Failed to get ability record.");
        return ERR_INVALID_VALUE;
    }

    // handle specified
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        targetAbilityRecord->SetSpecifiedFlag(abilityRequest.specifiedFlag);
    }

    if (targetAbilityRecord->IsTerminating()) {
        HILOG_ERROR("%{public}s is terminating.", targetAbilityRecord->GetAbilityInfo().name.c_str());
        return ERR_INVALID_VALUE;
    }

    if (targetAbilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        HILOG_DEBUG("pending state is FOREGROUND.");
        targetAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        if (iter == sessionItems_.end()) {
            sessionItems_.emplace(sessionInfo, targetAbilityRecord);
        }
        return ERR_OK;
    } else {
        HILOG_DEBUG("pending state is not FOREGROUND.");
        targetAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }

    UpdateAbilityRecordLaunchReason(abilityRequest, targetAbilityRecord);
    // 跨设备场景后续支持，单独落需求。从合一服务起存不存在跨设备场景？

    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback>(abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyAbilityToken(targetAbilityRecord->GetToken(), abilityRequest.want);
    }

    targetAbilityRecord->ProcessForegroundAbility();
    if (iter == sessionItems_.end()) {
        sessionItems_.emplace(sessionInfo, targetAbilityRecord);
    }
    return ERR_OK;
}

int UIAbilityManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    HILOG_DEBUG("AbilityMS attach abilityThread, name is %{public}s.", abilityRecord->GetAbilityInfo().name.c_str());

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetEventId());

    if (!IsContainsAbility(abilityRecord)) {
        return ERR_INVALID_VALUE;
    }

    abilityRecord->SetScheduler(scheduler);

    // byCall后面单独考虑
    // if (abilityRecord->IsStartedByCall()) {
    //     if (abilityRecord->GetWant().GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
    //         abilityRecord->SetStartToForeground(true);
    //         DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    //     } else {
    //         abilityRecord->SetStartToBackground(true);
    //         MoveToBackgroundTask(abilityRecord);
    //     }
    //     return ERR_OK;
    // }

    // if (abilityRecord->IsNeedToCallRequest()) {
    //     abilityRecord->CallRequest();
    // }

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    return ERR_OK;
}

void UIAbilityManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) const
{
    HILOG_DEBUG("Ability request state %{public}d done.", state);
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        // 判断abilityRecord是否在map中
        auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        if (IsContainsAbility(abilityRecord)) {
            std::string element = abilityRecord->GetWant().GetElement().GetURI();
            HILOG_DEBUG("Ability is %{public}s, start to foreground.", element.c_str());
            abilityRecord->ForegroundAbility();
        }
    }
}

sptr<IRemoteObject> UIAbilityManager::GetTokenBySceneSession(uint64_t persistentId)
{
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    for (auto iter = sessionItems_.begin(); iter != sessionItems_.end(); iter++) {
        auto sessionInfo = iter->first;
        if (sessionInfo == nullptr) {
            continue;
        }
        if (sessionInfo->persistentId == persistentId) {
            if (iter->second != nullptr) {
                return iter->second->GetToken();
            }
        }
    }
    return nullptr;
}

bool UIAbilityManager::IsContainsAbility(std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    for (auto iter = sessionItems_.begin(); iter != sessionItems_.end(); iter++) {
        if (iter->second == abilityRecord) {
            return true;
        }
    }
    return false;
}

void UIAbilityManager::EnqueueWaitingAbilityToFront(const AbilityRequest &abilityRequest)
{
    std::lock_guard<std::recursive_mutex> guard(sessionLock_);
    waitingAbilityQueue_.push(abilityRequest);
    // std::queue<AbilityRequest> abilityQueue;
    // abilityQueue.push(abilityRequest);
    // waitingAbilityQueue_.swap(abilityQueue);
    // while (!abilityQueue.empty()) {
    //     AbilityRequest tempAbilityRequest = abilityQueue.front();
    //     abilityQueue.pop();
    //     waitingAbilityQueue_.push(tempAbilityRequest);
    // }
}

void UIAbilityManager::UpdateAbilityRecordLaunchReason(
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
}
}
