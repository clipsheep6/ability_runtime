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
#include "js_ui_extension_callback.h"
#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "ui_content.h"
#include "ws_common.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* ERROR_MSG_INNER = "Inner error.";
JsUIExtensionCallback::~JsUIExtensionCallback()
{
    if (jsCallbackObject_  == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(jsCallbackObject_.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    [](uv_work_t *work, int status) {
        if (work == nullptr) {
            return;
        }
        if (work->data == nullptr) {
            delete work;
            work = nullptr;
            return;
        }
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
        work = nullptr;
    }
}

void JsUIExtensionCallback::SetSessionId(int32_t sessionId)
{
    sessionId_ = sessionId;
}

void JsUIExtensionCallback::SetUIContent(Ace::UIContent* uiContent)
{
    uiContent_ = uiContent;
}

void JsUIExtensionCallback::SetJsCallbackObject(napi_value jsCallbackObject)
{
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsCallbackObject, 1, &ref);
    jsCallbackObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
    if (jsCallbackObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsCallbackObject_ is nullptr");
    }
}

void JsUIExtensionCallback::OnError(int32_t number)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env_ null");
        return;
    }
    // js callback should run in js thread
    std::shared_ptr<JsUIExtensionCallback> jsUIExtensionCallback = shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsUIExtensionCallback, number](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (jsUIExtensionCallback != nullptr) {
                jsUIExtensionCallback->CallJsError(number);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsUIExtensionCallback::OnError:",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    if (uiContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiContent_ null");
        return;
    }
    uiContent_->CloseModalUIExtension(sessionId_);
}

void JsUIExtensionCallback::OnRelease(int32_t code)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call, code:%{public}d", code);
    if (uiContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiContent_ null");
        return;
    }
    uiContent_->CloseModalUIExtension(sessionId_);
}

void JsUIExtensionCallback::CallJsError(int32_t number)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "call");
    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env_ is null, not call js error.");
        return;
    }
    std::string name;
    std::string message;
    if (number != static_cast<int32_t>(Rosen::WSError::WS_OK)) {
        number = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
        name = ERROR_MSG_INNER;
        message = "StartAbilityByType failed.";
    }
    napi_value nativeNumber = CreateJsValue(env_, number);
    napi_value nativeName = CreateJsValue(env_, name);
    napi_value nativeMessage = CreateJsValue(env_, message);
    if (jsCallbackObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsCallbackObject_ is nullptr");
        return;
    }
    napi_value obj = jsCallbackObject_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get js object");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onError", &method);
    if (method == nullptr || AppExecFwk::IsTypeForNapiValue(env_, method, napi_undefined)
        || AppExecFwk::IsTypeForNapiValue(env_, method, napi_null)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onError method from object");
        return;
    }

    napi_value argv[] = { nativeNumber, nativeName, nativeMessage };
    napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    TAG_LOGI(AAFwkTag::UI_EXT, "end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS