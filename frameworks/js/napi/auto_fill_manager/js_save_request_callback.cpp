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

#include "js_save_request_callback.h"

#include "hilog_wrapper.h"
#include "js_auto_fill_manager.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string METHOD_ON_SAVE_REQUEST_SUCCESS = "onSuccess";
const std::string METHOD_ON_SAVE_REQUEST_FAILED = "onFailure";
} // namespace
JsSaveRequestCallback::JsSaveRequestCallback(napi_env env) : env_(env) {}

JsSaveRequestCallback::~JsSaveRequestCallback() {}

void JsSaveRequestCallback::OnSaveRequestSuccess()
{
    HILOG_DEBUG("Called.");
    JSCallFunction(METHOD_ON_SAVE_REQUEST_SUCCESS);
}

void JsSaveRequestCallback::OnSaveRequestFailed()
{
    HILOG_DEBUG("Called.");
    JSCallFunction(METHOD_ON_SAVE_REQUEST_FAILED);
}

void JsSaveRequestCallback::Register(napi_value value)
{
    HILOG_DEBUG("Called.");
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (IsJsCallbackEquals(callback_, value)) {
        HILOG_ERROR("The current callback already exists.");
        return;
    }

    napi_ref ref = nullptr;
    napi_create_reference(env_, value, 1, &ref);
    callback_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

void JsSaveRequestCallback::UnRegister(napi_value value)
{
    HILOG_DEBUG("Called.");
    napi_valuetype type = napi_undefined;
    napi_typeof(env_, value, &type);
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (type == napi_undefined || type == napi_null) {
        HILOG_DEBUG("jsCallback is nullptr, delete callback.");
        callback_ = nullptr;
        return;
    }

    if (IsJsCallbackEquals(callback_, value)) {
        callback_ = nullptr;
    }
}

void JsSaveRequestCallback::JSCallFunction(const std::string &methodName)
{
    NapiAsyncTask::CompleteCallback complete = [this, methodName](napi_env env, NapiAsyncTask &task, int32_t status) {
        JSCallFunctionWorker(methodName);
    };

    NapiAsyncTask::Schedule("JsSaveRequestCallback::JSCallFunction:" + methodName,
        env_,
        CreateAsyncTaskWithLastParam(env_, nullptr, nullptr, std::move(complete), nullptr));
}

void JsSaveRequestCallback::JSCallFunctionWorker(const std::string &methodName)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (callback_ == nullptr) {
        HILOG_ERROR("callback is nullptr.");
        return;
    }

    auto obj = callback_->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get value.");
        return;
    }

    napi_value funcObject;
    if (napi_get_named_property(env_, obj, methodName.c_str(), &funcObject) != napi_ok) {
        HILOG_ERROR("Get function by name failed.");
        return;
    }

    napi_call_function(env_, obj, funcObject, 0, NULL, nullptr);
}

bool JsSaveRequestCallback::IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, napi_value value)
{
    if (callback == nullptr) {
        HILOG_ERROR("Invalid jsCallback.");
        return false;
    }

    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object.");
        return false;
    }

    bool result = false;
    if (napi_strict_equals(env_, object, value, &result) != napi_ok) {
        HILOG_ERROR("Object does not match value.");
        return false;
    }

    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS