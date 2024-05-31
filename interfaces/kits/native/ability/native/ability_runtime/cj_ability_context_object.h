/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_OBJECT_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_OBJECT_H

#include "cj_want_ffi.h"
#include "cj_ability_context_broker.h"
#include "hilog_wrapper.h"


extern "C" {
struct CJAbilityResult {
    int32_t resultCode;
    WantHandle wantHandle;
};

struct CJPermissionRequestResult {
    VectorStringHandle permissions;
    VectorInt32Handle authResults;
};

struct CJDialogRequestResult {
    int32_t resultCode;
    WantHandle wantHandle;
};

CJ_EXPORT int32_t FFIAbilityContextRequestDialogService(int64_t id, WantHandle want, int64_t lambdaId);

struct CJAbilityCallbacks {
    void (*invokeAbilityResultCallback)(int64_t id, int32_t error, CJAbilityResult* cjAbilityResult);
    void (*invokePermissionRequestResultCallback)(
        int64_t id, int32_t error, CJPermissionRequestResult* cjPermissionRequestResult);
    void (*invokeDialogRequestResultCallback)(
        int64_t id, int32_t error, CJDialogRequestResult* cjDialogRequestResult);
};

CJ_EXPORT void RegisterCJAbilityCallbacks(void (*registerFunc)(CJAbilityCallbacks*));
}

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_OBJECT_H
