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
    ATTACH_ABILITY_THREAD = 2,

    // ipc id for ability transition done (3)
    ABILITY_TRANSITION_DONE = 3,

    // ipc id for connecting ability done (4)
    CONNECT_ABILITY_DONE = 4,

    // ipc id for disconnecting ability done (5)
    DISCONNECT_ABILITY_DONE = 5,

    // ipc id for add window token (6)
    ADD_WINDOW_INFO = 6,

    // ipc id for list stack info (8)
    LIST_STACK_INFO = 8,

    // ipc id for get recent mission (9)
    GET_RECENT_MISSION = 9,

    // ipc id for removing mission (10)
    REMOVE_MISSION = 10,

    // ipc id for removing mission (11)
    REMOVE_STACK = 11,

    // ipc id for removing mission (12)
    COMMAND_ABILITY_DONE = 12,

    // ipc id for get mission snapshot (13)
    GET_MISSION_SNAPSHOT = 13,

    // ipc id for acquire data ability (14)
    ACQUIRE_DATA_ABILITY = 14,

    // ipc id for release data ability (15)
    RELEASE_DATA_ABILITY = 15,

    // ipc id for move mission to top (16)
    MOVE_MISSION_TO_TOP = 16,

    // ipc id for kill process (17)
    KILL_PROCESS = 17,

    // ipc id for uninstall app (18)
    UNINSTALL_APP = 18,

    // ipc id for move mission to floating stack (20)
    MOVE_MISSION_TO_FLOATING_STACK = 20,

    // ipc id for move mission to floating stack (21)
    MOVE_MISSION_TO_SPLITSCREEN_STACK = 21,

    // ipc id for change focus ability (22)
    CHANGE_FOCUS_ABILITY = 22,

    // ipc id for Minimize MultiWindow (23)
    MINIMIZE_MULTI_WINDOW = 23,

    // ipc id for Maximize MultiWindow (24)
    MAXIMIZE_MULTI_WINDOW = 24,

    // ipc id for get floating missions (25)
    GET_FLOATING_MISSIONS = 25,

    // ipc id for get floating missions (26)
    CLOSE_MULTI_WINDOW = 26,

    // ipc id for move mission to end (27)
    MOVE_MISSION_TO_END = 27,

    // ipc id for compel verify permission (28)
    COMPEL_VERIFY_PERMISSION = 28,

    // ipc id for power off (29)
    POWER_OFF = 29,

    // ipc id for power off (30)
    POWER_ON = 30,

    // ipc id for luck mission (31)
    LUCK_MISSION = 31,

    // ipc id for unluck mission (32)
    UNLUCK_MISSION = 32,

    // ipc id for set mission info (33)
    SET_MISSION_INFO = 33,

    // ipc id for get mission lock mode state (34)
    GET_MISSION_LOCK_MODE_STATE = 34,

    // ipc id for minimize ability (35)
    MINIMIZE_ABILITY = 35,

    // ipc id for lock mission for cleanup operation (36)
    LOCK_MISSION_FOR_CLEANUP = 36,

    // ipc id for unlock mission for cleanup operation (37)
    UNLOCK_MISSION_FOR_CLEANUP = 37,

    // ipc id for register mission listener (38)
    REGISTER_MISSION_LISTENER = 38,

    // ipc id for unregister mission listener (39)
    UNREGISTER_MISSION_LISTENER = 39,

    // ipc id for get mission infos (40)
    GET_MISSION_INFOS = 40,

    // ipc id for get mission info by id (41)
    GET_MISSION_INFO_BY_ID = 41,

    // ipc id for clean mission (42)
    CLEAN_MISSION = 42,

    // ipc id for clean all missions (43)
    CLEAN_ALL_MISSIONS = 43,

    // ipc id for move mission to front (44)
    MOVE_MISSION_TO_FRONT = 44,

    // ipc id for get mission snap shot (45)
    GET_MISSION_SNAPSHOT_BY_ID = 45,

    // ipc id for move mission to front (46)
    START_USER = 46,

    // ipc id for move mission to front (47)
    STOP_USER = 47,

    // ipc id for set ability controller (48)
    SET_ABILITY_CONTROLLER = 48,

    // ipc id for get stability test flag (49)
    IS_USER_A_STABILITY_TEST = 49,

    // ipc id for set mission label (50)
    SET_MISSION_LABEL = 50,

    // ipc id for ability foreground (51)
    DO_ABILITY_FOREGROUND = 51,

    // ipc id for ability background (52)
    DO_ABILITY_BACKGROUND = 52,

    // ipc id for move mission to front by options (53)
    MOVE_MISSION_TO_FRONT_BY_OPTIONS = 53,

    // ipc for get mission id by ability token (54)
    GET_MISSION_ID_BY_ABILITY_TOKEN = 54,

    // ipc id for set mission icon (55)
    SET_MISSION_ICON = 55,

    // dump ability info done (56)
    DUMP_ABILITY_INFO_DONE = 56,

    // start extension ability (57)
    START_EXTENSION_ABILITY = 57,

    // stop extension ability (58)
    STOP_EXTENSION_ABILITY = 58,

    SET_COMPONENT_INTERCEPTION = 59,

    SEND_ABILITY_RESULT_BY_TOKEN = 60,

    // ipc id for set rootSceneSession (61)
    SET_ROOT_SCENE_SESSION = 61,

    // prepare terminate ability (62)
    PREPARE_TERMINATE_ABILITY = 62,

    COMMAND_ABILITY_WINDOW_DONE = 63,

    // prepare terminate ability (64)
    CALL_ABILITY_BY_SCB = 64,

    MOVE_ABILITY_TO_BACKGROUND = 65,

    // ipc id for set mission continue state (66)
    SET_MISSION_CONTINUE_STATE = 66,

    // ipc id for set session locked state (67)
    SET_SESSION_LOCKED_STATE = 67,

    // Register the app debug mode listener (68)
    REGISTER_APP_DEBUG_LISTENER = 68,

    // Cancel register the app debug mode listener (69)
    UNREGISTER_APP_DEBUG_LISTENER = 69,

    // Attach app debug (70)
    ATTACH_APP_DEBUG = 70,

    // Deatch app debug (71)
    DETACH_APP_DEBUG = 71,

    // Execute intent (72)
    EXECUTE_INTENT = 72,

    // execute insight intent done with result (73)
    EXECUTE_INSIGHT_INTENT_DONE = 73,

    // ipc id for logout user (74)
    LOGOUT_USER = 74,
	
    // Get forgeround UI abilities(75)
    GET_FOREGROUND_UI_ABILITIES = 75,

    // ipc id 1001-2000 for DMS
    // ipc id for starting ability (1001)
    START_ABILITY = 1001,

    // ipc id for connecting ability (1002)
    CONNECT_ABILITY = 1002,

    // ipc id for disconnecting ability (1003)
    DISCONNECT_ABILITY = 1003,

    // ipc id for disconnecting ability (1004)
    STOP_SERVICE_ABILITY = 1004,

    // ipc id for starting ability by caller(1005)
    START_ABILITY_ADD_CALLER = 1005,

    GET_PENDING_WANT_SENDER = 1006,

    SEND_PENDING_WANT_SENDER = 1007,

    CANCEL_PENDING_WANT_SENDER = 1008,

    GET_PENDING_WANT_UID = 1009,

    GET_PENDING_WANT_BUNDLENAME = 1010,

    GET_PENDING_WANT_USERID = 1011,

    GET_PENDING_WANT_TYPE = 1012,

    GET_PENDING_WANT_CODE = 1013,

    REGISTER_CANCEL_LISTENER = 1014,

    UNREGISTER_CANCEL_LISTENER = 1015,

    GET_PENDING_REQUEST_WANT = 1016,

    GET_PENDING_WANT_SENDER_INFO = 1017,
    SET_SHOW_ON_LOCK_SCREEN = 1018,

    SEND_APP_NOT_RESPONSE_PROCESS_ID = 1019,

    // ipc id for starting ability by settings(1020)
    START_ABILITY_FOR_SETTINGS = 1020,

    GET_ABILITY_MISSION_SNAPSHOT = 1021,

    GET_APP_MEMORY_SIZE = 1022,

    IS_RAM_CONSTRAINED_DEVICE = 1023,

    GET_ABILITY_RUNNING_INFO = 1024,

    GET_EXTENSION_RUNNING_INFO = 1025,

    GET_PROCESS_RUNNING_INFO = 1026,

    CLEAR_UP_APPLICATION_DATA = 1027,

    START_ABILITY_FOR_OPTIONS = 1028,

    BLOCK_AMS_SERVICE = 1029,

    BLOCK_ABILITY = 1030,

    BLOCK_APP_SERVICE = 1031,

    // ipc id for call ability
    START_CALL_ABILITY = 1032,

    RELEASE_CALL_ABILITY = 1033,

    CONNECT_ABILITY_WITH_TYPE = 1034,

    // start ui extension ability
    START_UI_EXTENSION_ABILITY = 1035,

    CALL_REQUEST_DONE = 1036,

    START_ABILITY_AS_CALLER_BY_TOKEN = 1037,

    START_ABILITY_AS_CALLER_FOR_OPTIONS = 1038,

    // ipc id for minimize ui extension ability
    MINIMIZE_UI_EXTENSION_ABILITY = 1039,

    // ipc id for terminating ui extension ability
    TERMINATE_UI_EXTENSION_ABILITY = 1040,

    // ipc id for connect ui extension ability
    CONNECT_UI_EXTENSION_ABILITY = 1041,

    CHECK_UI_EXTENSION_IS_FOCUSED = 1042,

    START_UI_ABILITY_BY_SCB = 1043,

    // ipc id for minimize ui ability by scb
    MINIMIZE_UI_ABILITY_BY_SCB = 1044,

    // ipc id for close ui ability by scb
    CLOSE_UI_ABILITY_BY_SCB = 1045,

    // ipc id for request dialog service
    REQUEST_DIALOG_SERVICE = 1046,

    // ipc id for start specified ability by scb
    START_SPECIFIED_ABILITY_BY_SCB = 1047,

    // ipc id for set sessionManagerService
    SET_SESSIONMANAGERSERVICE = 1048,

    // ipc id for report drawn completed
    REPORT_DRAWN_COMPLETED = 1049,

    // ipc id for prepare to terminate ability by scb
    PREPARE_TERMINATE_ABILITY_BY_SCB = 1050,

    // start ui session ability
    START_UI_SESSION_ABILITY_ADD_CALLER = 1051,

    START_UI_SESSION_ABILITY_FOR_OPTIONS = 1052,

    // start ability by insigt intent
    START_ABILITY_BY_INSIGHT_INTENT = 1053,

    // get dialog session info
    GET_DIALOG_SESSION_INFO = 1054,

    // send dialog result
    SEND_DIALOG_RESULT = 1055,

    // request modal UIExtension by want
    REQUESET_MODAL_UIEXTENSION = 1056,

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
    GET_CONNECTION_DATA = 2505,

    GET_TOP_ABILITY = 3000,
    FREE_INSTALL_ABILITY_FROM_REMOTE = 3001,
    ADD_FREE_INSTALL_OBSERVER = 3002,
    GET_ELEMENT_NAME_BY_TOKEN = 3003,

    // ipc id for app recovery(3010)
    ABILITY_RECOVERY = 3010,
    ABILITY_RECOVERY_ENABLE = 3011,

    QUERY_MISSION_VAILD = 3012,

    VERIFY_PERMISSION = 3013,

    ACQUIRE_SHARE_DATA = 4001,
    SHARE_DATA_DONE = 4002,

    // ipc id for notify as result (notify to snadbox app)
    NOTIFY_SAVE_AS_RESULT = 4201,

    // ipc id for collborator
    REGISTER_COLLABORATOR = 4050,
    UNREGISTER_COLLABORATOR = 4051,

    IS_ABILITY_CONTROLLER_START = 4054,
    OPEN_FILE = 4055,

    GET_ABILITY_TOKEN = 5001,

    FORCE_EXIT_APP = 6001,
    RECORD_APP_EXIT_REASON = 6002,

    // ipc id for register auto startup system callback
    REGISTER_AUTO_STARTUP_SYSTEM_CALLBACK = 6101,
    // ipc id for unregister auto startup system callback
    UNREGISTER_AUTO_STARTUP_SYSTEM_CALLBACK = 6102,
    // ipc id for set application auto startup
    SET_APPLICATION_AUTO_STARTUP = 6103,
    // ipc id for cancel application auto startup
    CANCEL_APPLICATION_AUTO_STARTUP = 6104,
    // ipc id for auery all auto startup application
    QUERY_ALL_AUTO_STARTUP_APPLICATION = 6105,

    // ipc id for register auto startup callback
    REGISTER_AUTO_STARTUP_CALLBACK = 6106,
    // ipc id for unregister auto startup callback
    UNREGISTER_AUTO_STARTUP_CALLBACK = 6107,
    // ipc id for set auto startup
    SET_AUTO_STARTUP = 6108,
    // ipc id for cancel auto startup
    CANCEL_AUTO_STARTUP = 6109,
    // ipc id for is auto startup
    IS_AUTO_STARTUP = 6110,

    // ipc id for on auto starup on
    ON_AUTO_STARTUP_ON = 6111,
    // ipc id for on auto starup off
    ON_AUTO_STARTUP_OFF = 6112,

    // ipc id for register session handler
    REGISTER_SESSION_HANDLER = 6010,
    // ipc id for update session info
    UPDATE_SESSION_INFO = 6011,

    // ipc id for set application auto startup by EDM
    SET_APPLICATION_AUTO_STARTUP_BY_EDM = 6113,
    // ipc id for cancel application auto startup by EDM
    CANCEL_APPLICATION_AUTO_STARTUP_BY_EDM = 6114,
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_IPC_INTERFACE_CODE_H
