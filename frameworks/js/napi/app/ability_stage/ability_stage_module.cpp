/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "native_engine/native_engine.h"

extern const char _binary_ability_stage_js_start[];
extern const char _binary_ability_stage_js_end[];
extern const char _binary_ability_stage_abc_start[];
extern const char _binary_ability_stage_abc_end[];

static napi_module _module = {
#ifdef ENABLE_ERRCODE
    .nm_version = 0,
    .nm_filename = "app/ability/libabilitystage.so/ability_stage.js",
    .nm_modname = "app.ability.AbilityStage",
#else
    .nm_version = 0,
    .nm_filename = "application/libabilitystage_napi.so/ability_stage.js",
    .nm_modname = "application.AbilityStage",
#endif
};
extern "C" __attribute__((constructor))
#ifdef ENABLE_ERRCODE
void NAPI_app_ability_AbilityStage_AutoRegister()
#else
void NAPI_application_AbilityStage_AutoRegister()
#endif
{
    napi_module_register(&_module);
}

extern "C" __attribute__((visibility("default")))
#ifdef ENABLE_ERRCODE
void NAPI_app_ability_AbilityStage_GetJSCode(const char **buf, int *bufLen)
#else
void NAPI_application_AbilityStage_GetJSCode(const char **buf, int *bufLen)
#endif
{
    if (buf != nullptr) {
        *buf = _binary_ability_stage_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_ability_stage_js_end - _binary_ability_stage_js_start;
    }
}

// ability_stage JS register
extern "C" __attribute__((visibility("default")))
#ifdef ENABLE_ERRCODE
void NAPI_app_ability_AbilityStage_GetABCCode(const char **buf, int *buflen)
#else
void NAPI_application_AbilityStage_GetABCCode(const char **buf, int *buflen)
#endif
{
    if (buf != nullptr) {
        *buf = _binary_ability_stage_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_ability_stage_abc_end - _binary_ability_stage_abc_start;
    }
}
