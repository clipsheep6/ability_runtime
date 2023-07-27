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

#ifndef OHOS_ABILITY_RUNTIME_SIMULATOR_COMMON_FUNC_H
#define OHOS_ABILITY_RUNTIME_SIMULATOR_COMMON_FUNC_H

#include <vector>
#include <mutex>

#include "ability_info.h"
#include "application_info.h"
#include "extension_ability_info.h"
#include "hap_module_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AppExecFwk {
class CommonFunc {
public:
    static std::string GetStringFromNAPI(napi_env env, napi_value value);
    static void ConvertMetadata(napi_env env, const Metadata &metadata, napi_value value);
    static void ConvertDependency(napi_env env, const Dependency &dependency, napi_value value);
    static void ConvertPreloadItem(napi_env env, const PreloadItem &preloadItem, napi_value value);
    static void ConvertExtensionInfos(
        napi_env env, const std::vector<ExtensionAbilityInfo> &extensionInfos, napi_value value);
    static void ConvertStringArrays(napi_env env, const std::vector<std::string> &strs, napi_value value);
    static void ConvertExtensionInfo(
        napi_env env, const ExtensionAbilityInfo &extensionInfo, napi_value objExtensionInfo);
    static void ConvertAbilityInfos(napi_env env, const std::vector<AbilityInfo> &abilityInfos, napi_value value);
    static bool ParsePropertyArray(
        napi_env env, napi_value args, const std::string &propertyName, std::vector<napi_value> &valueVec);
    static bool ParseString(napi_env env, napi_value value, std::string &result);
    static napi_value ParseStringArray(napi_env env, std::vector<std::string> &stringArray, napi_value args);
    static bool ParseAbilityInfo(napi_env env, napi_value param, AbilityInfo &abilityInfo);
    static void ConvertWindowSize(napi_env env, const AbilityInfo &abilityInfo, napi_value value);
    static void ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo, napi_value objAbilityInfo);
    static void ConvertResource(napi_env env, const Resource &resource, napi_value objResource);
    static void ConvertApplicationInfo(napi_env env, napi_value objAppInfo, const ApplicationInfo &appInfo);
    static void ConvertHapModuleInfo(napi_env env, const HapModuleInfo &hapModuleInfo, napi_value objHapModuleInfo);
};
} // AppExecFwk
} // OHOS
#endif