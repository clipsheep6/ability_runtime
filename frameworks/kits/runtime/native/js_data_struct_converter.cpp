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

#include "js_data_struct_converter.h"

#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsWantObject(NativeEngine& engine, const AAFwk::Want& want)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("deviceId", CreateJsValue(engine, want.GetOperation().GetDeviceId()));
    object->SetProperty("bundleName", CreateJsValue(engine, want.GetBundle()));
    object->SetProperty("abilityName", CreateJsValue(engine, want.GetOperation().GetAbilityName()));
    object->SetProperty("uri", CreateJsValue(engine, want.GetUriString()));
    object->SetProperty("type", CreateJsValue(engine, want.GetType()));
    object->SetProperty("flags", CreateJsValue(engine, want.GetFlags()));
    object->SetProperty("action", CreateJsValue(engine, want.GetAction()));
    // object->SetProperty("parameters");

    object->SetProperty("entities", CreateNativeArray(engine, want.GetEntities()));
    return objValue;
}

NativeValue* CreateJsAbilityInfo(NativeEngine& engine, const AppExecFwk::AbilityInfo& abilityInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("bundleName", CreateJsValue(engine, abilityInfo.bundleName));
    object->SetProperty("name", CreateJsValue(engine, abilityInfo.name));
    object->SetProperty("label", CreateJsValue(engine, abilityInfo.label));
    object->SetProperty("description", CreateJsValue(engine, abilityInfo.description));
    object->SetProperty("icon", CreateJsValue(engine, abilityInfo.iconPath));
    object->SetProperty("labelId", CreateJsValue(engine, abilityInfo.labelId));
    object->SetProperty("descriptionId", CreateJsValue(engine, abilityInfo.descriptionId));
    object->SetProperty("iconId", CreateJsValue(engine, abilityInfo.iconId));
    object->SetProperty("moduleName", CreateJsValue(engine, abilityInfo.moduleName));
    object->SetProperty("process", CreateJsValue(engine, abilityInfo.process));
    object->SetProperty("targetAbility", CreateJsValue(engine, abilityInfo.targetAbility));
    object->SetProperty("backgroundModes", CreateJsValue(engine, abilityInfo.backgroundModes));
    object->SetProperty("isVisible", CreateJsValue(engine, abilityInfo.visible));
    object->SetProperty("formEnabled", CreateJsValue(engine, abilityInfo.formEnabled));

    object->SetProperty("type", CreateJsValue(engine, abilityInfo.type));
    object->SetProperty("subType", CreateJsValue(engine, abilityInfo.subType));
    object->SetProperty("orientation", CreateJsValue(engine, abilityInfo.orientation));
    object->SetProperty("launchMode", CreateJsValue(engine, abilityInfo.launchMode));

    object->SetProperty("permissions", CreateNativeArray(engine, abilityInfo.permissions));
    object->SetProperty("deviceTypes", CreateNativeArray(engine, abilityInfo.deviceTypes));
    object->SetProperty("deviceCapabilities", CreateNativeArray(engine, abilityInfo.deviceCapabilities));

    object->SetProperty("readPermission", CreateJsValue(engine, abilityInfo.readPermission));
    object->SetProperty("writePermission", CreateJsValue(engine, abilityInfo.writePermission));

    object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, abilityInfo.applicationInfo));

    object->SetProperty("formEntity", CreateJsValue(engine, abilityInfo.formEntity));
    object->SetProperty("minFormHeight", CreateJsValue(engine, abilityInfo.minFormHeight));
    object->SetProperty("defaultFormHeight", CreateJsValue(engine, abilityInfo.defaultFormHeight));
    object->SetProperty("minFormWidth", CreateJsValue(engine, abilityInfo.minFormWidth));
    object->SetProperty("defaultFormWidth", CreateJsValue(engine, abilityInfo.defaultFormWidth));
    object->SetProperty("uri", CreateJsValue(engine, abilityInfo.uri));
    // customizeData: Map<string, Array<CustomizeData>>;

    return objValue;
}

NativeValue* CreateJsModuleInfo(NativeEngine& engine, const AppExecFwk::ModuleInfo &moduleInfo)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("moduleName", CreateJsValue(engine, moduleInfo.moduleName));
    object->SetProperty("moduleSourceDir", CreateJsValue(engine, moduleInfo.moduleSourceDir));

    return objValue;
}

NativeValue* CreateJsApplicationInfo(NativeEngine& engine, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("name", CreateJsValue(engine, applicationInfo.name));
    object->SetProperty("description", CreateJsValue(engine, applicationInfo.description));
    object->SetProperty("descriptionId", CreateJsValue(engine, applicationInfo.descriptionId));
    object->SetProperty("systemApp", CreateJsValue(engine, applicationInfo.isSystemApp));
    object->SetProperty("enabled", CreateJsValue(engine, applicationInfo.enabled));
    object->SetProperty("label", CreateJsValue(engine, applicationInfo.label));
    object->SetProperty("labelId", CreateJsValue(engine, std::to_string(applicationInfo.labelId)));
    object->SetProperty("icon", CreateJsValue(engine, applicationInfo.icon));
    object->SetProperty("iconId", CreateJsValue(engine, std::to_string(applicationInfo.iconId)));
    object->SetProperty("process", CreateJsValue(engine, applicationInfo.process));
    object->SetProperty("supportedModes", CreateJsValue(engine, applicationInfo.supportedModes));
    object->SetProperty("entryDir", CreateJsValue(engine, applicationInfo.entryDir));
    object->SetProperty("codePath", CreateJsValue(engine, applicationInfo.codePath));
    object->SetProperty("removable", CreateJsValue(engine, applicationInfo.removable));

    object->SetProperty("moduleSourceDirs", CreateNativeArray(engine, applicationInfo.moduleSourceDirs));
    object->SetProperty("permissions", CreateNativeArray(engine, applicationInfo.permissions));

    NativeValue *arrayValue = engine.CreateArray(applicationInfo.moduleInfos.size());
    NativeArray *array = ConvertNativeValueTo<NativeArray>(arrayValue);
    if (array != nullptr) {
        uint32_t index = 0;
        for (auto entity : applicationInfo.moduleInfos) {
            array->SetElement(index++, CreateJsModuleInfo(engine, entity));
        }
    }
    object->SetProperty("moduleInfo", arrayValue);

    return objValue;
}

NativeValue* CreateJsLaunchParam(NativeEngine& engine, const AAFwk::LaunchParam& launchParam)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    object->SetProperty("launchReason", CreateJsValue(engine, launchParam.launchReason));
    object->SetProperty("lastExitReason", CreateJsValue(engine, launchParam.lastExitReason));

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
