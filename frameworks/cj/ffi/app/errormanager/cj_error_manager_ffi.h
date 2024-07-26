/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_CJ_FFI_APP_ERROR_MANAGER_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_FFI_APP_ERROR_MANAGER_FFI_H

#include "cj_ffi/cj_common_ffi.h"
#include "cj_common.h"
#include <cstdint>

extern "C" {
    FFI_EXPORT RetDataI32 FfiOHOSErrorManagerOn(char* onType, CErrorObserver observer);
    FFI_EXPORT int FfiOHOSErrorManagerOff(char* offType, int observerId);
}

#endif