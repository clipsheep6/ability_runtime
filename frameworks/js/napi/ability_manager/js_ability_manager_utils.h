/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_MANAGER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_MANAGER_UTILS_H

#include "ability_running_info.h"
#include "ability_state_data.h"
#include "element_name.h"
#include "extension_running_info.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AbilityStateData;
napi_value CreateJsAbilityRunningInfoArray(
    napi_env env, const std::vector<AAFwk::AbilityRunningInfo> &infos);
napi_value CreateJsExtensionRunningInfoArray(
    napi_env env, const std::vector<AAFwk::ExtensionRunningInfo> &infos);
napi_value CreateJsAbilityRunningInfo(napi_env env, const AAFwk::AbilityRunningInfo &info);
napi_value CreateJsExtensionRunningInfo(napi_env env, const AAFwk::ExtensionRunningInfo &info);
napi_value AbilityStateInit(napi_env env);
napi_value CreateJsElementName(napi_env env, const AppExecFwk::ElementName &elementName);
napi_value CreateJsAbilityStateData(napi_env env, const AbilityStateData &abilityStateData);
napi_value CreateJsAbilityStateDataArray(
    napi_env env, const std::vector<AppExecFwk::AbilityStateData> &abilityStateDatas);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_MANAGER_UTILS_H
