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

#ifndef OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H
#define OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H

#include "ability_info.h"
#include "application_info.h"
#include "launch_param.h"
#include "hap_module_info.h"
#include "want.h"
#include "configuration.h"

class NativeEngine;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsWantObject(NativeEngine& engine, const AAFwk::Want& want);
NativeValue* CreateJsAbilityInfo(NativeEngine& engine, const AppExecFwk::AbilityInfo& abilityInfo);
NativeValue* CreateJsApplicationInfo(NativeEngine& engine, const AppExecFwk::ApplicationInfo &applicationInfo);
NativeValue* CreateJsLaunchParam(NativeEngine& engine, const AAFwk::LaunchParam& launchParam);
NativeValue* CreateJsConfiguration(NativeEngine& engine, const AppExecFwk::Configuration& configuration);
NativeValue* CreateJsExtensionAbilityInfo(NativeEngine& engine, const AppExecFwk::ExtensionAbilityInfo& info);
NativeValue* CreateJsHapModuleInfo(NativeEngine& engine, const AppExecFwk::HapModuleInfo& hapModuleInfo);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H
