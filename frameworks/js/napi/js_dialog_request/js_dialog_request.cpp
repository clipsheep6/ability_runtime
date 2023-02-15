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

#include "js_dialog_request.h"

#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_dialog_request_callback.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "request_constants.h"
#include "request_info.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr int32_t RESULT_OK = 0;
constexpr int32_t RESULT_CANCEL = 1;
}

NativeValue* ResultCodeInit(NativeEngine* engine)
{
    HILOG_INFO("%{public}s is called", __FUNCTION__);
    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeValue* objValue = engine->CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("RESULT_OK", CreateJsValue(*engine, RESULT_OK));
    object->SetProperty("RESULT_CANCEL", CreateJsValue(*engine, RESULT_CANCEL));

    return objValue;
}

class JsDialogRequest {
public:
    JsDialogRequest() = default;
    ~JsDialogRequest() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsDialogRequest::Finalizer is called");
        std::unique_ptr<JsDialogRequest>(static_cast<JsDialogRequest*>(data));
    }

    static NativeValue* GetRequestInfo(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsDialogRequest* me = CheckParamsAndGetThis<JsDialogRequest>(engine, info);
        return (me != nullptr) ? me->OnGetRequestInfo(*engine, *info) : nullptr;
    }

    static NativeValue* GetRequestCallback(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsDialogRequest* me = CheckParamsAndGetThis<JsDialogRequest>(engine, info);
        return (me != nullptr) ? me->OnGetRequestCallback(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnGetRequestInfo(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        OHOS::AAFwk::Want want;
        if (!OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), want)) {
            HILOG_ERROR("The input want is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        sptr<IRemoteObject> callerToken = want.GetRemoteObject(RequestConstants::REQUEST_TOKEN_KEY);
        if (!callerToken) {
            HILOG_ERROR("Can not get token from target want.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        auto requestInfo = new RequestInfo(callerToken);
        auto jsRequestInfo = RequestInfo::WrapRequestInfo(engine, requestInfo);
        if (jsRequestInfo == nullptr) {
            HILOG_ERROR("Can not wrap requestinfo from target request.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        return jsRequestInfo;
    }

    NativeValue* OnGetRequestCallback(NativeEngine &engine, NativeCallbackInfo &info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Params is not match");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        OHOS::AAFwk::Want want;
        if (!OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[0]), want)) {
            HILOG_ERROR("The input want is invalid.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        sptr<IRemoteObject> remoteObj = want.GetRemoteObject(RequestConstants::REQUEST_CALLBACK_KEY);
        if (!remoteObj) {
            HILOG_ERROR("Can not get callback from target want.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        sptr<IDialogRequestCallback> callback = iface_cast<IDialogRequestCallback>(remoteObj);
        if (!callback) {
            HILOG_ERROR("Cast to IDialogRequestCallback failed.");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }

        return CreateJsDialogRequestCallback(engine, callback);
    }
};

NativeValue* JsDialogRequestInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsDialogRequestInit is called");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("Invalid input parameters");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsDialogRequest> jsDialogRequest = std::make_unique<JsDialogRequest>();
    object->SetNativePointer(jsDialogRequest.release(), JsDialogRequest::Finalizer, nullptr);
    object->SetProperty("ResultCode", ResultCodeInit(engine));

    const char *moduleName = "JsDialogRequest";
    BindNativeFunction(*engine, *object, "getRequestInfo", moduleName, JsDialogRequest::GetRequestInfo);
    BindNativeFunction(*engine, *object, "getRequestCallback", moduleName, JsDialogRequest::GetRequestCallback);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
