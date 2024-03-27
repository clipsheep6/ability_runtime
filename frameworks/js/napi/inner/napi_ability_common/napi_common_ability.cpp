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

#include <chrono>
#include <dlfcn.h>
#include <memory>
#include <uv.h>

#include "ability_util.h"
#include "hilog_tag_wrapper.h"
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
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s cbBase == nullptr", __func__);
        return false;
    }

    if (cbBase->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s cbBase->ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = cbBase->ability->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s info == nullptr", __func__);
        return false;
    }
    return CheckAbilityType((AbilityType)info->type, cbBase->abilityType);
}

bool CheckAbilityType(const AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == null", __func__);
        return false;
    }

    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == null", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s info == null", __func__);
        return false;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s exit.", __func__);
    return CheckAbilityType((AbilityType)info->type, asyncCallbackInfo->abilityType);
}

bool CheckAbilityType(const AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == nullptr", __func__);
        return false;
    }

    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == nullptr", __func__);
        return false;
    }

    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s info == nullptr", __func__);
        return false;
    }

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return CheckAbilityType((AbilityType)info->type, asyncCallbackInfo->abilityType);
}

napi_value GetContinueAbilityOptionsInfoCommon(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
    return result;
}

napi_value GetContinueAbilityOptionsReversible(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
    napi_valuetype valuetype = napi_undefined;
    napi_value result = nullptr;
    bool hasProperty = false;
    bool reversible = false;

    NAPI_CALL(env, napi_has_named_property(env, value, "reversible", &hasProperty));
    if (hasProperty) {
        napi_get_named_property(env, value, "reversible", &result);
        NAPI_CALL(env, napi_typeof(env, result, &valuetype));
        if (valuetype != napi_boolean) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument type. Bool expected.", __func__);
            return nullptr;
        }
        napi_get_value_bool(env, result, &reversible);
        info.reversible = reversible;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
    return result;
}

napi_value GetContinueAbilityOptionsDeviceID(
    const napi_env &env, const napi_value &value, ContinueAbilityOptionsInfo &info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
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
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument type. String expected.", __func__);
            return nullptr;
        }
        NAPI_CALL(env, napi_get_value_string_utf8(env, result, str, STR_MAX_SIZE - 1, &strLen));
        info.deviceId = str;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
    return result;
}

napi_value WrapAppInfo(napi_env env, const ApplicationInfo &appInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
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
void GetFilesDirExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is nullptr", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s error ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s GetAbilityContext is null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetFilesDir();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end. filesDir=%{public}s", __func__,
             asyncCallbackInfo->native_data.str_value.c_str());
}

void IsUpdatingConfigurationsExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s begin", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is null.", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == nullptr.", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s fail ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->IsUpdatingConfigurations();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
}

/**
 * @brief PrintDrawnCompleted asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void PrintDrawnCompletedExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s enter", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is nullptr", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s fail ability type.", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->PrintDrawnCompleted();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
}

napi_value NAPI_GetFilesDirWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, params is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s begin, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s begin. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetFilesDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}
napi_value NAPI_GetFilesDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetFilesDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return ret;
}

void GetOrCreateDistributedDirExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is nullptr.", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability: nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s wrong type of ability", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s GetAbilityContext is nullptr.", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetDistributedFilesDir();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end. filesDir=%{public}s", __func__,
             asyncCallbackInfo->native_data.str_value.c_str());
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s called, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s called, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirCallback";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetFilesDirPromise";
        asyncParamEx.execute = GetOrCreateDistributedDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetOrCreateDistributedDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetOrCreateDistributedDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return ret;
}

/**
 * @brief GetCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetCacheDirExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "GetCacheDirExecuteCallback called");
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability: null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s error type of ability", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = asyncCallbackInfo->ability->GetAbilityContext();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s GetAbilityContext is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }
    asyncCallbackInfo->native_data.str_value = abilityContext->GetCacheDir();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end. CacheDir=%{public}s", __func__,
             asyncCallbackInfo->native_data.str_value.c_str());
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s begin", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s called, arguments is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s called, the first argument is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetCacheDirCallback";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetCacheDirPromise";
        asyncParamEx.execute = GetCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return ret;
}

/**
 * @brief GetExternalCacheDir asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetExternalCacheDirExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "GetExternalCacheDirExecuteCallback called");
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. asyncCallbackInfo is nullptr", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == null.", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s wrong ability type.", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_STRING;
    asyncCallbackInfo->native_data.str_value = asyncCallbackInfo->ability->GetExternalCacheDir();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end. ExternalCacheDir=%{private}s", __func__,
             asyncCallbackInfo->native_data.str_value.c_str());
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s begin", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s start. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetExternalCacheDirCallback";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetExternalCacheDirPromise";
        asyncParamEx.execute = GetExternalCacheDirExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_GetExternalCacheDirCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_GetExternalCacheDirWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return ret;
}

napi_value NAPI_IsUpdatingConfigurationsWrap(
    napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s enter", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s enter, parameters is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s enter, the first parameter is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s enter. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsCallback";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_IsUpdatingConfigurationsPromise";
        asyncParamEx.execute = IsUpdatingConfigurationsExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_PrintDrawnCompletedWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s begin", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (argc > ARGS_ONE) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, arguments is invalid.", __func__);
        return nullptr;
    }

    if (argc == ARGS_ONE) {
        if (!CreateAsyncCallback(env, args[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, the first argument is invalid.", __func__);
            return nullptr;
        }
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedCallback";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompleteAsyncVoidCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_PrintDrawnCompletedPromise";
        asyncParamEx.execute = PrintDrawnCompletedExecuteCallback;
        asyncParamEx.complete = CompletePromiseVoidCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

napi_value NAPI_IsUpdatingConfigurationsCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_IsUpdatingConfigurationsWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
    return ret;
}

napi_value NAPI_PrintDrawnCompletedCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = NAPI_PrintDrawnCompletedWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AppTypeCB *appTypeCB = new (std::nothrow) AppTypeCB;
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appTypeCB == nullptr.", __func__);
        return nullptr;
    }
    appTypeCB->cbBase.cbInfo.env = env;
    appTypeCB->cbBase.asyncWork = nullptr;
    appTypeCB->cbBase.deferred = nullptr;
    appTypeCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return appTypeCB;
}

/**
 * @brief GetAppType asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, worker pool thread execute.");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo,appTypeCB == nullptr");
        return;
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appTypeCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo,ability == nullptr");
        appTypeCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appTypeCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo,wrong ability type");
        appTypeCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    appTypeCB->name = appTypeCB->cbBase.ability->GetAppType();
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypePromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "GetAppTypePromiseCompleteCB, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "GetAppTypePromiseCompleteCB, main event thread complete end.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, appTypeCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, appTypeCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appTypeCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong arguments count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppTypeAsync(env, args, 0, appTypeCB);
    } else {
        ret = GetAppTypePromise(env, appTypeCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AppTypeCB *appTypeCB = CreateAppTypeCBInfo(env);
    if (appTypeCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appTypeCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppTypeWrap(env, info, appTypeCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ret == nullptr.", __func__);
        if (appTypeCB != nullptr) {
            delete appTypeCB;
            appTypeCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return ret;
}

#ifdef SUPPORT_GRAPHICS
napi_value GetDisplayOrientationWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamGetDisplayOrientationWrap(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke UnwrapParamGetDisplayOrientationWrap fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapCallback";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
        asyncParamEx.resource = "NAPI_GetDisplayOrientationWrapPromise";
        asyncParamEx.execute = GetDisplayOrientationExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

void GetDisplayOrientationExecuteCallback(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo is nullptr", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability is nullptr", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s fail type of ability", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_INT32;
    asyncCallbackInfo->native_data.int32_value = asyncCallbackInfo->ability->GetDisplayOrientation();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
}

bool UnwrapParamGetDisplayOrientationWrap(napi_env env, size_t argc, napi_value *argv,
    AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, argc=%{public}zu", __func__, argc);
    const size_t argcMax = 1;
    if (argc > argcMax || argc < argcMax - 1) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM0], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, the first parameter is invalid.", __func__);
            return false;
        }
    }

    return true;
}

napi_value NAPI_GetDisplayOrientationCommon(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetDisplayOrientationWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ret == nullptr.", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AbilityInfoCB *abilityInfoCB = new (std::nothrow) AbilityInfoCB;
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, abilityInfoCB == nullptr.", __func__);
        return nullptr;
    }
    abilityInfoCB->cbBase.cbInfo.env = env;
    abilityInfoCB->cbBase.asyncWork = nullptr;
    abilityInfoCB->cbBase.deferred = nullptr;
    abilityInfoCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return abilityInfoCB;
}

napi_value WrapAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
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
void GetAbilityInfoExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, worker pool thread execute.");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, abilityInfoCB == nullptr");
        return;
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, ability == nullptr");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityInfoCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo,wrong ability type");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfoPtr = abilityInfoCB->cbBase.ability->GetAbilityInfo();
    if (abilityInfoPtr != nullptr) {
        abilityInfoCB->abilityInfo = *abilityInfoPtr;
    } else {
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, worker pool thread execute end.");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete end.");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoPromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetApplicationInfo, main event thread complete end.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, abilityInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, abilityInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, abilityInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong parameter count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityInfoAsync(env, args, 0, abilityInfoCB);
    } else {
        ret = GetAbilityInfoPromise(env, abilityInfoCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AbilityInfoCB *abilityInfoCB = CreateAbilityInfoCBInfo(env);
    if (abilityInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    abilityInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityInfoWrap(env, info, abilityInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        if (abilityInfoCB != nullptr) {
            delete abilityInfoCB;
            abilityInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    HapModuleInfoCB *hapModuleInfoCB = new (std::nothrow) HapModuleInfoCB;
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, hapModuleInfoCB == nullptr.", __func__);
        return nullptr;
    }
    hapModuleInfoCB->cbBase.cbInfo.env = env;
    hapModuleInfoCB->cbBase.asyncWork = nullptr;
    hapModuleInfoCB->cbBase.deferred = nullptr;
    hapModuleInfoCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return hapModuleInfoCB;
}

napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &hapModuleInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return result;
}

void GetHapModuleInfoExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, worker pool thread execute.");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, hapModuleInfoCB == nullptr");
        return;
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (hapModuleInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, ability == nullptr");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&hapModuleInfoCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo,wrong ability type");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<HapModuleInfo> hapModuleInfoPtr = hapModuleInfoCB->cbBase.ability->GetHapModuleInfo();
    if (hapModuleInfoPtr != nullptr) {
        hapModuleInfoCB->hapModuleInfo = *hapModuleInfoPtr;
    } else {
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, worker pool thread execute end.");
}

void GetHapModuleInfoAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, main event thread complete end.");
}

void GetHapModuleInfoPromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetHapModuleInfo, main event thread complete end.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, hapModuleInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, hapModuleInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, hapModuleInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong parameters count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetHapModuleInfoAsync(env, args, 0, hapModuleInfoCB);
    } else {
        ret = GetHapModuleInfoPromise(env, hapModuleInfoCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    HapModuleInfoCB *hapModuleInfoCB = CreateHapModuleInfoCBInfo(env);
    if (hapModuleInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    hapModuleInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetHapModuleInfoWrap(env, info, hapModuleInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        if (hapModuleInfoCB != nullptr) {
            delete hapModuleInfoCB;
            hapModuleInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AppVersionInfoCB *appVersionInfoCB = new (std::nothrow) AppVersionInfoCB;
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appVersionInfoCB == nullptr.", __func__);
        return nullptr;
    }
    appVersionInfoCB->cbBase.cbInfo.env = env;
    appVersionInfoCB->cbBase.asyncWork = nullptr;
    appVersionInfoCB->cbBase.deferred = nullptr;
    appVersionInfoCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return appVersionInfoCB;
}

void SaveAppVersionInfo(AppVersionInfo &appVersionInfo, const std::string appName, const std::string versionName,
    const int32_t versionCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    appVersionInfo.appName = appName;
    appVersionInfo.versionName = versionName;
    appVersionInfo.versionCode = versionCode;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
}

napi_value WrapAppVersionInfo(napi_env env, const AppVersionInfoCB &appVersionInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return result;
}

void GetAppVersionInfoExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, worker pool thread execute.");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, appVersionInfoCB == nullptr");
        return;
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appVersionInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, ability == nullptr");
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appVersionInfoCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo,wrong ability type");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, worker pool thread execute end.");
}

void GetAppVersionInfoAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, main event thread complete end.");
}

void GetAppVersionInfoPromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAppVersionInfo, main event thread complete end.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, appVersionInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, appVersionInfoCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value GetAppVersionInfoWrap(napi_env env, napi_callback_info info, AppVersionInfoCB *appVersionInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, appVersionInfoCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, fail argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppVersionInfoAsync(env, args, 0, appVersionInfoCB);
    } else {
        ret = GetAppVersionInfoPromise(env, appVersionInfoCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AppVersionInfoCB *appVersionInfoCB = CreateAppVersionInfoCBInfo(env);
    if (appVersionInfoCB == nullptr) {
        return WrapVoidToJS(env);
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    appVersionInfoCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAppVersionInfoWrap(env, info, appVersionInfoCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        if (appVersionInfoCB != nullptr) {
            delete appVersionInfoCB;
            appVersionInfoCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    if (env == nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s env == nullptr.", __func__);
        return nullptr;
    }

    napi_status ret;
    napi_value global = nullptr;
    const napi_extended_error_info *errorInfo = nullptr;
    ret = napi_get_global(env, &global);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s get_global=%{public}d err:%{public}s", __func__, ret,
                 errorInfo->error_message);
    }

    napi_value abilityObj = nullptr;
    ret = napi_get_named_property(env, global, "ability", &abilityObj);
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s get_named_property=%{public}d err:%{public}s", __func__, ret,
                 errorInfo->error_message);
    }

    Ability *ability = nullptr;
    ret = napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability));
    if (ret != napi_ok) {
        napi_get_last_error_info(env, &errorInfo);
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s get_value_external=%{public}d err:%{public}s", __func__, ret,
                 errorInfo->error_message);
    }

    AsyncCallbackInfo *asyncCallbackInfo = new (std::nothrow) AsyncCallbackInfo;
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == nullptr", __func__);
        return nullptr;
    }
    asyncCallbackInfo->cbInfo.env = env;
    asyncCallbackInfo->asyncWork = nullptr;
    asyncCallbackInfo->deferred = nullptr;
    asyncCallbackInfo->ability = ability;
    asyncCallbackInfo->native_result = false;
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = AbilityType::UNKNOWN;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return asyncCallbackInfo;
}

void GetContextAsyncExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "GetContextAsync, worker pool thread execute.");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetContextAsync, asyncCallbackInfo == nullptr");
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetContextAsync, ability == nullptr");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetContextAsync,wrong ability type");
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "GetContextAsync, worker pool thread execute end.");
}

napi_value GetContextAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, parameter == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        TAG_LOGD(AAFwkTag::JSNAPI, "napi_create_reference");
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetContextAsyncExecuteCB,
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "GetContextAsync, main event thread complete.");
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
                TAG_LOGD(AAFwkTag::JSNAPI, "Delete GetContextAsync callback reference.");
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            TAG_LOGI(AAFwkTag::JSNAPI, "GetContextAsync, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetContextPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr", __func__);
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
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "GetContextPromise, main event thread complete.");
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
            TAG_LOGI(AAFwkTag::JSNAPI, "GetContextPromise, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
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
napi_value GetContextWrap(napi_env env, napi_callback_info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s,wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return nullptr;
    }

    napi_value result = nullptr;
    napi_new_instance(env, GetGlobalClassContext(env), 0, nullptr, &result);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetContextWrap(env, info, asyncCallbackInfo);

    delete asyncCallbackInfo;
    asyncCallbackInfo = nullptr;

    if (ret == nullptr) {
        ret = WrapVoidToJS(env);
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr.", __func__);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    }
    return ret;
}

void GetWantExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, asyncCallbackInfo == nullptr", __func__);
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ability == nullptr", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, wrong ability type", __func__);
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();
    if (ptrWant != nullptr) {
        asyncCallbackInfo->param.want = *ptrWant;
    } else {
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
}

napi_value GetWantAsync(napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, parameter == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args[argCallback], &valuetype);
    if (valuetype == napi_function) {
        TAG_LOGD(AAFwkTag::JSNAPI, "napi_create_reference.");
        napi_create_reference(env, args[argCallback], 1, &asyncCallbackInfo->cbInfo.callback);
    }
    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        GetWantExecuteCB,
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "GetWantAsync, main event thread complete.");
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
                TAG_LOGD(AAFwkTag::JSNAPI, "Delete GetWantAsync callback reference.");
                napi_delete_reference(env, asyncCallbackInfo->cbInfo.callback);
            }
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
            TAG_LOGI(AAFwkTag::JSNAPI, "GetWantAsync, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    napi_value result = nullptr;
    napi_get_null(env, &result);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetWantPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, parameter == nullptr.", __func__);
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
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "GetWantPromise, main event thread complete.");
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
            TAG_LOGI(AAFwkTag::JSNAPI, "GetWantPromise, main event thread complete end.");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, asyncCallbackInfo == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, error argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetWantAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = GetWantPromise(env, asyncCallbackInfo);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, asyncCallbackInfo == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = GetWantWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ret == nullptr", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));

    napi_value abilityObj = nullptr;
    NAPI_CALL(env, napi_get_named_property(env, global, "ability", &abilityObj));

    Ability *ability = nullptr;
    NAPI_CALL(env, napi_get_value_external(env, abilityObj, reinterpret_cast<void **>(&ability)));

    AbilityNameCB *abilityNameCB = new (std::nothrow) AbilityNameCB;
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, abilityNameCB == nullptr.", __func__);
        return nullptr;
    }
    abilityNameCB->cbBase.cbInfo.env = env;
    abilityNameCB->cbBase.asyncWork = nullptr;
    abilityNameCB->cbBase.deferred = nullptr;
    abilityNameCB->cbBase.ability = ability;

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return abilityNameCB;
}

napi_value WrapAbilityName(napi_env env, const AbilityNameCB *abilityNameCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Invalid param(abilityNameCB == nullptr)", __func__);
        return nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, abilityNameCB->name.c_str(), NAPI_AUTO_LENGTH, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return result;
}

/**
 * @brief GetAbilityName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, abilityNameCB == nullptr", __func__);
        return;
    }
    abilityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityNameCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ability == nullptr", __func__);
        abilityNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityNameCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, wrong ability type", __func__);
        abilityNameCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    abilityNameCB->name = abilityNameCB->cbBase.ability->GetAbilityName();
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, called.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNamePromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAbilityName, main event thread complete.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "NAPI_GetAbilityName, main event thread complete end.");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, abilityNameCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, abilityNameCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, abilityNameCB == nullptr.", __func__);
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, argument count wrong.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityNameAsync(env, args, 0, abilityNameCB);
    } else {
        ret = GetAbilityNamePromise(env, abilityNameCB);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AbilityNameCB *ablityNameCB = CreateAbilityNameCBInfo(env);
    if (ablityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ablityNameCB == nullptr", __func__);
        return WrapVoidToJS(env);
    }

    ablityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    ablityNameCB->cbBase.abilityType = abilityType;
    napi_value ret = GetAbilityNameWrap(env, info, ablityNameCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullptr", __func__);
        if (ablityNameCB != nullptr) {
            delete ablityNameCB;
            ablityNameCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
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
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Property value=%{private}d.", __func__, natValue32);
        isReadValue32 = true;
    }

    if (napi_get_value_double(env, param, &natValueDouble) == napi_ok) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Property value=%{private}lf.", __func__, natValueDouble);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Property size: %{public}d.", __func__, jsProCount);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);

        std::string strProName = UnwrapStringFromJS(env, jsProName);
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Property value=%{private}s.", __func__,
                         natValue.c_str());
                setting.AddProperty(strProName, natValue);
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Property value=%{public}s.", __func__,
                         natValue ? "true" : "false");
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
bool UnwrapParamForWant(napi_env env, napi_value args, AbilityType, CallAbilityParam &param)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    bool ret = false;
    napi_valuetype valueType = napi_undefined;
    param.setting = nullptr;
    NAPI_CALL_BASE(env, napi_typeof(env, args, &valueType), false);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument type.", __func__);
        return false;
    }

    napi_value jsWant = GetPropertyValueByPropertyName(env, args, "want", napi_object);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, jsWant == nullptr", __func__);
        return false;
    }

    ret = UnwrapWant(env, jsWant, param.want);

    napi_value jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSettings", napi_object);
    if (jsSettingObj == nullptr) {
        jsSettingObj = GetPropertyValueByPropertyName(env, args, "abilityStartSetting", napi_object);
    }
    if (jsSettingObj != nullptr) {
        param.setting = AbilityStartSetting::GetEmptySetting();
        if (!UnwrapAbilityStartSetting(env, jsSettingObj, *(param.setting))) {
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, unwrap abilityStartSetting falied.", __func__);
        }
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s abilityStartSetting", __func__);
    }

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

bool UnwrapParamStopAbilityWrap(napi_env env, size_t argc, napi_value *argv, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called, argc=%{public}zu", __func__, argc);
    const size_t argcMax = 2;
    if (argc > argcMax || argc < argcMax - 1) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Params is invalid.", __func__);
        return false;
    }

    if (argc == argcMax) {
        if (!CreateAsyncCallback(env, argv[PARAM1], asyncCallbackInfo)) {
            TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, the second parameter is invalid.", __func__);
            return false;
        }
    }

    return UnwrapWant(env, argv[PARAM0], asyncCallbackInfo->param.want);
}

void StopAbilityExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo is null", __func__);
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability is null", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s wrong ability type", __func__);
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->StopAbility(asyncCallbackInfo->param.want);
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
}

napi_value StopAbilityWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamStopAbilityWrap(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. Invoke UnwrapParamStopAbility fail", __func__);
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. asyncCallback.", __func__);
        asyncParamEx.resource = "NAPI_StopAbilityWrapCallback";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called. promise.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncJSCallbackInfo *asyncCallbackInfo = CreateAsyncJSCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. Invoke CreateAsyncJSCallbackInfo failed.", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->abilityType = abilityType;
    napi_value ret = StopAbilityWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s. ret == nullptr", __func__);
        FreeAsyncJSCallbackInfo(&asyncCallbackInfo);
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

void ClearCallbackWork(uv_work_t* req, int)
{
    std::unique_ptr<uv_work_t> work(req);
    if (!req) {
        TAG_LOGE(AAFwkTag::JSNAPI, "work null");
        return;
    }
    std::unique_ptr<ConnectionCallback> callback(reinterpret_cast<ConnectionCallback*>(req->data));
    if (!callback) {
        TAG_LOGE(AAFwkTag::JSNAPI, "data null");
        return;
    }
    callback->Reset();
}

void ConnectionCallback::Reset()
{
    auto engine = reinterpret_cast<NativeEngine*>(env);
    if (engine == nullptr) {
        removeKey = nullptr;
        return;
    }
    if (pthread_self() == engine->GetTid()) {
        TAG_LOGD(AAFwkTag::JSNAPI, "in-js-thread");
        if (connectCallbackRef) {
            napi_delete_reference(env, connectCallbackRef);
            connectCallbackRef = nullptr;
        }
        if (disconnectCallbackRef) {
            napi_delete_reference(env, disconnectCallbackRef);
            disconnectCallbackRef = nullptr;
        }
        if (failedCallbackRef) {
            napi_delete_reference(env, failedCallbackRef);
            failedCallbackRef = nullptr;
        }
        env = nullptr;
        removeKey = nullptr;
        return;
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "not in-js-thread");
    auto loop = engine->GetUVLoop();
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, loop == nullptr.", __func__);
        env = nullptr;
        removeKey = nullptr;
        return;
    }
    uv_work_t *work = new(std::nothrow) uv_work_t;
    ConnectionCallback *data = new(std::nothrow) ConnectionCallback(std::move(*this));
    work->data = data;
    auto ret = uv_queue_work(loop, work, [](uv_work_t*) {}, ClearCallbackWork);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::JSNAPI, "uv_queue_work failed: %{public}d", ret);
        data->env = nullptr;
        data->removeKey = nullptr;
        delete data;
        delete work;
    }
}

void NAPIAbilityConnection::AddConnectionCallback(std::shared_ptr<ConnectionCallback> callback)
{
    std::lock_guard<std::mutex> guard(lock_);
    callbacks_.emplace_back(callback);
}

int NAPIAbilityConnection::GetConnectionState() const
{
    std::lock_guard<std::mutex> guard(lock_);
    return connectionState_;
}

void NAPIAbilityConnection::SetConnectionState(int connectionState)
{
    std::lock_guard<std::mutex> guard(lock_);
    connectionState_ = connectionState;
}

size_t NAPIAbilityConnection::GetCallbackSize()
{
    std::lock_guard<std::mutex> guard(lock_);
    return callbacks_.size();
}

size_t NAPIAbilityConnection::RemoveAllCallbacks(ConnectRemoveKeyType key)
{
    size_t result = 0;
    std::lock_guard<std::mutex> guard(lock_);
    for (auto it = callbacks_.begin(); it != callbacks_.end();) {
        auto callback = *it;
        if (callback && callback->removeKey == key) {
            it = callbacks_.erase(it);
            result++;
        } else {
            ++it;
        }
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "RemoveAllCallbacks removed size:%{public}zu, left size:%{public}zu", result,
             callbacks_.size());
    return result;
}

void UvWorkOnAbilityConnectDone(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, uv_queue_work");
    std::unique_ptr<uv_work_t> managedWork(work);
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, work is null");
        return;
    }
    // JS Thread
    std::unique_ptr<ConnectAbilityCB> connectAbilityCB(static_cast<ConnectAbilityCB *>(work->data));
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, connectAbilityCB is null");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cbInfo.env, &scope);
    if (scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
        return;
    }

    napi_value globalValue;
    napi_get_global(cbInfo.env, &globalValue);
    napi_value func;
    napi_get_named_property(cbInfo.env, globalValue, "requireNapi", &func);

    napi_value rpcInfo;
    napi_create_string_utf8(cbInfo.env, "rpc", NAPI_AUTO_LENGTH, &rpcInfo);
    napi_value funcArgv[1] = { rpcInfo };
    napi_value returnValue;
    napi_call_function(cbInfo.env, globalValue, func, 1, funcArgv, &returnValue);

    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] =
        WrapElementName(cbInfo.env, connectAbilityCB->abilityConnectionCB.elementName);
    napi_value jsRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(
        cbInfo.env, connectAbilityCB->abilityConnectionCB.connection);
    result[PARAM1] = jsRemoteObject;

    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(cbInfo.env, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(cbInfo.env, cbInfo.callback, &callback);

    napi_call_function(
        cbInfo.env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);
    if (cbInfo.callback != nullptr) {
        napi_delete_reference(cbInfo.env, cbInfo.callback);
    }
    napi_close_handle_scope(cbInfo.env, scope);
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityConnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::HandleOnAbilityConnectDone(ConnectionCallback &callback, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, loop == null.", __func__);
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, work == null.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, connectAbilityCB == null.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }
    connectAbilityCB->cbBase.cbInfo.env = callback.env;
    connectAbilityCB->cbBase.cbInfo.callback = callback.connectCallbackRef;
    callback.connectCallbackRef = nullptr;
    connectAbilityCB->abilityConnectionCB.elementName = element_;
    connectAbilityCB->abilityConnectionCB.resultCode = resultCode;
    connectAbilityCB->abilityConnectionCB.connection = serviceRemoteObject_;
    work->data = static_cast<void *>(connectAbilityCB);

    int rev = uv_queue_work_with_qos(
        loop, work, [](uv_work_t *work) {}, UvWorkOnAbilityConnectDone, uv_qos_user_initiated);
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
}

void NAPIAbilityConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
             __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, remoteObject == nullptr.", __func__);
        return;
    }
    std::lock_guard<std::mutex> guard(lock_);
    element_ = element;
    serviceRemoteObject_ = remoteObject;
    for (const auto &callback : callbacks_) {
        HandleOnAbilityConnectDone(*callback, resultCode);
    }
    connectionState_ = CONNECTION_STATE_CONNECTED;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
}

void UvWorkOnAbilityDisconnectDone(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, uv_queue_work");
    std::unique_ptr<uv_work_t> managedWork(work);
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, work is null");
        return;
    }
    // JS Thread
    std::unique_ptr<ConnectAbilityCB> connectAbilityCB(static_cast<ConnectAbilityCB *>(work->data));
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, connectAbilityCB is null");
        return;
    }
    CallbackInfo &cbInfo = connectAbilityCB->cbBase.cbInfo;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cbInfo.env, &scope);
    if (scope == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_open_handle_scope failed");
        return;
    }
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
    napi_close_handle_scope(cbInfo.env, scope);

    // release connect
    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone connects_.size:%{public}zu", connects_.size());
    std::string deviceId = connectAbilityCB->abilityConnectionCB.elementName.GetDeviceID();
    std::string bundleName = connectAbilityCB->abilityConnectionCB.elementName.GetBundleName();
    std::string abilityName = connectAbilityCB->abilityConnectionCB.elementName.GetAbilityName();
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [deviceId, bundleName, abilityName](const std::map<ConnectionKey,
            sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetDeviceId()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (item != connects_.end()) {
        // match deviceid & bundlename && abilityname
        connects_.erase(item);
        TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone erase connects_.size:%{public}zu", connects_.size());
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "UvWorkOnAbilityDisconnectDone, uv_queue_work end");
}

void NAPIAbilityConnection::HandleOnAbilityDisconnectDone(ConnectionCallback &callback, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(callback.env, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, loop == nullptr.", __func__);
        return;
    }

    uv_work_t *work = new(std::nothrow) uv_work_t;
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, work == nullptr.", __func__);
        return;
    }

    ConnectAbilityCB *connectAbilityCB = new (std::nothrow) ConnectAbilityCB;
    if (connectAbilityCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, connectAbilityCB == nullptr.", __func__);
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
        return;
    }

    connectAbilityCB->cbBase.cbInfo.env = callback.env;
    connectAbilityCB->cbBase.cbInfo.callback = callback.disconnectCallbackRef;
    callback.disconnectCallbackRef = nullptr;
    connectAbilityCB->abilityConnectionCB.elementName = element_;
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
}

void NAPIAbilityConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s bundleName:%{public}s abilityName:%{public}s, resultCode:%{public}d",
             __func__, element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::lock_guard<std::mutex> guard(lock_);
    element_ = element;
    for (const auto &callback : callbacks_) {
        HandleOnAbilityDisconnectDone(*callback, resultCode);
    }
    connectionState_ = CONNECTION_STATE_DISCONNECTED;
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "AcquireDataAbilityHelper called");
    DataAbilityHelperCB *dataAbilityHelperCB = new DataAbilityHelperCB;
    dataAbilityHelperCB->cbBase.cbInfo.env = env;
    dataAbilityHelperCB->cbBase.ability = nullptr; // temporary value assignment
    dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    dataAbilityHelperCB->cbBase.abilityType = abilityType;
    napi_value ret = AcquireDataAbilityHelperWrap(env, info, dataAbilityHelperCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, ret == nullptr", __func__);
        if (dataAbilityHelperCB != nullptr) {
            delete dataAbilityHelperCB;
            dataAbilityHelperCB = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "AcquireDataAbilityHelper end");
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
    if (dataAbilityHelperCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s,dataAbilityHelperCB == nullptr", __func__);
        return nullptr;
    }

    size_t requireArgc = ARGS_TWO;
    size_t argc = ARGS_TWO;
    napi_value args[ARGS_TWO] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    if (argc > requireArgc) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    size_t uriIndex = PARAM0;
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, args[0], stageMode);
    if (status == napi_ok) {
        uriIndex = PARAM1;
        TAG_LOGI(AAFwkTag::JSNAPI, "argv[0] is a context, Stage Model: %{public}d", stageMode);
    }

    if (!stageMode) {
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Failed to get native context instance");
            return nullptr;
        }
        dataAbilityHelperCB->cbBase.ability = ability;

        if (!CheckAbilityType(&dataAbilityHelperCB->cbBase)) {
            dataAbilityHelperCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
            TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability type invalid.", __func__);
            return nullptr;
        }
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[uriIndex], &valuetype));
    if (valuetype != napi_string) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument type.", __func__);
        return nullptr;
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_new_instance(env, GetGlobalDataAbilityHelper(env), uriIndex + 1, &args[PARAM0], &result));

    if (!IsTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, IsTypeForNapiValue isn`t object", __func__);
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_null)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, IsTypeForNapiValue is null", __func__);
        return nullptr;
    }

    if (IsTypeForNapiValue(env, result, napi_undefined)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, IsTypeForNapiValue is undefined", __func__);
        return nullptr;
    }

    if (!GetDataAbilityHelperStatus()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, GetDataAbilityHelperStatus is false", __func__);
        return nullptr;
    }

    delete dataAbilityHelperCB;
    dataAbilityHelperCB = nullptr;
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == nullptr", __func__);
        return;
    }
    if (asyncCallbackInfo->errCode == NAPI_ERR_PARAM_INVALID) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse input param failed");
        return;
    }
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ability == nullptr", __func__);
        return;
    }
    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "abilityinfo is null");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    AbilityRuntime::WantAgent::WantAgent wantAgentObj;
    if (!asyncCallbackInfo->wantAgent) {
        TAG_LOGW(AAFwkTag::JSNAPI, "input param without wantAgent");
        wantAgentObj = AbilityRuntime::WantAgent::WantAgent();
    } else {
        wantAgentObj = *asyncCallbackInfo->wantAgent;
    }
    asyncCallbackInfo->errCode = asyncCallbackInfo->ability->StartBackgroundRunning(wantAgentObj);

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
}

void BackgroundRunningCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
}

void BackgroundRunningPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    delete asyncCallbackInfo;
}

napi_value StartBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_utility));

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s asyncCallback end.", __func__);
    return WrapVoidToJS(env);
}

napi_value StartBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr.", __func__);
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
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_utility));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, end.", __func__);
    return promise;
}

napi_value StartBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    size_t paramNums = 3;
    const size_t minParamNums = 2;
    const size_t maxParamNums = 3;
    napi_value args[maxParamNums] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNums, args, NULL, NULL));

    if (paramNums < minParamNums || paramNums > maxParamNums) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument count.", __func__);
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

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_StartBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = StartBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == null", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s finish.", __func__);
    return ret;
}

void CancelBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo->ability != nullptr) {
        asyncCallbackInfo->errCode = asyncCallbackInfo->ability->StopBackgroundRunning();
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "NAPI_PACancelBackgroundRunning, ability == nullptr");
    }
}

napi_value CancelBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == nullptr", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value CancelBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise.", __func__);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, param == null.", __func__);
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
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s, promise end", __func__);
    return promise;
}

napi_value CancelBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, NULL, NULL));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = CancelBackgroundRunningAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = CancelBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

napi_value NAPI_CancelBackgroundRunningCommon(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    AsyncCallbackInfo *asyncCallbackInfo = CreateAsyncCallbackInfo(env);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s asyncCallbackInfo == nullpter", __func__);
        return WrapVoidToJS(env);
    }

    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    napi_value ret = CancelBackgroundRunningWrap(env, info, asyncCallbackInfo);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s ret == nullpter", __func__);
        if (asyncCallbackInfo != nullptr) {
            delete asyncCallbackInfo;
            asyncCallbackInfo = nullptr;
        }
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return ret;
}

JsNapiCommon::JsNapiCommon() : ability_(nullptr)
{}

JsNapiCommon::~JsNapiCommon()
{
    RemoveAllCallbacksLocked();
}

napi_value JsNapiCommon::JsConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s is called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc != ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    Want want;
    if (!UnwrapWant(env, argv[PARAM0], want)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "called. Invoke UnwrapWant fail");
        return CreateJsUndefined(env);
    }

    auto connectionCallback = std::make_shared<ConnectionCallback>(env, argv[PARAM1], this);
    bool result = false;
    int32_t errorVal = static_cast<int32_t>(NAPI_ERR_NO_ERROR);
    int64_t id = 0;
    sptr<NAPIAbilityConnection> abilityConnection = nullptr;
    if (CheckAbilityType(abilityType)) {
        abilityConnection = FindConnectionLocked(want, id);
        if (abilityConnection) {
            TAG_LOGI(AAFwkTag::JSNAPI, "find abilityConnection exist, current callbackSize: %{public}zu.",
                     abilityConnection->GetCallbackSize());
            // Add callback to connection
            abilityConnection->AddConnectionCallback(connectionCallback);
            // Judge connection-state
            auto connectionState = abilityConnection->GetConnectionState();
            if (connectionState == CONNECTION_STATE_CONNECTED) {
                TAG_LOGI(AAFwkTag::JSNAPI, "Ability is connected, callback to client.");
                abilityConnection->HandleOnAbilityConnectDone(*connectionCallback, ERR_OK);
                return CreateJsValue(env, id);
            } else if (connectionState == CONNECTION_STATE_CONNECTING) {
                TAG_LOGI(AAFwkTag::JSNAPI, "Ability is connecting, just wait callback.");
                return CreateJsValue(env, id);
            } else {
                TAG_LOGE(AAFwkTag::JSNAPI, "AbilityConnection has disconnected, erase it.");
                RemoveConnectionLocked(want);
                return CreateJsUndefined(env);
            }
        } else {
            result = CreateConnectionAndConnectAbilityLocked(connectionCallback, want, id);
        }
    } else {
        errorVal = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
    }

    if (errorVal != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || result == false) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CommonJsConnectAbility failed.");
        // return error code in onFailed async callback
        napi_value callback = nullptr;
        napi_value undefinedVal = nullptr;
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
        NAPI_CALL_BASE(env, napi_create_int32(env, errorCode, &resultVal), CreateJsUndefined(env));
        NAPI_CALL_BASE(env, napi_get_reference_value(env, connectionCallback->failedCallbackRef, &callback),
            CreateJsUndefined(env));
        NAPI_CALL_BASE(env, napi_call_function(env, undefinedVal, callback, ARGS_ONE, &resultVal, &callResult),
            CreateJsUndefined(env));
        connectionCallback->Reset();
        RemoveConnectionLocked(want);
    }
    // free failedcallback here, avoid possible multi-threading problems when disconnect success
    napi_delete_reference(env, connectionCallback->failedCallbackRef);
    connectionCallback->failedCallbackRef = nullptr;
    return CreateJsValue(env, id);
}

napi_value JsNapiCommon::JsDisConnectAbility(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s is called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ZERO || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    int64_t id = 0;
    sptr<NAPIAbilityConnection> abilityConnection = nullptr;
    if (!ConvertFromJsValue(env, argv[PARAM0], id)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params int error");
        return CreateJsUndefined(env);
    }
    auto item = std::find_if(connects_.begin(), connects_.end(),
        [&id](const std::map<ConnectionKey, sptr<NAPIAbilityConnection>>::value_type &obj) {
            return id == obj.first.id;
        });
    if (item != connects_.end()) {
        abilityConnection = item->second;
        TAG_LOGD(AAFwkTag::JSNAPI, "find conn ability exist");
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "there is no ability to disconnect.");
        return CreateJsUndefined(env);
    }
    auto execute = [obj = this, value = errorVal, abilityType, abilityConnection] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr.");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        *value = obj->ability_->DisconnectAbility(abilityConnection);
    };
    auto complete = [obj = this, value = errorVal]
        (napi_env env, NapiAsyncTask &task, const int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR)) {
            task.Reject(env, CreateJsError(env, *value, "DisconnectAbility failed."));
            return;
        }
        task.Resolve(env, CreateJsValue(env, *value));
    };
    napi_value lastParam = (argc == ARGS_ONE) ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsDisConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

bool JsNapiCommon::CreateConnectionAndConnectAbilityLocked(
    std::shared_ptr<ConnectionCallback> callback, const Want &want, int64_t &id)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "Create new connection");
    // Create connection
    sptr<NAPIAbilityConnection> connection(new (std::nothrow) NAPIAbilityConnection());
    ConnectionKey key;
    id = serialNumber_;
    key.id = id;
    key.want = want;
    connects_.emplace(key, connection);
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    // Set callback
    connection->AddConnectionCallback(callback);

    // connectAbility
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ConnectAbility, the ability is nullptr");
        return false;
    }
    connection->SetConnectionState(CONNECTION_STATE_CONNECTING);
    return ability_->ConnectAbility(want, connection);
}

sptr<NAPIAbilityConnection> JsNapiCommon::FindConnectionLocked(const Want &want, int64_t &id)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s uri:%{public}s", __func__, want.GetElement().GetURI().c_str());
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string bundleName = want.GetBundle();
    std::string abilityName = want.GetElement().GetAbilityName();
    auto iter = std::find_if(connects_.begin(),
        connects_.end(), [&deviceId, &bundleName, &abilityName](const std::map<ConnectionKey,
        sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetElement().GetDeviceID()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    if (iter != connects_.end()) {
        TAG_LOGD(AAFwkTag::JSNAPI, "find connection exist");
        auto connection = iter->second;
        if (connection == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "connection is nullptr");
            connects_.erase(iter);
            return nullptr;
        }
        id = iter->first.id;
        return connection;
    }
    return nullptr;
}

void JsNapiCommon::RemoveAllCallbacksLocked()
{
    TAG_LOGD(AAFwkTag::JSNAPI, "RemoveAllCallbacksLocked begin");
    std::lock_guard<std::mutex> lock(g_connectionsLock_);
    for (auto it = connects_.begin(); it != connects_.end();) {
        auto connection = it->second;
        if (!connection) {
            TAG_LOGE(AAFwkTag::JSNAPI, "connection is nullptr");
            it = connects_.erase(it);
            continue;
        }
        connection->RemoveAllCallbacks(this);
        if (connection->GetCallbackSize() == 0) {
            it = connects_.erase(it);
        } else {
            ++it;
        }
    }
}

void JsNapiCommon::RemoveConnectionLocked(const Want &want)
{
    std::string deviceId = want.GetElement().GetDeviceID();
    std::string bundleName = want.GetBundle();
    std::string abilityName = want.GetElement().GetAbilityName();
    auto iter = std::find_if(connects_.begin(),
        connects_.end(), [&deviceId, &bundleName, &abilityName](const std::map<ConnectionKey,
        sptr<NAPIAbilityConnection>>::value_type &obj) {
            return (deviceId == obj.first.want.GetElement().GetDeviceID()) &&
                   (bundleName == obj.first.want.GetBundle()) &&
                   (abilityName == obj.first.want.GetElement().GetAbilityName());
        });
    connects_.erase(iter);
}

napi_value JsNapiCommon::JsGetContext(napi_env env, const napi_callback_info info, const AbilityType abilityType)
{
    if (!CheckAbilityType(abilityType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ability type error");
        return CreateJsUndefined(env);
    }

    return CreateNapiJSContext(env);
}

napi_value JsNapiCommon::JsGetFilesDir(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetFilesDir called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "JsGetFilesDir input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsFilesDir> filesDir = std::make_shared<JsFilesDir>();
    auto execute = [obj = this, dir = filesDir, abilityType, value = errorVal] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsGetFilesDir task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsGetFilesDir task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetFilesDir();
    };
    auto complete = [obj = this, dir = filesDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetFilesDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsNapiCommon::JsIsUpdatingConfigurations(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsIsUpdatingConfigurations called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "JsIsUpdatingConfigurations input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsConfigurations> config = std::make_shared<JsConfigurations>();
    auto execute = [obj = this, data = config, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsIsUpdatingConfigurations task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (data == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsIsUpdatingConfigurations task execute error, param is nullptr");
            return;
        }
        data->status = obj->ability_->IsUpdatingConfigurations();
    };
    auto complete = [obj = this, info = config, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, info->status));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsIsUpdatingConfigurations",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsPrintDrawnCompleted(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsPrintDrawnCompleted called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "JsPrintDrawnCompleted input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsDrawnCompleted> drawComplete = std::make_shared<JsDrawnCompleted>();
    auto execute = [obj = this, data = drawComplete, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsPrintDrawnCompleted task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        if (data == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsPrintDrawnCompleted task execute error, data is nullptr");
            return;
        }
        data->status = obj->ability_->PrintDrawnCompleted();
    };
    auto complete = [obj = this, draw = drawComplete, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || draw == nullptr) {
            auto ecode = draw == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsNull(env));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsPrintDrawnCompleted",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCacheDir(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetCacheDir called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "JsGetCacheDir input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCacheDir> cacheDir = std::make_shared<JsCacheDir>();
    auto execute = [obj = this, dir = cacheDir, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsGetCacheDir task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            TAG_LOGE(AAFwkTag::JSNAPI, "JsGetCacheDir task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetCacheDir();
    };
    auto complete = [obj = this, dir = cacheDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "JsCacheDir is null or errorVal is error");
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCacheDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCtxAppType(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetCtxAppType called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsCtxAppType> type = std::make_shared<JsCtxAppType>();
    auto execute = [obj = this, apptype = type, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
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
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || apptype == nullptr) {
            auto ecode = apptype == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, apptype->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCtxAppType",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCtxHapModuleInfo(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetCtxHapModuleInfo called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsHapModuleInfo> infoData = std::make_shared<JsHapModuleInfo>();
    auto execute = [obj = this, hapMod = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
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
            TAG_LOGE(AAFwkTag::JSNAPI, "GetHapModuleInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "JsHapModuleInfo is null or errorVal is 0.");
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateHapModuleInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCtxHapModuleInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetAppVersionInfo(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetAppVersionInfo called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input arguments count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsApplicationInfo> infoData = std::make_shared<JsApplicationInfo>();
    auto execute = [obj = this, appInfo = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is null");
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
            TAG_LOGE(AAFwkTag::JSNAPI, "GetApplicationInfo return null");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGD(AAFwkTag::JSNAPI, "JsHapModuleInfo is null or errorVal is 0");
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateAppVersionInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetAppVersionInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetCtxAbilityInfo(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetCtxAbilityInfo called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsAbilityInfoInfo> infoData = std::make_shared<JsAbilityInfoInfo>();
    auto execute = [obj = this, abilityInfo = infoData, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
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
            TAG_LOGE(AAFwkTag::JSNAPI, "GetAbilityInfo return nullptr");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
        }
    };
    auto complete = [obj = this, info = infoData, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || info == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "errorVal is 0 or JsHapModuleInfo is null.");
            auto ecode = info == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, obj->CreateAbilityInfo(env, info));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetCtxAbilityInfo",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsNapiCommon::JsGetOrCreateDistributedDir(
    napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetOrCreateDistributedDir called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    std::shared_ptr<JsOrCreateDistributedDir> orCreateDistributedDir = std::make_shared<JsOrCreateDistributedDir>();
    auto execute = [obj = this, dir = orCreateDistributedDir, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        auto context = obj->ability_->GetAbilityContext();
        if (context == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the abilitycontext is nullptr");
            return;
        }
        dir->name = context->GetDistributedFilesDir();
    };
    auto complete = [obj = this, dir = orCreateDistributedDir, value = errorVal]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value != static_cast<int32_t>(NAPI_ERR_NO_ERROR) || dir == nullptr) {
            TAG_LOGD(AAFwkTag::JSNAPI, "errorVal is error or JsCacheDir is null");
            auto ecode = dir == nullptr ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, ecode, obj->ConvertErrorCode(ecode)));
            return;
        }
        task.Resolve(env, CreateJsValue(env, dir->name));
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsNapiCommon::JsGetOrCreateDistributedDir",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}

#ifdef SUPPORT_GRAPHICS
napi_value JsNapiCommon::JsGetDisplayOrientation(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "JsGetDisplayOrientation called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto execute = [obj = this, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }
        *value = obj->ability_->GetDisplayOrientation();
    };
    auto complete = [value = errorVal] (napi_env env, NapiAsyncTask &task, int32_t status) {
        TAG_LOGD(AAFwkTag::JSNAPI, "JsGetDisplayOrientation value = %{public}d", *value);
        if (*value == NAPI_ERR_ACE_ABILITY) {
            task.Reject(env, CreateJsError(env, NAPI_ERR_ACE_ABILITY, "ability is nullptr"));
        } else if (*value == NAPI_ERR_ABILITY_TYPE_INVALID) {
            task.Reject(env, CreateJsError(env, NAPI_ERR_ABILITY_TYPE_INVALID, "ability type is invalid."));
        } else if (*value == NAPI_ERR_NO_WINDOW) {
            task.Reject(env, CreateJsError(env, NAPI_ERR_NO_WINDOW, "window is nullptr"));
        } else {
            task.Resolve(env, CreateJsValue(env, *value));
        }
    };

    auto callback = argc == ARGS_ZERO ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetDisplayOrientation",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));

    return result;
}
#endif

napi_value JsNapiCommon::CreateProcessInfo(napi_env env, const std::shared_ptr<JsProcessInfo> &processInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateProcessInfo called");
    CHECK_POINTER_AND_RETURN_LOG(processInfo, CreateJsUndefined(env), "input params error");

    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "processName", CreateJsValue(env, processInfo->processName));
    napi_set_named_property(env, objContext, "pid", CreateJsValue(env, processInfo->pid));

    return objContext;
}

napi_value JsNapiCommon::CreateElementName(napi_env env, const std::shared_ptr<JsElementName> &elementName)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateElementName called");
    CHECK_POINTER_AND_RETURN_LOG(elementName, CreateJsUndefined(env), "input params error");

    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "deviceId", CreateJsValue(env, elementName->deviceId));
    napi_set_named_property(env, objContext, "bundleName", CreateJsValue(env, elementName->bundleName));
    napi_set_named_property(env, objContext, "abilityName", CreateJsValue(env, elementName->abilityName));
    napi_set_named_property(env, objContext, "uri", CreateJsValue(env, elementName->uri));
    napi_set_named_property(env, objContext, "shortName", CreateJsValue(env, elementName->shortName));

    return objContext;
}

napi_value JsNapiCommon::CreateHapModuleInfo(napi_env env, const std::shared_ptr<JsHapModuleInfo> &hapModInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateHapModuleInfo called");
    CHECK_POINTER_AND_RETURN_LOG(hapModInfo, CreateJsUndefined(env), "input params error");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "name", CreateJsValue(env, hapModInfo->hapModInfo.name));
    napi_set_named_property(env, objContext, "description", CreateJsValue(env, hapModInfo->hapModInfo.description));
    napi_set_named_property(env, objContext, "icon", CreateJsValue(env, hapModInfo->hapModInfo.iconPath));
    napi_set_named_property(env, objContext, "label", CreateJsValue(env, hapModInfo->hapModInfo.label));
    napi_set_named_property(env, objContext, "backgroundImg",
        CreateJsValue(env, hapModInfo->hapModInfo.backgroundImg));
    napi_set_named_property(env, objContext, "moduleName", CreateJsValue(env, hapModInfo->hapModInfo.moduleName));
    napi_set_named_property(env, objContext, "mainAbilityName",
        CreateJsValue(env, hapModInfo->hapModInfo.mainAbility));
    napi_set_named_property(env, objContext, "supportedModes",
        CreateJsValue(env, hapModInfo->hapModInfo.supportedModes));
    napi_set_named_property(env, objContext, "descriptionId",
        CreateJsValue(env, hapModInfo->hapModInfo.descriptionId));
    napi_set_named_property(env, objContext, "labelId", CreateJsValue(env, hapModInfo->hapModInfo.labelId));
    napi_set_named_property(env, objContext, "iconId", CreateJsValue(env, hapModInfo->hapModInfo.iconId));
    napi_set_named_property(env, objContext, "installationFree",
        CreateJsValue(env, hapModInfo->hapModInfo.installationFree));
    napi_set_named_property(env, objContext, "reqCapabilities",
        CreateNativeArray(env, hapModInfo->hapModInfo.reqCapabilities));
    napi_set_named_property(env, objContext, "deviceTypes",
        CreateNativeArray(env, hapModInfo->hapModInfo.deviceTypes));
    napi_set_named_property(env, objContext, "abilityInfo",
        CreateAbilityInfos(env, hapModInfo->hapModInfo.abilityInfos));

    return objContext;
}

napi_value JsNapiCommon::CreateModuleInfo(napi_env env, const ModuleInfo &modInfo)
{
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateObject error");
        return CreateJsUndefined(env);
    }
    if (!CheckTypeForNapiValue(env, objContext, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ConvertNativeValueTo object error");
        return CreateJsUndefined(env);
    }

    napi_set_named_property(env, objContext, "moduleName", CreateJsValue(env, modInfo.moduleName));
    napi_set_named_property(env, objContext, "moduleSourceDir", CreateJsValue(env, modInfo.moduleSourceDir));

    return objContext;
}

napi_value JsNapiCommon::CreateModuleInfos(napi_env env, const std::vector<ModuleInfo> &moduleInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, moduleInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateArray failed");
        return CreateJsUndefined(env);
    }
    for (uint32_t i = 0; i < moduleInfos.size(); i++) {
        napi_set_element(env, arrayValue, i, CreateModuleInfo(env, moduleInfos.at(i)));
    }

    return arrayValue;
}

napi_value JsNapiCommon::CreateAppInfo(napi_env env, const ApplicationInfo &appInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateAppInfo called");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateObject error");
        return CreateJsUndefined(env);
    }
    if (!CheckTypeForNapiValue(env, objContext, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateAppInfo, ConvertNativeValueTo object error");
        return CreateJsUndefined(env);
    }

    napi_set_named_property(env, objContext, "name", CreateJsValue(env, appInfo.name));
    napi_set_named_property(env, objContext, "description", CreateJsValue(env, appInfo.description));
    napi_set_named_property(env, objContext, "descriptionId", CreateJsValue(env, appInfo.descriptionId));
    napi_set_named_property(env, objContext, "systemApp", CreateJsValue(env, appInfo.isSystemApp));
    napi_set_named_property(env, objContext, "enabled", CreateJsValue(env, appInfo.enabled));
    napi_set_named_property(env, objContext, "label", CreateJsValue(env, appInfo.label));
    napi_set_named_property(env, objContext, "labelId", CreateJsValue(env, std::to_string(appInfo.labelId)));
    napi_set_named_property(env, objContext, "icon", CreateJsValue(env, appInfo.iconPath));
    napi_set_named_property(env, objContext, "iconId", CreateJsValue(env, std::to_string(appInfo.iconId)));
    napi_set_named_property(env, objContext, "process", CreateJsValue(env, appInfo.process));
    napi_set_named_property(env, objContext, "entryDir", CreateJsValue(env, appInfo.entryDir));
    napi_set_named_property(env, objContext, "supportedModes", CreateJsValue(env, appInfo.supportedModes));
    napi_set_named_property(env, objContext, "moduleSourceDirs", CreateNativeArray(env, appInfo.moduleSourceDirs));
    napi_set_named_property(env, objContext, "permissions", CreateNativeArray(env, appInfo.permissions));
    napi_set_named_property(env, objContext, "moduleInfos", CreateModuleInfos(env, appInfo.moduleInfos));

    return objContext;
}

napi_value JsNapiCommon::CreateAppInfo(napi_env env, const std::shared_ptr<JsApplicationInfo> &appInfo)
{
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input param error");
        return CreateJsUndefined(env);
    }

    return CreateAppInfo(env, appInfo->appInfo);
}

napi_value JsNapiCommon::CreateAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateAbilityInfo called");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    if (objContext == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateAbilityInfo, CreateObject failed");
        return CreateJsUndefined(env);
    }
    if (!CheckTypeForNapiValue(env, objContext, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateAbilityInfo, ConvertNativeValueTo object error");
        return CreateJsUndefined(env);
    }

    napi_set_named_property(env, objContext, "bundleName", CreateJsValue(env, abilityInfo.bundleName));
    napi_set_named_property(env, objContext, "name", CreateJsValue(env, abilityInfo.name));
    napi_set_named_property(env, objContext, "label", CreateJsValue(env, abilityInfo.label));
    napi_set_named_property(env, objContext, "description", CreateJsValue(env, abilityInfo.description));
    napi_set_named_property(env, objContext, "icon", CreateJsValue(env, abilityInfo.iconPath));
    napi_set_named_property(env, objContext, "moduleName", CreateJsValue(env, abilityInfo.moduleName));
    napi_set_named_property(env, objContext, "process", CreateJsValue(env, abilityInfo.process));
    napi_set_named_property(env, objContext, "uri", CreateJsValue(env, abilityInfo.uri));
    napi_set_named_property(env, objContext, "readPermission", CreateJsValue(env, abilityInfo.readPermission));
    napi_set_named_property(env, objContext, "writePermission", CreateJsValue(env, abilityInfo.writePermission));
    napi_set_named_property(env, objContext, "targetAbility", CreateJsValue(env, abilityInfo.targetAbility));
    napi_set_named_property(env, objContext, "type", CreateJsValue(env, static_cast<int32_t>(abilityInfo.type)));
    napi_set_named_property(env, objContext, "orientation",
        CreateJsValue(env, static_cast<int32_t>(abilityInfo.orientation)));
    napi_set_named_property(env, objContext, "launchMode",
        CreateJsValue(env, static_cast<int32_t>(abilityInfo.launchMode)));
    napi_set_named_property(env, objContext, "labelId", CreateJsValue(env, abilityInfo.labelId));
    napi_set_named_property(env, objContext, "descriptionId", CreateJsValue(env, abilityInfo.descriptionId));
    napi_set_named_property(env, objContext, "iconId", CreateJsValue(env, abilityInfo.iconId));
    napi_set_named_property(env, objContext, "formEntity", CreateJsValue(env, abilityInfo.formEntity));
    napi_set_named_property(env, objContext, "minFormHeight", CreateJsValue(env, abilityInfo.minFormHeight));
    napi_set_named_property(env, objContext, "defaultFormHeight", CreateJsValue(env, abilityInfo.defaultFormHeight));
    napi_set_named_property(env, objContext, "minFormWidth", CreateJsValue(env, abilityInfo.minFormWidth));
    napi_set_named_property(env, objContext, "defaultFormWidth", CreateJsValue(env, abilityInfo.defaultFormWidth));
    napi_set_named_property(env, objContext, "backgroundModes", CreateJsValue(env, abilityInfo.backgroundModes));
    napi_set_named_property(env, objContext, "subType", CreateJsValue(env, static_cast<int32_t>(abilityInfo.subType)));
    napi_set_named_property(env, objContext, "isVisible", CreateJsValue(env, abilityInfo.visible));
    napi_set_named_property(env, objContext, "formEnabled", CreateJsValue(env, abilityInfo.formEnabled));
    napi_set_named_property(env, objContext, "permissions", CreateNativeArray(env, abilityInfo.permissions));
    napi_set_named_property(env, objContext, "deviceCapabilities",
        CreateNativeArray(env, abilityInfo.deviceCapabilities));
    napi_set_named_property(env, objContext, "deviceTypes", CreateNativeArray(env, abilityInfo.deviceTypes));
    napi_set_named_property(env, objContext, "applicationInfo", CreateAppInfo(env, abilityInfo.applicationInfo));

    return objContext;
}

napi_value JsNapiCommon::CreateAbilityInfo(
    napi_env env, const std::shared_ptr<JsAbilityInfoInfo> &abilityInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called");
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "called");
        return CreateJsUndefined(env);
    }

    return CreateAbilityInfo(env, abilityInfo->abilityInfo);
}

napi_value JsNapiCommon::CreateAbilityInfos(napi_env env, const std::vector<AbilityInfo> &abilityInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, abilityInfos.size(), &arrayValue);
    if (arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateArray failed");
        return CreateJsUndefined(env);
    }
    for (uint32_t i = 0; i < abilityInfos.size(); i++) {
        napi_set_element(env, arrayValue, i, CreateAbilityInfo(env, abilityInfos.at(i)));
    }

    return arrayValue;
}

bool JsNapiCommon::CheckAbilityType(const AbilityType typeWant)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params int error");
        return false;
    }
    const std::shared_ptr<AbilityInfo> info = ability_->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "get ability info error");
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

napi_value JsNapiCommon::CreateAppVersionInfo(napi_env env, const std::shared_ptr<JsApplicationInfo> &appInfo)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "CreateAppVersionInfo called");
    CHECK_POINTER_AND_RETURN_LOG(appInfo, CreateJsUndefined(env), "input params error");
    napi_value objContext = nullptr;
    napi_create_object(env, &objContext);
    CHECK_POINTER_AND_RETURN_LOG(objContext, CreateJsUndefined(env), "CreateObject failed");

    napi_set_named_property(env, objContext, "appName", CreateJsValue(env, appInfo->appInfo.name));
    napi_set_named_property(env, objContext, "versionName", CreateJsValue(env, appInfo->appInfo.versionName));
    napi_set_named_property(env, objContext, "versionCode",
        CreateJsValue(env, static_cast<int32_t>(appInfo->appInfo.versionCode)));

    return objContext;
}

bool JsNapiCommon::UnwarpVerifyPermissionParams(napi_env env, napi_callback_info info, JsPermissionOptions &options)
{
    bool flagCall = true;
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == ARGS_ONE) {
        flagCall = false;
    } else if (argc == ARGS_TWO && !AppExecFwk::IsTypeForNapiValue(env, argv[PARAM1], napi_function)) {
        if (!GetPermissionOptions(env, argv[PARAM1], options)) {
            TAG_LOGW(AAFwkTag::JSNAPI, "input params string error");
        }
        flagCall = false;
    } else if (argc == ARGS_THREE) {
        if (!GetPermissionOptions(env, argv[PARAM1], options)) {
            TAG_LOGW(AAFwkTag::JSNAPI, "input params string error");
        }
    }

    return flagCall;
}

bool JsNapiCommon::GetStringsValue(napi_env env, napi_value object, std::vector<std::string> &strList)
{
    bool isArray = false;
    napi_is_array(env, object, &isArray);
    if (object == nullptr || !isArray) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params error");
        return false;
    }
    uint32_t length = 0;
    napi_get_array_length(env, object, &length);
    for (uint32_t i = 0; i < length; i++) {
        std::string itemStr("");
        napi_value elementVal = nullptr;
        napi_get_element(env, object, i, &elementVal);
        if (!ConvertFromJsValue(env, elementVal, itemStr)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetElement from to array [%{public}u] error", i);
            return false;
        }
        strList.push_back(itemStr);
    }

    return true;
}

bool JsNapiCommon::GetPermissionOptions(napi_env env, napi_value object, JsPermissionOptions &options)
{
    if (object == nullptr || !CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params error");
        return false;
    }
    napi_value uidValue = nullptr;
    napi_get_named_property(env, object, "uid", &uidValue);
    napi_value pidValue = nullptr;
    napi_get_named_property(env, object, "pid", &pidValue);
    options.uidFlag = ConvertFromJsValue(env, uidValue, options.uid);
    options.pidFlag = ConvertFromJsValue(env, pidValue, options.pid);

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
        TAG_LOGE(AAFwkTag::JSNAPI, "convert error code failed");
        return std::string("execution failed");
    }

    return findECode->second;
}

napi_value JsNapiCommon::JsGetWant(napi_env env, napi_callback_info info, const AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<JsWant> pwant = std::make_shared<JsWant>();
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto execute = [obj = this, want = pwant, value = errorVal, abilityType] () {
        if (obj->ability_ == nullptr) {
            *value = static_cast<int32_t>(NAPI_ERR_ACE_ABILITY);
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }
        if (!obj->CheckAbilityType(abilityType)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the abilityType is error");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_TYPE_INVALID);
            return;
        }

        auto wantData = obj->ability_->GetWant();
        if (wantData == nullptr || want == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "wantData or want is nullptr!");
            *value = static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID);
            return;
        }
        want->want = *wantData;
    };

    auto complete = [obj = this, value = errorVal, pwant]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*value == NAPI_ERR_NO_ERROR && pwant != nullptr) {
            task.Resolve(env, obj->CreateWant(env, pwant));
        } else {
            auto error = (pwant == nullptr) ? static_cast<int32_t>(NAPI_ERR_ABILITY_CALL_INVALID) : *value;
            task.Reject(env, CreateJsError(env, error, "GetAbilityInfo return nullptr"));
        }
    };

    auto callback = (argc == ARGS_ZERO) ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetWant",
        env, CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsNapiCommon::CreateWant(napi_env env, const std::shared_ptr<JsWant> &want)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s,called", __func__);
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "CreateWant error, want is nullptr.");
        return CreateJsUndefined(env);
    }

    return CreateJsWant(env, want->want);
}

napi_value JsNapiCommon::JsTerminateAbility(napi_env env, NapiCallbackInfo& info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    if (info.argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s input params count error, argc=%{public}zu", __func__, info.argc);
        return CreateJsUndefined(env);
    }

    auto complete = [obj = this](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ != nullptr) {
            obj->ability_->TerminateAbility();
        } else {
            TAG_LOGE(AAFwkTag::JSNAPI, "JsTerminateAbility ability is nullptr");
        }
        task.Resolve(env, CreateJsNull(env));
    };

    auto callback = (info.argc == ARGS_ZERO) ? nullptr : info.argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsTerminateAbility",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsNapiCommon::JsStartAbility(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    auto errorVal = std::make_shared<int32_t>(static_cast<int32_t>(NAPI_ERR_NO_ERROR));
    auto param = std::make_shared<CallAbilityParam>();
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc == 0 || argc > ARGS_TWO) {
        TAG_LOGE(AAFwkTag::JSNAPI, "input params count error, argc=%{public}zu", argc);
        *errorVal = NAPI_ERR_PARAM_INVALID;
    } else {
        if (!UnwrapParamForWant(env, argv[PARAM0], abilityType, *param)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "call UnwrapParamForWant failed.");
            *errorVal = NAPI_ERR_PARAM_INVALID;
        }
    }

    if ((param->want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        param->want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    }

    auto execute = [obj = this, value = errorVal, abilityType, paramObj = param, &observer = freeInstallObserver_] () {
        if (*value != NAPI_ERR_NO_ERROR) {
            TAG_LOGE(AAFwkTag::JSNAPI, "JsStartAbility params error!");
            return;
        }

        if (obj->ability_ == nullptr) {
            *value = NAPI_ERR_ACE_ABILITY;
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the ability is nullptr");
            return;
        }

        if (!obj->CheckAbilityType(abilityType)) {
            *value = NAPI_ERR_ABILITY_TYPE_INVALID;
            TAG_LOGE(AAFwkTag::JSNAPI, "task execute error, the abilityType is error");
            return;
        }
#ifdef SUPPORT_GRAPHICS
        // inherit split mode
        auto windowMode = obj->ability_->GetCurrentWindowMode();
        if (windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
            windowMode == AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
            paramObj->want.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
        }
        TAG_LOGD(AAFwkTag::JSNAPI, "window mode is %{public}d", windowMode);

        // follow orientation
        paramObj->want.SetParam("ohos.aafwk.Orientation", 0);
        if (windowMode != AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
            auto orientation = obj->ability_->GetDisplayOrientation();
            paramObj->want.SetParam("ohos.aafwk.Orientation", orientation);
            TAG_LOGD(AAFwkTag::JSNAPI, "display orientation is %{public}d", orientation);
        }
#endif
        if (paramObj->setting == nullptr) {
            TAG_LOGI(AAFwkTag::JSNAPI, "param.setting == nullptr call StartAbility.");
            *value = obj->ability_->StartAbility(paramObj->want);
        } else {
            TAG_LOGI(AAFwkTag::JSNAPI, "param.setting != nullptr call StartAbility.");
            *value = obj->ability_->StartAbility(paramObj->want, *(paramObj->setting));
        }
        if ((paramObj->want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND &&
            *value != 0 && observer != nullptr) {
            std::string bundleName = paramObj->want.GetElement().GetBundleName();
            std::string abilityName = paramObj->want.GetElement().GetAbilityName();
            std::string startTime = paramObj->want.GetStringParam(Want::PARAM_RESV_START_TIME);
            observer->OnInstallFinished(bundleName, abilityName, startTime, *value);
        }
    };

    auto complete = [value = errorVal]
        (napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*value != NAPI_ERR_NO_ERROR) {
            int32_t errCode = GetStartAbilityErrorCode(*value);
            task.Reject(env, CreateJsError(env, errCode, "StartAbility Failed"));
            return;
        }
        task.Resolve(env, CreateJsValue(env, *value));
    };

    auto callback = (argc == ARGS_ONE) ? nullptr : argv[PARAM1];
    napi_value result = nullptr;
    if ((param->want.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        AddFreeInstallObserver(env, param->want, callback, &result);
        NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsStartAbility", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), nullptr, nullptr));
    } else {
        NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsStartAbility", env,
            CreateAsyncTaskWithLastParam(env, callback, std::move(execute), std::move(complete), &result));
    }

    return result;
}

napi_value JsNapiCommon::JsGetExternalCacheDir(napi_env env, napi_callback_info info, AbilityType abilityType)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "%{public}s called", __func__);
    size_t argc = ARGS_MAX_COUNT;
    napi_value argv[ARGS_MAX_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc > ARGS_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "%{public}s input params count error, argc=%{public}zu", __func__, argc);
        return CreateJsUndefined(env);
    }

    auto complete = [obj = this, abilityType](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (obj->ability_ == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "JsGetExternalCacheDir ability is nullptr");
            task.RejectWithCustomize(
                env,
                CreateJsError(env, NAPI_ERR_ACE_ABILITY, "JsGetExternalCacheDir Failed"),
                CreateJsNull(env));
            return;
        }

        if (!obj->CheckAbilityType(abilityType)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "JsGetExternalCacheDir abilityType is error");
            task.Reject(env, CreateJsError(env, NAPI_ERR_ABILITY_TYPE_INVALID, "JsGetExternalCacheDir Failed"));
            return;
        }

        std::string result = obj->ability_->GetExternalCacheDir();
        task.Resolve(env, CreateJsValue(env, result));
    };

    auto callback = (argc == ARGS_ZERO) ? nullptr : argv[PARAM0];
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsNapiCommon::JsGetExternalCacheDir",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

void JsNapiCommon::AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
    napi_value* result)
{
    // adapter free install async return install and start result
    TAG_LOGD(AAFwkTag::JSNAPI, "AddFreeInstallObserver start.");
    int ret = 0;
    if (freeInstallObserver_ == nullptr) {
        freeInstallObserver_ = new JsFreeInstallObserver(env);
        ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(freeInstallObserver_);
    }

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "AddFreeInstallObserver wrong.");
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "AddJsObserverObject");
        // build a callback observer with last param
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        std::string startTime = want.GetStringParam(Want::PARAM_RESV_START_TIME);
        freeInstallObserver_->AddJsObserverObject(bundleName, abilityName, startTime, callback, result);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
