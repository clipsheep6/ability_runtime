/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_ability_auto_startup_manager_utils.h"

namespace OHOS {
namespace AbilityRuntime {

bool UnwrapAutoStartupInfo(NativeEngine &engine, NativeValue *param, AutoStartupInfo &info)
{
    if (!IsNormalObject(param)) {
        HILOG_ERROR("param is invalid.");
        return false;
    }

    NativeObject *infoObj = ConvertNativeValueTo<NativeObject>(param);
    if (infoObj == nullptr) {
        HILOG_ERROR("infoObj is invalid.");
        return false;
    }

    UnwrapStringValue(infoObj->GetProperty("bundleName"), info.bundleName);
    UnwrapStringValue(infoObj->GetProperty("abilityName"), info.abilityName);
    UnwrapStringValue(infoObj->GetProperty("moduleName"), info.moduleName);
    return true;
}

bool UnwrapStringValue(NativeValue *param, std::string &value)
{
    if (param == nullptr) {
        HILOG_ERROR("param is nullptr.");
        return false;
    }
    if (param->TypeOf() != NativeValueType::NATIVE_STRING) {
        return false;
    }

    auto nativeString = ConvertNativeValueTo<NativeString>(param);
    size_t size = 0;
    nativeString->GetCString(nullptr, 0, &size);
    if (size == 0 || size >= INT_MAX) {
        HILOG_ERROR("String size abnormal: %{public}zu.", size);
        return true;
    }

    value.resize(size + 1);
    nativeString->GetCString(value.data(), size + 1, &size);
    value.pop_back();
    return true;
}

bool IsNormalObject(NativeValue *value)
{
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return false;
    }
    if (value->TypeOf() == NativeValueType::NATIVE_UNDEFINED) {
        HILOG_ERROR("value is undefined.");
        return false;
    }
    if (value->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("Invalid type.");
        return false;
    }
    return true;
}

NativeValue *CreateJsAutoStartupInfoArray(NativeEngine &engine, const std::vector<AutoStartupInfo> &infoList)
{
    HILOG_DEBUG("Called.");
    NativeValue *arrayValue = engine.CreateArray(infoList.size());
    NativeArray *array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &info : infoList) {
        array->SetElement(index++, CreateJsAutoStartupInfo(engine, info));
    }
    return arrayValue;
}

NativeValue *CreateJsAutoStartupInfo(NativeEngine &engine, const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called.");
    NativeValue *objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("objValue is nullptr.");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return nullptr;
    }
    object->SetProperty("bundleName", CreateJsValue(engine, info.bundleName));
    object->SetProperty("abilityName", CreateJsValue(engine, info.abilityName));
    object->SetProperty("moduleName", CreateJsValue(engine, info.moduleName));
    object->SetProperty("abilityTypeName", CreateJsValue(engine, info.abilityTypeName));
    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS