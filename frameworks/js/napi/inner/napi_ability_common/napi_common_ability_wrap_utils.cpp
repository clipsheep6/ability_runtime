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

#include "napi_common_ability_wrap_utils.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
const std::int32_t STR_MAX_SIZE = 128;
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

napi_value BuildJsAbilityInfoNamedPropertyFirst(napi_env env, const AbilityInfo &abilityInfo, napi_value &result,
    napi_value &proValue)
{
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

    return result;
}

napi_value BuildJsAbilityInfoNamedPropertySecond(napi_env env, const AbilityInfo &abilityInfo, napi_value &result,
    napi_value &proValue)
{
    NAPI_CALL(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.subType), &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "subType", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.visible, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "isVisible", proValue));

    NAPI_CALL(env, napi_get_boolean(env, abilityInfo.formEnabled, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "formEnabled", proValue));
    return result;
}

napi_value WrapAbilityInfo(napi_env env, const AbilityInfo &abilityInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    (void)BuildJsAbilityInfoNamedPropertyFirst(env, abilityInfo, result, proValue);
    (void)BuildJsAbilityInfoNamedPropertySecond(env, abilityInfo, result, proValue);
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

napi_value BuildJsHapModuleInfoNamedProperty(napi_env env, const HapModuleInfoCB &cb, napi_value &result,
    napi_value &proValue)
{
    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "name", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "description", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "icon", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "label", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.backgroundImg.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "backgroundImg", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "moduleName", proValue));

    NAPI_CALL(env, napi_create_int32(env, cb.hapModuleInfo.supportedModes, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "supportedModes", proValue));

    NAPI_CALL(env, napi_create_int32(env, cb.hapModuleInfo.descriptionId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "descriptionId", proValue));

    NAPI_CALL(env, napi_create_int32(env, cb.hapModuleInfo.labelId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "labelId", proValue));

    NAPI_CALL(env, napi_create_int32(env, cb.hapModuleInfo.iconId, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "iconId", proValue));

    NAPI_CALL(env, napi_create_string_utf8(env, cb.hapModuleInfo.mainAbility.c_str(), NAPI_AUTO_LENGTH, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "mainAbilityName", proValue));

    NAPI_CALL(env, napi_get_boolean(env, cb.hapModuleInfo.installationFree, &proValue));
    NAPI_CALL(env, napi_set_named_property(env, result, "installationFree", proValue));
    return result;
}

napi_value WrapHapModuleInfo(napi_env env, const HapModuleInfoCB &cb)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s called.", __func__);
    napi_value result = nullptr;
    napi_value proValue = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    napi_value ret = BuildJsHapModuleInfoNamedProperty(env, cb, result, proValue);
    if (ret == nullptr) {
        return ret;
    }

    napi_value jsArrayreqCapabilities = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayreqCapabilities));
    for (size_t i = 0; i < cb.hapModuleInfo.reqCapabilities.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, cb.hapModuleInfo.reqCapabilities.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayreqCapabilities, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "reqCapabilities", jsArrayreqCapabilities));

    napi_value jsArraydeviceTypes = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArraydeviceTypes));
    for (size_t i = 0; i < cb.hapModuleInfo.deviceTypes.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env,
            napi_create_string_utf8(env, cb.hapModuleInfo.deviceTypes.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArraydeviceTypes, i, proValue));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "deviceTypes", jsArraydeviceTypes));

    napi_value abilityInfos = nullptr;
    NAPI_CALL(env, napi_create_array(env, &abilityInfos));
    for (size_t i = 0; i < cb.hapModuleInfo.abilityInfos.size(); i++) {
        napi_value abilityInfo = nullptr;
        abilityInfo = WrapAbilityInfo(env, cb.hapModuleInfo.abilityInfos.at(i));
        NAPI_CALL(env, napi_set_element(env, abilityInfos, i, abilityInfo));
    }
    NAPI_CALL(env, napi_set_named_property(env, result, "abilityInfo", abilityInfos));
    TAG_LOGI(AAFwkTag::JSNAPI, "%{public}s end.", __func__);
    return result;
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
}  // namespace AppExecFwk
}  // namespace OHOS