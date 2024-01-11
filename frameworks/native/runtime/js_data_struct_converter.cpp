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
napi_value CreateJsWantObject(napi_env env, const AAFwk::Want& want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "deviceId", CreateJsValue(env, want.GetOperation().GetDeviceId()));
    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, want.GetBundle()));
    napi_set_named_property(env, object, "abilityName", CreateJsValue(env, want.GetOperation().GetAbilityName()));
    napi_set_named_property(env, object, "uri", CreateJsValue(env, want.GetUriString()));
    napi_set_named_property(env, object, "type", CreateJsValue(env, want.GetType()));
    napi_set_named_property(env, object, "flags", CreateJsValue(env, want.GetFlags()));
    napi_set_named_property(env, object, "action", CreateJsValue(env, want.GetAction()));
    napi_set_named_property(env, object, "entities", CreateNativeArray(env, want.GetEntities()));
    return object;
}

napi_value CreateJsAbilityInfo(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertAbilityInfo(env, abilityInfo, object);
    return object;
}

napi_value CreateJsApplicationInfo(napi_env env, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertApplicationInfo(env, object, applicationInfo);
    return object;
}

napi_value CreateJsLaunchParam(napi_env env, const AAFwk::LaunchParam& launchParam)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return nullptr;
    }
    napi_set_named_property(env, object, "launchReason", CreateJsValue(env, launchParam.launchReason));
    napi_set_named_property(env, object, "lastExitReason", CreateJsValue(env, launchParam.lastExitReason));
    return object;
}

napi_value CreateJsConfiguration(napi_env env, const AppExecFwk::Configuration& configuration)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return nullptr;
    }

    napi_set_named_property(env, object, "language", CreateJsValue(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE)));
    napi_set_named_property(env, object, "colorMode", CreateJsValue(env,
        ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE))));
    
    napi_set_named_property(env, object, "time24", CreateJsValue(env,
        ConvertTimeFormat(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_HOUR))));

    int32_t displayId = ConvertDisplayId(configuration.GetItem(ConfigurationInner::APPLICATION_DISPLAYID));
    std::string direction = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    napi_set_named_property(env, object, "direction", CreateJsValue(env, ConvertDirection(direction)));

    std::string density = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    napi_set_named_property(env, object, "screenDensity", CreateJsValue(env, ConvertDensity(density)));
    napi_set_named_property(env, object, "displayId", CreateJsValue(env, displayId));

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    napi_set_named_property(env, object, "hasPointerDevice",
        CreateJsValue(env, hasPointerDevice == "true" ? true : false));
    return object;
}

napi_value CreateJsExtensionAbilityInfo(napi_env env, const AppExecFwk::ExtensionAbilityInfo& info)
{
    HILOG_DEBUG("CreateJsExtensionAbilityInfo begin");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertExtensionInfo(env, info, object);
    return object;
}

napi_value CreateJsHapModuleInfo(napi_env env, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }
    AppExecFwk::CommonFunc::ConvertHapModuleInfo(env, hapModuleInfo, object);
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS
