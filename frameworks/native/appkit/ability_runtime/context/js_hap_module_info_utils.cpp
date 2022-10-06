/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "js_hap_module_info_utils.h"

#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsHapModuleInfo(NativeEngine& engine, AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        return objValue;
    }

    object->SetProperty("name", CreateJsValue(engine, hapModuleInfo.name));
    object->SetProperty("moduleName", CreateJsValue(engine, hapModuleInfo.moduleName));
    object->SetProperty("description", CreateJsValue(engine, hapModuleInfo.description));
    object->SetProperty("descriptionId", CreateJsValue(engine, hapModuleInfo.descriptionId));
    object->SetProperty("icon", CreateJsValue(engine, hapModuleInfo.iconPath));
    object->SetProperty("iconId", CreateJsValue(engine, hapModuleInfo.iconId));
    object->SetProperty("label", CreateJsValue(engine, hapModuleInfo.label));
    object->SetProperty("labelId", CreateJsValue(engine, hapModuleInfo.labelId));
    object->SetProperty("backgroundImg", CreateJsValue(engine, hapModuleInfo.backgroundImg));
    object->SetProperty("mainAbilityName", CreateJsValue(engine, hapModuleInfo.mainAbility));
    object->SetProperty("supportedModes", CreateJsValue(engine, hapModuleInfo.supportedModes));
    object->SetProperty("installationFree", CreateJsValue(engine, hapModuleInfo.installationFree));
    object->SetProperty("mainElementName", CreateJsValue(engine, hapModuleInfo.mainElementName));
    object->SetProperty("hashValue", CreateJsValue(engine, hapModuleInfo.hashValue));

    SetProperty(engine, object, hapModuleInfo.reqCapabilities, "reqCapabilities");
    SetProperty(engine, object, hapModuleInfo.deviceTypes, "deviceTypes");

    NativeValue *abilityArrayValue = engine.CreateArray(hapModuleInfo.abilityInfos.size());
    NativeArray *abilityArray = ConvertNativeValueTo<NativeArray>(abilityArrayValue);
    if (abilityArray != nullptr) {
        int index = 0;
        for (auto abilityInfo : hapModuleInfo.abilityInfos) {
            abilityArray->SetElement(index++, CreateJsAbilityInfo(engine, abilityInfo));
        }
    }
    object->SetProperty("abilityInfo", abilityArrayValue);

    NativeValue *extensionArrayValue = engine.CreateArray(hapModuleInfo.extensionInfos.size());
    NativeArray *extensionArray = ConvertNativeValueTo<NativeArray>(extensionArrayValue);
    if (extensionArray != nullptr) {
        int index = 0;
        for (auto extensionInfo : hapModuleInfo.extensionInfos) {
            extensionArray->SetElement(index++, CreateJsExtensionAbilityInfo(engine, extensionInfo));
        }
    }
    object->SetProperty("extensionAbilityInfo", extensionArrayValue);

    NativeValue *metadataArrayValue = engine.CreateArray(hapModuleInfo.metadata.size());
    NativeArray *metadataArray = ConvertNativeValueTo<NativeArray>(metadataArrayValue);
    if (metadataArray != nullptr) {
        int index = 0;
        for (auto metadata : hapModuleInfo.metadata) {
            metadataArray->SetElement(index++, CreateJsMetadata(engine, metadata));
        }
    }
    object->SetProperty("metadata", metadataArrayValue);

    return objValue;
}

void SetProperty(NativeEngine &engine, NativeObject* &object, const std::vector<std::string> &properties,
    const std::string &proName)
{
    NativeValue *arrayValue = engine.CreateArray(properties.size());
    NativeArray *array = ConvertNativeValueTo<NativeArray>(arrayValue);
    if (array != nullptr) {
        int index = 0;
        for (auto property : properties) {
            array->SetElement(index++, CreateJsValue(engine, property));
        }
    }
    object->SetProperty(proName.c_str(), arrayValue);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
