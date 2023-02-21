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

#include "configuration_utils.h"

#include "configuration_convertor.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void ConfigurationUtils::UpdateConfigToResourceManager(const AppExecFwk::Configuration &configuration,
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager)
{
    HILOG_DEBUG("Enter");
    if (resourceManager == nullptr) {
        HILOG_ERROR("Resource manager is invalid.");
        return;
    }

    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    GetConfigurationProperties(configuration, language, colormode, hasPointerDevice);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("Create resource config failed.");
        return;
    }
    resourceManager->GetResConfig(*resConfig);

#ifdef SUPPORT_GRAPHICS
    if (!language.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        icu::Locale locale = icu::Locale::forLanguageTag(language, status);
        HILOG_DEBUG("get Locale::forLanguageTag return[%{public}d].", static_cast<int>(status));
        if (status == U_ZERO_ERROR) {
            resConfig->SetLocaleInfo(locale);
        }

        const icu::Locale *localeInfo = resConfig->GetLocaleInfo();
        if (localeInfo != nullptr) {
            HILOG_DEBUG("Update config, language: %{public}s, script: %{public}s, region: %{public}s",
                localeInfo->getLanguage(), localeInfo->getScript(), localeInfo->getCountry());
        }
    }
#endif

    if (!colormode.empty()) {
        resConfig->SetColorMode(AppExecFwk::ConvertColorMode(colormode));
        HILOG_DEBUG("Update config, colorMode: %{public}d", resConfig->GetColorMode());
    }

    if (!hasPointerDevice.empty()) {
        resConfig->SetInputDevice(AppExecFwk::ConvertHasPointerDevice(hasPointerDevice));
        HILOG_DEBUG("Update config, hasPointerDevice: %{public}d", resConfig->GetInputDevice());
    }

    Global::Resource::RState ret = resourceManager->UpdateResConfig(*resConfig);
    if (ret != Global::Resource::RState::SUCCESS) {
        HILOG_ERROR("Update resource config failed with %{public}d.", static_cast<int>(ret));
        return;
    }

    HILOG_DEBUG("Update resource config succeed.");
}

void ConfigurationUtils::GetConfigurationProperties(const AppExecFwk::Configuration &configuration,
    std::string &language, std::string &colormode, std::string &hasPointerDevice)
{
    language = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
    colormode = configuration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
    hasPointerDevice = configuration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
}
} // namespace AbilityRuntime
} // namespace OHOS
