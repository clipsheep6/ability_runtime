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

#include "configuration_utils.h"

#include "configuration_convertor.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#ifdef SUPPORT_GRAPHICS
#include "window.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
using namespace AppExecFwk;

void ConfigurationUtils::UpdateGlobalConfig(const Configuration &configuration,
    std::shared_ptr<ResourceManager> resourceManager)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "Enter");
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Resource manager is invalid.");
        return;
    }

    ResourceConfigHelper resourceConfig;
    GetGlobalConfig(configuration, resourceConfig);
    resourceConfig.UpdateResConfig(configuration, resourceManager);
}

void ConfigurationUtils::GetGlobalConfig(const Configuration &configuration,
    OHOS::AbilityRuntime::ResourceConfigHelper &resourceConfig)
{
    resourceConfig.SetLanguage(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE));
    resourceConfig.SetColormode(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE));
    resourceConfig.SetHasPointerDevice(configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE));
    resourceConfig.SetMcc(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC));
    resourceConfig.SetMnc(configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC));
    resourceConfig.SetColorModeIsSetByApp(
        configuration.GetItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP));
    resourceConfig.SetThemeId(configuration.GetItem(AAFwk::GlobalConfigurationKey::THEME_ID));
}

#ifdef SUPPORT_GRAPHICS
void ConfigurationUtils::InitDisplayConfig(Rosen::DisplayId displayId, std::shared_ptr<Configuration> configuration,
    std::shared_ptr<ResourceManager> resourceManager)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Init display config.");
    if (configuration == nullptr || resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Input invalid.");
        return;
    }

    float density;
    std::string direction;
    if (!GetDisplayConfig(displayId, density, direction)) {
        return;
    }

    configuration->AddItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));
    configuration->AddItem(displayId, ConfigurationInner::APPLICATION_DIRECTION, direction);
    configuration->AddItem(ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(displayId));

    UpdateDisplayResConfig(resourceManager, density, direction);
}

void ConfigurationUtils::UpdateDisplayConfig(Rosen::DisplayId displayId, std::shared_ptr<Configuration> configuration,
    std::shared_ptr<ResourceManager> resourceManager, bool &configChanged)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Update display config.");
    if (configuration == nullptr || resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Input invalid.");
        return;
    }

    float density;
    std::string direction;
    if (!GetDisplayConfig(displayId, density, direction)) {
        return;
    }

    Configuration newConfig;
    newConfig.AddItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));
    newConfig.AddItem(displayId, ConfigurationInner::APPLICATION_DIRECTION, direction);

    std::vector<std::string> changeKeyV;
    configuration->CompareDifferent(changeKeyV, newConfig);
    if (changeKeyV.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY, "There's no changed config, return.");
        return;
    }
    configuration->Merge(changeKeyV, newConfig);
    configChanged = true;

    UpdateDisplayResConfig(resourceManager, density, direction);

    auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
    TAG_LOGI(AAFwkTag::ABILITY, "Update display config %{public}s for all windows.",
        diffConfiguration->GetName().c_str());
    Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
}

bool ConfigurationUtils::GetDisplayConfig(Rosen::DisplayId displayId, float &density,
    std::string &directionStr)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Get display by id %{public}" PRIu64".", displayId);
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Get display %{public}" PRIu64" failed.", displayId);
        return false;
    }

    density = display->GetVirtualPixelRatio();
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    directionStr = GetDirectionStr(height, width);
    TAG_LOGD(AAFwkTag::ABILITY, "DisplayId: %{public}" PRIu64", density: %{public}f, direction: %{public}s.",
        displayId, density, directionStr.c_str());
    return true;
}

void ConfigurationUtils::UpdateDisplayResConfig(std::shared_ptr<ResourceManager> resourceManager,
    float &density, std::string &direction)
{
    // resourceManager has checked in caller function.
    TAG_LOGD(AAFwkTag::ABILITY, "Update resConfig, density: %{public}f, direction: %{public}s.",
        density, direction.c_str());
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Create resConfig failed.");
        return;
    }

    resourceManager->GetResConfig(*resConfig);
    resConfig->SetScreenDensity(density);
    resConfig->SetDirection(ConvertDirection(direction));
    resourceManager->UpdateResConfig(*resConfig);
    TAG_LOGD(AAFwkTag::ABILITY, "Update resConfig finished, density: %{public}f, direction: %{public}d.",
        resConfig->GetScreenDensity(), resConfig->GetDirection());
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
