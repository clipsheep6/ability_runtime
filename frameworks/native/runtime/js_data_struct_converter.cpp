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

#include "js_data_struct_converter.h"

#include "common_func.h"
#include "configuration_convertor.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
NativeValue* CreateJsWantObject(NativeEngine& engine, const AAFwk::Want& want)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }

    object->SetProperty("deviceId", CreateJsValue(engine, want.GetOperation().GetDeviceId()));
    object->SetProperty("bundleName", CreateJsValue(engine, want.GetBundle()));
    object->SetProperty("abilityName", CreateJsValue(engine, want.GetOperation().GetAbilityName()));
    object->SetProperty("uri", CreateJsValue(engine, want.GetUriString()));
    object->SetProperty("type", CreateJsValue(engine, want.GetType()));
    object->SetProperty("flags", CreateJsValue(engine, want.GetFlags()));
    object->SetProperty("action", CreateJsValue(engine, want.GetAction()));
    object->SetProperty("entities", CreateNativeArray(engine, want.GetEntities()));
    return objValue;
}

NativeValue* CreateJsAbilityInfo(NativeEngine& engine, const AppExecFwk::AbilityInfo& abilityInfo)
{
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertAbilityInfo(reinterpret_cast<napi_env>(&engine), abilityInfo,
        reinterpret_cast<napi_value>(objValue));
    return objValue;
}

NativeValue* CreateJsApplicationInfo(NativeEngine& engine, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertApplicationInfo(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(objValue), applicationInfo);
    return objValue;
}

NativeValue* CreateJsLaunchParam(NativeEngine& engine, const AAFwk::LaunchParam& launchParam)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }

    object->SetProperty("launchReason", CreateJsValue(engine, launchParam.launchReason));
    object->SetProperty("lastExitReason", CreateJsValue(engine, launchParam.lastExitReason));

    return objValue;
}

NativeValue* CreateJsConfiguration(NativeEngine& engine, const AppExecFwk::Configuration& configuration)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }

    object->SetProperty("language", CreateJsValue(engine,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE)));
    object->SetProperty("colorMode", CreateJsValue(engine,
        ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE))));

    int32_t displayId = ConvertDisplayId(configuration.GetItem(ConfigurationInner::APPLICATION_DISPLAYID));

    std::string direction = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    object->SetProperty("direction", CreateJsValue(engine, ConvertDirection(direction)));

    std::string density = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    object->SetProperty("screenDensity", CreateJsValue(engine, ConvertDensity(density)));

    object->SetProperty("displayId", CreateJsValue(engine, displayId));

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    object->SetProperty("hasPointerDevice", CreateJsValue(engine, hasPointerDevice == "true" ? true : false));

    return objValue;
}

NativeValue* CreateJsExtensionAbilityInfo(NativeEngine& engine, const AppExecFwk::ExtensionAbilityInfo& info)
{
    HILOG_DEBUG("CreateJsExtensionAbilityInfo begin");
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertExtensionInfo(reinterpret_cast<napi_env>(&engine), info,
        reinterpret_cast<napi_value>(objValue));
    return objValue;
}

NativeValue* CreateJsHapModuleInfo(NativeEngine& engine, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertHapModuleInfo(reinterpret_cast<napi_env>(&engine), hapModuleInfo,
        reinterpret_cast<napi_value>(objValue));
    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
