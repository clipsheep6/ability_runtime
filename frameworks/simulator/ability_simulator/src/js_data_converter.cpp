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

#include "js_data_converter.h"

#include "common_func.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr float DPI_BASE = 160.0;
}
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

NativeValue *CreateJsConfiguration(NativeEngine &engine, const AppExecFwk::Configuration &configuration)
{
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }

    object->SetProperty("language", CreateJsValue(engine,
        configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE)));
    object->SetProperty("colorMode", CreateJsValue(engine,
        ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE))));

    int32_t displayId = ConvertDisplayId(configuration.GetItem(AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID));

    std::string direction = configuration.GetItem(displayId, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION);
    object->SetProperty("direction", CreateJsValue(engine, ConvertDirection(direction)));

    std::string density = configuration.GetItem(displayId, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI);
    object->SetProperty("screenDensity", CreateJsValue(engine, ConvertDensity(density)));

    object->SetProperty("displayId", CreateJsValue(engine, displayId));

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    object->SetProperty("hasPointerDevice", CreateJsValue(engine, hasPointerDevice == "true" ? true : false));

    return objValue;
}

NativeValue *CreateJsApplicationInfo(NativeEngine &engine, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    NativeValue *objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertApplicationInfo(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(objValue), applicationInfo);
    return objValue;
}

NativeValue *CreateJsHapModuleInfo(NativeEngine &engine, const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    NativeValue *objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertHapModuleInfo(reinterpret_cast<napi_env>(&engine), hapModuleInfo,
        reinterpret_cast<napi_value>(objValue));
    return objValue;
}

NativeValue *CreateJsAbilityInfo(NativeEngine &engine, const AppExecFwk::AbilityInfo &abilityInfo)
{
    NativeValue *objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    AppExecFwk::CommonFunc::ConvertAbilityInfo(reinterpret_cast<napi_env>(&engine), abilityInfo,
        reinterpret_cast<napi_value>(objValue));
    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
