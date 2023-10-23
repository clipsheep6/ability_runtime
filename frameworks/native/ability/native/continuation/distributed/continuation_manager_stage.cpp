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

#include "continuation_manager_stage.h"

#include "ability_continuation_interface.h"
#include "ability_manager_client.h"
#include "bool_wrapper.h"
#include "continuation_handler.h"
#include "distributed_client.h"
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
    HILOG_DEBUG("Begin.");
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }
    ability_ = ability;

    std::shared_ptr<AbilityRuntime::UIAbility> abilityTmp = nullptr;
    abilityTmp = ability_.lock();
    if (abilityTmp == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    if (abilityTmp->GetAbilityInfo() == nullptr) {
        HILOG_ERROR("AbilityInfo is nullptr.");
        return false;
    }
    abilityInfo_ = abilityTmp->GetAbilityInfo();

    if (continueToken == nullptr) {
        HILOG_ERROR("ContinueToken is nullptr.");
        return false;
    }
    continueToken_ = continueToken;

    continuationHandler_ = continuationHandler;
    HILOG_DEBUG("End.");
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
    HILOG_DEBUG("Begin.");
    HandleContinueAbilityWithStack(deviceId, versionCode);
    HILOG_DEBUG("End.");
}

bool ContinuationManagerStage::HandleContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    HILOG_DEBUG("Begin.");
    if (!CheckAbilityToken()) {
        HILOG_ERROR("CheckAbilityToken failed.");
        return false;
    }

    InitMainHandlerIfNeed();
    wptr<IRemoteObject> continueTokenWeak(continueToken_);
    auto task = [continuationHandlerWeak = continuationHandler_, continueTokenWeak, deviceId, versionCode]() {
        auto continuationHandler = continuationHandlerWeak.lock();
        if (continuationHandler == nullptr) {
            HILOG_ERROR("ContinuationHandler is nullptr.");
            return;
        }

        auto continueToken = continueTokenWeak.promote();
        if (continueToken == nullptr) {
            HILOG_ERROR("continueToken is nullptr.");
            return;
        }
        continuationHandler->HandleStartContinuationWithStack(continueToken, deviceId, versionCode);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("PostTask failed.");
        return false;
    }
    HILOG_DEBUG("End.");
    return true;
}

int32_t ContinuationManagerStage::OnStartAndSaveData(WantParams &wantParams)
{
    HILOG_DEBUG("Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (!ability->OnStartContinuation()) {
        HILOG_ERROR("Ability rejected.");
        return CONTINUE_ABILITY_REJECTED;
    }
    if (!ability->OnSaveData(wantParams)) {
        HILOG_ERROR("SaveData failed.");
        return CONTINUE_SAVE_DATA_FAILED;
    }
    HILOG_DEBUG("End.");
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
    HILOG_DEBUG("Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return ERR_INVALID_VALUE;
    }

    HILOG_DEBUG("OnContinue begin.");
    int32_t status = ability->OnContinue(wantParams);
    HILOG_DEBUG("OnContinue end, status: %{public}d.", status);
    if (status != OnContinueResult::AGREE) {
        if (status == OnContinueResult::MISMATCH) {
            HILOG_ERROR("OnContinue version mismatch.");
            return CONTINUE_ON_CONTINUE_MISMATCH;
        }
        HILOG_ERROR("OnContinue failed.");
        return CONTINUE_ON_CONTINUE_FAILED;
    }

#ifdef SUPPORT_GRAPHICS
    if (IsContinuePageStack(wantParams)) {
        bool ret = GetContentInfo(wantParams);
        if (!ret) {
            HILOG_ERROR("GetContentInfo failed");
            return CONTINUE_GET_CONTENT_FAILED;
        }
    }
#endif
    HILOG_DEBUG("End.");
    return ERR_OK;
}

int32_t ContinuationManagerStage::OnContinue(WantParams &wantParams)
{
    HILOG_DEBUG("Begin.");
    auto ability = ability_.lock();
    auto abilityInfo = abilityInfo_.lock();
    if (ability == nullptr || abilityInfo == nullptr) {
        HILOG_ERROR("Ability or abilityInfo is nullptr.");
        return ERR_INVALID_VALUE;
    }

    bool stageBased = abilityInfo->isStageBasedModel;
    HILOG_DEBUG("Ability isStageBasedModel %{public}d.", stageBased);
    if (!stageBased) {
        return OnStartAndSaveData(wantParams);
    }
    return OnContinueAndGetContent(wantParams);
}

#ifdef SUPPORT_GRAPHICS
bool ContinuationManagerStage::GetContentInfo(WantParams &wantParams)
{
    HILOG_DEBUG("Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    std::string pageStack = ability->GetContentInfo();
    if (pageStack.empty()) {
        HILOG_ERROR("GetContentInfo failed.");
        return false;
    }
    HILOG_DEBUG("Ability pageStack: %{public}s.", pageStack.c_str());
    wantParams.SetParam(PAGE_STACK_PROPERTY_NAME, String::Box(pageStack));

    HILOG_DEBUG("End.");
    return true;
}
#endif

void ContinuationManagerStage::ContinueAbility(bool reversible, const std::string &deviceId)
{
    HILOG_DEBUG("Begin.");
    if (CheckContinuationIllegal()) {
        HILOG_ERROR("Ability not available to continueAbility.");
        return;
    }

    if (progressState_ != ProgressState::INITIAL) {
        HILOG_ERROR("Another request in progressState_: %{public}d.", progressState_);
        return;
    }

    if (continuationState_ != ContinuationState::LOCAL_RUNNING) {
        HILOG_ERROR("Illegal continuation state %{public}d.", continuationState_);
        return;
    }

    if (HandleContinueAbility(reversible, deviceId)) {
        reversible_ = reversible;
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
    }
    HILOG_DEBUG("end");
}

bool ContinuationManagerStage::ReverseContinueAbility()
{
    HILOG_DEBUG("begin");
    if (progressState_ != ProgressState::INITIAL) {
        HILOG_ERROR("Failed progressState_ is %{public}d", progressState_);
        return false;
    }

    if (continuationState_ != ContinuationState::REMOTE_RUNNING) {
        HILOG_ERROR("Failed continuationState_ is %{public}d", continuationState_);
        return false;
    }

    std::shared_ptr<ContinuationHandlerStage> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        HILOG_ERROR("continuationHandler_ is nullptr");
        return false;
    }

    bool requestSuccess = continuationHandler->ReverseContinueAbility();
    if (requestSuccess) {
        ChangeProcessState(ProgressState::WAITING_SCHEDULE);
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_REMOTE_NOTIFY_BACK, ProgressState::WAITING_SCHEDULE);
    }
    HILOG_DEBUG("end");
    return requestSuccess;
}

bool ContinuationManagerStage::StartContinuation()
{
    HILOG_DEBUG("begin");
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleStartContinuation();
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    }
    HILOG_DEBUG("end");
    return result;
}

bool ContinuationManagerStage::SaveData(WantParams &saveData)
{
    HILOG_DEBUG("begin");
    bool result = DoScheduleSaveData(saveData);
    if (!result) {
        ChangeProcessState(ProgressState::INITIAL);
    } else {
        RestoreStateWhenTimeout(TIMEOUT_MS_WAIT_DMS_NOTIFY_CONTINUATION_COMPLETE, ProgressState::IN_PROGRESS);
    }
    HILOG_DEBUG("end");
    return result;
}

bool ContinuationManagerStage::RestoreData(
    const WantParams &restoreData, bool reversible, const std::string &originalDeviceId)
{
    HILOG_DEBUG("Begin.");
    ChangeProcessState(ProgressState::IN_PROGRESS);
    bool result = DoScheduleRestoreData(restoreData);
    if (reversible) {
        continuationState_ = ContinuationState::REPLICA_RUNNING;
    }
    originalDeviceId_ = originalDeviceId;
    ChangeProcessState(ProgressState::INITIAL);
    HILOG_DEBUG("End.");
    return result;
}

void ContinuationManagerStage::NotifyCompleteContinuation(
    const std::string &originDeviceId, int sessionId, bool success, const sptr<IRemoteObject> &reverseScheduler)
{
    HILOG_DEBUG("Begin.");
    AAFwk::AbilityManagerClient::GetInstance()->NotifyCompleteContinuation(originDeviceId, sessionId, success);
    HILOG_DEBUG("End.");
}

void ContinuationManagerStage::CompleteContinuation(int result)
{
    HILOG_DEBUG("Begin.");
    if (CheckContinuationIllegal()) {
        HILOG_ERROR("Ability not available to complete continuation.");
        return;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
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
    HILOG_DEBUG("End.");
}

bool ContinuationManagerStage::RestoreFromRemote(const WantParams &restoreData)
{
    HILOG_DEBUG("Begin.");
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
    HILOG_DEBUG("End.");
    return result;
}

bool ContinuationManagerStage::NotifyRemoteTerminated()
{
    HILOG_DEBUG("Begin.");
    continuationState_ = ContinuationState::LOCAL_RUNNING;
    ChangeProcessState(ProgressState::INITIAL);

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    ability->OnRemoteTerminated();
    HILOG_DEBUG("End.");
    return true;
}

bool ContinuationManagerStage::CheckContinuationIllegal()
{
    HILOG_DEBUG("Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    if (ability->GetState() >= AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED) {
        HILOG_ERROR("Ability state is wrong: %{public}d.", ability->GetState());
        return true;
    }
    HILOG_DEBUG("End.");
    return false;
}

bool ContinuationManagerStage::HandleContinueAbility(bool reversible, const std::string &deviceId)
{
    HILOG_DEBUG("Begin.");

    if (!CheckAbilityToken()) {
        HILOG_ERROR("CheckAbilityToken failed.");
        return false;
    }

    std::shared_ptr<ContinuationHandlerStage> continuationHandler = continuationHandler_.lock();
    if (continuationHandler == nullptr) {
        HILOG_ERROR("ContinuationHandler is nullptr.");
        return false;
    }
    continuationHandler->SetReversible(reversible);

    InitMainHandlerIfNeed();
    wptr<IRemoteObject> continueTokeWeak(continueToken_);
    auto task = [continuationHandlerWeak = continuationHandler_, continueTokeWeak, deviceId]() {
        auto continuationHandler = continuationHandlerWeak.lock();
        if (continuationHandler == nullptr) {
            HILOG_ERROR("ContinuationHandler is nullptr.");
            return;
        }

        auto continueToken = continueTokeWeak.promote();
        if (continueToken == nullptr) {
            HILOG_ERROR("continueToken is nullptr.");
            return;
        }
        continuationHandler->HandleStartContinuation(continueToken, deviceId);
    };
    if (!mainHandler_->PostTask(task)) {
        HILOG_ERROR("PostTask failed.");
        return false;
    }

    HILOG_DEBUG("End.");
    return true;
}

ContinuationManagerStage::ProgressState ContinuationManagerStage::GetProcessState()
{
    return progressState_;
}

void ContinuationManagerStage::ChangeProcessState(const ProgressState &newState)
{
    HILOG_DEBUG("Begin progressState_: %{public}d, newState: %{public}d.", progressState_, newState);
    progressState_ = newState;
}

void ContinuationManagerStage::ChangeProcessStateToInit()
{
    if (mainHandler_ != nullptr) {
        mainHandler_->RemoveTask("Restore_State_When_Timeout");
        HILOG_DEBUG("Restore_State_When_Timeout task removed.");
    }
    ChangeProcessState(ProgressState::INITIAL);
}

void ContinuationManagerStage::RestoreStateWhenTimeout(long timeoutInMs, const ProgressState &preState)
{
    HILOG_DEBUG("Begin.");
    InitMainHandlerIfNeed();

    auto timeoutTask = [continuationManager = shared_from_this(), preState]() {
        HILOG_DEBUG(
            "preState = %{public}d, currentState = %{public}d", preState, continuationManager->GetProcessState());
        if (preState == continuationManager->GetProcessState()) {
            continuationManager->ChangeProcessState(ProgressState::INITIAL);
        }
    };
    mainHandler_->PostTask(timeoutTask, "Restore_State_When_Timeout", timeoutInMs);
    HILOG_DEBUG("End.");
}

void ContinuationManagerStage::InitMainHandlerIfNeed()
{
    HILOG_DEBUG("Begin.");
    if (mainHandler_ == nullptr) {
        HILOG_DEBUG("Try to init main handler.");
        std::lock_guard<std::mutex> lock_l(lock_);
        if ((mainHandler_ == nullptr) && (EventRunner::GetMainEventRunner() != nullptr)) {
            mainHandler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
        }
    }
    HILOG_DEBUG("End.");
}

bool ContinuationManagerStage::CheckAbilityToken()
{
    HILOG_DEBUG("Begin.");
    if (continueToken_ == nullptr) {
        HILOG_ERROR("ContinueToken is nullptr.");
        return false;
    }
    HILOG_DEBUG("End.");
    return true;
}

void ContinuationManagerStage::CheckDmsInterfaceResult(int result, const std::string &interfaceName)
{
    HILOG_DEBUG("interfaceName: %{public}s, result: %{public}d", interfaceName.c_str(), result);
}

bool ContinuationManagerStage::DoScheduleStartContinuation()
{
    HILOG_DEBUG("Begin.");
    if (CheckContinuationIllegal()) {
        HILOG_ERROR("Ability not available to startContinuation.");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }
    if (!ability->OnStartContinuation()) {
        HILOG_ERROR("Failed to StartContinuation.");
        return false;
    }
    HILOG_DEBUG("End.");
    return true;
}

bool ContinuationManagerStage::DoScheduleSaveData(WantParams &saveData)
{
    HILOG_DEBUG("Begin.");
    if (CheckContinuationIllegal()) {
        HILOG_ERROR("Ability not available to save data.");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    WantParams abilitySaveData;
    bool ret = ability->OnSaveData(abilitySaveData);
    for (std::string key : abilitySaveData.KeySet()) {
        saveData.SetParam(key, abilitySaveData.GetParam(key).GetRefPtr());
    }

    if (!ret) {
        HILOG_ERROR("Ability save data failed.");
    }
    HILOG_DEBUG("End.");
    return ret;
}

bool ContinuationManagerStage::DoScheduleRestoreData(const WantParams &restoreData)
{
    HILOG_DEBUG("Begin.");
    if (CheckContinuationIllegal()) {
        HILOG_ERROR("Ability not available to restore data.");
        return false;
    }

    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        HILOG_ERROR("Ability restore data failed.");
    }
    HILOG_DEBUG("End.");
    return ret;
}

bool ContinuationManagerStage::DoRestoreFromRemote(const WantParams &restoreData)
{
    HILOG_DEBUG("Begin.");
    std::shared_ptr<AbilityRuntime::UIAbility> ability = ability_.lock();
    if (ability == nullptr) {
        HILOG_ERROR("Ability is nullptr.");
        return false;
    }

    WantParams abilityRestoreData;
    for (std::string key : restoreData.KeySet()) {
        abilityRestoreData.SetParam(key, restoreData.GetParam(key).GetRefPtr());
    }

    bool ret = ability->OnRestoreData(abilityRestoreData);
    if (!ret) {
        HILOG_ERROR("Ability restore data failed.");
    }
    HILOG_DEBUG("End.");
    return ret;
}
} // namespace AppExecFwk
} // namespace OHOS
