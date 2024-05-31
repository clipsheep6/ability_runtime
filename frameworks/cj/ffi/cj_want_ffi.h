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

#ifndef OHOS_ABILITY_RUNTIME_CJ_WANT_FFI_H
#define OHOS_ABILITY_RUNTIME_CJ_WANT_FFI_H

#include <cstdint>

#include "cj_macro.h"
#include "cj_element_name_ffi.h"

using WantHandle = void*;
using VectorStringHandle = void*;
using VectorInt32Handle = void*;

extern "C" {
struct CJWantParams {
    ElementNameHandle elementName;
    uint32_t flags;
    char* uri;
    char* action;
    VectorStringHandle entities;
    char* wantType;
    char* parameters;
};

CJ_EXPORT void FFICJWantDelete(WantHandle want);
CJ_EXPORT CJWantParams* FFICJWantGetWantInfo(WantHandle want);
CJ_EXPORT void FFICJWantParamsDelete(CJWantParams* params);
CJ_EXPORT WantHandle FFICJWantCreateWithWantInfo(CJWantParams params);
CJ_EXPORT WantHandle FFICJWantParseUri(const char* uri);
CJ_EXPORT void FFICJWantAddEntity(WantHandle want, const char* entity);
};

#endif // OHOS_ABILITY_RUNTIME_CJ_WANT_FFI_H
