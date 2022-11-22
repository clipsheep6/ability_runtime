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

#include "ability_lifecycle_callback.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsAbilityLifecycleCallback::JsAbilityLifecycleCallback(NativeEngine* engine)
    : engine_(engine)
{
}

int32_t JsAbilityLifecycleCallback::serialNumber_ = 0;

void JsAbilityLifecycleCallback::CallJsMethodInner(
    const std::string &methodName, const std::shared_ptr<NativeReference> &ability)
{
    CallJsMethodInnerCommon(methodName, ability, nullptr, false);
}

void JsAbilityLifecycleCallback::CallWindowStageJsMethodInner(const std::string &methodName,
    const std::shared_ptr<NativeReference> &ability, const std::shared_ptr<NativeReference> &windowStage)
{
    CallJsMethodInnerCommon(methodName, ability, windowStage, true);
}

void JsAbilityLifecycleCallback::CallJsMethodInnerCommon(const std::string &methodName,
    const std::shared_ptr<NativeReference> &ability, const std::shared_ptr<NativeReference> &windowStage,
    bool isWindowStage)
{
    auto nativeAbilityObj = engine_->CreateNull();
    if (ability != nullptr) {
        nativeAbilityObj = ability->Get();
    }

    auto nativeWindowStageObj = engine_->CreateNull();
    if (windowStage != nullptr) {
        nativeWindowStageObj = windowStage->Get();
    }

    for (auto &callback : callbacks_) {
        if (!callback.second) {
            HILOG_ERROR("CallJsMethodInnerCommon, Invalid jsCallback");
            return;
        }

        auto value = callback.second->Get();
        auto obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("CallJsMethodInnerCommon, Failed to get object");
            return;
        }

        auto method = obj->GetProperty(methodName.data());
        if (method == nullptr) {
            HILOG_ERROR("CallJsMethodInnerCommon, Failed to get %{public}s from object", methodName.data());
            return;
        }

        if (!isWindowStage) {
            NativeValue *argv[] = { nativeAbilityObj };
            engine_->CallFunction(value, method, argv, ArraySize(argv));
        } else {
            NativeValue *argv[] = { nativeAbilityObj, nativeWindowStageObj };
            engine_->CallFunction(value, method, argv, ArraySize(argv));
        }
    }
}

void JsAbilityLifecycleCallback::CallJsMethod(
    const std::string &methodName, const std::shared_ptr<NativeReference> &ability)
{
    HILOG_DEBUG("methodName = %{public}s", methodName.c_str());
    if (!ability) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    std::weak_ptr<JsAbilityLifecycleCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, ability](NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsAbilityLifecycleCallback> jsCallback = thisWeakPtr.lock();
            if (jsCallback) {
                jsCallback->CallJsMethodInner(methodName, ability);
            }
        }
    );
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsAbilityLifecycleCallback::CallJsMethod:" + methodName,
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsAbilityLifecycleCallback::CallWindowStageJsMethod(const std::string &methodName,
    const std::shared_ptr<NativeReference> &ability, const std::shared_ptr<NativeReference> &windowStage)
{
    HILOG_DEBUG("methodName = %{public}s", methodName.c_str());
    if (!ability || !windowStage) {
        HILOG_ERROR("ability or windowStage is nullptr");
        return;
    }
    std::weak_ptr<JsAbilityLifecycleCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, ability, windowStage](NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsAbilityLifecycleCallback> jsCallback = thisWeakPtr.lock();
            if (jsCallback) {
                jsCallback->CallWindowStageJsMethodInner(methodName, ability, windowStage);
            }
        }
    );
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsAbilityLifecycleCallback::CallWindowStageJsMethod:" + methodName,
        *engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsAbilityLifecycleCallback::OnAbilityCreate(const std::shared_ptr<NativeReference> &ability)
{
    CallJsMethod("onAbilityCreate", ability);
}

void JsAbilityLifecycleCallback::OnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    CallWindowStageJsMethod("onWindowStageCreate", ability, windowStage);
}

void JsAbilityLifecycleCallback::OnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    CallWindowStageJsMethod("onWindowStageDestroy", ability, windowStage);
}

void JsAbilityLifecycleCallback::OnWindowStageActive(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    CallWindowStageJsMethod("onWindowStageActive", ability, windowStage);
}

void JsAbilityLifecycleCallback::OnWindowStageInactive(const std::shared_ptr<NativeReference> &ability,
    const std::shared_ptr<NativeReference> &windowStage)
{
    CallWindowStageJsMethod("onWindowStageInactive", ability, windowStage);
}

void JsAbilityLifecycleCallback::OnAbilityDestroy(const std::shared_ptr<NativeReference> &ability)
{
    CallJsMethod("onAbilityDestroy", ability);
}

void JsAbilityLifecycleCallback::OnAbilityForeground(const std::shared_ptr<NativeReference> &ability)
{
    CallJsMethod("onAbilityForeground", ability);
}

void JsAbilityLifecycleCallback::OnAbilityBackground(const std::shared_ptr<NativeReference> &ability)
{
    CallJsMethod("onAbilityBackground", ability);
}

void JsAbilityLifecycleCallback::OnAbilityContinue(const std::shared_ptr<NativeReference> &ability)
{
    CallJsMethod("onAbilityContinue", ability);
}

int32_t JsAbilityLifecycleCallback::Register(NativeValue *jsCallback)
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
    callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(engine_->CreateReference(jsCallback, 1)));
    return callbackId;
}

bool JsAbilityLifecycleCallback::UnRegister(int32_t callbackId)
{
    HILOG_INFO("UnRegister called, callbackId : %{public}d", callbackId);
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        HILOG_ERROR("UnRegister callbackId: %{public}d is not in callbacks_", callbackId);
        return false;
    }
    return callbacks_.erase(callbackId) == 1;
}

bool JsAbilityLifecycleCallback::IsEmpty() const
{
    return callbacks_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
