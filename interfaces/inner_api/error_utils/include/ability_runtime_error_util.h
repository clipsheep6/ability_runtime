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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RUNTIME_ERROR_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RUNTIME_ERROR_UTIL_H

#include <string>
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
enum {
    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_ABILITY_NAME = 16000001,
    ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SUPPORT_OPERATION = 16000002,
    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_ID = 16000003,
    ERR_ABILITY_RUNTIME_EXTERNAL_VISIBILITY_VERIFICATION_FAILED = 16000004,
    ERR_ABILITY_RUNTIME_EXTERNAL_CROSS_USER_OPERATION = 16000006,
    ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY = 16000007,
    ERR_ABILITY_RUNTIME_EXTERNAL_CROWDTEST_APP_EXPIRATION = 16000008,
    ERR_ABILITY_RUNTIME_EXTERNAL_WUKONG_MODE = 16000009,
    ERR_ABILITY_RUNTIME_EXTERNAL_OPERATION_WITH_CONTINUE_FLAG = 16000010,
    ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST = 16000011,
    ERR_ABILITY_RUNTIME_EXTERNAL_ABILITY_ALREADY_AT_TOP = 16000012,
    ERR_ABILITY_RUNTIME_EXTERNAL_CONNECTION_NOT_EXIST = 16000013,
    ERR_ABILITY_RUNTIME_EXTERNAL_CONNECTION_STATE_ABNORMAL = 16000014,
    ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_TIMEOUT = 16000015,
    ERR_ABILITY_RUNTIME_EXTERNAL_APP_UNDER_CONTROL = 16000016,
    ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR = 16000050,
    ERR_ABILITY_RUNTIME_EXTERNAL_NETWORK_ERROR = 16000051,
    ERR_ABILITY_RUNTIME_EXTERNAL_FREE_INSTALL_NOT_SUPPORT = 16000052,
    ERR_ABILITY_RUNTIME_EXTERNAL_NOT_TOP_ABILITY = 16000053,
    ERR_ABILITY_RUNTIME_EXTERNAL_FREE_INSTALL_BUSY = 16000054,
    ERR_ABILITY_RUNTIME_EXTERNAL_FREE_INSTALL_TIMEOUT = 16000055,
    ERR_ABILITY_RUNTIME_EXTERNAL_CANNOT_FREE_INSTALL_OTHER_ABILITY = 16000056,
    ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL = 16000057,
    ERR_ABILITY_RUNTIME_EXTERNAL_EXECUTE_SHELL_COMMAND_FAILED = 16000101,
    ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT = 16000151,
    ERR_ABILITY_RUNTIME_EXTERNAL_WANTAGENT_NOT_FOUND = 16000152,
    ERR_ABILITY_RUNTIME_EXTERNAL_WANTAGENT_CANCELED = 16000153,

    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_URI_ABILITY = 16100001,
    ERR_ABILITY_RUNTIME_EXTERNAL_FA_NOT_SUPPORT_OPERATION = 16100002,

    ERR_ABILITY_RUNTIME_EXTERNAL_CALLER_RELEASED = 16200001,
    ERR_ABILITY_RUNTIME_EXTERNAL_CALLEE_INVALID = 16200002,
    ERR_ABILITY_RUNTIME_EXTERNAL_RELEASE_ERROR = 16200003,
    ERR_ABILITY_RUNTIME_EXTERNAL_METHOED_REGISTERED = 16200004,
    ERR_ABILITY_RUNTIME_EXTERNAL_METHOED_NOT_REGISTERED = 16200005,

    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_MISSION = 16300001,
    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_MISSION_LISTENER = 16300002,

    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_BUNDLENAME = 18500001,
    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_HQF = 18500002,
    ERR_ABILITY_RUNTIME_EXTERNAL_DEPLOY_HQF_FAILED = 18500003,
    ERR_ABILITY_RUNTIME_EXTERNAL_SWITCH_HQF_FAILED = 18500004,
    ERR_ABILITY_RUNTIME_EXTERNAL_DELETE_HQF_FAILED = 18500005,
    ERR_ABILITY_RUNTIME_EXTERNAL_LOAD_PATCH_FAILED = 18500006,
    ERR_ABILITY_RUNTIME_EXTERNAL_UNLOAD_PATCH_FAILED = 18500007,
    ERR_ABILITY_RUNTIME_EXTERNAL_QUICK_FIX_INTERNAL_ERROR = 18500008,

    ERR_ABILITY_RUNTIME_EXTERNAL_NO_ACCESS_PERMISSION = 201,
    ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER = 401,
    ERR_ABILITY_RUNTIME_EXTERNAL_NO_SUCH_SYSCAP = 801,
};

class AbilityRuntimeErrorUtil {
public:
    /**
     * @brief Throw an runtime exception for js.
     *
     * @param engine js runtime engine.
     * @param errCode external errorcode.
     * @param errMessage error msg.
     * @return true if success.
     */
    static bool Throw(NativeEngine &engine, int32_t errCode, const std::string &errMessage = "");

    /**
     * @brief Throw an runtime exception for js with internal error code.
     *
     * @param engine js runtime engine.
     * @param errCode internal errorcode.
     * @return true if success.
     */
    static bool ThrowByInternalErrCode(NativeEngine &engine, int32_t errCode);

    /**
     * @brief Create a Error By Internal errorCode
     *
     * @param engine js runtime engine.
     * @param errCode internal errorcode.
     * @return js error object.
     */
    static NativeValue *CreateErrorByInternalErrCode(NativeEngine &engine, int32_t errCode);

    /**
     * @brief Get error message By error code
     * 
     * @param errCode internal errorcode.
     * @return error message.
     */
    static std::string GetErrMessage(int32_t errCode);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_RUNTIME_ERROR_UTIL_H