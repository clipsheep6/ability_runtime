/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_H

#include <memory>
#include <mutex>
#include <list>

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "application_info.h"
#include "feature_ability_common.h"
#include "napi_ability_connection.h"
#include "napi_common_ability_utils.h"

namespace OHOS {
namespace AppExecFwk {
napi_status SetGlobalClassContext(napi_env env, napi_value constructor);
napi_value GetGlobalClassContext(napi_env env);

napi_status SaveGlobalDataAbilityHelper(napi_env env, napi_value constructor);
napi_value GetGlobalDataAbilityHelper(napi_env env);
bool& GetDataAbilityHelperStatus();

/**
 * @brief Get Files Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetFilesDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get OrCreateDistribute Dir.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetOrCreateDistributedDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_GetCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_GetExternalCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_IsUpdatingConfigurationsCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value NAPI_PrintDrawnCompletedCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppTypeCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get the display orientation of the main window.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
#ifdef SUPPORT_GRAPHICS
napi_value NAPI_GetDisplayOrientationCommon(napi_env env, napi_callback_info info, AbilityType abilityType);
bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo);
void GetDisplayOrientationExecuteCallback(napi_env env, void *data);
#endif

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the AppVersionInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppVersionInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContextCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

/**
 * @brief stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StopAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

bool UnwrapAbilityStartSetting(napi_env env, napi_value param, AAFwk::AbilityStartSetting &setting);

/**
 * @brief acquireDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value AcquireDataAbilityHelperWrap(
    napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB);

/**
 * @brief AcquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelperCommon(napi_env env, napi_callback_info info, AbilityType abilityType);

napi_value ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo);

/**
 * @brief start background running.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartBackgroundRunningCommon(napi_env env, napi_callback_info info);

/**
 * @brief cancel background running.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_CancelBackgroundRunningCommon(napi_env env, napi_callback_info info);

enum ErrorCode {
    NO_ERROR = 0,
    INVALID_PARAMETER = -1,
    ABILITY_NOT_FOUND = -2,
    PERMISSION_DENY = -3,
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_H
