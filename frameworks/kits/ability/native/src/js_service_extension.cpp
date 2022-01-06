/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "js_service_extension.h"

#include "ability_info.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_service_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

using namespace OHOS::AppExecFwk;
JsServiceExtension* JsServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsServiceExtension(static_cast<JsRuntime&>(*runtime));
}

JsServiceExtension::JsServiceExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsServiceExtension::~JsServiceExtension() = default;

void JsServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ServiceExtension::Init(record, application, handler, token);
    std::string srcPath(Extension::abilityInfo_->package);
    srcPath.append("/assets/js/");
    if (!Extension::abilityInfo_->srcPath.empty()) {
        srcPath.append(Extension::abilityInfo_->srcPath);
    }
    srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_INFO("JsServiceExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    HILOG_INFO("JsServiceExtension::Init ConvertNativeValueTo.");
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsServiceExtension object");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("JsServiceExtension::Init CreateJsServiceExtensionContext.");
    NativeValue* contextObj = CreateJsServiceExtensionContext(engine, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("application.ServiceExtensionContext", &contextObj, ARGC_ONE);
    contextObj = shellContextRef->Get();
    HILOG_INFO("JsServiceExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef.release());
    HILOG_INFO("JsServiceExtension::SetProperty.");
    obj->SetProperty("context", contextObj);
    HILOG_INFO("JsServiceExtension::Init end.");
}

void JsServiceExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    HILOG_INFO("JsServiceExtension OnStart begin..");
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsServiceExtension::OnStop()
{
    ServiceExtension::OnStop();
    HILOG_INFO("JsServiceExtension OnStop begin.");
    CallObjectMethod("onDestroy");
    HILOG_INFO("%{public}s end.", __func__);
}

sptr<IRemoteObject> JsServiceExtension::OnConnect(const AAFwk::Want &want)
{
    Extension::OnConnect(want);
    HILOG_INFO("%{public}s begin.", __func__);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty("onConnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onConnect from ServiceExtension object");
        return nullptr;
    }
    HILOG_INFO("JsServiceExtension::CallFunction onConnect, success");
    NativeValue* remoteNative = nativeEngine->CallFunction(value, method, argv, ARGC_ONE);
    if (remoteNative == nullptr) {
        HILOG_ERROR("remoteNative nullptr.");
    }
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(
        reinterpret_cast<napi_env>(nativeEngine), reinterpret_cast<napi_value>(remoteNative));
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj nullptr.");
    }
    return remoteObj;
}

void JsServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    Extension::OnDisconnect(want);
    HILOG_INFO("%{public}s begin.", __func__);
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return;
    }

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return;
    }

    NativeValue* method = obj->GetProperty("onDisconnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from ServiceExtension object");
        return;
    }
    nativeEngine->CallFunction(value, method, argv, ARGC_ONE);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    HILOG_INFO("%{public}s begin restart=%{public}s,startId=%{public}d.",
        __func__,
        restart ? "true" : "false",
        startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(reinterpret_cast<napi_env>(nativeEngine), startId, &napiStartId);
    NativeValue* nativeStartId = reinterpret_cast<NativeValue*>(napiStartId);
    NativeValue* argv[] = {nativeWant, nativeStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    HILOG_INFO("%{public}s end.", __func__);
}

NativeValue* JsServiceExtension::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsServiceExtension::CallObjectMethod(%{public}s), begin", name);

    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from ServiceExtension object", name);
        return nullptr;
    }
    HILOG_INFO("JsServiceExtension::CallFunction(%{public}s), success", name);
    return nativeEngine.CallFunction(value, method, argv, argc);
}
}
}