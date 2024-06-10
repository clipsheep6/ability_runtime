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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_UTILS_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_UTILS_H

#include <map>
#include <stdint.h>

#include "ability_manager_client.h"
#include "js_napi_common_ability.h"
#include "js_runtime_utils.h"
#include "napi_common_error.h"

namespace OHOS {
namespace AppExecFwk {
const std::int32_t STR_MAX_SIZE = 128;
bool CheckAbilityType(const CBBase *cbBase);
bool CheckAbilityType(const AsyncJSCallbackInfo *asyncCallbackInfo);
bool CheckAbilityType(const AsyncCallbackInfo *asyncCallbackInfo);

napi_value GetContinueAbilityOptionsInfoCommon(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

napi_value GetContinueAbilityOptionsReversible(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info);

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
void GetOrCreateDistributedDirExecuteCallback(napi_env, void *data);
/**
 * @brief GetCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCacheDirExecuteCallback(napi_env, void *data);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppTypeCB on success, nullptr on failure.
 */
AppTypeCB *CreateAppTypeCBInfo(napi_env env);

/**
 * @brief GetAppType asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeExecuteCB(napi_env, void *data);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeAsyncCompleteCB(napi_env env, napi_status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypePromiseCompleteCB(napi_env env, napi_status, void *data);

/**
 * @brief GetAppType Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeAsync(napi_env env, napi_value *args, const size_t argCallback, AppTypeCB *appTypeCB);

/**
 * @brief GetAppType Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypePromise(napi_env env, AppTypeCB *appTypeCB);

/**
 * @brief GetAppType processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeWrap(napi_env env, napi_callback_info info, AppTypeCB *appTypeCB);

bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo);

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
napi_value ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo);

/**
 * @brief GetAbilityInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoExecuteCB(napi_env, void *data);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoAsyncCompleteCB(napi_env env, napi_status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoPromiseCompleteCB(napi_env env, napi_status, void *data);

/**
 * @brief GetAbilityInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoAsync(napi_env env, napi_value *args, const size_t argCallback, AbilityInfoCB *abilityInfoCB);

/**
 * @brief GetAbilityInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoPromise(napi_env env, AbilityInfoCB *abilityInfoCB);

/**
 * @brief GetAbilityInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoWrap(napi_env env, napi_callback_info info, AbilityInfoCB *abilityInfoCB);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to HapModuleInfoCB on success, nullptr on failure.
 */
HapModuleInfoCB *CreateHapModuleInfoCBInfo(napi_env env);

napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &cb);
void GetHapModuleInfoExecuteCB(napi_env, void *data);
void GetHapModuleInfoAsyncCompleteCB(napi_env env, napi_status, void *data);
void GetHapModuleInfoPromiseCompleteCB(napi_env env, napi_status, void *data);

/**
 * @brief GetHapModuleInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoAsync(
    napi_env env, napi_value *args, const size_t argCallback, HapModuleInfoCB *hapModuleInfoCB);

/**
 * @brief GetHapModuleInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoPromise(napi_env env, HapModuleInfoCB *hapModuleInfoCB);

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppVersionInfoCB on success, nullptr on failure.
 */
AppVersionInfoCB *CreateAppVersionInfoCBInfo(napi_env env);
napi_value WrapAppVersionInfo(napi_env env, const AppVersionInfoCB &appVersionInfoCB);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncCallbackInfo on success, nullptr on failure
 */
AsyncCallbackInfo *CreateAsyncCallbackInfo(napi_env env);

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
AbilityNameCB *CreateAbilityNameCBInfo(napi_env env);

napi_value WrapAbilityName(napi_env env, const AbilityNameCB *abilityNameCB);
void UnwrapAbilityStartSettingForNumber(
    napi_env env, const std::string key, napi_value param, AAFwk::AbilityStartSetting &setting);
bool UnwrapAbilityStartSetting(napi_env env, napi_value param, AAFwk::AbilityStartSetting &setting);

bool UnwrapParamStopAbilityWrap(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
napi_value UnwrapParamForWantAgent(napi_env &env, napi_value &args, AbilityRuntime::WantAgent::WantAgent *&wantAgent);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ABILITY_UTILS_H