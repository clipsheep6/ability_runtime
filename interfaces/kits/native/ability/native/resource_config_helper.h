/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RESOURCE_CONFIG_HELPER_H
#define OHOS_ABILITY_RUNTIME_RESOURCE_CONFIG_HELPER_H
#include "resource_manager.h"
#include "configuration.h"
#ifdef SUPPORT_GRAPHICS
#include "display_manager.h"
#endif

namespace OHOS {
namespace AppExecFwk {

}
namespace AbilityRuntime {
class ResourceConfigHelper {
public:
    ResourceConfigHelper() = default;
    ~ResourceConfigHelper() = default;
    std::string GetLanguage();
    void SetLanguage(std::string language);
    std::string GetColormode();
    void SetColormode(std::string colormode);
    std::string GetHasPointerDevice();
    void SetHasPointerDevice(std::string hasPointerDevice);
    std::string GetMcc();
    void SetMcc(std::string mcc);
    std::string GetMnc();
    void SetMnc(std::string mnc);
    std::string GetColorModeIsSetByApp();
    void SetColorModeIsSetByApp(std::string colorModeIsSetByApp);

    void UpdateResConfig(const AppExecFwk::Configuration &configuration,
        std::shared_ptr<Global::Resource::ResourceManager> resourceManager);

    static bool ConvertStringToUint32(std::string source, uint32_t &result);

private:
    std::string language_;
    std::string colormode_;
    std::string hasPointerDevice_;
    std::string mcc_;
    std::string mnc_;
    std::string colorModeIsSetByApp_;

    void UpdateResConfig(std::unique_ptr<Global::Resource::ResConfig> &resConfig);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RESOURCE_CONFIG_HELPER_H
