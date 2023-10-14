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

#include "js_ability_auto_startup_callback.h"

#include "hilog_wrapper.h"
#include "js_ability_auto_startup_manager_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string METHOD_ON = "onAutoStartupOn";
const std::string METHOD_OFF = "onAutoStartupOff";
} // namespace
JsAbilityAutoStartupCallBack::JsAbilityAutoStartupCallBack(NativeEngine &engine) : engine_(engine) {}

JsAbilityAutoStartupCallBack::~JsAbilityAutoStartupCallBack() {}

void JsAbilityAutoStartupCallBack::OnAutoStartupOn(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called.");
    JSCallFunction(info, METHOD_ON);
}

void JsAbilityAutoStartupCallBack::OnAutoStartupOff(const AutoStartupInfo &info)
{
    HILOG_DEBUG("Called.");
    JSCallFunction(info, METHOD_OFF);
}

void JsAbilityAutoStartupCallBack::Register(NativeValue *jsCallback)
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(mutexlock);
    if (jsCallback == nullptr) {
        HILOG_ERROR("jsCallback is nullptr.");
        return;
    }

    for (auto callback : callbacks_) {
        if (IsJsCallbackEquals(callback, jsCallback)) {
            HILOG_ERROR("The current callback already exists.");
            return;
        }
    }
    callbacks_.emplace_back(std::shared_ptr<NativeReference>(engine_.CreateReference(jsCallback, 1)));
}

void JsAbilityAutoStartupCallBack::UnRegister(NativeValue *jsCallback)
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(mutexlock);
    if (jsCallback == nullptr) {
        HILOG_DEBUG("jsCallback is nullptr, delete all callback.");
        callbacks_.clear();
        return;
    }
    for (auto item = callbacks_.begin(); item != callbacks_.end();) {
        if (IsJsCallbackEquals(*item, jsCallback)) {
            item = callbacks_.erase(item);
        } else {
            item++;
        }
    }
}

bool JsAbilityAutoStartupCallBack::IsCallbacksEmpty()
{
    return callbacks_.empty();
}

void JsAbilityAutoStartupCallBack::JSCallFunction(const AutoStartupInfo &info, const std::string &methodName)
{
    std::lock_guard<std::mutex> lock(mutexlock);
    AbilityRuntime::HandleEscape handleEscape(engine_);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            HILOG_ERROR("callback is nullptr.");
            continue;
        }
        NativeValue *value = callback->Get();
        if (value == nullptr) {
            HILOG_ERROR("Failed to get value.");
            return;
        }
        NativeObject *jObj = ConvertNativeValueTo<NativeObject>(value);
        if (jObj == nullptr) {
            HILOG_ERROR("Failed to convert native value to object.");
            return;
        }
        auto method = jObj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("Failed to get %{public}s from object.", methodName.data());
            return;
        }
        NativeValue *argv[] = { CreateJsAutoStartupInfo(engine_, info) };
        handleEscape.Escape(engine_.CallFunction(value, method, argv, ArraySize(argv)));
    }
}

bool JsAbilityAutoStartupCallBack::IsJsCallbackEquals(
    std::shared_ptr<NativeReference> callback, NativeValue *jsCallback)
{
    if (!callback) {
        HILOG_ERROR("Invalid jsCallback.");
        return false;
    }

    NativeValue *value = callback->Get();
    if (value == nullptr) {
        HILOG_ERROR("Failed to get object.");
        return false;
    }

    if (value->StrictEquals(jsCallback)) {
        return true;
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS