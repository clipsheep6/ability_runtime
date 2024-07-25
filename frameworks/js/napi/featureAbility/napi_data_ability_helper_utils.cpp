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
#include "napi_data_ability_helper_utils.h"

#include "napi_common_ability.h"
#include "data_ability_result.h"
#include "hilog_tag_wrapper.h"
#include "napi_data_ability_observer.h"
#include "napi_data_ability_predicates.h"
#include "napi_rdb_predicates.h"
#include "napi_result_set.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {

napi_value InsertAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperInsertCB *insertCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || insertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value InsertPromise(napi_env env, DAHelperInsertCB *insertCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (insertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void InsertExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void InsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void InsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperInsertCB *insertCB = static_cast<DAHelperInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, insertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, insertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, insertCB->cbBase.asyncWork));
    delete insertCB;
    insertCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value NotifyChangeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperNotifyChangeCB *notifyChangeCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || notifyChangeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    if (notifyChangeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    auto dataAbilityHelper = notifyChangeCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        notifyChangeCB->execResult = INVALID_PARAMETER;
        if (!notifyChangeCB->uri.empty()) {
            OHOS::Uri uri(notifyChangeCB->uri);
            dataAbilityHelper->NotifyChange(uri);
            notifyChangeCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    }
}

void NotifyChangeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
        TAG_LOGI(AAFwkTag::FA, "invalid callback");
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
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperNotifyChangeCB *notifyChangeCB = static_cast<DAHelperNotifyChangeCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, 0, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, notifyChangeCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, notifyChangeCB->cbBase.asyncWork));
    delete notifyChangeCB;
    notifyChangeCB = nullptr;
}

napi_value GetTypeAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperGetTypeCB *gettypeCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || gettypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value GetTypePromise(napi_env env, DAHelperGetTypeCB *gettypeCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (gettypeCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void GetTypeExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    auto dataAbilityHelper = gettypeCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        gettypeCB->execResult = INVALID_PARAMETER;
        if (!gettypeCB->uri.empty()) {
            OHOS::Uri uri(gettypeCB->uri);
            gettypeCB->result = dataAbilityHelper->GetType(uri);
            gettypeCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetTypeAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetTypePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperGetTypeCB *gettypeCB = static_cast<DAHelperGetTypeCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, gettypeCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, gettypeCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, gettypeCB->cbBase.asyncWork));
    delete gettypeCB;
    gettypeCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value GetFileTypesAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperGetFileTypesCB *getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || getfiletypesCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value GetFileTypesPromise(napi_env env, DAHelperGetFileTypesCB *getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (getfiletypesCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void GetFileTypesExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    auto dataAbilityHelper = getfiletypesCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        getfiletypesCB->execResult = INVALID_PARAMETER;
        if (!getfiletypesCB->uri.empty()) {
            OHOS::Uri uri(getfiletypesCB->uri);
            TAG_LOGI(AAFwkTag::FA, "uri:%{public}s", uri.ToString().c_str());
            TAG_LOGI(
                AAFwkTag::FA, "mimeTypeFilter:%{public}s", getfiletypesCB->mimeTypeFilter.c_str());
            getfiletypesCB->result = dataAbilityHelper->GetFileTypes(uri, getfiletypesCB->mimeTypeFilter);
            getfiletypesCB->execResult = NO_ERROR;
        } else {
            TAG_LOGI(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGI(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetFileTypesAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetFileTypesPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperGetFileTypesCB *getfiletypesCB = static_cast<DAHelperGetFileTypesCB *>(data);
    napi_value result = nullptr;

    result = WrapGetFileTypesCB(env, *getfiletypesCB);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, getfiletypesCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, getfiletypesCB->cbBase.asyncWork));
    delete getfiletypesCB;
    getfiletypesCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value WrapGetFileTypesCB(napi_env env, const DAHelperGetFileTypesCB &getfiletypesCB)
{
    TAG_LOGI(AAFwkTag::FA, "result.size:%{public}zu", getfiletypesCB.result.size());
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        TAG_LOGI(
            AAFwkTag::FA, "result[%{public}zu]:%{public}s", i, getfiletypesCB.result.at(i).c_str());
    }
    napi_value proValue = nullptr;

    napi_value jsArrayresult = nullptr;
    NAPI_CALL(env, napi_create_array(env, &jsArrayresult));
    for (size_t i = 0; i < getfiletypesCB.result.size(); i++) {
        proValue = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, getfiletypesCB.result.at(i).c_str(), NAPI_AUTO_LENGTH, &proValue));
        NAPI_CALL(env, napi_set_element(env, jsArrayresult, i, proValue));
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return jsArrayresult;
}

napi_value NormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperNormalizeUriCB *normalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || normalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value NormalizeUriPromise(napi_env env, DAHelperNormalizeUriCB *normalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (normalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void NormalizeUriExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
        TAG_LOGI(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void NormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void NormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperNormalizeUriCB *normalizeuriCB = static_cast<DAHelperNormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, normalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, normalizeuriCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, normalizeuriCB->cbBase.asyncWork));
    delete normalizeuriCB;
    normalizeuriCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value DenormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || denormalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value DenormalizeUriPromise(napi_env env, DAHelperDenormalizeUriCB *denormalizeuriCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (denormalizeuriCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void DenormalizeUriExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void DenormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void DenormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperDenormalizeUriCB *denormalizeuriCB = static_cast<DAHelperDenormalizeUriCB *>(data);
    napi_value result = nullptr;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, denormalizeuriCB->result.c_str(), NAPI_AUTO_LENGTH, &result));
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, denormalizeuriCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, denormalizeuriCB->cbBase.asyncWork));
    delete denormalizeuriCB;
    denormalizeuriCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value DeleteAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperDeleteCB *deleteCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || deleteCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value DeletePromise(napi_env env, DAHelperDeleteCB *deleteCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (deleteCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void DeleteExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperDeleteCB *deleteCB = static_cast<DAHelperDeleteCB *>(data);
    auto dataAbilityHelper = deleteCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        deleteCB->execResult = INVALID_PARAMETER;
        if (!deleteCB->uri.empty()) {
            OHOS::Uri uri(deleteCB->uri);
            deleteCB->result = dataAbilityHelper->Delete(uri, deleteCB->predicates);
            deleteCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void DeleteAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "complete");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void DeletePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperDeleteCB *DeleteCB = static_cast<DAHelperDeleteCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, DeleteCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, DeleteCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, DeleteCB->cbBase.asyncWork));
    delete DeleteCB;
    DeleteCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value UpdateAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperUpdateCB *updateCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || updateCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value UpdatePromise(napi_env env, DAHelperUpdateCB *updateCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (updateCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void UpdateExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    auto dataAbilityHelper = updateCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        updateCB->execResult = INVALID_PARAMETER;
        if (!updateCB->uri.empty()) {
            OHOS::Uri uri(updateCB->uri);
            updateCB->result = dataAbilityHelper->Update(uri, updateCB->valueBucket, updateCB->predicates);
            updateCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void UpdateAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void UpdatePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperUpdateCB *updateCB = static_cast<DAHelperUpdateCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, updateCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, updateCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, updateCB->cbBase.asyncWork));
    delete updateCB;
    updateCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

void CallErrorAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void CallErrorPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        napi_value result = nullptr;
        napi_create_int32(env, errorCB->execResult, &result);
        NAPI_CALL_RETURN_VOID(env, napi_reject_deferred(env, errorCB->cbBase.deferred, result));
        NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, errorCB->cbBase.asyncWork));
    }
    delete errorCB;
    errorCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

void CallErrorExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperErrorCB *errorCB = static_cast<DAHelperErrorCB *>(data);
    if (errorCB != nullptr) {
        errorCB->execResult = INVALID_PARAMETER;
    } else {
        TAG_LOGE(AAFwkTag::FA, "null errorCB");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value CallErrorAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperErrorCB *errorCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || errorCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value CallErrorPromise(napi_env env, DAHelperErrorCB *errorCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (errorCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "param is null");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

napi_value CallErrorWrap(napi_env env, napi_value thisVar, napi_callback_info info, napi_value *args, bool isPromise)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
        TAG_LOGE(AAFwkTag::FA, "null ret");
        delete errorCB;
        errorCB = nullptr;
        ret = WrapVoidToJS(env);
    }
    TAG_LOGI(AAFwkTag::FA, "end");
    return ret;
}

void CallExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

static std::string ExcludeTag(const std::string& jsonString, const std::string& tagString)
{
    TAG_LOGD(AAFwkTag::FA, "called");
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
    TAG_LOGD(AAFwkTag::FA, "end");
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
        TAG_LOGE(AAFwkTag::FA, "null ret");
    }
    return value;
}

void CallAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void CallPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperCallCB *callCB = static_cast<DAHelperCallCB *>(data);
    napi_value result = nullptr;
    result = CallPacMapValue(env, callCB->result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, callCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, callCB->cbBase.asyncWork));
    delete callCB;
    callCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value CallAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperCallCB *callCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || callCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value CallPromise(napi_env env, DAHelperCallCB *callCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (callCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

napi_value OpenFileAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperOpenFileCB *openFileCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || openFileCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value OpenFilePromise(napi_env env, DAHelperOpenFileCB *openFileCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (openFileCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void OpenFileExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    auto dataAbilityHelper = OpenFileCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        OpenFileCB->execResult = INVALID_PARAMETER;
        if (!OpenFileCB->uri.empty()) {
            OHOS::Uri uri(OpenFileCB->uri);
            OpenFileCB->result = dataAbilityHelper->OpenFile(uri, OpenFileCB->mode);
            OpenFileCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void OpenFileAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void OpenFilePromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperOpenFileCB *OpenFileCB = static_cast<DAHelperOpenFileCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, OpenFileCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, OpenFileCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, OpenFileCB->cbBase.asyncWork));
    delete OpenFileCB;
    OpenFileCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value BatchInsertAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperBatchInsertCB *batchInsertCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || batchInsertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value BatchInsertPromise(napi_env env, DAHelperBatchInsertCB *batchInsertCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (batchInsertCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void BatchInsertExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperBatchInsertCB *batchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    auto dataAbilityHelper = batchInsertCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        batchInsertCB->execResult = INVALID_PARAMETER;
        if (!batchInsertCB->uri.empty()) {
            OHOS::Uri uri(batchInsertCB->uri);
            batchInsertCB->result = dataAbilityHelper->BatchInsert(uri, batchInsertCB->values);
            batchInsertCB->execResult = NO_ERROR;
        } else {
            TAG_LOGE(AAFwkTag::FA, "empty uri");
        }
    } else {
        TAG_LOGE(AAFwkTag::FA, "null dataAbilityHelper");
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void BatchInsertAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void BatchInsertPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperBatchInsertCB *BatchInsertCB = static_cast<DAHelperBatchInsertCB *>(data);
    napi_value result = nullptr;
    napi_create_int32(env, BatchInsertCB->result, &result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, BatchInsertCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, BatchInsertCB->cbBase.asyncWork));
    delete BatchInsertCB;
    BatchInsertCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

napi_value QuerySync(napi_env env, napi_value *args, const size_t argCallback, DAHelperQueryCB *queryCB)
{
    TAG_LOGD(AAFwkTag::FA, "called");
    if (args == nullptr || queryCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGD(AAFwkTag::FA, "called");
    if (queryCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null queryCB");
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
        TAG_LOGE(AAFwkTag::FA, "null resultSet");
        return WrapVoidToJS(env);
    }

    return RdbJsKit::ResultSetProxy::NewInstance(env, resultSet);
}

napi_value ExecuteBatchAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperExecuteBatchCB *executeBatchCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (args == nullptr || executeBatchCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return result;
}

napi_value ExecuteBatchPromise(napi_env env, DAHelperExecuteBatchCB *executeBatchCB)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    if (executeBatchCB == nullptr) {
        TAG_LOGE(AAFwkTag::FA, "null param");
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
    TAG_LOGI(AAFwkTag::FA, "end");
    return promise;
}

void ExecuteBatchExecuteCB(napi_env env, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    auto dataAbilityHelper = executeBatchCB->dataAbilityHelper;
    if (dataAbilityHelper != nullptr) {
        OHOS::Uri uri(executeBatchCB->uri);
        executeBatchCB->result = dataAbilityHelper->ExecuteBatch(uri, executeBatchCB->operations);
        TAG_LOGI(AAFwkTag::FA, "%{public}s, dataAbilityHelper is not nullptr. %{public}zu",
            __func__, executeBatchCB->result.size());
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void ExecuteBatchAsyncCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
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
    TAG_LOGI(AAFwkTag::FA, "end");
}

void ExecuteBatchPromiseCompleteCB(napi_env env, napi_status status, void *data)
{
    TAG_LOGI(AAFwkTag::FA, "called");
    DAHelperExecuteBatchCB *executeBatchCB = static_cast<DAHelperExecuteBatchCB *>(data);
    napi_value result = nullptr;
    napi_create_array(env, &result);
    GetDataAbilityResultForResult(env, executeBatchCB->result, result);
    NAPI_CALL_RETURN_VOID(env, napi_resolve_deferred(env, executeBatchCB->cbBase.deferred, result));
    NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, executeBatchCB->cbBase.asyncWork));
    delete executeBatchCB;
    executeBatchCB = nullptr;
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetDataAbilityResultForResult(
    napi_env env, const std::vector<std::shared_ptr<DataAbilityResult>> &dataAbilityResult, napi_value result)
{
    TAG_LOGI(AAFwkTag::FA, "size:%{public}zu", dataAbilityResult.size());
    int32_t index = 0;
    std::vector<std::shared_ptr<DataAbilityResult>> entities = dataAbilityResult;
    for (const auto &item : entities) {
        napi_value objDataAbilityResult;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objDataAbilityResult));

        napi_value uri;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, item->GetUri().ToString().c_str(), NAPI_AUTO_LENGTH, &uri));
        TAG_LOGI(AAFwkTag::FA, "uri= [%{public}s]", item->GetUri().ToString().c_str());
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objDataAbilityResult, "uri", uri));

        napi_value count;
        NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, item->GetCount(), &count));
        TAG_LOGI(AAFwkTag::FA, "count= [%{public}d]", item->GetCount());
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objDataAbilityResult, "count", count));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, result, index, objDataAbilityResult));
        index++;
    }
    TAG_LOGI(AAFwkTag::FA, "end");
}

void GetDataAbilityHelper(napi_env env, napi_value thisVar, std::shared_ptr<DataAbilityHelper>& dataAbilityHelper)
{
    NAPIDataAbilityHelperWrapper* wrapper = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&wrapper));
    if (wrapper != nullptr) {
        dataAbilityHelper = wrapper->GetDataAbilityHelper();
    }
}
} // AppExecFwk
} // OHOS