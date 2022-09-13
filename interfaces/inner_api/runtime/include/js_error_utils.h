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

#ifndef OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H

namespace OHOS {
namespace AbilityRuntime {
//Common error code
#define ERROR_CODE_PERMISSION_DENIED (201)          // The application does not have permission to call the interface.
#define ERROR_CODE_PARAM_INVALID (401)              // Invalid input parameter.
#define ERROR_CODE_SYSTEMCAP_ERROR (801)            // The specified SystemCapability names was not found.

//AbilityRuntime.Core error code
#define ERROR_CODE_CONTEXT_EMPTY (16000011)     // Context does not exist.

//Common error message
constexpr const char* ERROR_MSG_PERMISSION_DENIED = "The application does not have permission to call the interface.";
constexpr const char* ERROR_MSG_PARAM_INVALID = "Invalid input parameter.";
constexpr const char* ERROR_MSG_SYSTEMCAP_ERROR = "The specified SystemCapability names was not found.";

//AbilityRuntime.Core error message
constexpr const char* ERROR_MSG_CONTEXT_EMPTY = "Context does not exist, please check the context is available";
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
