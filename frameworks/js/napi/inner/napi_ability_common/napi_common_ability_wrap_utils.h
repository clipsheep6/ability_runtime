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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_COMMON_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_COMMON_H

#include <map>

#include "ability_manager_client.h"
#include "js_napi_common_ability.h"
#include "js_runtime_utils.h"
#include "napi_common_error.h"

namespace OHOS {
namespace AppExecFwk {

bool CheckAbilityType(const CBBase *cbBase);
bool CheckAbilityType(const AsyncJSCallbackInfo *asyncCallbackInfo);
bool CheckAbilityType(const AsyncCallbackInfo *asyncCallbackInfo);

/**
 * @brief Obtains the continue ability Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param value The value passed into the info.
 * @param info The continue ability options info
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value GetContinueAbilityOptionsInfoCommon(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

/**
 * @brief Obtains the continue ability can reversible or not
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param value The value passed into the info.
 * @param info The continue ability options info
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value GetContinueAbilityOptionsReversible(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

/**
 * @brief Obtains the continue ability Info this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param value The value passed into the info.
 * @param info The continue ability options info
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value GetContinueAbilityOptionsDeviceID(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

napi_value WrapAppInfo(napi_env env, const ApplicationInfo &appInfo);
int32_t GetStartAbilityErrorCode(ErrCode innerErrorCode);

/**
 * @brief GetFilesDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetFilesDirExecuteCallback(napi_env, void *data);
void IsUpdatingConfigurationsExecuteCallback(napi_env, void *data);

/**
 * @brief PrintDrawnCompleted asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void PrintDrawnCompletedExecuteCallback(napi_env, void *data);
void GetOrCreateDistributedDirExecuteCallback(napi_env, void *data);

/**
 * @brief GetCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCacheDirExecuteCallback(napi_env, void *data);

/**
 * @brief GetExternalCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetExternalCacheDirExecuteCallback(napi_env, void *data);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppTypeCB on success, nullptr on failure.
 */
AppTypeCB *CreateAppTypeCBInfo(napi_env env);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityInfoCB on success, nullptr on failure.
 */
AbilityInfoCB *CreateAbilityInfoCBInfo(napi_env env);

napi_value WrapAbilityInfo(napi_env env, const AbilityInfo &abilityInfo);
napi_value WrapProperties(napi_env env, const std::vector<std::string> properties, const std::string &proName,
    napi_value &result);
napi_value WrapModuleInfos(napi_env env, const ApplicationInfo &appInfo, napi_value &result);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to HapModuleInfoCB on success, nullptr on failure.
 */
HapModuleInfoCB *CreateHapModuleInfoCBInfo(napi_env env);
napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &cb);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppVersionInfoCB on success, nullptr on failure.
 */
AppVersionInfoCB *CreateAppVersionInfoCBInfo(napi_env env);
void SaveAppVersionInfo(AppVersionInfo &appVersionInfo, const std::string appName, const std::string versionName,
    const int32_t versionCode);
napi_value WrapAppVersionInfo(napi_env env, const AppVersionInfoCB &appVersionInfoCB);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
AbilityNameCB *CreateAbilityNameCBInfo(napi_env env);
napi_value WrapAbilityName(napi_env env, const AbilityNameCB *abilityNameCB);

bool UnwrapAbilityStartSetting(napi_env env, napi_value param, AAFwk::AbilityStartSetting &setting);

bool UnwrapParamStopAbilityWrap(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
napi_value UnwrapParamForWantAgent(napi_env &env, napi_value &args, AbilityRuntime::WantAgent::WantAgent *&wantAgent);
    
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_COMMON_H