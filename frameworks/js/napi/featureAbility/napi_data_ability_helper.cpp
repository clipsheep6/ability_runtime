/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "napi_data_ability_helper.h"

#include <cstring>
#include <uv.h>
#include <vector>
#include <string>

#include "data_ability_helper.h"
#include "data_ability_observer_interface.h"
#include "uri.h"

#include "napi_common_ability.h"
#include "data_ability_operation.h"
#include "data_ability_result.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "napi_base_context.h"
#include "napi_data_ability_operation.h"
#include "napi_data_ability_predicates.h"
#include "napi_rdb_predicates.h"
#include "napi_result_set.h"
#include "securec.h"

#ifndef SUPPORT_GRAPHICS
#define DBL_MIN ((double)2.22507385850720138309e-308L)
#define DBL_MAX ((double)2.22507385850720138309e-308L)
#endif

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
std::list<std::shared_ptr<DataAbilityHelper>> g_dataAbilityHelperList;
std::vector<DAHelperOnOffCB *> g_registerInstances;

/**
 * @brief DataAbilityHelper NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value DataAbilityHelperInit(napi_env env, napi_value exports)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("insert", NAPI_Insert),
        DECLARE_NAPI_FUNCTION("notifyChange", NAPI_NotifyChange),
        DECLARE_NAPI_FUNCTION("on", NAPI_Register),
        DECLARE_NAPI_FUNCTION("off", NAPI_UnRegister),
        DECLARE_NAPI_FUNCTION("delete", NAPI_Delete),
        DECLARE_NAPI_FUNCTION("query", NAPI_Query),
        DECLARE_NAPI_FUNCTION("update", NAPI_Update),
        DECLARE_NAPI_FUNCTION("batchInsert", NAPI_BatchInsert),
        DECLARE_NAPI_FUNCTION("openFile", NAPI_OpenFile),
        DECLARE_NAPI_FUNCTION("getType", NAPI_GetType),
        DECLARE_NAPI_FUNCTION("getFileTypes", NAPI_GetFileTypes),
        DECLARE_NAPI_FUNCTION("normalizeUri", NAPI_NormalizeUri),
        DECLARE_NAPI_FUNCTION("denormalizeUri", NAPI_DenormalizeUri),
        DECLARE_NAPI_FUNCTION("executeBatch", NAPI_ExecuteBatch),
        DECLARE_NAPI_FUNCTION("call", NAPI_Call),
    };

    napi_value constructor;
    NAPI_CALL(env,
        napi_define_class(env,
            "dataAbilityHelper",
            NAPI_AUTO_LENGTH,
            DataAbilityHelperConstructor,
            nullptr,
            sizeof(properties) / sizeof(*properties),
            properties,
            &constructor));
    NAPI_CALL(env, SaveGlobalDataAbilityHelper(env, constructor));
    return exports;
}

napi_value DataAbilityHelperConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = {nullptr};
    napi_value thisVar = nullptr;
    auto& dataAbilityHelperStatus = GetDataAbilityHelperStatus();
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    std::shared_ptr<DataAbilityHelper> dataAbilityHelper = nullptr;
    bool stageMode = false;
    napi_status status = OHOS::AbilityRuntime::IsStageContext(env, argv[0], stageMode);
    if (status != napi_ok) {
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::FA, "Failed to get native context instance");
            return nullptr;
        }
        std::string strUri = NapiValueToStringUtf8(env, argv[0]);
        TAG_LOGI(AAFwkTag::FA, "FA Model: strUri = %{public}s", strUri.c_str());
        dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
    } else {
        if (stageMode) {
            auto context = OHOS::AbilityRuntime::GetStageModeContext(env, argv[0]);
            if (context == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "Failed to get native context instance");
                return nullptr;
            }
            std::string strUri = NapiValueToStringUtf8(env, argv[PARAM1]);
            TAG_LOGI(AAFwkTag::FA, "Stage Model: strUri = %{public}s", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(context, std::make_shared<Uri>(strUri));
        } else {
            auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
            if (ability == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "Failed to get native context instance");
                return nullptr;
            }
            std::string strUri = NapiValueToStringUtf8(env, argv[PARAM1]);
            TAG_LOGI(AAFwkTag::FA, "FA Model: strUri = %{public}s", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
        }
    }

    if (dataAbilityHelper == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "dataAbilityHelper is nullptr");
        dataAbilityHelperStatus = false;
        return nullptr;
    }
    dataAbilityHelper->SetCallFromJs();
    g_dataAbilityHelperList.emplace_back(dataAbilityHelper);
    auto wrapper = new NAPIDataAbilityHelperWrapper(dataAbilityHelper);

    napi_wrap(
        env,
        thisVar,
        wrapper,
        [](napi_env env, void *data, void *hint) {
            auto objectInfo = static_cast<NAPIDataAbilityHelperWrapper *>(data);
            if (objectInfo == nullptr) {
                TAG_LOGW(AAFwkTag::FA, "DAHelper finalize_cb objectInfo is nullptr.");
                return;
            }
            TAG_LOGD(AAFwkTag::FA, "DAHelper finalize_cb dataAbilityHelperList.size = %{public}zu, "
                "regInstances_.size = %{public}zu",
                g_dataAbilityHelperList.size(), g_registerInstances.size());
            for (auto iter = g_registerInstances.begin(); iter != g_registerInstances.end();) {
                if (!NeedErase(iter, objectInfo->GetDataAbilityHelper())) {
                    iter = g_registerInstances.erase(iter);
                }
            }
            g_dataAbilityHelperList.remove_if(
                [objectInfo](const std::shared_ptr<DataAbilityHelper> &dataAbilityHelper) {
                    return objectInfo->GetDataAbilityHelper() == dataAbilityHelper;
                });
            TAG_LOGD(AAFwkTag::FA, "dataAbilityHelperList.size = %{public}zu, regInstances_.size = %{public}zu",
                g_dataAbilityHelperList.size(), g_registerInstances.size());
            delete objectInfo;
            objectInfo = nullptr;
        },
        nullptr,
        nullptr);

    dataAbilityHelperStatus = true;
    return thisVar;
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Insert(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperInsertCB *insertCB = new (std::nothrow) DAHelperInsertCB;
    if (insertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, insertCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    insertCB->cbBase.cbInfo.env = env;
    insertCB->cbBase.asyncWork = nullptr;
    insertCB->cbBase.deferred = nullptr;
    insertCB->cbBase.ability = nullptr;

    napi_value ret = InsertWrap(env, info, insertCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, ret == nullptr.", __func__);
        delete insertCB;
        insertCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called end", __func__);
    return ret;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value InsertWrap(napi_env env, napi_callback_info info, DAHelperInsertCB *insertCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong parameter count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        insertCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, insertCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
    }

    insertCB->valueBucket.Clear();
    AnalysisValuesBucket(insertCB->valueBucket, env, args[PARAM1]);
    GetDataAbilityHelper(env, thisVar, insertCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = InsertAsync(env, args, ARGS_TWO, insertCB);
    } else {
        ret = InsertPromise(env, insertCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

void AnalysisValuesBucket(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, const napi_value &arg)
{
    napi_value keys = nullptr;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FA, "ValuesBucket errr");
        return;
    }
    TAG_LOGI(AAFwkTag::FA, "ValuesBucket num:%{public}d ", arrLen);
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key = nullptr;
        (void)napi_get_element(env, keys, i, &key);
        std::string keyStr = UnwrapStringFromJS(env, key);
        napi_value value = nullptr;
        napi_get_property(env, arg, key, &value);

        SetValuesBucketObject(valuesBucket, env, keyStr, value);
    }
}

void SetValuesBucketObject(
    NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, std::string keyStr, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        std::string valueString = UnwrapStringFromJS(env, value);
        TAG_LOGI(AAFwkTag::FA, "ValueObject type:%{public}d, key:%{public}s, value:%{private}s",
            valueType,
            keyStr.c_str(),
            valueString.c_str());
        valuesBucket.PutString(keyStr, valueString);
    } else if (valueType == napi_number) {
        double valueNumber = 0;
        napi_get_value_double(env, value, &valueNumber);
        valuesBucket.PutDouble(keyStr, valueNumber);
        TAG_LOGI(AAFwkTag::FA, "ValueObject type:%{public}d, key:%{public}s, value:%{private}lf", valueType,
            keyStr.c_str(), valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, value, &valueBool);
        TAG_LOGI(AAFwkTag::FA, "ValueObject type:%{public}d, key:%{public}s, value:%{private}d", valueType,
            keyStr.c_str(), valueBool);
        valuesBucket.PutBool(keyStr, valueBool);
    } else if (valueType == napi_null) {
        valuesBucket.PutNull(keyStr);
        TAG_LOGI(AAFwkTag::FA, "ValueObject type:%{public}d, key:%{public}s, value:null", valueType, keyStr.c_str());
    } else if (valueType == napi_object) {
        TAG_LOGI(
            AAFwkTag::FA, "ValueObject type:%{public}d, key:%{public}s, value:Uint8Array", valueType, keyStr.c_str());
        valuesBucket.PutBlob(keyStr, ConvertU8Vector(env, value));
    } else {
        TAG_LOGE(AAFwkTag::FA, "valuesBucket error");
    }
}
napi_value InsertAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperInsertCB *insertCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || insertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &insertCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            InsertExecuteCB,
            InsertAsyncCompleteCB,
            static_cast<void *>(insertCB),
            &insertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, insertCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value InsertPromise(napi_env env, DAHelperInsertCB *insertCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (insertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    insertCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            InsertExecuteCB,
            InsertPromiseCompleteCB,
            static_cast<void *>(insertCB),
            &insertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, insertCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end", __func__);
    return promise;
}

void InsertExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Insert, worker pool thread execute.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    if (insertCB == nullptr) {
        TAG_LOGW(AAFwkTag::FA, "NAPI_Insert, insertCB is nullptr.");
        return;
    }
    auto dataAbilityHelper = insertCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        insertCB->execResult = INVALID_PARAMETER;
        if (!insertCB->uri.empty()) {
            OHOS::Uri uri(insertCB->uri);
            insertCB->result = dataAbilityHelper->Insert(uri, insertCB->valueBucket);
            insertCB->execResult = NO_ERROR;
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_Insert, dataAbilityHelper == nullptr.");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_Insert, worker pool thread execute end.");
}

void InsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Insert, main event thread complete.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, insertCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, insertCB->execResult);
    napi_create_int32(env, insertCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (insertCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, insertCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_Insert, main event thread complete end.");
}

void InsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Insert,  main event thread complete.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, insertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, insertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_Insert,  main event thread complete end.");
}

/**
 * @brief Parse the ValuesBucket parameters.
 *
 * @param param Indicates the want parameters saved the parse result.
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value UnwrapValuesBucket(std::string &value, napi_env env, napi_value args)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, valueType != napi_object.", __func__);
        return nullptr;
    }

    std::string strValue = "";
    if (UnwrapStringByPropertyName(env, args, "value", strValue)) {
        TAG_LOGI(AAFwkTag::FA, "%{public}s,strValue=%{private}s", __func__, strValue.c_str());
        value = strValue;
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, value == nullptr.", __func__);
        return nullptr;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return result;
}

/**
 * @brief DataAbilityHelper NAPI method : notifyChange.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_NotifyChange(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperNotifyChangeCB *notifyChangeCB = new DAHelperNotifyChangeCB;
    notifyChangeCB->cbBase.cbInfo.env = env;
    notifyChangeCB->cbBase.asyncWork = nullptr;
    notifyChangeCB->cbBase.deferred = nullptr;
    notifyChangeCB->cbBase.ability = nullptr;

    napi_value ret = NotifyChangeWrap(env, info, notifyChangeCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, ret == nullptr.", __func__);
        delete notifyChangeCB;
        notifyChangeCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

/**
 * @brief NotifyChange processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param notifyChangeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NotifyChangeWrap(napi_env env, napi_callback_info info, DAHelperNotifyChangeCB *notifyChangeCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong parameter count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        notifyChangeCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, notifyChangeCB->uri.c_str());
    }
    GetDataAbilityHelper(env, thisVar, notifyChangeCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = NotifyChangeAsync(env, args, argcAsync, argcPromise, notifyChangeCB);
    } else {
        ret = NotifyChangePromise(env, notifyChangeCB);
    }
    return ret;
}

napi_value NotifyChangeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperNotifyChangeCB *notifyChangeCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || notifyChangeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &notifyChangeCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NotifyChangeExecuteCB,
            NotifyChangeAsyncCompleteCB,
            static_cast<void *>(notifyChangeCB),
            &notifyChangeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, notifyChangeCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value NotifyChangePromise(napi_env env, DAHelperNotifyChangeCB *notifyChangeCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (notifyChangeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    notifyChangeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NotifyChangeExecuteCB,
            NotifyChangePromiseCompleteCB,
            static_cast<void *>(notifyChangeCB),
            &notifyChangeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, notifyChangeCB->cbBase.asyncWork));
    return promise;
}

void NotifyChangeExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_NotifyChange, worker pool thread execute.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    auto dataAbilityHelper = notifyChangeCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        notifyChangeCB->execResult = INVALID_PARAMETER;
        if (!notifyChangeCB->uri.empty()) {
            OHOS::Uri uri(notifyChangeCB->uri);
            dataAbilityHelper->NotifyChange(uri);
            notifyChangeCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, notifyChangeCB uri is empty.", __func__);
        }
    }
}

void NotifyChangeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_NotifyChange, main event thread complete.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, notifyChangeCB->cbBase.cbInfo.callback, &callback));

    if (!IsTypeForNapiValue(env, callback, napi_function)) {
        delete notifyChangeCB;
        notifyChangeCB = nullptr;
        TAG_LOGI(AAFwkTag::FA, "NAPI_NotifyChange, callback is invalid.");
        return;
    }

    result[PARAM0] = GetCallbackErrorValue(env, notifyChangeCB->execResult);
    result[PARAM1] = WrapVoidToJS(env);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (notifyChangeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, notifyChangeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, notifyChangeCB->cbBase.asyncWork));
    delete notifyChangeCB;
    notifyChangeCB = nullptr;
}

void NotifyChangePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_NotifyChange,  main event thread complete.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, 0, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, notifyChangeCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, notifyChangeCB->cbBase.asyncWork));
    delete notifyChangeCB;
    notifyChangeCB = nullptr;
}

/**
 * @brief DataAbilityHelper NAPI method : on.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Register(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->cbBase.cbInfo.env = env;
    onCB->cbBase.asyncWork = nullptr;
    onCB->cbBase.deferred = nullptr;
    onCB->cbBase.ability = nullptr;

    napi_value ret = RegisterWrap(env, info, onCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, ret == nullptr.", __func__);
        delete onCB;
        onCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called end", __func__);
    return ret;
}

/**
 * @brief On processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param onCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value RegisterWrap(napi_env env, napi_callback_info info, DAHelperOnOffCB *onCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,start", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong parameter count.", __func__);
        return nullptr;
    }

    onCB->result = NO_ERROR;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        std::string type = NapiValueToStringUtf8(env, args[PARAM0]);
        if (type == "dataChange") {
            TAG_LOGI(AAFwkTag::FA, "%{public}s, Right type=%{public}s", __func__, type.c_str());
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type is %{public}s.", __func__, type.c_str());
            onCB->result = INVALID_PARAMETER;
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
        onCB->result = INVALID_PARAMETER;
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        onCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, onCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
        onCB->result = INVALID_PARAMETER;
    }
    GetDataAbilityHelper(env, thisVar, onCB->dataAbilityHelper);

    ret = RegisterAsync(env, args, argcAsync, argcPromise, onCB);
    return ret;
}

napi_value RegisterAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperOnOffCB *onCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || onCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        TAG_LOGI(AAFwkTag::FA, "valuetype is napi_function");
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &onCB->cbBase.cbInfo.callback));
    } else {
        TAG_LOGI(AAFwkTag::FA, "not valuetype isn't napi_function");
        onCB->result = INVALID_PARAMETER;
    }

    sptr<NAPIDataAbilityObserver> observer(new NAPIDataAbilityObserver());
    observer->SetEnv(env);
    observer->SetCallbackRef(onCB->cbBase.cbInfo.callback);
    onCB->observer = observer;

    if (onCB->result == NO_ERROR) {
        g_registerInstances.emplace_back(onCB);
    }

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            RegisterExecuteCB,
            RegisterCompleteCB,
            static_cast<void *>(onCB),
            &onCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, onCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

void RegisterExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Register, worker pool thread execute.");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (onCBIter == g_registerInstances.end()) {
        // onCB is invalid or onCB has been delete
        TAG_LOGE(AAFwkTag::FA, "%{public}s, input params onCB is invalid.", __func__);
        return;
    }

    auto dataAbilityHelper = onCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        if (onCB->result != INVALID_PARAMETER && !onCB->uri.empty() && onCB->cbBase.cbInfo.callback != nullptr) {
            OHOS::Uri uri(onCB->uri);
            dataAbilityHelper->RegisterObserver(uri, onCB->observer);
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, dataAbilityHelper uri is empty or callback is nullptr.", __func__);
        }
    }
}

void RegisterCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Register, main event thread complete.");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, input params onCB is nullptr.", __func__);
        return;
    }

    auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (onCBIter == g_registerInstances.end()) {
        // onCB is invalid or onCB has been delete
        TAG_LOGE(AAFwkTag::FA, "%{public}s, input params onCB is invalid.", __func__);
        return;
    }

    if (onCB->result == NO_ERROR) {
        return;
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_Register, input params onCB will be release");
    DeleteDAHelperOnOffCB(onCB);
    TAG_LOGI(AAFwkTag::FA, "NAPI_Register, main event thread complete over an release invalid onCB.");
}

/**
 * @brief DataAbilityHelper NAPI method : Off.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_UnRegister(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperOnOffCB *offCB = new DAHelperOnOffCB;
    offCB->cbBase.cbInfo.env = env;
    offCB->cbBase.asyncWork = nullptr;
    offCB->cbBase.deferred = nullptr;
    offCB->cbBase.ability = nullptr;

    napi_value ret = UnRegisterWrap(env, info, offCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, ret == nullptr.", __func__);
        delete offCB;
        offCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called end", __func__);
    return ret;
}

/**
 * @brief Off processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param offCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value UnRegisterWrap(napi_env env, napi_callback_info info, DAHelperOnOffCB *offCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argCountWithAsync = ARGS_TWO + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    offCB->result = NO_ERROR;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        std::string type = NapiValueToStringUtf8(env, args[PARAM0]);
        if (type == "dataChange") {
            TAG_LOGI(AAFwkTag::FA, "%{public}s, Wrong type=%{public}s", __func__, type.c_str());
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type %{public}s.", __func__, type.c_str());
            offCB->result = INVALID_PARAMETER;
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
        offCB->result = INVALID_PARAMETER;
    }

    offCB->uri = "";
    if (argcAsync > ARGS_TWO) {
        // parse uri and callback
        NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, offCB->uri.c_str());
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
            offCB->result = INVALID_PARAMETER;
        }
        NAPI_CALL(env, napi_typeof(env, args[PARAM2], &valuetype));
        if (valuetype == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, args[PARAM2], 1, &offCB->cbBase.cbInfo.callback));
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
            offCB->result = INVALID_PARAMETER;
        }
    } else {
        // parse uri or callback
        NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, offCB->uri.c_str());
        } else if (valuetype == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, args[PARAM1], 1, &offCB->cbBase.cbInfo.callback));
        } else {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
            offCB->result = INVALID_PARAMETER;
        }
    }
    GetDataAbilityHelper(env, thisVar, offCB->dataAbilityHelper);

    ret = UnRegisterSync(env, offCB);
    return ret;
}

napi_value UnRegisterSync(napi_env env, DAHelperOnOffCB *offCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, syncCallback.", __func__);
    if (offCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, offCB == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    if (offCB->result == NO_ERROR) {
        FindRegisterObs(env, offCB);
    }

    TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister, offCB->NotifyList size is %{public}zu", offCB->NotifyList.size());
    for (auto &iter : offCB->NotifyList) {
        if (iter != nullptr && iter->observer != nullptr) {
            OHOS::Uri uri(iter->uri);
            auto dataAbilityHelper = iter->dataAbilityHelper;
            if (dataAbilityHelper != nullptr) {
                dataAbilityHelper->UnregisterObserver(uri, iter->observer);
            }
            offCB->DestroyList.emplace_back(iter);
        }
    }
    offCB->NotifyList.clear();

    TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister, offCB->DestroyList size is %{public}zu", offCB->DestroyList.size());
    for (auto &iter : offCB->DestroyList) {
        TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister ReleaseJSCallback. 1 ---");
        if (iter->observer != nullptr) {
            iter->observer->ReleaseJSCallback();
            delete iter;
            iter = nullptr;
            TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister ReleaseJSCallback. 2 ---");
        }
    }

    offCB->DestroyList.clear();
    delete offCB;
    offCB = nullptr;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

void FindRegisterObs(napi_env env, DAHelperOnOffCB *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister, FindRegisterObs main event thread execute.");
    if (data == nullptr || data->dataAbilityHelper == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "NAPI_UnRegister, param is null.");
        return;
    }

    TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister, uri=%{public}s.", data->uri.c_str());
    if (!data->uri.empty()) {
        // if match uri, unregister all observers corresponding the uri
        std::string strUri = data->uri;
        auto iter = g_registerInstances.begin();
        while (iter != g_registerInstances.end()) {
            DAHelperOnOffCB *helper = *iter;
            if (helper == nullptr || helper->uri != strUri) {
                iter++;
                continue;
            }
            data->NotifyList.emplace_back(helper);
            iter = g_registerInstances.erase(iter);
            TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister Instances erase size = %{public}zu", g_registerInstances.size());
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_UnRegister, error: uri is null.");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_UnRegister, FindRegisterObs main event thread execute.end %{public}zu",
        data->NotifyList.size());
}

void NAPIDataAbilityObserver::ReleaseJSCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ref_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "NAPIDataAbilityObserver::ReleaseJSCallback, ref_ is null.");
        return;
    }

    if (isCallingback_) {
        needRelease_ = true;
        TAG_LOGW(AAFwkTag::FA, "%{public}s, ref_ is calling back.", __func__);
        return;
    }

    SafeReleaseJSCallback();
    TAG_LOGI(AAFwkTag::FA, "NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SafeReleaseJSCallback()
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, loop == nullptr.", __func__);
        return;
    }

    struct DelRefCallbackInfo {
        napi_env env_;
        napi_ref ref_;
    };

    DelRefCallbackInfo* delRefCallbackInfo = new DelRefCallbackInfo {
        .env_ = env_,
        .ref_ = ref_,
    };

    uv_work_t* work = new uv_work_t;
    work->data = static_cast<void*>(delRefCallbackInfo);
    int ret = uv_queue_work_with_qos(
        loop, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            // JS Thread
            if (work == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "uv_queue_work input work is nullptr");
                return;
            }
            auto delRefCallbackInfo =  reinterpret_cast<DelRefCallbackInfo*>(work->data);
            if (delRefCallbackInfo == nullptr) {
                TAG_LOGE(AAFwkTag::FA, "uv_queue_work delRefCallbackInfo is nullptr");
                delete work;
                work = nullptr;
                return;
            }

            napi_delete_reference(delRefCallbackInfo->env_, delRefCallbackInfo->ref_);
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
            delete work;
            work = nullptr;
        }, uv_qos_user_initiated);
    if (ret != 0) {
        if (delRefCallbackInfo != nullptr) {
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    ref_ = nullptr;
}

void NAPIDataAbilityObserver::SetEnv(const napi_env &env)
{
    env_ = env;
    TAG_LOGI(AAFwkTag::FA, "NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

void NAPIDataAbilityObserver::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
    TAG_LOGI(AAFwkTag::FA, "NAPIDataAbilityObserver::%{public}s, called. end", __func__);
}

static void OnChangeJSThreadWorker(uv_work_t *work, int status)
{
    TAG_LOGI(AAFwkTag::FA, "OnChange, uv_queue_work");
    if (work == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "OnChange, uv_queue_work input work is nullptr");
        return;
    }
    DAHelperOnOffCB *onCB = (DAHelperOnOffCB *)work->data;
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "OnChange, uv_queue_work onCB is nullptr");
        delete work;
        work = nullptr;
        return;
    }

    if (onCB->observer != nullptr) {
        onCB->observer->CallJsMethod();
    }

    delete onCB;
    onCB = nullptr;
    delete work;
    work = nullptr;
    TAG_LOGI(AAFwkTag::FA, "OnChange, uv_queue_work. end");
}

void NAPIDataAbilityObserver::CallJsMethod()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ref_ == nullptr || env_ == nullptr) {
            TAG_LOGW(AAFwkTag::FA, "%{public}s observer is invalid.", __func__);
            return;
        }
        isCallingback_ = true;
    }
    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] = GetCallbackErrorValue(env_, NO_ERROR);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_get_undefined(env_, &undefined);
    napi_value callResult = nullptr;
    napi_get_reference_value(env_, ref_, &callback);
    napi_call_function(env_, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (needRelease_ && ref_ != nullptr) {
            TAG_LOGI(AAFwkTag::FA, "%{public}s to delete callback.", __func__);
            napi_delete_reference(env_, ref_);
            ref_ = nullptr;
            needRelease_ = false;
        }
        isCallingback_ = false;
    }
}

void NAPIDataAbilityObserver::OnChange()
{
    if (ref_ == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, OnChange ref is nullptr.", __func__);
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, loop is nullptr.", __func__);
        return;
    }

    uv_work_t *work = new uv_work_t;
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->observer = this;
    work->data = static_cast<void *>(onCB);
    int rev = uv_queue_work(
        loop,
        work,
        [](uv_work_t *work) {},
        OnChangeJSThreadWorker);
    if (rev != 0) {
        if (onCB != nullptr) {
            delete onCB;
            onCB = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s, called. end", __func__);
}

napi_value NAPI_GetType(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperGetTypeCB *gettypeCB = new (std::nothrow) DAHelperGetTypeCB;
    if (gettypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, gettypeCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    gettypeCB->cbBase.cbInfo.env = env;
    gettypeCB->cbBase.asyncWork = nullptr;
    gettypeCB->cbBase.deferred = nullptr;
    gettypeCB->cbBase.ability = nullptr;

    napi_value ret = GetTypeWrap(env, info, gettypeCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete gettypeCB;
        gettypeCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value GetTypeWrap(napi_env env, napi_callback_info info, DAHelperGetTypeCB *gettypeCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,start", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, error argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        gettypeCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, gettypeCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
    }
    GetDataAbilityHelper(env, thisVar, gettypeCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = GetTypeAsync(env, args, ARGS_ONE, gettypeCB);
    } else {
        ret = GetTypePromise(env, gettypeCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value GetTypeAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperGetTypeCB *gettypeCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || gettypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &gettypeCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetTypeExecuteCB,
            GetTypeAsyncCompleteCB,
            static_cast<void *>(gettypeCB),
            &gettypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, gettypeCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value GetTypePromise(napi_env env, DAHelperGetTypeCB *gettypeCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (gettypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    gettypeCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetTypeExecuteCB,
            GetTypePromiseCompleteCB,
            static_cast<void *>(gettypeCB),
            &gettypeCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, gettypeCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void GetTypeExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetType, worker pool thread execute.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    auto dataAbilityHelper = gettypeCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        gettypeCB->execResult = INVALID_PARAMETER;
        if (!gettypeCB->uri.empty()) {
            OHOS::Uri uri(gettypeCB->uri);
            gettypeCB->result = dataAbilityHelper->GetType(uri);
            gettypeCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "NAPI_GetType, dataAbilityHelper uri is empty.");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_GetType, dataAbilityHelper == nullptr.");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetType, worker pool thread execute end.");
}

void GetTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetType, main event thread complete.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, gettypeCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, gettypeCB->execResult);
    napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]);

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (gettypeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, gettypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetType, main event thread complete end.");
}

void GetTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetType,  main event thread complete.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, gettypeCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetType,  main event thread complete end.");
}

napi_value NAPI_GetFileTypes(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperGetFileTypesCB *getfiletypesCB = new (std::nothrow) DAHelperGetFileTypesCB;
    if (getfiletypesCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, getfiletypesCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    getfiletypesCB->cbBase.cbInfo.env = env;
    getfiletypesCB->cbBase.asyncWork = nullptr;
    getfiletypesCB->cbBase.deferred = nullptr;
    getfiletypesCB->cbBase.ability = nullptr;

    napi_value ret = GetFileTypesWrap(env, info, getfiletypesCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete getfiletypesCB;
        getfiletypesCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value GetFileTypesWrap(napi_env env, napi_callback_info info, DAHelperGetFileTypesCB *getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong arguments count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        getfiletypesCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, getfiletypesCB->uri.c_str());
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        getfiletypesCB->mimeTypeFilter = NapiValueToStringUtf8(env, args[PARAM1]);
        TAG_LOGI(
            AAFwkTag::FA, "%{public}s,mimeTypeFilter=%{public}s", __func__, getfiletypesCB->mimeTypeFilter.c_str());
    }
    GetDataAbilityHelper(env, thisVar, getfiletypesCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = GetFileTypesAsync(env, args, ARGS_TWO, getfiletypesCB);
    } else {
        ret = GetFileTypesPromise(env, getfiletypesCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}
napi_value GetFileTypesAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperGetFileTypesCB *getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || getfiletypesCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &getfiletypesCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetFileTypesExecuteCB,
            GetFileTypesAsyncCompleteCB,
            static_cast<void *>(getfiletypesCB),
            &getfiletypesCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, getfiletypesCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value GetFileTypesPromise(napi_env env, DAHelperGetFileTypesCB *getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (getfiletypesCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    getfiletypesCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            GetFileTypesExecuteCB,
            GetFileTypesPromiseCompleteCB,
            static_cast<void *>(getfiletypesCB),
            &getfiletypesCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, getfiletypesCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void GetFileTypesExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, worker pool thread execute.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    auto dataAbilityHelper = getfiletypesCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        getfiletypesCB->execResult = INVALID_PARAMETER;
        if (!getfiletypesCB->uri.empty()) {
            OHOS::Uri uri(getfiletypesCB->uri);
            TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, uri:%{public}s", uri.ToString().c_str());
            TAG_LOGI(
                AAFwkTag::FA, "NAPI_GetFileTypes, mimeTypeFilter:%{public}s", getfiletypesCB->mimeTypeFilter.c_str());
            getfiletypesCB->result = dataAbilityHelper->GetFileTypes(uri, getfiletypesCB->mimeTypeFilter);
            getfiletypesCB->execResult = NO_ERROR;
        } else {
            TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, dataAbilityHelper uri is empty.");
        }
    } else {
        TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, dataAbilityHelper == nullptr.");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, worker pool thread execute end.");
}

void GetFileTypesAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, main event thread complete.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, getfiletypesCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, getfiletypesCB->execResult);
    result[PARAM1] = WrapGetFileTypesCB(env, *getfiletypesCB);

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (getfiletypesCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, getfiletypesCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, main event thread complete end.");
}

napi_value WrapGetFileTypesCB(napi_env env, const DAHelperGetFileTypesCB &getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "WrapGetFileTypesCB, called.");
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes, result.size:%{public}zu", getfiletypesCB.result.size());
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        TAG_LOGI(
            AAFwkTag::FA, "NAPI_GetFileTypes, result[%{public}zu]:%{public}s", i, getfiletypesCB.result.at(i).c_str());
    }
    napi_value proValue = nullptr;

    napi_value jsArrayresult = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayresult));
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, getfiletypesCB.result.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayresult, i, proValue));
    }
    TAG_LOGI(AAFwkTag::FA, "WrapGetFileTypesCB, end.");
    return jsArrayresult;
}

void GetFileTypesPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes,  main event thread complete.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value result = nullptr;

    result = WrapGetFileTypesCB(env, *getfiletypesCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, getfiletypesCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_GetFileTypes,  main event thread complete end.");
}

napi_value NAPI_NormalizeUri(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperNormalizeUriCB *normalizeuriCB = new (std::nothrow) DAHelperNormalizeUriCB;
    if (normalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, normalizeuriCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    normalizeuriCB->cbBase.cbInfo.env = env;
    normalizeuriCB->cbBase.asyncWork = nullptr;
    normalizeuriCB->cbBase.deferred = nullptr;
    normalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = NormalizeUriWrap(env, info, normalizeuriCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete normalizeuriCB;
        normalizeuriCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value NormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperNormalizeUriCB *normalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,begin", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Error argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        normalizeuriCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, normalizeuriCB->uri.c_str());
    }
    GetDataAbilityHelper(env, thisVar, normalizeuriCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = NormalizeUriAsync(env, args, ARGS_ONE, normalizeuriCB);
    } else {
        ret = NormalizeUriPromise(env, normalizeuriCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}
napi_value NormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperNormalizeUriCB *normalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || normalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &normalizeuriCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NormalizeUriExecuteCB,
            NormalizeUriAsyncCompleteCB,
            static_cast<void *>(normalizeuriCB),
            &normalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, normalizeuriCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value NormalizeUriPromise(napi_env env, DAHelperNormalizeUriCB *normalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (normalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    normalizeuriCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            NormalizeUriExecuteCB,
            NormalizeUriPromiseCompleteCB,
            static_cast<void *>(normalizeuriCB),
            &normalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, normalizeuriCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void NormalizeUriExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri, worker pool thread execute.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    Uri uriValue(normalizeuriCB->uri);
    auto dataAbilityHelper = normalizeuriCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        normalizeuriCB->execResult = INVALID_PARAMETER;
        if (!normalizeuriCB->uri.empty()) {
        OHOS::Uri uri(normalizeuriCB->uri);
            uriValue = dataAbilityHelper->NormalizeUri(uri);
            normalizeuriCB->result = uriValue.ToString();
            normalizeuriCB->execResult = NO_ERROR;
        }
    } else {
        TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri, dataAbilityHelper == nullptr");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri, worker pool thread execute end.");
}

void NormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri, main event thread complete.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, normalizeuriCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, normalizeuriCB->execResult);
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (normalizeuriCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, normalizeuriCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri, main event thread complete end.");
}

void NormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri,  main event thread complete.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, normalizeuriCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_NormalizeUri,  main event thread complete end.");
}

napi_value NAPI_DenormalizeUri(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperDenormalizeUriCB *denormalizeuriCB = new (std::nothrow) DAHelperDenormalizeUriCB;
    if (denormalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, denormalizeuriCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    denormalizeuriCB->cbBase.cbInfo.env = env;
    denormalizeuriCB->cbBase.asyncWork = nullptr;
    denormalizeuriCB->cbBase.deferred = nullptr;
    denormalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = DenormalizeUriWrap(env, info, denormalizeuriCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete denormalizeuriCB;
        denormalizeuriCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value DenormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Parameter count error", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        denormalizeuriCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, denormalizeuriCB->uri.c_str());
    }
    GetDataAbilityHelper(env, thisVar, denormalizeuriCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = DenormalizeUriAsync(env, args, ARGS_ONE, denormalizeuriCB);
    } else {
        ret = DenormalizeUriPromise(env, denormalizeuriCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}
napi_value DenormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || denormalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &denormalizeuriCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DenormalizeUriExecuteCB,
            DenormalizeUriAsyncCompleteCB,
            static_cast<void *>(denormalizeuriCB),
            &denormalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, denormalizeuriCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value DenormalizeUriPromise(napi_env env, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (denormalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    denormalizeuriCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DenormalizeUriExecuteCB,
            DenormalizeUriPromiseCompleteCB,
            static_cast<void *>(denormalizeuriCB),
            &denormalizeuriCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, denormalizeuriCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void DenormalizeUriExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_DenormalizeUri, worker pool thread execute.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    Uri uriValue(denormalizeuriCB->uri);
    auto dataAbilityHelper = denormalizeuriCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        denormalizeuriCB->execResult = INVALID_PARAMETER;
        if (!denormalizeuriCB->uri.empty()) {
            OHOS::Uri uri(denormalizeuriCB->uri);
            uriValue = dataAbilityHelper->DenormalizeUri(uri);
            denormalizeuriCB->result = uriValue.ToString();
            denormalizeuriCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "NAPI_DenormalizeUri, dataAbilityHelper uri is empty");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_DenormalizeUri, dataAbilityHelper == nullptr");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_DenormalizeUri, worker pool thread execute end.");
}

void DenormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_DenormalizeUri, main event thread complete.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, denormalizeuriCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, denormalizeuriCB->execResult);
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result[PARAM1]));

    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (denormalizeuriCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, denormalizeuriCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_DenormalizeUri, main event thread complete end.");
}

void DenormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_DenormalizeUri,  main event thread complete.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, denormalizeuriCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_DenormalizeUri,  main event thread complete end.");
}

void UnwrapDataAbilityPredicates(NativeRdb::DataAbilityPredicates &predicates, napi_env env, napi_value value)
{
    auto tempPredicates = DataAbilityJsKit::DataAbilityPredicatesProxy::GetNativePredicates(env, value);
    if (tempPredicates == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "GetNativePredicates retval Marshalling failed.");
        return;
    }
    predicates = *tempPredicates;
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Delete(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperDeleteCB *deleteCB = new DAHelperDeleteCB;
    deleteCB->cbBase.cbInfo.env = env;
    deleteCB->cbBase.asyncWork = nullptr;
    deleteCB->cbBase.deferred = nullptr;
    deleteCB->cbBase.ability = nullptr;

    napi_value ret = DeleteWrap(env, info, deleteCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete deleteCB;
        deleteCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value DeleteWrap(napi_env env, napi_callback_info info, DAHelperDeleteCB *deleteCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Error argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        deleteCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, deleteCB->uri.c_str());
    }

    UnwrapDataAbilityPredicates(deleteCB->predicates, env, args[PARAM1]);
    GetDataAbilityHelper(env, thisVar, deleteCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = DeleteAsync(env, args, ARGS_TWO, deleteCB);
    } else {
        ret = DeletePromise(env, deleteCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value DeleteAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperDeleteCB *deleteCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || deleteCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &deleteCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DeleteExecuteCB,
            DeleteAsyncCompleteCB,
            static_cast<void *>(deleteCB),
            &deleteCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, deleteCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value DeletePromise(napi_env env, DAHelperDeleteCB *deleteCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (deleteCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    deleteCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            DeleteExecuteCB,
            DeletePromiseCompleteCB,
            static_cast<void *>(deleteCB),
            &deleteCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, deleteCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void DeleteExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Delete, worker pool thread execute.");
    DAHelperDeleteCB *deleteCB = static_cast<DAHelperDeleteCB *>(data);
    auto dataAbilityHelper = deleteCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        deleteCB->execResult = INVALID_PARAMETER;
        if (!deleteCB->uri.empty()) {
            OHOS::Uri uri(deleteCB->uri);
            deleteCB->result = dataAbilityHelper->Delete(uri, deleteCB->predicates);
            deleteCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "NAPI_Delete, dataAbilityHelper uri is empty");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_Delete, dataAbilityHelper == nullptr");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_Delete, worker pool thread execute end.");
}

void DeleteAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Delete, main event thread complete.");
    DAHelperDeleteCB *DeleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, DeleteCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, DeleteCB->execResult);
    napi_create_int32(env, DeleteCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (DeleteCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, DeleteCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, DeleteCB->cbBase.asyncWork));
    delete DeleteCB;
    DeleteCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_Delete, main event thread complete end.");
}

void DeletePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Delete,  main event thread complete.");
    DAHelperDeleteCB *DeleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, DeleteCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, DeleteCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, DeleteCB->cbBase.asyncWork));
    delete DeleteCB;
    DeleteCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_Delete,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Update(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperUpdateCB *updateCB = new DAHelperUpdateCB;
    updateCB->cbBase.cbInfo.env = env;
    updateCB->cbBase.asyncWork = nullptr;
    updateCB->cbBase.deferred = nullptr;
    updateCB->cbBase.ability = nullptr;

    napi_value ret = UpdateWrap(env, info, updateCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete updateCB;
        updateCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value UpdateWrap(napi_env env, napi_callback_info info, DAHelperUpdateCB *updateCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_FOUR;
    const size_t argcPromise = ARGS_THREE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Parameter count error.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        updateCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, updateCB->uri.c_str());
    }

    updateCB->valueBucket.Clear();
    AnalysisValuesBucket(updateCB->valueBucket, env, args[PARAM1]);
    UnwrapDataAbilityPredicates(updateCB->predicates, env, args[PARAM2]);
    GetDataAbilityHelper(env, thisVar, updateCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = UpdateAsync(env, args, ARGS_THREE, updateCB);
    } else {
        ret = UpdatePromise(env, updateCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value UpdateAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperUpdateCB *updateCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || updateCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &updateCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            UpdateExecuteCB,
            UpdateAsyncCompleteCB,
            static_cast<void *>(updateCB),
            &updateCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, updateCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value UpdatePromise(napi_env env, DAHelperUpdateCB *updateCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (updateCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    updateCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            UpdateExecuteCB,
            UpdatePromiseCompleteCB,
            static_cast<void *>(updateCB),
            &updateCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, updateCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void UpdateExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Update, worker pool thread execute.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    auto dataAbilityHelper = updateCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        updateCB->execResult = INVALID_PARAMETER;
        if (!updateCB->uri.empty()) {
            OHOS::Uri uri(updateCB->uri);
            updateCB->result = dataAbilityHelper->Update(uri, updateCB->valueBucket, updateCB->predicates);
            updateCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "NAPI_Update, dataAbilityHelper uri is empty");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_Update, dataAbilityHelper == nullptr");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_Update, worker pool thread execute end.");
}

void UpdateAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Update, main event thread complete.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, updateCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, updateCB->execResult);
    napi_create_int32(env, updateCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (updateCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, updateCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_Update, main event thread complete end.");
}

void UpdatePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Update,  main event thread complete.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, updateCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, updateCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_Update,  main event thread complete end.");
}

void CallErrorAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "CallErrorAsyncCompleteCB, main event thread complete.");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        napi_value callback = nullptr;
        napi_value undefined = nullptr;
        napi_value result[ARGS_TWO] = {nullptr};
        napi_value callResult = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
        NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, errorCB->cbBase.cbInfo.callback, &callback));

        napi_create_int32(env, errorCB->execResult, &result[PARAM0]);
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[0], &callResult));

        if (errorCB->cbBase.cbInfo.callback != nullptr) {
            NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, errorCB->cbBase.cbInfo.callback));
        }
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, errorCB->cbBase.asyncWork));
    }
    delete errorCB;
    errorCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "CallErrorAsyncCompleteCB, main event thread complete end.");
}

void CallErrorPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "CallErrorPromiseCompleteCB,  main event thread complete.");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        napi_value result = nullptr;
        napi_create_int32(env, errorCB->execResult, &result);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, errorCB->cbBase.deferred, result));
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, errorCB->cbBase.asyncWork));
    }
    delete errorCB;
    errorCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "CallErrorPromiseCompleteCB,  main event thread complete end.");
}

void CallErrorExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "CallErrorExecuteCB, worker pool thread execute.");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        errorCB->execResult = INVALID_PARAMETER;
    } else {
        TAG_LOGE(AAFwkTag::FA, "CallErrorExecuteCB, errorCB is null");
    }
    TAG_LOGI(AAFwkTag::FA, "CallErrorExecuteCB, worker pool thread execute end.");
}

napi_value CallErrorAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperErrorCB *errorCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || errorCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param or errorCB is null.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &errorCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, CallErrorExecuteCB, CallErrorAsyncCompleteCB,
                       static_cast<void *>(errorCB), &errorCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, errorCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value CallErrorPromise(napi_env env, DAHelperErrorCB *errorCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (errorCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param is null.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    errorCB->cbBase.deferred = deferred;

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, CallErrorExecuteCB, CallErrorPromiseCompleteCB,
                       static_cast<void *>(errorCB), &errorCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, errorCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

napi_value CallErrorWrap(napi_env env, napi_value thisVar, napi_callback_info info, napi_value *args, bool isPromise)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, called", __func__);
    DAHelperErrorCB *errorCB = new DAHelperErrorCB;
    errorCB->cbBase.cbInfo.env = env;
    errorCB->cbBase.asyncWork = nullptr;
    errorCB->cbBase.deferred = nullptr;
    errorCB->cbBase.ability = nullptr;
    napi_value ret = nullptr;
    if (!isPromise) {
        ret = CallErrorAsync(env, args, ARGS_FOUR, errorCB);
    } else {
        ret = CallErrorPromise(env, errorCB);
    }
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete errorCB;
        errorCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

void CallExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "CallExecuteCB, worker pool thread execute.");
    DAHelperCallCB *callCB = static_cast<DAHelperCallCB *>(data);
    auto dataAbilityHelper = callCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        callCB->execResult = INVALID_PARAMETER;
        if (!callCB->uri.empty()) {
            OHOS::Uri uri(callCB->uri);
            callCB->result = dataAbilityHelper->Call(uri, callCB->method, callCB->arg, callCB->pacMap);
            callCB->execResult = NO_ERROR;
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "CallExecuteCB, dataAbilityHelper == nullptr.");
    }
    TAG_LOGI(AAFwkTag::FA, "CallExecuteCB, worker pool thread execute end.");
}

static std::string ExcludeTag(const std::string& jsonString, const std::string& tagString)
{
    TAG_LOGD(AAFwkTag::FA, "enter");
    size_t pos = jsonString.find(tagString);
    if (pos == std::string::npos) {
        return jsonString;
    }
    std::string valueString = jsonString.substr(pos);
    pos = valueString.find(":");
    if (pos == std::string::npos) {
        return "";
    }
    size_t valuePos = pos + 1;
    while (valuePos < valueString.size()) {
        if (valueString.at(valuePos) != ' ' && valueString.at(valuePos) != '\t') {
            break;
        }
        valuePos++;
    }
    if (valuePos >= valueString.size()) {
        return "";
    }
    TAG_LOGD(AAFwkTag::FA, "exit");
    valueString = valueString.substr(valuePos);
    return valueString.substr(0, valueString.size() - 1);
}

napi_value CallPacMapValue(napi_env env, std::shared_ptr<AppExecFwk::PacMap> result)
{
    napi_value value = nullptr;

    NAPI_CALL(env, napi_create_object(env, &value));
    napi_value napiResult = nullptr;
    if (result != nullptr) {
        std::string resultWithoutTag = ExcludeTag(result->ToString(), "pacmap");
        napi_create_string_utf8(env, resultWithoutTag.c_str(), NAPI_AUTO_LENGTH, &napiResult);
        NAPI_CALL(env, napi_set_named_property(env, value, "result", napiResult));
    } else {
        TAG_LOGE(AAFwkTag::FA, "Return result is nullptr");
    }
    return value;
}

void CallAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "CallAsyncCompleteCB, main event thread complete.");
    DAHelperCallCB *callCB = static_cast<DAHelperCallCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, callCB->execResult);
    result[PARAM1] = CallPacMapValue(env, callCB->result);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (callCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, callCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callCB->cbBase.asyncWork));
    delete callCB;
    callCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "CallAsyncCompleteCB, main event thread complete end.");
}

void CallPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "CallPromiseCompleteCB, main event thread complete.");
    DAHelperCallCB *callCB = static_cast<DAHelperCallCB *>(data);
    napi_value result = nullptr;
    result = CallPacMapValue(env, callCB->result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, callCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callCB->cbBase.asyncWork));
    delete callCB;
    callCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "CallPromiseCompleteCB,  main event thread complete end.");
}

napi_value CallAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperCallCB *callCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || callCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &callCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            CallExecuteCB,
            CallAsyncCompleteCB,
            static_cast<void *>(callCB),
            &callCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end", __func__);
    return result;
}

napi_value CallPromise(napi_env env, DAHelperCallCB *callCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (callCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    callCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            CallExecuteCB,
            CallPromiseCompleteCB,
            static_cast<void *>(callCB),
            &callCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, callCB->cbBase.asyncWork));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end", __func__);
    return promise;
}

void SetPacMapObject(AppExecFwk::PacMap &pacMap, const napi_env &env, std::string keyStr, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        std::string valueString = UnwrapStringFromJS(env, value);
        pacMap.PutStringValue(keyStr, valueString);
    } else if (valueType == napi_number) {
        double valueNumber = 0;
        napi_get_value_double(env, value, &valueNumber);
        pacMap.PutDoubleValue(keyStr, valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, value, &valueBool);
        pacMap.PutBooleanValue(keyStr, valueBool);
    } else if (valueType == napi_null) {
        pacMap.PutObject(keyStr, nullptr);
    } else if (valueType == napi_object) {
        pacMap.PutStringValueArray(keyStr, ConvertStringVector(env, value));
    } else {
        TAG_LOGE(AAFwkTag::FA, "SetPacMapObject pacMap type error");
    }
}

void AnalysisPacMap(AppExecFwk::PacMap &pacMap, const napi_env &env, const napi_value &arg)
{
    napi_value keys = nullptr;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FA, "AnalysisPacMap errr");
        return;
    }
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key = nullptr;
        (void)napi_get_element(env, keys, i, &key);
        std::string keyStr = UnwrapStringFromJS(env, key);
        napi_value value = nullptr;
        napi_get_property(env, arg, key, &value);
        SetPacMapObject(pacMap, env, keyStr, value);
    }
}

/**
 * @brief Call processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param callCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value CallWrap(napi_env env, napi_callback_info info, DAHelperCallCB *callCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, called", __func__);
    size_t argcAsync = ARGS_FIVE;
    const size_t argcPromise = ARGS_FOUR;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync != ARGS_FOUR && argcAsync != ARGS_FIVE) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }
    bool isPromise = (argcAsync <= argcPromise) ? true : false;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        callCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
    } else {
        return CallErrorWrap(env, thisVar, info, args, isPromise);
    }
    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        callCB->method = NapiValueToStringUtf8(env, args[PARAM1]);
    } else {
        return CallErrorWrap(env, thisVar, info, args, isPromise);
    }
    NAPI_CALL(env, napi_typeof(env, args[PARAM2], &valuetype));
    if (valuetype == napi_string) {
        callCB->arg = NapiValueToStringUtf8(env, args[PARAM2]);
    }
    NAPI_CALL(env, napi_typeof(env, args[PARAM3], &valuetype));
    if (valuetype == napi_object) {
        AnalysisPacMap(callCB->pacMap, env, args[PARAM3]);
    }
    GetDataAbilityHelper(env, thisVar, callCB->dataAbilityHelper);
    if (!isPromise) {
        ret = CallAsync(env, args, ARGS_TWO, callCB);
    } else {
        ret = CallPromise(env, callCB);
    }
    return ret;
}

/**
 * @brief DataAbilityHelper NAPI method : call.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Call(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, called", __func__);
    DAHelperCallCB *callCB = new (std::nothrow) DAHelperCallCB;
    if (callCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, callCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    callCB->cbBase.cbInfo.env = env;
    callCB->cbBase.asyncWork = nullptr;
    callCB->cbBase.deferred = nullptr;
    callCB->cbBase.ability = nullptr;

    napi_value ret = CallWrap(env, info, callCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, ret == nullptr.", __func__);
        delete callCB;
        callCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s, called end", __func__);
    return ret;
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_OpenFile(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperOpenFileCB *openFileCB = new (std::nothrow) DAHelperOpenFileCB;
    if (openFileCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, openFileCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    openFileCB->cbBase.cbInfo.env = env;
    openFileCB->cbBase.asyncWork = nullptr;
    openFileCB->cbBase.deferred = nullptr;
    openFileCB->cbBase.ability = nullptr;

    napi_value ret = OpenFileWrap(env, info, openFileCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete openFileCB;
        openFileCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value OpenFileWrap(napi_env env, napi_callback_info info, DAHelperOpenFileCB *openFileCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, argument count error.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        openFileCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, openFileCB->uri.c_str());
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        openFileCB->mode = NapiValueToStringUtf8(env, args[PARAM1]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,mode=%{public}s", __func__, openFileCB->mode.c_str());
    }
    GetDataAbilityHelper(env, thisVar, openFileCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = OpenFileAsync(env, args, ARGS_TWO, openFileCB);
    } else {
        ret = OpenFilePromise(env, openFileCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value OpenFileAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperOpenFileCB *openFileCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || openFileCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &openFileCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            OpenFileExecuteCB,
            OpenFileAsyncCompleteCB,
            static_cast<void *>(openFileCB),
            &openFileCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, openFileCB->cbBase.asyncWork, napi_qos_user_initiated));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value OpenFilePromise(napi_env env, DAHelperOpenFileCB *openFileCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (openFileCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    openFileCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            OpenFileExecuteCB,
            OpenFilePromiseCompleteCB,
            static_cast<void *>(openFileCB),
            &openFileCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work_with_qos(env, openFileCB->cbBase.asyncWork, napi_qos_user_initiated));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void OpenFileExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_OpenFile, worker pool thread execute.");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    auto dataAbilityHelper = OpenFileCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        OpenFileCB->execResult = INVALID_PARAMETER;
        if (!OpenFileCB->uri.empty()) {
            OHOS::Uri uri(OpenFileCB->uri);
            OpenFileCB->result = dataAbilityHelper->OpenFile(uri, OpenFileCB->mode);
            OpenFileCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "NAPI_OpenFile, dataAbilityHelper uri is empty");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_OpenFile, dataAbilityHelper == nullptr");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_OpenFile, worker pool thread execute end.");
}

void OpenFileAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_OpenFile, main event thread complete.");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, OpenFileCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, OpenFileCB->execResult);
    napi_create_int32(env, OpenFileCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (OpenFileCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, OpenFileCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, OpenFileCB->cbBase.asyncWork));
    delete OpenFileCB;
    OpenFileCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_OpenFile, main event thread complete end.");
}

void OpenFilePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_OpenFileCB,  main event thread complete.");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, OpenFileCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, OpenFileCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, OpenFileCB->cbBase.asyncWork));
    delete OpenFileCB;
    OpenFileCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_OpenFileCB,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_BatchInsert(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    DAHelperBatchInsertCB *BatchInsertCB = new (std::nothrow) DAHelperBatchInsertCB;
    if (BatchInsertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, BatchInsertCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    BatchInsertCB->cbBase.cbInfo.env = env;
    BatchInsertCB->cbBase.asyncWork = nullptr;
    BatchInsertCB->cbBase.deferred = nullptr;
    BatchInsertCB->cbBase.ability = nullptr;

    napi_value ret = BatchInsertWrap(env, info, BatchInsertCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s,ret == nullptr", __func__);
        delete BatchInsertCB;
        BatchInsertCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

std::vector<NativeRdb::ValuesBucket> NapiValueObject(napi_env env, napi_value param)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    std::vector<NativeRdb::ValuesBucket> result;
    UnwrapArrayObjectFromJS(env, param, result);
    return result;
}

bool UnwrapArrayObjectFromJS(napi_env env, napi_value param, std::vector<NativeRdb::ValuesBucket> &value)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        TAG_LOGI(AAFwkTag::FA, "%{public}s, IsArrayForNapiValue is false", __func__);
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            TAG_LOGI(AAFwkTag::FA, "%{public}s, napi_get_element is false", __func__);
            return false;
        }

        NativeRdb::ValuesBucket valueBucket;
        valueBucket.Clear();
        AnalysisValuesBucket(valueBucket, env, jsValue);

        value.push_back(valueBucket);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return true;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value BatchInsertWrap(napi_env env, napi_callback_info info, DAHelperBatchInsertCB *batchInsertCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument count", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        batchInsertCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, batchInsertCB->uri.c_str());
    }

    batchInsertCB->values = NapiValueObject(env, args[PARAM1]);
    GetDataAbilityHelper(env, thisVar, batchInsertCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = BatchInsertAsync(env, args, ARGS_TWO, batchInsertCB);
    } else {
        ret = BatchInsertPromise(env, batchInsertCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value BatchInsertAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperBatchInsertCB *batchInsertCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback.", __func__);
    if (args == nullptr || batchInsertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &batchInsertCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            BatchInsertExecuteCB,
            BatchInsertAsyncCompleteCB,
            static_cast<void *>(batchInsertCB),
            &batchInsertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, batchInsertCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value BatchInsertPromise(napi_env env, DAHelperBatchInsertCB *batchInsertCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise.", __func__);
    if (batchInsertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param == nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    batchInsertCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            BatchInsertExecuteCB,
            BatchInsertPromiseCompleteCB,
            static_cast<void *>(batchInsertCB),
            &batchInsertCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, batchInsertCB->cbBase.asyncWork));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void BatchInsertExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_BatchInsert, worker pool thread execute.");
    DAHelperBatchInsertCB *batchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    auto dataAbilityHelper = batchInsertCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        batchInsertCB->execResult = INVALID_PARAMETER;
        if (!batchInsertCB->uri.empty()) {
            OHOS::Uri uri(batchInsertCB->uri);
            batchInsertCB->result = dataAbilityHelper->BatchInsert(uri, batchInsertCB->values);
            batchInsertCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "NAPI_BatchInsert, dataAbilityHelper uri is empyt");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "NAPI_BatchInsert, dataAbilityHelper == nullptr");
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_BatchInsert, worker pool thread execute end.");
}

void BatchInsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_BatchInsert, main event thread complete.");
    DAHelperBatchInsertCB *BatchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, BatchInsertCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, BatchInsertCB->execResult);
    napi_create_int32(env, BatchInsertCB->result, &result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (BatchInsertCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, BatchInsertCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, BatchInsertCB->cbBase.asyncWork));
    delete BatchInsertCB;
    BatchInsertCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_BatchInsert, main event thread complete end.");
}

void BatchInsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_BatchInsertCB,  main event thread complete.");
    DAHelperBatchInsertCB *BatchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, BatchInsertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, BatchInsertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, BatchInsertCB->cbBase.asyncWork));
    delete BatchInsertCB;
    BatchInsertCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "NAPI_BatchInsertCB,  main event thread complete end.");
}

/**
 * @brief DataAbilityHelper NAPI method : insert.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param info The callback info passed into the callback function.
 *
 * @return The return value from NAPI C++ to JS for the module.
 */
napi_value NAPI_Query(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "NAPI_Query called");
    DAHelperQueryCB *queryCB = new DAHelperQueryCB;
    queryCB->cbBase.cbInfo.env = env;
    queryCB->cbBase.asyncWork = nullptr;
    queryCB->cbBase.deferred = nullptr;
    queryCB->cbBase.ability = nullptr;

    napi_value ret = QueryWrap(env, info, queryCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "ret == nullptr");
        delete queryCB;
        queryCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "NAPI_Query end");
    return ret;
}

/**
 * @brief Insert processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value QueryWrap(napi_env env, napi_callback_info info, DAHelperQueryCB *queryCB)
{
    size_t argcAsync = ARGS_FOUR;
    const size_t argcPromise = ARGS_THREE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "Wrong argument count.");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        queryCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGD(AAFwkTag::FA, "uri=%{public}s", queryCB->uri.c_str());
    }

    std::vector<std::string> result;
    bool arrayStringbool = NapiValueToArrayStringUtf8(env, args[PARAM1], result);
    if (!arrayStringbool) {
        TAG_LOGE(AAFwkTag::FA, "The return value of arraystringbool is false");
    }
    queryCB->columns = result;

    UnwrapDataAbilityPredicates(queryCB->predicates, env, args[PARAM2]);
    GetDataAbilityHelper(env, thisVar, queryCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = QuerySync(env, args, ARGS_THREE, queryCB);
    } else {
        ret = QueryPromise(env, queryCB);
    }
    return ret;
}

napi_value QuerySync(napi_env env, napi_value *args, const size_t argCallback, DAHelperQueryCB *queryCB)
{
    TAG_LOGD(AAFwkTag::FA, "asyncCallback.");
    if (args == nullptr || queryCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "param == nullptr.");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &queryCB->cbBase.cbInfo.callback));
    }

    auto dataAbilityHelper = queryCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        queryCB->execResult = INVALID_PARAMETER;
        if (!queryCB->uri.empty()) {
            OHOS::Uri uri(queryCB->uri);
            auto resultset = dataAbilityHelper->Query(uri, queryCB->columns, queryCB->predicates);
            if (resultset != nullptr) {
                queryCB->result = resultset;
                queryCB->execResult = NO_ERROR;
            }
        }
    }

    napi_value callback = nullptr;
    NAPI_CALL(env, napi_get_reference_value(env, queryCB->cbBase.cbInfo.callback, &callback));
    napi_value result[ARGS_TWO] = {nullptr};
    result[PARAM0] = GetCallbackErrorValue(env, queryCB->execResult);
    result[PARAM1] = WrapResultSet(env, queryCB->result);
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    napi_value callResult = nullptr;
    NAPI_CALL(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (queryCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL(env, napi_delete_reference(env, queryCB->cbBase.cbInfo.callback));
    }
    delete queryCB;
    queryCB = nullptr;

    napi_value ret = nullptr;
    NAPI_CALL(env, napi_get_null(env, &ret));
    return ret;
}

napi_value QueryPromise(napi_env env, DAHelperQueryCB *queryCB)
{
    TAG_LOGD(AAFwkTag::FA, "promise.");
    if (queryCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "param == nullptr.");
        return nullptr;
    }

    auto dataAbilityHelper = queryCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        if (!queryCB->uri.empty()) {
            OHOS::Uri uri(queryCB->uri);
            auto resultset = dataAbilityHelper->Query(uri, queryCB->columns, queryCB->predicates);
            if (resultset != nullptr) {
                queryCB->result = resultset;
            }
        }
    }

    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    napi_value result = WrapResultSet(env, queryCB->result);
    NAPI_CALL(env, napi_resolve_deferred(env, deferred, result));
    delete queryCB;
    queryCB = nullptr;

    return promise;
}

napi_value WrapResultSet(napi_env env, const std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (resultSet == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "input parameter resultSet is nullptr");
        return WrapVoidToJS(env);
    }

    return RdbJsKit::ResultSetProxy::NewInstance(env, resultSet);
}

napi_value NAPI_ExecuteBatch(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,start", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = new (std::nothrow) DAHelperExecuteBatchCB;
    if (executeBatchCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, executeBatchCB == nullptr.", __func__);
        return WrapVoidToJS(env);
    }
    executeBatchCB->cbBase.cbInfo.env = env;
    executeBatchCB->cbBase.asyncWork = nullptr;
    executeBatchCB->cbBase.deferred = nullptr;
    executeBatchCB->cbBase.ability = nullptr;

    napi_value ret = ExecuteBatchWrap(env, info, executeBatchCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, ret == nullptr.", __func__);
        delete executeBatchCB;
        executeBatchCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

bool UnwrapArrayOperationFromJS(
    napi_env env, napi_callback_info info, napi_value param, std::vector<std::shared_ptr<DataAbilityOperation>> &result)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,called", __func__);
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type ", __func__);
        return false;
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s, param size:%{public}d ", __func__, arraySize);
    result.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            TAG_LOGE(AAFwkTag::FA, "%{public}s, get element failed, index:%{public}d ", __func__, i);
            return false;
        }
        std::shared_ptr<DataAbilityOperation> operation = nullptr;
        UnwrapDataAbilityOperation(operation, env, jsValue);
        TAG_LOGI(AAFwkTag::FA, "%{public}s, UnwrapDataAbilityOperation, index:%{public}d ", __func__, i);
        result.push_back(operation);
    }
    return true;
}

napi_value ExecuteBatchWrap(napi_env env, napi_callback_info info, DAHelperExecuteBatchCB *executeBatchCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,start", __func__);
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument count.", __func__);
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        executeBatchCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "%{public}s,uri=%{public}s", __func__, executeBatchCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, Wrong argument type.", __func__);
    }

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    UnwrapArrayOperationFromJS(env, info, args[PARAM1], operations);
    TAG_LOGI(AAFwkTag::FA, "%{public}s,operations size=%{public}zu", __func__, operations.size());
    executeBatchCB->operations = operations;
    GetDataAbilityHelper(env, thisVar, executeBatchCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = ExecuteBatchAsync(env, args, argcAsync, argcPromise, executeBatchCB);
    } else {
        ret = ExecuteBatchPromise(env, executeBatchCB);
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,end", __func__);
    return ret;
}

napi_value ExecuteBatchAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperExecuteBatchCB *executeBatchCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback start.", __func__);
    if (args == nullptr || executeBatchCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param is nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &executeBatchCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ExecuteBatchExecuteCB,
            ExecuteBatchAsyncCompleteCB,
            static_cast<void *>(executeBatchCB),
            &executeBatchCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, executeBatchCB->cbBase.asyncWork));
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_null(env, &result));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, asyncCallback end.", __func__);
    return result;
}

napi_value ExecuteBatchPromise(napi_env env, DAHelperExecuteBatchCB *executeBatchCB)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise start.", __func__);
    if (executeBatchCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "%{public}s, param is nullptr.", __func__);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = nullptr;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    executeBatchCB->cbBase.deferred = deferred;

    NAPI_CALL(env,
        napi_create_async_work(env,
            nullptr,
            resourceName,
            ExecuteBatchExecuteCB,
            ExecuteBatchPromiseCompleteCB,
            static_cast<void *>(executeBatchCB),
            &executeBatchCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, executeBatchCB->cbBase.asyncWork));
    TAG_LOGI(AAFwkTag::FA, "%{public}s, promise end.", __func__);
    return promise;
}

void ExecuteBatchExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s,NAPI_ExecuteBatch, worker pool thread execute start.", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    auto dataAbilityHelper = executeBatchCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        OHOS::Uri uri(executeBatchCB->uri);
        executeBatchCB->result = dataAbilityHelper->ExecuteBatch(uri, executeBatchCB->operations);
        TAG_LOGI(AAFwkTag::FA, "%{public}s, dataAbilityHelper is not nullptr. %{public}zu",
            __func__, executeBatchCB->result.size());
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s,NAPI_ExecuteBatch, worker pool thread execute end.", __func__);
}

void ExecuteBatchAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, main event thread complete start.", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value result[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, executeBatchCB->cbBase.cbInfo.callback, &callback));

    result[PARAM0] = GetCallbackErrorValue(env, NO_ERROR);
    napi_create_array(env, &result[PARAM1]);
    GetDataAbilityResultForResult(env, executeBatchCB->result, result[PARAM1]);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult));

    if (executeBatchCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, executeBatchCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeBatchCB->cbBase.asyncWork));
    delete executeBatchCB;
    executeBatchCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, main event thread complete end.", __func__);
}

void ExecuteBatchPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, main event thread complete start.", __func__);
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetDataAbilityResultForResult(env, executeBatchCB->result, result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, executeBatchCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeBatchCB->cbBase.asyncWork));
    delete executeBatchCB;
    executeBatchCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, main event thread complete end.", __func__);
}

void GetDataAbilityResultForResult(
    napi_env env, const std::vector<std::shared_ptr<DataAbilityResult>> &dataAbilityResult, napi_value result)
{
    TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, getDataAbilityResultForResult start. %{public}zu",
        __func__, dataAbilityResult.size());
    int32_t index = 0;
    std::vector<std::shared_ptr<DataAbilityResult>> entities = dataAbilityResult;
    for (const auto &item : entities) {
        napi_value objDataAbilityResult;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objDataAbilityResult));

        napi_value uri;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item->GetUri().ToString().c_str(), NAPI_AUTO_LENGTH, &uri));
        TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, uri = [%{public}s]", __func__,
            item->GetUri().ToString().c_str());
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objDataAbilityResult, "uri", uri));

        napi_value count;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item->GetCount(), &count));
        TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, count = [%{public}d]", __func__, item->GetCount());
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objDataAbilityResult, "count", count));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objDataAbilityResult));
        index++;
    }
    TAG_LOGI(AAFwkTag::FA, "%{public}s, NAPI_ExecuteBatch, getDataAbilityResultForResult end.", __func__);
}

void GetDataAbilityHelper(napi_env env, napi_value thisVar, std::shared_ptr<DataAbilityHelper>& dataAbilityHelper)
{
    NAPIDataAbilityHelperWrapper* wrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&wrapper));
    if (wrapper != nullptr) {
        dataAbilityHelper = wrapper->GetDataAbilityHelper();
    }
}

void EraseMemberProperties(DAHelperOnOffCB* onCB)
{
    if (onCB->observer) {
        TAG_LOGD(AAFwkTag::FA, "EraseMemberProperties, call ReleaseJSCallback");
        onCB->observer->ReleaseJSCallback();
    }
    auto dataAbilityHelper = onCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        TAG_LOGD(AAFwkTag::FA, "EraseMemberProperties, call Release");
        dataAbilityHelper->Release();
    }
}

bool NeedErase(std::vector<DAHelperOnOffCB*>::iterator& iter,
    const std::shared_ptr<DataAbilityHelper>&& dataAbilityHelper)
{
    if ((*iter) == nullptr) {
        return false;
    }
    if ((*iter)->dataAbilityHelper == dataAbilityHelper) {
        EraseMemberProperties(*iter);
        delete (*iter);
        (*iter) = nullptr;
        iter = g_registerInstances.erase(iter);
    } else {
        ++iter;
    }
    return true;
}

void DeleteDAHelperOnOffCB(DAHelperOnOffCB *onCB)
{
    if (!onCB) {
        TAG_LOGI(AAFwkTag::FA, "DeleteDAHelperOnOffCB, onCB is nullptr, no need delete");
        return;
    }
    EraseMemberProperties(onCB);

    auto end = remove(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (end != g_registerInstances.end()) {
        (void)g_registerInstances.erase(end);
    }
    delete onCB;
    onCB = nullptr;
}
}  // namespace AppExecFwk
}  // namespace OHOS
