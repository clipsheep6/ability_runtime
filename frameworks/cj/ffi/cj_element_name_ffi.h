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

#ifndef FOUNDATION_APPEXECFWK_OHOS_CJ_ELEMENT_NAME_FFI_H
#define FOUNDATION_APPEXECFWK_OHOS_CJ_ELEMENT_NAME_FFI_H

using ElementNameHandle = void*;

#define EXPORT __attribute__((visibility("default")))

extern "C" {
struct ElementNameParams {
    const char* deviceId;
    const char* bundleName;
    const char* abilityName;
    const char* moduleName;
};

EXPORT ElementNameHandle FFICJElementNameCreateWithContent(
    const char* deviceId, const char* bundleName, const char* abilityName, const char* moduleName);
EXPORT void FFICJElementNameDelete(ElementNameHandle elementNameHandle);
EXPORT ElementNameParams* FFICJElementNameGetElementNameInfo(ElementNameHandle elementNameHandle);
EXPORT void FFICJElementNameParamsDelete(ElementNameParams* elementNameParams);
};

#undef EXPORT

#endif // FOUNDATION_APPEXECFWK_OHOS_CJ_ELEMENT_NAME_FFI_H
