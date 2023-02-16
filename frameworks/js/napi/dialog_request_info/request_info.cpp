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

namespace OHOS {
namespace AbilityRuntime {

RequestInfo::RequestInfo(const sptr<IRemoteObject> &token)
{
    callerToken_ = token;
}

RequestInfo::~RequestInfo()
{
}

sptr<IRemoteObject> RequestInfo::GetToken()
{
    return callerToken_;
}

NativeValue* RequestInfo::WrapRequestInfo(NativeEngine &engine, RequestInfo *request)
{
    HILOG_DEBUG("WrapRequestInfo called.");
    if (request == nullptr) {
        HILOG_ERROR("request is nullptr.");
        return nullptr;
    }

    NativeCallback callback = [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* {
        return info->thisVar;
    };

    NativeValue* requestInfoClass = engine.DefineClass("RequestInfoClass", callback, nullptr, nullptr, 0);
    NativeValue* result = engine.CreateInstance(requestInfoClass, nullptr, 0);
    if (result == nullptr) {
        HILOG_ERROR("create instance failed.");
        return nullptr;
    }

    NativeObject* nativeObject = reinterpret_cast<NativeObject*>(result->GetInterface(NativeObject::INTERFACE_ID));
    if (nativeObject == nullptr) {
        HILOG_ERROR("get nativeObject failed.");
        return nullptr;
    }

    NativeFinalize nativeFinalize = [](NativeEngine* engine, void* data, void* hint) {
        HILOG_INFO("Js RequestInfo finalizer is called");
        auto requestInfo = static_cast<RequestInfo*>(data);
        if (requestInfo) {
            delete requestInfo;
            requestInfo = nullptr;
        }
    };

    nativeObject->SetNativePointer(reinterpret_cast<void*>(request), nativeFinalize, nullptr);
    return result;
}

std::shared_ptr<RequestInfo> RequestInfo::UnwrapRequestInfo(NativeEngine &engine, NativeValue *jsParam)
{
    HILOG_INFO("UnwrapRequestInfo called.");
    if (jsParam == nullptr) {
        HILOG_ERROR("jsParam is nullptr");
        return nullptr;
    }

    if (jsParam->TypeOf() != NATIVE_OBJECT) {
        HILOG_ERROR("UnwrapRequestInfo jsParam type error!");
        return nullptr;
    }

    NativeObject *nativeObject = reinterpret_cast<NativeObject*>(jsParam->GetInterface(NativeObject::INTERFACE_ID));
    if (nativeObject == nullptr) {
        HILOG_ERROR("UnwrapRequestInfo reinterpret_cast failed!");
        return nullptr;
    }
    HILOG_INFO("UnwrapRequestInfo success.");

    RequestInfo *info = static_cast<RequestInfo*>(nativeObject->GetNativePointer());
    return std::make_shared<RequestInfo>(*info);
}
}  // namespace AbilityRuntime
}  // namespace OHOS