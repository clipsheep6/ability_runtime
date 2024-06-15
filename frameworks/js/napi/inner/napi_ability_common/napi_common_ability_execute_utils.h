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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_IMPL_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_IMPL_H

#include <map>

#include "ability_manager_client.h"
#include "js_napi_common_ability.h"
#include "js_runtime_utils.h"
#include "napi_common_error.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief GetAppType processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeWrap(napi_env env, napi_callback_info info, AppTypeCB *appTypeCB);

/**
 * @brief GetAbilityInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoWrap(napi_env env, napi_callback_info info, AbilityInfoCB *abilityInfoCB);

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB);

napi_value GetAppVersionInfoWrap(napi_env env, napi_callback_info info, AppVersionInfoCB *appVersionInfoCB);

/**
 * @brief GetWantWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetWantWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo);

/**
 * @brief GetAbilityName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameWrap(napi_env env, napi_callback_info info, AbilityNameCB *abilityNameCB);
napi_value StopAbilityWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo);
napi_value StartBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo);
napi_value CancelBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_IMPL_H