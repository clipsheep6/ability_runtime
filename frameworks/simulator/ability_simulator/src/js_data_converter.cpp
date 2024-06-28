/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_data_converter.h"

#include "common_func.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
Global::Resource::ColorMode ConvertColorMode(const std::string &colormode)
{
    auto resolution = Global::Resource::ColorMode::COLOR_MODE_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ColorMode>> resolutions = {
        { "dark", Global::Resource::ColorMode::DARK },
        { "light", Global::Resource::ColorMode::LIGHT },
    };

    for (const auto &[tempColorMode, value] : resolutions) {
        if (tempColorMode == colormode) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

int32_t ConvertDisplayId(const std::string &displayId)
{
    if (displayId == AppExecFwk::ConfigurationInner::EMPTY_STRING) {
        return -1;
    }

    return std::stoi(displayId);
}

Global::Resource::ScreenDensity ConvertDensity(const std::string &density)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "ConvertDensity is called");
    auto resolution = Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ScreenDensity>> resolutions = {
        { "sdpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI },
        { "mdpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI },
        { "ldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI },
        { "xldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI },
        { "xxldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI },
        { "xxxldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI },
    };

    for (const auto &[tempdensity, value] : resolutions) {
        if (tempdensity == density) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::Direction ConvertDirection(const std::string &direction)
{
    auto resolution = Global::Resource::Direction::DIRECTION_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::Direction>> resolutions = {
        { "vertical", Global::Resource::Direction::DIRECTION_VERTICAL },
        { "horizontal", Global::Resource::Direction::DIRECTION_HORIZONTAL },
    };

    for (const auto &[tempDirection, value] : resolutions) {
        if (tempDirection == direction) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

napi_value CreateJsConfiguration(napi_env env, const AppExecFwk::Configuration &configuration)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Native object is nullptr.");
        return object;
    }

    napi_set_named_property(env, object, "language", CreateJsValue(env,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE)));
    
    napi_set_named_property(env, object, "colorMode", CreateJsValue(env,
        ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE))));

    std::string direction = configuration.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION);
    napi_set_named_property(env, object, "direction", CreateJsValue(env, ConvertDirection(direction)));

    std::string density = configuration.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI);
    napi_set_named_property(env, object, "screenDensity", CreateJsValue(env, ConvertDensity(density)));

    int32_t displayId = ConvertDisplayId(configuration.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID));
    napi_set_named_property(env, object, "displayId", CreateJsValue(env, displayId));

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    napi_set_named_property(
        env, object, "hasPointerDevice", CreateJsValue(env, hasPointerDevice == "true" ? true : false));

    return object;
}

napi_value CreateJsApplicationInfo(napi_env env, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertApplicationInfo(env, object, applicationInfo);
    return object;
}

napi_value CreateJsHapModuleInfo(napi_env env, const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertHapModuleInfo(env, hapModuleInfo, object);
    return object;
}

napi_value CreateJsAbilityInfo(napi_env env, const AppExecFwk::AbilityInfo &abilityInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertAbilityInfo(env, abilityInfo, object);
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS
