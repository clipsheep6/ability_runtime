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
#include "js_runtime_utils.h"
#include "js_data_struct_converter.h"

namespace OHOS {
namespace AbilityRuntime {
JsEnvironmentCallback::JsEnvironmentCallback(NativeEngine* engine)
    : engine_(engine)
{
}

int32_t JsEnvironmentCallback::serialNumber_ = 0;

void JsEnvironmentCallback::CallJsMethodInner(
    const std::string &methodName, const AppExecFwk::Configuration &config)
{
    for (auto &callback : callbacks_) {
        if (!callback.second) {
            HILOG_ERROR("Invalid jsCallback");
            return;
        }

        auto value = callback.second->Get();
        auto obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("Failed to get object");
            return;
        }

        auto method = obj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("Failed to get %{public}s from object", methodName.data());
            return;
        }

        NativeValue *argv[] = { CreateJsConfiguration(*engine_, config) };
        engine_->CallFunction(value, method, argv, ArraySize(argv));
    }
}

void JsEnvironmentCallback::CallJsMethod(
    const std::string &methodName, const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("CallJsMethod methodName = %{public}s", methodName.c_str());
    // js callback should run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [JsEnvironmentCallback = this, methodName, config](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (JsEnvironmentCallback) {
                JsEnvironmentCallback->CallJsMethodInner(methodName, config);
            }
        });
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsEnvironmentCallback::OnConfigurationUpdated",
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsEnvironmentCallback::OnConfigurationUpdated(const AppExecFwk::Configuration &config)
{
    CallJsMethod("onConfigurationUpdated", config);
}

int32_t JsEnvironmentCallback::Register(NativeValue *jsCallback)
{
    if (engine_ == nullptr) {
        return -1;
    }
    int32_t callbackId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
        return -1;
    }
    callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsCallback, 1)));
    return callbackId;
}

bool JsEnvironmentCallback::UnRegister(int32_t callbackId)
{
    HILOG_DEBUG("UnRegister called, env callbackId : %{public}d", callbackId);
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        HILOG_ERROR("UnRegister env callbackId: %{publid}d is not in callbacks_", callbackId);
        return false;
    }
    HILOG_DEBUG("callbacks_.callbackId : %{public}d", it->first);
    return callbacks_.erase(callbackId) == 1;
}

bool JsEnvironmentCallback::IsEmpty()
{
    return callbacks_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS