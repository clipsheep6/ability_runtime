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

#ifndef OHOS_ABILITY_RUNTIME_SIMULAOTR_JS_DATA_CONVERTER_H
#define OHOS_ABILITY_RUNTIME_SIMULAOTR_JS_DATA_CONVERTER_H

#include "ability_info.h"
#include "application_info.h"
#include "hap_module_info.h"
#include "configuration.h"
#include "res_common.h"

class NativeEngine;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
Global::Resource::ColorMode ConvertColorMode(std::string colormode);
Global::Resource::Direction ConvertDirection(std::string direction);
Global::Resource::ScreenDensity ConvertDensity(std::string density);
int32_t ConvertDisplayId(std::string displayId);
std::string GetDensityStr(float density);
NativeValue *CreateJsConfiguration(NativeEngine &engine, const AppExecFwk::Configuration &configuration);
NativeValue *CreateJsApplicationInfo(NativeEngine &engine, const AppExecFwk::ApplicationInfo &applicationInfo);
NativeValue *CreateJsHapModuleInfo(NativeEngine &engine, const AppExecFwk::HapModuleInfo &hapModuleInfo);
NativeValue *CreateJsAbilityInfo(NativeEngine &engine, const AppExecFwk::AbilityInfo &abilityInfo);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SIMULAOTR_JS_DATA_CONVERTER_H
