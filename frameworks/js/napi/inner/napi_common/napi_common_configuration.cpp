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

#include "napi_common_configuration.h"

#include "configuration_convertor.h"
#include "hilog_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr double FONT_SIZE_MIN_SCALE = 0.0;
constexpr double FONT_SIZE_MAX_SCALE = 3.2;
constexpr double FONT_WEIGHT_MIN_SCALE = 0.0;
constexpr double FONT_WEIGHT_MAX_SCALE = 1.25;
}

EXTERN_C_START

bool InnerWrapConfigurationString(
    napi_env env, napi_value jsObject, const std::string &key, const std::string &value)
{
    if (!value.empty()) {
        HILOG_INFO("%{public}s called. key=%{public}s, value=%{private}s", __func__, key.c_str(), value.c_str());
        napi_value jsValue = WrapStringToJS(env, value);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

napi_value WrapConfiguration(napi_env env, const AppExecFwk::Configuration &configuration)
{
    HILOG_DEBUG("called, config size %{public}d", static_cast<int>(configuration.GetItemSize()));
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    napi_value jsValue = nullptr;
    jsValue = WrapStringToJS(env, configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
    SetPropertyValueByPropertyName(env, jsObject, "language", jsValue);

    jsValue = WrapInt32ToJS(
        env, ConvertColorMode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE)));
    SetPropertyValueByPropertyName(env, jsObject, "colorMode", jsValue);

    int32_t displayId = ConvertDisplayId(configuration.GetItem(ConfigurationInner::APPLICATION_DISPLAYID));

    std::string direction = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DIRECTION);
    jsValue = WrapInt32ToJS(env, ConvertDirection(direction));
    SetPropertyValueByPropertyName(env, jsObject, "direction", jsValue);

    std::string density = configuration.GetItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI);
    jsValue = WrapInt32ToJS(env, ConvertDensity(density));
    SetPropertyValueByPropertyName(env, jsObject, "screenDensity", jsValue);

    jsValue = WrapInt32ToJS(env, displayId);
    SetPropertyValueByPropertyName(env, jsObject, "displayId", jsValue);

    std::string hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
    jsValue = WrapBoolToJS(env, hasPointerDevice == "true" ? true : false);
    SetPropertyValueByPropertyName(env, jsObject, "hasPointerDevice", jsValue);

    std::string fontSizeScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE);
    jsValue = WrapDoubleToJS(env, fontSizeScale != "" ? std::stod(fontSizeScale) : 1.0);
    SetPropertyValueByPropertyName(env, jsObject, "fontSizeScale", jsValue);
    
    std::string fontWeightScale = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE);
    jsValue = WrapDoubleToJS(env, fontWeightScale != "" ? std::stod(fontWeightScale) : 1.0);
    SetPropertyValueByPropertyName(env, jsObject, "fontWeightScale", jsValue);

    return jsObject;
}

bool UnwrapConfiguration(napi_env env, napi_value param, Configuration &config)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return false;
    }

    std::string language {""};
    if (UnwrapStringByPropertyName(env, param, "language", language)) {
        HILOG_DEBUG("The parsed language part %{public}s", language.c_str());
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language)) {
            HILOG_ERROR("language Parsing failed");
            return false;
        }
    }

    int32_t colormode = -1;
    if (UnwrapInt32ByPropertyName(env, param, "colorMode", colormode)) {
        HILOG_DEBUG("The parsed colormode part %{public}d", colormode);
        if (colormode != Global::Resource::DARK && colormode != Global::Resource::LIGHT) {
            HILOG_ERROR("Set colorMode to unsupported value.");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, GetColorModeStr(colormode))) {
            HILOG_ERROR("colorMode parsing failed");
            return false;
        }
    }

    double fontSizeScale = 0.0;
    if (UnwrapDoubleByPropertyName(env, param, "fontSizeScale", fontSizeScale)) {
        HILOG_DEBUG("The parsed fontSizeScale part %{public}lf", fontSizeScale);
        if (fontSizeScale < FONT_SIZE_MIN_SCALE || fontSizeScale > FONT_SIZE_MAX_SCALE) {
            HILOG_ERROR("invalid fontSizeScale");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, std::to_string(fontSizeScale))) {
            return false;
        }
    }

    double fontWeightScale = 0.0;
    if (UnwrapDoubleByPropertyName(env, param, "fontWeightScale", fontWeightScale)) {
        HILOG_DEBUG("The parsed fontWeightScale part %{public}lf", fontWeightScale);
        if (fontWeightScale < FONT_WEIGHT_MIN_SCALE || fontWeightScale > FONT_WEIGHT_MAX_SCALE) {
            HILOG_ERROR("invalid fontWeightScale");
            return false;
        }
        if (!config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE, std::to_string(fontWeightScale))) {
            return false;
        }
    }

    return true;
}
EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS
