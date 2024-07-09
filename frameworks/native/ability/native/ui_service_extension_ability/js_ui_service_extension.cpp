/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_ui_service_extension.h"

#include <regex>

#include "ability_business_error.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_service_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "scene_board_judgement.h"
#include "ability_context.h"
#include "session_info.h"
#include "window_scene.h"
#include "js_window_stage.h"
#include "wm_common.h"
#include "window.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

using namespace OHOS::AppExecFwk;

napi_value AttachUIServiceExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::UIServiceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "invalid context.");
        return nullptr;
    }
    napi_value object = AbilityRuntime::CreateJsUIServiceExtensionContext(env, ptr);
    auto sysModule = AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env,
        "application.UIServiceExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "load module failed.");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, AbilityRuntime::DetachCallbackFunc, AttachUIServiceExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::UIServiceExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::UIServiceExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

JsUIServiceExtension* JsUIServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsUIServiceExtension(static_cast<AbilityRuntime::JsRuntime&>(*runtime));
}

JsUIServiceExtension::JsUIServiceExtension(AbilityRuntime::JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}

JsUIServiceExtension::~JsUIServiceExtension()
{
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsUIServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    UIServiceExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "JsServiceExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get jsObj_");
        return;
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ConvertNativeValueTo.");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get JsServiceExtension object");
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
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
    ListenWMS();
}

void JsUIServiceExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "systemAbilityId: %{public}d add", systemAbilityId);
    if (systemAbilityId == WINDOW_MANAGER_SERVICE_ID) {
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(tmpDisplayListener_);
    }
}

void JsUIServiceExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get context");
        return;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");
    napi_value contextObj = CreateJsUIServiceExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.UIServiceExtensionContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to load module");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIServiceExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIServiceExtensionContext, workContext, nullptr);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Bind.");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            delete static_cast<std::weak_ptr<UIServiceExtensionContext>*>(data);
        },
        nullptr, nullptr);

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "end.");
}

void JsUIServiceExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "call");

    auto context = GetContext();
    if (context != nullptr) {
        int32_t  displayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "displayId %{public}d", displayId);
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
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ok");
}


void JsUIServiceExtension::OnStop()
{
    Extension::OnStop();
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "The service extension connection is not disconnected.");
    }
    Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(displayListener_);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ok");
}

sptr<IRemoteObject> JsUIServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    return nullptr;
}

void JsUIServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
}

void JsUIServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "restart=%{public}s,startId=%{public}d.",
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
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ok");
}

napi_value JsUIServiceExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "name:%{public}s", name);

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "Not found ServiceExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get '%{public}s' from ServiceExtension object", name);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsUIServiceExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "GetSrcPath start.");
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

void JsUIServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    UIServiceExtension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void JsUIServiceExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "called.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "configuration is nullptr.");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsUIServiceExtension::ListenWMS()
{
#ifdef SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "RegisterDisplayListener");
    auto abilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get SaMgr.");
        return;
    }

    auto jsUIServiceExtension = std::static_pointer_cast<JsUIServiceExtension>(shared_from_this());
    displayListener_ = sptr<JsUIServiceExtensionDisplayListener>::MakeSptr(jsUIServiceExtension);
    if (displayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create display listener.");
        return;
    }

    auto listener = sptr<SystemAbilityStatusChangeListener>::MakeSptr(displayListener_);
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create status change listener.");
        return;
    }

    auto ret = abilityManager->SubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, listener);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "subscribe system ability failed, ret = %{public}d.", ret);
    }
#endif
}

#ifdef SUPPORT_GRAPHICS
void JsUIServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "enter.");
}

void JsUIServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "exit.");
}

void JsUIServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Context is invalid.");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Configuration is invalid.");
        return;
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsUIServiceExtension = std::static_pointer_cast<JsUIServiceExtension>(shared_from_this());
        auto task = [jsUIServiceExtension]() {
            if (jsUIServiceExtension) {
                jsUIServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsServiceExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "finished.");
}

#endif
}
} // OHOS
