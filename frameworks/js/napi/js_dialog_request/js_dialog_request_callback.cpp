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

#include "js_dialog_request_callback.h"

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace { // nameless
class JsDialogRequestCallback {
public:
    explicit JsDialogRequestCallback(const sptr<IDialogRequestCallback> remoteObj) :callback_(remoteObj) {}

    virtual ~JsDialogRequestCallback() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_DEBUG("JsDialogRequestCallback::Finalizer is called.");
        std::unique_ptr<JsDialogRequestCallback>(static_cast<JsDialogRequestCallback*>(data));
    }

    static NativeValue* SetRequestResult(NativeEngine* engine, NativeCallbackInfo* info)
    {
        if (engine == nullptr || info == nullptr) {
            HILOG_ERROR("input parameters %{public}s is nullptr", ((engine == nullptr) ? "engine" : "info"));
            return nullptr;
        }

        auto object = CheckParamsAndGetThis<JsDialogRequestCallback>(engine, info);
        if (object == nullptr) {
            HILOG_ERROR("CheckParamsAndGetThis return nullptr");
            return nullptr;
        }

        return object->OnSetRequestResult(*engine, *info);
    }

private:
    NativeValue* OnSetRequestResult(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("function called");
        if (info.argc < 1) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        if (info.argv[0]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
            HILOG_ERROR("param type mismatch!");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        NativeObject* paramObject = ConvertNativeValueTo<NativeObject>(info.argv[0]);
        NativeValue* resultCode = paramObject->GetProperty("result");
        int32_t resultCodeValue = 0;
        if (!ConvertFromJsValue(engine, resultCode, resultCodeValue)) {
            HILOG_ERROR("Convert result failed!");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        if (callback_ == nullptr) {
            HILOG_ERROR("JsDialogRequestCallback::%{public}s, callback_ is nullptr", __func__);
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }
        callback_->SendResult(resultCodeValue);
        HILOG_INFO("function called end.");
        return engine.CreateUndefined();
    }

private:
    sptr<IDialogRequestCallback> callback_;
};
} // nameless

NativeValue* CreateJsDialogRequestCallback(NativeEngine &engine, const sptr<IDialogRequestCallback> &remoteObj)
{
    HILOG_INFO("CreateJsDialogRequestCallback");
    if (!remoteObj) {
        HILOG_ERROR("remoteObj is invalid.");
        return engine.CreateUndefined();
    }

    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is invalid.");
        return engine.CreateUndefined();
    }

    auto jsDialogRequestCallback = std::make_unique<JsDialogRequestCallback>(remoteObj);
    object->SetNativePointer(jsDialogRequestCallback.release(), JsDialogRequestCallback::Finalizer, nullptr);
    const char *moduleName = "JsDialogRequestCallback";
    BindNativeFunction(engine, *object, "setRequestResult", moduleName, JsDialogRequestCallback::SetRequestResult);

    HILOG_INFO("CreateJsDialogRequestCallback end");
    return objValue;
}
} // AbilityRuntime
} // OHOS
