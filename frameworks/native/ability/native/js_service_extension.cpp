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

#include "ability_business_error.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
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
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

namespace {
sptr<IRemoteObject> GetNativeRemoteObject(napi_env env, napi_value obj)
{
    if (env == nullptr || obj == nullptr) {
        HILOG_ERROR("obj is null.");
        return nullptr;
    }
    napi_valuetype type;
    napi_typeof(env, obj, &type);
    if (type == napi_undefined || type == napi_null) {
        HILOG_ERROR("obj is invalid type.");
        return nullptr;
    }
    if (type != napi_object) {
        HILOG_ERROR("obj is not an object.");
        return nullptr;
    }
    return NAPI_ohos_rpc_getNativeRemoteObject(env, obj);
}

napi_value PromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}

napi_value OnConnectPromiseCallback(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("enter");
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *>(data);
    sptr<IRemoteObject> service = nullptr;
    if (argc > 0) {
        service = GetNativeRemoteObject(env, argv[0]);
    }
    callbackInfo->Call(service);
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
    data = nullptr;
    HILOG_DEBUG("end");
    return nullptr;
}
}

using namespace OHOS::AppExecFwk;

napi_value AttachServiceExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<ServiceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsServiceExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.ServiceExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        HILOG_WARN("load module failed.");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachServiceExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<ServiceExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<ServiceExtensionContext> *>(data);
        },
        nullptr, nullptr);
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
    HILOG_DEBUG("JsServiceExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }

    HILOG_DEBUG("ConvertNativeValueTo.");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get JsServiceExtension object");
        return;
    }

    BindContext(env, obj);

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
    ListenWMS();
}

void JsServiceExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    HILOG_DEBUG("RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        HILOG_ERROR("Failed to get SaMgr.");
        return;
    }

    auto jsServiceExtension = std::static_pointer_cast<JsServiceExtension>(shared_from_this());
    displayListener_ = sptr<JsServiceExtensionDisplayListener>::MakeSptr(jsServiceExtension);
    if (displayListener_ == nullptr) {
        HILOG_ERROR("Failed to create display listener.");
        return;
    }

    auto listener = sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_);
    if (listener == nullptr) {
        HILOG_ERROR("Failed to create status change listener.");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, listener);
    if (ret != 0) {
        HILOG_ERROR("subscribe system ability failed, ret = %{public}d.", ret);
    }
#endif
}

void JsServiceExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    HILOG_DEBUG("systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(tmpDisplayListener_);
    }
}

void JsServiceExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_DEBUG("call");
    napi_value contextObj = CreateJsServiceExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.ServiceExtensionContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to load module");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ServiceExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachServiceExtensionContext, workContext, nullptr);
    HILOG_DEBUG("Bind.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            delete static_cast<std::weak_ptr<ServiceExtensionContext>*>(data);
        },
        nullptr, nullptr);

    HILOG_DEBUG("end.");
}

void JsServiceExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    HILOG_DEBUG("call");

    auto context = GetContext();
    if (context != nullptr) {
        int32_t  displayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        HILOG_DEBUG("displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // display config has changed, need update context.config
    if (context != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    HILOG_DEBUG("ok");
}

void JsServiceExtension::OnStop()
{
    ServiceExtension::OnStop();
    HILOG_DEBUG("call");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_DEBUG("The service extension connection is not disconnected.");
    }
    Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(displayListener_);
    HILOG_DEBUG("ok");
}

sptr<IRemoteObject> JsServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = GetNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        HILOG_ERROR("remoteObj null.");
    }
    return remoteObj;
}

sptr<IRemoteObject> JsServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value result = CallOnConnect(want);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        sptr<IRemoteObject> remoteObj = GetNativeRemoteObject(env, result);
        if (remoteObj == nullptr) {
            HILOG_ERROR("remoteObj null.");
        }
        return remoteObj;
    }

    bool callResult = false;
    do {
        if (!CheckTypeForNapiValue(env, result, napi_object)) {
            HILOG_ERROR("CallPromise, error to convert native value to NativeObject.");
            break;
        }
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        if (then == nullptr) {
            HILOG_ERROR("CallPromise, error to get property: then.");
            break;
        }
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            HILOG_ERROR("CallPromise, property then is not callable");
            break;
        }
        napi_value promiseCallback = nullptr;
        napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
            OnConnectPromiseCallback, callbackInfo, &promiseCallback);
        napi_value argv[1] = { promiseCallback };
        napi_call_function(env, result, then, 1, argv, nullptr);
        callResult = true;
    } while (false);

    if (!callResult) {
        HILOG_ERROR("error to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }
    return nullptr;
}

void JsServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("%{public}s begin.", __func__);
    CallOnDisconnect(want, false);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void JsServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    HILOG_DEBUG("%{public}s start.", __func__);
    napi_value result = CallOnDisconnect(want, true);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        return;
    }
    bool callResult = CallPromise(result, callbackInfo);
    if (!callResult) {
        HILOG_ERROR("error to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }

    HILOG_DEBUG("%{public}s end.", __func__);
}

void JsServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    HILOG_DEBUG("restart=%{public}s,startId=%{public}d.",
        restart ? "true" : "false",
        startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = {napiWant, napiStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    HILOG_DEBUG("ok");
}

bool JsServiceExtension::HandleInsightIntent(const AAFwk::Want &want)
{
    HILOG_DEBUG("called.");
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    callback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (callback == nullptr) {
        HILOG_ERROR("Create async callback failed.");
        return false;
    }
    auto executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    bool ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        HILOG_ERROR("Generate execute param failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    HILOG_DEBUG("Insight intent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64"",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto asyncCallback = [weak = weak_from_this(), intentId = executeParam->insightIntentId_]
        (AppExecFwk::InsightIntentExecuteResult result) {
        HILOG_DEBUG("Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
        auto extension = weak.lock();
        if (extension == nullptr) {
            HILOG_ERROR("extension is nullptr.");
            return;
        }
        auto ret = extension->OnInsightIntentExecuteDone(intentId, result);
        if (!ret) {
            HILOG_ERROR("OnInsightIntentExecuteDone failed.");
        }
    };
    callback->Push(asyncCallback);
    InsightIntentExecutorInfo executorInfo;
    ret = GetInsightIntentExecutorInfo(want, executeParam, executorInfo);
    if (!ret) {
        HILOG_ERROR("Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(callback));
    if (!ret) {
        HILOG_ERROR("Execute insight intent failed.");
        return false;
    }
    return true;
}

bool JsServiceExtension::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo &executorInfo)
{
    HILOG_DEBUG("called.");
    auto context = GetContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        HILOG_ERROR("Param invalid.");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executorInfo.srcEntry = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRY);
    executorInfo.hapPath = abilityInfo_->hapPath;
    executorInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executorInfo.token = context->GetToken();
    executorInfo.executeParam = executeParam;
    return true;
}

bool JsServiceExtension::OnInsightIntentExecuteDone(uint64_t intentId,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    HILOG_INFO("Notify execute done, intentId %{public}" PRIu64"", intentId);
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("context is null.");
        return false;
    }
    auto token = context->GetToken();
    if (token == nullptr) {
        HILOG_ERROR("token is null.");
        return false;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token, intentId, result);
    if (ret != ERR_OK) {
        HILOG_ERROR("Notify execute done faild.");
        return false;
    }
    return true;
}

napi_value JsServiceExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    HILOG_DEBUG("name:%{public}s", name);

    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        HILOG_ERROR("Failed to get '%{public}s' from ServiceExtension object", name);
        return nullptr;
    }
    HILOG_DEBUG("CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsServiceExtension::GetSrcPath(std::string &srcPath)
{
    HILOG_DEBUG("GetSrcPath start.");
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

napi_value JsServiceExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    HILOG_DEBUG("call");
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onConnect from ServiceExtension object");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (remoteNative == nullptr) {
        HILOG_ERROR("remoteNative nullptr.");
    }
    HILOG_DEBUG("ok");
    return remoteNative;
}

napi_value JsServiceExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from ServiceExtension object");
        return nullptr;
    }

    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, ARGC_ONE, argv, &result);
        return handleEscape.Escape(result);
    } else {
        napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
        return nullptr;
    }
}

bool JsServiceExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        HILOG_DEBUG("CheckPromise, result is nullptr, no need to call promise.");
        return false;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        HILOG_DEBUG("CheckPromise, result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

bool JsServiceExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        HILOG_ERROR("CallPromise, Error to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        HILOG_ERROR("CallPromise, Error to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("CallPromise, Property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    HILOG_DEBUG("end");
    return true;
}

void JsServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    ServiceExtension::OnConfigurationUpdated(configuration);
    HILOG_DEBUG("call");
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
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
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, ARGC_ONE);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    HILOG_DEBUG("call");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        HILOG_WARN("Not found ServiceExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get ServiceExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            HILOG_ERROR("Failed to get onConnect from ServiceExtension object");
            return;
        }
    }
    HILOG_DEBUG("success");
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        HILOG_ERROR("dumpInfo nullptr.");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            HILOG_ERROR("Parse dumpInfoStr failed.");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}

#ifdef SUPPORT_GRAPHICS
void JsServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("enter.");
}

void JsServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("exit.");
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
            handler_->PostTask(task, "JsServiceExtension:OnChange");
        }
    }

    HILOG_DEBUG("finished.");
}
#endif
} // AbilityRuntime
} // OHOS
