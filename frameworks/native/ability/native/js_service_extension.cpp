/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_handler.h"
#include "ability_info.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_service_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#ifdef SUPPORT_GRAPHICS
#include "window_scene.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

namespace {
NativeValue *PromiseCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (info == nullptr || info->functionInfo == nullptr || info->functionInfo->data == nullptr) {
        HILOG_ERROR("PromiseCallback, Invalid input info.");
        return nullptr;
    }
    void *data = info->functionInfo->data;
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    info->functionInfo->data = nullptr;
    return nullptr;
}

NativeValue *OnConnectPromiseCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (info == nullptr || info->functionInfo == nullptr || info->functionInfo->data == nullptr) {
        HILOG_ERROR("PromiseCallback, Invalid input info.");
        return nullptr;
    }
    void *data = info->functionInfo->data;
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *>(data);
    sptr<IRemoteObject> service = nullptr;
    if (info->argc > 0) {
        service = NAPI_ohos_rpc_getNativeRemoteObject(reinterpret_cast<napi_env>(engine),
            reinterpret_cast<napi_value>(info->argv[0]));
    }
    callbackInfo->Call(service);
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
    info->functionInfo->data = nullptr;
    return nullptr;
}
}

using namespace OHOS::AppExecFwk;

NativeValue *AttachServiceExtensionContext(NativeEngine *engine, void *value, void *)
{
    HILOG_INFO("call");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<ServiceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue *object = CreateJsServiceExtensionContext(*engine, ptr);
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(engine,
        "application.ServiceExtensionContext", &object, 1)->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachServiceExtensionContext,
        value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<ServiceExtensionContext>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<ServiceExtensionContext> *>(data);
        }, nullptr);
    return contextObj;
}

JsServiceExtension* JsServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsServiceExtension(static_cast<JsRuntime&>(*runtime));
}

JsServiceExtension::JsServiceExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsServiceExtension::~JsServiceExtension()
{
    HILOG_DEBUG("Js service extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ServiceExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        HILOG_ERROR("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_DEBUG("JsStaticSubscriberExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
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

    BindContext(engine, obj);

    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));

    handler_ = handler;
    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            HILOG_DEBUG("Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }

#ifdef SUPPORT_GRAPHICS
    HILOG_INFO("RegisterDisplayListener");
    std::shared_ptr<JsServiceExtension> jsServiceExtension = std::static_pointer_cast<JsServiceExtension>(
        shared_from_this());
    displayListener_ = new (std::nothrow) JsServiceExtensionDisplayListener(jsServiceExtension);
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(displayListener_);
#endif
}

void JsServiceExtension::BindContext(NativeEngine& engine, NativeObject* obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("call");
    NativeValue* contextObj = CreateJsServiceExtensionContext(engine, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ServiceExtensionContext",
        &contextObj, ARGC_ONE);
    contextObj = shellContextRef_->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ServiceExtensionContext>(context);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachServiceExtensionContext,
        workContext, nullptr);
    HILOG_INFO("JsServiceExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    HILOG_INFO("JsServiceExtension::SetProperty.");
    obj->SetProperty("context", contextObj);
    HILOG_INFO("Set service extension context");

    nativeObj->SetNativePointer(workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<ServiceExtensionContext>*>(data);
        }, nullptr);

    HILOG_INFO("JsServiceExtension::Init end.");
}

void JsServiceExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    HILOG_INFO("call");

    auto context = GetContext();
    if (context != nullptr) {
        int displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, Rosen::WindowScene::DEFAULT_DISPLAY_ID);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
    }

    HandleScope handleScope(jsRuntime_);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();

    // display config has changed, need update context.config
    JsExtensionContext::ConfigurationUpdated(nativeEngine, shellContextRef_, context->GetConfiguration());

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    HILOG_INFO("ok");
}

void JsServiceExtension::OnStop()
{
    ServiceExtension::OnStop();
    HILOG_INFO("call");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_INFO("The service extension connection is not disconnected.");
    }
    HILOG_INFO("ok");
}

sptr<IRemoteObject> JsServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    NativeValue *result = CallOnConnect(want);
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(
        reinterpret_cast<napi_env>(nativeEngine), reinterpret_cast<napi_value>(result));
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj nullptr.");
    }
    return remoteObj;
}

sptr<IRemoteObject> JsServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HandleScope handleScope(jsRuntime_);
    NativeEngine *nativeEngine = &jsRuntime_.GetNativeEngine();
    NativeValue *result = CallOnConnect(want);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        sptr<IRemoteObject> remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(reinterpret_cast<napi_env>(nativeEngine),
            reinterpret_cast<napi_value>(result));
        if (remoteObj == nullptr) {
            HILOG_ERROR("remoteObj nullptr.");
        }
        return remoteObj;
    }

    bool callResult = false;
    do {
        auto *retObj = ConvertNativeValueTo<NativeObject>(result);
        if (retObj == nullptr) {
            HILOG_ERROR("CallPromise, Failed to convert native value to NativeObject.");
            break;
        }
        NativeValue *then = retObj->GetProperty("then");
        if (then == nullptr) {
            HILOG_ERROR("CallPromise, Failed to get property: then.");
            break;
        }
        if (!then->IsCallable()) {
            HILOG_ERROR("CallPromise, property then is not callable.");
            break;
        }
        auto promiseCallback = nativeEngine->CreateFunction("promiseCallback", strlen("promiseCallback"),
            OnConnectPromiseCallback, callbackInfo);
        NativeValue *argv[1] = { promiseCallback };
        nativeEngine->CallFunction(result, then, argv, 1);
        callResult = true;
    } while (false);

    if (!callResult) {
        HILOG_ERROR("Failed to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }
    return nullptr;
}

void JsServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("%{public}s begin.", __func__);
    CallOnDisconnect(want, false);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void JsServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("%{public}s begin.", __func__);
    NativeValue *result = CallOnDisconnect(want, true);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        return;
    }
    bool callResult = CallPromise(result, callbackInfo);
    if (!callResult) {
        HILOG_ERROR("Failed to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }

    HILOG_DEBUG("%{public}s end.", __func__);
}

void JsServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    HILOG_INFO("restart=%{public}s,startId=%{public}d.",
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
    HILOG_INFO("ok");
}

NativeValue* JsServiceExtension::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("CallObjectMethod(%{public}s)", name);

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
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from ServiceExtension object", name);
        return nullptr;
    }
    HILOG_INFO("CallFunction(%{public}s) ok", name);
    return nativeEngine.CallFunction(value, method, argv, argc);
}

void JsServiceExtension::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

NativeValue *JsServiceExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    HILOG_DEBUG("call");
    NativeEngine* nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    auto* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    NativeValue* value = jsObj_->Get();
    auto* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty("onConnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onConnect from ServiceExtension object");
        return nullptr;
    }
    NativeValue* remoteNative = nativeEngine->CallFunction(value, method, argv, ARGC_ONE);
    if (remoteNative == nullptr) {
        HILOG_ERROR("remoteNative nullptr.");
    }
    HILOG_INFO("ok");
    return remoteNative;
}

NativeValue *JsServiceExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    NativeEngine *nativeEngine = &jsRuntime_.GetNativeEngine();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue *nativeWant = reinterpret_cast<NativeValue *>(napiWant);
    NativeValue *argv[] = { nativeWant };
    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    NativeValue *value = jsObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    NativeValue *method = obj->GetProperty("onDisconnect");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from ServiceExtension object");
        return nullptr;
    }

    if (withResult) {
        return handleEscape.Escape(nativeEngine->CallFunction(value, method, argv, ARGC_ONE));
    } else {
        nativeEngine->CallFunction(value, method, argv, ARGC_ONE);
        return nullptr;
    }
}

bool JsServiceExtension::CheckPromise(NativeValue *result)
{
    if (result == nullptr) {
        HILOG_DEBUG("CheckPromise, result is null, no need to call promise.");
        return false;
    }
    if (!result->IsPromise()) {
        HILOG_DEBUG("CheckPromise, result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

bool JsServiceExtension::CallPromise(NativeValue *result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto *retObj = ConvertNativeValueTo<NativeObject>(result);
    if (retObj == nullptr) {
        HILOG_ERROR("CallPromise, Failed to convert native value to NativeObject.");
        return false;
    }
    NativeValue *then = retObj->GetProperty("then");
    if (then == nullptr) {
        HILOG_ERROR("CallPromise, Failed to get property: then.");
        return false;
    }
    if (!then->IsCallable()) {
        HILOG_ERROR("CallPromise, property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    auto promiseCallback = nativeEngine.CreateFunction("promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo);
    NativeValue *argv[1] = { promiseCallback };
    nativeEngine.CallFunction(result, then, argv, 1);
    return true;
}

void JsServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    ServiceExtension::OnConfigurationUpdated(configuration);
    HILOG_INFO("call");
    auto contextConfig = GetContext()->GetConfiguration();
    if (contextConfig != nullptr) {
        HILOG_DEBUG("Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        HILOG_DEBUG("Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void JsServiceExtension::ConfigurationUpdated()
{
    HILOG_DEBUG("called.");
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, ARGC_ONE);
    CallObjectMethod("onConfigurationUpdate", &jsConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);
}

void JsServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    HILOG_INFO("call");
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
        HILOG_WARN("Not found ServiceExtension.js");
        return;
    }

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return;
    }

    NativeValue* method = obj->GetProperty("onDump");
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        method = obj->GetProperty("dump");
        if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
            HILOG_ERROR("Failed to get onConnect from ServiceExtension object");
            return;
        }
    }
    HILOG_INFO("JsServiceExtension::CallFunction onConnect, success");
    NativeValue* dumpInfo = nativeEngine.CallFunction(value, method, argv, ARGC_ONE);
    if (dumpInfo == nullptr) {
        HILOG_ERROR("dumpInfo nullptr.");
        return;
    }
    NativeArray* dumpInfoNative = ConvertNativeValueTo<NativeArray>(dumpInfo);
    if (dumpInfoNative == nullptr) {
        HILOG_ERROR("dumpInfoNative nullptr.");
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

#ifdef SUPPORT_GRAPHICS
void JsServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("OnCreate.");
}

void JsServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("OnDestroy.");
}

void JsServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        HILOG_ERROR("Configuration is invalid.");
        return;
    }

    HILOG_DEBUG("Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    HILOG_DEBUG("Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsServiceExtension = std::static_pointer_cast<JsServiceExtension>(shared_from_this());
        auto task = [jsServiceExtension]() {
            if (jsServiceExtension) {
                jsServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task);
        }
    }

    HILOG_DEBUG("finished.");
}
#endif
} // AbilityRuntime
} // OHOS
