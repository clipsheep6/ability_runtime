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
#include "js_app_foreground_state_observer.h"

#include "hilog_wrapper.h"
#include "js_app_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}
JSAppForegroundStateObserver::JSAppForegroundStateObserver(napi_env env) : env_(env) {}

void JSAppForegroundStateObserver::OnAppStateChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("Called.");
    if (!valid_) {
        HILOG_ERROR("The app manager may has destoryed.");
        return;
    }
    wptr<JSAppForegroundStateObserver> self = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [self, appStateData](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JSAppForegroundStateObserver> jsObserver = self.promote();
            if (jsObserver == nullptr) {
                HILOG_ERROR("Js Observer Sptr is nullptr.");
                return;
            }
            jsObserver->HandleOnAppStateChanged(appStateData);
        });
    napi_ref callback = nullptr;
    NapiAsyncTask::Schedule("JSAppForegroundStateObserver::OnAppStateChanged", env_,
        std::make_unique<NapiAsyncTask>(callback, nullptr, std::move(complete)));
}

void JSAppForegroundStateObserver::HandleOnAppStateChanged(const AppStateData &appStateData)
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    for (auto &item : jsObserverObjectSet_) {
        napi_value obj = item->GetNapiValue();
        napi_value argv[] = { CreateJsAppStateData(env_, appStateData) };
        CallJsFunction(obj, "onAppStateChanged", argv, ARGC_ONE);
    }
}

void JSAppForegroundStateObserver::CallJsFunction(
    const napi_value value, const char *methodName, const napi_value *argv, const size_t argc)
{
    HILOG_DEBUG("Begin.");
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
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

void JSAppForegroundStateObserver::AddJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return;
    }

    if (GetObserverObject(jsObserverObject) == nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        napi_ref ref = nullptr;
        napi_create_reference(env_, jsObserverObject, 1, &ref);
        jsObserverObjectSet_.emplace(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
    } else {
        HILOG_DEBUG("Observer is exists.");
    }
}

void JSAppForegroundStateObserver::RemoveAllJsObserverObjects()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    jsObserverObjectSet_.clear();
}

void JSAppForegroundStateObserver::RemoveJsObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return;
    }

    auto observer = GetObserverObject(jsObserverObject);
    if (observer != nullptr) {
        std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
        jsObserverObjectSet_.erase(observer);
    }
}

std::shared_ptr<NativeReference> JSAppForegroundStateObserver::GetObserverObject(const napi_value &jsObserverObject)
{
    if (jsObserverObject == nullptr) {
        HILOG_ERROR("Observer is null.");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
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

void JSAppForegroundStateObserver::SetValid(bool valid)
{
    valid_ = valid;
}

bool JSAppForegroundStateObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsObserverObjectSetLock_);
    return jsObserverObjectSet_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS
