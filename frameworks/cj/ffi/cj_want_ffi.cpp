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

#include "cj_want_ffi.h"

#include <cstring>
#include <string>
#include <vector>

#include "cj_utils_ffi.h"
#include "want.h"
#include "want_params_wrapper.h"

using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::ElementName;

// Attention: The function does not handle entities.
WantHandle FFICJWantCreateWithWantInfo(CJWantParams params)
{
    Want* want = new (std::nothrow) Want();
    if (want == nullptr) {
        return nullptr;
    }

    auto element = reinterpret_cast<ElementName*>(params.elementName);
    want->SetElement(*element);
    want->SetFlags(params.flags);
    want->SetUri(params.uri);
    want->SetAction(params.action);
    want->SetType(params.wantType);
    want->SetParams(OHOS::AAFwk::WantParamWrapper::ParseWantParams(params.parameters));

    return want;
}

void FFICJWantDelete(WantHandle want)
{
    auto actualWant = reinterpret_cast<Want*>(want);
    delete actualWant;
    actualWant = nullptr;
}

CJWantParams* FFICJWantGetWantInfo(WantHandle want)
{
    CJWantParams* buffer = static_cast<CJWantParams*>(malloc(sizeof(CJWantParams)));
    if (buffer == nullptr) {
        return nullptr;
    }

    auto actualWant = reinterpret_cast<Want*>(want);
    auto element = actualWant->GetElement();
    ElementNameHandle elementName = new ElementName(
        element.GetDeviceID(), element.GetBundleName(), element.GetAbilityName(), element.GetModuleName());
    buffer->elementName = elementName;
    buffer->flags = actualWant->GetFlags();
    buffer->uri = CreateCStringFromString(actualWant->GetUriString());
    buffer->action = CreateCStringFromString(actualWant->GetAction());
    buffer->wantType = CreateCStringFromString(actualWant->GetType());
    buffer->entities = const_cast<std::vector<std::string>*>(&(actualWant->GetEntities())); // reference vector<String>
    buffer->parameters = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(actualWant->GetParams()).ToString());
    return buffer;
}

void FFICJWantParamsDelete(CJWantParams* params)
{
    auto actualElementName = reinterpret_cast<ElementName*>(params->elementName);
    delete actualElementName;
    actualElementName = nullptr;

    free((void*)params->uri);
    free((void*)params->action);
    free((void*)params->wantType);
    free((void*)params->parameters);
    // Entities are reference, do not free.
    free(static_cast<void *>(params));
}

void FFICJWantAddEntity(WantHandle want, const char* entity)
{
    auto actualWant = reinterpret_cast<Want*>(want);
    actualWant->AddEntity(entity);
}

WantHandle FFICJWantParseUri(const char* uri)
{
    return Want::ParseUri(uri);
}
