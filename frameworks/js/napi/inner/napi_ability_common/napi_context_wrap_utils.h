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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_CONTEXT_WRAP_UTILS_H
#define OHOS_ABILITY_RUNTIME_NAPI_CONTEXT_WRAP_UTILS_H

#include "ability.h"
#include "feature_ability_common.h"
#include "js_runtime_utils.h"
#include "napi_common.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

using Ability = OHOS::AppExecFwk::Ability;
namespace OHOS {
namespace AppExecFwk {

#ifdef SUPPORT_GRAPHICS
Ability* GetJSAbilityObject(napi_env env);
napi_value SetShowOnLockScreenAsync(napi_env env, napi_value *args, ShowOnLockScreenCB *showOnLockScreenCB);
napi_value SetShowOnLockScreenPromise(napi_env env, ShowOnLockScreenCB *cbData);
void SetDisplayOrientationExecuteCallbackWork(napi_env env, void *data);
bool UnwrapSetDisplayOrientation(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
napi_value SetWakeUpScreenWrap(napi_env env, napi_callback_info info, SetWakeUpScreenCB *cbData);
#endif

bool UnwrapParamVerifySelfPermission(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
void VerifySelfPermissionExecuteCallbackWork(napi_env env, void *data);
bool UnwrapRequestPermissionsFromUser(
    napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
void RequestPermissionsFromUserExecuteCallbackWork(napi_env env, void *data);
void RequestPermissionsFromUserCompleteAsyncCallbackWork(napi_env env, napi_status status, void *data);
bool UnwrapParamVerifyPermission(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo);
void VerifyPermissionExecuteCallback(napi_env env, void *data);
napi_value GetApplicationInfoWrap(napi_env env, napi_callback_info info, AppInfoCB *appInfoCB);
AppInfoCB *CreateAppInfoCBInfo(napi_env env);
void GetBundleNameExecuteCallback(napi_env env, void *data);
napi_value GetProcessInfoWrap(napi_env env, napi_callback_info info, ProcessInfoCB *processInfoCB);
ProcessInfoCB *CreateProcessInfoCBInfo(napi_env env);
ElementNameCB *CreateElementNameCBInfo(napi_env env);
napi_value GetElementNameWrap(napi_env env, napi_callback_info info, ElementNameCB *elementNameCB);
ProcessNameCB *CreateProcessNameCBInfo(napi_env env);
napi_value GetProcessNameWrap(napi_env env, napi_callback_info info, ProcessNameCB *processNameCB);
CallingBundleCB *CreateCallingBundleCBInfo(napi_env env);
napi_value GetCallingBundleWrap(napi_env env, napi_callback_info info, CallingBundleCB *callingBundleCB);
GetOrCreateLocalDirCB *CreateGetOrCreateLocalDirCBInfo(napi_env env);
napi_value GetOrCreateLocalDirWrap(napi_env env,
    napi_callback_info info, GetOrCreateLocalDirCB *getOrCreateLocalDirCB);
DatabaseDirCB *CreateGetDatabaseDirCBInfo(napi_env env);
napi_value GetDatabaseDirWrap(napi_env env, napi_callback_info info, DatabaseDirCB *getDatabaseDirCB);
PreferencesDirCB *CreateGetPreferencesDirCBInfo(napi_env env);
napi_value GetPreferencesDirWrap(napi_env env, napi_callback_info info, PreferencesDirCB *getPreferencesDirCB);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_NAPI_CONTEXT_WRAP_UTILS_H */
