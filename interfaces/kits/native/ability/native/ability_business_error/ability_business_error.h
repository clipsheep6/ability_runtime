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
    // Common error code
    ERROR_OK = 0,
    ERROR_CODE_PERMISSION_DENIED = 201,
    ERROR_CODE_INVALID_PARAM = 401,
    ERROR_CODE_SYSTEMCAP = 801,
    // AbilityRuntime.Core error code
    ERROR_CODE_ABILITY_NOT_EXIST = 16000001,
    ERROR_CODE_ABILITY_TYPE = 16000002,
    ERROR_CODE_ABILITY_VISIBLE = 16000004,
    ERROR_CODE_SERVICE_BUSY = 16000007,
    ERROR_CODE_APP_CROWDTEST = 16000008,
    ERROR_CODE_CONTEXT_EMPTY = 16000011,
    ERROR_CODE_ABILITY_EXISTED = 16000012,
    ERROR_CODE_CONNECTION_NOT_EXISTED = 16000013,
    ERROR_CODE_CONNECTION_INVALID_STATE = 16000014,
    ERROR_CODE_SERVICE_TIMEOUT = 16000015,
    ERROR_CODE_APP_CONTROL = 16000016,
    ERROR_CODE_INNER = 16000050, // inner error.
    ERROR_CODE_NETWORK_UNAVAILABLE = 16000051,
    ERROR_CODE_FREE_INSTALL_SUPPORT = 16000052,
    ERROR_CODE_TOP_ABILITY = 16000053,
    ERROR_CODE_FREE_INSTALL_BUSY = 16000054,
    ERROR_CODE_FREE_INSTALL_TIMEOUT = 16000055,
    ERROR_CODE_FREE_INSTALL_OTHER = 16000056,
    ERROR_CODE_FREE_INSTALL_DEVICE = 16000057,
    ERROR_CODE_NO_MISSION_ID = 16300001,
    ERROR_CODE_NO_MISSION_LISTENER = 16300002,
};

constexpr const char* TAG_PERMISSION = " permission:";
constexpr const char* ERROR_MSG_OK = "OK.";
constexpr const char* ERROR_MSG_PERMISSION_DENIED = "The application does not have permission to call the interface.";
constexpr const char* ERROR_MSG_INVALID_PARAM = "Invalid input parameter.";
constexpr const char* ERROR_MSG_SYSTEMCAP = "The specified SystemCapability name was not found.";
constexpr const char* ERROR_MSG_INNER = "Inner Error.";
constexpr const char* ERROR_MSG_CONTEXT_EMPTY = "Context does not exist, please check the context is available.";
constexpr const char* ERROR_MSG_NO_MISSION_ID = "The specified mission id does not exist.";
constexpr const char* ERROR_MSG_NO_MISSION_LISTENER = "The specified mission listener does not exist.";

static std::unordered_map<AbilityErrorCode, const char*> ERR_CODE_MAP = {
    { AbilityErrorCode::ERROR_OK, ERROR_MSG_OK },
    { AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED, ERROR_MSG_PERMISSION_DENIED },
    { AbilityErrorCode::ERROR_CODE_INVALID_PARAM, ERROR_MSG_INVALID_PARAM },
    { AbilityErrorCode::ERROR_CODE_SYSTEMCAP, ERROR_MSG_SYSTEMCAP },
    { AbilityErrorCode::ERROR_CODE_CONTEXT_EMPTY, ERROR_MSG_CONTEXT_EMPTY},
    { AbilityErrorCode::ERROR_CODE_INNER, ERROR_MSG_INNER },
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_ID, ERROR_MSG_NO_MISSION_ID },
    { AbilityErrorCode::ERROR_CODE_NO_MISSION_LISTENER, ERROR_MSG_NO_MISSION_LISTENER }
};

std::string GetErrorMsg(const AbilityErrorCode& errCode);
std::string GetNoPermissionErrorMsg(const std::string& permission);
AbilityErrorCode GetJsErrorCodeByNativeError(int32_t errCode);

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif