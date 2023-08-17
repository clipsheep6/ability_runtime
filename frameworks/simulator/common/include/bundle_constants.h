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

#ifndef FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_INCLUDE_BUNDLE_CONSTANTS_H
#define FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_INCLUDE_BUNDLE_CONSTANTS_H

#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
namespace Constants {
const std::string EMPTY_STRING = "";
const std::string PATH_SEPARATOR = "/";
constexpr const char* CURRENT_DEVICE_ID = "PHONE-001";
constexpr int DEFAULT_USERID = 0;
constexpr int UNSPECIFIED_USERID = -2;
constexpr int PERMISSION_GRANTED = 0;
constexpr const char* MODULE_NAME_SEPARATOR = ",";
enum class AppType {
    SYSTEM_APP = 0,
    THIRD_SYSTEM_APP,
    THIRD_PARTY_APP,
};
constexpr uint8_t MAX_BUNDLE_NAME = 127;
constexpr uint8_t MIN_BUNDLE_NAME = 7;
constexpr uint8_t MAX_MODULE_NAME = 31;
constexpr uint8_t MAX_JSON_ELEMENT_LENGTH = 255;
constexpr uint16_t MAX_JSON_ARRAY_LENGTH = 5120;
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* MODULE_NAME = "moduleName";
constexpr const char* HAP_PATH = "hapPath";
constexpr int32_t INITIAL_APP_INDEX = 0;
constexpr const char* RELATIVE_PATH = "../";
constexpr const char* APP_DETAIL_ABILITY = "AppDetailAbility";
} // namespace Constants
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_SIMULATOR_COMMON_INCLUDE_BUNDLE_CONSTANTS_H