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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_DATA_ABILITY_HELPER_UTILS_H
#define OHOS_ABILITY_RUNTIME_NAPI_DATA_ABILITY_HELPER_UTILS_H

#include "data_ability_helper_common.h"
#include "feature_ability_common.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Insert Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value InsertAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperInsertCB *insertCB);

/**
 * @brief Insert Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param insertCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value InsertPromise(napi_env env, DAHelperInsertCB *insertCB);

/**
 * @brief Insert asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void InsertExecuteCB(napi_env env, void *data);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void InsertAsyncCompleteCB(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void InsertPromiseCompleteCB(napi_env env, napi_status status, void *data);

/**
 * @brief NotifyChange Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param notifyChangeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NotifyChangeAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperNotifyChangeCB *notifyChangeCB);

/**
 * @brief NotifyChange Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param notifyChangeCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value NotifyChangePromise(napi_env env, DAHelperNotifyChangeCB *notifyChangeCB);

/**
 * @brief NotifyChange asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void NotifyChangeExecuteCB(napi_env env, void *data);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void NotifyChangeAsyncCompleteCB(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void NotifyChangePromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value GetTypeAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperGetTypeCB *gettypeCB);
napi_value GetTypePromise(napi_env env, DAHelperGetTypeCB *gettypeCB);
void GetTypeExecuteCB(napi_env env, void *data);
void GetTypeAsyncCompleteCB(napi_env env, napi_status status, void *data);
void GetTypePromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value GetFileTypesAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperGetFileTypesCB *getfiletypesCB);
napi_value GetFileTypesPromise(napi_env env, DAHelperGetFileTypesCB *getfiletypesCB);
void GetFileTypesExecuteCB(napi_env env, void *data);
void GetFileTypesAsyncCompleteCB(napi_env env, napi_status status, void *data);
void GetFileTypesPromiseCompleteCB(napi_env env, napi_status status, void *data);
napi_value WrapGetFileTypesCB(napi_env env, const DAHelperGetFileTypesCB &getfiletypesCB);

napi_value NormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperNormalizeUriCB *normalizeuriCB);
napi_value NormalizeUriPromise(napi_env env, DAHelperNormalizeUriCB *normalizeuriCB);
void NormalizeUriExecuteCB(napi_env env, void *data);
void NormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data);
void NormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value DenormalizeUriAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperDenormalizeUriCB *denormalizeuriCB);
napi_value DenormalizeUriPromise(napi_env env, DAHelperDenormalizeUriCB *denormalizeuriCB);
void DenormalizeUriExecuteCB(napi_env env, void *data);
void DenormalizeUriAsyncCompleteCB(napi_env env, napi_status status, void *data);
void DenormalizeUriPromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value DeleteAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperDeleteCB *deleteCB);
napi_value DeletePromise(napi_env env, DAHelperDeleteCB *deleteCB);
void DeleteExecuteCB(napi_env env, void *data);
void DeleteAsyncCompleteCB(napi_env env, napi_status status, void *data);
void DeletePromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value UpdateAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperUpdateCB *updateCB);
napi_value UpdatePromise(napi_env env, DAHelperUpdateCB *updateCB);
void UpdateExecuteCB(napi_env env, void *data);
void UpdateAsyncCompleteCB(napi_env env, napi_status status, void *data);
void UpdatePromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value CallErrorWrap(napi_env env, napi_value thisVar, napi_callback_info info, napi_value *args, bool isPromise);
napi_value CallAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperCallCB *callCB);
napi_value CallPromise(napi_env env, DAHelperCallCB *callCB);

napi_value OpenFileAsync(napi_env env, napi_value *args, const size_t argCallback, DAHelperOpenFileCB *openFileCB);
napi_value OpenFilePromise(napi_env env, DAHelperOpenFileCB *openFileCB);
void OpenFileExecuteCB(napi_env env, void *data);
void OpenFileAsyncCompleteCB(napi_env env, napi_status status, void *data);
void OpenFilePromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value BatchInsertAsync(
    napi_env env, napi_value *args, const size_t argCallback, DAHelperBatchInsertCB *batchInsertCB);
napi_value BatchInsertPromise(napi_env env, DAHelperBatchInsertCB *batchInsertCB);
void BatchInsertExecuteCB(napi_env env, void *data);
void BatchInsertAsyncCompleteCB(napi_env env, napi_status status, void *data);
void BatchInsertPromiseCompleteCB(napi_env env, napi_status status, void *data);

napi_value QuerySync(napi_env env, napi_value *args, const size_t argCallback, DAHelperQueryCB *queryCB);
napi_value QueryPromise(napi_env env, DAHelperQueryCB *queryCB);
napi_value WrapResultSet(napi_env env, const std::shared_ptr<NativeRdb::AbsSharedResultSet> &resultSet);

/**
 * @brief ExecuteBatch Async.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param args Indicates the arguments passed into the callback.
 * @param argcPromise Asynchronous data processing.
 * @param executeBatchCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ExecuteBatchAsync(
    napi_env env, napi_value *args, size_t argcAsync, const size_t argcPromise, DAHelperExecuteBatchCB *executeBatchCB);

/**
 * @brief ExecuteBatch Promise.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param executeBatchCB Process data asynchronously.
 *
 * @return Return JS data successfully, otherwise return nullptr.
 */
napi_value ExecuteBatchPromise(napi_env env, DAHelperExecuteBatchCB *executeBatchCB);

/**
 * @brief ExecuteBatch asynchronous processing function.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void ExecuteBatchExecuteCB(napi_env env, void *data);

/**
 * @brief The callback at the end of the asynchronous callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void ExecuteBatchAsyncCompleteCB(napi_env env, napi_status status, void *data);

/**
 * @brief The callback at the end of the Promise callback.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param data Point to asynchronous processing of data.
 */
void ExecuteBatchPromiseCompleteCB(napi_env env, napi_status status, void *data);

void GetDataAbilityResultForResult(
    napi_env env, const std::vector<std::shared_ptr<DataAbilityResult>> &dataAbilityResult, napi_value result);

void GetDataAbilityHelper(napi_env env, napi_value thisVar, std::shared_ptr<DataAbilityHelper>& dataAbilityHelper);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif /* OHOS_ABILITY_RUNTIME_NAPI_DATA_ABILITY_HELPER_UTILS_H */