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

namespace OHOS {
namespace AbilityRuntime {
enum class AbilityErrorCode {
    ERROR_OK = 0,
    ERROR_CODE_PERMISSION_DENIED = 201,
    ERROR_CODE_INVALID_PARAM = 401,
    ERROR_CODE_SYSTEMCAP = 801,
    ERROR_CODE_INNER = 16000050, // inner error.
    ERROR_CODE_NO_MISSION_ID = 16300001,
    ERROR_CODE_NO_MISSION_LISTENER = 16300002,
};

std::string GetErrorMsg(const AbilityErrorCode& errCode);
std::string GetNoPermissionErrorMsg(const std::string& permission);
AbilityErrorCode GetJsErrorCodeByNativeError(int32_t errCode);

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif