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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_FFI_H

#include <cstdint>

#ifdef WINDOWS_PLATFORM
#define CJ_EXPORT __declspec(dllexport)
#else
#define CJ_EXPORT __attribute__((visibility("default")))
#endif

using AbilityHandle = void*;

extern "C" {
CJ_EXPORT int64_t FFIAbilityGetAbilityContext(AbilityHandle abilityHandle);
CJ_EXPORT void FFIAbilityContextGetFilesDir(int64_t id, void(*accept)(const char*));
}

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_FFI_H
