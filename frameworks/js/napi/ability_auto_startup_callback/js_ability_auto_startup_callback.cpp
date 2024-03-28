/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
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
JsAbilityAutoStartupCallBack::JsAbilityAutoStartupCallBack(napi_env env) : env_(env) {}

JsAbilityAutoStartupCallBack::~JsAbilityAutoStartupCallBack() {}

void JsAbilityAutoStartupCallBack::OnAutoStartupOn(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called.");
    JSCallFunction(info, METHOD_ON);
}

void JsAbilityAutoStartupCallBack::OnAutoStartupOff(const AutoStartupInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called.");
    JSCallFunction(info, METHOD_OFF);
}

void JsAbilityAutoStartupCallBack::Register(napi_value value)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called.");
    std::lock_guard<std::mutex> lock(mutexlock);
    for (const auto &callback : callbacks_) {
        if (IsJsCallbackEquals(callback, value)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "The current callback already exists.");
            return;
        }
    }

    napi_ref ref = nullptr;
    napi_create_reference(env_, value, 1, &ref);
    callbacks_.emplace_back(std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref)));
}

void JsAbilityAutoStartupCallBack::UnRegister(napi_value value)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "Called.");
    napi_valuetype type = napi_undefined;
    napi_typeof(env_, value, &type);
    std::lock_guard<std::mutex> lock(mutexlock);
    if (type == napi_undefined || type == napi_null) {
        TAG_LOGD(AAFwkTag::AUTO_STARTUP, "jsCallback is nullptr, delete all callback.");
        callbacks_.clear();
        return;
    }

    for (auto item = callbacks_.begin(); item != callbacks_.end();) {
        if (IsJsCallbackEquals(*item, value)) {
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
    wptr<JsAbilityAutoStartupCallBack> stub = iface_cast<JsAbilityAutoStartupCallBack>(AsObject());
    NapiAsyncTask::CompleteCallback complete = [stub, info, methodName](
                                                   napi_env env, NapiAsyncTask &task, int32_t status) {
        sptr<JsAbilityAutoStartupCallBack> obj = stub.promote();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Callback object is nullptr");
            return;
        }

        obj->JSCallFunctionWorker(info, methodName);
    };

    NapiAsyncTask::Schedule("JsAbilityAutoStartupCallBack::JSCallFunction:" + methodName, env_,
        CreateAsyncTaskWithLastParam(env_, nullptr, nullptr, std::move(complete), nullptr));
}

void JsAbilityAutoStartupCallBack::JSCallFunctionWorker(const AutoStartupInfo &info, const std::string &methodName)
{
    std::lock_guard<std::mutex> lock(mutexlock);
    for (auto callback : callbacks_) {
        if (callback == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "callback is nullptr.");
            continue;
        }

        auto obj = callback->GetNapiValue();
        if (obj == nullptr) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get value.");
            continue;
        }

        napi_value funcObject;
        if (napi_get_named_property(env_, obj, methodName.c_str(), &funcObject) != napi_ok) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Get function by name failed.");
            continue;
        }

        napi_value argv[] = { CreateJsAutoStartupInfo(env_, info) };
        napi_call_function(env_, obj, funcObject, ArraySize(argv), argv, nullptr);
    }
}

bool JsAbilityAutoStartupCallBack::IsJsCallbackEquals(const std::shared_ptr<NativeReference> &callback,
    napi_value value)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Invalid jsCallback.");
        return false;
    }

    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Failed to get object.");
        return false;
    }

    bool result = false;
    if (napi_strict_equals(env_, object, value, &result) != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Object does not match value.");
        return false;
    }

    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS