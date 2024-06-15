/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_manager_stub_builder.h"

namespace OHOS {
namespace AAFwk {

void AbilityManagerStubBuilder::BuildMessageMap(sptr<AbilityManagerStub> stub)
{
    FirstStepInit(stub);
    SecondStepInit(stub);
    ThirdStepInit(stub);
    FourthStepInit(stub);
    FifthStepInit(stub);
}

void AbilityManagerStubBuilder::FirstStepInit(sptr<AbilityManagerStub> &stub)
{
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_ABILITY)] =
        &AbilityManagerStub::TerminateAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_ABILITY)] =
        &AbilityManagerStub::MinimizeAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_ABILITY_THREAD)] =
        &AbilityManagerStub::AttachAbilityThreadInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_TRANSITION_DONE)] =
        &AbilityManagerStub::AbilityTransitionDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_DONE)] =
        &AbilityManagerStub::ScheduleConnectAbilityDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY_DONE)] =
        &AbilityManagerStub::ScheduleDisconnectAbilityDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_DONE)] =
        &AbilityManagerStub::ScheduleCommandAbilityDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMMAND_ABILITY_WINDOW_DONE)] =
        &AbilityManagerStub::ScheduleCommandAbilityWindowDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY)] =
        &AbilityManagerStub::AcquireDataAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY)] =
        &AbilityManagerStub::ReleaseDataAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS)] =
        &AbilityManagerStub::KillProcessInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNINSTALL_APP)] =
        &AbilityManagerStub::UninstallAppInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UPGRADE_APP)] =
        &AbilityManagerStub::UpgradeAppInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY)] =
        &AbilityManagerStub::StartAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_ADD_CALLER)] =
        &AbilityManagerStub::StartAbilityAddCallerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_WITH_SPECIFY_TOKENID)] =
        &AbilityManagerStub::StartAbilityInnerSpecifyTokenId;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN)] =
        &AbilityManagerStub::StartAbilityAsCallerByTokenInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityAsCallerForOptionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_ADD_CALLER)] =
        &AbilityManagerStub::StartAbilityByUIContentSessionAddCallerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_SESSION_ABILITY_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityByUIContentSessionForOptionsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_BY_INSIGHT_INTENT)] =
        &AbilityManagerStub::StartAbilityByInsightIntentInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY)] =
        &AbilityManagerStub::ConnectAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DISCONNECT_ABILITY)] =
        &AbilityManagerStub::DisconnectAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SERVICE_ABILITY)] =
        &AbilityManagerStub::StopServiceAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_STATE)] =
        &AbilityManagerStub::DumpStateInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMPSYS_STATE)] =
        &AbilityManagerStub::DumpSysStateInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_SETTINGS)] =
        &AbilityManagerStub::StartAbilityForSettingsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION)] =
        &AbilityManagerStub::ContinueMissionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_MISSION_OF_BUNDLENAME)] =
        &AbilityManagerStub::ContinueMissionOfBundleNameInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONTINUE_ABILITY)] =
        &AbilityManagerStub::ContinueAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CONTINUATION)] =
        &AbilityManagerStub::StartContinuationInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_COMPLETE_CONTINUATION)] =
        &AbilityManagerStub::NotifyCompleteContinuationInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_CONTINUATION_RESULT)] =
        &AbilityManagerStub::NotifyContinuationResultInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_RESULT_TO_ABILITY)] =
        &AbilityManagerStub::SendResultToAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_MISSION_LISTENER)] =
        &AbilityManagerStub::RegisterRemoteMissionListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_ON_LISTENER)] =
        &AbilityManagerStub::RegisterRemoteOnListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_REMOTE_OFF_LISTENER)] =
        &AbilityManagerStub::RegisterRemoteOffListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_REMOTE_MISSION_LISTENER)] =
        &AbilityManagerStub::UnRegisterRemoteMissionListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityForOptionsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SYNC_MISSIONS)] =
        &AbilityManagerStub::StartSyncRemoteMissionsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_SYNC_MISSIONS)] =
        &AbilityManagerStub::StopSyncRemoteMissionsInner;
#ifdef ABILITY_COMMAND_FOR_TEST
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_TIMEOUT)] =
        &AbilityManagerStub::ForceTimeoutForTestInner;
#endif
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FREE_INSTALL_ABILITY_FROM_REMOTE)] =
        &AbilityManagerStub::FreeInstallAbilityFromRemoteInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ADD_FREE_INSTALL_OBSERVER)] =
        &AbilityManagerStub::AddFreeInstallObserverInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE)] =
        &AbilityManagerStub::ConnectAbilityWithTypeInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY)] =
        &AbilityManagerStub::ScheduleRecoverAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ABILITY_RECOVERY_ENABLE)] =
        &AbilityManagerStub::EnableRecoverAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_ABILITY_BY_SCB)] =
        &AbilityManagerStub::MinimizeUIAbilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLOSE_UI_ABILITY_BY_SCB)] =
        &AbilityManagerStub::CloseUIAbilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_COLLABORATOR)] =
        &AbilityManagerStub::RegisterIAbilityManagerCollaboratorInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_COLLABORATOR)] =
        &AbilityManagerStub::UnregisterIAbilityManagerCollaboratorInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_APP_DEBUG_LISTENER)] =
        &AbilityManagerStub::RegisterAppDebugListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_APP_DEBUG_LISTENER)] =
        &AbilityManagerStub::UnregisterAppDebugListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ATTACH_APP_DEBUG)] =
        &AbilityManagerStub::AttachAppDebugInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DETACH_APP_DEBUG)] =
        &AbilityManagerStub::DetachAppDebugInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_ABILITY_CONTROLLER_START)] =
        &AbilityManagerStub::IsAbilityControllerStartInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT)] =
        &AbilityManagerStub::ExecuteIntentInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INSIGHT_INTENT_DONE)] =
        &AbilityManagerStub::ExecuteInsightIntentDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_FILE)] =
        &AbilityManagerStub::OpenFileInner;
}

void AbilityManagerStubBuilder::SecondStepInit(sptr<AbilityManagerStub> &stub)
{
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER)] =
        &AbilityManagerStub::GetWantSenderInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_PENDING_WANT_SENDER)] =
        &AbilityManagerStub::SendWantSenderInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_PENDING_WANT_SENDER)] =
        &AbilityManagerStub::CancelWantSenderInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_UID)] =
        &AbilityManagerStub::GetPendingWantUidInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_USERID)] =
        &AbilityManagerStub::GetPendingWantUserIdInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_BUNDLENAME)] =
        &AbilityManagerStub::GetPendingWantBundleNameInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_CODE)] =
        &AbilityManagerStub::GetPendingWantCodeInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_TYPE)] =
        &AbilityManagerStub::GetPendingWantTypeInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CANCEL_LISTENER)] =
        &AbilityManagerStub::RegisterCancelListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CANCEL_LISTENER)] =
        &AbilityManagerStub::UnregisterCancelListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_REQUEST_WANT)] =
        &AbilityManagerStub::GetPendingRequestWantInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PENDING_WANT_SENDER_INFO)] =
        &AbilityManagerStub::GetWantSenderInfoInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_APP_MEMORY_SIZE)] =
        &AbilityManagerStub::GetAppMemorySizeInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_RAM_CONSTRAINED_DEVICE)] =
        &AbilityManagerStub::IsRamConstrainedDeviceInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAR_UP_APPLICATION_DATA)] =
        &AbilityManagerStub::ClearUpApplicationDataInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::LOCK_MISSION_FOR_CLEANUP)] =
        &AbilityManagerStub::LockMissionForCleanupInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNLOCK_MISSION_FOR_CLEANUP)] =
        &AbilityManagerStub::UnlockMissionForCleanupInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSION_LOCKED_STATE)] =
        &AbilityManagerStub::SetLockedStateInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_MISSION_LISTENER)] =
        &AbilityManagerStub::RegisterMissionListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_MISSION_LISTENER)] =
        &AbilityManagerStub::UnRegisterMissionListenerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFOS)] =
        &AbilityManagerStub::GetMissionInfosInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_INFO_BY_ID)] =
        &AbilityManagerStub::GetMissionInfoInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_MISSION)] =
        &AbilityManagerStub::CleanMissionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CLEAN_ALL_MISSIONS)] =
        &AbilityManagerStub::CleanAllMissionsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT)] =
        &AbilityManagerStub::MoveMissionToFrontInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSION_TO_FRONT_BY_OPTIONS)] =
        &AbilityManagerStub::MoveMissionToFrontByOptionsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_FOREGROUND)] =
        &AbilityManagerStub::MoveMissionsToForegroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_MISSIONS_TO_BACKGROUND)] =
        &AbilityManagerStub::MoveMissionsToBackgroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY)] =
        &AbilityManagerStub::StartAbilityByCallInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_REQUEST_DONE)] =
        &AbilityManagerStub::CallRequestDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RELEASE_CALL_ABILITY)] =
        &AbilityManagerStub::ReleaseCallInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER)] =
        &AbilityManagerStub::StartUserInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_USER)] =
        &AbilityManagerStub::StopUserInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::LOGOUT_USER)] =
        &AbilityManagerStub::LogoutUserInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_RUNNING_INFO)] =
        &AbilityManagerStub::GetAbilityRunningInfosInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_EXTENSION_RUNNING_INFO)] =
        &AbilityManagerStub::GetExtensionRunningInfosInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_PROCESS_RUNNING_INFO)] =
        &AbilityManagerStub::GetProcessRunningInfosInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ABILITY_CONTROLLER)] =
        &AbilityManagerStub::SetAbilityControllerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_SNAPSHOT_INFO)] =
        &AbilityManagerStub::GetMissionSnapshotInfoInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_USER_A_STABILITY_TEST)] =
        &AbilityManagerStub::IsRunningInStabilityTestInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::ACQUIRE_SHARE_DATA)] =
        &AbilityManagerStub::AcquireShareDataInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SHARE_DATA_DONE)] =
        &AbilityManagerStub::ShareDataDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_TOKEN)] =
        &AbilityManagerStub::GetAbilityTokenByCalleeObjInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FORCE_EXIT_APP)] =
        &AbilityManagerStub::ForceExitAppInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_APP_EXIT_REASON)] =
        &AbilityManagerStub::RecordAppExitReasonInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RECORD_PROCESS_EXIT_REASON)] =
        &AbilityManagerStub::RecordProcessExitReasonInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_SESSION_HANDLER)] =
        &AbilityManagerStub::RegisterSessionHandlerInner;
#ifdef ABILITY_COMMAND_FOR_TEST
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_ABILITY)] =
        &AbilityManagerStub::BlockAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_AMS_SERVICE)] =
        &AbilityManagerStub::BlockAmsServiceInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::BLOCK_APP_SERVICE)] =
        &AbilityManagerStub::BlockAppServiceInner;
#endif
}

void AbilityManagerStubBuilder::ThirdStepInit(sptr<AbilityManagerStub> &stub)
{
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_USER_TEST)] =
        &AbilityManagerStub::StartUserTestInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::FINISH_USER_TEST)] =
        &AbilityManagerStub::FinishUserTestInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY_TOKEN)] =
        &AbilityManagerStub::GetTopAbilityTokenInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CHECK_UI_EXTENSION_IS_FOCUSED)] =
        &AbilityManagerStub::CheckUIExtensionIsFocusedInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_FOREGROUND)] =
        &AbilityManagerStub::DelegatorDoAbilityForegroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DELEGATOR_DO_ABILITY_BACKGROUND)] =
        &AbilityManagerStub::DelegatorDoAbilityBackgroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_FOREGROUND)] =
        &AbilityManagerStub::DoAbilityForegroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DO_ABILITY_BACKGROUND)] =
        &AbilityManagerStub::DoAbilityBackgroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_MISSION_ID_BY_ABILITY_TOKEN)] =
        &AbilityManagerStub::GetMissionIdByTokenInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_TOP_ABILITY)] =
        &AbilityManagerStub::GetTopAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ELEMENT_NAME_BY_TOKEN)] =
        &AbilityManagerStub::GetElementNameByTokenInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::DUMP_ABILITY_INFO_DONE)] =
        &AbilityManagerStub::DumpAbilityInfoDoneInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY)] =
        &AbilityManagerStub::StartExtensionAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::STOP_EXTENSION_ABILITY)] =
        &AbilityManagerStub::StopExtensionAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_MISSION_SNAPSHOT_FROM_WMS)] =
        &AbilityManagerStub::UpdateMissionSnapShotFromWMSInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_CONNECTION_OBSERVER)] =
        &AbilityManagerStub::RegisterConnectionObserverInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UNREGISTER_CONNECTION_OBSERVER)] =
        &AbilityManagerStub::UnregisterConnectionObserverInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DLP_CONNECTION_INFOS)] =
        &AbilityManagerStub::GetDlpConnectionInfosInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_ABILITY_TO_BACKGROUND)] =
        &AbilityManagerStub::MoveAbilityToBackgroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MOVE_UI_ABILITY_TO_BACKGROUND)] =
        &AbilityManagerStub::MoveUIAbilityToBackgroundInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_CONTINUE_STATE)] =
        &AbilityManagerStub::SetMissionContinueStateInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY_BY_SCB)] =
        &AbilityManagerStub::PrepareTerminateAbilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUESET_MODAL_UIEXTENSION)] =
        &AbilityManagerStub::RequestModalUIExtensionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_UI_EXTENSION_ROOT_HOST_INFO)] =
        &AbilityManagerStub::GetUIExtensionRootHostInfoInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::PRELOAD_UIEXTENSION_ABILITY)] =
        &AbilityManagerStub::PreloadUIExtensionAbilityInner;
#ifdef SUPPORT_GRAPHICS
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_LABEL)] =
        &AbilityManagerStub::SetMissionLabelInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_MISSION_ICON)] =
        &AbilityManagerStub::SetMissionIconInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_WMS_HANDLER)] =
        &AbilityManagerStub::RegisterWindowManagerServiceHandlerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETEFIRSTFRAMEDRAWING)] =
        &AbilityManagerStub::CompleteFirstFrameDrawingInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::StartUIExtensionAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::MinimizeUIExtensionAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::TerminateUIExtensionAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CONNECT_UI_EXTENSION_ABILITY)] =
        &AbilityManagerStub::ConnectUIExtensionAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::PREPARE_TERMINATE_ABILITY)] =
        &AbilityManagerStub::PrepareTerminateAbilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_DIALOG_SESSION_INFO)] =
        &AbilityManagerStub::GetDialogSessionInfoInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SEND_DIALOG_RESULT)] =
        &AbilityManagerStub::SendDialogResultInner;
    stub->requestFuncMap_[static_cast<uint32_t>(
        AbilityManagerInterfaceCode::REGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER)]
        = &AbilityManagerStub::RegisterAbilityFirstFrameStateObserverInner;
    stub->requestFuncMap_[static_cast<uint32_t>(
        AbilityManagerInterfaceCode::UNREGISTER_ABILITY_FIRST_FRAME_STATE_OBSERVER)]
        = &AbilityManagerStub::UnregisterAbilityFirstFrameStateObserverInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::COMPLETE_FIRST_FRAME_DRAWING_BY_SCB)] =
        &AbilityManagerStub::CompleteFirstFrameDrawingBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY_NON_MODAL)] =
        &AbilityManagerStub::StartUIExtensionAbilityNonModalInner;
#endif
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_DIALOG_SERVICE)] =
        &AbilityManagerStub::HandleRequestDialogService;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REPORT_DRAWN_COMPLETED)] =
        &AbilityManagerStub::HandleReportDrawnCompleted;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_MISSION_VAILD)] =
        &AbilityManagerStub::IsValidMissionIdsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::VERIFY_PERMISSION)] =
        &AbilityManagerStub::VerifyPermissionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_ABILITY_BY_SCB)] =
        &AbilityManagerStub::StartUIAbilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_ROOT_SCENE_SESSION)] =
        &AbilityManagerStub::SetRootSceneSessionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CALL_ABILITY_BY_SCB)] =
        &AbilityManagerStub::CallUIAbilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SPECIFIED_ABILITY_BY_SCB)] =
        &AbilityManagerStub::StartSpecifiedAbilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_SAVE_AS_RESULT)] =
        &AbilityManagerStub::NotifySaveAsResultInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_SESSIONMANAGERSERVICE)] =
        &AbilityManagerStub::SetSessionManagerServiceInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::UPDATE_SESSION_INFO)] =
        &AbilityManagerStub::UpdateSessionInfoBySCBInner;
}

void AbilityManagerStubBuilder::FourthStepInit(sptr<AbilityManagerStub> &stub)
{
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_STATUS_BAR_DELEGATE)] =
        &AbilityManagerStub::RegisterStatusBarDelegateInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::KILL_PROCESS_WITH_PREPARE_TERMINATE)] =
        &AbilityManagerStub::KillProcessWithPrepareTerminateInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK)] =
        &AbilityManagerStub::RegisterAutoStartupSystemCallbackInner;
    stub->requestFuncMap_[static_cast<uint32_t>(
        AbilityManagerInterfaceCode::UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK)] =
        &AbilityManagerStub::UnregisterAutoStartupSystemCallbackInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP)] =
        &AbilityManagerStub::SetApplicationAutoStartupInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP)] =
        &AbilityManagerStub::CancelApplicationAutoStartupInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_ALL_AUTO_STARTUP_APPLICATION)] =
        &AbilityManagerStub::QueryAllAutoStartupApplicationsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_CONNECTION_DATA)] =
        &AbilityManagerStub::GetConnectionDataInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_APPLICATION_AUTO_STARTUP_BY_EDM)] =
        &AbilityManagerStub::SetApplicationAutoStartupByEDMInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM)] =
        &AbilityManagerStub::CancelApplicationAutoStartupByEDMInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER)] =
        &AbilityManagerStub::StartAbilityForResultAsCallerInner;
    stub->requestFuncMap_[static_cast<uint32_t>(
        AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER_FOR_OPTIONS)] =
        &AbilityManagerStub::StartAbilityForResultAsCallerForOptionsInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_FOREGROUND_UI_ABILITIES)] =
        &AbilityManagerStub::GetForegroundUIAbilitiesInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::RESTART_APP)] =
        &AbilityManagerStub::RestartAppInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::OPEN_ATOMIC_SERVICE)] =
        &AbilityManagerStub::OpenAtomicServiceInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::IS_EMBEDDED_OPEN_ALLOWED)] =
        &AbilityManagerStub::IsEmbeddedOpenAllowedInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::REQUEST_ASSERT_FAULT_DIALOG)] =
        &AbilityManagerStub::RequestAssertFaultDialogInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_DEBUG_ASSERT_RESULT)] =
        &AbilityManagerStub::NotifyDebugAssertResultInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_ABILITY_VISIBILITY)] =
        &AbilityManagerStub::ChangeAbilityVisibilityInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::CHANGE_UI_ABILITY_VISIBILITY_BY_SCB)] =
        &AbilityManagerStub::ChangeUIAbilityVisibilityBySCBInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::START_SHORTCUT)] =
        &AbilityManagerStub::StartShortcutInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::SET_RESIDENT_PROCESS_ENABLE)] =
        &AbilityManagerStub::SetResidentProcessEnableInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::GET_ABILITY_STATE_BY_PERSISTENT_ID)] =
        &AbilityManagerStub::GetAbilityStateByPersistentIdInner;
}

void AbilityManagerStubBuilder::FifthStepInit(sptr<AbilityManagerStub> &stub)
{
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::TRANSFER_ABILITY_RESULT)] =
        &AbilityManagerStub::TransferAbilityResultForExtensionInner;
    stub->requestFuncMap_[static_cast<uint32_t>(AbilityManagerInterfaceCode::NOTIFY_FROZEN_PROCESS_BY_RSS)] =
        &AbilityManagerStub::NotifyFrozenProcessByRSSInner;
}
}  // namespace AAFwk
}  // namespace OHOS