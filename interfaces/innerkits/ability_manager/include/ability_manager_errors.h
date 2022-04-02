/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_ABILITY_MANAGER_ERRORS_H
#define OHOS_AAFWK_ABILITY_MANAGER_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace AAFwk {
enum {
    /**
     *  Module type: Ability Manager Service side
     */
    ABILITY_MODULE_TYPE_SERVICE = 0,
    /**
     *  Module type: ABility Kit side
     */
    ABILITY_MODULE_TYPE_KIT = 1
};

// offset of aafwk error, only be used in this file.
constexpr ErrCode AAFWK_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_AAFWK, ABILITY_MODULE_TYPE_SERVICE);

enum {
    /**
     * Result(2097152) for StartAbility: An error of the Want could not be resolved
     * to ability info from BMS or DistributedMS.
     */
    RESOLVE_ABILITY_ERR = AAFWK_SERVICE_ERR_OFFSET,
    /**
     * Result(2097153) for Connect: An error of the get ability service.
     */
    GET_ABILITY_SERVICE_FAILED,
    /**
     * Result(2097154) for Connect State: An error of the the ability service not connect.
     */
    ABILITY_SERVICE_NOT_CONNECTED,
    /**
     * Result(2097155) for StartAbility: An error of the Want could not be resolved
     * to app info from BMS or DistributedMS.
     */
    RESOLVE_APP_ERR,
    /**
     * Result(2097156) for StartAbility: An error of getting ability stack manager object.
     */
    STACK_MANAGER_NOT_EXIST,
    /**
     * Result(2097157) for StartAbility: The ability to start is already at the top.
     */
    ABILITY_EXISTED,
    /**
     * Result(2097158) for StartAbility: An error to create mission stack.
     */
    CREATE_MISSION_STACK_FAILED,
    /**
     * Result(2097159) for StartAbility: An error to create mission record.
     */
    CREATE_MISSION_RECORD_FAILED,
    /**
     * Result(2097160) for StartAbility: An error to create ability record.
     */
    CREATE_ABILITY_RECORD_FAILED,
    /**
     * Result(2097161) for StartAbility: The ability to start is waiting.
     */
    START_ABILITY_WAITING,
    /**
     * Result(2097162) for TerminateAbility: Don't allow to terminate launcher.
     */
    TERMINATE_LAUNCHER_DENIED,
    /**
     * Result(2097163) for DisconnectAbility: Connection not exist.
     */
    CONNECTION_NOT_EXIST,
    /**
     * Result(2097164) for DisconnectAbility:Connection is invalid state.
     */
    INVALID_CONNECTION_STATE,
    /**
     * Result(2097165) for LoadctAbility:LoadAbility timeout.
     */
    LOAD_ABILITY_TIMEOUT,
    /**
     * Result(2097166) for DisconnectAbility:Connection timeout.
     */
    CONNECTION_TIMEOUT,
    /**
     * Result(2097167) for start service: An error of the get BundleManagerService.
     */
    GET_BUNDLE_MANAGER_SERVICE_FAILED,
    /**
     * Result(2097168) for Remove mission: target mission is not in default stack.
     */
    REMOVE_MISSION_ID_NOT_EXIST,
    /**
     * Result(2097169) for Remove mission: Don't allow to remove mission which has launcher ability.
     */
    REMOVE_MISSION_LAUNCHER_DENIED,
    /**
     * Result(2097170) for Remove mission: Don't allow to remove mission which has active ability.
     */
    REMOVE_MISSION_ACTIVE_DENIED,
    /**
     * Result(2097171) for Remove mission: An error of removing mission.
     */
    REMOVE_MISSION_FAILED,
    /**
     * Result(2097172) for All: An error occurs in server.
     */
    INNER_ERR,
    /**
     * Result(2097173) for Get recent mission: get recent missions failed
     */
    GET_RECENT_MISSIONS_FAILED,
    /**
     * Result(2097174) for Remove stack: Don't allow to remove stack which has launcher ability.
     */
    REMOVE_STACK_LAUNCHER_DENIED,
    /**
     * Result(2097175) for Remove stack: An error of removing stack.
     */
    REMOVE_STACK_FAILED,
    /**
     * Result(2097176) for Remove stack: mission stack is empty
     */
    MISSION_STACK_LIST_IS_EMPTY,
    /**
     * Result(2097177) for Remove stack: target stack is not exist.
     */
    REMOVE_STACK_ID_NOT_EXIST,
    /**
     * Result(2097178) for ConnectAbility:target ability is not service ability.
     */
    TARGET_ABILITY_NOT_SERVICE,
    /**
     * Result(2097179) for TerminateAbility:target service has a record of connect. It cannot be stopped.
     */
    TERMINATE_SERVICE_IS_CONNECTED,
    /**
     * Result(2097180) for StartAbility:The ability to start is already activing..
     */
    START_SERVICE_ABILITY_ACTIVING,
    /**
     * Result(2097181) for move mission to top: An error of moving stack.
     */
    MOVE_MISSION_FAILED,
    /**
     * Result(2097182) for kill process: An error of kill process.
     */
    KILL_PROCESS_FAILED,
    /**
     * Result(2097183) for uninstall app: An error of uninstall app.
     */
    UNINSTALL_APP_FAILED,
    /**
     * Result(2097184) for terminate ability result: An error of terminate service.
     */
    TERMINATE_ABILITY_RESULT_FAILED,
    /**
     * Result(2097185) for check permission failed.
     */
    CHECK_PERMISSION_FAILED,
    /**
     * Result(2097186) for waiting ability lifecycle complete.
     */
    POWER_OFF_WAITING,
    /**
     * Result(2097187) for power off failed.
     */
    POWER_OFF_FAILED,
    /**
     * Result(2097188) for power on failed.
     */
    POWER_ON_FAILED,
    /**
     * Result(2097189) for ability no first in mission.
     */
    NO_FIRST_IN_MISSION,

    /**
     * Result(2097190) for lock mission errors.
     */
    LOCK_MISSION_DENY_FAILED,

    /**
     * Result(2097191) for unlock mission errors.
     */
    UNLOCK_MISSION_DENY_FAILED,

    /**
     * Result(2097192) for set mission info errors.
     */
    SET_MISSION_INFO_FAILED,

    /**
     * Result(2097193) for system is lock mission state.
     */
    LOCK_MISSION_STATE_DENY_REQUEST,

    /**
     * Result(2097194) for move mission to stack: out of size of moving mission.
     */
    MOVE_MISSION_TO_STACK_OUT_OF_SIZE,

    /**
     * Result(2097195) for move mission to stack: moving missions are not same window mode.
     */
    MOVE_MISSION_TO_STACK_NOT_SAME_WIN_MODE,

    /**
     * Result(2097196) for move mission to stack: moving mission is not exist.
     */
    MOVE_MISSION_TO_STACK_NOT_EXIST_MISSION,

    /**
     * Result(2097197) for move mission to stack: target mission or ability can't support multi window display.
     */
    MOVE_MISSION_TO_STACK_NOT_SUPPORT_MULTI_WIN,

    /**
     * Result(2097198) for move mission to stack: target mission stack size is overflow.
     */
    MOVE_MISSION_TO_STACK_TARGET_STACK_OVERFLOW,

    /**
     * Result(2097199) for move mission to stack: already in moving state, request is denied.
     */
    MOVE_MISSION_TO_STACK_MOVING_DENIED,

    /**
     * Result(2097200) for minimize multi window failed.
     */
    MINIMIZE_MULTI_WINDOW_FAILED,

    /**
     * Result(2097201) for maximize multiwindow: target mission is not exist in multiwindow stack.
     */
    MAXIMIZE_MULTIWINDOW_NOT_EXIST,

    /**
     * Result(2097202) for maximize multi window failed.
     */
    MAXIMIZE_MULTIWINDOW_FAILED,

    /**
     * Result(2097203) for change focus ability failed.
     */
    CHANGE_FOCUS_ABILITY_FAILED,

    /**
     * Result(2097204) for get floating stack failed
     */
    GET_FLOATING_STACK_FAILED,

    /**
     * Result(2097205) for close multi window failed
     */
    CLOSE_MULTI_WINDOW_FAILED,

    /**
     * Result(2097206) for start ability by setting failed
     */
    START_ABILITY_SETTING_FAILED,

    /**
     * Result(2097207) for start ability by setting ,the ability not support multiwindow.
     */
    START_ABILITY_SETTING_NOT_SUPPORT_MULTI_WIN,

    /**
     * Result(2097208) for no found abilityrecord by caller
     */
    NO_FOUND_ABILITY_BY_CALLER,

    /**
     * Result(2097209) for ability visible attribute is false.
     */
    ABILITY_VISIBLE_FALSE_DENY_REQUEST,

    /**
     * Result(2097210) for caller is not systemapp.
     */
    CALLER_ISNOT_SYSTEMAPP,

    /**
     * Result(2097211) for get bundleName by uid fail.
     */
    GET_BUNDLENAME_BY_UID_FAIL,

    /**
     * Result(2097212) for set Permit list fail.
     */
    SET_WHITE_LIST_FAIL,

    /**
     * Result(2097213) for start ability failed,this ability is not on the Permit list
     */
    START_ABILITY_NOT_ONTHE_WHITELIST,

    /**
     * Result(2097214) for misson not found.
     */
    MISSION_NOT_FOUND,

    /**
     * Result(2097215) for get bundle info fail.
     */
    GET_BUNDLE_INFO_FAILED,

    /**
     * Result(2097216) for KillProcess: keep alive process can not be killed
     */
    KILL_PROCESS_KEEP_ALIVE,

    /**
     * Result(2097217) for clear the application data fail.
     */
    CLEAR_APPLICATION_DATA_FAIL,
	
	// for call ability
    /**
     * Result(2097218) for resolve ability failed, there is no permissions
     */
    RESOLVE_CALL_NO_PERMISSIONS,

    /**
     * Result(2097219) for resolve ability failed, target ability not page or singleton
     */
    RESOLVE_CALL_ABILITY_TYPE_ERR,
    
    /**
     * Result(2097220) for resolve ability failed, resolve failed.
     */
    RESOLVE_CALL_ABILITY_INNER_ERR,

    /**
     * Result(2097221) for resolve ability failed, resolve failed.
     */
    RESOLVE_CALL_ABILITY_VERSION_ERR,

    /**
     * Result(2097222) for release ability failed, release failed.
     */
    RELEASE_CALL_ABILITY_INNER_ERR,

    /**
     * Result(2097216) for register remote mission listener fail.
     */
    REGISTER_REMOTE_MISSION_LISTENER_FAIL,

    /**
     * Result(2097217) for unregister remote mission listener fail.
     */
    UNREGISTER_REMOTE_MISSION_LISTENER_FAIL,

    /**
     * Result() for invalid userid.
     */
    INVALID_USERID_VALUE,

    /**
     * Result() for start user test fail.
     */
    START_USER_TEST_FAIL,

    /**
     * Result(2097220) for send usr1 sig to the process of not response fail.
     */
    SEND_USR1_SIG_FAIL,

   /**
     * Result(2097221) for hidump fail.
     */
    ERR_AAFWK_HIDUMP_ERROR,

    /**
     * Result(2097222) for hidump params are invalid.
     */
    ERR_AAFWK_HIDUMP_INVALID_ARGS,
  
    /**
     * Result(2097223) target bundle name is not exist in targetBundleList.
     */
    TARGET_BUNDLE_NOT_EXIST,

    /**
     * Result(2097224) free install fail.
     */
    FREE_INSTALL_FAIL,

    /**
     * Result(2097223) free install fail.
     */
    NOT_TOP_ABILITY,
};

enum {
    /**
     * Flag for use with {@link #GetRecentMissions}: return all missions
     */
    RECENT_WITH_EXCLUDED = 0x0001,
    /**
     * Provides a list that does not contain any
     * recent missions that currently are not available to the user.
     */
    RECENT_IGNORE_UNAVAILABLE = 0x0002,
};

enum FreeInstallError {
    /**
     * FA search failed.
     */
    FA_FREE_INSTALL_QUERY_ERROR = -1,

    /**
     * FA Network unavailable.
     */
    FA_FREE_INSTALL_INTERNET_ERROR = -2,

    /**
     * FA internal system error.
     */
    FA_FREE_INSTALL_SERVICE_ERROR = -3,

    /**
     * HAG query timeout.
     */
    HAG_QUERY_TIMEOUT = -4,

    /**
     * Installation error in free installation.
     */
    INSTALLATION_ERROR_IN_FREE_INSTALL = -5,

    /**
     * There are concurrent tasks, waiting for retry.
     */
    CONCURRENT_TASKS_WAITING_FOR_RETRY = -6,

    /**
     * User gives up.
     */
    USER_GIVES_UP = -7,

    /**
     * The user confirms to jump to the application market upgrade.
     */
    JUMP_TO_THE_APPLICATION_MARKET_UPGRADE = -8,

    /**
     * HAP package download timed out.
     */
    HAP_PACKAGE_DOWNLOAD_TIMED_OUT = -9,

    /**
     * FA package does not support free installation.
     */
    FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL = -10,

    /**
     * It is not supported to pull up PA across applications on the same device.
     */
    ACROSS_APPLICATIONS_ON_THE_SAME_DEVICE = -11,

    /**
     * The app is not allowed to pull this FA.
     */
    NOT_ALLOWED_TO_PULL_THIS_FA = -901,

    /**
     * The calling app is not foreground.
     */
    CALLING_APP_IS_NOT_FOREGROUND = 0x820192,

    /**
     * The free install request cannot be sent to the FA distribution center.
     */
    REQUEST_CANNOT_SENT_TO_FA = 0x820101,

    /**
     * FA distribution center crash.
     */
    FA_CRASH = 0x820102,

    /**
     * FA distribution center processing timeout.
     */
    FA_TIMEOUT = 0x820103,

    /**
     * Unknown exception.
     */
    UNKNOWN_EXCEPTION = 0x820104,

    /**
     * The caller is not a DMS.
     */
    CALLER_IS_NOT_A_DMS = 0x820111,

    /**
     * Error in calling BMS interface parameters.
     */
    CALL_BMS_INTERFACE_PARAMETERS_ERROR = 0x830007,

    /**
     * The remote BMS service is not ready.
     */
    REMOTE_BMS_SERVICE_IS_NOT_READY = 0x800001,

    /**
     * Call BMS interface parcel serialization error.
     */
    CALL_BMS_INTERFACE_PARCEL_SERIALIZATION_ERROR = 0x800002,

    /**
     * BMS service exception.
     */
    BMS_SERVICE_EXCEPTION = 0x820001,

    /**
     * Distributed permission check failed.
     */
    DISTRIBUTED_PERMISSION_CHECK_FAILED = 29360176,

    /**
     * Parameter validation failed.
     */
    ERR_INVALID_PARAMETER = 29360128,

    /**
     * Remote device goes offline.
     */
    REMOTE_DEVICE_OFFLINE = 29360142,

    /**
     * Free install timeout in DMS.
     */
    DMS_FREE_INSTALL_TIMEOUT = 29360190,

    /**
     * Remote DMS is not compatible.
     */
    REMOTE_DMS_IS_NOT_COMPATIBLE = 502,

    /**
     * The visible of the remote FA is false and cannot be activated.
     */
    REMOTE_FA_IS_UNVISIBLE = 29360178,
};

static const std::map<FreeInstallError, std::string> FIErrorStrs = {
    {
        FA_FREE_INSTALL_QUERY_ERROR,
        "FA search failed"
    },
    {
        FA_FREE_INSTALL_INTERNET_ERROR,
        "Network unavailable."
    },
    {
        FA_FREE_INSTALL_SERVICE_ERROR,
        "FA internal system error."
    },
    {
        HAG_QUERY_TIMEOUT,
        "HAG query timeout."
    },
    {
        INSTALLATION_ERROR_IN_FREE_INSTALL,
        "Installation error in free installation."
    },
    {
        CONCURRENT_TASKS_WAITING_FOR_RETRY,
        "There are concurrent tasks, waiting for retry."
    },
    {
        USER_GIVES_UP,
        "User gives up."
    },
    {
        JUMP_TO_THE_APPLICATION_MARKET_UPGRADE,
        "The user confirms to jump to the application market upgrade."
    },
    {
        HAP_PACKAGE_DOWNLOAD_TIMED_OUT,
        "HAP package download timed out."
    },
    {
        FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL,
        "FA package does not support free installation."
    },
    {
        ACROSS_APPLICATIONS_ON_THE_SAME_DEVICE,
        "It is not supported to pull up PA across applications on the same device."
    },
    {
        NOT_ALLOWED_TO_PULL_THIS_FA,
        "The app is not allowed to pull this FA."
    },
    {
        CALLING_APP_IS_NOT_FOREGROUND,
        "The calling app is not foreground."
    },
    {
        REQUEST_CANNOT_SENT_TO_FA,
        "The free install request cannot be sent to the FA distribution center."
    },
    {
        FA_CRASH,
        "FA distribution center crash."
    },
    {
        FA_TIMEOUT,
        "FA distribution center processing timeout."
    },
    {
        UNKNOWN_EXCEPTION,
        "Unknown exception."
    },
    {
        CALLER_IS_NOT_A_DMS,
        "The caller is not a DMS."
    },
    {
        CALL_BMS_INTERFACE_PARAMETERS_ERROR,
        "Error in calling BMS interface parameters."
    },
    {
        REMOTE_BMS_SERVICE_IS_NOT_READY,
        "The remote BMS service is not ready."
    },
    {
        CALL_BMS_INTERFACE_PARCEL_SERIALIZATION_ERROR,
        "Call BMS interface parcel serialization error."
    },
    {
        BMS_SERVICE_EXCEPTION,
        "BMS service exception."
    },
    {
        DISTRIBUTED_PERMISSION_CHECK_FAILED,
        "Distributed permission check failed."
    },
    {
        ERR_INVALID_PARAMETER,
        "Parameter validation failed."
    },
    {
        REMOTE_DEVICE_OFFLINE,
        "Remote device goes offline."
    },
    {
        DMS_FREE_INSTALL_TIMEOUT,
        "Free install timeout in dms."
    },
    {
        REMOTE_DMS_IS_NOT_COMPATIBLE,
        "Remote DMS is not compatible."
    },
    {
        REMOTE_FA_IS_UNVISIBLE,
        "The visible of the remote FA is false and cannot be activated."
    },
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_MANAGER_ERRORS_H
