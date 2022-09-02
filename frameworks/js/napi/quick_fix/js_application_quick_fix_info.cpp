/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_APPLICATION_QUICK_FIX_INFO_H
#define OHOS_ABILITY_RUNTIME_JS_APPLICATION_QUICK_FIX_INFO_H

#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue *CreateJsApplicationQuickFixInfo(NativeEngine *engine, AppqfInfo& appQuickFixInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    object->SetProperty("quickFixVersionCode", CreateJsValue(engine, appQuickFixInfo.versionCode));
    object->SetProperty("quickFixVersionName", CreateJsValue(engine, appQuickFixInfo.versionName));
    object->SetProperty("hapModuleQuickFixInfo", CreateJsHapModuleQuickFixInfoArray(engine, appQuickFixInfo.hqfInfos));
    return objValue;
}

NativeValue *CreateJsHapModuleQuickFixInfoArray(NativeEngine *engine, std::vector<AppExecFwk::HqfInfo>& hqfInfos)
{
    NativeValue* arrayValue = engine.CreateArray(hqfInfos.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &hqfInfo : hqfInfos) {
        NativeValue* objValue = engine.CreateObject();
        NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
        object->SetProperty("moduleName", CreateJsValue(engine, hqfInfo.moduleName));
        object->SetProperty("moduleSha256", CreateJsValue(engine, hqfInfo.hapSha256));
        object->SetProperty("quickFixFilePath", CreateJsValue(engine, hqfInfo.hqfFilePath));
        array->SetElement(index++, objValue);
    }
    return arrayValue;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_APPLICATION_QUICK_FIX_INFO_H