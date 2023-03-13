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

#include "napi_common_ability.h"

#include <dlfcn.h>
#include <uv.h>

#include "ability_util.h"
#include "hilog_wrapper.h"
#include "js_napi_common_ability.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"
#include "napi_context.h"
#include "napi_base_context.h"
#include "napi_remote_object.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
napi_ref thread_local g_contextObject = nullptr;
napi_ref thread_local g_dataAbilityHelper = nullptr;
bool thread_local g_dataAbilityHelperStatus = false;
const int32_t ERR_ABILITY_START_SUCCESS = 0;
const int32_t ERR_ABILITY_QUERY_FAILED = 1;
const int32_t ERR_NETWORK_UNAVAILABLE = 2;
const int32_t ERR_SYSTEM_ERROR = 3;
const int32_t ERR_LOADING_ERROR = 4;
const int32_t ERR_CONCURRENT_TASKS_WAITING_FOR_RETRY = 5;
const int32_t ERR_FREE_INSTALL_NOT_SUPPORTED = 6;
const int32_t ERR_SERVICE_ERROR = 7;
const int32_t ERR_PERMISSION_VERIFY_FAILED = 8;
const int32_t ERR_PARAMETER_INVALID = 9;
const int32_t ERR_REMOTE_INCOMPATIBLE = 10;
const int32_t ERR_DEVICE_OFFLINE = 11;
const int32_t ERR_FREE_INSTALL_TIMEOUT = 12;
const int32_t ERR_NOT_TOP_ABILITY = 13;
const int32_t ERR_TARGET_BUNDLE_NOT_EXIST = 14;
const int32_t ERR_CONTINUE_FREE_INSTALL_FAILED = 15;
const int32_t ERR_PARAM_INVALID = 202;
const std::map<int32_t, int32_t> START_ABILITY_ERROR_CODE_MAP = {
    { NAPI_ERR_NO_ERROR, ERR_ABILITY_START_SUCCESS },
    { NAPI_ERR_NO_PERMISSION, ERR_PERMISSION_VERIFY_FAILED },
    { NAPI_ERR_ACE_ABILITY, ERR_ABILITY_QUERY_FAILED },
    { NAPI_ERR_PARAM_INVALID, ERR_PARAM_INVALID },
    { NAPI_ERR_ABILITY_TYPE_INVALID, ERR_ABILITY_QUERY_FAILED },
    { NAPI_ERR_ABILITY_CALL_INVALID, ERR_ABILITY_QUERY_FAILED },
    { ERR_OK, ERR_ABILITY_START_SUCCESS },
    { RESOLVE_ABILITY_ERR, ERR_ABILITY_QUERY_FAILED },
    { CHECK_PERMISSION_FAILED, ERR_PERMISSION_VERIFY_FAILED },
    { RESOLVE_CALL_NO_PERMISSIONS, ERR_PERMISSION_VERIFY_FAILED },
    { FA_FREE_INSTALL_QUERY_ERROR, ERR_ABILITY_QUERY_FAILED },
    { HAG_QUERY_TIMEOUT, ERR_ABILITY_QUERY_FAILED },
    { FA_NETWORK_UNAVAILABLE, ERR_NETWORK_UNAVAILABLE },
    { FA_FREE_INSTALL_SERVICE_ERROR, ERR_SYSTEM_ERROR },
    { FA_CRASH, ERR_SYSTEM_ERROR },
    { FA_TIMEOUT, ERR_SYSTEM_ERROR },
    { UNKNOWN_EXCEPTION, ERR_SYSTEM_ERROR },
    { NOT_SUPPORT_PA_ON_SAME_DEVICE, ERR_SYSTEM_ERROR },
    { FA_INTERNET_ERROR, ERR_SYSTEM_ERROR },
    { JUMP_TO_THE_APPLICATION_MARKET_UPGRADE, ERR_SYSTEM_ERROR },
    { USER_GIVES_UP, ERR_LOADING_ERROR },
    { INSTALLATION_ERROR_IN_FREE_INSTALL, ERR_LOADING_ERROR },
    { HAP_PACKAGE_DOWNLOAD_TIMED_OUT, ERR_LOADING_ERROR },
    { CONCURRENT_TASKS_WAITING_FOR_RETRY, ERR_CONCURRENT_TASKS_WAITING_FOR_RETRY },
    { FA_PACKAGE_DOES_NOT_SUPPORT_FREE_INSTALL, ERR_FREE_INSTALL_NOT_SUPPORTED },
    { NOT_ALLOWED_TO_PULL_THIS_FA, ERR_SERVICE_ERROR },
    { NOT_SUPPORT_CROSS_DEVICE_FREE_INSTALL_PA, ERR_SERVICE_ERROR },
    { DMS_PERMISSION_DENIED, ERR_PERMISSION_VERIFY_FAILED },
    { DMS_COMPONENT_ACCESS_PERMISSION_DENIED, ERR_PERMISSION_VERIFY_FAILED },
    { DMS_ACCOUNT_ACCESS_PERMISSION_DENIED, ERR_PERMISSION_VERIFY_FAILED },
    { INVALID_PARAMETERS_ERR, ERR_PARAMETER_INVALID },
    { INVALID_REMOTE_PARAMETERS_ERR, ERR_PARAMETER_INVALID },
    { REMOTE_DEVICE_NOT_COMPATIBLE, ERR_REMOTE_INCOMPATIBLE },
    { DEVICE_OFFLINE_ERR, ERR_DEVICE_OFFLINE },
    { FREE_INSTALL_TIMEOUT, ERR_FREE_INSTALL_TIMEOUT },
    { NOT_TOP_ABILITY, ERR_NOT_TOP_ABILITY },
    { TARGET_BUNDLE_NOT_EXIST, ERR_TARGET_BUNDLE_NOT_EXIST },
    { CONTINUE_FREE_INSTALL_FAILED, ERR_CONTINUE_FREE_INSTALL_FAILED }
};

using NAPICreateJsRemoteObject = napi_value (*)(napi_env env, const sptr<IRemoteObject> target);

napi_status SetGlobalClassContext(napi_env env, napi_value constructor)
{
    return napi_create_reference(env, constructor, 1, &g_contextObject);
}

napi_value GetGlobalClassContext(napi_env env)
{
    napi_value constructor;
    NAPI_CALL(env, napi_get_reference_value(env, g_contextObject, &constructor));
    return constructor;
}

napi_status SaveGlobalDataAbilityHelper(napi_env env, napi_value constructor)
{
    return napi_create_reference(env, constructor, 1, &g_dataAbilityHelper);
}

napi_value GetGlobalDataAbilityHelper(napi_env env)
{
    napi_value constructor;
    NAPI_CALL(env, napi_get_reference_value(env, g_dataAbilityHelper, &constructor));
    return constructor;
}

bool& GetDataAbilityHelperStatus()
{
    return g_dataAbilityHelperStatus;
}

bool CheckAbilityType(AbilityType typeInAbility, AbilityType typeWant)
{
    switch (typeWant) {
        case AbilityType::PAGE:
            if (typeInAbility == AbilityType::PAGE || typeInAbility == AbilityType::DATA) {
                return true;
            }
            return false;
        default:
            return typeInAbility != AbilityType::PAGE;
    }
    return false;
}

bool CheckAbilityType(const CBBase *cbBase)
{
    if (cbBase == nullptr) {
        HILOG_ERROR("%{public}s cbBase == nullptr", __func__);
        return false;
    }

    if (cbBase->ability == nullptr) {
        HILOG_ERROR("%{public}s cbBase->ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = cbBase->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }
    return CheckAbilityType((AbilityType)info->type, cbBase->abilityType);
}

bool CheckAbilityType(const AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return false;
    }

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return CheckAbilityType((AbilityType)info->type, asyncCallbackInfo->abilityType);
}

bool CheckAbilityType(const AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return false;
    }

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("%{public}s info == nullptr", __func__);
        return false;
    }

    HILOG_INFO("%{public}s end.", __func__);
    return CheckAbilityType((AbilityType)info->type, asyncCallbackInfo->abilityType);
}

napi_value GetContinueAbilityOptionsInfoCommon(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_value result = nullptr;

    // reversible?: boolean
    if (GetContinueAbilityOptionsReversible(env, value, info) == nullptr) {
        return nullptr;
    }

    // deviceId?: string
    if (GetContinueAbilityOptionsDeviceID(env, value, info) == nullptr) {
        return nullptr;
    }

    napi_get_null(env, &result);
    HILOG_INFO("%{public}s.", __func__);
    return result;
}

napi_value GetContinueAbilityOptionsReversible(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool reversible = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "reversible", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "reversible", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            HILOG_ERROR("%{public}s, Wrong argument type. Bool expected.", __func__);
            return nullptr;
        }
        napi_get_value_bool(env, result, &reversible);
        info.reversible = reversible;
    }
    HILOG_INFO("%{public}s.", __func__);
    return result;
}

napi_value GetContinueAbilityOptionsDeviceID(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    char str[STR_MAX_SIZE] = {0};
    size_t strLen = 0;

    NAPI_CALL(env, napi_has_named_property(env, value, "deviceId", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "deviceId", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_string) {
            HILOG_ERROR("%{public}s, Wrong argument type. String expected.", __func__);
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        info.deviceId = str;
    }
    HILOG_INFO("%{public}s.", __func__);
    return result;
}

napi_value WrapAppInfo(napi_env env, const ApplicationInfo &appInfo)
{
    HILOG_INFO("%{public}s.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_int32(env, appInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_get_boolean(env, appInfo.isSystemApp, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "systemApp", proValue));
    NAPI_CALL(env, napi_get_boolean(env, appInfo.enabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "enabled", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, std::to_string(appInfo.labelId).c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, std::to_string(appInfo.iconId).c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.process.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "process", proValue));
    NAPI_CALL(env, napi_create_int32(env, appInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));

    (void)WrapProperties(env, appInfo.moduleSourceDirs, "moduleSourceDirs", result);
    (void)WrapProperties(env, appInfo.permissions, "permissions", result);
    (void)WrapModuleInfos(env, appInfo, result);
    NAPI_CALL(env, napi_create_string_utf8(env, appInfo.entryDir.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "entryDir", proValue));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

int32_t GetStartAbilityErrorCode(ErrCode innerErrorCode)
{
    auto iter = START_ABILITY_ERROR_CODE_MAP.find(innerErrorCode);
    if (iter != START_ABILITY_ERROR_CODE_MAP.end()) {
        return iter->second;
    }
    return ERR_ABILITY_QUERY_FAILED;
}

/**
 * @brief GetFilesDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetFilesDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetFilesDir();
    HILOG_INFO("%{public}s end. filesDir=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

void IsUpdatingConfigurationsExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->IsUpdatingConfigurations();
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief PrintDrawnCompleted asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void PrintDrawnCompletedExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->PrintDrawnCompleted();
    HILOG_INFO("%{public}s end", __func__);
}

napi_value NAPI_GetFilesDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}
napi_value NAPI_GetFilesDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetFilesDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

void GetOrCreateDistributedDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetDistributedFilesDir();
    HILOG_INFO("%{public}s end. filesDir=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief GetFilesDir processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetOrCreateDistributedDirWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_ERROR("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_ERROR("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetOrCreateDistributedDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetOrCreateDistributedDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief GetCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCacheDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetCacheDir();
    HILOG_INFO("%{public}s end. CacheDir=%{public}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief NAPI_GetCacheDirWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetCacheDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_ERROR("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_ERROR("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetCacheDirCallback";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetCacheDirPromise";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief GetExternalCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetExternalCacheDirExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetExternalCacheDir();
    HILOG_INFO(
        "%{public}s end. ExternalCacheDir=%{private}s", __func__, asyncCallbackInfo->native_data.str_value.c_str());
}

/**
 * @brief NAPI_GetExternalCacheDirWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NAPI_GetExternalCacheDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetExternalCacheDirCallback";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetExternalCacheDirPromise";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetExternalCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetExternalCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

napi_value NAPI_IsUpdatingConfigurationsWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsCallback";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsPromise";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_PrintDrawnCompletedWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        HILOG_INFO("%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedCallback";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompleteAsyncVoidCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedPromise";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompletePromiseVoidCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_IsUpdatingConfigurationsCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_IsUpdatingConfigurationsWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

napi_value NAPI_PrintDrawnCompletedCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_INFO("%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_PrintDrawnCompletedWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppTypeCB on success, nullptr on failure.
 */
AppTypeCB *CreateAppTypeCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AppTypeCB *appTypeCB = new (std::nothrow) AppTypeCB;
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, appTypeCB == nullptr.", __func__);
        return nullptr;
    }
    appTypeCB->cbBase.cbInfo.env = env;
    appTypeCB->cbBase.asyncWork = nullptr;
    appTypeCB->cbBase.deferred = nullptr;
    appTypeCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return appTypeCB;
}

/**
 * @brief GetAppType asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo,appTypeCB == nullptr");
        return;
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appTypeCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo,ability == nullptr");
        appTypeCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appTypeCB->cbBase)) {
        HILOG_ERROR("NAPI_GetApplicationInfo,wrong ability type");
        appTypeCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    appTypeCB->name = appTypeCB->cbBase.ability->GetAppType();
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));

    result[PARAM0] = GetCallbackErrorValue(env, appTypeCB->cbBase.errCode);
    if (appTypeCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        NAPI_CALL_RETURN_VOID(env,
            napi_create_string_utf8(
                env, appTypeCB->cbBase.ability->GetAppType().c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appTypeCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appTypeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appTypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appTypeCB->cbBase.asyncWork));
    delete appTypeCB;
    appTypeCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("GetAppTypePromiseCompleteCB, main event thread complete.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    napi_value result = nullptr;
    if (appTypeCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        napi_create_string_utf8(env, appTypeCB->cbBase.ability->GetAppType().c_str(), NAPI_AUTO_LENGTH, &result);
        napi_resolve_deferred(env, appTypeCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appTypeCB->cbBase.errCode);
        napi_reject_deferred(env, appTypeCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appTypeCB->cbBase.asyncWork);
    delete appTypeCB;
    appTypeCB = nullptr;
    HILOG_INFO("GetAppTypePromiseCompleteCB, main event thread complete end.");
}

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
napi_value GetAppTypeAsync(napi_env env, napi_value *args, const size_t argCallback, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &appTypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppTypeExecuteCB,
            GetAppTypeAsyncCompleteCB,
            static_cast<void *>(appTypeCB),
            &appTypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appTypeCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAppType Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypePromise(napi_env env, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appTypeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAppTypeExecuteCB,
            GetAppTypePromiseCompleteCB,
            static_cast<void *>(appTypeCB),
            &appTypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appTypeCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetAppType processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param appTypeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppTypeWrap(napi_env env, napi_callback_info info, AppTypeCB *appTypeCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appTypeCB == nullptr) {
        HILOG_ERROR("%{public}s, appTypeCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppTypeAsync(env, args, 0, appTypeCB);
    } else {
        ret = GetAppTypePromise(env, appTypeCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the type of this application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppTypeCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AppTypeCB *appTypeCB = CreateAppTypeCBInfo(env);
    if (appTypeCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appTypeCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppTypeWrap(env, info, appTypeCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        if (appTypeCB != nullptr) {
            delete appTypeCB;
            appTypeCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s, end.", __func__);
    return ret;
}

#ifdef SUPPORT_GRAPHICS
napi_value GetDisplayOrientationWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamGetDisplayOrientationWrap(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamGetDisplayOrientationWrap fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapCallback";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapPromise";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

void GetDisplayOrientationExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability is null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->GetDisplayOrientation();
    HILOG_INFO("%{public}s end.", __func__);
}

bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);
    const size_t argcMax = 1;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_ERROR("%{public}s, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM0], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s, the first parameter is invalid.", __func__);
            return false;
        }
    }

    return true;
}

napi_value NAPI_GetDisplayOrientationCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetDisplayOrientationWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr.", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_DEBUG("%{public}s, end.", __func__);
    return ret;
}
#endif

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityInfoCB on success, nullptr on failure.
 */
AbilityInfoCB *CreateAbilityInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AbilityInfoCB *abilityInfoCB = new (std::nothrow) AbilityInfoCB;
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityInfoCB == nullptr.", __func__);
        return nullptr;
    }
    abilityInfoCB->cbBase.cbInfo.env = env;
    abilityInfoCB->cbBase.asyncWork = nullptr;
    abilityInfoCB->cbBase.deferred = nullptr;
    abilityInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return abilityInfoCB;
}

napi_value WrapAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "bundleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.process.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "process", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.type), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "type", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.orientation), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "orientation", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.launchMode), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "launchMode", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.uri.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "uri", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.readPermission.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "readPermission", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.writePermission.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "writePermission", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, abilityInfo.targetAbility.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "targetAbility", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.formEntity, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEntity", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.minFormHeight, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "minFormHeight", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.defaultFormHeight, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "defaultFormHeight", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.minFormWidth, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "minFormWidth", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.defaultFormWidth, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "defaultFormWidth", proValue));

    NAPI_CALL(env, napi_create_int32(env, abilityInfo.backgroundModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.subType), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "subType", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.visible, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "isVisible", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.formEnabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEnabled", proValue));

    (void)WrapProperties(env, abilityInfo.permissions, "permissions", result);
    (void)WrapProperties(env, abilityInfo.deviceCapabilities, "deviceCapabilities", result);
    (void)WrapProperties(env, abilityInfo.deviceTypes, "deviceTypes", result);

    napi_value applicationInfo = nullptr;
    applicationInfo = WrapAppInfo(env, abilityInfo.applicationInfo);
    NAPI_CALL(env, napi_set_named_property(env, result, "applicationInfo", applicationInfo));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

napi_value WrapProperties(napi_env env, const std::vector<std::string> properties, const std::string &proName,
    napi_value &result)
{
    napi_value jsArrayProperties = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayProperties));
    napi_value proValue = nullptr;
    for (size_t i = 0; i < properties.size(); i++) {
        NAPI_CALL(
            env, napi_create_string_utf8(env, properties.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayProperties, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, proName.c_str(), jsArrayProperties));
    return result;
}

napi_value WrapModuleInfos(napi_env env, const ApplicationInfo &appInfo, napi_value &result)
{
    napi_value jsArrayModuleInfo = nullptr;
    napi_value jsModuleInfoObject = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayModuleInfo));
    for (size_t i = 0; i < appInfo.moduleInfos.size(); i++) {
        NAPI_CALL(env, napi_create_object(env, &jsModuleInfoObject));
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, appInfo.moduleInfos.at(i).moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_named_property(env, jsModuleInfoObject, "moduleName", proValue));
        
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, appInfo.moduleInfos.at(i).moduleSourceDir.c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_named_property(env, jsModuleInfoObject, "moduleSourceDir", proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayModuleInfo, i, jsModuleInfoObject));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleInfos", jsArrayModuleInfo));
    return nullptr;
}

napi_value ConvertAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    return WrapAbilityInfo(env, abilityInfo);
}

/**
 * @brief GetAbilityInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, abilityInfoCB == nullptr");
        return;
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetApplicationInfo, ability == nullptr");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetApplicationInfo,wrong ability type");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfoPtr = abilityInfoCB->cbBase.ability->GetAbilityInfo();
    if (abilityInfoPtr != nullptr) {
        abilityInfoCB->abilityInfo = *abilityInfoPtr;
    } else {
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, abilityInfoCB->cbBase.errCode);
    if (abilityInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAbilityInfo(env, abilityInfoCB->abilityInfo);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, abilityInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (abilityInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, abilityInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityInfoCB->cbBase.asyncWork));
    delete abilityInfoCB;
    abilityInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    napi_value result = nullptr;
    if (abilityInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAbilityInfo(env, abilityInfoCB->abilityInfo);
        napi_resolve_deferred(env, abilityInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, abilityInfoCB->cbBase.errCode);
        napi_reject_deferred(env, abilityInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, abilityInfoCB->cbBase.asyncWork);
    delete abilityInfoCB;
    abilityInfoCB = nullptr;
    HILOG_INFO("NAPI_GetApplicationInfo, main event thread complete end.");
}

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
napi_value GetAbilityInfoAsync(napi_env env, napi_value *args, const size_t argCallback, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &abilityInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityInfoExecuteCB,
            GetAbilityInfoAsyncCompleteCB,
            static_cast<void *>(abilityInfoCB),
            &abilityInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAbilityInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoPromise(napi_env env, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    abilityInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityInfoExecuteCB,
            GetAbilityInfoPromiseCompleteCB,
            static_cast<void *>(abilityInfoCB),
            &abilityInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetAbilityInfo processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityInfoWrap(napi_env env, napi_callback_info info, AbilityInfoCB *abilityInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (abilityInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityInfoAsync(env, args, 0, abilityInfoCB);
    } else {
        ret = GetAbilityInfoPromise(env, abilityInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains information about the current ability.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AbilityInfoCB *abilityInfoCB = CreateAbilityInfoCBInfo(env);
    if (abilityInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    abilityInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityInfoWrap(env, info, abilityInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (abilityInfoCB != nullptr) {
            delete abilityInfoCB;
            abilityInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to HapModuleInfoCB on success, nullptr on failure.
 */
HapModuleInfoCB *CreateHapModuleInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    HapModuleInfoCB *hapModuleInfoCB = new (std::nothrow) HapModuleInfoCB;
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, hapModuleInfoCB == nullptr.", __func__);
        return nullptr;
    }
    hapModuleInfoCB->cbBase.cbInfo.env = env;
    hapModuleInfoCB->cbBase.asyncWork = nullptr;
    hapModuleInfoCB->cbBase.deferred = nullptr;
    hapModuleInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return hapModuleInfoCB;
}

napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &hapModuleInfoCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(
        env, napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.backgroundImg.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundImg", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, hapModuleInfoCB.hapModuleInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, hapModuleInfoCB.hapModuleInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(
            env, hapModuleInfoCB.hapModuleInfo.mainAbility.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "mainAbilityName", proValue));

    NAPI_CALL(env, napi_get_boolean(env, hapModuleInfoCB.hapModuleInfo.installationFree, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "installationFree", proValue));

    napi_value jsArrayreqCapabilities = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayreqCapabilities));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.reqCapabilities.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, hapModuleInfoCB.hapModuleInfo.reqCapabilities.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayreqCapabilities, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "reqCapabilities", jsArrayreqCapabilities));

    napi_value jsArraydeviceTypes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArraydeviceTypes));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.deviceTypes.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(
                env, hapModuleInfoCB.hapModuleInfo.deviceTypes.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArraydeviceTypes, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceTypes", jsArraydeviceTypes));

    napi_value abilityInfos = nullptr;
    NAPI_CALL(env, napi_create_array(env, &abilityInfos));
    for (size_t i = 0; i < hapModuleInfoCB.hapModuleInfo.abilityInfos.size(); i++) {
        napi_value abilityInfo = nullptr;
        abilityInfo = WrapAbilityInfo(env, hapModuleInfoCB.hapModuleInfo.abilityInfos.at(i));
        NAPI_CALL(env, napi_set_element(env, abilityInfos, i, abilityInfo));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityInfo", abilityInfos));
    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

void GetHapModuleInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, worker pool thread execute.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetHapModuleInfo, hapModuleInfoCB == nullptr");
        return;
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (hapModuleInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetHapModuleInfo, ability == nullptr");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&hapModuleInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetHapModuleInfo,wrong ability type");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<HapModuleInfo> hapModuleInfoPtr = hapModuleInfoCB->cbBase.ability->GetHapModuleInfo();
    if (hapModuleInfoPtr != nullptr) {
        hapModuleInfoCB->hapModuleInfo = *hapModuleInfoPtr;
    } else {
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetHapModuleInfo, worker pool thread execute end.");
}

void GetHapModuleInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, hapModuleInfoCB->cbBase.errCode);
    if (hapModuleInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapHapModuleInfo(env, *hapModuleInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, hapModuleInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (hapModuleInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, hapModuleInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    delete hapModuleInfoCB;
    hapModuleInfoCB = nullptr;
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete end.");
}

void GetHapModuleInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    napi_value result = nullptr;
    if (hapModuleInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapHapModuleInfo(env, *hapModuleInfoCB);
        napi_resolve_deferred(env, hapModuleInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, hapModuleInfoCB->cbBase.errCode);
        napi_reject_deferred(env, hapModuleInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, hapModuleInfoCB->cbBase.asyncWork);
    delete hapModuleInfoCB;
    hapModuleInfoCB = nullptr;
    HILOG_INFO("NAPI_GetHapModuleInfo, main event thread complete end.");
}

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
    napi_env env, napi_value *args, const size_t argCallback, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &hapModuleInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetHapModuleInfoExecuteCB,
            GetHapModuleInfoAsyncCompleteCB,
            static_cast<void *>(hapModuleInfoCB),
            &hapModuleInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetHapModuleInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param hapModuleInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetHapModuleInfoPromise(napi_env env, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    hapModuleInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetHapModuleInfoExecuteCB,
            GetHapModuleInfoPromiseCompleteCB,
            static_cast<void *>(hapModuleInfoCB),
            &hapModuleInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, hapModuleInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (hapModuleInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, hapModuleInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetHapModuleInfoAsync(env, args, 0, hapModuleInfoCB);
    } else {
        ret = GetHapModuleInfoPromise(env, hapModuleInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the HapModuleInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetHapModuleInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    HapModuleInfoCB *hapModuleInfoCB = CreateHapModuleInfoCBInfo(env);
    if (hapModuleInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    hapModuleInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetHapModuleInfoWrap(env, info, hapModuleInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (hapModuleInfoCB != nullptr) {
            delete hapModuleInfoCB;
            hapModuleInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AppVersionInfoCB on success, nullptr on failure.
 */
AppVersionInfoCB *CreateAppVersionInfoCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AppVersionInfoCB *appVersionInfoCB = new (std::nothrow) AppVersionInfoCB;
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appVersionInfoCB == nullptr.", __func__);
        return nullptr;
    }
    appVersionInfoCB->cbBase.cbInfo.env = env;
    appVersionInfoCB->cbBase.asyncWork = nullptr;
    appVersionInfoCB->cbBase.deferred = nullptr;
    appVersionInfoCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return appVersionInfoCB;
}

void SaveAppVersionInfo(AppVersionInfo &appVersionInfo, const std::string appName, const std::string versionName,
    const int32_t versionCode)
{
    HILOG_INFO("%{public}s called.", __func__);
    appVersionInfo.appName = appName;
    appVersionInfo.versionName = versionName;
    appVersionInfo.versionCode = versionCode;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value WrapAppVersionInfo(napi_env env, const AppVersionInfoCB &appVersionInfoCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env,
        napi_create_string_utf8(env, appVersionInfoCB.appVersionInfo.appName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "appName", proValue));

    NAPI_CALL(env,
        napi_create_string_utf8(env, appVersionInfoCB.appVersionInfo.versionName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "versionName", proValue));

    NAPI_CALL(env, napi_create_int32(env, appVersionInfoCB.appVersionInfo.versionCode, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "versionCode", proValue));

    HILOG_INFO("%{public}s end.", __func__);
    return result;
}

void GetAppVersionInfoExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("NAPI_GetAppVersionInfo, worker pool thread execute.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("NAPI_GetAppVersionInfo, appVersionInfoCB == nullptr");
        return;
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appVersionInfoCB->cbBase.ability == nullptr) {
        HILOG_ERROR("NAPI_GetAppVersionInfo, ability == nullptr");
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appVersionInfoCB->cbBase)) {
        HILOG_ERROR("NAPI_GetAppVersionInfo,wrong ability type");
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<ApplicationInfo> appInfoPtr = appVersionInfoCB->cbBase.ability->GetApplicationInfo();
    if (appInfoPtr != nullptr) {
        SaveAppVersionInfo(appVersionInfoCB->appVersionInfo, appInfoPtr->name, appInfoPtr->versionName,
            appInfoPtr->versionCode);
    } else {
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("NAPI_GetAppVersionInfo, worker pool thread execute end.");
}

void GetAppVersionInfoAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, appVersionInfoCB->cbBase.errCode);
    if (appVersionInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAppVersionInfo(env, *appVersionInfoCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, appVersionInfoCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (appVersionInfoCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, appVersionInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, appVersionInfoCB->cbBase.asyncWork));
    delete appVersionInfoCB;
    appVersionInfoCB = nullptr;
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete end.");
}

void GetAppVersionInfoPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    napi_value result = nullptr;
    if (appVersionInfoCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAppVersionInfo(env, *appVersionInfoCB);
        napi_resolve_deferred(env, appVersionInfoCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, appVersionInfoCB->cbBase.errCode);
        napi_reject_deferred(env, appVersionInfoCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, appVersionInfoCB->cbBase.asyncWork);
    delete appVersionInfoCB;
    appVersionInfoCB = nullptr;
    HILOG_INFO("NAPI_GetAppVersionInfo, main event thread complete end.");
}

/**
 * @brief GetAppVersionInfo Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param AppVersionInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppVersionInfoAsync(
    napi_env env, napi_value *args, const size_t argCallback, AppVersionInfoCB *appVersionInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &appVersionInfoCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(
        env, napi_create_async_work(env, nullptr, resourceName, GetAppVersionInfoExecuteCB,
                 GetAppVersionInfoAsyncCompleteCB, static_cast<void *>(appVersionInfoCB),
                 &appVersionInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appVersionInfoCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAppVersionInfo Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param AppVersionInfoCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAppVersionInfoPromise(napi_env env, AppVersionInfoCB *appVersionInfoCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    appVersionInfoCB->cbBase.deferred = deferred;

    NAPI_CALL(
        env, napi_create_async_work(env, nullptr, resourceName, GetAppVersionInfoExecuteCB,
                 GetAppVersionInfoPromiseCompleteCB, static_cast<void *>(appVersionInfoCB),
                 &appVersionInfoCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, appVersionInfoCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetAppVersionInfoWrap(napi_env env, napi_callback_info info, AppVersionInfoCB *appVersionInfoCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (appVersionInfoCB == nullptr) {
        HILOG_ERROR("%{public}s, appVersionInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppVersionInfoAsync(env, args, 0, appVersionInfoCB);
    } else {
        ret = GetAppVersionInfoPromise(env, appVersionInfoCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the AppVersionInfo object of the application.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAppVersionInfoCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AppVersionInfoCB *appVersionInfoCB = CreateAppVersionInfoCBInfo(env);
    if (appVersionInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appVersionInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppVersionInfoWrap(env, info, appVersionInfoCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (appVersionInfoCB != nullptr) {
            delete appVersionInfoCB;
            appVersionInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AsyncCallbackInfo on success, nullptr on failure
 */
AsyncCallbackInfo *CreateAsyncCallbackInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (env == nullptr) {
        HILOG_INFO("%{public}s env == nullptr.", __func__);
        return nullptr;
    }

    napi_status ret;
    napi_value global = nullptr;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_global=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    napi_value abilityObj = nullptr;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_named_property=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    Ability *ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability));
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        HILOG_ERROR("%{public}s get_value_external=%{public}d err:%{public}s", __func__, ret, errorInfo->error_message);
    }

    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return nullptr;
    }
    asyncCallbackInfo->cbInfo.env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->deferred = nullptr;
    asyncCallbackInfo->ability = ability;
    asyncCallbackInfo->native_result = false;
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = AbilityType::UNKNOWN;

    HILOG_INFO("%{public}s end.", __func__);
    return asyncCallbackInfo;
}

void GetContextAsyncExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("GetContextAsync, worker pool thread execute.");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("GetContextAsync, asyncCallbackInfo == nullptr");
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("GetContextAsync, ability == nullptr");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("GetContextAsync,wrong ability type");
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
    HILOG_INFO("GetContextAsync, worker pool thread execute end.");
}

napi_value GetContextAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value result[ARGS_TWO] = {nullptr};
            napi_value callResult = nullptr;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result[PARAM1]);
            } else {
                result[PARAM1] = WrapUndefinedToJS(env);
            }
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetContextAsync, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetContextPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetContextPromise, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value result = nullptr;
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            } else {
                result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }

            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetContextPromise, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetContext processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetContextWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s,wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return nullptr;
    }

    napi_value result = nullptr;
    napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result);
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief Get context.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetContextCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetContextWrap(env, info, asyncCallbackInfo);

    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
    }
    return ret;
}

void GetWantExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr", __func__);
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s, ability == nullptr", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s, wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();
    if (ptrWant != nullptr) {
        asyncCallbackInfo->param.want = *ptrWant;
    } else {
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

napi_value GetWantAsync(napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetWantExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantAsync, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value result[ARGS_TWO] = {nullptr};
            napi_value callResult = nullptr;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                result[PARAM1] = WrapWant(env, asyncCallbackInfo->param.want);
            } else {
                result[PARAM1] = WrapUndefinedToJS(env);
            }
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetWantAsync, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetWantPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);
    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetWantExecuteCB,
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("GetWantPromise, main event thread complete.");
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value result = nullptr;
            if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
                result = WrapWant(env, asyncCallbackInfo->param.want);
                napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            } else {
                result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
                napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
            }

            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            HILOG_INFO("GetWantPromise, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetWantWrap processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetWantWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetWantAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = GetWantPromise(env, asyncCallbackInfo);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Get want.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetWantCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetWantWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
AbilityNameCB *CreateAbilityNameCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AbilityNameCB *abilityNameCB = new (std::nothrow) AbilityNameCB;
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr.", __func__);
        return nullptr;
    }
    abilityNameCB->cbBase.cbInfo.env = env;
    abilityNameCB->cbBase.asyncWork = nullptr;
    abilityNameCB->cbBase.deferred = nullptr;
    abilityNameCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s, end.", __func__);
    return abilityNameCB;
}

napi_value WrapAbilityName(napi_env env, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, Invalid param(abilityNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, abilityNameCB->name.c_str(), NAPI_AUTO_LENGTH, &result));
    HILOG_INFO("%{public}s, end.", __func__);
    return result;
}

/**
 * @brief GetAbilityName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr", __func__);
        return;
    }
    abilityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityNameCB->cbBase.ability == nullptr) {
        HILOG_ERROR("%{public}s, ability == nullptr", __func__);
        abilityNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityNameCB->cbBase)) {
        HILOG_ERROR("%{public}s, wrong ability type", __func__);
        abilityNameCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    abilityNameCB->name = abilityNameCB->cbBase.ability->GetAbilityName();
    HILOG_INFO("%{public}s, end.", __func__);
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s, called.", __func__);
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    result[PARAM0] = GetCallbackErrorValue(env, abilityNameCB->cbBase.errCode);
    if (abilityNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapAbilityName(env, abilityNameCB);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, abilityNameCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));
    if (abilityNameCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, abilityNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, abilityNameCB->cbBase.asyncWork));
    delete abilityNameCB;
    abilityNameCB = nullptr;
    HILOG_INFO("%{public}s, end.", __func__);
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNamePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("NAPI_GetAbilityName, main event thread complete.");
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    napi_value result = nullptr;
    if (abilityNameCB->cbBase.errCode == NAPI_ERR_NO_ERROR) {
        result = WrapAbilityName(env, abilityNameCB);
        napi_resolve_deferred(env, abilityNameCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, abilityNameCB->cbBase.errCode);
        napi_reject_deferred(env, abilityNameCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, abilityNameCB->cbBase.asyncWork);
    delete abilityNameCB;
    abilityNameCB = nullptr;
    HILOG_INFO("NAPI_GetAbilityName, main event thread complete end.");
}

/**
 * @brief GetAbilityName Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameAsync(napi_env env, napi_value *args, const size_t argCallback, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &abilityNameCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityNameExecuteCB,
            GetAbilityNameAsyncCompleteCB,
            static_cast<void *>(abilityNameCB),
            &abilityNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityNameCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return result;
}

/**
 * @brief GetAbilityName Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNamePromise(napi_env env, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    abilityNameCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetAbilityNameExecuteCB,
            GetAbilityNamePromiseCompleteCB,
            static_cast<void *>(abilityNameCB),
            &abilityNameCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, abilityNameCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief GetAbilityName processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param abilityNameCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value GetAbilityNameWrap(napi_env env, napi_callback_info info, AbilityNameCB *abilityNameCB)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (abilityNameCB == nullptr) {
        HILOG_ERROR("%{public}s, abilityNameCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityNameAsync(env, args, 0, abilityNameCB);
    } else {
        ret = GetAbilityNamePromise(env, abilityNameCB);
    }
    HILOG_INFO("%{public}s, asyncCallback end.", __func__);
    return ret;
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_GetAbilityNameCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AbilityNameCB *ablityNameCB = CreateAbilityNameCBInfo(env);
    if (ablityNameCB == nullptr) {
        HILOG_ERROR("%{public}s ablityNameCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    ablityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    ablityNameCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityNameWrap(env, info, ablityNameCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (ablityNameCB != nullptr) {
            delete ablityNameCB;
            ablityNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void UnwrapAbilityStartSettingForNumber(
    napi_env env, const std::string key, napi_value param, AAFwk::AbilityStartSetting &setting)
{
    int32_t natValue32 = 0;
    double natValueDouble = 0.0;
    bool isReadValue32 = false;
    bool isReadDouble = false;
    if (napi_get_value_int32(env, param, &natValue32) == napi_ok) {
        HILOG_INFO("%{public}s called. Property value=%{private}d.", __func__, natValue32);
        isReadValue32 = true;
    }

    if (napi_get_value_double(env, param, &natValueDouble) == napi_ok) {
        HILOG_INFO("%{public}s called. Property value=%{private}lf.", __func__, natValueDouble);
        isReadDouble = true;
    }

    if (isReadValue32 && isReadDouble) {
        if (abs(natValueDouble - natValue32 * 1.0) > 0.0) {
            setting.AddProperty(key, std::to_string(natValueDouble));
        } else {
            setting.AddProperty(key, std::to_string(natValue32));
        }
    } else if (isReadValue32) {
        setting.AddProperty(key, std::to_string(natValue32));
    } else if (isReadDouble) {
        setting.AddProperty(key, std::to_string(natValueDouble));
    }
}

bool UnwrapAbilityStartSetting(napi_env env, napi_value param, AAFwk::AbilityStartSetting &setting)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);
    HILOG_INFO("%{public}s called. Property size=%{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);

        std::string strProName = UnwrapStringFromJS(env, jsProName);
        HILOG_INFO("%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                HILOG_INFO("%{public}s called. Property value=%{private}s.", __func__, natValue.c_str());
                setting.AddProperty(strProName, natValue);
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                HILOG_INFO("%{public}s called. Property value=%{public}s.", __func__, natValue ? "true" : "false");
                setting.AddProperty(strProName, std::to_string(natValue));
                break;
            }
            case napi_number:
                UnwrapAbilityStartSettingForNumber(env, strProName, jsProValue, setting);
                break;
            default:
                break;
        }
    }

    return true;
}

/**
 * @brief Parse the parameters.
 *
 * @param param Indicates the parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
bool UnwrapParamForWant(napi_env env, napi_value args, AbilityType abilityType, CallAbilityParam &param)
{
    HILOG_INFO("%{public}s called.", __func__);
    bool ret = false;
    napi_valuetype valueType = napi_undefined;
    param.setting = nullptr;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &valueType), false);
    if (valueType != napi_object) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return false;
    }

    napi_value jsWant = GetPropertyValueByPropertyName(env, args, "want", napi_object);
    if (jsWant == nullptr) {
        HILOG_ERROR("%{public}s, jsWant == nullptr", __func__);
        return false;
    }

    ret = UnwrapWant(env, jsWant, param.want);

    napi_value jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSetting", napi_object);
    if (jsSettingObj != nullptr) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        if (!UnwrapAbilityStartSetting(env, jsSettingObj, *(param.setting))) {
            HILOG_ERROR("%{public}s, unwrap abilityStartSetting falied.", __func__);
        }
        HILOG_INFO("%{public}s abilityStartSetting", __func__);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void StartAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return;
    }
    if (asyncCallbackInfo->errCode != NAPI_ERR_NO_ERROR) {
        HILOG_ERROR("%{public}s errCode:%{public}d", __func__, asyncCallbackInfo->errCode);
        return;
    }
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return;
    }
    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
#ifdef SUPPORT_GRAPHICS
    // inherit split mode
    auto windowMode = asyncCallbackInfo->ability->GetCurrentWindowMode();
    if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        asyncCallbackInfo->param.want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }
    HILOG_INFO("window mode is %{public}d", windowMode);

    // follow orientation
    asyncCallbackInfo->param.want.SetParam("ohos.aafwk.Orientation", 0);
    if (windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
        auto orientation = asyncCallbackInfo->ability->GetDisplayOrientation();
        asyncCallbackInfo->param.want.SetParam("ohos.aafwk.Orientation", orientation);
        HILOG_DEBUG("%{public}s display orientation is %{public}d", __func__, orientation);
    }
#endif
    ErrCode ret = ERR_OK;
    if (asyncCallbackInfo->param.setting == nullptr) {
        HILOG_INFO("%{public}s param.setting == nullptr call StartAbility.", __func__);
        ret = asyncCallbackInfo->ability->StartAbility(asyncCallbackInfo->param.want);
    } else {
        HILOG_INFO("%{public}s param.setting != nullptr call StartAbility.", __func__);
        ret = asyncCallbackInfo->ability->StartAbility(asyncCallbackInfo->param.want,
            *(asyncCallbackInfo->param.setting));
    }
    if (ret != ERR_OK) {
        asyncCallbackInfo->errCode = ret;
    }
    HILOG_INFO("%{public}s end. ret:%{public}d", __func__, ret);
}

void StartAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    napi_get_undefined(env, &undefined);

    int32_t errCode = GetStartAbilityErrorCode(asyncCallbackInfo->errCode);
    result[PARAM0] = GetCallbackErrorValue(env, errCode);
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        napi_create_int32(env, 0, &result[PARAM1]);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
    napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void StartAbilityPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value result = nullptr;
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        napi_create_int32(env, 0, &result);
        napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    } else {
        int32_t errCode = GetStartAbilityErrorCode(asyncCallbackInfo->errCode);
        result = GetCallbackErrorValue(env, errCode);
        napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, end.", __func__);
    delete asyncCallbackInfo;
}

napi_value StartAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s async call.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartAbilityExecuteCB,
            StartAbilityCallbackCompletedCB,
            static_cast<void *>(asyncCallbackInfo),
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    HILOG_INFO("%{public}s async end.", __func__);
    return WrapVoidToJS(env);
}

napi_value StartAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s promise call.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartAbilityExecuteCB,
            StartAbilityPromiseCompletedCB,
            static_cast<void *>(asyncCallbackInfo),
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    HILOG_INFO("%{public}s promise end.", __func__);
    return promise;
}

/**
 * @brief StartAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param asyncCallbackInfo Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value StartAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 2;
    const size_t argcPromise = 1;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_PARAM_INVALID;
    } else {
        CallAbilityParam param;
        if (UnwrapParamForWant(env, args[PARAM0], asyncCallbackInfo->abilityType, param)) {
            asyncCallbackInfo->param = param;
        } else {
            HILOG_ERROR("%{public}s, call UnwrapParamForWant failed.", __func__);
            asyncCallbackInfo->errCode = NAPI_ERR_PARAM_INVALID;
        }
    }
    if (argcAsync > argcPromise) {
        ret = StartAbilityAsync(env, args, 1, asyncCallbackInfo);
    } else {
        ret = StartAbilityPromise(env, asyncCallbackInfo);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief startAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StartAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = StartAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

bool UnwrapParamStopAbilityWrap(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called, argc=%{public}zu", __func__, argc);
    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        HILOG_ERROR("%{public}s, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            HILOG_INFO("%{public}s, the second parameter is invalid.", __func__);
            return false;
        }
    }

    return UnwrapWant(env, argv[PARAM0], asyncCallbackInfo->param.want);
}

void StopAbilityExecuteCallback(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        HILOG_ERROR("%{public}s ability is null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        HILOG_ERROR("%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->StopAbility(asyncCallbackInfo->param.want);
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value StopAbilityWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamStopAbilityWrap(env, argc, args, asyncCallbackInfo)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapParamStopAbility fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        HILOG_INFO("%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_StopAbilityWrapCallback";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        HILOG_INFO("%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_StopAbilityWrapPromise";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

/**
 * @brief stopAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_StopAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = StopAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s. ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
ConnectAbilityCB *CreateConnectAbilityCBInfo(napi_env env)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return nullptr;
    }
    connectAbilityCB->cbBase.cbInfo.env = env;
    connectAbilityCB->cbBase.asyncWork = nullptr;
    connectAbilityCB->cbBase.deferred = nullptr;
    connectAbilityCB->cbBase.ability = ability;

    return connectAbilityCB;
}

void ConnectAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr.", __func__);
        return;
    }
    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    if (connectAbilityCB->cbBase.ability == nullptr) {
        connectAbilityCB->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr.", __func__);
        return;
    }

    if (!CheckAbilityType(&connectAbilityCB->cbBase)) {
        connectAbilityCB->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        HILOG_ERROR("%{public}s ability type invalid.", __func__);
        return;
    }

    connectAbilityCB->abilityConnection->SetEnv(env);
    connectAbilityCB->abilityConnection->SetConnectCBRef(connectAbilityCB->abilityConnectionCB.callback[0]);
    connectAbilityCB->abilityConnection->SetDisconnectCBRef(connectAbilityCB->abilityConnectionCB.callback[1]);
    connectAbilityCB->result =
        connectAbilityCB->cbBase.ability->ConnectAbility(connectAbilityCB->want, connectAbilityCB->abilityConnection);
    HILOG_INFO("%{public}s end.bundlename:%{public}s abilityname:%{public}s result:%{public}d",
        __func__,
        connectAbilityCB->want.GetBundle().c_str(),
        connectAbilityCB->want.GetElement().GetAbilityName().c_str(),
        connectAbilityCB->result);
}

void ConnectAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result = nullptr;
    napi_value callResult = nullptr;
    napi_get_undefined(env, &undefined);
    HILOG_INFO("%{public}s errCode=%{public}d result=%{public}d id=%{public}" PRId64,
        __func__,
        connectAbilityCB->errCode,
        connectAbilityCB->result,
        connectAbilityCB->id);
    if (connectAbilityCB->errCode != NAPI_ERR_NO_ERROR || connectAbilityCB->result == false) {
        HILOG_INFO("%{public}s connectAbility failed.", __func__);
        // return error code in onFailed asynccallback
        int errorCode = NO_ERROR;
        switch (connectAbilityCB->errCode) {
            case NAPI_ERR_ACE_ABILITY:
                errorCode = ABILITY_NOT_FOUND;
                break;
            case NAPI_ERR_PARAM_INVALID:
                errorCode = INVALID_PARAMETER;
                break;
            default:
                break;
        }
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, errorCode, &result));
        NAPI_CALL_RETURN_VOID(
            env, napi_get_reference_value(env, connectAbilityCB->abilityConnectionCB.callback[PARAM2], &callback));
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_ONE, &result, &callResult));
    }
    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, connectAbilityCB->cbBase.cbInfo.callback));
    }
    if (connectAbilityCB->abilityConnectionCB.callback[PARAM2] != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, connectAbilityCB->abilityConnectionCB.callback[PARAM2]));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, connectAbilityCB->cbBase.asyncWork));
    delete connectAbilityCB;
    connectAbilityCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value ConnectAbilityAsync(napi_env env, const napi_value *args, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ConnectAbilityExecuteCB,
            ConnectAbilityCallbackCompletedCB,
            static_cast<void *>(connectAbilityCB),
            &connectAbilityCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, connectAbilityCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

/**
 * @brief ConnectAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param connectAbilityCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ConnectAbilityWrap(napi_env env, napi_callback_info info, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = ARGS_TWO;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync != ARGS_TWO) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        connectAbilityCB->errCode = NAPI_ERR_PARAM_INVALID;
        return nullptr;
    }

    if (!UnwrapWant(env, args[PARAM0], connectAbilityCB->want)) {
        HILOG_INFO("%{public}s called. Invoke UnwrapWant fail", __func__);
        return nullptr;
    }

    HILOG_INFO("%{public}s uri:%{public}s", __func__, connectAbilityCB->want.GetElement().GetURI().c_str());

    std::string deviceId = connectAbilityCB->want.GetElement().GetDeviceID();
    std::string bundleName = connectAbilityCB->want.GetBundle();
    std::string abilityName = connectAbilityCB->want.GetElement().GetAbilityName();

    auto item = std::find_if(connects_.begin(),
        connects_.end(), [&deviceId, &bundleName, &abilityName](const std::map<ConnecttionKey,
        sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetElement().GetDeviceID()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match deviceid & bundlename && abilityname
        connectAbilityCB->id = item->first.id;
        connectAbilityCB->abilityConnection = item->second;
        HILOG_INFO("%{public}s find connection exist", __func__);
    } else {
        sptr<NAPIAbilityConnection> conn(new (std::nothrow) NAPIAbilityConnection());
        connectAbilityCB->id = serialNumber_;
        connectAbilityCB->abilityConnection = conn;
        ConnecttionKey key;
        key.id = connectAbilityCB->id;
        key.want = connectAbilityCB->want;
        connects_.emplace(key, conn);
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s not find connection, make new one", __func__);
    }
    HILOG_INFO("%{public}s id:%{public}" PRId64, __func__, connectAbilityCB->id);

    if (argcAsync > PARAM1) {
        napi_value jsMethod = nullptr;
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, args[PARAM1], &valuetype);
        if (valuetype == napi_object) {
            NAPI_CALL(env, napi_get_named_property(env, args[PARAM1], "onConnect", &jsMethod));
            NAPI_CALL(env, napi_typeof(env, jsMethod, &valuetype));
            HILOG_INFO("%{public}s, function onConnect valuetype=%{public}d.", __func__, valuetype);
            NAPI_CALL(
                env, napi_create_reference(env, jsMethod, 1, &connectAbilityCB->abilityConnectionCB.callback[PARAM0]));

            NAPI_CALL(env, napi_get_named_property(env, args[PARAM1], "onDisconnect", &jsMethod));
            NAPI_CALL(env, napi_typeof(env, jsMethod, &valuetype));
            HILOG_INFO("%{public}s, function onDisconnect valuetype=%{public}d.", __func__, valuetype);
            NAPI_CALL(
                env, napi_create_reference(env, jsMethod, 1, &connectAbilityCB->abilityConnectionCB.callback[PARAM1]));

            NAPI_CALL(env, napi_get_named_property(env, args[PARAM1], "onFailed", &jsMethod));
            NAPI_CALL(env, napi_typeof(env, jsMethod, &valuetype));
            HILOG_INFO("%{public}s, function onFailed valuetype=%{public}d.", __func__, valuetype);
            NAPI_CALL(
                env, napi_create_reference(env, jsMethod, 1, &connectAbilityCB->abilityConnectionCB.callback[PARAM2]));
        } else {
            HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
            return nullptr;
        }
    }

    ret = ConnectAbilityAsync(env, args, connectAbilityCB);
    if (ret != nullptr) {
        // return number to js
        NAPI_CALL(env, napi_create_int64(env, connectAbilityCB->id, &ret));
        HILOG_INFO("%{public}s id=%{public}" PRId64, __func__, connectAbilityCB->id);
    }
    HILOG_INFO("%{public}s called end.", __func__);
    return ret;
}

/**
 * @brief ConnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_ConnectAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = CreateConnectAbilityCBInfo(env);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    connectAbilityCB->cbBase.abilityType = abilityType;
    napi_value ret = ConnectAbilityWrap(env, info, connectAbilityCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief Create asynchronous data.
 *
 * @param env The environment that the Node-API call is invoked under.
 *
 * @return Return a pointer to AbilityNameCB on success, nullptr on failure.
 */
ConnectAbilityCB *CreateDisConnectAbilityCBInfo(napi_env env)
{
    HILOG_INFO("%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return nullptr;
    }
    connectAbilityCB->cbBase.cbInfo.env = env;
    connectAbilityCB->cbBase.asyncWork = nullptr;
    connectAbilityCB->cbBase.deferred = nullptr;
    connectAbilityCB->cbBase.ability = ability;

    HILOG_INFO("%{public}s end.", __func__);
    return connectAbilityCB;
}

void DisConnectAbilityExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr.", __func__);
        return;
    }
    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    if (connectAbilityCB->cbBase.ability == nullptr) {
        connectAbilityCB->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr.", __func__);
        return;
    }

    if (!CheckAbilityType(&connectAbilityCB->cbBase)) {
        connectAbilityCB->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        HILOG_ERROR("%{public}s ability type invalid.", __func__);
        return;
    }

    HILOG_INFO("%{public}s DisconnectAbility called.", __func__);
    connectAbilityCB->cbBase.ability->DisconnectAbility(connectAbilityCB->abilityConnection);
    HILOG_INFO("%{public}s end. bundlename:%{public}s abilityname:%{public}s",
        __func__,
        connectAbilityCB->want.GetBundle().c_str(),
        connectAbilityCB->want.GetElement().GetAbilityName().c_str());
}

void DisConnectAbilityCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    napi_get_undefined(env, &undefined);
    result[PARAM0] = GetCallbackErrorValue(env, connectAbilityCB->errCode);
    if (connectAbilityCB->errCode == NAPI_ERR_NO_ERROR) {
        result[PARAM1] = WrapVoidToJS(env);
    } else {
        result[PARAM1] = WrapUndefinedToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, connectAbilityCB->cbBase.cbInfo.callback, &callback));
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, connectAbilityCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, connectAbilityCB->cbBase.asyncWork));
    delete connectAbilityCB;
    connectAbilityCB = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void DisConnectAbilityPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(data);
    napi_value result = nullptr;
    if (connectAbilityCB->errCode == NAPI_ERR_NO_ERROR) {
        result = WrapVoidToJS(env);
        napi_resolve_deferred(env, connectAbilityCB->cbBase.deferred, result);
    } else {
        result = GetCallbackErrorValue(env, connectAbilityCB->errCode);
        napi_reject_deferred(env, connectAbilityCB->cbBase.deferred, result);
    }

    napi_delete_async_work(env, connectAbilityCB->cbBase.asyncWork);
    delete connectAbilityCB;
    HILOG_INFO("%{public}s end.", __func__);
}

napi_value DisConnectAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &connectAbilityCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DisConnectAbilityExecuteCB,
            DisConnectAbilityCallbackCompletedCB,
            static_cast<void *>(connectAbilityCB),
            &connectAbilityCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, connectAbilityCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

napi_value DisConnectAbilityPromise(napi_env env, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    connectAbilityCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DisConnectAbilityExecuteCB,
            DisConnectAbilityPromiseCompletedCB,
            static_cast<void *>(connectAbilityCB),
            &connectAbilityCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, connectAbilityCB->cbBase.asyncWork));
    HILOG_INFO("%{public}s, promise end.", __func__);
    return promise;
}

/**
 * @brief DisConnectAbility processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param connectAbilityCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value DisConnectAbilityWrap(napi_env env, napi_callback_info info, ConnectAbilityCB *connectAbilityCB)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[PARAM0], &valuetype);
    if (valuetype == napi_number) {
        NAPI_CALL(env, napi_get_value_int64(env, args[PARAM0], &connectAbilityCB->id));
    }

    HILOG_INFO("%{public}s id:%{public}" PRId64, __func__, connectAbilityCB->id);
    int64_t id = connectAbilityCB->id;
    auto item = std::find_if(connects_.begin(),
        connects_.end(),
        [&id](const std::map<ConnecttionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return id == obj.first.id;
        });
    if (item != connects_.end()) {
        // match id
        connectAbilityCB->want = item->first.want;
        connectAbilityCB->abilityConnection = item->second;
        HILOG_INFO("%{public}s find conn ability exist", __func__);
    } else {
        HILOG_INFO("%{public}s there is no ability to disconnect.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = DisConnectAbilityAsync(env, args, ARGS_ONE, connectAbilityCB);
    } else {
        ret = DisConnectAbilityPromise(env, connectAbilityCB);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

/**
 * @brief DisConnectAbility.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_DisConnectAbilityCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s called.", __func__);
    ConnectAbilityCB *connectAbilityCB = CreateConnectAbilityCBInfo(env);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s connectAbilityCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    connectAbilityCB->errCode = NAPI_ERR_NO_ERROR;
    connectAbilityCB->cbBase.abilityType = abilityType;
    napi_value ret = DisConnectAbilityWrap(env, info, connectAbilityCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullptr", __func__);
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void NAPIAbilityConnection::SetEnv(const napi_env &env)
{
    env_ = env;
}

void NAPIAbilityConnection::SetConnectCBRef(const napi_ref &ref)
{
    connectRef_ = ref;
}

void NAPIAbilityConnection::SetDisconnectCBRef(const napi_ref &ref)
{
    disconnectRef_ = ref;
}

void UvWorkOnAbilityConnectDone(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnAbilityConnectDone, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityConnectDone, work is null");
        return;
    }
    // JS Thread
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(work->data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityConnectDone, connectAbilityCB is null");
        return;
    }
    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] =
        WrapElementName(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    napi_value jsRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->abilityConnectionCB.connection);
    result[PARAM1] = jsRemoteObject;

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(connectAbilityCB->cbBase.cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->cbBase.cbInfo.callback, &callback);

    napi_call_function(
        connectAbilityCB->cbBase.cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (connectAbilityCB->cbBase.cbInfo.callback != nullptr) {
        napi_delete_reference(connectAbilityCB->cbBase.cbInfo.env, connectAbilityCB->cbBase.cbInfo.callback);
    }
    if (connectAbilityCB != nullptr) {
        delete connectAbilityCB;
        connectAbilityCB = nullptr;
    }
    if (work != nullptr) {
        delete work;
        work = nullptr;
    }
    HILOG_INFO("UvWorkOnAbilityConnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_INFO("%{public}s, called.", __func__);
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s, remoteObject == nullptr.", __func__);
        return;
    }
    uv_loop_s *loop = nullptr;

    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work==nullptr.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, connectAbilityCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    connectAbilityCB->cbBase.cbInfo.env = env_;
    connectAbilityCB->cbBase.cbInfo.callback = connectRef_;
    connectAbilityCB->abilityConnectionCB.elementName = element;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    connectAbilityCB->abilityConnectionCB.connection = remoteObject;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityConnectDone);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

void UvWorkOnAbilityDisconnectDone(uv_work_t *work, int status)
{
    HILOG_INFO("UvWorkOnAbilityDisconnectDone, uv_queue_work");
    if (work == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityDisconnectDone, work is null");
        return;
    }
    // JS Thread
    ConnectAbilityCB *connectAbilityCB = static_cast<ConnectAbilityCB *>(work->data);
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("UvWorkOnAbilityDisconnectDone, connectAbilityCB is null");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_value result = WrapElementName(cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    if (cbInfo.callback != nullptr) {
        napi_value callback = nullptr;
        napi_value callResult = nullptr;
        napi_value undefined = nullptr;
        napi_get_undefined(cbInfo.env, &undefined);
        napi_get_reference_value(cbInfo.env, cbInfo.callback, &callback);
        napi_call_function(cbInfo.env, undefined, callback, ARGS_ONE, &result, &callResult);
        napi_delete_reference(cbInfo.env, cbInfo.callback);
        cbInfo.callback = nullptr;
    }

    // release connect
    HILOG_INFO("UvWorkOnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string deviceId = connectAbilityCB->abilityConnectionCB.elementName.GetDeviceID();
    std::string bundleName = connectAbilityCB->abilityConnectionCB.elementName.GetBundleName();
    std::string abilityName = connectAbilityCB->abilityConnectionCB.elementName.GetAbilityName();
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [deviceId, bundleName, abilityName](const std::map<ConnecttionKey,
            sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetDeviceId()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match deviceid & bundlename && abilityname
        connects_.erase(item);
        HILOG_INFO("UvWorkOnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
    }

    if (connectAbilityCB != nullptr) {
        delete connectAbilityCB;
        connectAbilityCB = nullptr;
    }
    if (work != nullptr) {
        delete work;
        work = nullptr;
    }
    HILOG_INFO("UvWorkOnAbilityDisconnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    HILOG_INFO("%{public}s, called.", __func__);

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("%{public}s, loop == nullptr.", __func__);
        return;
    }

    HILOG_INFO("%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
        __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        HILOG_ERROR("%{public}s, work == nullptr.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        HILOG_ERROR("%{public}s, connectAbilityCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }

    connectAbilityCB->cbBase.cbInfo.env = env_;
    connectAbilityCB->cbBase.cbInfo.callback = disconnectRef_;
    connectAbilityCB->abilityConnectionCB.elementName = element;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityDisconnectDone);
    if (rev != 0) {
        if (connectAbilityCB != nullptr) {
            delete connectAbilityCB;
            connectAbilityCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    HILOG_INFO("%{public}s, end.", __func__);
}

/**
 * @brief AcquireDataAbilityHelper.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_AcquireDataAbilityHelperCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    HILOG_INFO("%{public}s,called", __func__);
    DataAbilityHelperCB *dataAbilityHelperCB = new DataAbilityHelperCB;
    dataAbilityHelperCB->cbBase.cbInfo.env = env;
    dataAbilityHelperCB->cbBase.ability = nullptr; // temporary value assignment
    dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    dataAbilityHelperCB->cbBase.abilityType = abilityType;
    napi_value ret = AcquireDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s, ret == nullptr", __func__);
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

/**
 * @brief acquireDataAbilityHelper processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param dataAbilityHelperCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value AcquireDataAbilityHelperWrap(napi_env env, napi_callback_info info, DataAbilityHelperCB *dataAbilityHelperCB)
{
    HILOG_INFO("%{public}s,called", __func__);
    if (dataAbilityHelperCB == nullptr) {
        HILOG_ERROR("%{public}s,dataAbilityHelperCB == nullptr", __func__);
        return nullptr;
    }

    size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    size_t uriIndex = PARAM0;
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, args[0], stageMode);
    if (status != napi_ok) {
        HILOG_INFO("argv[0] is not a context, FA Model");
    } else {
        uriIndex = PARAM1;
        HILOG_INFO("argv[0] is a context, Stage Model: %{public}d", stageMode);
    }

    if (!stageMode) {
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            HILOG_ERROR("Failed to get native context instance");
            return nullptr;
        }
        dataAbilityHelperCB->cbBase.ability = ability;

        if (!CheckAbilityType(&dataAbilityHelperCB->cbBase)) {
            dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
            HILOG_ERROR("%{public}s ability type invalid.", __func__);
            return nullptr;
        }
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[uriIndex], &valuetype));
    if (valuetype != napi_string) {
        HILOG_ERROR("%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, GetGlobalDataAbilityHelper(env), uriIndex + 1, &args[PARAM0], &result));

    if (!IsTypeForNapiValue(env, result, napi_object)) {
        HILOG_ERROR("%{public}s, IsTypeForNapiValue isn`t object", __func__);
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_null)) {
        HILOG_ERROR("%{public}s, IsTypeForNapiValue is null", __func__);
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_undefined)) {
        HILOG_ERROR("%{public}s, IsTypeForNapiValue is undefined", __func__);
        return nullptr;
    }

    if (!GetDataAbilityHelperStatus()) {
        HILOG_ERROR("%{public}s, GetDataAbilityHelperStatus is false", __func__);
        return nullptr;
    }

    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
    HILOG_INFO("%{public}s,end", __func__);
    return result;
}

napi_value UnwrapParamForWantAgent(napi_env &env, napi_value &args, AbilityRuntime::WantAgent::WantAgent *&wantAgent)
{
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args, &valuetype));
    NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
    napi_value wantAgentParam = nullptr;
    napi_value result = nullptr;

    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, args, "wantAgent", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, args, "wantAgent", &wantAgentParam);
        NAPI_CALL(env, napi_typeof(env, wantAgentParam, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_object, "Wrong argument type. Object expected.");
        napi_unwrap(env, wantAgentParam, reinterpret_cast<void **>(&wantAgent));
    }

    napi_get_null(env, &result);
    return result;
}

void StartBackgroundRunningExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullptr", __func__);
        return;
    }
    if (asyncCallbackInfo->errCode == NAPI_ERR_PARAM_INVALID) {
        HILOG_ERROR("parse input param failed");
        return;
    }
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        HILOG_ERROR("%{public}s ability == nullptr", __func__);
        return;
    }
    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("abilityinfo is null");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    AbilityRuntime::WantAgent::WantAgent wantAgentObj;
    if (!asyncCallbackInfo->wantAgent) {
        HILOG_WARN("input param without wantAgent");
        wantAgentObj = AbilityRuntime::WantAgent::WantAgent();
    } else {
        wantAgentObj = *asyncCallbackInfo->wantAgent;
    }
    asyncCallbackInfo->errCode = asyncCallbackInfo->ability->StartBackgroundRunning(wantAgentObj);

    HILOG_INFO("%{public}s end.", __func__);
}

void BackgroundRunningCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    napi_get_undefined(env, &undefined);
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        result[0] = WrapUndefinedToJS(env);
        napi_create_int32(env, 0, &result[1]);
    } else {
        result[1] = WrapUndefinedToJS(env);
        result[0] = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
    }

    napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
    napi_call_function(env, undefined, callback, ARGS_TWO, result, &callResult);

    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
    }
    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;
    HILOG_INFO("%{public}s end.", __func__);
}

void BackgroundRunningPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    napi_value result = nullptr;
    if (asyncCallbackInfo->errCode == NAPI_ERR_NO_ERROR) {
        napi_create_int32(env, 0, &result);
        napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
    } else {
        result = GetCallbackErrorValue(env, asyncCallbackInfo->errCode);
        napi_reject_deferred(env, asyncCallbackInfo->deferred, result);
    }

    napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, end.", __func__);
    delete asyncCallbackInfo;
}

napi_value StartBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartBackgroundRunningExecuteCB,
            BackgroundRunningCallbackCompletedCB,
            static_cast<void *>(asyncCallbackInfo),
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    HILOG_INFO("%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

napi_value StartBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            StartBackgroundRunningExecuteCB,
            BackgroundRunningPromiseCompletedCB,
            static_cast<void *>(asyncCallbackInfo),
            &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    HILOG_INFO("%{public}s, end.", __func__);
    return promise;
}

napi_value StartBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t paramNums = 3;
    const size_t minParamNums = 2;
    const size_t maxParamNums = 3;
    napi_value args[maxParamNums] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNums, args, NULL, NULL));

    if (paramNums < minParamNums || paramNums > maxParamNums) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (UnwrapParamForWantAgent(env, args[1], asyncCallbackInfo->wantAgent) == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_PARAM_INVALID;
    }

    if (paramNums == maxParamNums) {
        ret = StartBackgroundRunningAsync(env, args, maxParamNums - 1, asyncCallbackInfo);
    } else {
        ret = StartBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_StartBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = StartBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

void CancelBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo->ability != nullptr) {
        asyncCallbackInfo->errCode = asyncCallbackInfo->ability->StopBackgroundRunning();
    } else {
        HILOG_ERROR("NAPI_PACancelBackgroundRunning, ability == nullptr");
    }
}

napi_value CancelBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        CancelBackgroundRunningExecuteCB,
        BackgroundRunningCallbackCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    HILOG_INFO("%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value CancelBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    napi_deferred deferred;
    napi_value promise = nullptr;
    napi_create_promise(env, &deferred, &promise);

    asyncCallbackInfo->deferred = deferred;

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        CancelBackgroundRunningExecuteCB,
        BackgroundRunningPromiseCompletedCB,
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work(env, asyncCallbackInfo->asyncWork);
    HILOG_INFO("%{public}s, promise end", __func__);
    return promise;
}

napi_value CancelBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s called.", __func__);
    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, NULL, NULL));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = CancelBackgroundRunningAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = CancelBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_CancelBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = CancelBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

napi_value TerminateAbilityWrap(napi_env env, napi_callback_info info, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        HILOG_ERROR("%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = TerminateAbilityAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = TerminateAbilityPromise(env, asyncCallbackInfo);
    }
    HILOG_INFO("%{public}s, asyncCallback end", __func__);
    return ret;
}

napi_value TerminateAbilityAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback));
    }

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("%{public}s, worker pool thread execute.", __func__);
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->TerminateAbility();
            } else {
                HILOG_ERROR("%{public}s, ability == nullptr", __func__);
            }
            HILOG_INFO("%{public}s, worker pool thread execute end.", __func__);
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("%{public}s, main event thread complete.", __func__);
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value callback = nullptr;
            napi_value undefined = nullptr;
            napi_value result[ARGS_TWO] = {nullptr};
            napi_value callResult = nullptr;
            napi_get_undefined(env, &undefined);
            result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
            napi_get_null(env, &result[PARAM1]);
            napi_get_reference_value(env, asyncCallbackInfo->cbInfo.callback, &callback);
            napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

            if (asyncCallbackInfo->cbInfo.callback != nullptr) {
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            HILOG_INFO("%{public}s, main event thread complete end.", __func__);
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value TerminateAbilityPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    HILOG_INFO("%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));

    asyncCallbackInfo->deferred = deferred;

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void *data) {
            HILOG_INFO("%{public}s, worker pool thread execute.", __func__);
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            if (asyncCallbackInfo->ability != nullptr) {
                asyncCallbackInfo->ability->TerminateAbility();
            } else {
                HILOG_INFO("%{public}s, ability == nullptr", __func__);
            }
            HILOG_INFO("%{public}s, worker pool thread execute end.", __func__);
        },
        [](napi_env env, napi_status status, void *data) {
            HILOG_INFO("%{public}s, main event thread complete.", __func__);
            AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
            napi_value result = nullptr;
            napi_get_null(env, &result);
            napi_resolve_deferred(env, asyncCallbackInfo->deferred, result);
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            HILOG_INFO("%{public}s, main event thread complete end.", __func__);
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));
    HILOG_INFO("%{public}s, promise end", __func__);
    return promise;
}

napi_value NAPI_TerminateAbilityCommon(napi_env env, napi_callback_info info)
{
    HILOG_INFO("%{public}s,called", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        HILOG_ERROR("%{public}s,asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    napi_value ret = TerminateAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        HILOG_ERROR("%{public}s,ret == nullptr", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("%{public}s,end", __func__);
    return ret;
}

JsNapiCommon::JsNapiCommon() : ability_(nullptr)
{}

NativeValue* JsNapiCommon::JsConnectAbility(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    int32_t errorVal = static_cast<int32_t>(NAPI_ERR_NO_ERROR);
    int64_t id = 0;
    HILOG_DEBUG("%{public}s is called", __func__);
    if (info.argc != ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    auto env = reinterpret_cast<napi_env>(&engine);
    auto firstParam = reinterpret_cast<napi_value>(info.argv[PARAM0]);
    auto secondParam = reinterpret_cast<napi_value>(info.argv[PARAM1]);
    Want want;
    if (!UnwrapWant(env, firstParam, want)) {
        HILOG_ERROR("called. Invoke UnwrapWant fail");
        return engine.CreateUndefined();
    }
    sptr<NAPIAbilityConnection> abilityConnection = BuildWant(want, id);
    if (abilityConnection == nullptr) {
        HILOG_ERROR("error, the abilityConnection is nullptr");
        return engine.CreateUndefined();
    }
    napi_ref callbackArray[PARAM3];
    ChangeAbilityConnection(callbackArray, env, secondParam);
    abilityConnection->SetEnv(env);
    abilityConnection->SetConnectCBRef(callbackArray[PARAM0]);
    abilityConnection->SetDisconnectCBRef(callbackArray[PARAM1]);

    if (ability_ == nullptr) {
        HILOG_ERROR("JsConnectAbility, the ability is nullptr");
        return engine.CreateUndefined();
    }

    bool result = false;
    if (!CheckAbilityType(abilityType)) {
        errorVal = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
    } else {
        result = ability_->ConnectAbility(want, abilityConnection);
    }

    if (errorVal != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || result == false) {
        HILOG_ERROR("CommonJsConnectAbility failed.");
        // return error code in onFailed asynccallback
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_value resultVal = nullptr;
        napi_value callResult = nullptr;
        int errorCode = NO_ERROR;
        switch (errorVal) {
            case NAPI_ERR_ACE_ABILITY:
                errorCode = ABILITY_NOT_FOUND;
                break;
            case NAPI_ERR_PARAM_INVALID:
                errorCode = INVALID_PARAMETER;
                break;
            default:
                break;
        }
        NAPI_CALL_BASE(env, napi_create_int32(env, errorCode, &resultVal), engine.CreateUndefined());
        NAPI_CALL_BASE(
            env, napi_get_reference_value(env, callbackArray[PARAM2], &callback), engine.CreateUndefined());
        NAPI_CALL_BASE(env, napi_call_function(env, undefined, callback, ARGS_ONE, &resultVal, &callResult),
            engine.CreateUndefined());
    }
    return CreateJsValue(engine, id);
}

NativeValue* JsNapiCommon::JsDisConnectAbility(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("%{public}s is called", __func__);
    if (info.argc == ARGS_ZERO || info.argc > ARGS_TWO) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    int64_t id = 0;
    sptr<NAPIAbilityConnection> abilityConnection = nullptr;
    if (!ConvertFromJsValue(engine, info.argv[PARAM0], id)) {
        HILOG_ERROR("input params int error");
        return engine.CreateUndefined();
    }
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [&id](const std::map<ConnecttionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return id == obj.first.id;
        });
    if (item != connects_.end()) {
        abilityConnection = item->second;
        HILOG_DEBUG("find conn ability exist");
    } else {
        HILOG_ERROR("there is no ability to disconnect.");
        return engine.CreateUndefined();
    }
    auto execute = [obj = this, value = errorVal, abilityType, abilityConnection] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr.");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        *value = obj->ability_->DisconnectAbility(abilityConnection);
    };
    auto complete = [obj = this, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, const int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR)) {
            task.Reject(engine, CreateJsError(engine, *value, "DisconnectAbility failed."));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, *value));
    };
    NativeValue *lastParam = (info.argc == ARGS_ONE) ? nullptr : info.argv[PARAM1];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsDisConnectAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

sptr<NAPIAbilityConnection> JsNapiCommon::BuildWant(const Want &want, int64_t &id)
{
    HILOG_DEBUG("%{public}s uri:%{public}s", __func__, want.GetElement().GetURI().c_str());
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string bundleName = want.GetBundle();
    std::string abilityName = want.GetElement().GetAbilityName();
    auto item = std::find_if(connects_.begin(),
        connects_.end(), [&deviceId, &bundleName, &abilityName](const std::map<ConnecttionKey,
        sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetElement().GetDeviceID()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    sptr<NAPIAbilityConnection> abilityConnection;
    if (item != connects_.end()) {
        id = item->first.id;
        abilityConnection = item->second;
        HILOG_DEBUG("find connection exist");
    } else {
        sptr<NAPIAbilityConnection> conn(new (std::nothrow) NAPIAbilityConnection());
        id = serialNumber_;
        abilityConnection = conn;
        ConnecttionKey key;
        key.id = id;
        key.want = want;
        connects_.emplace(key, conn);
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_DEBUG("not find connection, make new one");
    }
    HILOG_DEBUG("id:%{public}" PRId64, id);
    return abilityConnection;
}

void JsNapiCommon::ChangeAbilityConnection(napi_ref *callbackArray, const napi_env env, const napi_value &arg1)
{
    napi_value jsMethod = nullptr;
    napi_get_named_property(env, arg1, "onConnect", &jsMethod);
    napi_create_reference(env, jsMethod, 1, &callbackArray[PARAM0]);
    napi_get_named_property(env, arg1, "onDisconnect", &jsMethod);
    napi_create_reference(env, jsMethod, 1, &callbackArray[PARAM1]);
    napi_get_named_property(env, arg1, "onFailed", &jsMethod);
    napi_create_reference(env, jsMethod, 1, &callbackArray[PARAM2]);
}

NativeValue* JsNapiCommon::JsGetContext(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    if (!CheckAbilityType(abilityType)) {
        HILOG_ERROR("ability type error");
        return engine.CreateUndefined();
    }

    return CreateNapiJSContext(engine);
}

NativeValue* JsNapiCommon::JsGetFilesDir(NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("JsGetFilesDir called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("JsGetFilesDir input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsFilesDir> filesDir = std::make_shared<JsFilesDir>();
    auto execute = [obj = this, dir = filesDir, abilityType, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("JsGetFilesDir task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("JsGetFilesDir task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetFilesDir();
    };
    auto complete = [obj = this, dir = filesDir, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, dir->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetFilesDir",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsNapiCommon::JsIsUpdatingConfigurations(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("JsIsUpdatingConfigurations called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("JsIsUpdatingConfigurations input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsConfigurations> config = std::make_shared<JsConfigurations>();
    auto execute = [obj = this, data = config, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("JsIsUpdatingConfigurations task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (data == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("JsIsUpdatingConfigurations task execute error, param is nullptr");
            return;
        }
        data->status = obj->ability_->IsUpdatingConfigurations();
    };
    auto complete = [obj = this, info = config, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, engine.CreateBoolean(info->status));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsIsUpdatingConfigurations",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsPrintDrawnCompleted(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("JsPrintDrawnCompleted called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("JsPrintDrawnCompleted input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsDrawnCompleted> drawComplete = std::make_shared<JsDrawnCompleted>();
    auto execute = [obj = this, data = drawComplete, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("JsPrintDrawnCompleted task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (data == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("JsPrintDrawnCompleted task execute error, data is nullptr");
            return;
        }
        data->status = obj->ability_->PrintDrawnCompleted();
    };
    auto complete = [obj = this, draw = drawComplete, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || draw == nullptr) {
            auto ecode = draw == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, engine.CreateNull());
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsPrintDrawnCompleted",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetCacheDir(NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("JsGetCacheDir called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("JsGetCacheDir input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCacheDir> cacheDir = std::make_shared<JsCacheDir>();
    auto execute = [obj = this, dir = cacheDir, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("JsGetCacheDir task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            HILOG_ERROR("JsGetCacheDir task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetCacheDir();
    };
    auto complete = [obj = this, dir = cacheDir, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, dir->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetCacheDir",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetCtxAppType(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCtxAppType> type = std::make_shared<JsCtxAppType>();
    auto execute = [obj = this, apptype = type, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (apptype == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            return;
        }
        apptype->name = obj->ability_->GetAppType();
    };
    auto complete = [obj = this, apptype = type, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || apptype == nullptr) {
            auto ecode = apptype == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, apptype->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetCtxAppType",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetCtxHapModuleInfo(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsHapModuleInfo> infoData = std::make_shared<JsHapModuleInfo>();
    auto execute = [obj = this, hapMod = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto getInfo = obj->ability_->GetHapModuleInfo();
        if (getInfo != nullptr && hapMod != nullptr) {
            hapMod->hapModInfo = *getInfo;
        } else {
            HILOG_ERROR("GetHapModuleInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, obj->CreateHapModuleInfo(engine, info));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetCtxHapModuleInfo",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetAppVersionInfo(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsApplicationInfo> infoData = std::make_shared<JsApplicationInfo>();
    auto execute = [obj = this, appInfo = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto getInfo = obj->ability_->GetApplicationInfo();
        if (getInfo != nullptr && appInfo != nullptr) {
            appInfo->appInfo = *getInfo;
        } else {
            HILOG_ERROR("GetApplicationInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, obj->CreateAppVersionInfo(engine, info));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetAppVersionInfo",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetCtxAbilityInfo(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsAbilityInfoInfo> infoData = std::make_shared<JsAbilityInfoInfo>();
    auto execute = [obj = this, abilityInfo = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto getInfo = obj->ability_->GetAbilityInfo();
        if (getInfo != nullptr && abilityInfo != nullptr) {
            abilityInfo->abilityInfo = *getInfo;
        } else {
            HILOG_ERROR("GetAbilityInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, obj->CreateAbilityInfo(engine, info));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetCtxAbilityInfo",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetOrCreateDistributedDir(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsOrCreateDistributedDir> orCreateDistributedDir = std::make_shared<JsOrCreateDistributedDir>();
    auto execute = [obj = this, dir = orCreateDistributedDir, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetDistributedFilesDir();
    };
    auto complete = [obj = this, dir = orCreateDistributedDir, value = errorVal]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(engine, CreateJsValue(engine, dir->name));
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetOrCreateDistributedDir",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::JsGetDisplayOrientation(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("called");
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto execute = [obj = this, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        *value = obj->ability_->GetDisplayOrientation();
    };
    auto complete = [errorVal] (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*errorVal == NAPI_ERR_ACE_ABILITY) {
            task.Reject(engine, CreateJsError(engine, NAPI_ERR_ACE_ABILITY, "ability is nullptr"));
        } else if (*errorVal == NAPI_ERR_ABILITY_TYPE_INVALID) {
            task.Reject(engine, CreateJsError(engine, NAPI_ERR_ABILITY_TYPE_INVALID, "ability type is invalid."));
        } else if (*errorVal == NAPI_ERR_NO_WINDOW) {
            task.Reject(engine, CreateJsError(engine, NAPI_ERR_NO_WINDOW, "window is nullptr"));
        } else {
            task.Resolve(engine, CreateJsValue(engine, *errorVal));
        }
    };

    auto callback = info.argc == ARGS_ZERO ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetDisplayOrientation",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsNapiCommon::CreateProcessInfo(NativeEngine &engine, const std::shared_ptr<JsProcessInfo> &processInfo)
{
    HILOG_DEBUG("CreateProcessInfo called");
    CHECK_POINTER_AND_RETURN_LOG(processInfo, engine.CreateUndefined(), "input params error");
    auto objContext = engine.CreateObject();
    CHECK_POINTER_AND_RETURN_LOG(objContext, engine.CreateUndefined(), "CreateObject failed");
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    CHECK_POINTER_AND_RETURN_LOG(object, engine.CreateUndefined(), "ConvertNativeValueTo object failed");

    object->SetProperty("processName", CreateJsValue(engine, processInfo->processName));
    object->SetProperty("pid", CreateJsValue(engine, processInfo->pid));

    return objContext;
}

NativeValue* JsNapiCommon::CreateElementName(NativeEngine &engine, const std::shared_ptr<JsElementName> &elementName)
{
    HILOG_DEBUG("CreateElementName called");
    CHECK_POINTER_AND_RETURN_LOG(elementName, engine.CreateUndefined(), "input params error");
    auto objContext = engine.CreateObject();
    CHECK_POINTER_AND_RETURN_LOG(objContext, engine.CreateUndefined(), "CreateObject failed");
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    CHECK_POINTER_AND_RETURN_LOG(object, engine.CreateUndefined(), "ConvertNativeValueTo object failed");

    object->SetProperty("deviceId", CreateJsValue(engine, elementName->deviceId));
    object->SetProperty("bundleName", CreateJsValue(engine, elementName->bundleName));
    object->SetProperty("abilityName", CreateJsValue(engine, elementName->abilityName));
    object->SetProperty("uri", CreateJsValue(engine, elementName->uri));
    object->SetProperty("shortName", CreateJsValue(engine, elementName->shortName));

    return objContext;
}

NativeValue* JsNapiCommon::CreateHapModuleInfo(
    NativeEngine &engine, const std::shared_ptr<JsHapModuleInfo> &hapModInfo)
{
    HILOG_DEBUG("CreateHapModuleInfo called");
    CHECK_POINTER_AND_RETURN_LOG(hapModInfo, engine.CreateUndefined(), "input params error");
    auto objContext = engine.CreateObject();
    CHECK_POINTER_AND_RETURN_LOG(objContext, engine.CreateUndefined(), "CreateObject failed");
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    CHECK_POINTER_AND_RETURN_LOG(object, engine.CreateUndefined(), "ConvertNativeValueTo object failed");

    object->SetProperty("name", CreateJsValue(engine, hapModInfo->hapModInfo.name));
    object->SetProperty("description", CreateJsValue(engine, hapModInfo->hapModInfo.description));
    object->SetProperty("icon", CreateJsValue(engine, hapModInfo->hapModInfo.iconPath));
    object->SetProperty("label", CreateJsValue(engine, hapModInfo->hapModInfo.label));
    object->SetProperty("backgroundImg", CreateJsValue(engine, hapModInfo->hapModInfo.backgroundImg));
    object->SetProperty("moduleName", CreateJsValue(engine, hapModInfo->hapModInfo.moduleName));
    object->SetProperty("mainAbilityName", CreateJsValue(engine, hapModInfo->hapModInfo.mainAbility));
    object->SetProperty("supportedModes", CreateJsValue(engine, hapModInfo->hapModInfo.supportedModes));
    object->SetProperty("descriptionId", CreateJsValue(engine, hapModInfo->hapModInfo.descriptionId));
    object->SetProperty("labelId", CreateJsValue(engine, hapModInfo->hapModInfo.labelId));
    object->SetProperty("iconId", CreateJsValue(engine, hapModInfo->hapModInfo.iconId));
    object->SetProperty("installationFree", engine.CreateBoolean(hapModInfo->hapModInfo.installationFree));
    object->SetProperty("reqCapabilities", CreateNativeArray(engine, hapModInfo->hapModInfo.reqCapabilities));
    object->SetProperty("deviceTypes", CreateNativeArray(engine, hapModInfo->hapModInfo.deviceTypes));
    object->SetProperty("abilityInfo", CreateAbilityInfos(engine, hapModInfo->hapModInfo.abilityInfos));

    return objContext;
}

NativeValue* JsNapiCommon::CreateModuleInfo(NativeEngine &engine, const ModuleInfo &modInfo)
{
    auto objContext = engine.CreateObject();
    if (objContext == nullptr) {
        HILOG_ERROR("CreateObject failed");
        return engine.CreateUndefined();
    }
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    if (object == nullptr) {
        HILOG_ERROR("ConvertNativeValueTo object failed");
        return engine.CreateUndefined();
    }

    object->SetProperty("moduleName", CreateJsValue(engine, modInfo.moduleName));
    object->SetProperty("moduleSourceDir", CreateJsValue(engine, modInfo.moduleSourceDir));

    return objContext;
}

NativeValue* JsNapiCommon::CreateModuleInfos(NativeEngine &engine, const std::vector<ModuleInfo> &moduleInfos)
{
    auto arrayValue = engine.CreateArray(moduleInfos.size());
    auto array = ConvertNativeValueTo<NativeArray>(arrayValue);
    if (array == nullptr) {
        HILOG_ERROR("CreateArray failed");
        return engine.CreateUndefined();
    }
    for (uint32_t i = 0; i < moduleInfos.size(); i++) {
        array->SetElement(i, CreateModuleInfo(engine, moduleInfos.at(i)));
    }

    return arrayValue;
}

NativeValue* JsNapiCommon::CreateAppInfo(NativeEngine &engine, const ApplicationInfo &appInfo)
{
    HILOG_DEBUG("CreateAppInfo called");
    auto objContext = engine.CreateObject();
    if (objContext == nullptr) {
        HILOG_ERROR("CreateAppInfo, CreateObject failed");
        return engine.CreateUndefined();
    }
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    if (object == nullptr) {
        HILOG_ERROR("CreateAppInfo, ConvertNativeValueTo object failed");
        return engine.CreateUndefined();
    }

    object->SetProperty("name", CreateJsValue(engine, appInfo.name));
    object->SetProperty("description", CreateJsValue(engine, appInfo.description));
    object->SetProperty("descriptionId", CreateJsValue(engine, appInfo.descriptionId));
    object->SetProperty("systemApp", CreateJsValue(engine, appInfo.isSystemApp));
    object->SetProperty("enabled", CreateJsValue(engine, appInfo.enabled));
    object->SetProperty("label", CreateJsValue(engine, appInfo.label));
    object->SetProperty("labelId", CreateJsValue(engine, std::to_string(appInfo.labelId)));
    object->SetProperty("icon", CreateJsValue(engine, appInfo.iconPath));
    object->SetProperty("iconId", CreateJsValue(engine, std::to_string(appInfo.iconId)));
    object->SetProperty("process", CreateJsValue(engine, appInfo.process));
    object->SetProperty("entryDir", CreateJsValue(engine, appInfo.entryDir));
    object->SetProperty("supportedModes", CreateJsValue(engine, appInfo.supportedModes));
    object->SetProperty("moduleSourceDirs", CreateNativeArray(engine, appInfo.moduleSourceDirs));
    object->SetProperty("permissions", CreateNativeArray(engine, appInfo.permissions));
    object->SetProperty("moduleInfos", CreateModuleInfos(engine, appInfo.moduleInfos));

    return objContext;
}

NativeValue* JsNapiCommon::CreateAppInfo(NativeEngine &engine, const std::shared_ptr<JsApplicationInfo> &appInfo)
{
    if (appInfo == nullptr) {
        HILOG_ERROR("input param error");
        return engine.CreateUndefined();
    }

    return CreateAppInfo(engine, appInfo->appInfo);
}

NativeValue* JsNapiCommon::CreateAbilityInfo(NativeEngine &engine, const AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("CreateAbilityInfo called");
    auto objContext = engine.CreateObject();
    if (objContext == nullptr) {
        HILOG_ERROR("CreateAbilityInfo, CreateObject failed");
        return engine.CreateUndefined();
    }
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    if (object == nullptr) {
        HILOG_ERROR("CreateAbilityInfo, ConvertNativeValueTo object failed");
        return engine.CreateUndefined();
    }

    object->SetProperty("bundleName", CreateJsValue(engine, abilityInfo.bundleName));
    object->SetProperty("name", CreateJsValue(engine, abilityInfo.name));
    object->SetProperty("label", CreateJsValue(engine, abilityInfo.label));
    object->SetProperty("description", CreateJsValue(engine, abilityInfo.description));
    object->SetProperty("icon", CreateJsValue(engine, abilityInfo.iconPath));
    object->SetProperty("moduleName", CreateJsValue(engine, abilityInfo.moduleName));
    object->SetProperty("process", CreateJsValue(engine, abilityInfo.process));
    object->SetProperty("uri", CreateJsValue(engine, abilityInfo.uri));
    object->SetProperty("readPermission", CreateJsValue(engine, abilityInfo.readPermission));
    object->SetProperty("writePermission", CreateJsValue(engine, abilityInfo.writePermission));
    object->SetProperty("targetAbility", CreateJsValue(engine, abilityInfo.targetAbility));
    object->SetProperty("type", CreateJsValue(engine, static_cast<int32_t>(abilityInfo.type)));
    object->SetProperty("orientation", CreateJsValue(engine, static_cast<int32_t>(abilityInfo.orientation)));
    object->SetProperty("launchMode", CreateJsValue(engine, static_cast<int32_t>(abilityInfo.launchMode)));
    object->SetProperty("labelId", CreateJsValue(engine, abilityInfo.labelId));
    object->SetProperty("descriptionId", CreateJsValue(engine, abilityInfo.descriptionId));
    object->SetProperty("iconId", CreateJsValue(engine, abilityInfo.iconId));
    object->SetProperty("formEntity", CreateJsValue(engine, abilityInfo.formEntity));
    object->SetProperty("minFormHeight", CreateJsValue(engine, abilityInfo.minFormHeight));
    object->SetProperty("defaultFormHeight", CreateJsValue(engine, abilityInfo.defaultFormHeight));
    object->SetProperty("minFormWidth", CreateJsValue(engine, abilityInfo.minFormWidth));
    object->SetProperty("defaultFormWidth", CreateJsValue(engine, abilityInfo.defaultFormWidth));
    object->SetProperty("backgroundModes", CreateJsValue(engine, abilityInfo.backgroundModes));
    object->SetProperty("subType", CreateJsValue(engine, static_cast<int32_t>(abilityInfo.subType)));
    object->SetProperty("isVisible", CreateJsValue(engine, abilityInfo.visible));
    object->SetProperty("formEnabled", CreateJsValue(engine, abilityInfo.formEnabled));
    object->SetProperty("permissions", CreateNativeArray(engine, abilityInfo.permissions));
    object->SetProperty("deviceCapabilities", CreateNativeArray(engine, abilityInfo.deviceCapabilities));
    object->SetProperty("deviceTypes", CreateNativeArray(engine, abilityInfo.deviceTypes));
    object->SetProperty("applicationInfo", CreateAppInfo(engine, abilityInfo.applicationInfo));
    return objContext;
}

NativeValue* JsNapiCommon::CreateAbilityInfo(
    NativeEngine &engine, const std::shared_ptr<JsAbilityInfoInfo> &abilityInfo)
{
    HILOG_DEBUG("called");
    if (abilityInfo == nullptr) {
        HILOG_ERROR("called");
        return engine.CreateUndefined();
    }

    return CreateAbilityInfo(engine, abilityInfo->abilityInfo);
}

NativeValue* JsNapiCommon::CreateAbilityInfos(NativeEngine &engine, const std::vector<AbilityInfo> &abilityInfos)
{
    auto arrayValue = engine.CreateArray(abilityInfos.size());
    auto array = ConvertNativeValueTo<NativeArray>(arrayValue);
    if (array == nullptr) {
        HILOG_ERROR("CreateArray failed");
        return engine.CreateUndefined();
    }
    for (uint32_t i = 0; i < abilityInfos.size(); i++) {
        array->SetElement(i, CreateAbilityInfo(engine, abilityInfos.at(i)));
    }

    return arrayValue;
}

bool JsNapiCommon::CheckAbilityType(const AbilityType typeWant)
{
    if (ability_ == nullptr) {
        HILOG_ERROR("input params int error");
        return false;
    }
    const std::shared_ptr<AbilityInfo> info = ability_->GetAbilityInfo();
    if (info == nullptr) {
        HILOG_ERROR("get ability info error");
        return false;
    }

    switch (typeWant) {
        case AbilityType::PAGE:
            if (static_cast<AbilityType>(info->type) == AbilityType::PAGE ||
                static_cast<AbilityType>(info->type) == AbilityType::DATA) {
                return true;
            }
            return false;
        default:
            return static_cast<AbilityType>(info->type) != AbilityType::PAGE;
    }
    return false;
}

NativeValue* JsNapiCommon::CreateAppVersionInfo(
    NativeEngine &engine, const std::shared_ptr<JsApplicationInfo> &appInfo)
{
    HILOG_DEBUG("CreateAppVersionInfo called");
    CHECK_POINTER_AND_RETURN_LOG(appInfo, engine.CreateUndefined(), "input params error");
    auto objContext = engine.CreateObject();
    CHECK_POINTER_AND_RETURN_LOG(objContext, engine.CreateUndefined(), "CreateObject failed");
    auto object = ConvertNativeValueTo<NativeObject>(objContext);
    CHECK_POINTER_AND_RETURN_LOG(object, engine.CreateUndefined(), "ConvertNativeValueTo object failed");

    object->SetProperty("appName", CreateJsValue(engine, appInfo->appInfo.name));
    object->SetProperty("versionName", CreateJsValue(engine, appInfo->appInfo.versionName));
    object->SetProperty("versionCode", CreateJsValue(engine, static_cast<int32_t>(appInfo->appInfo.versionCode)));

    return objContext;
}

bool JsNapiCommon::UnwarpVerifyPermissionParams(
    NativeEngine &engine, NativeCallbackInfo &info, JsPermissionOptions &options)
{
    bool flagCall = true;
    if (info.argc == ARGS_ONE) {
        flagCall = false;
    } else if (info.argc == ARGS_TWO && info.argv[PARAM1]->TypeOf() != NATIVE_FUNCTION) {
        if (!GetPermissionOptions(engine, info.argv[PARAM1], options)) {
            HILOG_WARN("input params string error");
        }
        flagCall = false;
    } else if (info.argc == ARGS_THREE) {
        if (!GetPermissionOptions(engine, info.argv[PARAM1], options)) {
            HILOG_WARN("input params string error");
        }
    }

    return flagCall;
}

bool JsNapiCommon::GetStringsValue(NativeEngine &engine, NativeValue *object, std::vector<std::string> &strList)
{
    auto array = ConvertNativeValueTo<NativeArray>(object);
    if (array == nullptr) {
        HILOG_ERROR("input params error");
        return false;
    }
    for (uint32_t i = 0; i < array->GetLength(); i++) {
        std::string itemStr("");
        if (!ConvertFromJsValue(engine, array->GetElement(i), itemStr)) {
            HILOG_ERROR("GetElement from to array [%{public}u] error", i);
            return false;
        }
        strList.push_back(itemStr);
    }

    return true;
}

bool JsNapiCommon::GetPermissionOptions(NativeEngine &engine, NativeValue *object, JsPermissionOptions &options)
{
    auto obj = ConvertNativeValueTo<NativeObject>(object);
    if (obj == nullptr) {
        HILOG_ERROR("input params error");
        return false;
    }

    options.uidFlag = ConvertFromJsValue(engine, obj->GetProperty("uid"), options.uid);
    options.pidFlag = ConvertFromJsValue(engine, obj->GetProperty("pid"), options.pid);

    return true;
}

std::string JsNapiCommon::ConvertErrorCode(int32_t errCode)
{
    static std::map<int32_t, std::string> errMap = {
        { static_cast<int32_t>(NAPI_ERR_ACE_ABILITY), std::string("get ability error") },
        { static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID), std::string("ability call error") },
        { static_cast<int32_t>(NAPI_ERR_PARAM_INVALID), std::string("input param error") },
        { static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID), std::string("ability type error") }
    };
    auto findECode = errMap.find(errCode);
    if (findECode == errMap.end()) {
        HILOG_ERROR("convert error code failed");
        return std::string("execution failed");
    }

    return findECode->second;
}

NativeValue* JsNapiCommon::JsGetWant(
    NativeEngine &engine, NativeCallbackInfo &info, const AbilityType abilityType)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (info.argc > ARGS_ONE) {
        HILOG_ERROR("input params count error, argc=%{public}zu", info.argc);
        return engine.CreateUndefined();
    }

    std::shared_ptr<JsWant> pwant = std::make_shared<JsWant>();
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto execute = [obj = this, want = pwant, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            HILOG_ERROR("task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            HILOG_ERROR("task execute error, the abilityType is error");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }

        auto wantData = obj->ability_->GetWant();
        if (wantData == nullptr || want == nullptr) {
            HILOG_ERROR("wantData or want is nullptr!");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            return;
        }
        want->want = *wantData;
    };

    auto complete = [obj = this, value = errorVal, pwant]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
        if (*value == NAPI_ERR_NO_ERROR && pwant != nullptr) {
            task.Resolve(engine, obj->CreateWant(engine, pwant));
        } else {
            auto error = (pwant == nullptr) ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(engine, CreateJsError(engine, error, "GetAbilityInfo return nullptr"));
        }
    };

    auto callback = (info.argc == ARGS_ZERO) ? nullptr : info.argv[PARAM0];
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JsNapiCommon::JsGetWant",
        engine, CreateAsyncTaskWithLastParam(engine, callback, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsNapiCommon::CreateWant(NativeEngine& engine, const std::shared_ptr<JsWant> &want)
{
    HILOG_DEBUG("%{public}s,called", __func__);
    if (want == nullptr) {
        HILOG_DEBUG("%{public}s,called", __func__);
        return engine.CreateUndefined();
    }

    return CreateJsWant(engine, want->want);
}
}  // namespace AppExecFwk
}  // namespace OHOS
