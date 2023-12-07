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

#include "js_ability_foreground_state_observer.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}
JSAbilityForegroundStateObserver::JSAbilityForegroundStateObserver(napi_env env) : env_(env) {}

void JSAbilityForegroundStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_DEBUG("Called.");
    if (!valid_) {
        HILOG_ERROR("The app manager may has destoryed.");
        return;
    }
    wptr<JSAbilityForegroundStateObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [jsObserver, abilityStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAbilityForegroundStateObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr == nullptr) {
                HILOG_ERROR("Js Observer Sptr is nullptr.");
                return;
            }
            jsObserverSptr->HandleOnAbilityStateChanged(abilityStateData);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JSAbilityForegroundStateObserver::OnAbilityStateChanged", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JSAbilityForegroundStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(mutexlock);
    for (auto &item : jsObserverObjectSet_) {
        if (item == nullptr) {
            continue;
        }
        napi_value obj = item->GetNapiValue();
        napi_value argv[] = { CreateJsAbilityStateData(env_, abilityStateData) };
        CallJsFunction(obj, "onAbilityStateChanged", argv, ARGC_ONE);
    }
}

void JSAbilityForegroundStateObserver::CallJsFunction(
    const napi_value &value, const char *methodName, const napi_value *argv, const size_t argc)
{
    HILOG_DEBUG("Begin.");
    if (value == nullptr) {
        HILOG_ERROR("Value is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        HILOG_ERROR("Get name from object Failed.");
        return;
    }
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env_, value, method, argc, argv, &callResult);
    if (status != napi_ok) {
        HILOG_ERROR("Call Js Function failed %{public}d.", status);
    }
    HILOG_DEBUG("End.");
}

bool JSAbilityForegroundStateObserver::IsObserverObjectExsit(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return false;
    }

    if (GetObserverObject(jsObserverObject) == nullptr) {
        HILOG_DEBUG("Observer is not exists.");
        return false;
    }
    return true;
}

void JSAbilityForegroundStateObserver::AddJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return;
    }

    if (IsObserverObjectExsit(jsObserverObject)) {
        HILOG_DEBUG("Observer is exists.");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    std::lock_guard<std::mutex> lock(mutexlock);
    jsObserverObjectSet_.emplace(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
}

void JSAbilityForegroundStateObserver::RemoveJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return;
    }

    auto observer = GetObserverObject(jsObserverObject);
    std::lock_guard<std::mutex> lock(mutexlock);
    if (observer != nullptr) {
        jsObserverObjectSet_.erase(observer);
    }
}

void JSAbilityForegroundStateObserver::RemoveAllJsObserverObject()
{
    std::lock_guard<std::mutex> lock(mutexlock);
    if (!jsObserverObjectSet_.empty()) {
        jsObserverObjectSet_.clear();
    }
}

std::shared_ptr<NativeReference> JSAbilityForegroundStateObserver::GetObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutexlock);
    for (auto &observer : jsObserverObjectSet_) {
        if (observer == nullptr) {
            HILOG_ERROR("Invalid observer.");
            continue;
        }

        napi_value value = observer->GetNapiValue();
        if (value == nullptr) {
            HILOG_ERROR("Failed to get object.");
            continue;
        }

        bool isEqual = false;
        napi_strict_equals(env_, value, jsObserverObject, &isEqual);
        if (isEqual) {
            return observer;
        }
    }
    return nullptr;
}

void JSAbilityForegroundStateObserver::SetValid(const bool valid)
{
    valid_ = valid;
}

bool JSAbilityForegroundStateObserver::IsEmpty()
{
    return jsObserverObjectSet_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS
