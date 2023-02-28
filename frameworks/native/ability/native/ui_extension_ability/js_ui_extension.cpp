/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_ui_extension.h"

#include "ability_info.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

NativeValue *AttachUIExtensionContext(NativeEngine *engine, void *value, void *)
{
    HILOG_DEBUG("AttachUIExtensionContext");
    if (value == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }
    NativeValue *object = JsUIExtensionContext::CreateJsUIExtensionContext(*engine, ptr);
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(engine,
        "application.UIExtensionContext", &object, 1)->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachUIExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext> *>(data);
        }, nullptr);
    return contextObj;
}

JsUIExtension* JsUIExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsUIExtension(static_cast<JsRuntime&>(*runtime));
}

JsUIExtension::JsUIExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsUIExtension::~JsUIExtension() = default;

void JsUIExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    UIExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("JsUIExtension Init abilityInfo srcEntrance is empty");
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsUIExtension object");
        return;
    }

    BindContext(engine, obj);

    SetExtensionCommon(
        JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));
}

void JsUIExtension::BindContext(NativeEngine& engine, NativeObject* obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_DEBUG("BindContext CreateJsUIExtensionContext.");
    NativeValue* contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(engine, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(&engine, "application.UIExtensionContext",
        &contextObj, ARGC_ONE);
    contextObj = shellContextRef_->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachUIExtensionContext,
        workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);

    nativeObj->SetNativePointer(workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        }, nullptr);

    HILOG_DEBUG("Init end.");
}

void JsUIExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    HILOG_DEBUG("JsUIExtension OnStart begin.");
    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    HILOG_DEBUG("JsUIExtension OnStart end.");
}

void JsUIExtension::OnStop()
{
    UIExtension::OnStop();
    HILOG_DEBUG("JsUIExtension OnStop begin.");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_WARN("The ui extension connection is not disconnected.");
    }
    HILOG_DEBUG("JsUIExtension OnStop end.");
}

sptr<IRemoteObject> JsUIExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    NativeValue *result = CallOnConnect(want);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(
        reinterpret_cast<napi_env>(nativeEngine), reinterpret_cast<napi_value>(result));
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj is nullptr.");
    }
    return remoteObj;
}

void JsUIExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("JsUIExtension OnDisconnect begin.");
    CallOnDisconnect(want, false);
    HILOG_DEBUG("JsUIExtension OnDisconnect end.");
}

void JsUIExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    HILOG_DEBUG("JsUIExtension OnCommand begin restart=%{public}s,startId=%{public}d.",
        restart ? "true" : "false", startId);
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
    HILOG_DEBUG("JsUIExtension OnCommand end.");
}

NativeValue* JsUIExtension::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_DEBUG("JsUIExtension CallObjectMethod(%{public}s), begin", name);

    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get UIExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from UIExtension object", name);
        return nullptr;
    }
    HILOG_DEBUG("JsUIExtension CallFunction(%{public}s), success", name);
    return nativeEngine.CallFunction(value, method, argv, argc);
}

NativeValue *JsUIExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    HILOG_DEBUG("JsUIExtension CallOnConnect begin.");
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    auto* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return nullptr;
    }

    NativeValue* value = jsObj_->Get();
    auto* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get UIExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty("onConnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onConnect from UIExtension object");
        return nullptr;
    }
    NativeValue* remoteNative = nativeEngine->CallFunction(value, method, argv, ARGC_ONE);
    if (remoteNative == nullptr) {
        HILOG_ERROR("remoteNative is nullptr.");
    }
    HILOG_DEBUG("JsUIExtension CallOnConnect end.");
    return remoteNative;
}

NativeValue *JsUIExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleScope HandleScope(jsRuntime_);
    NativeEngine *nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue *nativeWant = reinterpret_cast<NativeValue *>(napiWant);
    NativeValue *argv[] = { nativeWant };
    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return nullptr;
    }

    NativeValue *value = jsObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get UIExtension object");
        return nullptr;
    }

    NativeValue *method = obj->GetProperty("onDisconnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from UIExtension object");
        return nullptr;
    }

    if (withResult) {
        return HandleScope.Escape(nativeEngine->CallFunction(value, method, argv, ARGC_ONE));
    } else {
        nativeEngine->CallFunction(value, method, argv, ARGC_ONE);
        return nullptr;
    }
}

void JsUIExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    Extension::OnConfigurationUpdated(configuration);
    HILOG_DEBUG("JsUIExtension OnConfigurationUpdated called.");

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdate", &jsConfiguration, ARGC_ONE);
}

void JsUIExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    HILOG_DEBUG("JsUIExtension Dump called.");
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    // create js array object of params
    NativeValue* arrayValue = nativeEngine.CreateArray(params.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const auto &param : params) {
        array->SetElement(index++, CreateJsValue(nativeEngine, param));
    }
    NativeValue* argv[] = { arrayValue };

    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return;
    }

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get UIExtension object");
        return;
    }

    NativeValue* method = obj->GetProperty("onDump");
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        method = obj->GetProperty("dump");
        if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
            HILOG_ERROR("Failed to get onDump from UIExtension object");
            return;
        }
    }
    NativeValue* dumpInfo = nativeEngine.CallFunction(value, method, argv, ARGC_ONE);
    if (dumpInfo == nullptr) {
        HILOG_ERROR("dumpInfo is nullptr.");
        return;
    }
    NativeArray* dumpInfoNative = ConvertNativeValueTo<NativeArray>(dumpInfo);
    if (dumpInfoNative == nullptr) {
        HILOG_ERROR("dumpInfoNative is nullptr.");
        return;
    }
    for (uint32_t i = 0; i < dumpInfoNative->GetLength(); i++) {
        std::string dumpInfoStr;
        if (!ConvertFromJsValue(nativeEngine, dumpInfoNative->GetElement(i), dumpInfoStr)) {
            HILOG_ERROR("Parse dumpInfoStr failed");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}
}
}
