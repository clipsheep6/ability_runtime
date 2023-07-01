/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "application_state_change_callback.h"

#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsApplicationStateChangeCallback::JsApplicationStateChangeCallback(NativeEngine* engine)
    : engine_(engine)
{
}

void JsApplicationStateChangeCallback::CallJsMethodInnerCommon(
    const std::string &methodName, const std::set<std::shared_ptr<NativeReference>> callbacks)
{
    for (auto &callback : callbacks) {
        if (!callback) {
            HILOG_ERROR("Invalid jsCallback");
            continue;
        }

        auto value = callback->Get();
        auto obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("Failed to get object");
            continue;
        }

        auto method = obj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("Failed to get %{public}s from object", methodName.data());
            continue;
        }
        engine_->CallFunction(value, method, nullptr, 0);
    }
}

void JsApplicationStateChangeCallback::CallJsMethod(const std::string &methodName)
{
    HILOG_DEBUG("MethodName = %{public}s", methodName.c_str());
    std::weak_ptr<JsApplicationStateChangeCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, callbacks = callbacks_]
        (NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsApplicationStateChangeCallback> jsCallback = thisWeakPtr.lock();
            if (jsCallback) {
                jsCallback->CallJsMethodInnerCommon(methodName, callbacks);
            }
        }
    );
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsApplicationStateChangeCallback::CallJsMethod:" + methodName,
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsApplicationStateChangeCallback::NotifyApplicationForeground()
{
    CallJsMethod("onApplicationForeground");
}

void JsApplicationStateChangeCallback::NotifyApplicationBackground()
{
    CallJsMethod("onApplicationBackground");
}

void JsApplicationStateChangeCallback::Register(NativeValue *jsCallback)
{
    if (engine_ == nullptr || jsCallback == nullptr) {
        HILOG_ERROR("Engine or jsCallback is nullptr");
        return;
    }
    callbacks_.emplace(std::shared_ptr<NativeReference>(engine_->CreateReference(jsCallback, 1)));
}

bool JsApplicationStateChangeCallback::UnRegister(NativeValue *jsCallback)
{
    if (jsCallback == nullptr) {
        HILOG_INFO("jsCallback is nullptr, delete all callback.");
        callbacks_.clear();
        return true;
    }

    for (auto &callback : callbacks_) {
        if (!callback) {
            HILOG_ERROR("Invalid jsCallback");
            continue;
        }

        NativeValue *value = callback->Get();
        if (value == nullptr) {
            HILOG_ERROR("Failed to get object");
            continue;
        }

        if (value->StrictEquals(jsCallback)) {
            return callbacks_.erase(callback) == 1;
        }
    }
    return false;
}

bool JsApplicationStateChangeCallback::IsEmpty() const
{
    return callbacks_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS