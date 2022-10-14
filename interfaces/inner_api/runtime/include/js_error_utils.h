/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

<<<<<<< HEAD
#include "ability_manager_errors.h"

#ifndef OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H

namespace OHOS {
namespace AbilityRuntime {
// Common error code
#define ERROR_CODE_NO_ERROR (0)
#define ERROR_CODE_QUERY_FAILED (1)                  // Can't match any other error code.
#define ERROR_CODE_PERMISSION_DENIED (201)           // The application does not have permission to call the interface.
#define ERROR_CODE_PARAM_INVALID (401)               // Invalid input parameter.
#define ERROR_CODE_SYSTEMCAP_ERROR (801)             // The specified SystemCapability names was not found.

// AbilityRuntime.Core error code
#define ERROR_CODE_ABILITY_NOT_EXIST (16000001)      // The specified ability name does not exit.
#define ERROR_CODE_ABILITY_VISIBLE (16000004)        // Visibility verification failed.
#define ERROR_CODE_CONTEXT_EMPTY (16000011)          // Context does not exist.
#define ERROR_CODE_ABILITY_EXISTED (16000012)        // Ability existed.
#define ERROR_CODE_CONNECTION_NOT_EXISTED (16000013) // Connection not exist.
#define ERROR_CODE_CONNECTION_INVALID_STATE (16000014) // Connection is invalid state.
#define ERROR_CODE_SERVICE_TIMEOUT (16000015)        // Service timeout.
#define ERROR_CODE_APP_CONTROL (16000016)            // App is under control.
#define ERROR_CODE_CALLEE_INVALID (16200002)         // Callee invalied.
#define ERROR_CODE_ABILITY_TYPE (16000002)           // Ability type error.
#define ERROR_CODE_SERVICE_BUSY (16000007)           // Service busyness.
#define ERROR_CODE_APP_CROWDTEST (16000008)          // Crowdtest app expired.
#define ERROR_CODE_INTERNAL_ERROR (16000050)         // Internal error.
#define ERROR_CODE_NETWORK_UNAVAILABLE (16000051)    // Network error.
#define ERROR_CODE_FREE_INSTALL_SUPPORT (16000052)   // Free install not support.
#define ERROR_CODE_TOP_ABILITY (16000053)            // Not top ability.
#define ERROR_CODE_FREE_INSTALL_BUSY (16000054)      // Free install busyness.
#define ERROR_CODE_FREE_INSTALL_TIMEOUT (16000055)   // Free install timeout.
#define ERROR_CODE_FREE_INSTALL_OTHER (16000056)     // Can not free install other ability.
#define ERROR_CODE_FREE_INSTALL_DEVICE (16000057)     // Not support cross device free install.

// Common error message
constexpr const char* ERROR_MSG_PERMISSION_DENIED = "The application does not have permission to call the interface.";
constexpr const char* ERROR_MSG_PARAM_INVALID = "Invalid input parameter.";
constexpr const char* ERROR_MSG_SYSTEMCAP_ERROR = "The specified SystemCapability names was not found.";

//AbilityRuntime.Core error message
constexpr const char* ERROR_MSG_ABILITY_NOT_EXIST = "The specified ability name does not exit.";
constexpr const char* ERROR_MSG_CONTEXT_EMPTY = "Context does not exist, please check the context is available.";
constexpr const char* ERROR_MSG_ABILITY_EXISTED = "The specified ability is already started";
constexpr const char* ERROR_MSG_CALLEE_INVALID = "Callee invalid, the callee does not exist.";
constexpr const char* ERROR_MSG_ABILITY_TYPE = "The specified ability type is wrong.";
constexpr const char* ERROR_MSG_SERVICE_BUSY = "There are concurrent tasks, waiting for retry";
constexpr const char* ERROR_MSG_CROWDTEST_EXPIRED = "The crowdtest application is expired.";
constexpr const char* ERROR_MSG_INTERNAL_ERROR = "Internal error.";
constexpr const char* ERROR_MSG_NETWORK_UNAVAILABLE = "The network is abnormal.";
constexpr const char* ERROR_MSG_FREE_INSTALL_SUPPORT = "The application does not support free install.";
constexpr const char* ERROR_MSG_TOP_ABILITY = "The application is not top ability.";
constexpr const char* ERROR_MSG_FREE_INSTALL_BUSY = "There are concurrent task, wait for retry.";
constexpr const char* ERROR_MSG_FREE_INSTALL_TIMEOUT = "Free install timeout.";
constexpr const char* ERROR_MSG_FREE_INSTALL_OTHER = "Can not free install other ability.";
constexpr const char* ERROR_MSG_FREE_INSTALL_DEVICE = "The device does not support free install.";

// map inner error code to js error code
const std::map<int32_t, int32_t> ERROR_CODE_MAP = {
    // ability manager error code
    { ERR_OK, ERROR_CODE_NO_ERROR },
    { ERR_INVALID_VALUE, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::RESOLVE_ABILITY_ERR, ERROR_CODE_ABILITY_NOT_EXIST },
    { AAFwk::GET_ABILITY_SERVICE_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ABILITY_SERVICE_NOT_CONNECTED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::RESOLVE_APP_ERR, ERROR_CODE_ABILITY_NOT_EXIST },
    { AAFwk::ABILITY_EXISTED, ERROR_CODE_ABILITY_EXISTED },
    { AAFwk::CREATE_MISSION_STACK_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::CREATE_ABILITY_RECORD_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::START_ABILITY_WAITING, ERROR_CODE_SERVICE_BUSY },
    { AAFwk::TERMINATE_LAUNCHER_DENIED, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::CONNECTION_NOT_EXIST, ERROR_CODE_CONNECTION_NOT_EXISTED },
    { AAFwk::INVALID_CONNECTION_STATE, ERROR_CODE_CONNECTION_INVALID_STATE },
    { AAFwk::LOAD_ABILITY_TIMEOUT, ERROR_CODE_SERVICE_TIMEOUT },
    { AAFwk::CONNECTION_TIMEOUT, ERROR_CODE_SERVICE_TIMEOUT },
    { AAFwk::GET_BUNDLE_MANAGER_SERVICE_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::REMOVE_MISSION_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::INNER_ERR, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::GET_RECENT_MISSIONS_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::REMOVE_STACK_LAUNCHER_DENIED, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::TARGET_ABILITY_NOT_SERVICE, ERROR_CODE_ABILITY_TYPE },
    { AAFwk::TERMINATE_SERVICE_IS_CONNECTED, ERROR_CODE_CONNECTION_INVALID_STATE },
    { AAFwk::START_SERVICE_ABILITY_ACTIVATING, ERROR_CODE_ABILITY_EXISTED },
    { AAFwk::MOVE_MISSION_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::KILL_PROCESS_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::UNINSTALL_APP_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::TERMINATE_ABILITY_RESULT_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::CHECK_PERMISSION_FAILED, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::NO_FOUND_ABILITY_BY_CALLER, ERROR_CODE_ABILITY_NOT_EXIST },
    { AAFwk::ABILITY_VISIBLE_FALSE_DENY_REQUEST, ERROR_CODE_ABILITY_VISIBLE },
    { AAFwk::CALLER_ISNOT_SYSTEMAPP, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::GET_BUNDLENAME_BY_UID_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::MISSION_NOT_FOUND, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::GET_BUNDLE_INFO_FAILED, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::KILL_PROCESS_KEEP_ALIVE, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::CLEAR_APPLICATION_DATA_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::RESOLVE_CALL_NO_PERMISSIONS, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::RESOLVE_CALL_ABILITY_TYPE_ERR, ERROR_CODE_ABILITY_TYPE },
    { AAFwk::RESOLVE_CALL_ABILITY_INNER_ERR, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::RESOLVE_CALL_ABILITY_VERSION_ERR, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::RELEASE_CALL_ABILITY_INNER_ERR, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::REGISTER_REMOTE_MISSION_LISTENER_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::UNREGISTER_REMOTE_MISSION_LISTENER_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::INVALID_USERID_VALUE, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::START_USER_TEST_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::SEND_USR1_SIG_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_AAFWK_HIDUMP_ERROR, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_AAFWK_HIDUMP_INVALID_ARGS, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_AAFWK_PARCEL_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_IMPLICIT_START_ABILITY_FAIL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_REACH_UPPER_LIMIT, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_AAFWK_INVALID_WINDOW_MODE, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_WRONG_INTERFACE_CALL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::ERR_CROWDTEST_EXPIRED, ERROR_CODE_APP_CROWDTEST },
    { AAFwk::ERR_DISPOSED_STATUS, ERROR_CODE_APP_CONTROL },
    { AAFwk::NativeFreeInstallError::FA_FREE_INSTALL_QUERY_ERROR, ERROR_CODE_ABILITY_NOT_EXIST },
    { AAFwk::NativeFreeInstallError::HAG_QUERY_TIMEOUT, ERROR_CODE_ABILITY_NOT_EXIST },
    { AAFwk::NativeFreeInstallError::FA_NETWORK_UNAVAILABLE, ERROR_CODE_NETWORK_UNAVAILABLE },
    { AAFwk::NativeFreeInstallError::FA_FREE_INSTALL_SERVICE_ERROR, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::NativeFreeInstallError::FA_CRASH, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::NativeFreeInstallError::FA_TIMEOUT, ERROR_CODE_SERVICE_TIMEOUT },
    { AAFwk::NativeFreeInstallError::UNKNOWN_EXCEPTION, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::NativeFreeInstallError::NOT_SUPPORT_PA_ON_SAME_DEVICE, ERROR_CODE_ABILITY_TYPE },
    { AAFwk::NativeFreeInstallError::FA_INTERNET_ERROR, ERROR_CODE_FREE_INSTALL_BUSY },
    { AAFwk::NativeFreeInstallError::JUMP_TO_THE_APPLICATION_MARKET_UPGRADE, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::NativeFreeInstallError::USER_GIVES_UP, ERROR_CODE_CONTEXT_EMPTY },
    { AAFwk::NativeFreeInstallError::INSTALLATION_ERROR_IN_FREE_INSTALL, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::NativeFreeInstallError::HAP_PACKAGE_DOWNLOAD_TIMED_OUT, ERROR_CODE_INTERNAL_ERROR },
    { AAFwk::NativeFreeInstallError::CONCURRENT_TASKS_WAITING_FOR_RETRY, ERROR_CODE_FREE_INSTALL_BUSY },
    { AAFwk::NativeFreeInstallError::FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL, ERROR_CODE_FREE_INSTALL_SUPPORT },
    { AAFwk::NativeFreeInstallError::NOT_ALLOWED_TO_PULL_THIS_FA, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::NativeFreeInstallError::NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL_PA, ERROR_CODE_FREE_INSTALL_SUPPORT },
    { AAFwk::NativeFreeInstallError::DMS_PERMISSION_DENIED, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::NativeFreeInstallError::DMS_COMPONENT_ACCESS_PERMISSION_DENIED, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::NativeFreeInstallError::DMS_ACCOUNT_ACCESS_PERMISSION_DENIED, ERROR_CODE_PERMISSION_DENIED },
    { AAFwk::NativeFreeInstallError::INVALID_PARAMETERS_ERR, ERROR_CODE_PARAM_INVALID },
    { AAFwk::NativeFreeInstallError::INVALID_REMOTE_PARAMETERS_ERR, ERROR_CODE_PARAM_INVALID },
    { AAFwk::NativeFreeInstallError::REMOTE_DEVICE_NOT_COMPATIBLE, ERROR_CODE_FREE_INSTALL_DEVICE },
    { AAFwk::NativeFreeInstallError::DEVICE_OFFLINE_ERR, ERROR_CODE_FREE_INSTALL_DEVICE },
    { AAFwk::NativeFreeInstallError::FREE_INSTALL_TIMEOUT, ERROR_CODE_FREE_INSTALL_TIMEOUT },
    { AAFwk::NativeFreeInstallError::NOT_TOP_ABILITY, ERROR_CODE_TOP_ABILITY },
    { AAFwk::NativeFreeInstallError::TARGET_BUNDLE_NOT_EXIST, ERROR_CODE_FREE_INSTALL_OTHER },
    { AAFwk::NativeFreeInstallError::CONTINUE_FREE_INSTALL_FAILED, ERROR_CODE_FREE_INSTALL_BUSY },
    // NAPI error code
    { AppExecFwk::NAPI_ERR_NO_PERMISSION, ERROR_CODE_PERMISSION_DENIED },
    { AppExecFwk::NAPI_ERR_INNER_DATA, ERROR_CODE_INTERNAL_ERROR },
    { AppExecFwk::NAPI_ERR_ACE_ABILITY, ERROR_CODE_ABILITY_NOT_EXIST },
    { AppExecFwk::NAPI_ERR_LONG_CALLBACK, ERROR_CODE_INTERNAL_ERROR },
    { AppExecFwk::NAPI_ERR_PARAM_INVALID, ERROR_CODE_PARAM_INVALID },
    { AppExecFwk::NAPI_ERR_ABILITY_TYPE_INVALID, ERROR_CODE_ABILITY_TYPE },
    { AppExecFwk::NAPI_ERR_ABILITY_CALL_INVALID, ERROR_CODE_ABILITY_NOT_EXIST }
};

int32_t GetErrorCode(const ErrCode &innerErrorCode)
{
    auto iter = ERROR_CODE_MAP.find(innerErrorCode);
    if (iter != ERROR_CODE_MAP.end()) {
        return iter->second;
    }
    return ERROR_CODE_QUERY_FAILED;
}

void ThrowError(NativeEngine& engine, int32_t errCode, const std::string& errorMsg = "");
void ThrowError(NativeEngine& engine, const AbilityErrorCode& err);
void ThrowTooFewParametersError(NativeEngine& engine);
void ThrowNoPermissionError(NativeEngine& engine, const std::string& permission);

NativeValue* CreateJsError(NativeEngine& engine, const AbilityErrorCode& err);
NativeValue* CreateNoPermissionError(NativeEngine& engine, const std::string& permission);
NativeValue* CreateJsErrorByNativeErr(NativeEngine& engine, int32_t err, const std::string& permission = "");
}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
