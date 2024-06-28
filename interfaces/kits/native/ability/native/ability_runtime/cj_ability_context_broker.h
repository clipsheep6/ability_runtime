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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_BROKER_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_BROKER_H

#include "ability_context_impl.h"
#include "cj_common_ffi.h"
#include "cj_want_ffi.h"

extern "C" {
struct CJStartOptions {
    int32_t windowMode;
    int32_t displayId;
};

using VectorStringHandle = void*;

struct AbilityContextBroker {
    bool (*isAbilityContextExisted)(int64_t id);
    int64_t (*getSizeOfStartOptions)();

    int64_t (*getAbilityInfo)(int64_t id);
    int64_t (*getHapModuleInfo)(int64_t id);
    int64_t (*getConfiguration)(int64_t id);

    int32_t (*startAbility)(int64_t id, WantHandle wantHandle);
    int32_t (*startAbilityWithOption)(int64_t id, WantHandle wantHandle, CJStartOptions* startOption);
    int32_t (*startAbilityWithAccount)(int64_t id, WantHandle wantHandle, int32_t accountId);
    int32_t (*startAbilityWithAccountAndOption)(
        int64_t id, WantHandle wantHandle, int32_t accountId, CJStartOptions* startOption);
    int32_t (*startServiceExtensionAbility)(int64_t id, WantHandle want);
    int32_t (*startServiceExtensionAbilityWithAccount)(int64_t id, WantHandle want, int32_t accountId);
    int32_t (*stopServiceExtensionAbility)(int64_t id, WantHandle want);
    int32_t (*stopServiceExtensionAbilityWithAccount)(int64_t id, WantHandle want, int32_t accountId);

    int32_t (*terminateSelf)(int64_t id);
    int32_t (*terminateSelfWithResult)(int64_t id, WantHandle want, int32_t resultCode);
    RetDataBool (*isTerminating)(int64_t id);

    int32_t (*connectAbility)(int64_t id, WantHandle want, int64_t connection);
    int32_t (*connectAbilityWithAccount)(int64_t id, WantHandle want, int32_t accountId, int64_t connection);
    int32_t (*disconnectAbility)(int64_t id, WantHandle want, int64_t connection);
    int32_t (*startAbilityForResult)(int64_t id, WantHandle want, int32_t requestCode, int64_t lambdaId);
    int32_t (*startAbilityForResultWithOption)(
        int64_t id, WantHandle want, CJStartOptions* startOption, int32_t requestCode, int64_t lambdaId);
    int32_t (*startAbilityForResultWithAccount)(
        int64_t id, WantHandle want, int32_t accountId, int32_t requestCode, int64_t lambdaId);
    int32_t (*startAbilityForResultWithAccountAndOption)(int64_t id, WantHandle want, int32_t accountId,
        CJStartOptions* startOption, int32_t requestCode, int64_t lambdaId);
    int32_t (*requestPermissionsFromUser)(
        int64_t id, VectorStringHandle permissions, int32_t requestCode, int64_t lambdaId);

    int32_t (*setMissionLabel)(int64_t id, const char* label);
    int32_t (*setMissionIcon)(int64_t id, int64_t pixelMapId);
};
}

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONTEXT_BROKER_H