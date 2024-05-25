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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_IPC_INTERFACE_CODE_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_IPC_INTERFACE_CODE_H

/* SAID:501 */
namespace OHOS {
namespace AppExecFwk {
enum class AppMgrInterfaceCode {
    // please add new code to the bottom in order to prevent some unexpected BUG
    APP_ATTACH_APPLICATION = 0,
    APP_APPLICATION_FOREGROUNDED = 1,
    APP_APPLICATION_BACKGROUNDED = 2,
    APP_APPLICATION_TERMINATED = 3,
    APP_ABILITY_CLEANED = 5,
    APP_GET_MGR_INSTANCE = 6,
    APP_CLEAR_UP_APPLICATION_DATA = 7,
    APP_GET_ALL_RUNNING_PROCESSES = 8,
    APP_GET_RUNNING_PROCESSES_BY_USER_ID = 9,
    APP_ADD_ABILITY_STAGE_INFO_DONE = 10,
    STARTUP_RESIDENT_PROCESS = 11,
    REGISTER_APPLICATION_STATE_OBSERVER = 12,
    UNREGISTER_APPLICATION_STATE_OBSERVER = 13,
    GET_FOREGROUND_APPLICATIONS = 14,
    START_USER_TEST_PROCESS = 15,
    FINISH_USER_TEST = 16,
    SCHEDULE_ACCEPT_WANT_DONE = 17,
    BLOCK_APP_SERVICE = 18,
    APP_GET_ABILITY_RECORDS_BY_PROCESS_ID = 19,
    START_RENDER_PROCESS = 20,
    ATTACH_RENDER_PROCESS = 21,
    GET_RENDER_PROCESS_TERMINATION_STATUS = 22,
    GET_CONFIGURATION = 23,
    UPDATE_CONFIGURATION = 24,
    REGISTER_CONFIGURATION_OBSERVER = 25,
    UNREGISTER_CONFIGURATION_OBSERVER = 26,
    APP_NOTIFY_MEMORY_LEVEL = 27,
    GET_APP_RUNNING_STATE = 28,
    NOTIFY_LOAD_REPAIR_PATCH = 29,
    NOTIFY_HOT_RELOAD_PAGE = 30,
    SET_CONTINUOUSTASK_PROCESS = 31,
    NOTIFY_UNLOAD_REPAIR_PATCH = 32,
    PRE_START_NWEBSPAWN_PROCESS = 33,
    APP_GET_PROCESS_RUNNING_INFORMATION = 34,
    IS_SHARED_BUNDLE_RUNNING = 35,
    DUMP_HEAP_MEMORY_PROCESS = 36,
    START_NATIVE_PROCESS_FOR_DEBUGGER = 37,
    NOTIFY_APP_FAULT = 38,
    NOTIFY_APP_FAULT_BY_SA = 39,
    JUDGE_SANDBOX_BY_PID = 40,
    GET_BUNDLE_NAME_BY_PID = 41,
    APP_GET_ALL_RENDER_PROCESSES = 42,
    GET_PROCESS_MEMORY_BY_PID = 43,
    GET_PIDS_BY_BUNDLENAME = 44,
    CHANGE_APP_GC_STATE = 45,
    REGISTER_APP_FOREGROUND_STATE_OBSERVER = 46,
    UNREGISTER_APP_FOREGROUND_STATE_OBSERVER = 47,
    NOTIFY_PAGE_SHOW = 48,
    NOTIFY_PAGE_HIDE = 49,
    // Register an application to start listening
    REGISTER_APP_RUNNING_STATUS_LISTENER = 50,
    // Unregister the app to start listening
    UNREGISTER_APP_RUNNING_STATUS_LISTENER = 51,
    SCHEDULE_NEW_PROCESS_REQUEST_DONE = 52,
    REGISTER_ABILITY_FOREGROUND_STATE_OBSERVER = 53,
    UNREGISTER_ABILITY_FOREGROUND_STATE_OBSERVER = 54,
    IS_APPLICATION_RUNNING = 55,
    START_CHILD_PROCESS = 56,
    GET_CHILD_PROCCESS_INFO_FOR_SELF = 57,
    ATTACH_CHILD_PROCESS = 58,
    EXIT_CHILD_PROCESS_SAFELY = 59,
    IS_FINAL_APP_PROCESS = 60,
    APP_CLEAR_UP_APPLICATION_DATA_BY_SELF = 61,

    REGISTER_RENDER_STATUS_OBSERVER = 62,
    UNREGISTER_RENDER_STATUS_OBSERVER = 63,
    UPDATE_RENDER_STATUS = 64,
    SIGN_RESTART_APP_FLAG = 65,
    DUMP_JSHEAP_MEMORY_PROCESS = 66,
    GET_APP_RUNNING_UNIQUE_ID_BY_PID = 67,
    APP_NOTIFY_PROC_MEMORY_LEVEL = 68,
    GET_ALL_UI_EXTENSION_ROOT_HOST_PID = 69,
    GET_ALL_UI_EXTENSION_PROVIDER_PID = 70,
    UPDATE_CONFIGURATION_BY_BUNDLE_NAME = 71,
    NOTIFY_MEMORY_SIZE_STATE_CHANGED = 72,
    PRELOAD_APPLICATION = 73,
    SET_SUPPORTED_PROCESS_CACHE_SELF = 74,
    APP_GET_RUNNING_PROCESSES_BY_BUNDLE_TYPE = 75,
    START_NATIVE_CHILD_PROCESS = 76,
    SAVE_BROWSER_CHANNEL = 77,
    GET_RUNNING_MULTIAPP_INFO_BY_BUNDLENAME = 78,
    SET_APP_ASSERT_PAUSE_STATE_SELF = 79,
    IS_APPCLONE_RUNNING = 80,
};
} // AppExecFwk
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_MGR_IPC_INTERFACE_CODE_H
