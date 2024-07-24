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
 
#include "cj_application_context.h"

#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_utils_ffi.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace ApplicationContextCJ {
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;

int CJApplicationContext::GetArea()
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "application context is null.");
        return INVALID_CODE;
    }
    return context->GetArea();
}

std::shared_ptr<AppExecFwk::ApplicationInfo> CJApplicationContext::GetApplicationInfo()
{
    auto context = applicationContext_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "application context is null.");
        return nullptr;
    }
    return context->GetApplicationInfo();
}

extern "C" {
int64_t FFIGetArea(int64_t id)
{
    auto context = FFI::FFIData::GetData<CJApplicationContext>(id);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj application context is null.");
        return INVALID_CODE;
    }
    return context->GetArea();
}

CApplicationInfo* FFICJApplicationInfo(int64_t id)
{
    auto context = FFI::FFIData::GetData<CJApplicationContext>(id);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "cj application context is null.");
        return nullptr;
    }
    auto appInfo = context->GetApplicationInfo();
    CApplicationInfo* buffer = static_cast<CApplicationInfo*>(malloc(sizeof(CApplicationInfo)));
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "fail to malloc appinfo.");
        return nullptr;
    }
    buffer->name = CreateCStringFromString(appInfo->name);
    buffer->bundleName = CreateCStringFromString(appInfo->bundleName);
    return buffer;
}
}
}
}