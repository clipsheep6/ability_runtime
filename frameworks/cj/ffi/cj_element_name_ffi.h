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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ELEMENT_NAME_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_ELEMENT_NAME_FFI_H

#include "cj_macro.h"

using ElementNameHandle = void*;

extern "C" {
struct ElementNameParams {
    char* deviceId;
    char* bundleName;
    char* abilityName;
    char* moduleName;
};

CJ_EXPORT ElementNameHandle FFICJElementNameCreateWithContent(
    char* deviceId, char* bundleName, char* abilityName, char* moduleName);
CJ_EXPORT void FFICJElementNameDelete(ElementNameHandle elementNameHandle);
CJ_EXPORT ElementNameParams* FFICJElementNameGetElementNameInfo(ElementNameHandle elementNameHandle);
CJ_EXPORT void FFICJElementNameParamsDelete(ElementNameParams* elementNameParams);
};

#endif // OHOS_ABILITY_RUNTIME_CJ_ELEMENT_NAME_FFI_H
