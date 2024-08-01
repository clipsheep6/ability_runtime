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
#include <vector>
#include <string>

#include "data_ability_helper.h"
#include "napi_common_ability.h"
#include "data_ability_operation.h"
#include "data_ability_result.h"
#include "hilog_tag_wrapper.h"
#include "napi_base_context.h"
#include "napi_data_ability_helper_utils.h"
#include "napi_data_ability_observer.h"
#include "napi_data_ability_operation.h"
#include "napi_data_ability_predicates.h"
#include "napi_rdb_predicates.h"
#include "napi_result_set.h"

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
            TAG_LOGE(AAFwkTag::FA, "null ability");
            return nullptr;
        }
        std::string strUri = NapiValueToStringUtf8(env, argv[0]);
        TAG_LOGI(AAFwkTag::FA, "strUri=%{public}s", strUri.c_str());
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
                TAG_LOGW(AAFwkTag::FA, "null objectInfo");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperInsertCB *insertCB = new (std::nothrow) DAHelperInsertCB;
    if (insertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null insertCB");
        return WrapVoidToJS(env);
    }
    insertCB->cbBase.cbInfo.env = env;
    insertCB->cbBase.asyncWork = nullptr;
    insertCB->cbBase.deferred = nullptr;
    insertCB->cbBase.ability = nullptr;

    napi_value ret = InsertWrap(env, info, insertCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete insertCB;
        insertCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        insertCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", insertCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "wrong argument type");
    }

    insertCB->valueBucket.Clear();
    AnalysisValuesBucket(insertCB->valueBucket, env, args[PARAM1]);
    GetDataAbilityHelper(env, thisVar, insertCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = InsertAsync(env, args, ARGS_TWO, insertCB);
    } else {
        ret = InsertPromise(env, insertCB);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

void AnalysisValuesBucket(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, const napi_value &arg)
{
    napi_value keys = nullptr;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FA, "ValuesBucket error");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, args, &valueType);
    if (valueType != napi_object) {
        TAG_LOGE(AAFwkTag::FA, "valueType != napi_object");
        return nullptr;
    }

    std::string strValue = "";
    if (UnwrapStringByPropertyName(env, args, "value", strValue)) {
        TAG_LOGI(AAFwkTag::FA, "strValue=%{private}s", strValue.c_str());
        value = strValue;
    } else {
        TAG_LOGE(AAFwkTag::FA, "invalid value");
        return nullptr;
    }

    napi_value result;
    NAPI_CALL(env, napi_create_int32(env, 1, &result));
    TAG_LOGI(AAFwkTag::FA, "end");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperNotifyChangeCB *notifyChangeCB = new DAHelperNotifyChangeCB;
    notifyChangeCB->cbBase.cbInfo.env = env;
    notifyChangeCB->cbBase.asyncWork = nullptr;
    notifyChangeCB->cbBase.deferred = nullptr;
    notifyChangeCB->cbBase.ability = nullptr;

    napi_value ret = NotifyChangeWrap(env, info, notifyChangeCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete notifyChangeCB;
        notifyChangeCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        notifyChangeCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", notifyChangeCB->uri.c_str());
    }
    GetDataAbilityHelper(env, thisVar, notifyChangeCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = NotifyChangeAsync(env, args, argcAsync, argcPromise, notifyChangeCB);
    } else {
        ret = NotifyChangePromise(env, notifyChangeCB);
    }
    return ret;
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->cbBase.cbInfo.env = env;
    onCB->cbBase.asyncWork = nullptr;
    onCB->cbBase.deferred = nullptr;
    onCB->cbBase.ability = nullptr;

    napi_value ret = RegisterWrap(env, info, onCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete onCB;
        onCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    onCB->result = NO_ERROR;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        std::string type = NapiValueToStringUtf8(env, args[PARAM0]);
        if (type == "dataChange") {
            TAG_LOGI(AAFwkTag::FA, "type:%{public}s", type.c_str());
        } else {
            TAG_LOGE(AAFwkTag::FA, "error type: %{public}s", type.c_str());
            onCB->result = INVALID_PARAMETER;
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "wrong type");
        onCB->result = INVALID_PARAMETER;
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        onCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", onCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "wrong type");
        onCB->result = INVALID_PARAMETER;
    }
    GetDataAbilityHelper(env, thisVar, onCB->dataAbilityHelper);

    ret = RegisterAsync(env, args, argcAsync, argcPromise, onCB);
    return ret;
}

napi_value RegisterAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperOnOffCB *onCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || onCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (onCBIter == g_registerInstances.end()) {
        // onCB is invalid or onCB has been delete
        TAG_LOGE(AAFwkTag::FA, "invalid onCB");
        return;
    }

    auto dataAbilityHelper = onCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        if (onCB->result != INVALID_PARAMETER && !onCB->uri.empty() && onCB->cbBase.cbInfo.callback != nullptr) {
            OHOS::Uri uri(onCB->uri);
            dataAbilityHelper->RegisterObserver(uri, onCB->observer);
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri or null callback");
        }
    }
}

void RegisterCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "complete");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    if (onCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null onCB");
        return;
    }

    auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (onCBIter == g_registerInstances.end()) {
        // onCB is invalid or onCB has been delete
        TAG_LOGE(AAFwkTag::FA, "invalid onCB");
        return;
    }

    if (onCB->result == NO_ERROR) {
        return;
    }
    TAG_LOGI(AAFwkTag::FA, "input params onCB will be release");
    DeleteDAHelperOnOffCB(onCB);
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperOnOffCB *offCB = new DAHelperOnOffCB;
    offCB->cbBase.cbInfo.env = env;
    offCB->cbBase.asyncWork = nullptr;
    offCB->cbBase.deferred = nullptr;
    offCB->cbBase.ability = nullptr;

    napi_value ret = UnRegisterWrap(env, info, offCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete offCB;
        offCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argCountWithAsync = ARGS_TWO + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    offCB->result = NO_ERROR;
    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        std::string type = NapiValueToStringUtf8(env, args[PARAM0]);
        if (type == "dataChange") {
            TAG_LOGI(AAFwkTag::FA, "Wrong type=%{public}s", type.c_str());
        } else {
            TAG_LOGE(AAFwkTag::FA, "Wrong argument type %{public}s", type.c_str());
            offCB->result = INVALID_PARAMETER;
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "wrong argument type");
        offCB->result = INVALID_PARAMETER;
    }

    offCB->uri = "";
    if (argcAsync > ARGS_TWO) {
        // parse uri and callback
        NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", offCB->uri.c_str());
        } else {
            TAG_LOGE(AAFwkTag::FA, "Wrong argument type");
            offCB->result = INVALID_PARAMETER;
        }
        NAPI_CALL(env, napi_typeof(env, args[PARAM2], &valuetype));
        if (valuetype == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, args[PARAM2], 1, &offCB->cbBase.cbInfo.callback));
        } else {
            TAG_LOGE(AAFwkTag::FA, "Wrong argument type");
            offCB->result = INVALID_PARAMETER;
        }
    } else {
        // parse uri or callback
        NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", offCB->uri.c_str());
        } else if (valuetype == napi_function) {
            NAPI_CALL(env, napi_create_reference(env, args[PARAM1], 1, &offCB->cbBase.cbInfo.callback));
        } else {
            TAG_LOGE(AAFwkTag::FA, "Wrong argument type");
            offCB->result = INVALID_PARAMETER;
        }
    }
    GetDataAbilityHelper(env, thisVar, offCB->dataAbilityHelper);

    ret = UnRegisterSync(env, offCB);
    return ret;
}

napi_value UnRegisterSync(napi_env env, DAHelperOnOffCB *offCB)
{
    TAG_LOGI(AAFwkTag::FA, "syncCallback");
    if (offCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null offCB");
        return nullptr;
    }
    napi_value resourceName = nullptr;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    if (offCB->result == NO_ERROR) {
        FindRegisterObs(env, offCB);
    }

    TAG_LOGI(AAFwkTag::FA, "notifyList size: %{public}zu", offCB->NotifyList.size());
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

    TAG_LOGI(AAFwkTag::FA, "offCB->DestroyList size is %{public}zu", offCB->DestroyList.size());
    for (auto &iter : offCB->DestroyList) {
        if (iter->observer != nullptr) {
            iter->observer->ReleaseJSCallback();
            delete iter;
            iter = nullptr;
            TAG_LOGI(AAFwkTag::FA, "ReleaseJSCallback");
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
    TAG_LOGI(AAFwkTag::FA, "execute");
    if (data == nullptr || data->dataAbilityHelper == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
        return;
    }

    TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", data->uri.c_str());
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
            TAG_LOGI(AAFwkTag::FA, "Instances erase size = %{public}zu", g_registerInstances.size());
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null uri");
    }
    TAG_LOGI(AAFwkTag::FA, "execute end %{public}zu",
        data->NotifyList.size());
}
napi_value NAPI_GetType(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperGetTypeCB *gettypeCB = new (std::nothrow) DAHelperGetTypeCB;
    if (gettypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null gettypeCB");
        return WrapVoidToJS(env);
    }
    gettypeCB->cbBase.cbInfo.env = env;
    gettypeCB->cbBase.asyncWork = nullptr;
    gettypeCB->cbBase.deferred = nullptr;
    gettypeCB->cbBase.ability = nullptr;

    napi_value ret = GetTypeWrap(env, info, gettypeCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete gettypeCB;
        gettypeCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    return ret;
}

napi_value GetTypeWrap(napi_env env, napi_callback_info info, DAHelperGetTypeCB *gettypeCB)
{
    TAG_LOGI(AAFwkTag::FA, "start");
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        gettypeCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", gettypeCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "not string");
    }
    GetDataAbilityHelper(env, thisVar, gettypeCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = GetTypeAsync(env, args, ARGS_ONE, gettypeCB);
    } else {
        ret = GetTypePromise(env, gettypeCB);
    }
    return ret;
}

napi_value NAPI_GetFileTypes(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperGetFileTypesCB *getfiletypesCB = new (std::nothrow) DAHelperGetFileTypesCB;
    if (getfiletypesCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null getfiletypesCB");
        return WrapVoidToJS(env);
    }
    getfiletypesCB->cbBase.cbInfo.env = env;
    getfiletypesCB->cbBase.asyncWork = nullptr;
    getfiletypesCB->cbBase.deferred = nullptr;
    getfiletypesCB->cbBase.ability = nullptr;

    napi_value ret = GetFileTypesWrap(env, info, getfiletypesCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete getfiletypesCB;
        getfiletypesCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    return ret;
}

napi_value GetFileTypesWrap(napi_env env, napi_callback_info info, DAHelperGetFileTypesCB *getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        getfiletypesCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", getfiletypesCB->uri.c_str());
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        getfiletypesCB->mimeTypeFilter = NapiValueToStringUtf8(env, args[PARAM1]);
        TAG_LOGI(
            AAFwkTag::FA, "mimeTypeFilter=%{public}s", getfiletypesCB->mimeTypeFilter.c_str());
    }
    GetDataAbilityHelper(env, thisVar, getfiletypesCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = GetFileTypesAsync(env, args, ARGS_TWO, getfiletypesCB);
    } else {
        ret = GetFileTypesPromise(env, getfiletypesCB);
    }

    return ret;
}

napi_value NAPI_NormalizeUri(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperNormalizeUriCB *normalizeuriCB = new (std::nothrow) DAHelperNormalizeUriCB;
    if (normalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null normalizeuriCB");
        return WrapVoidToJS(env);
    }
    normalizeuriCB->cbBase.cbInfo.env = env;
    normalizeuriCB->cbBase.asyncWork = nullptr;
    normalizeuriCB->cbBase.deferred = nullptr;
    normalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = NormalizeUriWrap(env, info, normalizeuriCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete normalizeuriCB;
        normalizeuriCB = nullptr;
        ret = WrapVoidToJS(env);
    }

    return ret;
}

napi_value NormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperNormalizeUriCB *normalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "begin");
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        normalizeuriCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", normalizeuriCB->uri.c_str());
    }
    GetDataAbilityHelper(env, thisVar, normalizeuriCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = NormalizeUriAsync(env, args, ARGS_ONE, normalizeuriCB);
    } else {
        ret = NormalizeUriPromise(env, normalizeuriCB);
    }

    return ret;
}

napi_value NAPI_DenormalizeUri(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperDenormalizeUriCB *denormalizeuriCB = new (std::nothrow) DAHelperDenormalizeUriCB;
    if (denormalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null denormalizeuriCB");
        return WrapVoidToJS(env);
    }
    denormalizeuriCB->cbBase.cbInfo.env = env;
    denormalizeuriCB->cbBase.asyncWork = nullptr;
    denormalizeuriCB->cbBase.deferred = nullptr;
    denormalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = DenormalizeUriWrap(env, info, denormalizeuriCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete denormalizeuriCB;
        denormalizeuriCB = nullptr;
        ret = WrapVoidToJS(env);
    }

    return ret;
}

napi_value DenormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_TWO;
    const size_t argcPromise = ARGS_ONE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        denormalizeuriCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", denormalizeuriCB->uri.c_str());
    }
    GetDataAbilityHelper(env, thisVar, denormalizeuriCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = DenormalizeUriAsync(env, args, ARGS_ONE, denormalizeuriCB);
    } else {
        ret = DenormalizeUriPromise(env, denormalizeuriCB);
    }

    return ret;
}

void UnwrapDataAbilityPredicates(NativeRdb::DataAbilityPredicates &predicates, napi_env env, napi_value value)
{
    auto tempPredicates = DataAbilityJsKit::DataAbilityPredicatesProxy::GetNativePredicates(env, value);
    if (tempPredicates == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null tempPredicates");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperDeleteCB *deleteCB = new DAHelperDeleteCB;
    deleteCB->cbBase.cbInfo.env = env;
    deleteCB->cbBase.asyncWork = nullptr;
    deleteCB->cbBase.deferred = nullptr;
    deleteCB->cbBase.ability = nullptr;

    napi_value ret = DeleteWrap(env, info, deleteCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete deleteCB;
        deleteCB = nullptr;
        ret = WrapVoidToJS(env);
    }

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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        deleteCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", deleteCB->uri.c_str());
    }

    UnwrapDataAbilityPredicates(deleteCB->predicates, env, args[PARAM1]);
    GetDataAbilityHelper(env, thisVar, deleteCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = DeleteAsync(env, args, ARGS_TWO, deleteCB);
    } else {
        ret = DeletePromise(env, deleteCB);
    }

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
napi_value NAPI_Update(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperUpdateCB *updateCB = new DAHelperUpdateCB;
    updateCB->cbBase.cbInfo.env = env;
    updateCB->cbBase.asyncWork = nullptr;
    updateCB->cbBase.deferred = nullptr;
    updateCB->cbBase.ability = nullptr;

    napi_value ret = UpdateWrap(env, info, updateCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete updateCB;
        updateCB = nullptr;
        ret = WrapVoidToJS(env);
    }

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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_FOUR;
    const size_t argcPromise = ARGS_THREE;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        updateCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", updateCB->uri.c_str());
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

    return ret;
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
        TAG_LOGE(AAFwkTag::FA, "pacMap type error");
    }
}

void AnalysisPacMap(AppExecFwk::PacMap &pacMap, const napi_env &env, const napi_value &arg)
{
    napi_value keys = nullptr;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FA, "status err");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_FIVE;
    const size_t argcPromise = ARGS_FOUR;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync != ARGS_FOUR && argcAsync != ARGS_FIVE) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperCallCB *callCB = new (std::nothrow) DAHelperCallCB;
    if (callCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null callCB");
        return WrapVoidToJS(env);
    }
    callCB->cbBase.cbInfo.env = env;
    callCB->cbBase.asyncWork = nullptr;
    callCB->cbBase.deferred = nullptr;
    callCB->cbBase.ability = nullptr;

    napi_value ret = CallWrap(env, info, callCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete callCB;
        callCB = nullptr;
        ret = WrapVoidToJS(env);
    }

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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperOpenFileCB *openFileCB = new (std::nothrow) DAHelperOpenFileCB;
    if (openFileCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null openFileCB");
        return WrapVoidToJS(env);
    }
    openFileCB->cbBase.cbInfo.env = env;
    openFileCB->cbBase.asyncWork = nullptr;
    openFileCB->cbBase.deferred = nullptr;
    openFileCB->cbBase.ability = nullptr;

    napi_value ret = OpenFileWrap(env, info, openFileCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete openFileCB;
        openFileCB = nullptr;
        ret = WrapVoidToJS(env);
    }

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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        openFileCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", openFileCB->uri.c_str());
    }

    NAPI_CALL(env, napi_typeof(env, args[PARAM1], &valuetype));
    if (valuetype == napi_string) {
        openFileCB->mode = NapiValueToStringUtf8(env, args[PARAM1]);
        TAG_LOGI(AAFwkTag::FA, "mode=%{public}s", openFileCB->mode.c_str());
    }
    GetDataAbilityHelper(env, thisVar, openFileCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = OpenFileAsync(env, args, ARGS_TWO, openFileCB);
    } else {
        ret = OpenFilePromise(env, openFileCB);
    }

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
napi_value NAPI_BatchInsert(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperBatchInsertCB *BatchInsertCB = new (std::nothrow) DAHelperBatchInsertCB;
    if (BatchInsertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null BatchInsertCB");
        return WrapVoidToJS(env);
    }
    BatchInsertCB->cbBase.cbInfo.env = env;
    BatchInsertCB->cbBase.asyncWork = nullptr;
    BatchInsertCB->cbBase.deferred = nullptr;
    BatchInsertCB->cbBase.ability = nullptr;

    napi_value ret = BatchInsertWrap(env, info, BatchInsertCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete BatchInsertCB;
        BatchInsertCB = nullptr;
        ret = WrapVoidToJS(env);
    }

    return ret;
}

std::vector<NativeRdb::ValuesBucket> NapiValueObject(napi_env env, napi_value param)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    std::vector<NativeRdb::ValuesBucket> result;
    UnwrapArrayObjectFromJS(env, param, result);
    return result;
}

bool UnwrapArrayObjectFromJS(napi_env env, napi_value param, std::vector<NativeRdb::ValuesBucket> &value)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        TAG_LOGI(AAFwkTag::FA, "IsArrayForNapiValue:false");
        return false;
    }

    value.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            TAG_LOGI(AAFwkTag::FA, "get jsValue failed");
            return false;
        }

        NativeRdb::ValuesBucket valueBucket;
        valueBucket.Clear();
        AnalysisValuesBucket(valueBucket, env, jsValue);

        value.push_back(valueBucket);
    }

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
    TAG_LOGI(AAFwkTag::FA, "called");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        batchInsertCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", batchInsertCB->uri.c_str());
    }

    batchInsertCB->values = NapiValueObject(env, args[PARAM1]);
    GetDataAbilityHelper(env, thisVar, batchInsertCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = BatchInsertAsync(env, args, ARGS_TWO, batchInsertCB);
    } else {
        ret = BatchInsertPromise(env, batchInsertCB);
    }

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
napi_value NAPI_Query(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperQueryCB *queryCB = new DAHelperQueryCB;
    queryCB->cbBase.cbInfo.env = env;
    queryCB->cbBase.asyncWork = nullptr;
    queryCB->cbBase.deferred = nullptr;
    queryCB->cbBase.ability = nullptr;

    napi_value ret = QueryWrap(env, info, queryCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete queryCB;
        queryCB = nullptr;
        ret = WrapVoidToJS(env);
    }

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
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
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

napi_value NAPI_ExecuteBatch(napi_env env, napi_callback_info info)
{
    TAG_LOGI(AAFwkTag::FA, "start");
    DAHelperExecuteBatchCB *executeBatchCB = new (std::nothrow) DAHelperExecuteBatchCB;
    if (executeBatchCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "invalid executeBatchCB");
        return WrapVoidToJS(env);
    }
    executeBatchCB->cbBase.cbInfo.env = env;
    executeBatchCB->cbBase.asyncWork = nullptr;
    executeBatchCB->cbBase.deferred = nullptr;
    executeBatchCB->cbBase.ability = nullptr;

    napi_value ret = ExecuteBatchWrap(env, info, executeBatchCB);
    if (ret == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete executeBatchCB;
        executeBatchCB = nullptr;
        ret = WrapVoidToJS(env);
    }

    return ret;
}

bool UnwrapArrayOperationFromJS(
    napi_env env, napi_callback_info info, napi_value param, std::vector<std::shared_ptr<DataAbilityOperation>> &result)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    uint32_t arraySize = 0;
    napi_value jsValue = nullptr;
    std::string strValue = "";

    if (!IsArrayForNapiValue(env, param, arraySize)) {
        TAG_LOGE(AAFwkTag::FA, "Wrong argument type");
        return false;
    }
    TAG_LOGI(AAFwkTag::FA, "param size:%{public}d", arraySize);
    result.clear();
    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        if (napi_get_element(env, param, i, &jsValue) != napi_ok) {
            TAG_LOGE(AAFwkTag::FA, "get index:%{public}d failed", i);
            return false;
        }
        std::shared_ptr<DataAbilityOperation> operation = nullptr;
        UnwrapDataAbilityOperation(operation, env, jsValue);
        TAG_LOGI(AAFwkTag::FA, "UnwrapDataAbilityOperation index:%{public}d", i);
        result.push_back(operation);
    }
    return true;
}

napi_value ExecuteBatchWrap(napi_env env, napi_callback_info info, DAHelperExecuteBatchCB *executeBatchCB)
{
    TAG_LOGI(AAFwkTag::FA, "start");
    size_t argcAsync = ARGS_THREE;
    const size_t argcPromise = ARGS_TWO;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = nullptr;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, &thisVar, nullptr));
    if (argcAsync > argCountWithAsync || argcAsync > ARGS_MAX_COUNT) {
        TAG_LOGE(AAFwkTag::FA, "invalid argc");
        return nullptr;
    }

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[PARAM0], &valuetype));
    if (valuetype == napi_string) {
        executeBatchCB->uri = NapiValueToStringUtf8(env, args[PARAM0]);
        TAG_LOGI(AAFwkTag::FA, "uri=%{public}s", executeBatchCB->uri.c_str());
    } else {
        TAG_LOGE(AAFwkTag::FA, "Wrong argument type");
    }

    std::vector<std::shared_ptr<DataAbilityOperation>> operations;
    UnwrapArrayOperationFromJS(env, info, args[PARAM1], operations);
    TAG_LOGI(AAFwkTag::FA, "operations size=%{public}zu", operations.size());
    executeBatchCB->operations = operations;
    GetDataAbilityHelper(env, thisVar, executeBatchCB->dataAbilityHelper);

    if (argcAsync > argcPromise) {
        ret = ExecuteBatchAsync(env, args, argcAsync, argcPromise, executeBatchCB);
    } else {
        ret = ExecuteBatchPromise(env, executeBatchCB);
    }

    return ret;
}


void EraseMemberProperties(DAHelperOnOffCB* onCB)
{
    if (onCB->observer) {
        TAG_LOGD(AAFwkTag::FA, "call ReleaseJSCallback");
        onCB->observer->ReleaseJSCallback();
    }
    auto dataAbilityHelper = onCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        TAG_LOGD(AAFwkTag::FA, "call Release");
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
        TAG_LOGI(AAFwkTag::FA, "null onCB");
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
