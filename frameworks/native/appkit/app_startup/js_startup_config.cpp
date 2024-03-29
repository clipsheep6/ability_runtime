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

#include "js_startup_config.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {

JsStartupConfig::JsStartupConfig(JsRuntime &jsRuntime, std::unique_ptr<NativeReference> &configEntryJsRef)
    : StartupConfig(), jsRuntime_(jsRuntime), configEntryJsRef_(std::move(configEntryJsRef))
{}

JsStartupConfig::~JsStartupConfig() = default;

int32_t JsStartupConfig::Init()
{
    if (configEntryJsRef_ == nullptr) {
        HILOG_ERROR("config entry is null");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value configEntry = configEntryJsRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, configEntry, napi_object)) {
        HILOG_ERROR("config entry is not napi object.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value onConfig = nullptr;
    napi_get_named_property(env, configEntry, "onConfig", &onConfig);
    if (onConfig == nullptr) {
        HILOG_ERROR("no property onConfig in config entry.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    bool isCallable = false;
    napi_is_callable(env, onConfig, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("onConfig is not callable.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value config = nullptr;
    napi_call_function(env, configEntry, onConfig, 0, nullptr, &config);
    if (config == nullptr) {
        HILOG_ERROR("config is null.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    InitAwaitTimeout(env, config);
    InitListener(env, config);
    return ERR_OK;
}

void JsStartupConfig::InitAwaitTimeout(napi_env env, napi_value config)
{
    napi_value awaitTimeout = nullptr;
    napi_get_named_property(env, config, "awaitTimeout", &awaitTimeout);
    if (awaitTimeout == nullptr) {
        HILOG_DEBUG("no property awaitTimeout in config.");
        return;
    }
    int32_t awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    if (!ConvertFromJsValue(env, awaitTimeout, awaitTimeoutNum)) {
        HILOG_DEBUG("failed to covert awaitTimeout to number.");
        return;
    }
    HILOG_DEBUG("set awaitTimeoutMs to %{public}d.", awaitTimeoutNum);
    awaitTimeoutMs_ = awaitTimeoutNum;
}

void JsStartupConfig::InitListener(napi_env env, napi_value config)
{
    napi_value listener = nullptr;
    napi_get_named_property(env, config, "startupListener", &listener);
    if (listener == nullptr) {
        HILOG_DEBUG("no property startupListener in config.");
        return;
    }
    if (!CheckTypeForNapiValue(env, listener, napi_object)) {
        HILOG_DEBUG("listener is not napi object.");
        return;
    }

    napi_value onCompleted = nullptr;
    napi_get_named_property(env, listener, "onCompleted", &onCompleted);
    if (onCompleted == nullptr) {
        HILOG_DEBUG("no property onCompleted in listener.");
        return;
    }
    napi_ref listenerRef = nullptr;
    napi_create_reference(env, listener, 1, &listenerRef);
    std::shared_ptr<NativeReference> listenerRefSp(reinterpret_cast<NativeReference*>(listenerRef));
    auto onCompletedCallback = std::make_shared<OnCompletedCallback>(
        [env, listenerRefSp](const std::shared_ptr<StartupTaskResult> &result) {
            if (env == nullptr || listenerRefSp == nullptr) {
                HILOG_ERROR("env or listenerRefSp is null");
                return;
            }
            HandleScope handleScope(env);
            napi_value listener = listenerRefSp->GetNapiValue();

            napi_value onCompleted = nullptr;
            napi_get_named_property(env, listener, "onCompleted", &onCompleted);
            if (onCompleted == nullptr) {
                HILOG_ERROR("no property onCompleted in listener.");
                return;
            }
            bool isCallable = false;
            napi_is_callable(env, onCompleted, &isCallable);
            if (!isCallable) {
                HILOG_ERROR("onCompleted is not callable.");
                return;
            }
            napi_value argv[1] = { JsStartupConfig::BuildResult(env, result) };
            napi_call_function(env, listener, onCompleted, 1, argv, nullptr);
        });
    listener_ = std::make_shared<StartupListener>(onCompletedCallback);
}

napi_value JsStartupConfig::BuildResult(napi_env env, const std::shared_ptr<StartupTaskResult> &result)
{
    if (result->GetResultCode() != ERR_OK) {
        return CreateJsError(env, result->GetResultCode(), result->GetResultMessage());
    }
    return CreateJsNull(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
