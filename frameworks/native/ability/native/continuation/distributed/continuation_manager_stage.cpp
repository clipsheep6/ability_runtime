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

#include "continuation_manager_stage.h"

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
#include "ui_ability.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE = 25000;
constexpr int TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK = 6000;
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const int32_t CONTINUE_ABILITY_REJECTED = 29360197;
const int32_t CONTINUE_SAVE_DATA_FAILED = 29360198;
const int32_t CONTINUE_ON_CONTINUE_FAILED = 29360199;
const int32_t CONTINUE_ON_CONTINUE_MISMATCH = 29360204;
#ifdef SUPPORT_GRAPHICS
const int32_t CONTINUE_GET_CONTENT_FAILED = 29360200;
#endif
}

ContinuationManagerStage::ContinuationManagerStage() : progressState_(ProgressState::INITIAL) {}

bool ContinuationManagerStage::Init(const std::shared_ptr<AbilityRuntime::UIAbility> &ability,
    const sptr<IRemoteObject> &continueToken, const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ContinuationHandlerStage> &continuationHandler)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }
    ability_ = ability;

    std::shared_ptr<AbilityRuntime::UIAbility> abilityTmp = nullptr;
    abilityTmp = ability_.lock();
    if (abilityTmp == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    if (abilityTmp->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "AbilityInfo is nullptr.");
        return false;
    }
    abilityInfo_ = abilityTmp->GetAbilityInfo();

    if (continueToken == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinueToken is nullptr.");
        return false;
    }
    continueToken_ = continueToken;

    continuationHandler_ = continuationHandler;
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

ContinuationState ContinuationManagerStage::GetContinuationState()
{
    return continuationState_;
}

std::string ContinuationManagerStage::GetOriginalDeviceId()
{
    return originalDeviceId_;
}

void ContinuationManagerStage::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    HandleContinueAbilityWithStack(deviceId, versionCode);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

bool ContinuationManagerStage::HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "CheckAbilityToken failed.");
        return false;
    }

    InitMainHandlerIfNeed();
    wptr<IRemoteObject> continueTokenWeak(continueToken_);
    auto task = [continuationHandlerWeak = continuationHandler_, continueTokenWeak, deviceId, versionCode]() {
        auto continuationHandler = continuationHandlerWeak.lock();
        if (continuationHandler == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler is nullptr.");
            return;
        }

        auto continueToken = continueTokenWeak.promote();
        if (continueToken == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "continueToken is nullptr.");
            return;
        }
        continuationHandler->HandleStartContinuationWithStack(continueToken, deviceId, versionCode);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

int32_t ContinuationManagerStage::OnStartAndSaveData(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
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
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return ERR_OK;
}

bool ContinuationManagerStage::IsContinuePageStack(const WantParams &wantParams)
{
    auto value = wantParams.GetParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME);
    IBoolean *ao = IBoolean::Query(value);
    if (ao != nullptr) {
        return AAFwk::Boolean::Unbox(ao);
    }
    return true;
}

int32_t ContinuationManagerStage::OnContinueAndGetContent(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::CONTINUATION, "OnContinue begin.");
    int32_t status = ability->OnContinue(wantParams);
    TAG_LOGD(AAFwkTag::CONTINUATION, "OnContinue end, status: %{public}d.", status);
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
            TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed");
            return CONTINUE_GET_CONTENT_FAILED;
        }
    }
#endif
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return ERR_OK;
}

int32_t ContinuationManagerStage::OnContinue(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability or abilityInfo is nullptr.");
        return ERR_INVALID_VALUE;
    }

    bool stageBased = abilityInfo->isStageBasedModel;
    TAG_LOGD(AAFwkTag::CONTINUATION, "Ability isStageBasedModel %{public}d.", stageBased);
    if (!stageBased) {
        return OnStartAndSaveData(wantParams);
    }
    return OnContinueAndGetContent(wantParams);
}

#ifdef SUPPORT_GRAPHICS
bool ContinuationManagerStage::GetContentInfo(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    std::string pageStack = ability->GetContentInfo();
    if (pageStack.empty()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "GetContentInfo failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "Ability pageStack: %{public}s.", pageStack.c_str());
    wantParams.SetParam(PAGE_STACK_PROPERTY_NAME, String::Box(pageStack));

    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}
#endif

void ContinuationManagerStage::ContinueAbility(bool reversible, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to continueAbility.");
        return;
    }

    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Another request in progressState_: %{public}d.", progressState_);
        return;
    }

    if (continuationState_ != ContinuationState::LOCAL_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Illegal continuation state %{public}d.", continuationState_);
        return;
    }

    if (HandleContinueAbility(reversible, deviceId)) {
        reversible_ = reversible;
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "end");
}

bool ContinuationManagerStage::ReverseContinueAbility()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    if (progressState_ != ProgressState::INITIAL) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Failed progressState_ is %{public}d", progressState_);
        return false;
    }

    if (continuationState_ != ContinuationState::REMOTE_RUNNING) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Failed continuationState_ is %{public}d", continuationState_);
        return false;
    }

    std::shared_ptr<ContinuationHandlerStage> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "continuationHandler_ is nullptr");
        return false;
    }

    bool requestSuccess = continuationHandler->ReverseContinueAbility();
    if (requestSuccess) {
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK, ProgressState::WAITING_SCHEDULE);
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "end");
    return requestSuccess;
}

bool ContinuationManagerStage::StartContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleStartContinuation();
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "end");
    return result;
}

bool ContinuationManagerStage::SaveData(WantParams &saveData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "begin");
    bool result = DoScheduleSaveData(saveData);
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    } else {
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE, ProgressState::IN_PROGRESS);
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "end");
    return result;
}

bool ContinuationManagerStage::RestoreData(
    const WantParams &restoreData, bool reversible, const std::string &originalDeviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleRestoreData(restoreData);
    if (reversible) {
        continuationState_ = ContinuationState::REPLICA_RUNNING;
    }
    originalDeviceId_ = originalDeviceId;
    ChangeProcessState(ProgressState::INITIAL);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return result;
}

void ContinuationManagerStage::NotifyCompleteContinuation(
    const std::string &originDeviceId, int sessionId, bool success, const sptr<IRemoteObject> &reverseScheduler)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    AAFwk::AbilityManagerClient::GetInstance()->NotifyCompleteContinuation(originDeviceId, sessionId, success);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ContinuationManagerStage::CompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to complete continuation.");
        return;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
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
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

bool ContinuationManagerStage::RestoreFromRemote(const WantParams &restoreData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
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
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return result;
}

bool ContinuationManagerStage::NotifyRemoteTerminated()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    continuationState_ = ContinuationState::LOCAL_RUNNING;
    ChangeProcessState(ProgressState::INITIAL);

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    ability->OnRemoteTerminated();
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

bool ContinuationManagerStage::CheckContinuationIllegal()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    if (ability->GetState() >= AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability state is wrong: %{public}d.", ability->GetState());
        return true;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return false;
}

bool ContinuationManagerStage::HandleContinueAbility(bool reversible, const std::string &deviceId)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");

    if (!CheckAbilityToken()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "CheckAbilityToken failed.");
        return false;
    }

    std::shared_ptr<ContinuationHandlerStage> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler is nullptr.");
        return false;
    }
    continuationHandler->SetReversible(reversible);

    InitMainHandlerIfNeed();
    wptr<IRemoteObject> continueTokeWeak(continueToken_);
    auto task = [continuationHandlerWeak = continuationHandler_, continueTokeWeak, deviceId]() {
        auto continuationHandler = continuationHandlerWeak.lock();
        if (continuationHandler == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "ContinuationHandler is nullptr.");
            return;
        }

        auto continueToken = continueTokeWeak.promote();
        if (continueToken == nullptr) {
            TAG_LOGE(AAFwkTag::CONTINUATION, "continueToken is nullptr.");
            return;
        }
        continuationHandler->HandleStartContinuation(continueToken, deviceId);
    };
    if (!mainHandler_->PostTask(task)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "PostTask failed.");
        return false;
    }

    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

ContinuationManagerStage::ProgressState ContinuationManagerStage::GetProcessState()
{
    return progressState_;
}

void ContinuationManagerStage::ChangeProcessState(const ProgressState &newState)
{
    TAG_LOGD(AAFwkTag::CONTINUATION,
        "Begin progressState_: %{public}d, newState: %{public}d.", progressState_, newState);
    progressState_ = newState;
}

void ContinuationManagerStage::ChangeProcessStateToInit()
{
    if (mainHandler_ != nullptr) {
        mainHandler_->RemoveTask("Restore_State_When_Timeout");
        TAG_LOGD(AAFwkTag::CONTINUATION, "Restore_State_When_Timeout task removed.");
    }
    ChangeProcessState(ProgressState::INITIAL);
}

void ContinuationManagerStage::RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    InitMainHandlerIfNeed();

    auto timeoutTask = [continuationManager = shared_from_this(), preState]() {
        TAG_LOGD(AAFwkTag::CONTINUATION,
            "preState = %{public}d, currentState = %{public}d", preState, continuationManager->GetProcessState());
        if (preState == continuationManager->GetProcessState()) {
            continuationManager->ChangeProcessState(ProgressState::INITIAL);
        }
    };
    mainHandler_->PostTask(timeoutTask, "Restore_State_When_Timeout", timeoutInMs);
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

void ContinuationManagerStage::InitMainHandlerIfNeed()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (mainHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::CONTINUATION, "Try to init main handler.");
        std::lock_guard<std::mutex> lock_l(lock_);
        if ((mainHandler_ == nullptr) && (EventRunner::GetMainEventRunner() != nullptr)) {
            mainHandler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        }
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
}

bool ContinuationManagerStage::CheckAbilityToken()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (continueToken_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "ContinueToken is nullptr.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

void ContinuationManagerStage::CheckDmsInterfaceResult(int result, const std::string &interfaceName)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "interfaceName: %{public}s, result: %{public}d", interfaceName.c_str(), result);
}

bool ContinuationManagerStage::DoScheduleStartContinuation()
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to startContinuation.");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }
    if (!ability->OnStartContinuation()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Failed to StartContinuation.");
        return false;
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return true;
}

bool ContinuationManagerStage::DoScheduleSaveData(WantParams &saveData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to save data.");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    WantParams abilitySaveData;
    bool ret = ability->OnSaveData(abilitySaveData);
    for (std::string key : abilitySaveData.KeySet()) {
        saveData.SetParam(key, abilitySaveData.GetParam(key).GetRefPtr());
    }

    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability save data failed.");
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return ret;
}

bool ContinuationManagerStage::DoScheduleRestoreData(const WantParams &restoreData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    if (CheckContinuationIllegal()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability not available to restore data.");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability restore data failed.");
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return ret;
}

bool ContinuationManagerStage::DoRestoreFromRemote(const WantParams &restoreData)
{
    TAG_LOGD(AAFwkTag::CONTINUATION, "Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability is nullptr.");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Ability restore data failed.");
    }
    TAG_LOGD(AAFwkTag::CONTINUATION, "End.");
    return ret;
}
} // namespace AppExecFwk
} // namespace OHOS
