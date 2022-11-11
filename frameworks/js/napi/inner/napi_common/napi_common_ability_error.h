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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_ERROR_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_ERROR_H
#include "ability_info.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "feature_ability_common.h"

namespace OHOS {
namespace AppExecFwk {
const int32_t ERR_ABILITY_START_SUCCESS = 0;
const int32_t ERR_ABILITY_QUERY_FAILED = 1;
const int32_t ERR_NETWORK_UNAVAILABLE = 2;
const int32_t ERR_SYSTEM_ERROR = 3;
const int32_t ERR_LOADING_ERROR = 4;
const int32_t ERR_CONCURRENT_TASKS_WAITING_FOR_RETRY = 5;
const int32_t ERR_FREE_INSTALL_NOT_SUPPORTED = 6;
const int32_t ERR_SERVICE_ERROR = 7;
const int32_t ERR_PERMISSION_VERIFY_FAILED = 8;
const int32_t ERR_PARAMETER_INVALID = 9;
const int32_t ERR_REMOTE_INCOMPATIBLE = 10;
const int32_t ERR_DEVICE_OFFLINE = 11;
const int32_t ERR_FREE_INSTALL_TIMEOUT = 12;
const int32_t ERR_NOT_TOP_ABILITY = 13;
const int32_t ERR_TARGET_BUNDLE_NOT_EXIST = 14;
const int32_t ERR_CONTINUE_FREE_INSTALL_FAILED = 15;
const int32_t ERR_PARAM_INVALID = 202;

int32_t GetStartAbilityErrorCode(ErrCode innerErrorCode);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_ERROR_H
