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

#ifndef FOUNDATION_ABILITY_RUNTIME_SIMULATOR_OPTIONS_H
#define FOUNDATION_ABILITY_RUNTIME_SIMULATOR_OPTIONS_H

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ability_info.h"
#include "application_info.h"
#include "configuration.h"
#include "hap_module_info.h"

namespace OHOS {
namespace AbilityRuntime {
enum class DeviceType {
    PHONE,
    TV,
    WATCH,
    CAR,
    TABLET,
    UNKNOWN,
};

enum class DeviceOrientation : int32_t {
    PORTRAIT,
    LANDSCAPE,
    ORIENTATION_UNDEFINED,
};

enum class ColorMode : int32_t {
    LIGHT = 0,
    DARK,
    COLOR_MODE_UNDEFINED,
};

struct DeviceConfig {
    DeviceOrientation orientation { DeviceOrientation::PORTRAIT };
    double density { 1.0 };
    DeviceType deviceType { DeviceType::PHONE };
    double fontRatio { 1.0 };
    ColorMode colorMode { ColorMode::LIGHT };
};

struct DeviceResourceInfo {
    DeviceConfig deviceConfig;
    std::vector<int64_t> resourcehandlers;
    std::string packagePath;
    int32_t themeId { -1 };
};

using SendCurrentRouterCallback = bool (*)(const std::string currentRouterPath);
using CallbackTypePostTask = std::function<void(const std::function<void()>&, int64_t)>;

struct Options {
    std::string bundleName;
    std::string moduleName;
    std::string modulePath;
    std::string resourcePath;
    int debugPort = -1;
    std::string assetPath;
    std::string systemResourcePath;
    std::string appResourcePath;
    std::string containerSdkPath;
    std::string url;
    std::string language;
    std::string region;
    std::string script;
    uint32_t themeId;
    int32_t deviceWidth;
    int32_t deviceHeight;
    bool isRound;
    SendCurrentRouterCallback onRouterChange;
    DeviceConfig deviceConfig;
    int32_t compatibleVersion;
    bool installationFree;
    int32_t labelId;
    std::string compileMode;
    std::string pageProfile;
    int32_t targetVersion;
    std::string releaseType;
    bool enablePartialUpdate;
    std::string previewPath;
    AppExecFwk::ApplicationInfo applicationInfo;
    AppExecFwk::HapModuleInfo hapModuleInfo;
    AppExecFwk::AbilityInfo abilityInfo;
    std::shared_ptr<AppExecFwk::Configuration> configuration;
    std::vector<uint8_t> moduleJsonBuffer;
    CallbackTypePostTask postTask;
    std::map<std::string, std::string> pkgContextInfoJsonStringMap;
    std::map<std::string, std::string> packageNameList;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_SIMULATOR_OPTIONS_H
