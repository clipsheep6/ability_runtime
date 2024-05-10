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

#include "js_startup_manager.h"

#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_startup_config.h"
#include "js_startup_task_result.h"
#include "napi/native_api.h"
#include "startup_manager.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace
void JsStartupManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    std::unique_ptr<JsStartupManager>(static_cast<JsStartupManager *>(data));
}

napi_value JsStartupManager::Run(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsStartupManager, OnRun);
}

napi_value JsStartupManager::RemoveAllResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsStartupManager, OnRemoveAllResult);
}

napi_value JsStartupManager::GetResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsStartupManager, OnGetResult);
}

napi_value JsStartupManager::IsInitialized(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsStartupManager, OnIsInitialized);
}

napi_value JsStartupManager::RemoveResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsStartupManager, OnRemoveResult);
}

napi_value JsStartupManager::OnRun(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called.");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::STARTUP, "the param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = RunStartupTask(env, info, startupTaskManager);
    if (result != ERR_OK) {
        AbilityRuntimeErrorUtil::Throw(env, result, StartupUtils::GetErrorMessage(result));
        return CreateJsUndefined(env);
    }

    napi_value promise = nullptr;
    napi_deferred nativeDeferred = nullptr;
    napi_create_promise(env, &nativeDeferred, &promise);

    auto callback = std::make_shared<OnCompletedCallback>(
        [env, nativeDeferred](const std::shared_ptr<StartupTaskResult> &result) {
            if (result == nullptr || result->GetResultCode() != ERR_OK) {
                napi_value error = JsStartupConfig::BuildResult(env, result);
                napi_reject_deferred(env, nativeDeferred, error);
                return;
            }
            napi_value resolution = JsStartupConfig::BuildResult(env, result);
            napi_resolve_deferred(env, nativeDeferred, resolution);
            return;
        });
    result = startupTaskManager->Run(callback);
    if (result != ERR_OK) {
        if (!callback->IsCalled()) {
            AbilityRuntimeErrorUtil::Throw(env, result, StartupUtils::GetErrorMessage(result));
            return CreateJsUndefined(env);
        }
    }
    return promise;
}

napi_value JsStartupManager::OnRemoveAllResult(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called.");
    DelayedSingleton<StartupManager>::GetInstance()->RemoveAllResult();
    return CreateJsUndefined(env);
}

napi_value JsStartupManager::OnGetResult(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called.");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::STARTUP, "the param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string startupTask;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], startupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "convert startupTask name failed.");
        ThrowInvalidParamError(env, "Parameter error: Failed to convert startupTask, must be a string.");
        return CreateJsUndefined(env);
    }

    std::shared_ptr<StartupTaskResult> result;
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->GetResult(startupTask, result);
    if (res != ERR_OK || result == nullptr || result->GetResultCode() != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, failed to get result.", startupTask.c_str());
        ThrowInvalidParamError(env, "%{public}s, failed to get result.");
        return CreateJsUndefined(env);
    }
    if (result->GetResultType() != StartupTaskResult::ResultType::JS) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, the result type is not js.", startupTask.c_str());
        ThrowInvalidParamError(env, "%{public}s, the result type is not js.");
        return CreateJsUndefined(env);
    }
    std::shared_ptr<JsStartupTaskResult> jsResult = std::static_pointer_cast<JsStartupTaskResult>(result);
    if (jsResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, failed to convert to js result.", startupTask.c_str());
        ThrowInvalidParamError(env, "%{public}s, failed to convert to js result.");
        return CreateJsUndefined(env);
    }
    std::shared_ptr<NativeReference> jsResultRef = jsResult->GetJsStartupResultRef();
    if (jsResultRef == nullptr) {
        return CreateJsUndefined(env);
    }
    return jsResultRef->GetNapiValue();
}

napi_value JsStartupManager::OnIsInitialized(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called.");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::STARTUP, "the param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string startupTask;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], startupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "convert startupTask name failed.");
        ThrowInvalidParamError(env, "Parameter error: Failed to convert startupTask,"
            "must be a string or startupTask name is not matched.");
        return CreateJsUndefined(env);
    }

    bool isInitialized = false;
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->IsInitialized(startupTask, isInitialized);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, failed to get result, res = %{public}d.", startupTask.c_str(), res);
        ThrowInvalidParamError(env, "%{public}s, failed to get result, res = %{public}d.");
        return CreateJsUndefined(env);
    }
    return CreateJsValue(env, isInitialized);
}

napi_value JsStartupManager::OnRemoveResult(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called.");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::STARTUP, "the param is invalid.");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string startupTask;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], startupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "convert startupTask name failed.");
        ThrowInvalidParamError(env, "Parameter error: Failed to convert startupTask,"
            "must be a string or startupTask name is not matched.");
        return CreateJsUndefined(env);
    }

    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->RemoveResult(startupTask);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, failed to remove result, res = %{public}d.", startupTask.c_str(), res);
        ThrowInvalidParamError(env, "%{public}s, failed to get result, res = %{public}d.");
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsStartupManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Env or exportObj nullptr.");
        return nullptr;
    }

    auto jsStartupManager = std::make_unique<JsStartupManager>();
    napi_wrap(env, exportObj, jsStartupManager.release(), JsStartupManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsStartupManager";
    BindNativeFunction(env, exportObj, "run", moduleName, JsStartupManager::Run);
    BindNativeFunction(env, exportObj, "removeAllStartupTaskResults", moduleName, JsStartupManager::RemoveAllResult);
    BindNativeFunction(env, exportObj, "getStartupTaskResult", moduleName, JsStartupManager::GetResult);
    BindNativeFunction(env, exportObj, "isStartupTaskInitialized", moduleName, JsStartupManager::IsInitialized);
    BindNativeFunction(env, exportObj, "removeStartupTaskResult", moduleName, JsStartupManager::RemoveResult);
    return CreateJsUndefined(env);
}

int32_t JsStartupManager::GetDependencies(napi_env env, napi_value value, std::vector<std::string> &dependencies)
{
    bool isArray;
    napi_is_array(env, value, &isArray);
    if (!isArray) {
        TAG_LOGE(AAFwkTag::STARTUP, "value is not array.");
        ThrowInvalidParamError(env, "Parameter error: StartupTasks must be a Array.");
        return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
    }

    uint32_t arrayLength = 0;
    napi_get_array_length(env, value, &arrayLength);

    for (size_t i = 0; i < arrayLength; i++) {
        napi_value napiDep;
        napi_get_element(env, value, i, &napiDep);

        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, napiDep, &valueType);
        if (valueType != napi_string) {
            TAG_LOGE(AAFwkTag::STARTUP, "element is not string.");
            ThrowInvalidParamError(env, "Parameter error: StartupTasks element must be a string.");
            return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        }

        std::string startupTask;
        if (!ConvertFromJsValue(env, napiDep, startupTask)) {
            TAG_LOGE(AAFwkTag::STARTUP, "convert startupTask name failed.");
            ThrowInvalidParamError(env, "Parameter error: Convert startupTask name failed. Please check it.");
            return ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER;
        }
        dependencies.push_back(startupTask);
    }
    return ERR_OK;
}

int32_t JsStartupManager::GetConfig(napi_env env, napi_value value, std::shared_ptr<StartupConfig> &config)
{
    std::shared_ptr<JsStartupConfig> startupConfig = std::make_shared<JsStartupConfig>(env);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupConfig is null.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (startupConfig->Init(value) != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to init config");
        ThrowInvalidParamError(env, "Parameter error: Failed to init config, must be a StartupConfig.");
        return ERR_STARTUP_INVALID_VALUE;
    }
    config = startupConfig;
    return ERR_OK;
}

int32_t JsStartupManager::RunStartupTask(napi_env env, NapiCallbackInfo &info,
    std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::vector<std::string> dependencies;
    if (GetDependencies(env, info.argv[INDEX_ZERO], dependencies) != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to get dependencies.");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::shared_ptr<StartupConfig> config;
    if (info.argc >= ARGC_TWO) {
        int32_t result = GetConfig(env, info.argv[INDEX_ONE], config);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::STARTUP, "failed to get config: %{public}d", result);
            return result;
        }
    }

    int32_t result = DelayedSingleton<StartupManager>::GetInstance()->BuildStartupTaskManager(dependencies,
        startupTaskManager);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to build startup task manager: %{public}d", result);
        return result;
    }
    if (startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupTaskManager is nullptr");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to prepare startup task manager: %{public}d", result);
        return result;
    }
    if (config != nullptr) {
        startupTaskManager->SetConfig(config);
    }
    return ERR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
