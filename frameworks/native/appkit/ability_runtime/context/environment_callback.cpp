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

#include "environment_callback.h"

#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsEnvironmentCallback::JsEnvironmentCallback(NativeEngine* engine)
    : engine_(engine)
{
}

int32_t JsEnvironmentCallback::serialNumber_ = 0;

void JsEnvironmentCallback::CallConfigurationUpdatedInner(const std::string &methodName,
    const AppExecFwk::Configuration &config, const std::map<int32_t, std::shared_ptr<NativeReference>> callbacks)
{
    HILOG_DEBUG("CallConfigurationUpdatedInner methodName = %{public}s", methodName.c_str());
    for (auto &callback : callbacks) {
        if (!callback.second) {
            HILOG_ERROR("CallConfigurationUpdatedInner, Invalid jsCallback");
            return;
        }

        auto value = callback.second->Get();
        auto obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("CallConfigurationUpdatedInner, Failed to get object");
            return;
        }

        auto method = obj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("CallConfigurationUpdatedInner, Failed to get %{public}s from object", methodName.data());
            return;
        }

        NativeValue *argv[] = { CreateJsConfiguration(*engine_, config) };
        engine_->CallFunction(value, method, argv, ArraySize(argv));
    }
}

void JsEnvironmentCallback::OnConfigurationUpdated(const AppExecFwk::Configuration &config)
{
    std::weak_ptr<JsEnvironmentCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [thisWeakPtr, config, callbacks = callbacks_, callbacksSync = callbacksSync_]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsEnvironmentCallback> jsEnvCallback = thisWeakPtr.lock();
            if (jsEnvCallback) {
                jsEnvCallback->CallConfigurationUpdatedInner("onConfigurationUpdated", config, callbacks);
                jsEnvCallback->CallConfigurationUpdatedInner("onConfigurationUpdated", config, callbacksSync);
            }
        }
    );
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsEnvironmentCallback::OnConfigurationUpdated",
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsEnvironmentCallback::CallMemoryLevelInner(const std::string &methodName, const int level,
    const std::map<int32_t, std::shared_ptr<NativeReference>> callbacks)
{
    HILOG_DEBUG("CallMemoryLevelInner methodName = %{public}s", methodName.c_str());
    for (auto &callback : callbacks) {
        if (!callback.second) {
            HILOG_ERROR("CallMemoryLevelInner, Invalid jsCallback");
            return;
        }

        auto value = callback.second->Get();
        auto obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("CallMemoryLevelInner, Failed to get object");
            return;
        }

        auto method = obj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("CallMemoryLevelInner, Failed to get %{public}s from object", methodName.data());
            return;
        }

        NativeValue *argv[] = { CreateJsValue(*engine_, level) };
        engine_->CallFunction(value, method, argv, ArraySize(argv));
    }
}

void JsEnvironmentCallback::OnMemoryLevel(const int level)
{
    std::weak_ptr<JsEnvironmentCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [thisWeakPtr, level, callbacks = callbacks_, callbacksSync = callbacksSync_]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsEnvironmentCallback> jsEnvCallback = thisWeakPtr.lock();
            if (jsEnvCallback) {
                jsEnvCallback->CallMemoryLevelInner("onMemoryLevel", level, callbacks);
                jsEnvCallback->CallMemoryLevelInner("onMemoryLevel", level, callbacksSync);
            }
        }
    );
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsEnvironmentCallback::OnMemoryLevel",
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

int32_t JsEnvironmentCallback::Register(NativeValue *jsCallback, bool isSync)
{
    if (engine_ == nullptr) {
        return -1;
    }
    int32_t callbackId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    if (isSync) {
        callbacksSync_.emplace(callbackId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsCallback, 1)));
    } else {
        callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsCallback, 1)));
    }
    return callbackId;
}

bool JsEnvironmentCallback::UnRegister(int32_t callbackId, bool isSync)
{
    HILOG_DEBUG("UnRegister called, env callbackId : %{public}d", callbackId);
    if (isSync) {
        auto it = callbacksSync_.find(callbackId);
        if (it == callbacksSync_.end()) {
            HILOG_ERROR("UnRegister env callbackId: %{public}d is not in callbacksSync_", callbackId);
            return false;
        }
        HILOG_DEBUG("callbacksSync_.callbackId : %{public}d", it->first);
        return callbacksSync_.erase(callbackId) == 1;
    }
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        HILOG_ERROR("UnRegister env callbackId: %{public}d is not in callbacks_", callbackId);
        return false;
    }
    HILOG_DEBUG("callbacks_.callbackId : %{public}d", it->first);
    return callbacks_.erase(callbackId) == 1;
}

bool JsEnvironmentCallback::IsEmpty() const
{
    return callbacks_.empty() && callbacksSync_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS