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
AbilityLifecycleCallback::AbilityLifecycleCallback(NativeEngine &engine)
    : engine_(engine)
{
}

AbilityLifecycleCallback::~AbilityLifecycleCallback() = default;

void AbilityLifecycleCallback::OnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("AbilityLifecycleCallback onAbilityCreate");
    CallJsMethod("onAbilityCreate", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityWindowStageCreate", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityWindowStageDestroy", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityDestroy", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityForeground", abilityObj);
}

void AbilityLifecycleCallback::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityBackground", abilityObj);
}
void AbilityLifecycleCallback::OnAbilityContinue(const std::weak_ptr<NativeReference> &abilityObj)
{
    CallJsMethod("onAbilityContinue", abilityObj);
}

void AbilityLifecycleCallback::SetJsCallback(NativeValue *jsCallback)
{
    jsCallback_ = std::shared_ptr<NativeReference>(engine_.CreateReference(jsCallback, 1));
}

void AbilityLifecycleCallback::CallJsMethod(
    const std::string &methodName, const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("methodName = %{public}s", methodName.c_str());
    // js callback should run in js thread
    std::unique_ptr<AsyncTask::CompleteCallback> complete =
        std::make_unique<AsyncTask::CompleteCallback>(
            [abilityLifecycleCallback = this, methodName, abilityObj](
            NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (abilityLifecycleCallback) {
                abilityLifecycleCallback->CallJsMethodInner(methodName, abilityObj);
            }
        });
    NativeReference *callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule(engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void AbilityLifecycleCallback::CallJsMethodInner(
    const std::string &methodName, const std::weak_ptr<NativeReference> &abilityObj)
{
    if (!jsCallback_) {
        HILOG_ERROR("Invalid jsCallback");
        return;
    }
    NativeValue *value = jsCallback_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }
    NativeValue *method = obj->GetProperty(methodName.c_str());
    if (method == nullptr || method->TypeOf() == NATIVE_UNDEFINED) {
        HILOG_ERROR("Failed to get %{public}s from object", methodName.c_str());
        return;
    }
    auto nativeAbilityObj = engine_.CreateNull();
    if (!abilityObj.expired()) {
        nativeAbilityObj = abilityObj.lock()->Get();
    }
    NativeValue *argv[] = { nativeAbilityObj };
    engine_.CallFunction(value, method, argv, ArraySize(argv));
}
}  // namespace AbilityRuntime
}  // namespace OHOS