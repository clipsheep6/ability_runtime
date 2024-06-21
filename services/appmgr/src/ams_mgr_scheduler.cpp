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

#include "ams_mgr_scheduler.h"
#include <sys/types.h>

#include "datetime_ex.h"
#include "ipc_skeleton.h"
#include "system_ability_definition.h"

#include "accesstoken_kit.h"
#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "hilog_tag_wrapper.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* TASK_TERMINATE_ABILITY = "TerminateAbilityTask";
constexpr const char* TASK_UPDATE_ABILITY_STATE = "UpdateAbilityStateTask";
constexpr const char* TASK_UPDATE_EXTENSION_STATE = "UpdateExtensionStateTask";
constexpr const char* TASK_REGISTER_APP_STATE_CALLBACK = "RegisterAppStateCallbackTask";
constexpr const char* TASK_STOP_ALL_PROCESS = "StopAllProcessTask";
constexpr const char* TASK_ABILITY_BEHAVIOR_ANALYSIS = "AbilityBehaviorAnalysisTask";
constexpr const char* TASK_KILL_PROCESS_BY_ABILITY_TOKEN = "KillProcessByAbilityTokenTask";
constexpr const char* TASK_KILL_PROCESSES_BY_USERID = "KillProcessesByUserIdTask";
constexpr const char* TASK_KILL_PROCESSES_BY_PIDS = "KillProcessesByPids";
constexpr const char* TASK_ATTACH_PID_TO_PARENT = "AttachPidToParent";
constexpr const char* TASK_KILL_APPLICATION = "KillApplicationTask";
constexpr const char* TASK_CLEAR_PROCESS_BY_ABILITY_TOKEN = "ClearProcessByAbilityTokenTask";
constexpr const char* FOUNDATION_NAME = "foundation";
constexpr const char* SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* TASK_SCENE_BOARD_ATTACH_TIMEOUT = "sceneBoardAttachTimeoutTask";
constexpr const char* TASK_ATTACHED_TO_STATUS_BAR = "AttachedToStatusBar";
constexpr int32_t SCENE_BOARD_ATTACH_TIMEOUT_TASK_TIME = 1000;
};  // namespace

AmsMgrScheduler::AmsMgrScheduler(
    const std::shared_ptr<AppMgrServiceInner> &mgrServiceInner_,
    const std::shared_ptr<AAFwk::TaskHandlerWrap> &handler_)
    : amsMgrServiceInner_(mgrServiceInner_), amsHandler_(handler_)
{}

AmsMgrScheduler::~AmsMgrScheduler()
{
    TAG_LOGI(AAFwkTag::APPMGR, "AmsMgrScheduler instance destroyed");
}

void AmsMgrScheduler::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId)
{
    if (!abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return;
    }

    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    PerfProfile::GetInstance().SetAbilityLoadStartTime(GetTickCount());
    TAG_LOGI(AAFwkTag::APPMGR, "SubmitLoadTask: %{public}s-%{public}s", abilityInfo->bundleName.c_str(),
        abilityInfo->name.c_str());
    std::function<void()> loadAbilityFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, preToken,
        abilityInfo, appInfo, want, abilityRecordId]() {
        amsMgrServiceInner->LoadAbility(token, preToken, abilityInfo, appInfo, want, abilityRecordId);
    };

    // cache other application load ability task before scene board attach
    if (!amsMgrServiceInner_->GetSceneBoardAttachFlag() && abilityInfo->bundleName != SCENE_BOARD_BUNDLE_NAME) {
        amsMgrServiceInner_->CacheLoabAbilityTask(loadAbilityFunc);
        return;
    }
    if (abilityInfo->bundleName == SCENE_BOARD_BUNDLE_NAME && abilityInfo->name == SCENEBOARD_ABILITY_NAME) {
        amsMgrServiceInner_->SetSceneBoardAttachFlag(false);
        // set scene board attach timeout task
        std::weak_ptr<AppMgrServiceInner> amsMgrServiceInner = amsMgrServiceInner_;
        auto timeoutTask = [amsMgrServiceInner]() {
            auto inner = amsMgrServiceInner.lock();
            if (inner != nullptr) {
                inner->SetSceneBoardAttachFlag(true);
            }
        };
        amsHandler_->SubmitTask(timeoutTask, TASK_SCENE_BOARD_ATTACH_TIMEOUT, SCENE_BOARD_ATTACH_TIMEOUT_TASK_TIME);
    }

    amsHandler_->SubmitTask(loadAbilityFunc);
}

void AmsMgrScheduler::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    std::function<void()> updateAbilityStateFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, state] () {
        amsMgrServiceInner->UpdateAbilityState(token, state);
    };
    amsHandler_->SubmitTask(updateAbilityStateFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_UPDATE_ABILITY_STATE,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    std::function<void()> updateExtensionStateFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, state]() {
        amsMgrServiceInner->UpdateExtensionState(token, state);
    };
    amsHandler_->SubmitTask(updateExtensionStateFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_UPDATE_EXTENSION_STATE,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    std::function<void()> terminateAbilityFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, clearMissionFlag]() {
        amsMgrServiceInner->TerminateAbility(token, clearMissionFlag);
    };
    amsHandler_->SubmitTask(terminateAbilityFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_TERMINATE_ABILITY,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AmsMgrScheduler::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    if (!IsReady()) {
        return;
    }
    std::function<void()> registerAppStateCallbackFunc = [amsMgrServiceInner = amsMgrServiceInner_, callback]() {
        amsMgrServiceInner->RegisterAppStateCallback(callback);
    };
    amsHandler_->SubmitTask(registerAppStateCallbackFunc, TASK_REGISTER_APP_STATE_CALLBACK);
}

void AmsMgrScheduler::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility, const int32_t perceptibility, const int32_t connectionState)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    std::function<void()> abilityBehaviorAnalysisFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, preToken,
        visibility, perceptibility, connectionState]() {
        amsMgrServiceInner->AbilityBehaviorAnalysis(token, preToken,
            visibility, perceptibility, connectionState);
    };
    amsHandler_->SubmitTask(abilityBehaviorAnalysisFunc, TASK_ABILITY_BEHAVIOR_ANALYSIS);
}

void AmsMgrScheduler::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyProcessPermission(token) != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification failed", __func__);
        return;
    }

    std::function<void()> killProcessByAbilityTokenFunc = [amsMgrServiceInner = amsMgrServiceInner_, token]() {
        amsMgrServiceInner->KillProcessByAbilityToken(token);
    };
    amsHandler_->SubmitTask(killProcessByAbilityTokenFunc, TASK_KILL_PROCESS_BY_ABILITY_TOKEN);
}

void AmsMgrScheduler::KillProcessesByUserId(int32_t userId)
{
    if (!IsReady()) {
        return;
    }

    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::APPMGR, "The caller is not system-app, can not use system-api");
        return;
    }

    bool isCallingFromFoundation =
        AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(FOUNDATION_NAME);
    auto permission = AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES;
    if (!isCallingFromFoundation &&
        amsMgrServiceInner_->VerifyAccountPermission(permission, userId) == ERR_PERMISSION_DENIED) {
        TAG_LOGE(AAFwkTag::APPMGR, "%{public}s: Permission verification failed", __func__);
        return;
    }

    std::function<void()> killProcessesByUserIdFunc = [amsMgrServiceInner = amsMgrServiceInner_, userId]() {
        amsMgrServiceInner->KillProcessesByUserId(userId);
    };
    amsHandler_->SubmitTask(killProcessesByUserIdFunc, TASK_KILL_PROCESSES_BY_USERID);
}

void AmsMgrScheduler::KillProcessesByPids(std::vector<int32_t> &pids)
{
    if (!IsReady()) {
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not allow other process to call.");
        return;
    }

    std::function<void()> killProcessesByPidsFunc = [amsMgrServiceInner = amsMgrServiceInner_, &pids]() {
        amsMgrServiceInner->KillProcessesByPids(pids);
    };
    amsHandler_->SubmitTask(killProcessesByPidsFunc, TASK_KILL_PROCESSES_BY_PIDS);
}

void AmsMgrScheduler::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    if (!IsReady()) {
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getprocpid();
    if (callingPid != pid) {
        TAG_LOGE(AAFwkTag::APPMGR, "Not allow other process to call.");
        return;
    }

    std::function<void()> attachPidToParentFunc = [amsMgrServiceInner = amsMgrServiceInner_, token, callerToken]() {
        amsMgrServiceInner->AttachPidToParent(token, callerToken);
    };
    amsHandler_->SubmitTask(attachPidToParentFunc, TASK_ATTACH_PID_TO_PARENT);
}

int32_t AmsMgrScheduler::KillProcessWithAccount(
    const std::string &bundleName, const int accountId, const bool clearPageStack)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, accountId = %{public}d, clearPageStack = %{public}d",
        bundleName.c_str(), accountId, clearPageStack);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillApplicationByUserId(bundleName, 0, accountId, clearPageStack);
}

void AmsMgrScheduler::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::APPMGR, "AmsMgrScheduler AttachTimeOut begin");
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    auto task = [amsMgrServiceInner = amsMgrServiceInner_, token]() {
        amsMgrServiceInner->HandleAbilityAttachTimeOut(token);
    };
    amsHandler_->SubmitTask(task);
}

void AmsMgrScheduler::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Notify AppMgrService to prepare to terminate the ability.");
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->PrepareTerminate(token); };
    amsHandler_->SubmitTask(task, AAFwk::TaskQoS::USER_INTERACTIVE);
}

int32_t AmsMgrScheduler::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    return amsMgrServiceInner_->UpdateApplicationInfoInstalled(bundleName, uid);
}

int32_t AmsMgrScheduler::KillApplication(const std::string &bundleName, const bool clearPageStack)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, clearPageStack = %{public}d",
        bundleName.c_str(), clearPageStack);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }

    return amsMgrServiceInner_->KillApplication(bundleName, clearPageStack);
}

int32_t AmsMgrScheduler::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    TAG_LOGI(AAFwkTag::APPMGR, "bundleName = %{public}s, uid = %{public}d", bundleName.c_str(), uid);
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillApplicationByUid(bundleName, uid);
}

int32_t AmsMgrScheduler::KillApplicationSelf(const bool clearPageStack)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->KillApplicationSelf(clearPageStack);
}

bool AmsMgrScheduler::IsReady() const
{
    if (!amsMgrServiceInner_) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsMgrServiceInner_ is null");
        return false;
    }
    if (!amsHandler_) {
        TAG_LOGE(AAFwkTag::APPMGR, "amsHandler_ is null");
        return false;
    }
    return true;
}

void AmsMgrScheduler::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    if (!IsReady()) {
        return;
    }

    amsMgrServiceInner_->GetRunningProcessInfoByToken(token, info);
}

void AmsMgrScheduler::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    if (!IsReady()) {
        return;
    }
    amsMgrServiceInner_->SetAbilityForegroundingFlagToAppRecord(pid);
}

void AmsMgrScheduler::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId)
{
    if (!IsReady()) {
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->StartSpecifiedAbility(want, abilityInfo, requestId); };
    amsHandler_->SubmitTask(task, AAFwk::TaskQoS::USER_INTERACTIVE);
}

void AmsMgrScheduler::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId)
{
    if (!IsReady()) {
        TAG_LOGW(AAFwkTag::APPMGR, "not ready.");
        return;
    }

    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->StartSpecifiedProcess(want, abilityInfo, requestId); };
    amsHandler_->SubmitTask(task, AAFwk::TaskQoS::USER_INTERACTIVE);
}

void AmsMgrScheduler::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    if (!IsReady()) {
        return;
    }
    auto task = [=]() { amsMgrServiceInner_->RegisterStartSpecifiedAbilityResponse(response); };
    amsHandler_->SubmitTask(task);
}

int AmsMgrScheduler::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->GetApplicationInfoByProcessID(pid, application, debug);
}

int32_t AmsMgrScheduler::NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->NotifyAppMgrRecordExitReason(pid, reason, exitMsg);
}

void AmsMgrScheduler::SetCurrentUserId(const int32_t userId)
{
    if (!IsReady()) {
        return;
    }
    amsMgrServiceInner_->SetCurrentUserId(userId);
}

int32_t AmsMgrScheduler::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->GetBundleNameByPid(pid, bundleName, uid);
}

int32_t AmsMgrScheduler::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->RegisterAppDebugListener(listener);
}

int32_t AmsMgrScheduler::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->UnregisterAppDebugListener(listener);
}

int32_t AmsMgrScheduler::AttachAppDebug(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->AttachAppDebug(bundleName);
}

int32_t AmsMgrScheduler::DetachAppDebug(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->DetachAppDebug(bundleName);
}

int32_t AmsMgrScheduler::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->SetAppWaitingDebug(bundleName, isPersist);
}

int32_t AmsMgrScheduler::CancelAppWaitingDebug()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->CancelAppWaitingDebug();
}

int32_t AmsMgrScheduler::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->GetWaitingDebugApp(debugInfoList);
}

bool AmsMgrScheduler::IsWaitingDebugApp(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return false;
    }
    return amsMgrServiceInner_->IsWaitingDebugApp(bundleName);
}

void AmsMgrScheduler::ClearNonPersistWaitingDebugFlag()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return;
    }
    amsMgrServiceInner_->ClearNonPersistWaitingDebugFlag();
}

int32_t AmsMgrScheduler::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return amsMgrServiceInner_->RegisterAbilityDebugResponse(response);
}

bool AmsMgrScheduler::IsAttachDebug(const std::string &bundleName)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return false;
    }
    return amsMgrServiceInner_->IsAttachDebug(bundleName);
}

void AmsMgrScheduler::SetKeepAliveEnableState(const std::string &bundleName, bool enable)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return;
    }
    amsMgrServiceInner_->SetKeepAliveEnableState(bundleName, enable);
}

void AmsMgrScheduler::ClearProcessByToken(sptr<IRemoteObject> token)
{
    if (!IsReady()) {
        return;
    }

    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (nativeInfo.processName != "foundation") {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not foundation.");
        return;
    }

    std::function<void()> clearProcessByTokenFunc = [amsMgrServiceInner = amsMgrServiceInner_, token]() {
        amsMgrServiceInner->ClearProcessByToken(token);
    };
    amsHandler_->SubmitTask(clearProcessByTokenFunc, TASK_CLEAR_PROCESS_BY_ABILITY_TOKEN);
}

bool AmsMgrScheduler::IsMemorySizeSufficent()
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return true;
    }
    if (amsMgrServiceInner_->VerifyRequestPermission() != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Permission verification failed.");
        return true;
    }
    return amsMgrServiceInner_->IsMemorySizeSufficent();
}

void AmsMgrScheduler::AttachedToStatusBar(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        TAG_LOGE(AAFwkTag::APPMGR, "AmsMgrService is not ready.");
        return;
    }
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (nativeInfo.processName != "foundation") {
        TAG_LOGE(AAFwkTag::APPMGR, "caller is not foundation.");
        return;
    }
    std::function<void()> attachedToStatusBarFunc =
        std::bind(&AppMgrServiceInner::AttachedToStatusBar, amsMgrServiceInner_, token);
    amsHandler_->SubmitTask(attachedToStatusBarFunc, TASK_ATTACHED_TO_STATUS_BAR);
}
}  // namespace AppExecFwk
}  // namespace OHOS
