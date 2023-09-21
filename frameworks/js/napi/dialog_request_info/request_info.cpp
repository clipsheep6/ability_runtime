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

#include "request_info.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {

RequestInfo::RequestInfo(const sptr<IRemoteObject> &token, int32_t left, int32_t top, int32_t width, int32_t height)
{
    callerToken_ = token;
    left_ = left;
    top_ = top;
    width_ = width;
    height_ = height;
}

RequestInfo::~RequestInfo()
{
}

sptr<IRemoteObject> RequestInfo::GetToken()
{
    return callerToken_;
}

napi_value RequestInfo::WrapRequestInfo(napi_env env, RequestInfo *request)
{
    HILOG_DEBUG("WrapRequestInfo called.");
    if (request == nullptr) {
        HILOG_ERROR("request is nullptr.");
        return nullptr;
    }

    auto callback = [](napi_env env, napi_callback_info info) -> napi_value {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        return thisVar;
    };

    napi_value requestInfoClass = nullptr;
    napi_define_class(
        env, "RequestInfoClass", NAPI_AUTO_LENGTH, callback, nullptr, 0, nullptr, &requestInfoClass);
    napi_value result = nullptr;
    napi_new_instance(env, requestInfoClass, 0, nullptr, &result);
    if (result == nullptr) {
        HILOG_ERROR("create instance failed.");
        return nullptr;
    }

    if (!AppExecFwk::IsTypeForNapiValue(env, result, napi_object)) {
        HILOG_ERROR("UnwrapRequestInfo result type error!");
        return nullptr;
    }

    auto nativeFinalize = [](napi_env env, void* data, void* hint) {
        HILOG_INFO("Js RequestInfo finalizer is called");
        auto requestInfo = static_cast<RequestInfo*>(data);
        if (requestInfo) {
            delete requestInfo;
            requestInfo = nullptr;
        }
    };
    napi_wrap(env, result, request, nativeFinalize, nullptr, nullptr);
    napi_set_named_property(env, result, "windowRect",
        CreateJsWindowRect(env, request->left_, request->top_, request->width_, request->height_));
    return result;
}

napi_value RequestInfo::CreateJsWindowRect(
    napi_env env, int32_t left, int32_t top, int32_t width, int32_t height)
{
    HILOG_DEBUG("left: %{public}d, top: %{public}d, width: %{public}d, height: %{public}d",
        left, top, width, height);
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }
    napi_set_named_property(env, objValue, "left", CreateJsValue(env, left));
    napi_set_named_property(env, objValue, "top", CreateJsValue(env, top));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, width));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, height));
    return objValue;
}

std::shared_ptr<RequestInfo> RequestInfo::UnwrapRequestInfo(napi_env env, napi_value jsParam)
{
    HILOG_INFO("UnwrapRequestInfo called.");
    if (jsParam == nullptr) {
        HILOG_ERROR("jsParam is nullptr");
        return nullptr;
    }

    if (!AppExecFwk::IsTypeForNapiValue(env, jsParam, napi_object)) {
        HILOG_ERROR("UnwrapRequestInfo jsParam type error!");
        return nullptr;
    }
    void* result = nullptr;
    napi_unwrap(env, jsParam, &result);
    RequestInfo *info = static_cast<RequestInfo*>(result);
    return std::make_shared<RequestInfo>(*info);
}

}  // namespace AbilityRuntime
}  // namespace OHOS