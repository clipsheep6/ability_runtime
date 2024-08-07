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

#include "js_dialog_request_callback.h"

#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace { // nameless
class JsDialogRequestCallback {
public:
    explicit JsDialogRequestCallback(const sptr<IDialogRequestCallback> remoteObj) :callback_(remoteObj) {}

    virtual ~JsDialogRequestCallback() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGD(AAFwkTag::DIALOG, "call");
        std::unique_ptr<JsDialogRequestCallback>(static_cast<JsDialogRequestCallback*>(data));
    }

    static napi_value SetRequestResult(napi_env env, napi_callback_info info)
    {
        if (env == nullptr || info == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "null %{public}s", ((env == nullptr) ? "env" : "info"));
            return nullptr;
        }

        GET_NAPI_INFO_AND_CALL(env, info, JsDialogRequestCallback, OnSetRequestResult);
    }

private:
    napi_value OnSetRequestResult(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGI(AAFwkTag::DIALOG, "call");
        if (info.argc < 1) {
            TAG_LOGE(AAFwkTag::DIALOG, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckTypeForNapiValue(env, info.argv[0], napi_object)) {
            TAG_LOGE(AAFwkTag::DIALOG, "param type error");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        napi_value resultCode = nullptr;
        napi_get_named_property(env, info.argv[0], "result", &resultCode);
        int32_t resultCodeValue = 0;
        if (!ConvertFromJsValue(env, resultCode, resultCodeValue)) {
            TAG_LOGE(AAFwkTag::DIALOG, "Convert result failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        AAFwk::Want wantValue;
        napi_value jWant = nullptr;
        napi_get_named_property(env, info.argv[0], "want", &jWant);
        if (jWant != nullptr && CheckTypeForNapiValue(env, jWant, napi_object)) {
            AppExecFwk::UnwrapWant(env, jWant, wantValue);
        } else {
            TAG_LOGW(AAFwkTag::DIALOG, "invalid jWant");
        }

        if (callback_ == nullptr) {
            TAG_LOGE(AAFwkTag::DIALOG, "null callback_");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        callback_->SendResult(resultCodeValue, wantValue);
        TAG_LOGI(AAFwkTag::DIALOG, "end");
        return CreateJsUndefined(env);
    }

private:
    sptr<IDialogRequestCallback> callback_;
};
} // nameless

napi_value CreateJsDialogRequestCallback(napi_env env, const sptr<IDialogRequestCallback> &remoteObj)
{
    TAG_LOGI(AAFwkTag::DIALOG, "call");
    if (!remoteObj) {
        TAG_LOGE(AAFwkTag::DIALOG, "remoteObj invalid.");
        return CreateJsUndefined(env);
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "object invalid.");
        return CreateJsUndefined(env);
    }

    auto jsDialogRequestCallback = std::make_unique<JsDialogRequestCallback>(remoteObj);
    napi_wrap(env, objValue, jsDialogRequestCallback.release(), JsDialogRequestCallback::Finalizer, nullptr, nullptr);
    const char *moduleName = "JsDialogRequestCallback";
    BindNativeFunction(env, objValue, "setRequestResult", moduleName, JsDialogRequestCallback::SetRequestResult);

    TAG_LOGI(AAFwkTag::DIALOG, "end");
    return objValue;
}
} // AbilityRuntime
} // OHOS
