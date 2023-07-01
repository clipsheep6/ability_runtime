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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_IPC_INTERFACE_CODE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_IPC_INTERFACE_CODE_H

/* SAID:180 */
namespace OHOS {
namespace AAFwk {
enum class AbilityManagerInterfaceCode {
    // ipc id 1-1000 for kit
    // ipc id for terminating ability (1)
    TERMINATE_ABILITY = 1,

    // ipc id for attaching ability thread (2)
    ATTACH_ABILITY_THREAD,

    // ipc id for ability transition done (3)
    ABILITY_TRANSITION_DONE,

    // ipc id for connecting ability done (4)
    CONNECT_ABILITY_DONE,

    // ipc id for disconnecting ability done (5)
    DISCONNECT_ABILITY_DONE,

    // ipc id for add window token (6)
    ADD_WINDOW_INFO,

    // ipc id for terminating ability for result (7)
    TERMINATE_ABILITY_RESULT,

    // ipc id for list stack info (8)
    LIST_STACK_INFO,

    // ipc id for get recent mission (9)
    GET_RECENT_MISSION,

    // ipc id for removing mission (10)
    REMOVE_MISSION,

    // ipc id for removing mission (11)
    REMOVE_STACK,

    // ipc id for removing mission (12)
    COMMAND_ABILITY_DONE,

    // ipc id for get mission snapshot (13)
    GET_MISSION_SNAPSHOT,

    // ipc id for acquire data ability (14)
    ACQUIRE_DATA_ABILITY,

    // ipc id for release data ability (15)
    RELEASE_DATA_ABILITY,

    // ipc id for move mission to top (16)
    MOVE_MISSION_TO_TOP,

    // ipc id for kill process (17)
    KILL_PROCESS,

    // ipc id for uninstall app (18)
    UNINSTALL_APP,

    // ipc id for terminate ability by callerToken and request code (19)
    TERMINATE_ABILITY_BY_CALLER,

    // ipc id for move mission to floating stack (20)
    MOVE_MISSION_TO_FLOATING_STACK,

    // ipc id for move mission to floating stack (21)
    MOVE_MISSION_TO_SPLITSCREEN_STACK,

    // ipc id for change focus ability (22)
    CHANGE_FOCUS_ABILITY,

    // ipc id for Minimize MultiWindow (23)
    MINIMIZE_MULTI_WINDOW,

    // ipc id for Maximize MultiWindow (24)
    MAXIMIZE_MULTI_WINDOW,

    // ipc id for get floating missions (25)
    GET_FLOATING_MISSIONS,

    // ipc id for get floating missions (26)
    CLOSE_MULTI_WINDOW,

    // ipc id for move mission to end (29)
    MOVE_MISSION_TO_END,

    // ipc id for compel verify permission (30)
    COMPEL_VERIFY_PERMISSION,

    // ipc id for power off (31)
    POWER_OFF,

    // ipc id for power off (32)
    POWER_ON,

    // ipc id for luck mission (33)
    LUCK_MISSION,

    // ipc id for unluck mission (34)
    UNLUCK_MISSION,

    // ipc id for set mission info (35)
    SET_MISSION_INFO,

    // ipc id for get mission lock mode state (36)
    GET_MISSION_LOCK_MODE_STATE,

    // ipc id for minimize ability (38)
    MINIMIZE_ABILITY,

    // ipc id for lock mission for cleanup operation (39)
    LOCK_MISSION_FOR_CLEANUP,

    // ipc id for unlock mission for cleanup operation (40)
    UNLOCK_MISSION_FOR_CLEANUP,

    // ipc id for register mission listener (41)
    REGISTER_MISSION_LISTENER,

    // ipc id for unregister mission listener (42)
    UNREGISTER_MISSION_LISTENER,

    // ipc id for get mission infos (43)
    GET_MISSION_INFOS,

    // ipc id for get mission info by id (44)
    GET_MISSION_INFO_BY_ID,

    // ipc id for clean mission (45)
    CLEAN_MISSION,

    // ipc id for clean all missions (46)
    CLEAN_ALL_MISSIONS,

    // ipc id for move mission to front (47)
    MOVE_MISSION_TO_FRONT,

    // ipc id for get mission snap shot (48)
    GET_MISSION_SNAPSHOT_BY_ID,

    // ipc id for move mission to front (49)
    START_USER,

    // ipc id for move mission to front (50)
    STOP_USER,

    // ipc id for set ability controller (51)
    SET_ABILITY_CONTROLLER,

    // ipc id for get stability test flag (52)
    IS_USER_A_STABILITY_TEST,

    // ipc id for set mission label (53)
    SET_MISSION_LABEL,

    // ipc id for ability foreground (54)
    DO_ABILITY_FOREGROUND,

    // ipc id for ability background (55)
    DO_ABILITY_BACKGROUND,

    // ipc id for move mission to front by options (56)
    MOVE_MISSION_TO_FRONT_BY_OPTIONS,

    // ipc for get mission id by ability token (57)
    GET_MISSION_ID_BY_ABILITY_TOKEN,

    // ipc id for set mission icon (58)
    SET_MISSION_ICON,

    // dump ability info done (59)
    DUMP_ABILITY_INFO_DONE,

    // start extension ability (60)
    START_EXTENSION_ABILITY,

    // stop extension ability (61)
    STOP_EXTENSION_ABILITY,

    SET_COMPONENT_INTERCEPTION,

    SEND_ABILITY_RESULT_BY_TOKEN,

    // ipc id for set rootSceneSession (64)
    SET_ROOT_SCENE_SESSION,

    // prepare terminate ability (65)
    PREPARE_TERMINATE_ABILITY,

    COMMAND_ABILITY_WINDOW_DONE,

    // prepare terminate ability (67)
    CALL_ABILITY_BY_SCB,

    MOVE_ABILITY_TO_BACKGROUND,

    // ipc id 1001-2000 for DMS
    // ipc id for starting ability (1001)
    START_ABILITY = 1001,

    // ipc id for connecting ability (1002)
    CONNECT_ABILITY,

    // ipc id for disconnecting ability (1003)
    DISCONNECT_ABILITY,

    // ipc id for disconnecting ability (1004)
    STOP_SERVICE_ABILITY,

    // ipc id for starting ability by caller(1005)
    START_ABILITY_ADD_CALLER,

    GET_PENDING_WANT_SENDER,

    SEND_PENDING_WANT_SENDER,

    CANCEL_PENDING_WANT_SENDER,

    GET_PENDING_WANT_UID,

    GET_PENDING_WANT_BUNDLENAME,

    GET_PENDING_WANT_USERID,

    GET_PENDING_WANT_TYPE,

    GET_PENDING_WANT_CODE,

    REGISTER_CANCEL_LISTENER,

    UNREGISTER_CANCEL_LISTENER,

    GET_PENDING_REQUEST_WANT,

    GET_PENDING_WANT_SENDER_INFO,
    SET_SHOW_ON_LOCK_SCREEN,

    SEND_APP_NOT_RESPONSE_PROCESS_ID,

    // ipc id for starting ability by settings(1018)
    START_ABILITY_FOR_SETTINGS,

    GET_ABILITY_MISSION_SNAPSHOT,

    GET_APP_MEMORY_SIZE,

    IS_RAM_CONSTRAINED_DEVICE,

    GET_ABILITY_RUNNING_INFO,

    GET_EXTENSION_RUNNING_INFO,

    GET_PROCESS_RUNNING_INFO,

    CLEAR_UP_APPLICATION_DATA,

    START_ABILITY_FOR_OPTIONS,

    BLOCK_AMS_SERVICE,

    BLOCK_ABILITY,

    BLOCK_APP_SERVICE,

    // ipc id for call ability
    START_CALL_ABILITY,

    RELEASE_CALL_ABILITY,

    CONNECT_ABILITY_WITH_TYPE,

    // start ui extension ability
    START_UI_EXTENSION_ABILITY,

    CALL_REQUEST_DONE,

    START_ABILITY_AS_CALLER_BY_TOKEN,

    START_ABILITY_AS_CALLER_FOR_OPTIONS,

    // ipc id for minimize ui extension ability
    MINIMIZE_UI_EXTENSION_ABILITY,

    // ipc id for terminating ui extension ability
    TERMINATE_UI_EXTENSION_ABILITY,

    // ipc id for connect ui extension ability
    CONNECT_UI_EXTENSION_ABILITY,

    START_UI_ABILITY_BY_SCB,

    // ipc id for minimize ui ability by scb
    MINIMIZE_UI_ABILITY_BY_SCB,

    // ipc id for close ui ability by scb
    CLOSE_UI_ABILITY_BY_SCB,

    // ipc id for request dialog service
    REQUEST_DIALOG_SERVICE,

    // ipc id for start specified ability by scb
    START_SPECIFIED_ABILITY_BY_SCB,

    // ipc id for set sessionManagerService
    SET_SESSIONMANAGERSERVICE,

    // ipc id for report drawn completed
    REPORT_DRAWN_COMPLETED,

    GET_SESSIONMANAGERSERVICE,

    // ipc id for continue ability(1101)
    START_CONTINUATION = 1101,

    NOTIFY_CONTINUATION_RESULT = 1102,

    NOTIFY_COMPLETE_CONTINUATION = 1103,

    CONTINUE_ABILITY = 1104,

    CONTINUE_MISSION = 1105,

    SEND_RESULT_TO_ABILITY = 1106,

    REGISTER_REMOTE_ON_LISTENER = 1107,

    REGISTER_REMOTE_OFF_LISTENER = 1108,

    CONTINUE_MISSION_OF_BUNDLENAME = 1109,

    // ipc id for mission manager(1110)
    REGISTER_REMOTE_MISSION_LISTENER = 1110,
    UNREGISTER_REMOTE_MISSION_LISTENER = 1111,
    START_SYNC_MISSIONS = 1112,
    STOP_SYNC_MISSIONS = 1113,
    REGISTER_SNAPSHOT_HANDLER = 1114,
    GET_MISSION_SNAPSHOT_INFO = 1115,
    UPDATE_MISSION_SNAPSHOT = 1116,
    MOVE_MISSIONS_TO_FOREGROUND = 1117,
    MOVE_MISSIONS_TO_BACKGROUND = 1118,
    UPDATE_MISSION_SNAPSHOT_FROM_WMS,

    // ipc id for user test(1120)
    START_USER_TEST = 1120,
    FINISH_USER_TEST = 1121,
    DELEGATOR_DO_ABILITY_FOREGROUND = 1122,
    DELEGATOR_DO_ABILITY_BACKGROUND = 1123,
    GET_TOP_ABILITY_TOKEN         = 1124,

    // ipc id 2001-3000 for tools
    // ipc id for dumping state (2001)
    DUMP_STATE = 2001,
    DUMPSYS_STATE = 2002,
    FORCE_TIMEOUT,

    REGISTER_WMS_HANDLER = 2500,
    COMPLETEFIRSTFRAMEDRAWING = 2501,
    REGISTER_CONNECTION_OBSERVER = 2502,
    UNREGISTER_CONNECTION_OBSERVER = 2503,
    GET_DLP_CONNECTION_INFOS = 2504,

    GET_TOP_ABILITY = 3000,
    FREE_INSTALL_ABILITY_FROM_REMOTE = 3001,
    ADD_FREE_INSTALL_OBSERVER = 3002,

    // ipc id for app recovery(3010)
    ABILITY_RECOVERY = 3010,
    ABILITY_RECOVERY_ENABLE = 3011,

    QUERY_MISSION_VAILD = 3012,

    VERIFY_PERMISSION = 3013,

    ACQUIRE_SHARE_DATA = 4001,
    SHARE_DATA_DONE = 4002,

    GET_ABILITY_TOKEN = 5001,

    FORCE_EXIT_APP = 6001,
    RECORD_APP_EXIT_REASON = 6002
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_IPC_INTERFACE_CODE_H
