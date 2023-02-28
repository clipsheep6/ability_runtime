/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "js_data_ability_helper.h"

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
namespace AbilityRuntime {
const int32_t JS_ERR_PARAM_INVALID = 401;        // Invalid input parameter
const int32_t JS_ERR_INTERNAL_ERROR = 16500001;  // System internal error

const std::unordered_map<int32_t, std::string> ERR_CODE_MSG = {
    {JS_ERR_PARAM_INVALID, "Invalid input parameter"},
    {JS_ERR_INTERNAL_ERROR, "System internal error"}
};
std::list<std::shared_ptr<DataAbilityHelper>> g_dataAbilityHelperList;
std::vector<DAHelperOnOffCB *> g_registerInstances;

static void OnChangeJSThreadWorker(uv_work_t *work, int status)
{
    HILOG_INFO("OnChange, uv_queue_work.");
    if (work == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work input work is nullptr.");
        return;
    }
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(work->data);
    if (onCB == nullptr) {
        HILOG_ERROR("OnChange, uv_queue_work onCB is nullptr.");
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
    HILOG_INFO("OnChange, uv_queue_work end.");
}

void NAPIDataAbilityObserver::ReleaseJSCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ref_ == nullptr) {
        HILOG_ERROR("ref_ is nullptr.");
        return;
    }

    if (isCallingback_) {
        needRelease_ = true;
        HILOG_WARN("ref_ is calling back.");
        return;
    }

    SafeReleaseJSCallback();
    HILOG_INFO("End.");
}

void NAPIDataAbilityObserver::SafeReleaseJSCallback()
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("loop is nullptr.");
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
    int ret = uv_queue_work(
        loop, work, [](uv_work_t* work) {},
        [](uv_work_t* work, int status) {
            // JS Thread
            if (work == nullptr) {
                HILOG_ERROR("work is nullptr.");
                return;
            }
            auto delRefCallbackInfo =  reinterpret_cast<DelRefCallbackInfo*>(work->data);
            if (delRefCallbackInfo == nullptr) {
                HILOG_ERROR("delRefCallbackInfo is nullptr.");
                delete work;
                work = nullptr;
                return;
            }

            napi_delete_reference(delRefCallbackInfo->env_, delRefCallbackInfo->ref_);
            delete delRefCallbackInfo;
            delRefCallbackInfo = nullptr;
            delete work;
            work = nullptr;
        });
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
    HILOG_INFO("End.");
}

void NAPIDataAbilityObserver::SetCallbackRef(const napi_ref &ref)
{
    ref_ = ref;
    HILOG_INFO("End.");
}

void NAPIDataAbilityObserver::CallJsMethod()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ref_ == nullptr || env_ == nullptr) {
            HILOG_WARN("ref_ or env_ is nullptr.");
            return;
        }
        isCallingback_ = true;
    }
    napi_value result[ARGS_TWO] = {0};
    result[PARAM0] = GetCallbackErrorValue(env_, NO_ERROR);
    napi_value callback = 0;
    napi_value undefined = 0;
    napi_get_undefined(env_, &undefined);
    napi_value callResult = 0;
    napi_get_reference_value(env_, ref_, &callback);
    napi_call_function(env_, undefined, callback, ARGS_TWO, &result[PARAM0], &callResult);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (needRelease_ && ref_ != nullptr) {
            HILOG_INFO("Delete callback.");
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
        HILOG_ERROR("ref_ is nullptr.");
        return;
    }
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        HILOG_ERROR("loop is nullptr.");
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
    HILOG_INFO("End.");
}

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
    HILOG_INFO("Enter.");
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
    g_dataAbilityHelperList.clear();
    return exports;
}

napi_value DataAbilityHelperConstructor(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Enter.");
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
        HILOG_INFO("argv[0] is not a context.");
        auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
        if (ability == nullptr) {
            HILOG_ERROR("Failed to get native context instance.");
            return nullptr;
        }
        std::string strUri = NapiValueToStringUtf8(env, argv[0]);
        HILOG_INFO("FA Model: strUri = %{public}s.", strUri.c_str());
        dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
    } else {
        HILOG_INFO("argv[0] is a context.");
        if (stageMode) {
            auto context = OHOS::AbilityRuntime::GetStageModeContext(env, argv[0]);
            if (context == nullptr) {
                HILOG_ERROR("Failed to get native context instance.");
                return nullptr;
            }
            std::string strUri = NapiValueToStringUtf8(env, argv[PARAM1]);
            HILOG_INFO("Stage Model: strUri = %{public}s.", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(context, std::make_shared<Uri>(strUri));
        } else {
            auto ability = OHOS::AbilityRuntime::GetCurrentAbility(env);
            if (ability == nullptr) {
                HILOG_ERROR("Failed to get native context instance.");
                return nullptr;
            }
            std::string strUri = NapiValueToStringUtf8(env, argv[PARAM1]);
            HILOG_INFO("FA Model: strUri = %{public}s.", strUri.c_str());
            dataAbilityHelper = DataAbilityHelper::Creator(ability->GetContext(), std::make_shared<Uri>(strUri));
        }
    }

    if (dataAbilityHelper == nullptr) {
        HILOG_INFO("dataAbilityHelper is nullptr.");
        dataAbilityHelperStatus = false;
        return nullptr;
    }
    dataAbilityHelper->SetCallFromJs();
    g_dataAbilityHelperList.emplace_back(dataAbilityHelper);
    HILOG_INFO("dataAbilityHelperList size = %{public}zu.", g_dataAbilityHelperList.size());

    napi_wrap(
        env,
        thisVar,
        dataAbilityHelper.get(),
        [](napi_env env, void *data, void *hint) {
            DataAbilityHelper *objectInfo = static_cast<DataAbilityHelper *>(data);
            HILOG_INFO("g_registerInstances size = %{public}zu.", g_registerInstances.size());
            for (auto iter = g_registerInstances.begin(); iter != g_registerInstances.end();) {
                if (!NeedErase(iter, objectInfo)) {
                    iter = g_registerInstances.erase(iter);
                }
            }
            HILOG_INFO("g_registerInstances size = %{public}zu.", g_registerInstances.size());
            g_dataAbilityHelperList.remove_if(
                [objectInfo](const std::shared_ptr<DataAbilityHelper> &dataAbilityHelper) {
                    return objectInfo == dataAbilityHelper.get();
                });
            HILOG_INFO("g_dataAbilityHelperList size = %{public}zu.", g_dataAbilityHelperList.size());
        },
        nullptr,
        nullptr);

    dataAbilityHelperStatus = true;
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    DAHelperInsertCB *insertCB = new DAHelperInsertCB;
    insertCB->cbBase.cbInfo.env = env;
    insertCB->cbBase.asyncWork = nullptr;
    insertCB->cbBase.deferred = nullptr;
    insertCB->cbBase.ability = nullptr;

    napi_value ret = InsertWrap(env, info, insertCB);
    if (ret == nullptr) {
        HILOG_ERROR("InsertWrap failed.");
        delete insertCB;
        insertCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_TWO;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], insertCB->uri)) {
        return nullptr;
    }
    insertCB->valueBucket.Clear();
    if (!CheckValuesBucket(env, args[PARAM1], insertCB->valueBucket)) {
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    insertCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = InsertAsync(env, args, ARGS_TWO, insertCB);
    } else {
        ret = InsertPromise(env, insertCB);
    }
    HILOG_INFO("End.");
    return ret;
}

bool AnalysisValuesBucket(NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, const napi_value &arg)
{
    napi_value keys = 0;
    napi_get_property_names(env, arg, &keys);
    uint32_t arrLen = 0;
    napi_status status = napi_get_array_length(env, keys, &arrLen);
    if (status != napi_ok) {
        HILOG_ERROR("ValuesBucket error.");
        return false;
    }
    HILOG_INFO("ValuesBucket num:%{public}zu.", arrLen);
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value key = 0;
        (void)napi_get_element(env, keys, i, &key);
        std::string keyStr = UnwrapStringFromJS(env, key);
        napi_value value = 0;
        napi_get_property(env, arg, key, &value);

        if (!SetValuesBucketObject(valuesBucket, env, keyStr, value)) {
            HILOG_ERROR("Set values bucket object error.");
            return false;
        }
    }
    return true;
}

bool SetValuesBucketObject(
    NativeRdb::ValuesBucket &valuesBucket, const napi_env &env, std::string keyStr, napi_value value)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    if (valueType == napi_string) {
        std::string valueString = UnwrapStringFromJS(env, value);
        HILOG_INFO("ValueObject type: %{public}d, key: %{public}s, value:  %{private}s.",
            valueType, keyStr.c_str(), valueString.c_str());
        valuesBucket.PutString(keyStr, valueString);
    } else if (valueType == napi_number) {
        double valueNumber = 0;
        napi_get_value_double(env, value, &valueNumber);
        valuesBucket.PutDouble(keyStr, valueNumber);
        HILOG_INFO("ValueObject type: %{public}d, key: %{public}s, value: %{private}lf.",
            valueType, keyStr.c_str(), valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, value, &valueBool);
        HILOG_INFO("ValueObject type: %{public}d, key:  %{public}s, value: %{private}d.",
            valueType, keyStr.c_str(), valueBool);
        valuesBucket.PutBool(keyStr, valueBool);
    } else if (valueType == napi_null) {
        valuesBucket.PutNull(keyStr);
        HILOG_INFO("ValueObject type: %{public}d, key: %{public}s, value: null.", valueType, keyStr.c_str());
    } else if (valueType == napi_object) {
        HILOG_INFO("ValueObject type: %{public}d, key: %{public}s, value: Uint8Array.", valueType, keyStr.c_str());
        valuesBucket.PutBlob(keyStr, ConvertU8Vector(env, value));
    } else {
        HILOG_ERROR("Invalid value type.");
        return false;
    }
    return true;
}
napi_value InsertAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperInsertCB *insertCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || insertCB == nullptr) {
        HILOG_ERROR("Input Param args or insertCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, insertCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value InsertPromise(napi_env env, DAHelperInsertCB *insertCB)
{
    HILOG_INFO("Enter.");
    if (insertCB == nullptr) {
        HILOG_ERROR("Input Param insertCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, insertCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void InsertExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    if (insertCB->dataAbilityHelper != nullptr) {
        insertCB->execResult = INVALID_PARAMETER;
        if (!insertCB->uri.empty()) {
            OHOS::Uri uri(insertCB->uri);
            auto ret = insertCB->dataAbilityHelper->Insert(uri, insertCB->valueBucket);
            if (ret != -1) {
                // success
                insertCB->execResult = NO_ERROR;
                insertCB->result = ret;
            } else {
                // fail
                insertCB->execResult = ret;
            }
        }
    } else {
        HILOG_ERROR("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void InsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, insertCB->result, &result);
    CreateCallBackValue(env, insertCB->cbBase.cbInfo.callback, insertCB->execResult, result);
    if (insertCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, insertCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void InsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, insertCB->result, &result);
    CreatePromiseValue(env, insertCB->cbBase.deferred, insertCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
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
    HILOG_INFO("Enter.");
    DAHelperNotifyChangeCB *notifyChangeCB = new DAHelperNotifyChangeCB;
    notifyChangeCB->cbBase.cbInfo.env = env;
    notifyChangeCB->cbBase.asyncWork = nullptr;
    notifyChangeCB->cbBase.deferred = nullptr;
    notifyChangeCB->cbBase.ability = nullptr;

    napi_value ret = NotifyChangeWrap(env, info, notifyChangeCB);
    if (ret == nullptr) {
        HILOG_ERROR("NotifyChangeWrap failed.");
        delete notifyChangeCB;
        notifyChangeCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_ONE;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], notifyChangeCB->uri)) {
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    notifyChangeCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = NotifyChangeAsync(env, args, argCount, promiseArgCount, notifyChangeCB);
    } else {
        ret = NotifyChangePromise(env, notifyChangeCB);
    }
    return ret;
}

napi_value NotifyChangeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperNotifyChangeCB *notifyChangeCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || notifyChangeCB == nullptr) {
        HILOG_ERROR("Input Param args or notifyChangeCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

napi_value NotifyChangePromise(napi_env env, DAHelperNotifyChangeCB *notifyChangeCB)
{
    HILOG_INFO("Enter.");
    if (notifyChangeCB == nullptr) {
        HILOG_ERROR("Input Param notifyChangeCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    HILOG_INFO("Worker pool thread execute.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    if (notifyChangeCB->dataAbilityHelper != nullptr) {
        notifyChangeCB->execResult = INVALID_PARAMETER;
        if (!notifyChangeCB->uri.empty()) {
            OHOS::Uri uri(notifyChangeCB->uri);
            notifyChangeCB->dataAbilityHelper->NotifyChange(uri);
            notifyChangeCB->execResult = NO_ERROR;
        } else {
            HILOG_ERROR("notifyChangeCB uri is empty.");
        }
    }
}

void NotifyChangeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value callback = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, notifyChangeCB->cbBase.cbInfo.callback, &callback));
    if (!IsTypeForNapiValue(env, callback, napi_function)) {
        delete notifyChangeCB;
        notifyChangeCB = nullptr;
        HILOG_INFO("Invalid callback.");
        return;
    }
    CreateCallBackValue(env, notifyChangeCB->cbBase.cbInfo.callback, notifyChangeCB->execResult);
    if (notifyChangeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, notifyChangeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, notifyChangeCB->cbBase.asyncWork));
    delete notifyChangeCB;
    notifyChangeCB = nullptr;
}

void NotifyChangePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, 0, &result);
    CreatePromiseValue(env, notifyChangeCB->cbBase.deferred, notifyChangeCB->execResult, result);
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
    HILOG_INFO("Enter.");
    DAHelperOnOffCB *onCB = new DAHelperOnOffCB;
    onCB->cbBase.cbInfo.env = env;
    onCB->cbBase.asyncWork = nullptr;
    onCB->cbBase.deferred = nullptr;
    onCB->cbBase.ability = nullptr;

    napi_value ret = RegisterWrap(env, info, onCB);
    if (ret == nullptr) {
        HILOG_ERROR("Register wrap failed.");
        delete onCB;
        onCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_TWO;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckNotifyType(env, args[PARAM0])) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM1], onCB->uri)) {
        return nullptr;
    }

    onCB->result = NO_ERROR;
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    onCB->dataAbilityHelper = objectInfo;

    ret = RegisterAsync(env, args, argCount, promiseArgCount, onCB);
    return ret;
}

napi_value RegisterAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperOnOffCB *onCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || onCB == nullptr) {
        HILOG_ERROR("Input Param args or onCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argcPromise], &valuetype));
    if (valuetype != napi_function) {
        HILOG_ERROR("Invalid callback.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }

    HILOG_INFO("valuetype is napi_function.");
    NAPI_CALL(env, napi_create_reference(env, args[argcPromise], 1, &onCB->cbBase.cbInfo.callback));

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
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

void RegisterExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (onCBIter == g_registerInstances.end()) {
        // onCB is invalid or onCB has been delete
        HILOG_ERROR("Input params onCB is invalid.");
        return;
    }

    if (onCB->dataAbilityHelper != nullptr) {
        if (onCB->result != INVALID_PARAMETER && !onCB->uri.empty() && onCB->cbBase.cbInfo.callback != nullptr) {
            OHOS::Uri uri(onCB->uri);
            onCB->dataAbilityHelper->RegisterObserver(uri, onCB->observer);
        } else {
            HILOG_ERROR("Uri is empty or callback is nullptr.");
        }
    }
}

void RegisterCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperOnOffCB *onCB = static_cast<DAHelperOnOffCB *>(data);
    if (onCB == nullptr) {
        HILOG_ERROR("Input params onCB is nullptr.");
        return;
    }

    auto onCBIter = std::find(g_registerInstances.begin(), g_registerInstances.end(), onCB);
    if (onCBIter == g_registerInstances.end()) {
        // onCB is invalid or onCB has been delete
        HILOG_ERROR("Input params onCB is invalid.");
        return;
    }

    if (onCB->result == NO_ERROR) {
        return;
    }
    HILOG_INFO("Input params onCB will be release");
    DeleteDAHelperOnOffCB(onCB);
    HILOG_INFO("Main event thread complete over an release invalid onCB.");
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
    HILOG_INFO("Enter.");
    DAHelperOnOffCB *offCB = new DAHelperOnOffCB;
    offCB->cbBase.cbInfo.env = env;
    offCB->cbBase.asyncWork = nullptr;
    offCB->cbBase.deferred = nullptr;
    offCB->cbBase.ability = nullptr;

    napi_value ret = UnRegisterWrap(env, info, offCB);
    if (ret == nullptr) {
        HILOG_ERROR("UnRegister wrap failed.");
        delete offCB;
        offCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_TWO;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckNotifyType(env, args[PARAM0])) {
        return nullptr;
    }
    if (!CheckUnregisterParam(env, args, argCount, offCB)) {
        return nullptr;
    }

    offCB->result = NO_ERROR;
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    offCB->dataAbilityHelper = objectInfo;

    ret = UnRegisterAsync(env, offCB);
    return ret;
}

napi_value UnRegisterAsync(napi_env env, DAHelperOnOffCB *offCB)
{
    HILOG_INFO("Enter.");
    if (offCB == nullptr) {
        HILOG_ERROR("Input Param offCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    if (offCB->result == NO_ERROR) {
        FindRegisterObs(env, offCB);
    }

    NAPI_CALL(env,
        napi_create_async_work(
            env,
            nullptr,
            resourceName,
            UnRegisterExecuteCB,
            UnRegisterCompleteCB,
            static_cast<void *>(offCB),
            &offCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, offCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    return result;
}

void UnRegisterExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Enter.");
    DAHelperOnOffCB *offCB = static_cast<DAHelperOnOffCB *>(data);
    if (offCB == nullptr || offCB->dataAbilityHelper == nullptr) {
        HILOG_ERROR("Input param is invalid.");
        if (offCB != nullptr) {
            delete offCB;
            offCB = nullptr;
        }
        return;
    }
    HILOG_INFO("offCB->NotifyList size is %{public}zu.", offCB->NotifyList.size());
    for (auto &iter : offCB->NotifyList) {
        if (iter != nullptr && iter->observer != nullptr) {
            OHOS::Uri uri(iter->uri);
            iter->dataAbilityHelper->UnregisterObserver(uri, iter->observer);
            offCB->DestroyList.emplace_back(iter);
        }
    }
    offCB->NotifyList.clear();
    HILOG_INFO("Main event thread execute end.");
}

void UnRegisterCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    // cannot run it in executeCB, because need to use napi_strict_equals compare callbacks.
    DAHelperOnOffCB *offCB = static_cast<DAHelperOnOffCB *>(data);
    if (offCB == nullptr || offCB->dataAbilityHelper == nullptr) {
        HILOG_ERROR("Input param is invalid.");
        if (offCB != nullptr) {
            delete offCB;
            offCB = nullptr;
        }
        return;
    }
    HILOG_INFO("offCB->DestroyList size is %{public}zu.", offCB->DestroyList.size());
    for (auto &iter : offCB->DestroyList) {
        HILOG_INFO("ReleaseJSCallback. 1 ---");
        if (iter->observer != nullptr) {
            iter->observer->ReleaseJSCallback();
            delete iter;
            iter = nullptr;
            HILOG_INFO("ReleaseJSCallback. 2---");
        }
    }

    offCB->DestroyList.clear();
    delete offCB;
    offCB = nullptr;

    HILOG_INFO("Main event thread complete end.");
}

void FindRegisterObs(napi_env env, DAHelperOnOffCB *data)
{
    HILOG_INFO("Enter.");
    if (data == nullptr || data->dataAbilityHelper == nullptr) {
        HILOG_ERROR("Input Param is invalid.");
        return;
    }

    HILOG_INFO("uri = %{public}s.", data->uri.c_str());
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
            HILOG_INFO("Instances erase size = %{public}zu", g_registerInstances.size());
        }
    } else {
        HILOG_ERROR("error: uri is empty.");
    }
    HILOG_INFO("End. %{public}zu", data->NotifyList.size());
}

napi_value NAPI_GetType(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Enter.");
    DAHelperGetTypeCB *gettypeCB = new DAHelperGetTypeCB;
    gettypeCB->cbBase.cbInfo.env = env;
    gettypeCB->cbBase.asyncWork = nullptr;
    gettypeCB->cbBase.deferred = nullptr;
    gettypeCB->cbBase.ability = nullptr;

    napi_value ret = GetTypeWrap(env, info, gettypeCB);
    if (ret == nullptr) {
        HILOG_ERROR("GetTypeWrap failed.");
        delete gettypeCB;
        gettypeCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value GetTypeWrap(napi_env env, napi_callback_info info, DAHelperGetTypeCB *gettypeCB)
{
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_ONE;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], gettypeCB->uri)) {
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    gettypeCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = GetTypeAsync(env, args, ARGS_ONE, gettypeCB);
    } else {
        ret = GetTypePromise(env, gettypeCB);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value GetTypeAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperGetTypeCB *gettypeCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || gettypeCB == nullptr) {
        HILOG_ERROR("Input Param args or gettypeCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, gettypeCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value GetTypePromise(napi_env env, DAHelperGetTypeCB *gettypeCB)
{
    HILOG_INFO("Enter.");
    if (gettypeCB == nullptr) {
        HILOG_ERROR("Input Param gettypeCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, gettypeCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void GetTypeExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    if (gettypeCB->dataAbilityHelper != nullptr) {
        gettypeCB->execResult = INVALID_PARAMETER;
        if (!gettypeCB->uri.empty()) {
            OHOS::Uri uri(gettypeCB->uri);
            auto ret = gettypeCB->dataAbilityHelper->GetType(uri);
            if (!ret.empty()) {
                // success
                gettypeCB->result = ret;
                gettypeCB->execResult = NO_ERROR;
            }
        } else {
            HILOG_ERROR("dataAbilityHelper uri is empty.");
        }
    } else {
        HILOG_ERROR("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void GetTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value result = nullptr;
    napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result);
    CreateCallBackValue(env, gettypeCB->cbBase.cbInfo.callback,  gettypeCB->execResult, result);

    if (gettypeCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, gettypeCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void GetTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    CreatePromiseValue(env, gettypeCB->cbBase.deferred, gettypeCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

napi_value NAPI_GetFileTypes(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Enter.");
    DAHelperGetFileTypesCB *getfiletypesCB = new DAHelperGetFileTypesCB;
    getfiletypesCB->cbBase.cbInfo.env = env;
    getfiletypesCB->cbBase.asyncWork = nullptr;
    getfiletypesCB->cbBase.deferred = nullptr;
    getfiletypesCB->cbBase.ability = nullptr;

    napi_value ret = GetFileTypesWrap(env, info, getfiletypesCB);
    if (ret == nullptr) {
        HILOG_ERROR("GetFileTypesWrap failed.");
        delete getfiletypesCB;
        getfiletypesCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value GetFileTypesWrap(napi_env env, napi_callback_info info, DAHelperGetFileTypesCB *getfiletypesCB)
{
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_TWO;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], getfiletypesCB->uri)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM1], getfiletypesCB->mimeTypeFilter)) {
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    getfiletypesCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = GetFileTypesAsync(env, args, ARGS_TWO, getfiletypesCB);
    } else {
        ret = GetFileTypesPromise(env, getfiletypesCB);
    }
    HILOG_INFO("End.");
    return ret;
}
napi_value GetFileTypesAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperGetFileTypesCB *getfiletypesCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || getfiletypesCB == nullptr) {
        HILOG_ERROR("Input Param args or getfiletypesCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, getfiletypesCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value GetFileTypesPromise(napi_env env, DAHelperGetFileTypesCB *getfiletypesCB)
{
    HILOG_INFO("Enter.");
    if (getfiletypesCB == nullptr) {
        HILOG_ERROR("Input Param getfiletypesCB is nullptr.");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, getfiletypesCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void GetFileTypesExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    if (getfiletypesCB->dataAbilityHelper != nullptr) {
        getfiletypesCB->execResult = INVALID_PARAMETER;
        if (!getfiletypesCB->uri.empty()) {
            OHOS::Uri uri(getfiletypesCB->uri);
            HILOG_INFO("uri: %{public}s", uri.ToString().c_str());
            HILOG_INFO("mimeTypeFilter: %{public}s", getfiletypesCB->mimeTypeFilter.c_str());
            auto ret = getfiletypesCB->dataAbilityHelper->GetFileTypes(uri, getfiletypesCB->mimeTypeFilter);
            if (!ret.empty()) {
                // success
                getfiletypesCB->result = ret;
                getfiletypesCB->execResult = NO_ERROR;
            }
        } else {
            HILOG_INFO("dataAbilityHelper uri is empty.");
        }
    } else {
        HILOG_INFO("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void GetFileTypesAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value result = WrapGetFileTypesCB(env, *getfiletypesCB);
    CreateCallBackValue(env, getfiletypesCB->cbBase.cbInfo.callback, getfiletypesCB->execResult, result);
    if (getfiletypesCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, getfiletypesCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

napi_value WrapGetFileTypesCB(napi_env env, const DAHelperGetFileTypesCB &getfiletypesCB)
{
    HILOG_INFO("Enter. result.size:%{public}zu.", getfiletypesCB.result.size());
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        HILOG_INFO("result[%{public}zu]: %{public}s.", i, getfiletypesCB.result.at(i).c_str());
    }
    napi_value proValue = nullptr;

    napi_value jsArrayresult = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayresult));
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, getfiletypesCB.result.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayresult, i, proValue));
    }
    HILOG_INFO("End.");
    return jsArrayresult;
}

void GetFileTypesPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value result = nullptr;

    result = WrapGetFileTypesCB(env, *getfiletypesCB);
    CreatePromiseValue(env, getfiletypesCB->cbBase.deferred, getfiletypesCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

napi_value NAPI_NormalizeUri(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Enter.");
    DAHelperNormalizeUriCB *normalizeuriCB = new DAHelperNormalizeUriCB;
    normalizeuriCB->cbBase.cbInfo.env = env;
    normalizeuriCB->cbBase.asyncWork = nullptr;
    normalizeuriCB->cbBase.deferred = nullptr;
    normalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = NormalizeUriWrap(env, info, normalizeuriCB);
    if (ret == nullptr) {
        HILOG_ERROR("NormalizeUriWrap failed.");
        delete normalizeuriCB;
        normalizeuriCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value NormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperNormalizeUriCB *normalizeuriCB)
{
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_ONE;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], normalizeuriCB->uri)) {
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    normalizeuriCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = NormalizeUriAsync(env, args, ARGS_ONE, normalizeuriCB);
    } else {
        ret = NormalizeUriPromise(env, normalizeuriCB);
    }
    HILOG_INFO("End.");
    return ret;
}
napi_value NormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperNormalizeUriCB *normalizeuriCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || normalizeuriCB == nullptr) {
        HILOG_ERROR("Input Param args or normalizeuriCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, normalizeuriCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value NormalizeUriPromise(napi_env env, DAHelperNormalizeUriCB *normalizeuriCB)
{
    HILOG_INFO("Enter.");
    if (normalizeuriCB == nullptr) {
        HILOG_ERROR("Input Param normalizeuriCB is nullptr.");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, normalizeuriCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void NormalizeUriExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    if (normalizeuriCB->dataAbilityHelper != nullptr) {
        normalizeuriCB->execResult = INVALID_PARAMETER;
        if (!normalizeuriCB->uri.empty()) {
            OHOS::Uri uri(normalizeuriCB->uri);
            auto uriValue = normalizeuriCB->dataAbilityHelper->NormalizeUri(uri);
            if (!uriValue.ToString().empty()) {
                // success
                normalizeuriCB->result = uriValue.ToString();
                normalizeuriCB->execResult = NO_ERROR;
            }
        }
    } else {
        HILOG_INFO("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void NormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    CreateCallBackValue(env, normalizeuriCB->cbBase.cbInfo.callback,  normalizeuriCB->execResult, result);
    if (normalizeuriCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, normalizeuriCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void NormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    CreatePromiseValue(env, normalizeuriCB->cbBase.deferred, normalizeuriCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

napi_value NAPI_DenormalizeUri(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Enter.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = new DAHelperDenormalizeUriCB;
    denormalizeuriCB->cbBase.cbInfo.env = env;
    denormalizeuriCB->cbBase.asyncWork = nullptr;
    denormalizeuriCB->cbBase.deferred = nullptr;
    denormalizeuriCB->cbBase.ability = nullptr;

    napi_value ret = DenormalizeUriWrap(env, info, denormalizeuriCB);
    if (ret == nullptr) {
        HILOG_ERROR("DenormalizeUriWrap failed.");
        delete denormalizeuriCB;
        denormalizeuriCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value DenormalizeUriWrap(napi_env env, napi_callback_info info, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_ONE;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], denormalizeuriCB->uri)) {
        return nullptr;
    }

    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    denormalizeuriCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = DenormalizeUriAsync(env, args, ARGS_ONE, denormalizeuriCB);
    } else {
        ret = DenormalizeUriPromise(env, denormalizeuriCB);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value DenormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || denormalizeuriCB == nullptr) {
        HILOG_ERROR("Input Param args or denormalizeuriCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value DenormalizeUriPromise(napi_env env, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    HILOG_INFO("Enter.");
    if (denormalizeuriCB == nullptr) {
        HILOG_ERROR("Input Param args or denormalizeuriCB is nullptr.");
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void DenormalizeUriExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    if (denormalizeuriCB->dataAbilityHelper != nullptr) {
        denormalizeuriCB->execResult = INVALID_PARAMETER;
        if (!denormalizeuriCB->uri.empty()) {
            OHOS::Uri uri(denormalizeuriCB->uri);
            auto uriValue = denormalizeuriCB->dataAbilityHelper->DenormalizeUri(uri);
            if (!uriValue.ToString().empty()) {
                // success
                denormalizeuriCB->result = uriValue.ToString();
                denormalizeuriCB->execResult = NO_ERROR;
            }
        } else {
            HILOG_ERROR("dataAbilityHelper uri is empty.");
        }
    } else {
        HILOG_ERROR("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void DenormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    CreateCallBackValue(env, denormalizeuriCB->cbBase.cbInfo.callback,  denormalizeuriCB->execResult, result);
    if (denormalizeuriCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, denormalizeuriCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void DenormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    CreatePromiseValue(env, denormalizeuriCB->cbBase.deferred, denormalizeuriCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void UnwrapDataAbilityPredicates(NativeRdb::DataAbilityPredicates &predicates, napi_env env, napi_value value)
{
    auto tempPredicates = DataAbilityJsKit::DataAbilityPredicatesProxy::GetNativePredicates(env, value);
    if (tempPredicates == nullptr) {
        HILOG_ERROR("GetNativePredicates retval Marshalling failed.");
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
    HILOG_INFO("Enter.");
    DAHelperDeleteCB *deleteCB = new DAHelperDeleteCB;
    deleteCB->cbBase.cbInfo.env = env;
    deleteCB->cbBase.asyncWork = nullptr;
    deleteCB->cbBase.deferred = nullptr;
    deleteCB->cbBase.ability = nullptr;

    napi_value ret = DeleteWrap(env, info, deleteCB);
    if (ret == nullptr) {
        HILOG_ERROR("DeleteWrap failed.");
        delete deleteCB;
        deleteCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_TWO;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], deleteCB->uri)) {
        return nullptr;
    }

    UnwrapDataAbilityPredicates(deleteCB->predicates, env, args[PARAM1]);
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    deleteCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = DeleteAsync(env, args, ARGS_TWO, deleteCB);
    } else {
        ret = DeletePromise(env, deleteCB);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value DeleteAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperDeleteCB *deleteCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || deleteCB == nullptr) {
        HILOG_ERROR("Input Param args or deleteCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, deleteCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value DeletePromise(napi_env env, DAHelperDeleteCB *deleteCB)
{
    HILOG_INFO("Enter.");
    if (deleteCB == nullptr) {
        HILOG_ERROR("Input deleteCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, deleteCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void DeleteExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperDeleteCB *deleteCB = static_cast<DAHelperDeleteCB *>(data);
    if (deleteCB->dataAbilityHelper != nullptr) {
        deleteCB->execResult = INVALID_PARAMETER;
        if (!deleteCB->uri.empty()) {
            OHOS::Uri uri(deleteCB->uri);
            auto ret = deleteCB->dataAbilityHelper->Delete(uri, deleteCB->predicates);
            if (ret != -1) {
                // success
                deleteCB->execResult = NO_ERROR;
                deleteCB->result = ret;
            } else {
                // fail
                deleteCB->execResult = ret;
            }
        } else {
            HILOG_ERROR("dataAbilityHelper uri is empty.");
        }
    } else {
        HILOG_ERROR("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void DeleteAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperDeleteCB *deleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, deleteCB->result, &result);
    CreateCallBackValue(env, deleteCB->cbBase.cbInfo.callback,  deleteCB->execResult, result);
    if (deleteCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, deleteCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, deleteCB->cbBase.asyncWork));
    delete deleteCB;
    deleteCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void DeletePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperDeleteCB *deleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, deleteCB->result, &result);
    CreatePromiseValue(env, deleteCB->cbBase.deferred, deleteCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, deleteCB->cbBase.asyncWork));
    delete deleteCB;
    deleteCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
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
    HILOG_INFO("Enter.");
    DAHelperUpdateCB *updateCB = new DAHelperUpdateCB;
    updateCB->cbBase.cbInfo.env = env;
    updateCB->cbBase.asyncWork = nullptr;
    updateCB->cbBase.deferred = nullptr;
    updateCB->cbBase.ability = nullptr;

    napi_value ret = UpdateWrap(env, info, updateCB);
    if (ret == nullptr) {
        HILOG_ERROR("UpdateWrap failed.");
        delete updateCB;
        updateCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
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
    HILOG_INFO("Enter.");
    size_t argCount = ARGS_MAX_COUNT;
    const size_t promiseArgCount = ARGS_THREE;
    const size_t asyncArgCount = promiseArgCount + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = {nullptr};
    napi_value ret = 0;
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argCount, args, &thisVar, nullptr));
    if (!CheckArgCount(env, argCount, promiseArgCount)) {
        return nullptr;
    }
    if (!CheckStringParam(env, args[PARAM0], updateCB->uri)) {
        return nullptr;
    }
    updateCB->valueBucket.Clear();
    if (!CheckValuesBucket(env, args[PARAM1], updateCB->valueBucket)) {
        return nullptr;
    }

    UnwrapDataAbilityPredicates(updateCB->predicates, env, args[PARAM2]);
    DataAbilityHelper *objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void **)&objectInfo);
    updateCB->dataAbilityHelper = objectInfo;

    if (argCount >= asyncArgCount) {
        ret = UpdateAsync(env, args, ARGS_THREE, updateCB);
    } else {
        ret = UpdatePromise(env, updateCB);
    }
    HILOG_INFO("End.");
    return ret;
}

napi_value UpdateAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperUpdateCB *updateCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || updateCB == nullptr) {
        HILOG_ERROR("Input Param args or updateCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, updateCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value UpdatePromise(napi_env env, DAHelperUpdateCB *updateCB)
{
    HILOG_INFO("Enter.");
    if (updateCB == nullptr) {
        HILOG_ERROR("Input Param updateCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
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
    NAPI_CALL(env, napi_queue_async_work(env, updateCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

void UpdateExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    if (updateCB->dataAbilityHelper != nullptr) {
        updateCB->execResult = INVALID_PARAMETER;
        if (!updateCB->uri.empty()) {
            OHOS::Uri uri(updateCB->uri);
            auto ret = updateCB->dataAbilityHelper->Update(uri, updateCB->valueBucket, updateCB->predicates);
            if (ret != -1) {
                // success
                updateCB->execResult = NO_ERROR;
                updateCB->result = ret;
            } else {
                // fail
                updateCB->execResult = ret;
            }
        } else {
            HILOG_ERROR("dataAbilityHelper uri is empty.");
        }
    } else {
        HILOG_ERROR("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

void UpdateAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, updateCB->result, &result);
    CreateCallBackValue(env, updateCB->cbBase.cbInfo.callback, updateCB->execResult, result);
    if (updateCB->cbBase.cbInfo.callback != nullptr) {
        NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, updateCB->cbBase.cbInfo.callback));
    }
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void UpdatePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, updateCB->result, &result);
    CreatePromiseValue(env, updateCB->cbBase.deferred, updateCB->execResult, result);
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void CallErrorAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        napi_value result = nullptr;
        napi_create_int32(env, errorCB->execResult, &result);
        CreateCallBackValue(env, errorCB->cbBase.cbInfo.callback, errorCB->execResult, result);
        if (errorCB->cbBase.cbInfo.callback != nullptr) {
            NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, errorCB->cbBase.cbInfo.callback));
        }
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, errorCB->cbBase.asyncWork));
    }
    delete errorCB;
    errorCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void CallErrorPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    HILOG_INFO("Main event thread complete.");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        napi_value result = nullptr;
        napi_create_int32(env, errorCB->execResult, &result);
        CreatePromiseValue(env, errorCB->cbBase.deferred, errorCB->execResult, result);
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, errorCB->cbBase.asyncWork));
    }
    delete errorCB;
    errorCB = nullptr;
    HILOG_INFO("Main event thread complete end.");
}

void CallErrorExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        errorCB->execResult = INVALID_PARAMETER;
    } else {
        HILOG_ERROR("errorCB is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

napi_value CallErrorAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperErrorCB *errorCB)
{
    HILOG_INFO("Enter.");
    if (args == nullptr || errorCB == nullptr) {
        HILOG_ERROR("Input Param args or errorCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName = 0;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));

    napi_valuetype valuetype = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[argCallback], &valuetype));
    if (valuetype == napi_function) {
        NAPI_CALL(env, napi_create_reference(env, args[argCallback], 1, &errorCB->cbBase.cbInfo.callback));
    }

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, CallErrorExecuteCB, CallErrorAsyncCompleteCB,
                       static_cast<void *>(errorCB), &errorCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, errorCB->cbBase.asyncWork));
    napi_value result = 0;
    NAPI_CALL(env, napi_get_null(env, &result));
    HILOG_INFO("End.");
    return result;
}

napi_value CallErrorPromise(napi_env env, DAHelperErrorCB *errorCB)
{
    HILOG_INFO("Enter.");
    if (errorCB == nullptr) {
        HILOG_ERROR("Input Param errorCB is nullptr.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return nullptr;
    }
    napi_value resourceName;
    NAPI_CALL(env, napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName));
    napi_deferred deferred;
    napi_value promise = 0;
    NAPI_CALL(env, napi_create_promise(env, &deferred, &promise));
    errorCB->cbBase.deferred = deferred;

    NAPI_CALL(env, napi_create_async_work(env, nullptr, resourceName, CallErrorExecuteCB, CallErrorPromiseCompleteCB,
                       static_cast<void *>(errorCB), &errorCB->cbBase.asyncWork));
    NAPI_CALL(env, napi_queue_async_work(env, errorCB->cbBase.asyncWork));
    HILOG_INFO("End.");
    return promise;
}

napi_value CallErrorWrap(napi_env env, napi_value thisVar, napi_callback_info info, napi_value *args, bool isPromise)
{
    HILOG_INFO("Enter.");
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
        HILOG_ERROR("CallError failed.");
        delete errorCB;
        errorCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    HILOG_INFO("End.");
    return ret;
}

void CallExecuteCB(napi_env env, void *data)
{
    HILOG_INFO("Worker pool thread execute.");
    DAHelperCallCB *callCB = static_cast<DAHelperCallCB *>(data);
    if (callCB->dataAbilityHelper != nullptr) {
        callCB->execResult = INVALID_PARAMETER;
        if (!callCB->uri.empty()) {
            OHOS::Uri uri(callCB->uri);
            auto ret = callCB->dataAbilityHelper->Call(uri, callCB->method, callCB->arg, callCB->pacMap);
            if (!ret) {
                // success
                callCB->result = ret;
                callCB->execResult = NO_ERROR;
            }
        }
    } else {
        HILOG_ERROR("dataAbilityHelper is nullptr.");
    }
    HILOG_INFO("Worker pool thread execute end.");
}

static std::string ExcludeTag(const std::string& jsonString, const std::string& tagString)
{
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
        HILOG_ERROR("Return result is nullptr.");
    }
    return value;
}

bool CheckArgCount(const napi_env &env, uint32_t argCount, uint32_t promiseArgCount)
{
    if (argCount < promiseArgCount || argCount > ARGS_MAX_COUNT) {
        HILOG_ERROR("Wrong argument count.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckStringParam(const napi_env &env, const napi_value &arg, std::string &stringParam)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, arg, &valuetype);
    if (valuetype != napi_string) {
        HILOG_ERROR("Wrong argument type.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    stringParam = NapiValueToStringUtf8(env, arg);
    HILOG_INFO("StringParam = %{public}s.", stringParam.c_str());
    return true;
}

bool CheckValuesBucket(const napi_env &env, const napi_value &arg, NativeRdb::ValuesBucket &valuesBucket)
{
    if (!AnalysisValuesBucket(valuesBucket, env, arg)) {
        HILOG_ERROR("Unwrap vaules bucket from JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckNotifyType(const napi_env &env, const napi_value &arg)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, arg, &valuetype);
    if (valuetype != napi_string) {
        HILOG_ERROR("Wrong argument type.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }

    std::string type = NapiValueToStringUtf8(env, arg);
    if (type != "dataChange") {
        HILOG_ERROR("Wrong argument type, type: %{public}s.", type.c_str());
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckUnregisterParam(const napi_env &env, napi_value *args, uint32_t argCount, DAHelperOnOffCB *offCB)
{
    // check param1
    offCB->uri = "";
    napi_valuetype valuetype = napi_undefined;
    if (argCount > ARGS_TWO) {
        // parse uri and callback
        napi_typeof(env, args[PARAM1], &valuetype);
        if (valuetype == napi_string) {
            offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
            HILOG_INFO("Unregister uri = %{public}s.", offCB->uri.c_str());
        } else {
            HILOG_ERROR("Wrong argument type.");
            ThrowException(env, JS_ERR_PARAM_INVALID);
            return false;
        }
        napi_typeof(env, args[PARAM2], &valuetype);
        if (valuetype == napi_function) {
            napi_create_reference(env, args[PARAM2], 1, &offCB->cbBase.cbInfo.callback);
        } else {
            HILOG_ERROR("Wrong argument type.");
            ThrowException(env, JS_ERR_PARAM_INVALID);
            return false;
        }
        return true;
    }

    // parse uri or callback
    napi_typeof(env, args[PARAM1], &valuetype);
    if (valuetype == napi_string) {
        offCB->uri = NapiValueToStringUtf8(env, args[PARAM1]);
        HILOG_INFO("Unregister uri = %{public}s.", offCB->uri.c_str());
    } else if (valuetype == napi_function) {
        napi_create_reference(env, args[PARAM1], 1, &offCB->cbBase.cbInfo.callback);
    } else {
        HILOG_ERROR("Wrong argument type.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckArrayStringParam(napi_env env, napi_value param, std::vector<std::string> &result)
{
    if (!NapiValueToArrayStringUtf8(env, param, result)) {
        HILOG_ERROR("Unwrap array string from JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckArrayValuesBucket(const napi_env &env, const napi_value &param, std::vector<NativeRdb::ValuesBucket> &result)
{
    if (!UnwrapArrayObjectFromJS(env, param, result)) {
        HILOG_ERROR("Unwrap array values bucket JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

bool CheckArrayOperation(napi_env env, napi_callback_info info, napi_value param,
    std::vector<std::shared_ptr<DataAbilityOperation>> &result)
{
    if (UnwrapArrayOperationFromJS(env, info, param, result)) {
        HILOG_ERROR("Unwrap array operation from JS failed.");
        ThrowException(env, JS_ERR_PARAM_INVALID);
        return false;
    }
    return true;
}

int32_t TransferErrorToExternal(int32_t errCode)
{
    int32_t ExternalCode = JS_ERR_INTERNAL_ERROR;
    switch (errCode) {
        default:
            ExternalCode = JS_ERR_INTERNAL_ERROR;
    }

    HILOG_DEBUG("Internal errorCode[%{public}d] to external errorCode[%{public}d].", errCode, ExternalCode);
    return ExternalCode;
}

napi_value CreateErrorValue(napi_env env, int32_t errCode)
{
    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    auto iter = ERR_CODE_MSG.find(errCode);
    std::string errMsg = iter != ERR_CODE_MSG.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return error;
}

void ThrowException(napi_env env, int32_t errCode)
{
    HILOG_DEBUG("Enter.");
    napi_throw(env, CreateErrorValue(env, errCode));
}

void CreateCallBackValue(const napi_env &env, const napi_ref &callbackIn, int32_t errorCode, const napi_value &result)
{
    HILOG_DEBUG("Enter.");
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value jsResults[ARGS_TWO] = {nullptr};
    napi_value callResult = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callbackIn, &callback));

    if (errorCode != 0) {
        // native function executes failed, return (error)
        jsResults[PARAM0] = CreateErrorValue(env, TransferErrorToExternal(errorCode));
        NAPI_CALL_RETURN_VOID(env,
            napi_call_function(env, undefined, callback, ARGS_ONE, &jsResults[PARAM0], &callResult));
    } else {
        // native function executes successfully, return (null, data)
        jsResults[PARAM0] = WrapVoidToJS(env);
        jsResults[PARAM1] = result;
        NAPI_CALL_RETURN_VOID(env,
            napi_call_function(env, undefined, callback, ARGS_TWO, &jsResults[PARAM0], &callResult));
    }
    HILOG_DEBUG("End.");
}

void CreateCallBackValue(const napi_env &env, const napi_ref &callbackIn, int32_t errorCode)
{
    HILOG_DEBUG("Enter.");
    napi_value callback = nullptr;
    napi_value undefined = nullptr;
    napi_value jsResults[ARGS_ONE] = {nullptr};
    napi_value callResult = nullptr;

    NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &undefined));
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, callbackIn, &callback));

    if (errorCode != NO_ERROR) {
        // native function executes failed, return (error)
        jsResults[PARAM0] = CreateErrorValue(env, TransferErrorToExternal(errorCode));
    } else {
        // native function executes successfully, return (null)
        jsResults[PARAM0] = WrapVoidToJS(env);
    }

    NAPI_CALL_RETURN_VOID(env,
        napi_call_function(env, undefined, callback, ARGS_ONE, &jsResults[PARAM0], &callResult));
    HILOG_DEBUG("End.");
}

void CreatePromiseValue(const napi_env &env,
    const napi_deferred &deferred, int32_t errorCode, const napi_value &result)
{
    HILOG_DEBUG("Enter");
    if (errorCode != NO_ERROR) {
        NAPI_CALL_RETURN_VOID(env,
            napi_reject_deferred(env, deferred, CreateErrorValue(env, TransferErrorToExternal(errorCode))));
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, deferred, result));
    }
    HILOG_DEBUG("End.");
}
}  // namespace AppExecFwk
}  // namespace OHOS

