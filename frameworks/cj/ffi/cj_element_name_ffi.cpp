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

#include "cj_element_name_ffi.h"

#include "cj_utils_ffi.h"
#include "element_name.h"

using OHOS::AppExecFwk::ElementName;

ElementNameHandle FFICJElementNameCreateWithContent(
    const char* deviceId, const char* bundleName, const char* abilityName, const char* moduleName)
{
    return new ElementName(deviceId, bundleName, abilityName, moduleName);
}

void FFICJElementNameDelete(ElementNameHandle elementNameHandle)
{
    auto actualElementName = reinterpret_cast<ElementName*>(elementNameHandle);
    delete actualElementName;
    actualElementName = nullptr;
}

// The return variable needs to delete in Cangjie by `FFICJElementNameParamsDelete`.
ElementNameParams* FFICJElementNameGetElementNameInfo(ElementNameHandle elementNameHandle)
{
    ElementNameParams* buffer = static_cast<ElementNameParams*>(malloc(sizeof(ElementNameParams)));
    if (buffer == nullptr) {
        return nullptr;
    }

    auto actualElementName = reinterpret_cast<ElementName*>(elementNameHandle);
    buffer->deviceId = CreateCStringFromString(actualElementName->GetDeviceID());
    buffer->bundleName = CreateCStringFromString(actualElementName->GetBundleName());
    buffer->abilityName = CreateCStringFromString(actualElementName->GetAbilityName());
    buffer->moduleName = CreateCStringFromString(actualElementName->GetModuleName());
    return buffer;
}

void FFICJElementNameParamsDelete(ElementNameParams* elementNameParams)
{
    free(static_cast<void*>(const_cast<char*>(elementNameParams->deviceId)));
    free(static_cast<void*>(const_cast<char*>(elementNameParams->bundleName)));        
    free(static_cast<void*>(const_cast<char*>(elementNameParams->abilityName)));
    free(static_cast<void*>(const_cast<char*>(elementNameParams->moduleName)));
    free(static_cast<void*>(const_cast<char*>(elementNameParams->deviceId)));
    free(static_cast<void *>(elementNameParams));
}