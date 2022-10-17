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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_BUSINESS_ERROR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_BUSINESS_ERROR_H

#include <string>
#include <unordered_map>

namespace OHOS {
namespace AbilityRuntime {
enum class AbilityErrorCode {
    // success
    ERROR_OK = 0,

    // no such permission.
    ERROR_CODE_PERMISSION_DENIED = 201,

    // invalid param.
    ERROR_CODE_INVALID_PARAM = 401,

    // no such systemcap.
    ERROR_CODE_SYSTEMCAP = 801,

    // common inner error.
    ERROR_CODE_INNER = 16000050,

    // can not find target ability.
    ERROR_CODE_RESOLVE_ABILITY = 16000001,

    // ability type is wrong.
    ERROR_CODE_INVALID_ABILITY_TYPE = 16000002,

    // no start invisible ability permission.
    ERROR_CODE_NO_INVISIBLE_PERMISSION = 16000004,

    // check static permission failed.
    ERROR_CODE_STATIC_CFG_PERMISSION = 16000005,

    // no permission to cross user.
    ERROR_CODE_CROSS_USER = 16000006,

    // crowdtest app expiration.
    ERROR_CODE_CROWDTEST_EXPIRED = 16000008,

    // wukong mode.
    ERROR_CODE_WUKONG_MODE = 16000009,

    // not allowed for continuation flag.
    ERROR_CODE_CONTINUATION_FLAG = 16000010,

    // context is invalid.
    ERROR_CODE_INVALID_CONTEXT = 16000011,

    // free install network abnormal.
    ERROR_CODE_NETWORK_ABNORMAL = 16000051,

    // not support free install.
    ERROR_CODE_NOT_SUPPORT_FREE_INSTALL = 16000052,

    // not top ability, not enable to free install.
    ERROR_CODE_NOT_TOP_ABILITY = 16000053,

    // too busy for free install.
    ERROR_CODE_FREE_INSTALL_TOO_BUSY = 16000054,

    // free install timeout.
    ERROR_CODE_FREE_INSTALL_TIMEOUT = 16000055,

    // free install other ability.
    ERROR_CODE_FREE_INSTALL_OTHERS = 16000056,

    // invalid caller.
    ERROR_CODE_INVALID_CALLER = 16200001,

    // no such mission id.
    ERROR_CODE_NO_MISSION_ID = 16300001,

    // no such mission listener.
    ERROR_CODE_NO_MISSION_LISTENER = 16300002,
};

constexpr const char* TAG_PERMISSION = " permission:";
constexpr const char* ERROR_MSG_OK = "OK.";
constexpr const char* ERROR_MSG_PERMISSION_DENIED = "The application does not have permission to call the interface.";
constexpr const char* ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
constexpr const char* ERROR_MSG_SYSTEMCAP = "The specified SystemCapability name was not found.";
constexpr const char* ERROR_MSG_INNER = "Inner Error.";
constexpr const char* ERROR_MSG_RESOLVE_ABILITY = "The specified ability does not exist.";
constexpr const char* ERROR_MSG_INVALID_ABILITY_TYPE = "Ability type error. The specified ability type is wrong";
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

static std::unordered_map<AbilityErrorCode, const char*> ERR_CODE_MAP = {
    { AbilityErrorCode::ERROR_OK, ERROR_MSG_OK },
    { AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED, ERROR_MSG_PERMISSION_DENIED },
    { AbilityErrorCode::ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM },
    { AbilityErrorCode::ERROR_CODE_SYSTEMCAP, ERROR_MSG_SYSTEMCAP },
    { AbilityErrorCode::ERROR_CODE_INNER, ERROR_MSG_INNER },
    { AbilityErrorCode::ERROR_CODE_RESOLVE_ABILITY, ERROR_MSG_RESOLVE_ABILITY },
    { AbilityErrorCode::ERROR_CODE_INVALID_ABILITY_TYPE, ERROR_MSG_INVALID_ABILITY_TYPE },
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
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER, ERROR_MSG_NO_MISSION_LISTENER }
};

std::string GetErrorMsg(const AbilityErrorCode& errCode);
std::string GetNoPermissionErrorMsg(const std::string& permission);
AbilityErrorCode GetJsErrorCodeByNativeError(int32_t errCode);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif