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

napi_value ResultCodeInit(napi_env env)
{
    HILOG_DEBUG("called");
    if (env == nullptr) {
        HILOG_ERROR("Invalid input parameters.");
        return nullptr;
    }

    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "RESULT_OK", CreateJsValue(env, RESULT_OK));
    napi_set_named_property(env, objValue, "RESULT_CANCEL", CreateJsValue(env, RESULT_CANCEL));

    return objValue;
}

class JsDialogRequest {
public:
    JsDialogRequest() = default;
    ~JsDialogRequest() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_INFO("JsDialogRequest::Finalizer is called");
        std::unique_ptr<JsDialogRequest>(static_cast<JsDialogRequest*>(data));
    }

    static napi_value GetRequestInfo(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsDialogRequest, OnGetRequestInfo);
    }

    static napi_value GetRequestCallback(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsDialogRequest, OnGetRequestCallback);
    }

private:
    napi_value OnGetRequestInfo(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        OHOS::AAFwk::Want want;
        if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
            HILOG_ERROR("The input want is invalid.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        sptr<IRemoteObject> callerToken = want.GetRemoteObject(RequestConstants::REQUEST_TOKEN_KEY);
        if (!callerToken) {
            HILOG_ERROR("Can not get token from target want.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        int32_t left = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_LEFT_KEY, 0);
        int32_t top = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_TOP_KEY, 0);
        int32_t width = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_WIDTH_KEY, 0);
        int32_t height = want.GetIntParam(RequestConstants::WINDOW_RECTANGLE_HEIGHT_KEY, 0);

        auto requestInfo = new RequestInfo(callerToken, left, top, width, height);
        auto jsRequestInfo = RequestInfo::WrapRequestInfo(env, requestInfo);
        if (jsRequestInfo == nullptr) {
            HILOG_ERROR("Can not wrap requestinfo from target request.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        return jsRequestInfo;
    }

    napi_value OnGetRequestCallback(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_INFO("%{public}s is called", __FUNCTION__);
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Params is not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        OHOS::AAFwk::Want want;
        if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want)) {
            HILOG_ERROR("The input want is invalid.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        sptr<IRemoteObject> remoteObj = want.GetRemoteObject(RequestConstants::REQUEST_CALLBACK_KEY);
        if (!remoteObj) {
            HILOG_ERROR("Can not get callback from target want.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        sptr<IDialogRequestCallback> callback = iface_cast<IDialogRequestCallback>(remoteObj);
        if (!callback) {
            HILOG_ERROR("Cast to IDialogRequestCallback failed.");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }

        return CreateJsDialogRequestCallback(env, callback);
    }
};

napi_value JsDialogRequestInit(napi_env env, napi_value exportObj)
{
    HILOG_DEBUG("called");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_INFO("Invalid input parameters");
        return nullptr;
    }

    std::unique_ptr<JsDialogRequest> jsDialogRequest = std::make_unique<JsDialogRequest>();
    napi_wrap(env, exportObj, jsDialogRequest.release(), JsDialogRequest::Finalizer, nullptr, nullptr);
    napi_set_named_property(env, exportObj, "ResultCode", ResultCodeInit(env));

    const char *moduleName = "JsDialogRequest";
    BindNativeFunction(env, exportObj, "getRequestInfo", moduleName, JsDialogRequest::GetRequestInfo);
    BindNativeFunction(env, exportObj, "getRequestCallback", moduleName, JsDialogRequest::GetRequestCallback);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
