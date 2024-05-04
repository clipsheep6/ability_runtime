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

#ifndef FOUNDATION_APPEXECFWK_OHOS_CJ_WANT_FFI_H
#define FOUNDATION_APPEXECFWK_OHOS_CJ_WANT_FFI_H

#include <cstdint>

#include "cj_element_name_ffi.h"

using WantHandle = void*;
using VectorStringHandle = void*;

#define EXPORT __attribute__((visibility("default")))

extern "C" {
struct CJWantParams {
    ElementNameHandle elementName;
    uint32_t flags;
    const char* uri;
    const char* action;
    VectorStringHandle entities;
    const char* wantType;
    const char* parameters;
};

EXPORT void FFICJWantDelete(WantHandle want);
EXPORT CJWantParams* FFICJWantGetWantInfo(WantHandle want);
EXPORT void FFICJWantParamsDelete(CJWantParams* params);
EXPORT WantHandle FFICJWantCreateWithWantInfo(CJWantParams params);
EXPORT WantHandle FFICJWantParseUri(const char* uri);
EXPORT void FFICJWantAddEntity(WantHandle want, const char* entity);
};

#undef EXPORT

#endif // FOUNDATION_APPEXECFWK_OHOS_CJ_WANT_FFI_H
