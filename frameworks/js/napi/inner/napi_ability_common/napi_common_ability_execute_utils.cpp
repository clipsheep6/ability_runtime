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

#include "napi_common_ability_execute_utils.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_ability_wrap_utils.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief GetAppType asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AppTypeCB *appTypeCB = static_cast<AppTypeCB *>(data);
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null appTypeCB");
        return;
    }

    appTypeCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appTypeCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        appTypeCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appTypeCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        appTypeCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    appTypeCB->name = appTypeCB->cbBase.ability->GetAppType();
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypeAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAppTypePromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (appTypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null appTypeCB");
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Wrong arguments count");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppTypeAsync(env, args, 0, appTypeCB);
    } else {
        ret = GetAppTypePromise(env, appTypeCB);
    }

    return ret;
}

/**
 * @brief GetAbilityInfo asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AbilityInfoCB *abilityInfoCB = static_cast<AbilityInfoCB *>(data);
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "nullabilityInfoCB");
        return;
    }

    abilityInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityInfoCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfoPtr = abilityInfoCB->cbBase.ability->GetAbilityInfo();
    if (abilityInfoPtr != nullptr) {
        abilityInfoCB->abilityInfo = *abilityInfoPtr;
    } else {
        abilityInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityInfoPromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (abilityInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "nullabilityInfoCB");
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityInfoAsync(env, args, 0, abilityInfoCB);
    } else {
        ret = GetAbilityInfoPromise(env, abilityInfoCB);
    }

    return ret;
}

void GetHapModuleInfoExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    HapModuleInfoCB *hapModuleInfoCB = static_cast<HapModuleInfoCB *>(data);
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null hapModuleInfoCB");
        return;
    }

    hapModuleInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (hapModuleInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&hapModuleInfoCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<HapModuleInfo> hapModuleInfoPtr = hapModuleInfoCB->cbBase.ability->GetHapModuleInfo();
    if (hapModuleInfoPtr != nullptr) {
        hapModuleInfoCB->hapModuleInfo = *hapModuleInfoPtr;
    } else {
        hapModuleInfoCB->cbBase.errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

void GetHapModuleInfoAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

void GetHapModuleInfoPromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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

    return promise;
}

napi_value GetHapModuleInfoWrap(napi_env env, napi_callback_info info, HapModuleInfoCB *hapModuleInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (hapModuleInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null hapModuleInfoCB");
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetHapModuleInfoAsync(env, args, 0, hapModuleInfoCB);
    } else {
        ret = GetHapModuleInfoPromise(env, hapModuleInfoCB);
    }

    return ret;
}

void GetAppVersionInfoExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AppVersionInfoCB *appVersionInfoCB = static_cast<AppVersionInfoCB *>(data);
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null appVersionInfoCB");
        return;
    }

    appVersionInfoCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (appVersionInfoCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        appVersionInfoCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&appVersionInfoCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

void GetAppVersionInfoAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

void GetAppVersionInfoPromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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

    return promise;
}

napi_value GetAppVersionInfoWrap(napi_env env, napi_callback_info info, AppVersionInfoCB *appVersionInfoCB)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (appVersionInfoCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null appVersionInfoCB");
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "fail argument count");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAppVersionInfoAsync(env, args, 0, appVersionInfoCB);
    } else {
        ret = GetAppVersionInfoPromise(env, appVersionInfoCB);
    }

    return ret;
}

void GetWantExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return;
    }
    asyncCallbackInfo->errCode = NAPI_ERR_NO_ERROR;
    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    std::shared_ptr<AAFwk::Want> ptrWant = asyncCallbackInfo->ability->GetWant();
    if (ptrWant != nullptr) {
        asyncCallbackInfo->param.want = *ptrWant;
    } else {
        asyncCallbackInfo->errCode = NAPI_ERR_ABILITY_CALL_INVALID;
    }
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

napi_value GetWantAsync(napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null params");
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
    napi_create_async_work(env, nullptr, resourceName, GetWantExecuteCB,
        [](napi_env env, napi_status, void *data) {
            TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
            TAG_LOGD(AAFwkTag::JSNAPI, "end");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);
    napi_value result = nullptr;
    napi_get_null(env, &result);

    return result;
}

napi_value GetWantPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null params");
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
            TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
            TAG_LOGD(AAFwkTag::JSNAPI, "end");
        },
        static_cast<void *>(asyncCallbackInfo),
        &asyncCallbackInfo->asyncWork);
    napi_queue_async_work_with_qos(env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated);

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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetWantAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = GetWantPromise(env, asyncCallbackInfo);
    }

    return ret;
}

/**
 * @brief GetAbilityName asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameExecuteCB(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AbilityNameCB *abilityNameCB = static_cast<AbilityNameCB *>(data);
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null abilityNameCB");
        return;
    }
    abilityNameCB->cbBase.errCode = NAPI_ERR_NO_ERROR;
    if (abilityNameCB->cbBase.ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        abilityNameCB->cbBase.errCode = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(&abilityNameCB->cbBase)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        abilityNameCB->cbBase.errCode = NAPI_ERR_ABILITY_TYPE_INVALID;
        return;
    }

    abilityNameCB->name = abilityNameCB->cbBase.ability->GetAbilityName();
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNameAsyncCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void GetAbilityNamePromiseCompleteCB(napi_env env, napi_status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (abilityNameCB == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null abilityNameCB");
        return nullptr;
    }

    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = GetAbilityNameAsync(env, args, 0, abilityNameCB);
    } else {
        ret = GetAbilityNamePromise(env, abilityNameCB);
    }

    return ret;
}

void StopAbilityExecuteCallback(napi_env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncJSCallbackInfo *asyncCallbackInfo = static_cast<AsyncJSCallbackInfo *>(data);
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return;
    }

    asyncCallbackInfo->error_code = NAPI_ERR_NO_ERROR;
    asyncCallbackInfo->native_data.data_type = NVT_NONE;

    if (asyncCallbackInfo->ability == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        asyncCallbackInfo->error_code = NAPI_ERR_ACE_ABILITY;
        return;
    }

    if (!CheckAbilityType(asyncCallbackInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "wrong ability type");
        asyncCallbackInfo->error_code = NAPI_ERR_ABILITY_TYPE_INVALID;
        asyncCallbackInfo->native_data.data_type = NVT_UNDEFINED;
        return;
    }

    asyncCallbackInfo->native_data.data_type = NVT_BOOL;
    asyncCallbackInfo->native_data.bool_value = asyncCallbackInfo->ability->StopAbility(asyncCallbackInfo->param.want);
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

napi_value StopAbilityWrap(napi_env env, napi_callback_info info, AsyncJSCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    size_t argc = ARGS_MAX_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value jsthis = nullptr;
    void *data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &jsthis, &data));

    if (!UnwrapParamStopAbilityWrap(env, argc, args, asyncCallbackInfo)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "Invoke UnwrapParamStopAbility fail");
        return nullptr;
    }

    AsyncParamEx asyncParamEx;
    if (asyncCallbackInfo->cbInfo.callback != nullptr) {
        TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
        asyncParamEx.resource = "NAPI_StopAbilityWrapCallback";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompleteAsyncCallbackWork;

        return ExecuteAsyncCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    } else {
        TAG_LOGI(AAFwkTag::JSNAPI, "promise");
        asyncParamEx.resource = "NAPI_StopAbilityWrapPromise";
        asyncParamEx.execute = StopAbilityExecuteCallback;
        asyncParamEx.complete = CompletePromiseCallbackWork;

        return ExecutePromiseCallbackWork(env, asyncCallbackInfo, &asyncParamEx);
    }
}

void StartBackgroundRunningExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    AsyncCallbackInfo *asyncCallbackInfo = (AsyncCallbackInfo *)data;
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null asyncCallbackInfo");
        return;
    }
    if (asyncCallbackInfo->errCode == NAPI_ERR_PARAM_INVALID) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse input param failed");
        return;
    }
    if (asyncCallbackInfo->ability == nullptr) {
        asyncCallbackInfo->errCode = NAPI_ERR_ACE_ABILITY;
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
        return;
    }
    const std::shared_ptr<AbilityInfo> info = asyncCallbackInfo->ability->GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "ability info is null");
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

    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

void BackgroundRunningCallbackCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
}

void BackgroundRunningPromiseCompletedCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
    delete asyncCallbackInfo;
}

napi_value StartBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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

    return WrapVoidToJS(env);
}

napi_value StartBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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
    TAG_LOGD(AAFwkTag::JSNAPI, "end");
    return promise;
}

napi_value StartBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    size_t paramNums = 3;
    const size_t minParamNums = 2;
    const size_t maxParamNums = 3;
    napi_value args[maxParamNums] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &paramNums, args, NULL, NULL));

    if (paramNums < minParamNums || paramNums > maxParamNums) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
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

    TAG_LOGD(AAFwkTag::JSNAPI, "end");
    return ret;
}

void CancelBackgroundRunningExecuteCB(napi_env env, void *data)
{
    AsyncCallbackInfo *asyncCallbackInfo = static_cast<AsyncCallbackInfo *>(data);
    if (asyncCallbackInfo->ability != nullptr) {
        asyncCallbackInfo->errCode = asyncCallbackInfo->ability->StopBackgroundRunning();
    } else {
        TAG_LOGE(AAFwkTag::JSNAPI, "null ability");
    }
}

napi_value CancelBackgroundRunningAsync(
    napi_env env, napi_value *args, const size_t argCallback, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "asyncCallback");
    if (args == nullptr || asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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

    return result;
}

napi_value CancelBackgroundRunningPromise(napi_env env, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "promise");
    if (asyncCallbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null param");
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

    return promise;
}

napi_value CancelBackgroundRunningWrap(napi_env &env, napi_callback_info &info, AsyncCallbackInfo *asyncCallbackInfo)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, NULL, NULL));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        return nullptr;
    }

    if (argcAsync > argcPromise) {
        ret = CancelBackgroundRunningAsync(env, args, 0, asyncCallbackInfo);
    } else {
        ret = CancelBackgroundRunningPromise(env, asyncCallbackInfo);
    }

    TAG_LOGD(AAFwkTag::JSNAPI, "end");
    return ret;
}
}  // namespace AppExecFwk
}  // namespace OHOS