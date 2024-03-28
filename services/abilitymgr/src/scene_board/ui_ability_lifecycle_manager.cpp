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

#include "scene_board/ui_ability_lifecycle_manager.h"

#include "ability_manager_service.h"
#include "ability_running_info.h"
#include "ability_util.h"
#include "appfreeze_manager.h"
#include "app_exit_reason_data_manager.h"
#include "app_utils.h"
#include "errors.h"
#include "exit_reason.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iability_info_callback.h"
#include "mission_info.h"
#include "process_options.h"
#include "session_info.h"
#include "session_manager_lite.h"

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AAFwk {
namespace {
constexpr const char* SEPARATOR = ":";
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
const std::string DLP_INDEX = "ohos.dlp.params.index";
constexpr int32_t PREPARE_TERMINATE_TIMEOUT_MULTIPLE = 10;
const std::string PARAM_MISSION_AFFINITY_KEY = "ohos.anco.param.missionAffinity";
const std::string DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";
const std::string DMS_MISSION_ID = "dmsMissionId";
const int DEFAULT_DMS_MISSION_ID = -1;
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
#ifdef SUPPORT_ASAN
const int KILL_TIMEOUT_MULTIPLE = 45;
#else
const int KILL_TIMEOUT_MULTIPLE = 3;
#endif
constexpr int32_t DEFAULT_USER_ID = 0;
const std::unordered_map<uint32_t, FreezeUtil::TimeoutState> stateMap = {
    { AbilityManagerService::LOAD_TIMEOUT_MSG, FreezeUtil::TimeoutState::LOAD },
    { AbilityManagerService::FOREGROUND_TIMEOUT_MSG, FreezeUtil::TimeoutState::FOREGROUND },
    { AbilityManagerService::BACKGROUND_TIMEOUT_MSG, FreezeUtil::TimeoutState::BACKGROUND }
};

auto g_deleteLifecycleEventTask = [](const sptr<Token> &token, FreezeUtil::TimeoutState state) {
    CHECK_POINTER_LOG(token, "token is nullptr.");
    FreezeUtil::LifecycleFlow flow = { token->AsObject(), state };
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
};
}

int UIAbilityLifecycleManager::StartUIAbility(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (!CheckSessionInfo(sessionInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is invalid.");
        return ERR_INVALID_VALUE;
    }
    abilityRequest.sessionInfo = sessionInfo;

    TAG_LOGI(AAFwkTag::ABILITYMGR, "session id: %{public}d. bundle: %{public}s, ability: %{public}s",
        sessionInfo->persistentId, abilityRequest.abilityInfo.bundleName.c_str(),
        abilityRequest.abilityInfo.name.c_str());
    std::shared_ptr<AbilityRecord> uiAbilityRecord = nullptr;
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "isNewWant: %{public}d.", sessionInfo->isNewWant);
        uiAbilityRecord = iter->second;
        uiAbilityRecord->SetIsNewWant(sessionInfo->isNewWant);
        if (sessionInfo->isNewWant) {
            uiAbilityRecord->SetWant(abilityRequest.want);
        }
    } else {
        uiAbilityRecord = CreateAbilityRecord(abilityRequest, sessionInfo);
        UpdateProcessName(abilityRequest, uiAbilityRecord);
    }
    CHECK_POINTER_AND_RETURN(uiAbilityRecord, ERR_INVALID_VALUE);

    if (uiAbilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is FOREGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        return ERR_OK;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is not FOREGROUND.");
        uiAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        if (uiAbilityRecord->IsLoading()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ability: %{public}s is loading.", abilityRequest.abilityInfo.name.c_str());
            return ERR_OK;
        }
    }
    if (iter == sessionAbilityMap_.end()) {
        sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
    }

    UpdateAbilityRecordLaunchReason(abilityRequest, uiAbilityRecord);
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);
    AddCallerRecord(abilityRequest, sessionInfo, uiAbilityRecord);
    uiAbilityRecord->ProcessForegroundAbility(sessionInfo->callingTokenId);
    CheckSpecified(abilityRequest, uiAbilityRecord);
    SendKeyEvent(abilityRequest);
    return ERR_OK;
}

bool UIAbilityLifecycleManager::CheckSessionInfo(sptr<SessionInfo> sessionInfo) const
{
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is invalid.");
        return false;
    }
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    auto descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token's Descriptor: %{public}s", descriptor.c_str());
        return false;
    }
    return true;
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::CreateAbilityRecord(AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo) const
{
    if (sessionInfo->startSetting != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "startSetting is valid.");
        abilityRequest.startSetting = sessionInfo->startSetting;
    }
    auto uiAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (uiAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uiAbilityRecord is invalid.");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "user id: %{public}d.", sessionInfo->userId);
    uiAbilityRecord->SetOwnerMissionUserId(sessionInfo->userId);
    SetRevicerInfo(abilityRequest, uiAbilityRecord);
    SetLastExitReason(uiAbilityRecord);
    return uiAbilityRecord;
}

void UIAbilityLifecycleManager::AddCallerRecord(AbilityRequest &abilityRequest, sptr<SessionInfo> sessionInfo,
    std::shared_ptr<AbilityRecord> uiAbilityRecord) const
{
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is invalid.");
        return;
    }
    CHECK_POINTER(uiAbilityRecord);
    std::string srcAbilityId = "";
    if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_FOR_RESULT, false)) {
        std::string srcDeviceId = abilityRequest.want.GetStringParam(DMS_SRC_NETWORK_ID);
        int missionId = abilityRequest.want.GetIntParam(DMS_MISSION_ID, DEFAULT_DMS_MISSION_ID);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get srcNetWorkId = %{public}s, missionId = %{public}d", srcDeviceId.c_str(),
            missionId);
        Want *newWant = const_cast<Want*>(&abilityRequest.want);
        newWant->RemoveParam(DMS_SRC_NETWORK_ID);
        newWant->RemoveParam(DMS_MISSION_ID);
        newWant->RemoveParam(Want::PARAM_RESV_FOR_RESULT);
        srcAbilityId = srcDeviceId + "_" + std::to_string(missionId);
    }
    uiAbilityRecord->AddCallerRecord(sessionInfo->callerToken,
        sessionInfo->requestCode, srcAbilityId, sessionInfo->callingTokenId);
}

void UIAbilityLifecycleManager::CheckSpecified(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> uiAbilityRecord)
{
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED && !specifiedInfoQueue_.empty()) {
        SpecifiedInfo specifiedInfo = specifiedInfoQueue_.front();
        specifiedInfoQueue_.pop();
        uiAbilityRecord->SetSpecifiedFlag(specifiedInfo.flag);
        specifiedAbilityMap_.emplace(specifiedInfo, uiAbilityRecord);
    }
}

void UIAbilityLifecycleManager::SendKeyEvent(AbilityRequest &abilityRequest) const
{
    if (abilityRequest.abilityInfo.visible == false) {
        EventInfo eventInfo;
        eventInfo.abilityName = abilityRequest.abilityInfo.name;
        eventInfo.bundleName = abilityRequest.abilityInfo.bundleName;
        eventInfo.moduleName = abilityRequest.abilityInfo.moduleName;
        EventReport::SendKeyEvent(EventName::START_PRIVATE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    }
}

int UIAbilityLifecycleManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (!IsContainsAbilityInner(token)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not in running list");
        return ERR_INVALID_VALUE;
    }
    auto&& abilityRecord = Token::GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Lifecycle: name is %{public}s.", abilityRecord->GetAbilityInfo().name.c_str());

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    abilityRecord->SetLoading(false);
    FreezeUtil::LifecycleFlow flow = {token, FreezeUtil::TimeoutState::LOAD};
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);

    abilityRecord->SetScheduler(scheduler);
    if (abilityRecord->IsStartedByCall()) {
        if (abilityRecord->GetWant().GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            abilityRecord->SetStartToForeground(true);
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
        } else {
            abilityRecord->SetStartToBackground(true);
            MoveToBackground(abilityRecord);
        }
        return ERR_OK;
    }

    if (abilityRecord->IsNeedToCallRequest()) {
        abilityRecord->CallRequest();
    }

    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, int32_t state) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability request state %{public}d done.", state);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetElementName().GetURI();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability is %{public}s, start to foreground.", element.c_str());
        abilityRecord->ForegroundAbility();
    }
}

int UIAbilityLifecycleManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state,
    const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityTransactionDone, state: %{public}s.", abilityState.c_str());

    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int UIAbilityLifecycleManager::NotifySCBToStartUIAbility(const AbilityRequest &abilityRequest, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityInfo = abilityRequest.abilityInfo;
    bool isUIAbility = (abilityInfo.type == AppExecFwk::AbilityType::PAGE && abilityInfo.isStageBasedModel);
    if (abilityInfo.isolationProcess && AppUtils::GetInstance().IsStartSpecifiedProcess() && isUIAbility) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "StartSpecifiedProcess");
        EnqueueAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedProcess(abilityRequest.want, abilityInfo);
        return ERR_OK;
    }
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return ERR_OK;
    }
    auto sessionInfo = CreateSessionInfo(abilityRequest);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest, sessionInfo->reuse, userId);
    sessionInfo->userId = userId;
    sessionInfo->processOptions = abilityRequest.processOptions;
    TAG_LOGI(
        AAFwkTag::ABILITYMGR, "Reused sessionId: %{public}d, userId: %{public}d.", sessionInfo->persistentId, userId);
    return NotifySCBPendingActivation(sessionInfo, abilityRequest);
}

int UIAbilityLifecycleManager::DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    switch (state) {
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord);
        }
        case AbilityState::BACKGROUND:
        case AbilityState::BACKGROUND_FAILED: {
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
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int UIAbilityLifecycleManager::DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
    AbilityState state)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(taskHandler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "DispatchForeground Ability transition life state error. expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING, abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundLifecycle: end.");
    handler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    g_deleteLifecycleEventTask(abilityRecord->GetToken(), FreezeUtil::TimeoutState::FOREGROUND);
    auto self(weak_from_this());
    if (success) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "foreground succeeded.");
        auto task = [self, abilityRecord]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "mgr is invalid.");
                return;
            }
            selfObj->CompleteForegroundSuccess(abilityRecord);
        };
        taskHandler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);
    } else {
        auto task = [self, abilityRecord, state]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "Mission list mgr is invalid.");
                return;
            }
            if (state == AbilityState::FOREGROUND_WINDOW_FREEZED) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "Window was freezed.");
                if (abilityRecord != nullptr) {
                    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
                    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
                }
                return;
            }
            selfObj->HandleForegroundFailed(abilityRecord, state);
        };
        taskHandler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability transition life state error. actual %{public}d",
            abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "end.");
    // remove background timeout task.
    handler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    g_deleteLifecycleEventTask(abilityRecord->GetToken(), FreezeUtil::TimeoutState::BACKGROUND);
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    handler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

    return ERR_OK;
}

int UIAbilityLifecycleManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DispatchTerminate error, ability state is %{public}d",
            abilityRecord->GetAbilityState());
        return INNER_ERR;
    }

    // remove terminate timeout task.
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    handler->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteTerminate(abilityRecord); };
    handler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);

    return ERR_OK;
}

void UIAbilityLifecycleManager::CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    CHECK_POINTER(abilityRecord);
    // ability do not save window mode
    abilityRecord->RemoveWindowMode();
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability: %{public}s", element.c_str());
    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
    abilityRecord->UpdateAbilityVisibilityState();

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToForeground() && abilityRecord->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call request after completing foreground state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToForeground(false);
    }

    if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        abilityRecord->SetMinimizeReason(true);
        MoveToBackground(abilityRecord);
    } else if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
    if (handler_ != nullptr && abilityRecord->GetSessionInfo() != nullptr) {
        handler_->OnSessionMovedToFront(abilityRecord->GetSessionInfo()->persistentId);
    }
}

void UIAbilityLifecycleManager::HandleForegroundFailed(const std::shared_ptr<AbilityRecord> &ability,
    AbilityState state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "state: %{public}d.", static_cast<int32_t>(state));
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is nullptr.");
        return;
    }

    if (!ability->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "this ability is not foregrounding state.");
        return;
    }

    NotifySCBToHandleException(ability,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleForegroundTimeout");

    CloseUIAbilityInner(ability, 0, nullptr, false);
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    const
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr.");
        return nullptr;
    }

    for (auto ability : terminateAbilityList_) {
        if (ability && token == ability->GetToken()->AsObject()) {
            return ability;
        }
    }

    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == token) {
            return iter->second;
        }
    }
    return nullptr;
}

bool UIAbilityLifecycleManager::IsContainsAbility(const sptr<IRemoteObject> &token) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    return IsContainsAbilityInner(token);
}

bool UIAbilityLifecycleManager::IsContainsAbilityInner(const sptr<IRemoteObject> &token) const
{
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

    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject()) {
            sessionAbilityMap_.erase(iter);
            break;
        }
    }
}

void UIAbilityLifecycleManager::EraseSpecifiedAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    for (auto iter = specifiedAbilityMap_.begin(); iter != specifiedAbilityMap_.end(); iter++) {
        auto abilityInfo = abilityRecord->GetAbilityInfo();
        if (iter->second != nullptr && iter->second->GetToken()->AsObject() == abilityRecord->GetToken()->AsObject() &&
            iter->first.abilityName == abilityInfo.name && iter->first.bundleName == abilityInfo.bundleName &&
            iter->first.flag == abilityRecord->GetSpecifiedFlag()) {
            specifiedAbilityMap_.erase(iter);
            break;
        }
    }
}

void UIAbilityLifecycleManager::UpdateProcessName(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr || abilityRequest.sessionInfo == nullptr ||
        abilityRequest.sessionInfo->processOptions == nullptr ||
        !ProcessOptions::IsNewProcessMode(abilityRequest.sessionInfo->processOptions->processMode)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "No need to update process name.");
        return;
    }
    static uint32_t index = 0;
    std::string processName = abilityRequest.abilityInfo.bundleName + SEPARATOR +
        abilityRequest.abilityInfo.moduleName + SEPARATOR + abilityRequest.abilityInfo.name +
        SEPARATOR + std::to_string(index++);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "processName: %{public}s", processName.c_str());
    abilityRecord->SetProcessName(processName);
}

void UIAbilityLifecycleManager::UpdateAbilityRecordLaunchReason(
    const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    if (abilityRecord == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "input record is nullptr.");
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

    if (abilityRequest.IsAcquireShareData()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_SHARE);
        return;
    }

    abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    return;
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetUIAbilityRecordBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    CHECK_POINTER_AND_RETURN(sessionInfo->sessionToken, nullptr);
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    std::string descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, input token is not a sessionToken, token->GetDescriptor(): %{public}s",
            descriptor.c_str());
        return nullptr;
    }

    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter != sessionAbilityMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

int32_t UIAbilityLifecycleManager::NotifySCBToMinimizeUIAbility(const std::shared_ptr<AbilityRecord> abilityRecord,
    const sptr<IRemoteObject> token)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifySCBToMinimizeUIAbility:%{public}s.",
        abilityRecord->GetWant().GetElement().GetURI().c_str());
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_NULL_OBJECT);
    Rosen::WSError ret = sceneSessionManager->PendingSessionToBackgroundForDelegator(token);
    if (ret != Rosen::WSError::WS_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "Call sceneSessionManager PendingSessionToBackgroundForDelegator error:%{public}d", ret);
    }
    return static_cast<int32_t>(ret);
}

int UIAbilityLifecycleManager::MinimizeUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is null");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "abilityInfoName:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetMinimizeReason(fromUser);
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);
    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability state is not foreground");
        return ERR_OK;
    }
    MoveToBackground(abilityRecord);
    return ERR_OK;
}

void UIAbilityLifecycleManager::MoveToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is null");
        return;
    }
    abilityRecord->SetIsNewWant(false);
    auto self(weak_from_this());
    auto task = [abilityRecord, self]() {
        auto selfObj = self.lock();
        if (selfObj == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "UIAbilityLifecycleManager is invalid");
            return;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UIAbilityLifecycleManager move to background timeout");
        selfObj->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        selfObj->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

int UIAbilityLifecycleManager::ResolveLocked(const AbilityRequest &abilityRequest, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ability_name:%{public}s", abilityRequest.want.GetElement().GetURI().c_str());

    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, resolve ability_name:", __func__);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    return CallAbilityLocked(abilityRequest, userId);
}

bool UIAbilityLifecycleManager::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord, const int32_t oriValidUserId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    bool reuse = false;
    auto persistentId = GetPersistentIdByAbilityRequest(abilityRequest, reuse, oriValidUserId);
    if (persistentId == 0) {
        return false;
    }
    targetRecord = sessionAbilityMap_.at(persistentId);
    if (targetRecord) {
        targetRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
        targetRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);
    }
    return true;
}

int UIAbilityLifecycleManager::CallAbilityLocked(const AbilityRequest &abilityRequest, int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    // Get target uiAbility record.
    std::shared_ptr<AbilityRecord> uiAbilityRecord;
    bool reuse = false;
    auto persistentId = GetPersistentIdByAbilityRequest(abilityRequest, reuse, userId);
    if (persistentId == 0) {
        uiAbilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        uiAbilityRecord->SetOwnerMissionUserId(DelayedSingleton<AbilityManagerService>::GetInstance()->GetUserId());
        SetRevicerInfo(abilityRequest, uiAbilityRecord);
        SetLastExitReason(uiAbilityRecord);
    } else {
        uiAbilityRecord = sessionAbilityMap_.at(persistentId);
    }
    uiAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    uiAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);
    NotifyAbilityToken(uiAbilityRecord->GetToken(), abilityRequest);

    // new version started by call type
    auto ret = ResolveAbility(uiAbilityRecord, abilityRequest);
    if (ret == ResolveResultType::OK_HAS_REMOTE_OBJ) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target ability has been resolved.");
        if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "target ability needs to be switched to foreground.");
            auto sessionInfo = CreateSessionInfo(abilityRequest);
            sessionInfo->persistentId = persistentId;
            sessionInfo->state = CallToState::FOREGROUND;
            sessionInfo->reuse = reuse;
            sessionInfo->uiAbilityId = uiAbilityRecord->GetAbilityRecordId();
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(uiAbilityRecord->GetToken());
            return NotifySCBPendingActivation(sessionInfo, abilityRequest);
        }
        return ERR_OK;
    } else if (ret == ResolveResultType::NG_INNER_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resolve failed, error: %{public}d.", RESOLVE_CALL_ABILITY_INNER_ERR);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    auto sessionInfo = CreateSessionInfo(abilityRequest);
    sessionInfo->persistentId = persistentId;
    sessionInfo->reuse = reuse;
    sessionInfo->uiAbilityId = uiAbilityRecord->GetAbilityRecordId();
    if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
        sessionInfo->state = CallToState::FOREGROUND;
    } else {
        sessionInfo->state = CallToState::BACKGROUND;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Notify scb's abilityId is %{public}" PRIu64 ".", sessionInfo->uiAbilityId);
    tmpAbilityMap_.emplace(uiAbilityRecord->GetAbilityRecordId(), uiAbilityRecord);
    return NotifySCBPendingActivation(sessionInfo, abilityRequest);
}

void UIAbilityLifecycleManager::CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sessionInfo is invalid.");
        return;
    }
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    auto descriptor = Str16ToStr8(sessionToken->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token's Descriptor: %{public}s", descriptor.c_str());
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "SCB output abilityId is %{public}" PRIu64 ".", sessionInfo->uiAbilityId);
    auto search = tmpAbilityMap_.find(sessionInfo->uiAbilityId);
    if (search == tmpAbilityMap_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not found UIAbility.");
        return;
    }
    auto uiAbilityRecord = search->second;
    if (uiAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UIAbility not exist.");
        return;
    }
    auto sessionSearch = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (sessionSearch != sessionAbilityMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Session already exist.");
        return;
    }

    sessionAbilityMap_.emplace(sessionInfo->persistentId, uiAbilityRecord);
    tmpAbilityMap_.erase(search);
    uiAbilityRecord->SetSessionInfo(sessionInfo);

    uiAbilityRecord->LoadAbility();
}

sptr<SessionInfo> UIAbilityLifecycleManager::CreateSessionInfo(const AbilityRequest &abilityRequest) const
{
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->callerToken = abilityRequest.callerToken;
    sessionInfo->want = abilityRequest.want;
    if (abilityRequest.startSetting != nullptr) {
        sessionInfo->startSetting = abilityRequest.startSetting;
    }
    sessionInfo->callingTokenId = static_cast<uint32_t>(abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN,
        IPCSkeleton::GetCallingTokenID()));
    return sessionInfo;
}

int UIAbilityLifecycleManager::NotifySCBPendingActivation(sptr<SessionInfo> &sessionInfo,
    const AbilityRequest &abilityRequest) const
{
    auto abilityRecord = GetAbilityRecordByToken(abilityRequest.callerToken);
    if (abilityRecord != nullptr && !abilityRecord->GetRestartAppFlag()) {
        auto callerSessionInfo = abilityRecord->GetSessionInfo();
        CHECK_POINTER_AND_RETURN(callerSessionInfo, ERR_INVALID_VALUE);
        CHECK_POINTER_AND_RETURN(callerSessionInfo->sessionToken, ERR_INVALID_VALUE);
        auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Call PendingSessionActivation by callerSession.");
        return static_cast<int>(callerSession->PendingSessionActivation(sessionInfo));
    }
    CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
    if (sessionInfo->persistentId == 0) {
        const auto &abilityInfo = abilityRequest.abilityInfo;
        auto isStandard = abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD && !abilityRequest.startRecent;
        if (!isStandard) {
            (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->GetAbilitySessionId(
                abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name, sessionInfo->persistentId);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "session id: %{public}d.", sessionInfo->persistentId);
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call PendingSessionActivation by rootSceneSession.");
    return static_cast<int>(rootSceneSession_->PendingSessionActivation(sessionInfo));
}

int UIAbilityLifecycleManager::ResolveAbility(
    const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbilityRecord resolve call record.");
    CHECK_POINTER_AND_RETURN(targetAbility, ResolveResultType::NG_INNER_ERROR);

    ResolveResultType result = targetAbility->Resolve(abilityRequest);
    switch (result) {
        case ResolveResultType::NG_INNER_ERROR:
        case ResolveResultType::OK_HAS_REMOTE_OBJ:
            return result;
        default:
            break;
    }

    if (targetAbility->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbility is ready, directly scheduler call request.");
        targetAbility->CallRequest();
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbility need to call request after lifecycle.");
    return result;
}

void UIAbilityLifecycleManager::NotifyAbilityToken(const sptr<IRemoteObject> &token,
    const AbilityRequest &abilityRequest) const
{
    auto abilityInfoCallback = iface_cast<AppExecFwk::IAbilityInfoCallback>(abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyAbilityToken(token, abilityRequest.want);
    }
}

void UIAbilityLifecycleManager::PrintTimeOutLog(std::shared_ptr<AbilityRecord> ability, uint32_t msgId, bool isHalf)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, ability is nullptr");
        return;
    }
    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
    if (processInfo.pid_ == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "the ability:%{public}s, app may fork fail or not running.",
            ability->GetAbilityInfo().name.data());
        return;
    }
    int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    std::string msgContent = "ability:" + ability->GetAbilityInfo().name + " ";
    if (!GetContentAndTypeId(msgId, msgContent, typeId)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "msgId is invalid.");
        return;
    }

    std::string eventName = isHalf ?
        AppExecFwk::AppFreezeType::LIFECYCLE_HALF_TIMEOUT : AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT;
    TAG_LOGW(AAFwkTag::ABILITYMGR,
        "%{public}s: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, abilityName: %{public}s,"
        "msg: %{public}s",
        eventName.c_str(), processInfo.uid_, processInfo.pid_, ability->GetAbilityInfo().bundleName.c_str(),
        ability->GetAbilityInfo().name.c_str(), msgContent.c_str());

    AppExecFwk::AppfreezeManager::ParamInfo info = {
        .typeId = typeId,
        .pid = processInfo.pid_,
        .eventName = eventName,
        .bundleName = ability->GetAbilityInfo().bundleName,
    };
    FreezeUtil::TimeoutState state = FreezeUtil::TimeoutState::UNKNOWN;
    auto search = stateMap.find(msgId);
    if (search != stateMap.end()) {
        state = search->second;
    }
    if (state != FreezeUtil::TimeoutState::UNKNOWN) {
        auto flow = std::make_unique<FreezeUtil::LifecycleFlow>();
        if (ability->GetToken() != nullptr) {
            flow->token = ability->GetToken()->AsObject();
            flow->state = state;
        }
        info.msg = msgContent + "\nserver:\n" + FreezeUtil::GetInstance().GetLifecycleEvent(*flow);
        if (!isHalf) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(*flow);
        }
        AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info, std::move(flow));
    } else {
        info.msg = msgContent;
        AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info);
    }
}

bool UIAbilityLifecycleManager::GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const
{
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent += "load timeout.";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            msgContent += "foreground timeout.";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
            msgContent += "background timeout.";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent += "terminate timeout.";
            break;
        default:
            return false;
    }
    return true;
}

void UIAbilityLifecycleManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, ability state is %{public}d, it can't complete background.",
            abilityRecord->GetAbilityState());
        return;
    }
    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());

    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
    } else if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToBackground() && abilityRecord->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call request after completing background state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToBackground(false);
    }

    // Abilities ahead of the one started were put in terminate list, we need to terminate them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityList_) {
        if (terminateAbility->GetAbilityState() == AbilityState::BACKGROUND) {
            auto timeoutTask = [terminateAbility, self]() {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "Terminate ability timeout after background.");
                self->DelayCompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }
}

int UIAbilityLifecycleManager::CloseUIAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int resultCode, const Want *resultWant, bool isClearSession)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    return CloseUIAbilityInner(abilityRecord, resultCode, resultWant, isClearSession);
}

int UIAbilityLifecycleManager::CloseUIAbilityInner(std::shared_ptr<AbilityRecord> abilityRecord,
    int resultCode, const Want *resultWant, bool isClearSession)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call, from ability: %{public}s", element.c_str());
    if (abilityRecord->IsTerminating() && !abilityRecord->IsForeground()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ability is on terminating");
        return ERR_OK;
    }
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(abilityRecord->GetToken());
    abilityRecord->SetTerminatingState();
    abilityRecord->SetClearMissionFlag(isClearSession);
    // save result to caller AbilityRecord
    if (resultWant != nullptr) {
        Want* newWant = const_cast<Want*>(resultWant);
        newWant->RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
        abilityRecord->SaveResultToCallers(resultCode, newWant);
    } else {
        Want want;
        abilityRecord->SaveResultToCallers(-1, &want);
    }
    EraseAbilityRecord(abilityRecord);
    if (abilityRecord->GetAbilityState() == AbilityState::INITIAL) {
        if (abilityRecord->GetScheduler() == nullptr) {
            auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
            CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
            handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
        }
        return abilityRecord->TerminateAbility();
    }

    terminateAbilityList_.push_back(abilityRecord);
    abilityRecord->SendResultToCallers();

    if (abilityRecord->IsDebug() && isClearSession) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "notify AppMS terminate");
        return abilityRecord->TerminateAbility();
    }

    if (abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "current ability is active");
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        MoveToBackground(abilityRecord);
        return ERR_OK;
    }

    // ability on background, schedule to terminate.
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
        auto self(shared_from_this());
        auto task = [abilityRecord, self]() {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "close ability by scb timeout");
            self->DelayCompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(task);
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    PrintTimeOutLog(abilityRecord, AbilityManagerService::TERMINATE_TIMEOUT_MSG);

    auto timeoutTask = [self = shared_from_this(), abilityRecord]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "emit delay complete terminate task!");
        self->CompleteTerminate(abilityRecord);
    };
    int killTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * KILL_TIMEOUT_MULTIPLE;
    handler->SubmitTask(timeoutTask, "DELAY_KILL_PROCESS", killTimeout);
}

void UIAbilityLifecycleManager::CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, %{public}s, ability is not terminating.", __func__);
        return;
    }
    abilityRecord->RemoveAbilityDeathRecipient();

    // notify AppMS terminate
    if (abilityRecord->TerminateAbility() != ERR_OK) {
        // Don't return here
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppMS fail to terminate ability.");
    }
    abilityRecord->RevokeUriPermission();
    EraseSpecifiedAbilityRecord(abilityRecord);
    terminateAbilityList_.remove(abilityRecord);
}

int32_t UIAbilityLifecycleManager::GetPersistentIdByAbilityRequest(const AbilityRequest &abilityRequest,
    bool &reuse, int32_t userId) const
{
    if (abilityRequest.collaboratorType != CollaboratorType::DEFAULT_TYPE) {
        return GetReusedCollaboratorPersistentId(abilityRequest, reuse);
    }

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        return GetReusedSpecifiedPersistentId(abilityRequest, reuse, userId);
    }

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        return GetReusedStandardPersistentId(abilityRequest, reuse, userId);
    }

    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Launch mode is not singleton.");
        return 0;
    }

    reuse = true;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::SINGLETON, userId)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "SINGLETON: find.");
            return first;
        }
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "Not find existed ui ability.");
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedSpecifiedPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse, int32_t userId) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not SPECIFIED.");
        return 0;
    }

    reuse = true;
    // specified ability name and bundle name and module name and appIndex format is same as singleton.
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second->GetSpecifiedFlag() == abilityRequest.specifiedFlag &&
            CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::SPECIFIED, userId)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "SPECIFIED: find.");
            return first;
        }
    }
    return 0;
}

int32_t UIAbilityLifecycleManager::GetReusedStandardPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse, int32_t userId) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::STANDARD) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Not STANDARD.");
        return 0;
    }

    if (!abilityRequest.startRecent) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "startRecent is false.");
        return 0;
    }

    reuse = true;
    int64_t sessionTime = 0;
    int32_t persistentId = 0;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (CheckProperties(second, abilityRequest, AppExecFwk::LaunchMode::STANDARD, userId) &&
            second->GetRestartTime() >= sessionTime) {
            persistentId = first;
            sessionTime = second->GetRestartTime();
        }
    }
    return persistentId;
}

int32_t UIAbilityLifecycleManager::GetReusedCollaboratorPersistentId(const AbilityRequest &abilityRequest,
    bool &reuse) const
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");

    reuse = false;
    int64_t sessionTime = 0;
    int32_t persistentId = 0;
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second->GetCollaboratorType() != CollaboratorType::DEFAULT_TYPE &&
            abilityRequest.want.GetStringParam(PARAM_MISSION_AFFINITY_KEY) == second->GetMissionAffinity() &&
            second->GetRestartTime() >= sessionTime) {
            reuse = true;
            persistentId = first;
            sessionTime = second->GetRestartTime();
        }
    }
    return persistentId;
}

bool UIAbilityLifecycleManager::CheckProperties(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const AbilityRequest &abilityRequest, AppExecFwk::LaunchMode launchMode, int32_t userId) const
{
    if (userId != abilityRecord->GetOwnerMissionUserId()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "userId: %{public}d, ability's userId: %{public}d", userId,
            abilityRecord->GetOwnerMissionUserId());
        return false;
    }
    const auto& abilityInfo = abilityRecord->GetAbilityInfo();
    return abilityInfo.launchMode == launchMode && abilityRequest.abilityInfo.name == abilityInfo.name &&
        abilityRequest.abilityInfo.bundleName == abilityInfo.bundleName &&
        abilityRequest.abilityInfo.moduleName == abilityInfo.moduleName &&
        abilityRequest.want.GetIntParam(DLP_INDEX, 0) == abilityRecord->GetAppIndex();
}

void UIAbilityLifecycleManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call, msgId is %{public}d", msgId);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    std::shared_ptr<AbilityRecord> abilityRecord;
    for (auto iter = sessionAbilityMap_.begin(); iter != sessionAbilityMap_.end(); iter++) {
        if (iter->second != nullptr && iter->second->GetAbilityRecordId() == abilityRecordId) {
            abilityRecord = iter->second;
            break;
        }
    }
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, ability record is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call, msgId:%{public}d, name:%{public}s", msgId,
        abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->RevokeUriPermission();
    PrintTimeOutLog(abilityRecord, msgId, isHalf);
    if (isHalf) {
        return;
    }
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            abilityRecord->SetLoading(false);
            HandleLoadTimeout(abilityRecord);
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            HandleForegroundTimeout(abilityRecord);
            break;
        default:
            break;
    }
}

void UIAbilityLifecycleManager::SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (rootSceneSession == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "rootSceneSession is invalid.");
        return;
    }
    auto tmpSceneSession = iface_cast<Rosen::ISession>(rootSceneSession);
    auto descriptor = Str16ToStr8(tmpSceneSession->GetDescriptor());
    if (descriptor != "OHOS.ISession") {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token's Descriptor: %{public}s", descriptor.c_str());
        return;
    }
    rootSceneSession_ = tmpSceneSession;
}

void UIAbilityLifecycleManager::NotifySCBToHandleException(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int32_t errorCode, std::string errorReason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is nullptr");
        return;
    }
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER(sessionInfo);
    CHECK_POINTER(sessionInfo->sessionToken);
    auto session = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call notifySessionException");
    sptr<SessionInfo> info = abilityRecord->GetSessionInfo();
    info->errorCode = errorCode;
    info->errorReason = errorReason;
    session->NotifySessionException(info);
    EraseAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, ability record is nullptr");
        return;
    }
    NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_LOAD_TIMEOUT), "handleLoadTimeout");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
}

void UIAbilityLifecycleManager::HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is nullptr");
        return;
    }
    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "this ability is not foregrounding state");
        return;
    }
    NotifySCBToHandleException(abilityRecord,
        static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_FOREGROUND_TIMEOUT), "handleForegroundTimeout");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
    EraseSpecifiedAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, ability record is nullptr");
        return;
    }
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");
    if (abilityRecord->GetAbilityState() == AbilityState::INITIAL) {
        handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
        abilityRecord->SetLoading(false);
    }
    if (abilityRecord->GetAbilityState() == AbilityState::FOREGROUNDING) {
        handler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    }
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(taskHandler, "Get AbilityTaskHandler failed.");
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUNDING) {
        taskHandler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    }

    terminateAbilityList_.push_back(abilityRecord);
    abilityRecord->SetAbilityState(AbilityState::TERMINATING);
    NotifySCBToHandleException(abilityRecord, static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED),
        "onAbilityDied");
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(abilityRecord->GetToken());
    DispatchTerminate(abilityRecord);
    EraseSpecifiedAbilityRecord(abilityRecord);
}

void UIAbilityLifecycleManager::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityQueue_.empty()) {
        return;
    }

    AbilityRequest abilityRequest = abilityQueue_.front();
    abilityQueue_.pop();
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        return;
    }
    auto callerAbility = GetAbilityRecordByToken(abilityRequest.callerToken);
    if (!flag.empty()) {
        abilityRequest.specifiedFlag = flag;
        bool reuse = false;
        auto currentAccountId = DelayedSingleton<AbilityManagerService>::GetInstance()->GetUserId();
        auto persistentId = GetReusedSpecifiedPersistentId(abilityRequest, reuse, currentAccountId);
        if (persistentId != 0) {
            auto abilityRecord = GetReusedSpecifiedAbility(want, flag);
            if (!abilityRecord) {
                return;
            }
            abilityRecord->SetWant(abilityRequest.want);
            abilityRecord->SetIsNewWant(true);
            UpdateAbilityRecordLaunchReason(abilityRequest, abilityRecord);
            MoveAbilityToFront(abilityRequest, abilityRecord, callerAbility);
            NotifyRestartSpecifiedAbility(abilityRequest, abilityRecord->GetToken());
            return;
        }
    }
    NotifyStartSpecifiedAbility(abilityRequest, want);
    StartAbilityBySpecifed(abilityRequest, callerAbility);
}

void UIAbilityLifecycleManager::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s called.", __func__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityQueue_.empty()) {
        return;
    }
    abilityQueue_.pop();
}

void UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse(const AAFwk::Want &want, const std::string &flag)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityQueue_.empty()) {
        return;
    }
    AbilityRequest abilityRequest = abilityQueue_.front();
    abilityQueue_.pop();
    std::string specifiedProcessFlag = flag;
    abilityRequest.want.SetParam(PARAM_SPECIFIED_PROCESS_FLAG, specifiedProcessFlag);
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return;
    }
    auto sessionInfo = CreateSessionInfo(abilityRequest);
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->persistentId = GetPersistentIdByAbilityRequest(abilityRequest, sessionInfo->reuse,
        abilityRequest.userId);
    sessionInfo->userId = abilityRequest.userId;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Reused sessionId: %{public}d, userId: %{public}d.", sessionInfo->persistentId,
        abilityRequest.userId);
    NotifySCBPendingActivation(sessionInfo, abilityRequest);
}

void UIAbilityLifecycleManager::OnStartSpecifiedProcessTimeoutResponse(const AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s called.", __func__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityQueue_.size() = %{public}zu", abilityQueue_.size());
    if (abilityQueue_.empty()) {
        return;
    }
    abilityQueue_.pop();
}

void UIAbilityLifecycleManager::StartSpecifiedAbilityBySCB(const Want &want, int32_t userId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    AbilityRequest abilityRequest;
    int result = DelayedSingleton<AbilityManagerService>::GetInstance()->GenerateAbilityRequest(
        want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, userId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "cannot find generate ability request");
        return;
    }
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        EnqueueAbilityToFront(abilityRequest);
    }
    DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
        abilityRequest.want, abilityRequest.abilityInfo);
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetReusedSpecifiedAbility(const AAFwk::Want &want,
    const std::string &flag)
{
    auto element = want.GetElement();
    for (const auto& [first, second] : specifiedAbilityMap_) {
        if (flag == first.flag && element.GetAbilityName() == first.abilityName &&
            element.GetBundleName() == first.bundleName) {
            return second;
        }
    }
    return nullptr;
}

void UIAbilityLifecycleManager::EnqueueAbilityToFront(const AbilityRequest &abilityRequest)
{
    abilityQueue_.push(abilityRequest);
}

void UIAbilityLifecycleManager::NotifyRestartSpecifiedAbility(AbilityRequest &request,
    const sptr<IRemoteObject> &token)
{
    if (request.abilityInfoCallback == nullptr) {
        return;
    }
    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (request.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s called.", __func__);
        abilityInfoCallback->NotifyRestartSpecifiedAbility(token);
    }
}

void UIAbilityLifecycleManager::NotifyStartSpecifiedAbility(AbilityRequest &abilityRequest, const AAFwk::Want &want)
{
    if (abilityRequest.abilityInfoCallback == nullptr) {
        return;
    }

    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        Want newWant = want;
        int32_t type = static_cast<int32_t>(abilityRequest.abilityInfo.type);
        newWant.SetParam("abilityType", type);
        sptr<Want> extraParam = new (std::nothrow) Want();
        abilityInfoCallback->NotifyStartSpecifiedAbility(abilityRequest.callerToken, newWant,
            abilityRequest.requestCode, extraParam);
        int32_t procCode = extraParam->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
        if (procCode != 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE, procCode);
        }
        int32_t tokenCode = extraParam->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0);
        if (tokenCode != 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, tokenCode);
        }
    }
}

int UIAbilityLifecycleManager::MoveAbilityToFront(const AbilityRequest &abilityRequest,
    const std::shared_ptr<AbilityRecord> &abilityRecord, std::shared_ptr<AbilityRecord> callerAbility,
    std::shared_ptr<StartOptions> startOptions)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get target ability record failed");
        return ERR_INVALID_VALUE;
    }
    sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
    sessionInfo->want = abilityRequest.want;
    SendSessionInfoToSCB(callerAbility, sessionInfo);
    abilityRecord->RemoveWindowMode();
    if (startOptions != nullptr) {
        abilityRecord->SetWindowMode(startOptions->GetWindowMode());
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::SendSessionInfoToSCB(std::shared_ptr<AbilityRecord> &callerAbility,
    sptr<SessionInfo> &sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (callerAbility != nullptr) {
        auto callerSessionInfo = callerAbility->GetSessionInfo();
        if (callerSessionInfo != nullptr && callerSessionInfo->sessionToken != nullptr) {
            auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
            callerSession->PendingSessionActivation(sessionInfo);
        } else {
            CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
            rootSceneSession_->PendingSessionActivation(sessionInfo);
        }
    } else {
        CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
        rootSceneSession_->PendingSessionActivation(sessionInfo);
    }
    return ERR_OK;
}

int UIAbilityLifecycleManager::StartAbilityBySpecifed(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &callerAbility)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    sptr<SessionInfo> sessionInfo = new SessionInfo();
    sessionInfo->callerToken = abilityRequest.callerToken;
    sessionInfo->want = abilityRequest.want;
    sessionInfo->requestCode = abilityRequest.requestCode;
    sessionInfo->processOptions = abilityRequest.processOptions;
    SpecifiedInfo specifiedInfo;
    specifiedInfo.abilityName = abilityRequest.abilityInfo.name;
    specifiedInfo.bundleName = abilityRequest.abilityInfo.bundleName;
    specifiedInfo.flag = abilityRequest.specifiedFlag;
    specifiedInfoQueue_.push(specifiedInfo);

    SendSessionInfoToSCB(callerAbility, sessionInfo);
    return ERR_OK;
}

void UIAbilityLifecycleManager::CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<IRemoteObject> &callStub)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is null.");
        return;
    }
    if (callStub == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call stub is null.");
        return;
    }
    abilityRecord->CallRequestDone(callStub);
}

int UIAbilityLifecycleManager::ReleaseCallLocked(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "release call ability.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    auto abilityRecords = GetAbilityRecordsByNameInner(element);
    auto isExist = [connect] (const std::shared_ptr<AbilityRecord> &abilityRecord) {
        return abilityRecord->IsExistConnection(connect);
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found ability record by callback.");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER_AND_RETURN(abilityRecord, RELEASE_CALL_ABILITY_INNER_ERR);

    if (!abilityRecord->ReleaseCall(connect)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability release call record failed.");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    return ERR_OK;
}

void UIAbilityLifecycleManager::OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "On callConnect died.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(callRecord);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);

    AppExecFwk::ElementName element = callRecord->GetTargetServiceName();
    auto abilityRecords = GetAbilityRecordsByNameInner(element);
    auto isExist = [callRecord] (const std::shared_ptr<AbilityRecord> &abilityRecord) {
        return abilityRecord->IsExistConnection(callRecord->GetConCallBack());
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found ability record by callback");
        return;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER(abilityRecord);
    abilityRecord->ReleaseCall(callRecord->GetConCallBack());
}

std::vector<std::shared_ptr<AbilityRecord>> UIAbilityLifecycleManager::GetAbilityRecordsByName(
    const AppExecFwk::ElementName &element)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    return GetAbilityRecordsByNameInner(element);
}

std::vector<std::shared_ptr<AbilityRecord>> UIAbilityLifecycleManager::GetAbilityRecordsByNameInner(
    const AppExecFwk::ElementName &element)
{
    std::vector<std::shared_ptr<AbilityRecord>> records;
    for (const auto& [first, second] : sessionAbilityMap_) {
        auto &abilityInfo = second->GetAbilityInfo();
        AppExecFwk::ElementName localElement(abilityInfo.deviceId, abilityInfo.bundleName,
            abilityInfo.name, abilityInfo.moduleName);
        AppExecFwk::ElementName localElementNoModuleName(abilityInfo.deviceId,
            abilityInfo.bundleName, abilityInfo.name);
        if (localElement == element || localElementNoModuleName == element) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "find element %{public}s", localElement.GetURI().c_str());
            records.push_back(second);
        }
    }
    return records;
}

int32_t UIAbilityLifecycleManager::GetSessionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second && second->GetToken()->AsObject() == token) {
            return first;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "not find");
    return 0;
}

void UIAbilityLifecycleManager::GetActiveAbilityList(const std::string &bundleName,
    std::vector<std::string> &abilityList, int32_t pid)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto currentAccountId = DelayedSingleton<AbilityManagerService>::GetInstance()->GetUserId();
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
        CHECK_POINTER_CONTINUE(abilityRecord);
        if (!CheckPid(abilityRecord, pid)) {
            continue;
        }
        if (abilityRecord->GetOwnerMissionUserId() == currentAccountId) {
            const auto &abilityInfo = abilityRecord->GetAbilityInfo();
            if (abilityInfo.bundleName == bundleName && !abilityInfo.name.empty()) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "find ability name is %{public}s", abilityInfo.name.c_str());
                abilityList.push_back(abilityInfo.name);
            }
        }
    }
}

void UIAbilityLifecycleManager::SetRevicerInfo(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    const auto &abilityInfo = abilityRequest.abilityInfo;
    auto isStandard = abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD && !abilityRequest.startRecent;
    if (!isStandard) {
        bool hasRecoverInfo = false;
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            GetAbilityRecoverInfo(abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name, hasRecoverInfo);
        abilityRecord->UpdateRecoveryInfo(hasRecoverInfo);
        (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
            DeleteAbilityRecoverInfo(abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name);
    }
}

void UIAbilityLifecycleManager::SetLastExitReason(std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
        return;
    }

    if (abilityRecord->GetAbilityInfo().bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName is empty.");
        return;
    }

    ExitReason exitReason;
    bool isSetReason;
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        abilityRecord->GetAbilityInfo().bundleName, abilityRecord->GetAbilityInfo().name, isSetReason, exitReason);

    if (isSetReason) {
        abilityRecord->SetLastExitReason(exitReason);
    }
}

bool UIAbilityLifecycleManager::PrepareTerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is null");
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "abilityInfoName:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    if (!CheckPrepareTerminateEnable(abilityRecord)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not support prepare terminate.");
        return false;
    }
    // execute onPrepareToTerminate util timeout
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (taskHandler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to get AbilityTaskHandler.");
        return false;
    }
    auto promise = std::make_shared<std::promise<bool>>();
    auto future = promise->get_future();
    auto task = [promise, abilityRecord]() {
        promise->set_value(abilityRecord->PrepareTerminateAbility());
    };
    taskHandler->SubmitTask(task);
    int prepareTerminateTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * PREPARE_TERMINATE_TIMEOUT_MULTIPLE;
    std::future_status status = future.wait_for(std::chrono::milliseconds(prepareTerminateTimeout));
    if (status == std::future_status::timeout) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "onPrepareToTerminate timeout.");
        return false;
    }
    return future.get();
}

bool UIAbilityLifecycleManager::CheckPrepareTerminateEnable(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability record is not exist or is on terminating.");
        return false;
    }
    auto type = abilityRecord->GetAbilityInfo().type;
    bool isStageBasedModel = abilityRecord->GetAbilityInfo().isStageBasedModel;
    if (!isStageBasedModel || type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability mode not support.");
        return false;
    }
    auto tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission(tokenId)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "failed, please apply permission ohos.permission.PREPARE_APP_TERMINATE");
        return false;
    }
    return true;
}

void UIAbilityLifecycleManager::SetSessionHandler(const sptr<ISessionHandler> &handler)
{
    handler_ = handler;
}

std::shared_ptr<AbilityRecord> UIAbilityLifecycleManager::GetAbilityRecordsById(int32_t sessionId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto search = sessionAbilityMap_.find(sessionId);
    if (search == sessionAbilityMap_.end()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "sessionId is invalid.");
        return nullptr;
    }
    return search->second;
}

void UIAbilityLifecycleManager::GetActiveAbilityList(const std::string &bundleName,
    std::vector<std::string> &abilityList, int32_t targetUserId, int32_t pid) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call.");
    for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "second is nullptr.");
            continue;
        }
        if (!CheckPid(abilityRecord, pid)) {
            continue;
        }
        const auto &abilityInfo = abilityRecord->GetAbilityInfo();
        if (abilityInfo.bundleName == bundleName && !abilityInfo.name.empty() &&
            (targetUserId == DEFAULT_USER_ID || abilityRecord->GetOwnerMissionUserId() == targetUserId)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "find ability name is %{public}s", abilityInfo.name.c_str());
            abilityList.push_back(abilityInfo.name);
        }
    }
    if (!abilityList.empty()) {
        sort(abilityList.begin(), abilityList.end());
        abilityList.erase(unique(abilityList.begin(), abilityList.end()), abilityList.end());
    }
}

bool UIAbilityLifecycleManager::CheckPid(const std::shared_ptr<AbilityRecord> abilityRecord, const int32_t pid) const
{
    CHECK_POINTER_RETURN_BOOL(abilityRecord);
    return pid == NO_PID || abilityRecord->GetPid() == pid;
}

void UIAbilityLifecycleManager::OnAppStateChanged(const AppInfo &info, int32_t targetUserId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    if (info.state == AppState::TERMINATED || info.state == AppState::END) {
        for (const auto& abilityRecord : terminateAbilityList_) {
            if (abilityRecord == nullptr) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "the abilityRecord is nullptr.");
                continue;
            }
            if ((info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) &&
                targetUserId == abilityRecord->GetOwnerMissionUserId()) {
                abilityRecord->SetAppState(info.state);
            }
        }
        return;
    }
    for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        if ((info.processName == abilityRecord->GetAbilityInfo().process ||
            info.processName == abilityRecord->GetApplicationInfo().bundleName) &&
            targetUserId == abilityRecord->GetOwnerMissionUserId()) {
            abilityRecord->SetAppState(info.state);
        }
    }
}

void UIAbilityLifecycleManager::UninstallApp(const std::string &bundleName, int32_t uid, int32_t targetUserId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call.");
    for (auto it = sessionAbilityMap_.begin(); it != sessionAbilityMap_.end();) {
        if (it->second == nullptr) {
            it++;
            continue;
        }
        auto &abilityInfo = it->second->GetAbilityInfo();
        if (abilityInfo.bundleName == bundleName && it->second->GetUid() == uid &&
            (targetUserId == DEFAULT_USER_ID || it->second->GetOwnerMissionUserId() == targetUserId)) {
            (void)DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->
                DeleteAbilityRecoverInfo(abilityInfo.bundleName, abilityInfo.moduleName, abilityInfo.name);
        }
        it++;
    }
}

void UIAbilityLifecycleManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm,
    int32_t userId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call.");
    for (auto [sessionId, abilityRecord] : sessionAbilityMap_) {
        if (abilityRecord == nullptr || userId != abilityRecord->GetOwnerMissionUserId()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, abilityRecord);
            }
        }
    }
}

#ifdef ABILITY_COMMAND_FOR_TEST
int UIAbilityLifecycleManager::BlockAbility(int32_t abilityRecordId, int32_t targetUserId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call.");
    for (const auto& [first, second] : sessionAbilityMap_) {
        if (second == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        if (second->GetRecordId() == abilityRecordId && targetUserId == abilityRecord->GetOwnerMissionUserId()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Call BlockAbility.");
            return second->BlockAbility();
        }
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "The abilityRecordId is invalid.");
    return -1;
}
#endif

void UIAbilityLifecycleManager::Dump(std::vector<std::string> &info)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call begin.");
    std::unordered_map<int32_t, std::shared_ptr<AbilityRecord>> sessionAbilityMapLocked;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
            sessionAbilityMapLocked[sessionId] = abilityRecord;
        }
    }

    int userId = DelayedSingleton<AbilityManagerService>::GetInstance()->GetUserId();
    std::string dumpInfo = "User ID #" + std::to_string(userId);
    info.push_back(dumpInfo);
    dumpInfo = "  current mission lists:{";
    info.push_back(dumpInfo);

    for (const auto& [sessionId, abilityRecord] : sessionAbilityMapLocked) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        if (abilityRecord->GetOwnerMissionUserId() != userId) {
            continue;
        }

        sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
        dumpInfo = "    Mission ID #" + std::to_string(sessionId);
        if (sessionInfo) {
            dumpInfo += "  mission name #[" + sessionInfo->sessionName + "]";
        }
        dumpInfo += "  lockedState #" + std::to_string(abilityRecord->GetLockedState());
        dumpInfo += "  mission affinity #[" + abilityRecord->GetMissionAffinity() + "]";
        info.push_back(dumpInfo);

        abilityRecord->Dump(info);

        dumpInfo = " }";
        info.push_back(dumpInfo);
    }
}

void UIAbilityLifecycleManager::DumpMissionList(
    std::vector<std::string> &info, bool isClient, int userId, const std::string &args)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call start.");
    std::unordered_map<int32_t, std::shared_ptr<AbilityRecord>> sessionAbilityMapLocked;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
            sessionAbilityMapLocked[sessionId] = abilityRecord;
        }
    }
    std::string dumpInfo = "User ID #" + std::to_string(userId);
    info.push_back(dumpInfo);
    dumpInfo = "  current mission lists:{";
    info.push_back(dumpInfo);

    for (const auto& [sessionId, abilityRecord] : sessionAbilityMapLocked) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        if (abilityRecord->GetOwnerMissionUserId() != userId) {
            continue;
        }
        sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
        dumpInfo = "    Mission ID #" + std::to_string(sessionId);
        if (sessionInfo) {
            dumpInfo += "  mission name #[" + sessionInfo->sessionName + "]";
        }
        dumpInfo += "  lockedState #" + std::to_string(abilityRecord->GetLockedState());
        dumpInfo += "  mission affinity #[" + abilityRecord->GetMissionAffinity() + "]";
        info.push_back(dumpInfo);

        std::vector<std::string> params;
        abilityRecord->DumpAbilityState(info, isClient, params);

        dumpInfo = " }";
        info.push_back(dumpInfo);
    }
}

void UIAbilityLifecycleManager::DumpMissionListByRecordId(std::vector<std::string> &info, bool isClient,
    int32_t abilityRecordId, const std::vector<std::string> &params, int userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call.");
    std::unordered_map<int32_t, std::shared_ptr<AbilityRecord>> sessionAbilityMapLocked;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (const auto& [sessionId, abilityRecord] : sessionAbilityMap_) {
            sessionAbilityMapLocked[sessionId] = abilityRecord;
        }
    }
    std::string dumpInfo = "User ID #" + std::to_string(userId);
    info.push_back(dumpInfo);
    dumpInfo = "  current mission lists:{";
    info.push_back(dumpInfo);

    for (const auto& [sessionId, abilityRecord] : sessionAbilityMapLocked) {
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        if (abilityRecord->GetAbilityRecordId() != abilityRecordId) {
            continue;
        }
        sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
        dumpInfo = "    Mission ID #" + std::to_string(sessionId);
        if (sessionInfo) {
            dumpInfo += "  mission name #[" + sessionInfo->sessionName + "]";
        }
        dumpInfo += "  lockedState #" + std::to_string(abilityRecord->GetLockedState());
        dumpInfo += "  mission affinity #[" + abilityRecord->GetMissionAffinity() + "]";
        info.push_back(dumpInfo);

        abilityRecord->DumpAbilityState(info, isClient, params);

        dumpInfo = " }";
        info.push_back(dumpInfo);
    }
}

int UIAbilityLifecycleManager::MoveMissionToFront(int32_t sessionId, std::shared_ptr<StartOptions> startOptions)
{
    CHECK_POINTER_AND_RETURN(rootSceneSession_, ERR_INVALID_VALUE);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecordsById(sessionId);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (startOptions != nullptr) {
        abilityRecord->SetWindowMode(startOptions->GetWindowMode());
    }
    sptr<SessionInfo> sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Call PendingSessionActivation by rootSceneSession.");
    return static_cast<int>(rootSceneSession_->PendingSessionActivation(sessionInfo));
}

int UIAbilityLifecycleManager::ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto abilityRecord = GetAbilityRecordByToken(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Is not self.");
        return ERR_NATIVE_NOT_SELF_APPLICATION;
    }
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    if (sessionInfo->processOptions == nullptr ||
        sessionInfo->processOptions->processMode != ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Process options check failed.");
        return ERR_START_OPTIONS_CHECK_FAILED;
    }
    auto callerSessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN(callerSessionInfo, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(callerSessionInfo->sessionToken, ERR_INVALID_VALUE);
    auto callerSession = iface_cast<Rosen::ISession>(callerSessionInfo->sessionToken);
    return static_cast<int>(callerSession->ChangeSessionVisibilityWithStatusBar(callerSessionInfo, isShow));
}

int UIAbilityLifecycleManager::ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    CHECK_POINTER_AND_RETURN(sessionInfo, ERR_INVALID_VALUE);
    auto iter = sessionAbilityMap_.find(sessionInfo->persistentId);
    if (iter == sessionAbilityMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability not found.");
        return ERR_NATIVE_ABILITY_NOT_FOUND;
    }
    std::shared_ptr<AbilityRecord> uiAbilityRecord = iter->second;
    CHECK_POINTER_AND_RETURN(uiAbilityRecord, ERR_INVALID_VALUE);
    auto state = uiAbilityRecord->GetAbilityVisibilityState();
    if (state == AbilityVisibilityState::UNSPECIFIED || state == AbilityVisibilityState::INITIAL) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability visibility state check failed.");
        return ERR_NATIVE_ABILITY_STATE_CHECK_FAILED;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Change ability visibility state to: %{public}d", isShow);
    if (isShow) {
        uiAbilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_SHOW);
        uiAbilityRecord->ProcessForegroundAbility(sessionInfo->callingTokenId);
    } else {
        uiAbilityRecord->SetAbilityVisibilityState(AbilityVisibilityState::FOREGROUND_HIDE);
    }
    return ERR_OK;
}

int32_t UIAbilityLifecycleManager::UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
    std::vector<int32_t> &sessionIds)
{
    std::unordered_set<std::shared_ptr<AbilityRecord>> abilitySet;
    {
        std::lock_guard<ffrt::mutex> guard(sessionLock_);
        for (auto [sessionId, abilityRecord] : sessionAbilityMap_) {
            if (abilityRecord->GetOwnerMissionUserId() != userId) {
                continue;
            }
            bool isFind = false;
            for (auto iter = sessionInfos.begin(); iter != sessionInfos.end(); iter++) {
                if (iter->persistentId == sessionId) {
                    abilityRecord->UpdateSessionInfo(iter->sessionToken);
                    sessionInfos.erase(iter);
                    isFind = true;
                    break;
                }
            }
            if (!isFind) {
                abilitySet.emplace(abilityRecord);
            }
        }
    }
    for (const auto &info : sessionInfos) {
        sessionIds.emplace_back(info.persistentId);
    }

    auto closeTask = [ self = shared_from_this(), abilitySet]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "The abilities need to be closed.");
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "The manager is nullptr.");
            return;
        }
        for (auto ability : abilitySet) {
            self->CloseUIAbility(ability, -1, nullptr, false);
        }
    };
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (taskHandler != nullptr) {
        taskHandler->SubmitTask(closeTask, TaskQoS::USER_INTERACTIVE);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "The end of updating session info.");
    return ERR_OK;
}

void UIAbilityLifecycleManager::SignRestartAppFlag(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> guard(sessionLock_);
    auto tempSessionAbilityMap = sessionAbilityMap_;
    for (auto &[sessionId, abilityRecord] : tempSessionAbilityMap) {
        if (abilityRecord == nullptr || abilityRecord->GetApplicationInfo().bundleName != bundleName) {
            continue;
        }
        abilityRecord->SetRestartAppFlag(true);
        NotifySCBToHandleException(abilityRecord, static_cast<int32_t>(ErrorLifecycleState::ABILITY_STATE_DIED),
            "onAbilityDied");
    }
}
}  // namespace AAFwk
}  // namespace OHOS