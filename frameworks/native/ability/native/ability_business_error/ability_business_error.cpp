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

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AAFwk;
namespace {
constexpr const char* TAG_PERMISSION = " permission:";
constexpr const char* ERROR_MSG_OK = "OK.";
constexpr const char* ERROR_MSG_PERMISSION_DENIED = "The application does not have permission to call the interface.";
constexpr const char* ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
constexpr const char* ERROR_MSG_SYSTEMCAP = "The specified SystemCapability name was not found.";
constexpr const char* ERROR_MSG_INNER = "Inner Error.";
constexpr const char* ERROR_MSG_RESOLVE_ABILITY = "The specified ability does not exist.";
constexpr const char* ERROR_MSG_INVALID_ABILITY_TYPE = "Ability type error. The specified ability type is wrong";
constexpr const char* ERROR_MSG_INVALID_ID = "The specified id does not exist.";
constexpr const char* ERROR_MSG_INVISIBLE = "Can not start invisible component.";
constexpr const char* ERROR_MSG_STATIC_CFG_PERMISSION = "The specified process does not have the permission.";
constexpr const char* ERROR_MSG_CROSS_USER = "Can not cross user operations.";
constexpr const char* ERROR_MSG_CROWDTEST_EXPIRED = "Crowdtest App Expiration.";
constexpr const char* ERROR_MSG_WUKONG_MODE = "Cannot operate in wukong mode.";
constexpr const char* ERROR_MSG_CONTINUATION_FLAG = "StartAbility with continuation flags is not allowed!";
constexpr const char* ERROR_MSG_INVALID_CONTEXT = "Context does not exist!";
constexpr const char* ERROR_MSG_NETWORK_ABNORMAL = "Network error. The network is abnormal when free install.";
constexpr const char* ERROR_MSG_NOT_SUPPORT_FREE_INSTALL = "The Application does not support free install.";
constexpr const char* ERROR_MSG_NOT_TOP_ABILITY = "Not top ability, Can not free install ability.";
constexpr const char* ERROR_MSG_FREE_INSTALL_TOO_BUSY =
    "Free install busyness. There are concurrent tasks waiting for retry.";
constexpr const char* ERROR_MSG_FREE_INSTALL_TIMEOUT = "Free install timeout.";
constexpr const char* ERROR_MSG_FREE_INSTALL_OTHERS = "Can not free install other ability.";
constexpr const char* ERROR_MSG_INVALID_CALLER = "Caller released, The caller has been released.";
constexpr const char* ERROR_MSG_NO_MISSION_ID = "The specified mission id does not exist.";
constexpr const char* ERROR_MSG_NO_MISSION_LISTENER = "The specified mission listener does not exist.";
constexpr const char* ERROR_MSG_START_ABILITY_WAITTING = "The previous ability is starting, wait start later.";

static std::unordered_map<AbilityErrorCode, const char*> ERR_CODE_MAP = {
    { AbilityErrorCode::ERROR_OK, ERROR_MSG_OK },
    { AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED, ERROR_MSG_PERMISSION_DENIED },
    { AbilityErrorCode::ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM },
    { AbilityErrorCode::ERROR_CODE_SYSTEMCAP, ERROR_MSG_SYSTEMCAP },
    { AbilityErrorCode::ERROR_CODE_INNER, ERROR_MSG_INNER },
    { AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY, ERROR_MSG_RESOLVE_ABILITY },
    { AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE, ERROR_MSG_INVALID_ABILITY_TYPE },
    { AbilityErrorCode::ERROR_CODE_INVALID_ID, ERROR_MSG_INVALID_ID },
    { AbilityErrorCode::ERROR_CODE_NO_INVISIBLE_PERMISSION, ERROR_MSG_INVISIBLE },
    { AbilityErrorCode::ERROR_CODE_STATIC_CFG_PERMISSION, ERROR_MSG_STATIC_CFG_PERMISSION },
    { AbilityErrorCode::ERROR_CODE_CROSS_USER, ERROR_MSG_CROSS_USER },
    { AbilityErrorCode::ERROR_CODE_CROWDTEST_EXPIRED, ERROR_MSG_CROWDTEST_EXPIRED },
    { AbilityErrorCode::ERROR_CODE_WUKONG_MODE, ERROR_MSG_WUKONG_MODE },
    { AbilityErrorCode::ERROR_CODE_CONTINUATION_FLAG, ERROR_MSG_CONTINUATION_FLAG},
    { AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT, ERROR_MSG_INVALID_CONTEXT},
    { AbilityErrorCode::ERROR_CODE_NETWORK_ABNORMAL, ERROR_MSG_NETWORK_ABNORMAL },
    { AbilityErrorCode::ERROR_CODE_NOT_SUPPORT_FREE_INSTALL, ERROR_MSG_NOT_SUPPORT_FREE_INSTALL },
    { AbilityErrorCode::ERROR_CODE_NOT_TOP_ABILITY, ERROR_MSG_NOT_TOP_ABILITY },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TOO_BUSY, ERROR_MSG_FREE_INSTALL_TOO_BUSY },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TIMEOUT, ERROR_MSG_FREE_INSTALL_TIMEOUT },
    { AbilityErrorCode::ERROR_CODE_FREE_INSTALL_OTHERS, ERROR_MSG_FREE_INSTALL_OTHERS },
    { AbilityErrorCode::ERROR_CODE_INVALID_CALLER, ERROR_MSG_INVALID_CALLER },
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_ID, ERROR_MSG_NO_MISSION_ID },
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER, ERROR_MSG_NO_MISSION_LISTENER },
    { AbilityErrorCode::ERROR_START_ABILITY_WAITTING, ERROR_MSG_START_ABILITY_WAITTING }
};

static std::unordered_map<int32_t, AbilityErrorCode> INNER_TO_JS_ERROR_CODE_MAP {
    {0, AbilityErrorCode::ERROR_OK},
    {CHECK_PERMISSION_FAILED, AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED},
    {RESOLVE_ABILITY_ERR, AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY},
    {ERR_WRONG_INTERFACE_CALL, AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE},
    {TARGET_ABILITY_NOT_SERVICE, AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE},
    {RESOLVE_CALL_ABILITY_TYPE_ERR, AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE},
    {ABILITY_VISIBLE_FALSE_DENY_REQUEST, AbilityErrorCode::ERROR_CODE_NO_INVISIBLE_PERMISSION},
    {ERR_STATIC_CFG_PERMISSION, AbilityErrorCode::ERROR_CODE_STATIC_CFG_PERMISSION},
    {ERR_CROSS_USER, AbilityErrorCode::ERROR_CODE_CROSS_USER},
    {ERR_CROWDTEST_EXPIRED, AbilityErrorCode::ERROR_CODE_CROWDTEST_EXPIRED},
    {ERR_WOULD_BLOCK, AbilityErrorCode::ERROR_CODE_WUKONG_MODE},
    {ERR_INVALID_CONTINUATION_FLAG, AbilityErrorCode::ERROR_CODE_CONTINUATION_FLAG},
    {HAP_PACKAGE_DOWNLOAD_TIMED_OUT, AbilityErrorCode::ERROR_CODE_NETWORK_ABNORMAL},
    {FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL, AbilityErrorCode::ERROR_CODE_NOT_SUPPORT_FREE_INSTALL},
    {NOT_TOP_ABILITY, AbilityErrorCode::ERROR_CODE_NOT_TOP_ABILITY},
    {CONCURRENT_TASKS_WAITING_FOR_RETRY, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TOO_BUSY},
    {FREE_INSTALL_TIMEOUT, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_TIMEOUT},
    {NOT_ALLOWED_TO_PULL_THIS_FA, AbilityErrorCode::ERROR_CODE_FREE_INSTALL_OTHERS},
    {ERR_INVALID_CALLER, AbilityErrorCode::ERROR_CODE_INVALID_CALLER},
    {MISSION_NOT_FOUND, AbilityErrorCode::ERROR_CODE_NO_MISSION_ID},
    {START_ABILITY_WAITING, AbilityErrorCode::ERROR_START_ABILITY_WAITTING}
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