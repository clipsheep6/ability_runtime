/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "continuation_manager.h"

#include "ability.h"
#include "ability_continuation_interface.h"
#include "ability_manager_client.h"
#include "bool_wrapper.h"
#include "continuation_handler.h"
#include "distributed_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "operation_builder.h"
#include "string_ex.h"
#include "string_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
const int ContinuationManager::TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE = 25000;
const int ContinuationManager::TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK = 6000;
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const int32_t CONTINUE_ABILITY_REJECTED = 29360197;
const int32_t CONTINUE_SAVE_DATA_FAILED = 29360198;
const int32_t CONTINUE_ON_CONTINUE_FAILED = 29360199;
const int32_t CONTINUE_ON_CONTINUE_MISMATCH = 29360204;
#ifdef SUPPORT_GRAPHICS
const int32_t CONTINUE_GET_CONTENT_FAILED = 29360200;
#endif
ContinuationManager::ContinuationManager()
{
    progressState_ = ProgressState::INITIAL;
}

bool ContinuationManager::Init(const std::shared_ptr<Ability> &ability, const sptr<IRemoteObject> &continueToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ContinuationHandler> &continuationHandler)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::Init failed. ability is nullptr");
        return false;
    }
    ability_ = ability;

    std::shared_ptr<Ability> abilityTmp = nullptr;
    abilityTmp = ability_.lock();
    if (abilityTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::Init failed. get ability is nullptr");
        return false;
    }

    if (abilityTmp->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::Init failed. abilityInfo is nullptr");
        return false;
    }
    abilityInfo_ = abilityTmp->GetAbilityInfo();

    if (continueToken == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::Init failed. continueToken is nullptr");
        return false;
    }
    continueToken_ = continueToken;

    continuationHandler_ = continuationHandler;
    TAG_LOGD(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

ContinuationState ContinuationManager::GetContinuationState()
{
    return continuationState_;
}

std::string ContinuationManager::GetOriginalDeviceId()
{
    return originalDeviceId_;
}

void ContinuationManager::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);

    HandleContinueAbilityWithStack(deviceId, versionCode);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

bool ContinuationManager::HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);

    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "HandleContinueAbilityWithStack checkAbilityToken failed");
        return false;
    }

    sptr<IRemoteObject> continueToken = continueToken_;
    std::shared_ptr<ContinuationHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "HandleContinueAbilityWithStack continuationHandler is nullptr");
        return false;
    }

    InitMainHandlerIfNeed();
    auto task = [continuationHandler, continueToken, deviceId, versionCode]() {
        continuationHandler->HandleStartContinuationWithStack(continueToken, deviceId, versionCode);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "HandleContinueAbilityWithStack postTask failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

int32_t ContinuationManager::OnStartAndSaveData(WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!ability->OnStartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability rejected.");
        return CONTINUE_ABILITY_REJECTED;
    }
    if (!ability->OnSaveData(wantParams)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "SaveData failed.");
        return CONTINUE_SAVE_DATA_FAILED;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return ERR_OK;
}

bool ContinuationManager::IsContinuePageStack(const WantParams &wantParams)
{
    auto value = wantParams.GetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME);
    IBoolean *ao = IBoolean::Query(value);
    if (ao != nullptr) {
        return AAFwk::Boolean::Unbox(ao);
    }
    return true;
}

int32_t ContinuationManager::OnContinueAndGetContent(WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability is nullptr");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::CONTINUATION, "OnContinue begin");
    int32_t status = ability->OnContinue(wantParams);
    TAG_LOGI(AAFwkTag::CONTINUATION, "OnContinue end, status: %{public}d", status);
    if (status != OnContinueResult::AGREE) {
        if (status == OnContinueResult::MISMATCH) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "OnContinue version mismatch.");
            return CONTINUE_ON_CONTINUE_MISMATCH;
        }
        TAG_LOGE(AAFwkTag::CONTINUATION, "OnContinue failed.");
        return CONTINUE_ON_CONTINUE_FAILED;
    }

#ifdef SUPPORT_GRAPHICS
    if (IsContinuePageStack(wantParams)) {
        bool ret = GetContentInfo(wantParams);
        if (!ret) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed.");
            return CONTINUE_GET_CONTENT_FAILED;
        }
    }
#endif
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return ERR_OK;
}

int32_t ContinuationManager::OnContinue(WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability or abilityInfo is nullptr");
        return ERR_INVALID_VALUE;
    }

    bool stageBased = abilityInfo->isStageBasedModel;
    TAG_LOGI(AAFwkTag::CONTINUATION, "ability isStageBasedModel %{public}d", stageBased);
    if (!stageBased) {
        return OnStartAndSaveData(wantParams);
    } else {
        return OnContinueAndGetContent(wantParams);
    }
}

#ifdef SUPPORT_GRAPHICS
bool ContinuationManager::GetContentInfo(WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ability is nullptr");
        return false;
    }

    std::string pageStack = ability->GetContentInfo();
    if (pageStack.empty()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed.");
        return false;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "ability pageStack: %{public}s", pageStack.c_str());
    wantParams.SetParam(PAGE_STACK_PROPERTY_NAME, String::Box(pageStack));

    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}
#endif

void ContinuationManager::ContinueAbility(bool reversible, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::ContinueAbility failed. Ability not available to continueAbility.");
        return;
    }

    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::ContinueAbility failed. Another request in progress. progressState_: %{public}d",
            progressState_);
        return;
    }

    if (continuationState_ != ContinuationState::LOCAL_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::ContinueAbility failed. Illegal continuation state. Current state is %{public}d",
            continuationState_);
        return;
    }

    if (HandleContinueAbility(reversible, deviceId)) {
        reversible_ = reversible;
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

bool ContinuationManager::ReverseContinueAbility()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::ReverseContinueAbility failed. progressState_ is %{public}d.", progressState_);
        return false;
    }

    if (continuationState_ != ContinuationState::REMOTE_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::ReverseContinueAbility failed. continuationState_ is %{public}d.",
            continuationState_);
        return false;
    }

    std::shared_ptr<ContinuationHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::ReverseContinueAbility failed. continuationHandler_ is nullptr.");
        return false;
    }

    bool requestSuccess = continuationHandler->ReverseContinueAbility();
    if (requestSuccess) {
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK, ProgressState::WAITING_SCHEDULE);
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return requestSuccess;
}

bool ContinuationManager::StartContinuation()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleStartContinuation();
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return result;
}

bool ContinuationManager::SaveData(WantParams &saveData)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    bool result = DoScheduleSaveData(saveData);
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    } else {
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE, ProgressState::IN_PROGRESS);
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return result;
}

bool ContinuationManager::RestoreData(
    const WantParams &restoreData, bool reversible, const std::string &originalDeviceId)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleRestoreData(restoreData);
    if (reversible) {
        continuationState_ = ContinuationState::REPLICA_RUNNING;
    }
    originalDeviceId_ = originalDeviceId;
    ChangeProcessState(ProgressState::INITIAL);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return result;
}

void ContinuationManager::NotifyCompleteContinuation(
    const std::string &originDeviceId, int sessionId, bool success, const sptr<IRemoteObject> &reverseScheduler)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    AAFwk::AbilityManagerClient::GetInstance()->NotifyCompleteContinuation(
        originDeviceId, sessionId, success);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

void ContinuationManager::CompleteContinuation(int result)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::CompleteContinuation failed. Ability not available to complete continuation.");
        return;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::CheckContinuationIllegal failed. ability is nullptr");
        return;
    }

    if (result == 0 && reversible_) {
        continuationState_ = ContinuationState::REMOTE_RUNNING;
    }
    ChangeProcessState(ProgressState::INITIAL);

    ability->OnCompleteContinuation(result);

    if (!reversible_) {
        ability->TerminateAbility();
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

bool ContinuationManager::RestoreFromRemote(const WantParams &restoreData)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoRestoreFromRemote(restoreData);
    /*
     * No matter what the result is, we should reset the status. Because even it fail, we can do
     * nothing but let the user send another reverse continuation request again.
     */
    ChangeProcessState(ProgressState::INITIAL);
    if (result) {
        continuationState_ = ContinuationState::LOCAL_RUNNING;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return result;
}

bool ContinuationManager::NotifyRemoteTerminated()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    continuationState_ = ContinuationState::LOCAL_RUNNING;
    ChangeProcessState(ProgressState::INITIAL);

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::NotifyRemoteTerminated failed. ability is nullptr");
        return false;
    }

    ability->OnRemoteTerminated();
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

bool ContinuationManager::CheckContinuationIllegal()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::CheckContinuationIllegal failed. ability is nullptr");
        return false;
    }

    if (ability->GetState() >= AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::CheckContinuationIllegal failed. ability state is wrong: %{public}d",
            ability->GetState());
        return true;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return false;
}

bool ContinuationManager::HandleContinueAbility(bool reversible, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);

    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::HandleContinueAbility failed. CheckAbilityToken failed");
        return false;
    }

    sptr<IRemoteObject> continueToken = continueToken_;
    std::shared_ptr<ContinuationHandler> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::HandleContinueAbility failed. continuationHandler is nullptr");
        return false;
    }
    continuationHandler->SetReversible(reversible);

    InitMainHandlerIfNeed();
    auto task = [continuationHandler, continueToken, deviceId]() {
        continuationHandler->HandleStartContinuation(continueToken, deviceId);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::HandleContinueAbility failed.PostTask failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

ContinuationManager::ProgressState ContinuationManager::GetProcessState()
{
    return progressState_;
}

void ContinuationManager::ChangeProcessState(const ProgressState &newState)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin. progressState_: %{public}d, newState: %{public}d",
        __func__,
        progressState_,
        newState);

    progressState_ = newState;
}


void ContinuationManager::ChangeProcessStateToInit()
{
    if (mainHandler_ != nullptr) {
        mainHandler_->RemoveTask("Restore_State_When_Timeout");
        TAG_LOGI(AAFwkTag::CONTINUATION, "Restore_State_When_Timeout task removed");
    }
    ChangeProcessState(ProgressState::INITIAL);
}

void ContinuationManager::RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    InitMainHandlerIfNeed();

    auto timeoutTask = [continuationManager = shared_from_this(), preState]() {
        TAG_LOGI(AAFwkTag::CONTINUATION,
            "ContinuationManager::RestoreStateWhenTimeout called. preState = %{public}d, currentState = %{public}d.",
            preState,
            continuationManager->GetProcessState());
        if (preState == continuationManager->GetProcessState()) {
            continuationManager->ChangeProcessState(ProgressState::INITIAL);
        }
    };
    mainHandler_->PostTask(timeoutTask, "Restore_State_When_Timeout", timeoutInMs);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

void ContinuationManager::InitMainHandlerIfNeed()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (mainHandler_ == nullptr) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "Try to init main handler.");
        std::lock_guard<std::mutex> lock_l(lock_);
        if ((mainHandler_ == nullptr) && (EventRunner::GetMainEventRunner() != nullptr)) {
            mainHandler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        }
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
}

bool ContinuationManager::CheckAbilityToken()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called", __func__);
    if (continueToken_ == nullptr) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called failed", __func__);
        return false;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called success", __func__);
    return true;
}

void ContinuationManager::CheckDmsInterfaceResult(int result, const std::string &interfaceName)
{
    TAG_LOGI(AAFwkTag::CONTINUATION,
        "ContinuationManager::CheckDmsInterfaceResult called. interfaceName: %{public}s, result: %{public}d.",
        interfaceName.c_str(),
        result);
}

bool ContinuationManager::DoScheduleStartContinuation()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::DoScheduleStartContinuation called. Ability not available to startContinuation.");
        return false;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::DoScheduleStartContinuation failed. ability is nullptr");
        return false;
    }
    if (!ability->OnStartContinuation()) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called failed to StartContinuation", __func__);
        return false;
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return true;
}

bool ContinuationManager::DoScheduleSaveData(WantParams &saveData)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::DoScheduleSaveData failed. Ability not available to save data.");
        return false;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::DoScheduleSaveData failed. ability is nullptr");
        return false;
    }

    WantParams abilitySaveData;
    bool ret = ability->OnSaveData(abilitySaveData);
    for (std::string key : abilitySaveData.KeySet()) {
        saveData.SetParam(key, abilitySaveData.GetParam(key).GetRefPtr());
    }

    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::DoScheduleSaveData failed. Ability save data failed.");
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return ret;
}

bool ContinuationManager::DoScheduleRestoreData(const WantParams &restoreData)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::DoScheduleRestoreData failed. Ability not available to restore data.");
        return false;
    }

    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::DoScheduleRestoreData failed. ability is nullptr");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::DoScheduleRestoreData failed. Ability restore data failed.");
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return ret;
}

bool ContinuationManager::DoRestoreFromRemote(const WantParams &restoreData)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::shared_ptr<Ability> ability = nullptr;
    ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationManager::DoRestoreFromRemote failed. ability is nullptr");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "ContinuationManager::DoRestoreFromRemote failed. Ability restore data failed.");
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS
