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

#include "js_auto_save_request_callback.h"

#include "hilog_tag_wrapper.h"
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
JsAutoSaveRequestCallback::JsAutoSaveRequestCallback(
    napi_env env, int32_t instanceId, AutoFillManagerFunc autoFillManagerFunc)
    : env_(env), instanceId_(instanceId), autoFillManagerFunc_(autoFillManagerFunc) {}

JsAutoSaveRequestCallback::~JsAutoSaveRequestCallback() {}

void JsAutoSaveRequestCallback::OnSaveRequestSuccess()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    JSCallFunction(METHOD_ON_SAVE_REQUEST_SUCCESS);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void JsAutoSaveRequestCallback::OnSaveRequestFailed()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    JSCallFunction(METHOD_ON_SAVE_REQUEST_FAILED);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void JsAutoSaveRequestCallback::Register(napi_value value)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "Called.");
    if (IsJsCallbackEquals(callback_, value)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "The current callback already exists.");
        return;
    }

    napi_ref ref = nullptr;
    napi_create_reference(env_, value, 1, &ref);
    callback_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

void JsAutoSaveRequestCallback::JSCallFunction(const std::string &methodName)
{
    auto thisPtr = shared_from_this();
    NapiAsyncTask::CompleteCallback complete =
        [thisPtr, methodName](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (thisPtr) {
                thisPtr->JSCallFunctionWorker(methodName);
            }
        };

    NapiAsyncTask::Schedule("JsAutoSaveRequestCallback::JSCallFunction:" + methodName,
        env_,
        CreateAsyncTaskWithLastParam(env_, nullptr, nullptr, std::move(complete), nullptr));
}

void JsAutoSaveRequestCallback::JSCallFunctionWorker(const std::string &methodName)
{
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "callback is nullptr.");
        return;
    }

    auto obj = callback_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Failed to get value.");
        return;
    }

    napi_value funcObject;
    if (napi_get_named_property(env_, obj, methodName.c_str(), &funcObject) != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Get function by name failed.");
        return;
    }

    napi_call_function(env_, obj, funcObject, 0, NULL, nullptr);
}

bool JsAutoSaveRequestCallback::IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, napi_value value)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Invalid jsCallback.");
        return false;
    }

    auto object = callback->GetNapiValue();
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Failed to get object.");
        return false;
    }

    bool result = false;
    if (napi_strict_equals(env_, object, value, &result) != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object does not match value.");
        return false;
    }

    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS