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

#include "ability_business_error.h"

#include <unordered_map>

#include "ability_manager_errors.h"
#include "napi_common_error.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
namespace {
static std::unordered_map<int32_t, AbilityErrorCode> INNER_TO_JS_ERROR_CODE_MAP {
    {0, AbilityErrorCode::ERROR_OK},
    {CHECK_PERMISSION_FAILED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {MISSION_NOT_FOUND, AbilityErrorCode::ERROR_CODE_NO_MISSION_ID},
    {RESOLVE_ABILITY_ERR, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST},
    {GET_ABILITY_SERVICE_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {ABILITY_SERVICE_NOT_CONNECTED, AbilityErrorCode::ERROR_CODE_INNER},
    {RESOLVE_APP_ERR, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST},
    {ABILITY_EXISTED, AbilityErrorCode::ERROR_CODE_ABILITY_EXISTED},
    {CREATE_MISSION_STACK_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {CREATE_ABILITY_RECORD_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {START_ABILITY_WAITING, AbilityErrorCode::ERROR_CODE_SERVICE_BUSY},
    {TERMINATE_LAUNCHER_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {CONNECTION_NOT_EXIST, AbilityErrorCode::ERROR_CODE_CONNECTION_NOT_EXISTED},
    {INVALID_CONNECTION_STATE, AbilityErrorCode::ERROR_CODE_CONNECTION_INVALID_STATE},
    {LOAD_ABILITY_TIMEOUT, AbilityErrorCode::ERROR_CODE_SERVICE_TIMEOUT},
    {CONNECTION_TIMEOUT, AbilityErrorCode::ERROR_CODE_SERVICE_TIMEOUT},
    {GET_BUNDLE_MANAGER_SERVICE_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {REMOVE_MISSION_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {INNER_ERR, AbilityErrorCode::ERROR_CODE_INNER},
    {GET_RECENT_MISSIONS_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {REMOVE_STACK_LAUNCHER_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {TARGET_ABILITY_NOT_SERVICE, AbilityErrorCode::ERROR_CODE_ABILITY_TYPE},
    {TERMINATE_SERVICE_IS_CONNECTED, AbilityErrorCode::ERROR_CODE_CONNECTION_INVALID_STATE},
    {START_SERVICE_ABILITY_ACTIVATING, AbilityErrorCode::ERROR_CODE_ABILITY_EXISTED},
    {MOVE_MISSION_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {KILL_PROCESS_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {UNINSTALL_APP_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {TERMINATE_ABILITY_RESULT_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {NO_FOUND_ABILITY_BY_CALLER, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST},
    {ABILITY_VISIBLE_FALSE_DENY_REQUEST, AbilityErrorCode::ERROR_CODE_ABILITY_VISIBLE},
    {CALLER_ISNOT_SYSTEMAPP, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {GET_BUNDLENAME_BY_UID_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {GET_BUNDLE_INFO_FAILED, AbilityErrorCode::ERROR_CODE_INNER},
    {KILL_PROCESS_KEEP_ALIVE, AbilityErrorCode::ERROR_CODE_INNER},
    {CLEAR_APPLICATION_DATA_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {RESOLVE_CALL_NO_PERMISSIONS, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {RESOLVE_CALL_ABILITY_TYPE_ERR, AbilityErrorCode::ERROR_CODE_ABILITY_TYPE},
    {RESOLVE_CALL_ABILITY_INNER_ERR, AbilityErrorCode::ERROR_CODE_INNER},
    {RESOLVE_CALL_ABILITY_VERSION_ERR, AbilityErrorCode::ERROR_CODE_INNER},
    {RELEASE_CALL_ABILITY_INNER_ERR, AbilityErrorCode::ERROR_CODE_INNER},
    {REGISTER_REMOTE_MISSION_LISTENER_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {UNREGISTER_REMOTE_MISSION_LISTENER_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {INVALID_USERID_VALUE, AbilityErrorCode::ERROR_CODE_INNER},
    {START_USER_TEST_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {SEND_USR1_SIG_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_AAFWK_HIDUMP_ERROR, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_AAFWK_HIDUMP_INVALID_ARGS, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_AAFWK_PARCEL_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_IMPLICIT_START_ABILITY_FAIL, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_REACH_UPPER_LIMIT, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_AAFWK_INVALID_WINDOW_MODE, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_WRONG_INTERFACE_CALL, AbilityErrorCode::ERROR_CODE_INNER},
    {ERR_CROWDTEST_EXPIRED, AbilityErrorCode::ERROR_CODE_APP_CROWDTEST},
    {ERR_DISPOSED_STATUS, AbilityErrorCode::ERROR_CODE_APP_CONTROL},
    {NativeFreeInstallError::FA_FREE_INSTALL_QUERY_ERROR, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST},
    {NativeFreeInstallError::HAG_QUERY_TIMEOUT, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST},
    {NativeFreeInstallError::FA_NETWORK_UNAVAILABLE, AbilityErrorCode::ERROR_CODE_NETWORK_UNAVAILABLE},
    {NativeFreeInstallError::FA_FREE_INSTALL_SERVICE_ERROR, AbilityErrorCode::ERROR_CODE_INNER},
    {NativeFreeInstallError::FA_CRASH, AbilityErrorCode::ERROR_CODE_INNER},
    {NativeFreeInstallError::FA_TIMEOUT, AbilityErrorCode::ERROR_CODE_SERVICE_TIMEOUT},
    {NativeFreeInstallError::UNKNOWN_EXCEPTION, AbilityErrorCode::ERROR_CODE_INNER},
    {NativeFreeInstallError::NOT_SUPPORT_PA_ON_SAME_DEVICE, AbilityErrorCode::ERROR_CODE_ABILITY_TYPE},
    {NativeFreeInstallError::FA_INTERNET_ERROR, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_BUSY},
    {NativeFreeInstallError::JUMP_TO_THE_APPLICATION_MARKET_UPGRADE, AbilityErrorCode::ERROR_CODE_INNER},
    {NativeFreeInstallError::USER_GIVES_UP, AbilityErrorCode::ERROR_CODE_CONTEXT_EMPTY},
    {NativeFreeInstallError::INSTALLATION_ERROR_IN_FREE_INSTALL, AbilityErrorCode::ERROR_CODE_INNER},
    {NativeFreeInstallError::HAP_PACKAGE_DOWNLOAD_TIMED_OUT, AbilityErrorCode::ERROR_CODE_INNER},
    {NativeFreeInstallError::CONCURRENT_TASKS_WAITING_FOR_RETRY, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_BUSY},
    {NativeFreeInstallError::FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_SUPPORT},
    {NativeFreeInstallError::NOT_ALLOWED_TO_PULL_THIS_FA, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {NativeFreeInstallError::NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL_PA, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_SUPPORT},
    {NativeFreeInstallError::DMS_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {NativeFreeInstallError::DMS_COMPONENT_ACCESS_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {NativeFreeInstallError::DMS_ACCOUNT_ACCESS_PERMISSION_DENIED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {NativeFreeInstallError::INVALID_PARAMETERS_ERR, AbilityErrorCode::ERROR_CODE_INVALID_PARAM},
    {NativeFreeInstallError::INVALID_REMOTE_PARAMETERS_ERR, AbilityErrorCode::ERROR_CODE_INVALID_PARAM},
    {NativeFreeInstallError::REMOTE_DEVICE_NOT_COMPATIBLE, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_DEVICE},
    {NativeFreeInstallError::DEVICE_OFFLINE_ERR, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_DEVICE},
    {NativeFreeInstallError::FREE_INSTALL_TIMEOUT, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TIMEOUT},
    {NativeFreeInstallError::NOT_TOP_ABILITY, AbilityErrorCode::ERROR_CODE_TOP_ABILITY},
    {NativeFreeInstallError::TARGET_BUNDLE_NOT_EXIST, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_OTHER},
    {NativeFreeInstallError::CONTINUE_FREE_INSTALL_FAILED, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_BUSY},
    // NAPI error code
    {NAPI_ERR_NO_PERMISSION, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {NAPI_ERR_INNER_DATA, AbilityErrorCode::ERROR_CODE_INNER},
    {NAPI_ERR_ACE_ABILITY, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST},
    {NAPI_ERR_LONG_CALLBACK, AbilityErrorCode::ERROR_CODE_INNER},
    {NAPI_ERR_PARAM_INVALID, AbilityErrorCode::ERROR_CODE_INVALID_PARAM},
    {NAPI_ERR_ABILITY_TYPE_INVALID, AbilityErrorCode::ERROR_CODE_ABILITY_TYPE},
    {NAPI_ERR_ABILITY_CALL_INVALID, AbilityErrorCode::ERROR_CODE_ABILITY_NOT_EXIST}
};
}

std::string GetErrorMsg(const AbilityErrorCode& errCode)
{
    auto it = ERR_CODE_MAP.find(errCode);
    if (it != ERR_CODE_MAP.end()) {
        return it->second;
    }

    return "";
}

std::string GetNoPermissionErrorMsg(const std::string& permission)
{
    return std::string(ERROR_MSG_PERMISSION_DENIED) + std::string(TAG_PERMISSION) + permission;
}

AbilityErrorCode GetJsErrorCodeByNativeError(int32_t errCode)
{
    auto it = INNER_TO_JS_ERROR_CODE_MAP.find(errCode);
    if (it != INNER_TO_JS_ERROR_CODE_MAP.end()) {
        return it->second;
    }

    return AbilityErrorCode::ERROR_CODE_INNER;
}
}  // namespace AbilityRuntime
}  // namespace OHOS