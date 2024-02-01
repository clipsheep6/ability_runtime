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

#include "ability_manager_stub.h"

#include "errors.h"
#include "string_ex.h"

#include "ability_connect_callback_proxy.h"
#include "ability_connect_callback_stub.h"
#include "ability_manager_collaborator_proxy.h"
#include "ability_manager_errors.h"
#include "ability_scheduler_proxy.h"
#include "ability_scheduler_stub.h"
#include "configuration.h"
#include "session_info.h"

namespace OHOS {
namespace AAFwk {
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
namespace {
const std::u16string extensionDescriptor = u"ohos.aafwk.ExtensionManager";
constexpr int32_t CYCLE_LIMIT = 1000;
} // namespace
AbilityManagerStub::AbilityManagerStub()
{
    FirstStepInit();
    SecondStepInit();
    ThirdStepInit();
    FourthStepInit();
}

AbilityManagerStub::~AbilityManagerStub()
{
    requestFuncMap_.clear();
}

void AbilityManagerStub::FirstStepInit()
{
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY)] =
        &AbilityManagerStub::TerminateAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_ABILITY)] =
        &AbilityManagerStub::MinimizeAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD)] =
        &AbilityManagerStub::AttachAbilityThreadInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE)] =
        &AbilityManagerStub::AbilityTransitionDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE)] =
        &AbilityManagerStub::ScheduleConnectAbilityDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE)] =
        &AbilityManagerStub::ScheduleDisconnectAbilityDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE)] =
        &AbilityManagerStub::ScheduleCommandAbilityDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE)] =
        &AbilityManagerStub::ScheduleCommandAbilityWindowDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY)] =
        &AbilityManagerStub::AcquireDataAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY)] =
        &AbilityManagerStub::ReleaseDataAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS)] =
        &AbilityManagerStub::KillProcessInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNINSTALL_APP)] =
        &AbilityManagerStub::UninstallAppInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY)] =
        &AbilityManagerStub::StartAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER)] =
        &AbilityManagerStub::StartAbilityAddCallerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN)] =
        &AbilityManagerStub::StartAbilityAsCallerByTokenInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityAsCallerForOptionInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER)] =
        &AbilityManagerStub::StartAbilityByUIContentSessionAddCallerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityByUIContentSessionForOptionsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT)] =
        &AbilityManagerStub::StartAbilityByInsightIntentInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY)] =
        &AbilityManagerStub::ConnectAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY)] =
        &AbilityManagerStub::DisconnectAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY)] =
        &AbilityManagerStub::StopServiceAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE)] =
        &AbilityManagerStub::DumpStateInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE)] =
        &AbilityManagerStub::DumpSysStateInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_SETTINGS)] =
        &AbilityManagerStub::StartAbilityForSettingsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION)] =
        &AbilityManagerStub::ContinueMissionInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME)] =
        &AbilityManagerStub::ContinueMissionOfBundleNameInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_ABILITY)] =
        &AbilityManagerStub::ContinueAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION)] =
        &AbilityManagerStub::StartContinuationInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION)] =
        &AbilityManagerStub::NotifyCompleteContinuationInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT)] =
        &AbilityManagerStub::NotifyContinuationResultInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY)] =
        &AbilityManagerStub::SendResultToAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_MISSION_LISTENER)] =
        &AbilityManagerStub::RegisterRemoteMissionListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_ON_LISTENER)] =
        &AbilityManagerStub::RegisterRemoteOnListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_OFF_LISTENER)] =
        &AbilityManagerStub::RegisterRemoteOffListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_REMOTE_MISSION_LISTENER)] =
        &AbilityManagerStub::UnRegisterRemoteMissionListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityForOptionsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SYNC_MISSIONS)] =
        &AbilityManagerStub::StartSyncRemoteMissionsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS)] =
        &AbilityManagerStub::StopSyncRemoteMissionsInner;
#ifdef ABILITY_COMMAND_FOR_TEST
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_TIMEOUT)] =
        &AbilityManagerStub::ForceTimeoutForTestInner;
#endif
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE)] =
        &AbilityManagerStub::FreeInstallAbilityFromRemoteInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_FREE_INSTALL_OBSERVER)] =
        &AbilityManagerStub::AddFreeInstallObserverInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE)] =
        &AbilityManagerStub::ConnectAbilityWithTypeInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY)] =
        &AbilityManagerStub::ScheduleRecoverAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE)] =
        &AbilityManagerStub::EnableRecoverAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_ABILITY_BY_SCB)] =
        &AbilityManagerStub::MinimizeUIAbilityBySCBInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLOSE_UI_ABILITY_BY_SCB)] =
        &AbilityManagerStub::CloseUIAbilityBySCBInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_COLLABORATOR)] =
        &AbilityManagerStub::RegisterIAbilityManagerCollaboratorInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_COLLABORATOR)] =
        &AbilityManagerStub::UnregisterIAbilityManagerCollaboratorInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_APP_DEBUG_LISTENER)] =
        &AbilityManagerStub::RegisterAppDebugListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_APP_DEBUG_LISTENER)] =
        &AbilityManagerStub::UnregisterAppDebugListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_APP_DEBUG)] =
        &AbilityManagerStub::AttachAppDebugInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DETACH_APP_DEBUG)] =
        &AbilityManagerStub::DetachAppDebugInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_ABILITY_CONTROLLER_START)] =
        &AbilityManagerStub::IsAbilityControllerStartInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT)] =
        &AbilityManagerStub::ExecuteIntentInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INSIGHT_INTENT_DONE)] =
        &AbilityManagerStub::ExecuteInsightIntentDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_FILE)] =
        &AbilityManagerStub::OpenFileInner;
}

void AbilityManagerStub::SecondStepInit()
{
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER)] =
        &AbilityManagerStub::GetWantSenderInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER)] =
        &AbilityManagerStub::SendWantSenderInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER)] =
        &AbilityManagerStub::CancelWantSenderInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID)] =
        &AbilityManagerStub::GetPendingWantUidInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID)] =
        &AbilityManagerStub::GetPendingWantUserIdInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME)] =
        &AbilityManagerStub::GetPendingWantBundleNameInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE)] =
        &AbilityManagerStub::GetPendingWantCodeInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE)] =
        &AbilityManagerStub::GetPendingWantTypeInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER)] =
        &AbilityManagerStub::RegisterCancelListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER)] =
        &AbilityManagerStub::UnregisterCancelListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT)] =
        &AbilityManagerStub::GetPendingRequestWantInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO)] =
        &AbilityManagerStub::GetWantSenderInfoInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE)] =
        &AbilityManagerStub::GetAppMemorySizeInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE)] =
        &AbilityManagerStub::IsRamConstrainedDeviceInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_UP_APPLICATION_DATA)] =
        &AbilityManagerStub::ClearUpApplicationDataInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP)] =
        &AbilityManagerStub::LockMissionForCleanupInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP)] =
        &AbilityManagerStub::UnlockMissionForCleanupInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSION_LOCKED_STATE)] =
        &AbilityManagerStub::SetLockedStateInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_MISSION_LISTENER)] =
        &AbilityManagerStub::RegisterMissionListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_MISSION_LISTENER)] =
        &AbilityManagerStub::UnRegisterMissionListenerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFOS)] =
        &AbilityManagerStub::GetMissionInfosInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID)] =
        &AbilityManagerStub::GetMissionInfoInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_MISSION)] =
        &AbilityManagerStub::CleanMissionInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS)] =
        &AbilityManagerStub::CleanAllMissionsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT)] =
        &AbilityManagerStub::MoveMissionToFrontInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS)] =
        &AbilityManagerStub::MoveMissionToFrontByOptionsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND)] =
        &AbilityManagerStub::MoveMissionsToForegroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND)] =
        &AbilityManagerStub::MoveMissionsToBackgroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY)] =
        &AbilityManagerStub::StartAbilityByCallInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_REQUEST_DONE)] =
        &AbilityManagerStub::CallRequestDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY)] =
        &AbilityManagerStub::ReleaseCallInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER)] =
        &AbilityManagerStub::StartUserInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER)] =
        &AbilityManagerStub::StopUserInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::LOGOUT_USER)] =
        &AbilityManagerStub::LogoutUserInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO)] =
        &AbilityManagerStub::GetAbilityRunningInfosInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO)] =
        &AbilityManagerStub::GetExtensionRunningInfosInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO)] =
        &AbilityManagerStub::GetProcessRunningInfosInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ABILITY_CONTROLLER)] =
        &AbilityManagerStub::SetAbilityControllerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO)] =
        &AbilityManagerStub::GetMissionSnapshotInfoInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST)] =
        &AbilityManagerStub::IsRunningInStabilityTestInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_APP_NOT_RESPONSE_PROCESS_ID)] =
        &AbilityManagerStub::SendANRProcessIDInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA)] =
        &AbilityManagerStub::AcquireShareDataInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SHARE_DATA_DONE)] =
        &AbilityManagerStub::ShareDataDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_TOKEN)] =
        &AbilityManagerStub::GetAbilityTokenByCalleeObjInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_EXIT_APP)] =
        &AbilityManagerStub::ForceExitAppInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON)] =
        &AbilityManagerStub::RecordAppExitReasonInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON)] =
        &AbilityManagerStub::RecordProcessExitReasonInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SESSION_HANDLER)] =
        &AbilityManagerStub::RegisterSessionHandlerInner;
#ifdef ABILITY_COMMAND_FOR_TEST
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_ABILITY)] =
        &AbilityManagerStub::BlockAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_AMS_SERVICE)] =
        &AbilityManagerStub::BlockAmsServiceInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_APP_SERVICE)] =
        &AbilityManagerStub::BlockAppServiceInner;
#endif
}

void AbilityManagerStub::ThirdStepInit()
{
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER_TEST)] =
        &AbilityManagerStub::StartUserTestInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FINISH_USER_TEST)] =
        &AbilityManagerStub::FinishUserTestInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN)] =
        &AbilityManagerStub::GetTopAbilityTokenInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CHECK_UI_EXTENSION_IS_FOCUSED)] =
        &AbilityManagerStub::CheckUIExtensionIsFocusedInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND)] =
        &AbilityManagerStub::DelegatorDoAbilityForegroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND)] =
        &AbilityManagerStub::DelegatorDoAbilityBackgroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_FOREGROUND)] =
        &AbilityManagerStub::DoAbilityForegroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_BACKGROUND)] =
        &AbilityManagerStub::DoAbilityBackgroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN)] =
        &AbilityManagerStub::GetMissionIdByTokenInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY)] =
        &AbilityManagerStub::GetTopAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN)] =
        &AbilityManagerStub::GetElementNameByTokenInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_ABILITY_INFO_DONE)] =
        &AbilityManagerStub::DumpAbilityInfoDoneInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY)] =
        &AbilityManagerStub::StartExtensionAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY)] =
        &AbilityManagerStub::StopExtensionAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS)] =
        &AbilityManagerStub::UpdateMissionSnapShotFromWMSInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CONNECTION_OBSERVER)] =
        &AbilityManagerStub::RegisterConnectionObserverInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CONNECTION_OBSERVER)] =
        &AbilityManagerStub::UnregisterConnectionObserverInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DLP_CONNECTION_INFOS)] =
        &AbilityManagerStub::GetDlpConnectionInfosInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_ABILITY_TO_BACKGROUND)] =
        &AbilityManagerStub::MoveAbilityToBackgroundInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE)] =
        &AbilityManagerStub::SetMissionContinueStateInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB)] =
        &AbilityManagerStub::PrepareTerminateAbilityBySCBInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUESET_MODAL_UIEXTENSION)] =
        &AbilityManagerStub::RequestModalUIExtensionInner;
#ifdef SUPPORT_GRAPHICS
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL)] =
        &AbilityManagerStub::SetMissionLabelInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_ICON)] =
        &AbilityManagerStub::SetMissionIconInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER)] =
        &AbilityManagerStub::RegisterWindowManagerServiceHandlerInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING)] =
        &AbilityManagerStub::CompleteFirstFrameDrawingInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::StartUIExtensionAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::MinimizeUIExtensionAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::TerminateUIExtensionAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::ConnectUIExtensionAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY)] =
        &AbilityManagerStub::PrepareTerminateAbilityInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO)] =
        &AbilityManagerStub::GetDialogSessionInfoInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT)] =
        &AbilityManagerStub::SendDialogResultInner;
#endif
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_DIALOG_SERVICE)] =
        &AbilityManagerStub::HandleRequestDialogService;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED)] =
        &AbilityManagerStub::HandleReportDrawnCompleted;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD)] =
        &AbilityManagerStub::IsValidMissionIdsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::VERIFY_PERMISSION)] =
        &AbilityManagerStub::VerifyPermissionInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITY_BY_SCB)] =
        &AbilityManagerStub::StartUIAbilityBySCBInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ROOT_SCENE_SESSION)] =
        &AbilityManagerStub::SetRootSceneSessionInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_ABILITY_BY_SCB)] =
        &AbilityManagerStub::CallUIAbilityBySCBInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SPECIFIED_ABILITY_BY_SCB)] =
        &AbilityManagerStub::StartSpecifiedAbilityBySCBInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_SAVE_AS_RESULT)] =
        &AbilityManagerStub::NotifySaveAsResultInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSIONMANAGERSERVICE)] =
        &AbilityManagerStub::SetSessionManagerServiceInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_SESSION_INFO)] =
        &AbilityManagerStub::UpdateSessionInfoBySCBInner;
}

void AbilityManagerStub::FourthStepInit()
{
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK)] =
        &AbilityManagerStub::RegisterAutoStartupSystemCallbackInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK)] =
        &AbilityManagerStub::UnregisterAutoStartupSystemCallbackInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP)] =
        &AbilityManagerStub::SetApplicationAutoStartupInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP)] =
        &AbilityManagerStub::CancelApplicationAutoStartupInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_ALL_AUTO_STARTUP_APPLICATION)] =
        &AbilityManagerStub::QueryAllAutoStartupApplicationsInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_CONNECTION_DATA)] =
        &AbilityManagerStub::GetConnectionDataInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP_BY_EDM)] =
        &AbilityManagerStub::SetApplicationAutoStartupByEDMInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM)] =
        &AbilityManagerStub::CancelApplicationAutoStartupByEDMInner;
    requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES)] =
        &AbilityManagerStub::GetForegroundUIAbilitiesInner;
}

int AbilityManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("Received code : %{public}d", code);
    std::u16string abilityDescriptor = AbilityManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (abilityDescriptor != remoteDescriptor && extensionDescriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is unequal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AbilityManagerStub::GetTopAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    bool isNeedLocalDeviceId = data.ReadBool();
    AppExecFwk::ElementName result = GetTopAbility(isNeedLocalDeviceId);
    if (result.GetDeviceID().empty()) {
        HILOG_DEBUG("GetTopAbilityInner is nullptr");
    }
    reply.WriteParcelable(&result);
    return NO_ERROR;
}

int AbilityManagerStub::GetElementNameByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    bool isNeedLocalDeviceId = data.ReadBool();
    AppExecFwk::ElementName result = GetElementNameByToken(token, isNeedLocalDeviceId);
    if (result.GetDeviceID().empty()) {
        HILOG_DEBUG("GetElementNameByTokenInner is nullptr");
    }
    reply.WriteParcelable(&result);
    return NO_ERROR;
}

int AbilityManagerStub::MoveAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t result = MoveAbilityToBackground(token);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result failed");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::TerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    bool flag = data.ReadBool();
    int32_t result;
    if (flag) {
        result = TerminateAbility(token, resultCode, resultWant);
    } else {
        result = CloseAbility(token, resultCode, resultWant);
    }
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::TerminateUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    int32_t result = TerminateUIExtensionAbility(extensionSessionInfo, resultCode, resultWant);
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendResultToAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    int requestCode = data.ReadInt32();
    int resultCode = data.ReadInt32();
    Want *resultWant = data.ReadParcelable<Want>();
    if (resultWant == nullptr) {
        HILOG_ERROR("resultWant is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendResultToAbility(requestCode, resultCode, *resultWant);
    reply.WriteInt32(result);
    if (resultWant != nullptr) {
        delete resultWant;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    auto fromUser = data.ReadBool();
    int32_t result = MinimizeAbility(token, fromUser);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
    }
    auto fromUser = data.ReadBool();
    int32_t result = MinimizeUIExtensionAbility(extensionSessionInfo, fromUser);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::MinimizeUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    bool fromUser = data.ReadBool();
    int32_t result = MinimizeUIAbilityBySCB(sessionInfo, fromUser);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AttachAbilityThreadInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadRemoteObject());
    auto token = data.ReadRemoteObject();
    int32_t result = AttachAbilityThread(scheduler, token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AbilityTransitionDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int targetState = data.ReadInt32();
    std::unique_ptr<PacMap> saveData(data.ReadParcelable<PacMap>());
    if (!saveData) {
        HILOG_INFO("save data is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AbilityTransitionDone(token, targetState, *saveData);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleConnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    sptr<IRemoteObject> remoteObject = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        remoteObject = data.ReadRemoteObject();
    }
    int32_t result = ScheduleConnectAbilityDone(token, remoteObject);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleDisconnectAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int32_t result = ScheduleDisconnectAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleCommandAbilityDoneInner(MessageParcel &data, MessageParcel &reply)
{
    auto token = data.ReadRemoteObject();
    int32_t result = ScheduleCommandAbilityDone(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleCommandAbilityWindowDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<SessionInfo> sessionInfo = data.ReadParcelable<SessionInfo>();
    int32_t winCmd = data.ReadInt32();
    int32_t abilityCmd = data.ReadInt32();
    int32_t result = ScheduleCommandAbilityWindowDone(token, sessionInfo,
        static_cast<WindowCommand>(winCmd), static_cast<AbilityCommand>(abilityCmd));
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::AcquireDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri = std::make_unique<Uri>(data.ReadString());
    bool tryBind = data.ReadBool();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    sptr<IAbilityScheduler> result = AcquireDataAbility(*uri, tryBind, callerToken);
    HILOG_DEBUG("acquire data ability %{public}s", result ? "ok" : "failed");
    if (result) {
        reply.WriteRemoteObject(result->AsObject());
    } else {
        reply.WriteParcelable(nullptr);
    }
    return NO_ERROR;
}

int AbilityManagerStub::ReleaseDataAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    auto scheduler = iface_cast<IAbilityScheduler>(data.ReadRemoteObject());
    auto callerToken = data.ReadRemoteObject();
    int32_t result = ReleaseDataAbility(scheduler, callerToken);
    HILOG_DEBUG("release data ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::KillProcessInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int result = KillProcess(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ClearUpApplicationDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int32_t userId = data.ReadInt32();
    int result = ClearUpApplicationData(bundleName, userId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("ClearUpApplicationData error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UninstallAppInner(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = Str16ToStr8(data.ReadString16());
    int32_t uid = data.ReadInt32();
    int result = UninstallApp(bundleName, uid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("remove stack error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByUIContentSessionAddCallerInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            HILOG_ERROR("callerToken is nullptr");
            return ERR_INVALID_VALUE;
        }
    }

    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
        if (sessionInfo == nullptr) {
            HILOG_ERROR("sessionInfo is nullptr");
            return ERR_INVALID_VALUE;
        }
    }

    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityByUIContentSession(*want, callerToken, sessionInfo, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByUIContentSessionForOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<StartOptions> startOptions(data.ReadParcelable<StartOptions>());
    if (startOptions == nullptr) {
        HILOG_ERROR("startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
        if (callerToken == nullptr) {
            HILOG_ERROR("callerToken is nullptr");
            return ERR_INVALID_VALUE;
        }
    }
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
        if (sessionInfo == nullptr) {
            HILOG_ERROR("sessionInfo is nullptr");
            return ERR_INVALID_VALUE;
        }
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityByUIContentSession(*want, *startOptions,
        callerToken, sessionInfo, userId, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StartExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t extensionType = data.ReadInt32();
    int32_t result = StartExtensionAbility(*want, callerToken, userId,
        static_cast<AppExecFwk::ExtensionAbilityType>(extensionType));
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::RequestModalUIExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RequestModalUIExtension(*want);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> extensionSessionInfo = nullptr;
    if (data.ReadBool()) {
        extensionSessionInfo = data.ReadParcelable<SessionInfo>();
    }

    int32_t userId = data.ReadInt32();

    int32_t result = StartUIExtensionAbility(extensionSessionInfo, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StopExtensionAbilityInner(MessageParcel& data, MessageParcel& reply)
{
    Want* want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t extensionType = data.ReadInt32();
    int32_t result =
        StopExtensionAbility(*want, callerToken, userId, static_cast<AppExecFwk::ExtensionAbilityType>(extensionType));
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAddCallerInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }

    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAsCallerByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr!");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        asCallerSourceToken =  data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    bool isSendDialogResult = data.ReadBool();
    int32_t result = StartAbilityAsCaller(*want, callerToken, asCallerSourceToken, userId, requestCode,
        isSendDialogResult);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityAsCallerForOptionInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    StartOptions *startOptions = data.ReadParcelable<StartOptions>();
    if (startOptions == nullptr) {
        HILOG_ERROR("startOptions is nullptr");
        delete want;
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> asCallerSourceToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    if (data.ReadBool()) {
        asCallerSourceToken =  data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbilityAsCaller(*want, *startOptions, callerToken, asCallerSourceToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    delete startOptions;
    return NO_ERROR;
}

int AbilityManagerStub::ConnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int32_t result = ConnectAbilityCommon(*want, callback, token, AppExecFwk::ExtensionAbilityType::SERVICE, userId);
    reply.WriteInt32(result);
    if (want != nullptr) {
        delete want;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ConnectAbilityWithTypeInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("%{public}s, want is nullptr!", __func__);
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    AppExecFwk::ExtensionAbilityType extensionType = static_cast<AppExecFwk::ExtensionAbilityType>(data.ReadInt32());
    bool isQueryExtensionOnly = data.ReadBool();
    int32_t result = ConnectAbilityCommon(*want, callback, token, extensionType, userId, isQueryExtensionOnly);
    reply.WriteInt32(result);
    if (want != nullptr) {
        delete want;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ConnectUIExtensionAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("%{public}s, want is nullptr", __func__);
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    }
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t userId = data.ReadInt32();

    sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr;
    if (data.ReadBool()) {
        connectInfo = data.ReadParcelable<UIExtensionAbilityConnectInfo>();
    }

    int32_t result = ConnectUIExtensionAbility(*want, callback, sessionInfo, userId, connectInfo);
    if (connectInfo != nullptr && !reply.WriteParcelable(connectInfo)) {
        HILOG_ERROR("connectInfo write failed.");
    }

    reply.WriteInt32(result);
    if (want != nullptr) {
        delete want;
    }
    return NO_ERROR;
}

int AbilityManagerStub::DisconnectAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IAbilityConnection> callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    int32_t result = DisconnectAbility(callback);
    HILOG_DEBUG("disconnect ability ret = %d", result);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::StopServiceAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t userId = data.ReadInt32();
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t result = StopServiceAbility(*want, userId, token);
    reply.WriteInt32(result);
    delete want;
    return NO_ERROR;
}

int AbilityManagerStub::DumpSysStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> result;
    std::string args = Str16ToStr8(data.ReadString16());
    std::vector<std::string> argList;

    auto isClient = data.ReadBool();
    auto isUserID = data.ReadBool();
    auto UserID = data.ReadInt32();
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return ERR_INVALID_VALUE;
    }
    DumpSysState(args, result, isClient, isUserID, UserID);
    reply.WriteInt32(result.size());
    for (auto stack : result) {
        reply.WriteString16(Str8ToStr16(stack));
    }
    return NO_ERROR;
}

int AbilityManagerStub::DumpStateInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> result;
    std::string args = Str16ToStr8(data.ReadString16());
    std::vector<std::string> argList;
    SplitStr(args, " ", argList);
    if (argList.empty()) {
        return ERR_INVALID_VALUE;
    }
    DumpState(args, result);
    reply.WriteInt32(result.size());
    for (auto stack : result) {
        reply.WriteString16(Str8ToStr16(stack));
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForSettingsInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    AbilityStartSetting *abilityStartSetting = data.ReadParcelable<AbilityStartSetting>();
    if (abilityStartSetting == nullptr) {
        HILOG_ERROR("abilityStartSetting is nullptr");
        delete want;
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *abilityStartSetting, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    delete abilityStartSetting;
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityForOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr.");
        return ERR_INVALID_VALUE;
    }
    StartOptions *startOptions = data.ReadParcelable<StartOptions>();
    if (startOptions == nullptr) {
        HILOG_ERROR("startOptions is nullptr.");
        delete want;
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    int32_t userId = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = StartAbility(*want, *startOptions, callerToken, userId, requestCode);
    reply.WriteInt32(result);
    delete want;
    delete startOptions;
    return NO_ERROR;
}

int AbilityManagerStub::CloseUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t result = CloseUIAbilityBySCB(sessionInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<WantSenderInfo> wantSenderInfo(data.ReadParcelable<WantSenderInfo>());
    if (wantSenderInfo == nullptr) {
        HILOG_ERROR("wantSenderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }
    sptr<IWantSender> wantSender = GetWantSender(*wantSenderInfo, callerToken);
    if (!reply.WriteRemoteObject(((wantSender == nullptr) ? nullptr : wantSender->AsObject()))) {
        HILOG_ERROR("failed to reply wantSender instance to client, for write parcel error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<SenderInfo> senderInfo(data.ReadParcelable<SenderInfo>());
    if (senderInfo == nullptr) {
        HILOG_ERROR("senderInfo is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SendWantSender(wantSender, *senderInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::CancelWantSenderInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }
    CancelWantSender(wantSender);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUidInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t uid = GetPendingWantUid(wantSender);
    reply.WriteInt32(uid);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantUserIdInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = GetPendingWantUserId(wantSender);
    reply.WriteInt32(userId);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantBundleNameInner(MessageParcel &data, MessageParcel &reply)
{
    auto remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        HILOG_ERROR("ReadRemoteObject is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IWantSender> wantSender = iface_cast<IWantSender>(remote);
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = GetPendingWantBundleName(wantSender);
    reply.WriteString16(Str8ToStr16(bundleName));
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantCodeInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t code = GetPendingWantCode(wantSender);
    reply.WriteInt32(code);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingWantTypeInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t type = GetPendingWantType(wantSender);
    reply.WriteInt32(type);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadRemoteObject());
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    RegisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterCancelListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> sender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (sender == nullptr) {
        HILOG_ERROR("sender is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IWantReceiver> receiver = iface_cast<IWantReceiver>(data.ReadRemoteObject());
    if (receiver == nullptr) {
        HILOG_ERROR("receiver is nullptr");
        return ERR_INVALID_VALUE;
    }
    UnregisterCancelListener(sender, receiver);
    return NO_ERROR;
}

int AbilityManagerStub::GetPendingRequestWantInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    int32_t result = GetPendingRequestWant(wantSender, want);
    if (result != NO_ERROR) {
        HILOG_ERROR("GetPendingRequestWant is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(want.get());
    return NO_ERROR;
}

int AbilityManagerStub::GetWantSenderInfoInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWantSender> wantSender = iface_cast<IWantSender>(data.ReadRemoteObject());
    if (wantSender == nullptr) {
        HILOG_ERROR("wantSender is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<WantSenderInfo> info(data.ReadParcelable<WantSenderInfo>());
    int32_t result = GetWantSenderInfo(wantSender, info);
    if (result != NO_ERROR) {
        HILOG_ERROR("GetWantSenderInfo is failed");
        return ERR_INVALID_VALUE;
    }
    reply.WriteParcelable(info.get());
    return NO_ERROR;
}

int AbilityManagerStub::GetAppMemorySizeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetAppMemorySize();
    HILOG_INFO("GetAppMemorySizeInner result %{public}d", result);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetAppMemorySize error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::IsRamConstrainedDeviceInner(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsRamConstrainedDevice();
    if (!reply.WriteBool(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ContinueMissionInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("amsStub %{public}s called.", __func__);
    std::string srcDeviceId = data.ReadString();
    std::string dstDeviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("ContinueMissionInner callback readParcelable failed.");
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        HILOG_ERROR("ContinueMissionInner wantParams readParcelable failed.");
        return ERR_NULL_OBJECT;
    }
    int32_t result = ContinueMission(srcDeviceId, dstDeviceId, missionId, callback, *wantParams);
    HILOG_INFO("ContinueMissionInner result = %{public}d.", result);
    return result;
}

int AbilityManagerStub::ContinueMissionOfBundleNameInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("amsStub %{public}s called!", __func__);
    std::string srcDeviceId = data.ReadString();
    std::string dstDeviceId = data.ReadString();
    std::string bundleName = data.ReadString();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("ContinueMissionInner callback readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    std::unique_ptr<WantParams> wantParams(data.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        HILOG_ERROR("ContinueMissionInner wantParams readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = ContinueMission(srcDeviceId, dstDeviceId, bundleName, callback, *wantParams);
    HILOG_INFO("ContinueMissionInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::ContinueAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    uint32_t versionCode = data.ReadUint32();
    int32_t result = ContinueAbility(deviceId, missionId, versionCode);
    HILOG_INFO("ContinueAbilityInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::StartContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("StartContinuationInner want readParcelable failed!");
        return ERR_NULL_OBJECT;
    }

    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    if (abilityToken == nullptr) {
        HILOG_ERROR("Get abilityToken failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t status = data.ReadInt32();
    int32_t result = StartContinuation(*want, abilityToken, status);
    HILOG_INFO("StartContinuationInner result = %{public}d", result);

    return result;
}

int AbilityManagerStub::NotifyCompleteContinuationInner(MessageParcel &data, MessageParcel &reply)
{
    std::string devId = data.ReadString();
    int32_t sessionId = data.ReadInt32();
    bool isSuccess = data.ReadBool();

    NotifyCompleteContinuation(devId, sessionId, isSuccess);
    HILOG_INFO("NotifyCompleteContinuationInner end");
    return NO_ERROR;
}

int AbilityManagerStub::NotifyContinuationResultInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t continuationResult = data.ReadInt32();

    int32_t result = NotifyContinuationResult(missionId, continuationResult);
    HILOG_INFO("StartContinuationInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = LockMissionForCleanup(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t id = data.ReadInt32();
    int result = UnlockMissionForCleanup(id);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("AbilityManagerStub: unlock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetLockedStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t sessionId = data.ReadInt32();
    bool flag = data.ReadBool();
    SetLockedState(sessionId, flag);
    return NO_ERROR;
}

int AbilityManagerStub::RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("stub register mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = RegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("stub unregister mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = UnRegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int numMax = data.ReadInt32();
    std::vector<MissionInfo> missionInfos;
    int32_t result = GetMissionInfos(deviceId, numMax, missionInfos);
    reply.WriteInt32(missionInfos.size());
    for (auto &it : missionInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetMissionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    MissionInfo info;
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t missionId = data.ReadInt32();
    int result = GetMissionInfo(deviceId, missionId, info);
    if (!reply.WriteParcelable(&info)) {
        HILOG_ERROR("GetMissionInfo error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetMissionInfo result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CleanMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int result = CleanMission(missionId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("CleanMission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int result = CleanAllMissions();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("CleanAllMissions failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFrontInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int result = MoveMissionToFront(missionId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("MoveMissionToFront failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t missionId = GetMissionIdByToken(token);
    if (!reply.WriteInt32(missionId)) {
        HILOG_ERROR("GetMissionIdByToken write missionId failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionToFrontByOptionsInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    std::unique_ptr<StartOptions> startOptions(data.ReadParcelable<StartOptions>());
    if (startOptions == nullptr) {
        HILOG_ERROR("startOptions is nullptr");
        return ERR_INVALID_VALUE;
    }
    int result = MoveMissionToFront(missionId, *startOptions);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("MoveMissionToFront failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::vector<int32_t> missionIds;
    data.ReadInt32Vector(&missionIds);
    int32_t topMissionId = data.ReadInt32();
    int32_t errCode = MoveMissionsToForeground(missionIds, topMissionId);
    if (!reply.WriteInt32(errCode)) {
        return ERR_INVALID_VALUE;
    }
    return errCode;
}

int AbilityManagerStub::MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::vector<int32_t> missionIds;
    std::vector<int32_t> result;

    data.ReadInt32Vector(&missionIds);
    int32_t errCode = MoveMissionsToBackground(missionIds, result);
    HILOG_DEBUG("%{public}s is called. resultSize: %{public}zu", __func__, result.size());
    if (!reply.WriteInt32Vector(result)) {
        HILOG_ERROR("%{public}s is called. WriteInt32Vector Failed", __func__);
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(errCode)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartAbilityByCallInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("AbilityManagerStub::StartAbilityByCallInner begin.");
    Want *want = data.ReadParcelable<Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    sptr<IRemoteObject> callerToken = nullptr;
    if (data.ReadBool()) {
        callerToken = data.ReadRemoteObject();
    }

    int32_t accountId = data.ReadInt32();
    int32_t result = StartAbilityByCall(*want, callback, callerToken, accountId);

    HILOG_DEBUG("resolve call ability ret = %d", result);

    reply.WriteInt32(result);
    delete want;

    HILOG_DEBUG("AbilityManagerStub::StartAbilityByCallInner end.");

    return NO_ERROR;
}

int AbilityManagerStub::StartUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    int32_t result = StartUIAbilityBySCB(sessionInfo);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::CallRequestDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    sptr<IRemoteObject> callStub = data.ReadRemoteObject();
    CallRequestDone(token, callStub);
    return NO_ERROR;
}

int AbilityManagerStub::ReleaseCallInner(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<IAbilityConnection>(data.ReadRemoteObject());
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::unique_ptr<AppExecFwk::ElementName> element(data.ReadParcelable<AppExecFwk::ElementName>());
    if (element == nullptr) {
        HILOG_ERROR("callback stub receive element is nullptr");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ReleaseCall(callback, *element);

    HILOG_DEBUG("release call ability ret = %d", result);

    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::StartUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    int result = StartUser(userId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StartUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StopUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    sptr<IStopUserCallback> callback = nullptr;
    if (data.ReadBool()) {
        callback = iface_cast<IStopUserCallback>(data.ReadRemoteObject());
    }
    int result = StopUser(userId, callback);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StopUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::LogoutUserInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    int result = LogoutUser(userId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("LogoutUser failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetAbilityRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRunningInfo> abilityRunningInfos;
    auto result = GetAbilityRunningInfos(abilityRunningInfos);
    reply.WriteInt32(abilityRunningInfos.size());
    for (auto &it : abilityRunningInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetExtensionRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    auto upperLimit = data.ReadInt32();
    std::vector<ExtensionRunningInfo> infos;
    auto result = GetExtensionRunningInfos(upperLimit, infos);
    reply.WriteInt32(infos.size());
    for (auto &it : infos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int AbilityManagerStub::GetProcessRunningInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto result = GetProcessRunningInfos(infos);
    reply.WriteInt32(infos.size());
    for (auto &it : infos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    bool fixConflict = data.ReadBool();
    int64_t tag = data.ReadInt64();
    int result = StartSyncRemoteMissions(deviceId, fixConflict, tag);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StartSyncRemoteMissionsInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StopSyncRemoteMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int result = StopSyncRemoteMissions(data.ReadString());
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("StopSyncRemoteMissionsInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteMissionListenerInner deviceId empty!");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IRemoteMissionListener> listener = iface_cast<IRemoteMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteMissionListenerInner listener readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterMissionListener(deviceId, listener);
    HILOG_INFO("AbilityManagerStub: RegisterRemoteMissionListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::RegisterRemoteOnListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string type = data.ReadString();
    if (type.empty()) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteOnListenerInner type empty!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteOnListener> listener = iface_cast<IRemoteOnListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteOnListenerInner listener readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterOnListener(type, listener);
    HILOG_INFO("AbilityManagerStub: RegisterRemoteOnListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::RegisterRemoteOffListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string type = data.ReadString();
    if (type.empty()) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteOffListenerInner type empty!");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteOnListener> listener = iface_cast<IRemoteOnListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("AbilityManagerStub: RegisterRemoteOffListenerInner listener readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterOffListener(type, listener);
    HILOG_INFO("AbilityManagerStub: RegisterRemoteOffListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::UnRegisterRemoteMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    if (deviceId.empty()) {
        HILOG_ERROR("AbilityManagerStub: UnRegisterRemoteMissionListenerInner deviceId empty!");
        return INVALID_PARAMETERS_ERR;
    }
    sptr<IRemoteMissionListener> listener = iface_cast<IRemoteMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        HILOG_ERROR("AbilityManagerStub: UnRegisterRemoteMissionListenerInner listener readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = UnRegisterMissionListener(deviceId, listener);
    HILOG_INFO("AbilityManagerStub: UnRegisterRemoteMissionListenerInner result = %{public}d", result);
    return result;
}

int AbilityManagerStub::RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<ISnapshotHandler> handler = iface_cast<ISnapshotHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        HILOG_ERROR("snapshot: AbilityManagerStub read snapshot handler failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterSnapshotHandler(handler);
    HILOG_INFO("snapshot: AbilityManagerStub register snapshot handler result = %{public}d", result);
    return result;
}

int AbilityManagerStub::GetMissionSnapshotInfoInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    bool isLowResolution = data.ReadBool();
    MissionSnapshot missionSnapshot;
    int32_t result = GetMissionSnapshot(deviceId, missionId, missionSnapshot, isLowResolution);
    HILOG_INFO("snapshot: AbilityManagerStub get snapshot result = %{public}d", result);
    if (!reply.WriteParcelable(&missionSnapshot)) {
        HILOG_ERROR("GetMissionSnapshot error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("GetMissionSnapshot result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetAbilityControllerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AppExecFwk::IAbilityController> controller =
        iface_cast<AppExecFwk::IAbilityController>(data.ReadRemoteObject());
    if (controller == nullptr) {
        HILOG_ERROR("AbilityManagerStub: setAbilityControllerInner controller readParcelable failed!");
        return ERR_NULL_OBJECT;
    }
    bool imAStabilityTest = data.ReadBool();
    int32_t result = SetAbilityController(controller, imAStabilityTest);
    HILOG_INFO("AbilityManagerStub: setAbilityControllerInner result = %{public}d", result);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("setAbilityControllerInner failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::IsRunningInStabilityTestInner(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsRunningInStabilityTest();
    HILOG_INFO("AbilityManagerStub: IsRunningInStabilityTest result = %{public}d", result);
    if (!reply.WriteBool(result)) {
        HILOG_ERROR("IsRunningInStabilityTest failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::StartUserTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto observer = data.ReadRemoteObject();
    int32_t result = StartUserTest(*want, observer);
    reply.WriteInt32(result);
    return result;
}

int AbilityManagerStub::FinishUserTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string msg = data.ReadString();
    int64_t resultCode = data.ReadInt64();
    std::string bundleName = data.ReadString();
    int32_t result = FinishUserTest(msg, resultCode, bundleName);
    reply.WriteInt32(result);
    return result;
}

int AbilityManagerStub::GetTopAbilityTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token;
    auto result = GetTopAbility(token);
    if (!reply.WriteRemoteObject(token)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::CheckUIExtensionIsFocusedInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t uiExtensionTokenId = data.ReadUint32();
    bool isFocused = false;
    auto result = CheckUIExtensionIsFocused(uiExtensionTokenId, isFocused);
    if (result == ERR_OK) {
        if (!reply.WriteBool(isFocused)) {
            HILOG_ERROR("reply write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return result;
}

int AbilityManagerStub::DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityForeground(token);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityBackground(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    return 0;
}

int AbilityManagerStub::DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag)
{
    return 0;
}

int AbilityManagerStub::DoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint32_t flag = data.ReadUint32();
    auto result = DoAbilityForeground(token, flag);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int AbilityManagerStub::DoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    uint32_t flag = data.ReadUint32();
    auto result = DoAbilityBackground(token, flag);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::SendANRProcessIDInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    int32_t result = SendANRProcessID(pid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    // should implement in child.
    return NO_ERROR;
}

int AbilityManagerStub::UnregisterObserver(const sptr<AbilityRuntime::IConnectionObserver> &observer)
{
    // should implement in child
    return NO_ERROR;
}

int AbilityManagerStub::GetDlpConnectionInfos(std::vector<AbilityRuntime::DlpConnectionInfo> &infos)
{
    // should implement in child
    return NO_ERROR;
}

int AbilityManagerStub::GetConnectionData(std::vector<AbilityRuntime::ConnectionData> &infos)
{
    // should implement in child
    return NO_ERROR;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityManagerStub::ForceTimeoutForTestInner(MessageParcel &data, MessageParcel &reply)
{
    std::string abilityName = Str16ToStr8(data.ReadString16());
    std::string state = Str16ToStr8(data.ReadString16());
    int result = ForceTimeoutForTest(abilityName, state);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("force ability timeout error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t abilityRecordId = data.ReadInt32();
    int32_t result = BlockAbility(abilityRecordId);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAmsServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BlockAmsService();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::BlockAppServiceInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = BlockAppService();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif

int AbilityManagerStub::FreeInstallAbilityFromRemoteInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    want->SetParam(FROM_REMOTE_KEY, true);

    auto callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t userId = data.ReadInt32();
    int32_t requestCode = data.ReadInt32();
    int32_t result = FreeInstallAbilityFromRemote(*want, callback, userId, requestCode);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::AddFreeInstallObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AbilityRuntime::IFreeInstallObserver> observer =
        iface_cast<AbilityRuntime::IFreeInstallObserver>(data.ReadRemoteObject());
    int32_t result = AddFreeInstallObserver(observer);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::DumpAbilityInfoDoneInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> infos;
    data.ReadStringVector(&infos);
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    int32_t result = DumpAbilityInfoDone(infos, callerToken);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::UpdateMissionSnapShotFromWMSInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        HILOG_ERROR("read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::shared_ptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    if (pixelMap == nullptr) {
        HILOG_ERROR("read pixelMap failed.");
        return ERR_NULL_OBJECT;
    }
    UpdateMissionSnapShot(token, pixelMap);
    return NO_ERROR;
}

int AbilityManagerStub::EnableRecoverAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("EnableRecoverAbilityInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    EnableRecoverAbility(token);
    return NO_ERROR;
}

int AbilityManagerStub::HandleRequestDialogService(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (!callerToken) {
        HILOG_ERROR("callerToken is invalid.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = RequestDialogService(*want, callerToken);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::HandleReportDrawnCompleted(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("called.");
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (callerToken == nullptr) {
        HILOG_ERROR("callerToken is invalid.");
        return ERR_INVALID_VALUE;
    }

    auto result = ReportDrawnCompleted(callerToken);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::AcquireShareDataInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    sptr<IAcquireShareDataCallback> shareData = iface_cast<IAcquireShareDataCallback>(data.ReadRemoteObject());
    if (!shareData) {
        HILOG_ERROR("shareData read failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = AcquireShareData(missionId, shareData);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::ShareDataDoneInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("ShareDataDone read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    int32_t resultCode = data.ReadInt32();
    int32_t uniqueId = data.ReadInt32();
    std::shared_ptr<WantParams> wantParam(data.ReadParcelable<WantParams>());
    if (!wantParam) {
        HILOG_ERROR("wantParam read failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ShareDataDone(token, resultCode, uniqueId, *wantParam);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetAbilityTokenByCalleeObjInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callStub = data.ReadRemoteObject();
    if (!callStub) {
        HILOG_ERROR("GetAbilityToken read call stub failed.");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> result;
    GetAbilityTokenByCalleeObj(callStub, result);
    reply.WriteRemoteObject(result);
    return NO_ERROR;
}

int AbilityManagerStub::ScheduleRecoverAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("ScheduleRecoverAbility read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    int reason = data.ReadInt32();
    Want *want = data.ReadParcelable<Want>();
    ScheduleRecoverAbility(token, reason, want);
    if (want != nullptr) {
        delete want;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = iface_cast<AbilityRuntime::IConnectionObserver>(
        data.ReadRemoteObject());
    if (!observer) {
        HILOG_ERROR("RegisterConnectionObserverInner read observer failed.");
        return ERR_NULL_OBJECT;
    }

    return RegisterObserver(observer);
}

int AbilityManagerStub::UnregisterConnectionObserverInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AbilityRuntime::IConnectionObserver> observer = iface_cast<AbilityRuntime::IConnectionObserver>(
        data.ReadRemoteObject());
    if (!observer) {
        HILOG_ERROR("UnregisterConnectionObserverInner read observer failed.");
        return ERR_NULL_OBJECT;
    }

    return UnregisterObserver(observer);
}

int AbilityManagerStub::GetDlpConnectionInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRuntime::DlpConnectionInfo> infos;
    auto result = GetDlpConnectionInfos(infos);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result failed");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(infos.size())) {
        HILOG_ERROR("write infos size failed");
        return ERR_INVALID_VALUE;
    }

    for (auto &item : infos) {
        if (!reply.WriteParcelable(&item)) {
            HILOG_ERROR("write info item failed");
            return ERR_INVALID_VALUE;
        }
    }

    return ERR_OK;
}

int AbilityManagerStub::GetConnectionDataInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AbilityRuntime::ConnectionData> connectionData;
    auto result = GetConnectionData(connectionData);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result failed");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(connectionData.size())) {
        HILOG_ERROR("write infos size failed");
        return ERR_INVALID_VALUE;
    }

    for (auto &item : connectionData) {
        if (!reply.WriteParcelable(&item)) {
            HILOG_ERROR("write info item failed");
            return ERR_INVALID_VALUE;
        }
    }

    return ERR_OK;
}

int AbilityManagerStub::SetMissionContinueStateInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("SetMissionContinueStateInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    int32_t state = data.ReadInt32();
    int result = SetMissionContinueState(token, static_cast<AAFwk::ContinueState>(state));
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("SetMissionContinueState failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef SUPPORT_GRAPHICS
int AbilityManagerStub::SetMissionLabelInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("SetMissionLabelInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::string label = Str16ToStr8(data.ReadString16());
    int result = SetMissionLabel(token, label);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("SetMissionLabel failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetMissionIconInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        HILOG_ERROR("SetMissionIconInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    if (!icon) {
        HILOG_ERROR("SetMissionIconInner read icon failed.");
        return ERR_NULL_OBJECT;
    }

    int result = SetMissionIcon(token, icon);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("SetMissionIcon failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterWindowManagerServiceHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IWindowManagerServiceHandler> handler = iface_cast<IWindowManagerServiceHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        HILOG_ERROR("%{public}s read WMS handler failed!", __func__);
        return ERR_NULL_OBJECT;
    }
    return RegisterWindowManagerServiceHandler(handler);
}

int AbilityManagerStub::CompleteFirstFrameDrawingInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<IRemoteObject> abilityToken = data.ReadRemoteObject();
    if (abilityToken == nullptr) {
        HILOG_ERROR("%{public}s read abilityToken failed!", __func__);
        return ERR_NULL_OBJECT;
    }
    CompleteFirstFrameDrawing(abilityToken);
    return 0;
}

int AbilityManagerStub::PrepareTerminateAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("call");
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    sptr<IPrepareTerminateCallback> callback = iface_cast<IPrepareTerminateCallback>(data.ReadRemoteObject());
    if (callback == nullptr) {
        HILOG_ERROR("callback is nullptr");
        return ERR_NULL_OBJECT;
    }
    int result = PrepareTerminateAbility(token, callback);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("end faild. err: %{public}d", result);
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::GetDialogSessionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("call");
    std::string dialogSessionId = data.ReadString();
    sptr<DialogSessionInfo> info;
    int result = GetDialogSessionInfo(dialogSessionId, info);
    if (result != ERR_OK || info == nullptr) {
        HILOG_ERROR("not find dialogSessionInfo");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(info)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SendDialogResultInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("call");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::string dialogSessionId = data.ReadString();
    bool isAllow = data.ReadBool();
    int result = SendDialogResult(*want, dialogSessionId, isAllow);
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif

int32_t AbilityManagerStub::IsValidMissionIdsInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;

    data.ReadInt32Vector(&missionIds);
    auto err = IsValidMissionIds(missionIds, results);
    if (err != ERR_OK) {
        results.clear();
    }

    if (!reply.WriteInt32(err)) {
        return ERR_INVALID_VALUE;
    }

    reply.WriteInt32(static_cast<int32_t>(results.size()));
    for (auto &item : results) {
        if (!reply.WriteParcelable(&item)) {
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int AbilityManagerStub::VerifyPermissionInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("VerifyPermission call.");
    std::string permission = data.ReadString();
    int32_t pid = data.ReadInt32();
    int32_t uid = data.ReadInt32();

    auto result = VerifyPermission(permission, pid, uid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("VerifyPermission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::ForceExitAppInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::unique_ptr<ExitReason> exitReason(data.ReadParcelable<ExitReason>());
    if (!exitReason) {
        HILOG_ERROR("exitReason is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = ForceExitApp(pid, *exitReason);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::RecordAppExitReasonInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ExitReason> exitReason(data.ReadParcelable<ExitReason>());
    if (!exitReason) {
        HILOG_ERROR("exitReason is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RecordAppExitReason(*exitReason);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::RecordProcessExitReasonInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::unique_ptr<ExitReason> exitReason(data.ReadParcelable<ExitReason>());
    if (!exitReason) {
        HILOG_ERROR("exitReason is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RecordProcessExitReason(pid, *exitReason);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int AbilityManagerStub::SetRootSceneSessionInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Call.");
    auto rootSceneSession = data.ReadRemoteObject();
    if (rootSceneSession == nullptr) {
        HILOG_ERROR("Read rootSceneSession failed.");
        return ERR_INVALID_VALUE;
    }
    SetRootSceneSession(rootSceneSession);
    return NO_ERROR;
}

int AbilityManagerStub::CallUIAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Call.");
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    CallUIAbilityBySCB(sessionInfo);
    return NO_ERROR;
}

int32_t AbilityManagerStub::StartSpecifiedAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Call.");
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    StartSpecifiedAbilityBySCB(*want);
    return NO_ERROR;
}

int AbilityManagerStub::NotifySaveAsResultInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (!want) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    int resultCode = data.ReadInt32();
    int requestCode = data.ReadInt32();
    int32_t result = NotifySaveAsResult(*want, resultCode, requestCode);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int AbilityManagerStub::SetSessionManagerServiceInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> sessionManagerService = data.ReadRemoteObject();
    if (!sessionManagerService) {
        HILOG_ERROR("SetSessionManagerServiceInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }
    SetSessionManagerService(sessionManagerService);
    return NO_ERROR;
}

int32_t AbilityManagerStub::RegisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = data.ReadInt32();
    sptr<IAbilityManagerCollaborator> collaborator = iface_cast<IAbilityManagerCollaborator>(data.ReadRemoteObject());
    if (collaborator == nullptr) {
        HILOG_ERROR("read collaborator failed.");
        return ERR_NULL_OBJECT;
    }
    int32_t ret = RegisterIAbilityManagerCollaborator(type, collaborator);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

int32_t AbilityManagerStub::UnregisterIAbilityManagerCollaboratorInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t type = data.ReadInt32();
    int32_t ret = UnregisterIAbilityManagerCollaborator(type);
    reply.WriteInt32(ret);
    return NO_ERROR;
}

int AbilityManagerStub::PrepareTerminateAbilityBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Call.");
    sptr<SessionInfo> sessionInfo = nullptr;
    if (data.ReadBool()) {
        sessionInfo = data.ReadParcelable<SessionInfo>();
    }
    bool isPrepareTerminate = false;
    auto result = PrepareTerminateAbilityBySCB(sessionInfo, isPrepareTerminate);
    if (result == ERR_OK) {
        if (!reply.WriteBool(isPrepareTerminate)) {
            HILOG_ERROR("reply write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return result;
}

int32_t AbilityManagerStub::RegisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("Callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterAutoStartupSystemCallback(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::UnregisterAutoStartupSystemCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        HILOG_ERROR("Callback is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = UnregisterAutoStartupSystemCallback(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::SetApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        HILOG_ERROR("Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SetApplicationAutoStartup(*info);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::CancelApplicationAutoStartupInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        HILOG_ERROR("Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = CancelApplicationAutoStartup(*info);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::QueryAllAutoStartupApplicationsInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AutoStartupInfo> infoList;
    auto result = QueryAllAutoStartupApplications(infoList);
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }

    reply.WriteInt32(static_cast<int32_t>(infoList.size()));
    for (auto &info : infoList) {
        if (!reply.WriteParcelable(&info)) {
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int AbilityManagerStub::RegisterSessionHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> handler = data.ReadRemoteObject();
    if (handler == nullptr) {
        HILOG_ERROR("stub register session handler, handler is nullptr.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RegisterSessionHandler(handler);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::RegisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Called.");
    auto appDebugLister = iface_cast<AppExecFwk::IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        HILOG_ERROR("App debug lister is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = RegisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Failed to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::UnregisterAppDebugListenerInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Called.");
    auto appDebugLister = iface_cast<AppExecFwk::IAppDebugListener>(data.ReadRemoteObject());
    if (appDebugLister == nullptr) {
        HILOG_ERROR("App debug lister is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto result = UnregisterAppDebugListener(appDebugLister);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::AttachAppDebugInner(MessageParcel &data, MessageParcel &reply)
{
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        HILOG_ERROR("Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = AttachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::DetachAppDebugInner(MessageParcel &data, MessageParcel &reply)
{
    auto bundleName = data.ReadString();
    if (bundleName.empty()) {
        HILOG_ERROR("Bundle name is empty.");
        return ERR_INVALID_VALUE;
    }

    auto result = DetachAppDebug(bundleName);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::IsAbilityControllerStartInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return true;
    }
    bool result = IsAbilityControllerStart(*want);
    reply.WriteBool(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::ExecuteIntentInner(MessageParcel &data, MessageParcel &reply)
{
    uint64_t key = data.ReadUint64();
    sptr<IRemoteObject> callerToken = data.ReadRemoteObject();
    if (callerToken == nullptr) {
        HILOG_ERROR("failed to get remote object.");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<InsightIntentExecuteParam> param(data.ReadParcelable<InsightIntentExecuteParam>());
    if (param == nullptr) {
        HILOG_ERROR("param is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto result = ExecuteIntent(key, callerToken, *param);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AbilityManagerStub::StartAbilityByInsightIntentInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> callerToken = nullptr;
    if (!data.ReadBool()) {
        HILOG_ERROR("invalid caller token");
        return ERR_INVALID_VALUE;
    }
    callerToken = data.ReadRemoteObject();
    uint64_t intentId = data.ReadUint64();
    int32_t userId = data.ReadInt32();
    int32_t result = StartAbilityByInsightIntent(*want, callerToken, intentId, userId);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::ExecuteInsightIntentDoneInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Called.");
    auto token = data.ReadRemoteObject();
    if (token == nullptr) {
        HILOG_ERROR("Failed to get remote object.");
        return ERR_INVALID_VALUE;
    }

    auto intentId = data.ReadInt64();
    std::unique_ptr<InsightIntentExecuteResult> executeResult(data.ReadParcelable<InsightIntentExecuteResult>());
    if (!executeResult) {
        HILOG_ERROR("Execute result is nullptr");
        return ERR_INVALID_VALUE;
    }

    int32_t result = ExecuteInsightIntentDone(token, intentId, *executeResult);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AbilityManagerStub::SetApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        HILOG_ERROR("Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadBool();
    int32_t result = SetApplicationAutoStartupByEDM(*info, flag);
    return reply.WriteInt32(result);
}

int32_t AbilityManagerStub::CancelApplicationAutoStartupByEDMInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<AutoStartupInfo> info = data.ReadParcelable<AutoStartupInfo>();
    if (info == nullptr) {
        HILOG_ERROR("Info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadBool();
    int32_t result = CancelApplicationAutoStartupByEDM(*info, flag);
    return reply.WriteInt32(result);
}

int32_t AbilityManagerStub::OpenFileInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        HILOG_ERROR("To read uri failed.");
        return ERR_DEAD_OBJECT;
    }
    auto flag = data.ReadInt32();
    int fd = OpenFile(*uri, flag);
    reply.WriteFileDescriptor(fd);
    return ERR_OK;
}

int32_t AbilityManagerStub::GetForegroundUIAbilitiesInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("Called.");
    std::vector<AppExecFwk::AbilityStateData> abilityStateDatas;
    int32_t result = GetForegroundUIAbilities(abilityStateDatas);
    if (result != ERR_OK) {
        HILOG_ERROR("Get foreground uI abilities is failed.");
        return result;
    }
    auto infoSize = abilityStateDatas.size();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("Info size exceeds the limit.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(infoSize)) {
        HILOG_ERROR("Write data size failed.");
        return ERR_INVALID_VALUE;
    }
    for (auto &it : abilityStateDatas) {
        if (!reply.WriteParcelable(&it)) {
            HILOG_ERROR("Write parcelable failed.");
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AbilityManagerStub::UpdateSessionInfoBySCBInner(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadInt32();
    int32_t threshold = 512;
    if (size > threshold) {
        HILOG_ERROR("Size of vector too large.");
        return ERR_ENOUGH_DATA;
    }
    std::vector<SessionInfo> sessionInfos;
    for (auto i = 0; i < size; i++) {
        std::unique_ptr<SessionInfo> info(data.ReadParcelable<SessionInfo>());
        if (info == nullptr) {
            HILOG_ERROR("Read session info failed.");
            return INNER_ERR;
        }
        sessionInfos.emplace_back(*info);
    }
    int32_t userId = data.ReadInt32();
    UpdateSessionInfoBySCB(sessionInfos, userId);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS