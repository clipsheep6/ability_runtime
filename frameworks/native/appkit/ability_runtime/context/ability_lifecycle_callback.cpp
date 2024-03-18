/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
JsAbilityLifecycleCallback::JsAbilityLifecycleCallback(napi_env env)
    : env_(env)
{
}

int32_t JsAbilityLifecycleCallback::serialNumber_ = 0;

void JsAbilityLifecycleCallback::CallJsMethodInnerCommon(const std::string &methodName,
    const std::shared_ptr<NativeReference> &ability, const std::shared_ptr<NativeReference> &windowStage,
    const std::map<int32_t, std::shared_ptr<NativeReference>> callbacks)
{
    auto nativeAbilityObj = CreateJsNull(env_);
    if (ability != nullptr) {
        nativeAbilityObj = ability->GetNapiValue();
    }

    bool isWindowStage = false;
    auto nativeWindowStageObj = CreateJsNull(env_);
    if (windowStage != nullptr) {
        nativeWindowStageObj = windowStage->GetNapiValue();
        isWindowStage = true;
    }

    for (auto &callback : callbacks) {
        if (!callback.second) {
            TAG_LOGE(AAFwkTag::CONTEXT, "CallJsMethodInnerCommon, Invalid jsCallback");
            return;
        }

        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::CONTEXT, "CallJsMethodInnerCommon, Failed to get object");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName.data(), &method);
        if (method == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "CallJsMethodInnerCommon, Failed to get %{public}s from object",
                methodName.data());
            return;
        }

        if (!isWindowStage) {
            napi_value argv[] = { nativeAbilityObj };
            napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
        } else {
            napi_value argv[] = { nativeAbilityObj, nativeWindowStageObj };
            napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
        }
    }
}

void JsAbilityLifecycleCallback::CallJsMethod(
    const std::string &methodName, const std::shared_ptr<NativeReference> &ability)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CallJsMethod methodName = %{public}s", methodName.c_str());
    if (!ability) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability is nullptr");
        return;
    }
    HandleScope handleScope(env_);
    CallJsMethodInnerCommon(methodName, ability, nullptr, callbacks_);
    CallJsMethodInnerCommon(methodName, ability, nullptr, callbacksSync_);
}

void JsAbilityLifecycleCallback::CallWindowStageJsMethod(const std::string &methodName,
    const std::shared_ptr<NativeReference> &ability, const std::shared_ptr<NativeReference> &windowStage)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "CallWindowStageJsMethod methodName = %{public}s", methodName.c_str());
    if (!ability || !windowStage) {
        TAG_LOGE(AAFwkTag::CONTEXT, "ability or windowStage is nullptr");
        return;
    }
    HandleScope handleScope(env_);
    CallJsMethodInnerCommon(methodName, ability, windowStage, callbacks_);
    CallJsMethodInnerCommon(methodName, ability, windowStage, callbacksSync_);
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

int32_t JsAbilityLifecycleCallback::Register(napi_value jsCallback, bool isSync)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "enter");
    if (env_ == nullptr) {
        return -1;
    }
    int32_t callbackId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsCallback, 1, &ref);
    if (isSync) {
        callbacksSync_.emplace(callbackId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    } else {
        callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    }
    return callbackId;
}

bool JsAbilityLifecycleCallback::UnRegister(int32_t callbackId, bool isSync)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "UnRegister called, callbackId : %{public}d", callbackId);
    if (isSync) {
        auto it = callbacksSync_.find(callbackId);
        if (it == callbacksSync_.end()) {
            TAG_LOGE(AAFwkTag::CONTEXT, "UnRegister callbackId: %{public}d is not in callbacksSync_", callbackId);
            return false;
        }
        return callbacksSync_.erase(callbackId) == 1;
    }
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        TAG_LOGE(AAFwkTag::CONTEXT, "UnRegister callbackId: %{public}d is not in callbacks_", callbackId);
        return false;
    }
    return callbacks_.erase(callbackId) == 1;
}

bool JsAbilityLifecycleCallback::IsEmpty() const
{
    return callbacks_.empty() && callbacksSync_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
