/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "ability_delegator_registry.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_service_extension_context.h"
#include "js_ui_service_host_proxy.h"
#include "string_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "scene_board_judgement.h"
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
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
const std::string LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";
const std::string LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
const int32_t BASE_DISPLAY_ID_NUM (10);
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const std::string METHOD_NAME = "WindowScene::GoForeground";
#endif
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
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "enter");
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
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "end");
    return nullptr;
}
}

int32_t UIServiceExtensionHostStub::SendData(OHOS::AAFwk::WantParams &data)
{
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "UIServiceExtensionHostStub::SendData");
    auto sptr = extension_.lock();
    if (sptr) {
        return sptr->SendData(data);
    }

    return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
}

napi_value AttachUIServiceExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<UIServiceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsUIServiceExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.UIServiceExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "load module failed.");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIServiceExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<UIServiceExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "Finalizer for weak_ptr service extension context is called");
            delete static_cast<std::weak_ptr<UIServiceExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}


#ifdef SUPPORT_GRAPHICS
void JsUIServiceExtension::OnSceneWillCreated(std::shared_ptr<Rosen::WindowStageConfig> windowStageConfig)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto jsAppWindowStageConfig = CreateJsWindowStageConfig(env, windowStageConfig);
    if (jsAppWindowStageConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create jsWindowSatgeConfig object.");
        return;
    }
    napi_value argv[] = {jsAppWindowStageConfig};
    {
        std::string methodName = "OnSceneWillCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        CallObjectMethod("onWindowStageWillCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIServiceExtension::OnSceneDidCreated()
{
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsAppWindowStage is nullptr.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    {
        std::string methodName = "OnSceneDidCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        CallObjectMethod("onWindowStageDidCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "Call onWindowStageDidCreate finish.");
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "Call PostPerformScenceCreated.");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }
    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageDidCreate(jsObj_, jsWindowStageObj_);
    }

    TAG_LOGI(AAFwkTag::UISERVC_EXT, "End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIServiceExtension::OnSceneRestored()
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "called.");
    HandleScope handleScope(jsRuntime_);
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsAppWindowStage is nullptr.");
        return;
    }
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Call PostPerformScenceRestored.");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
}

std::unique_ptr<NativeReference> JsUIServiceExtension::CreateAppWindowStage()
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to create jsWindowSatge object.");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void JsUIServiceExtension::UpdateJsWindowStage(napi_value windowStage)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Called.");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "shellContextRef_ is nullptr.");
        return;
    }
    napi_value contextObj = shellContextRef_->GetNapiValue();
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get context native object.");
        return;
    }
    if (windowStage == nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Set context windowStage is undefined.");
        napi_set_named_property(env, contextObj, "windowStage", CreateJsUndefined(env));
        return;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Set context windowStage object.");
    napi_set_named_property(env, contextObj, "windowStage", windowStage);
}

void JsUIServiceExtension::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool JsUIServiceExtension::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void JsUIServiceExtension::RestorePageStack(const Want &want)
{
    if (IsRestorePageStack(want)) {
        std::string pageStack;
        GetPageStackFromWant(want, pageStack);
        HandleScope handleScope(jsRuntime_);
        auto env = jsRuntime_.GetNapiEnv();
        if (abilityContext_->GetContentStorage()) {
            scene_->GetMainWindow()->NapiSetUIContent(pageStack, env,
                abilityContext_->GetContentStorage()->GetNapiValue(), true);
        } else {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "Content storage is nullptr.");
        }
    }
}

void JsUIServiceExtension::AbilityContinuationOrRecover(const Want &want)
{
    // multi-instance ability continuation
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Launch reason is %{public}d.", launchParam_.launchReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
    } else if (ShouldRecoverState(want)) {
        std::string pageStack = abilityRecovery_->GetSavedPageStack(AppExecFwk::StateReason::DEVELOPER_REQUEST);
        HandleScope handleScope(jsRuntime_);
        auto env = jsRuntime_.GetNapiEnv();
        auto mainWindow = scene_->GetMainWindow();
        if (mainWindow != nullptr) {
            mainWindow->NapiSetUIContent(pageStack, env, abilityContext_->GetContentStorage()->GetNapiValue(), true);
        } else {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "MainWindow is nullptr.");
        }
        OnSceneRestored();
    } else {
        OnSceneDidCreated();
    }
}

void JsUIServiceExtension::DoOnForegroundForSceneIsNull(const Want &want,
    const std::shared_ptr< Rosen::WindowStageConfig> windowStageConfig)
{
    scene_ = std::make_shared<Rosen::WindowScene>();
    int32_t displayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
    if (setting_ != nullptr) {
        std::string strDisplayId = setting_->GetProperty(OHOS::AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        std::regex formatRegex("[0-9]{0,9}$");
        std::smatch sm;
        bool flag = std::regex_match(strDisplayId, sm, formatRegex);
        if (flag && !strDisplayId.empty()) {
            displayId = strtol(strDisplayId.c_str(), nullptr, BASE_DISPLAY_ID_NUM);
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "Success displayId is %{public}d.", displayId);
        } else {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to formatRegex: [%{public}s].", strDisplayId.c_str());
        }
    }
    auto option = GetWindowOption(want, windowStageConfig);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto sessionToken = GetSessionToken();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken);
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to init window scene.");
        return;
    }

    AbilityContinuationOrRecover(want);
}

void JsUIServiceExtension::DoOnForeground(const Want &want)
{
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "AbilityContext or sceneListener_ is nullptr.");
            return;
        }
        std::shared_ptr<Rosen::WindowStageConfig> windowStageConfig = std::make_shared<Rosen::WindowStageConfig>();
        OnSceneWillCreated(windowStageConfig);
        DoOnForegroundForSceneIsNull(want, windowStageConfig);
    } else {
        auto window = scene_->GetMainWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(
                Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            windowMode_ = windowMode;
            TAG_LOGD(AAFwkTag::UISERVC_EXT, "Set window mode is %{public}d.", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    if (CheckIsSilentForeground()) {
        TAG_LOGI(AAFwkTag::UISERVC_EXT, "silent foreground, do not show window");
        return;
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Move scene to foreground, sceneFlag_: %{public}d.", UIServiceExtension::sceneFlag_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIServiceExtension::sceneFlag_);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "End.");
}
#endif

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> JsUIServiceExtension::CreateADelegatorAbilityProperty()
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "abilityContext_ is nullptr.");
        return nullptr;
    }
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    property->token_ = abilityContext_->GetToken();
    property->name_ = GetAbilityName();
    property->moduleName_ = GetModuleName();
    if (GetContext()->GetApplicationInfo() == nullptr || GetContext()->GetApplicationInfo()->bundleName.empty()) {
        property->fullName_ = GetAbilityName();
    } else {
        std::string::size_type pos = GetAbilityName().find(GetContext()->GetApplicationInfo()->bundleName);
        if (pos == std::string::npos || pos != 0) {
            property->fullName_ = GetContext()->GetApplicationInfo()->bundleName + "." + GetAbilityName();
        } else {
            property->fullName_ = GetAbilityName();
        }
    }
    property->lifecycleState_ = GetState();
    property->object_ = jsObj_;
    return property;
}

JsUIServiceExtension* JsUIServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsUIServiceExtension(static_cast<JsRuntime&>(*runtime));
}

JsUIServiceExtension::JsUIServiceExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {
}

JsUIServiceExtension::~JsUIServiceExtension()
{
//    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Js UIservice extension destructor.");
    if (extensionStub_) {
        extensionStub_.reset();
    }
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
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");

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
#ifdef SUPPORT_GRAPHICS	
	// windowStage
//	WindowStageConfig  config;
//	OnWindowStageWillCreate(config);
//	CreateWindowStage(want,config);	
//	if(windowStage_ != nullptr){		
//		OnWindowStageDidCreate(windowStage_);
//	}		
	
#endif	
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

sptr<IRemoteObject> JsUIServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = GetNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "remoteObj null.");
    }
    return remoteObj;
}

sptr<IRemoteObject> JsUIServiceExtension::OnConnect(const AAFwk::Want &want,
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
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "remoteObj null.");
        }
        return remoteObj;
    }

    bool callResult = false;
    do {
        if (!CheckTypeForNapiValue(env, result, napi_object)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "CallPromise, error to convert native value to NativeObject.");
            break;
        }
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        if (then == nullptr) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "CallPromise, error to get property: then.");
            break;
        }
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "CallPromise, property then is not callable");
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
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "error to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }
    return nullptr;
}

void JsUIServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "%{public}s begin.", __func__);
    CallOnDisconnect(want, false);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "%{public}s end.", __func__);
}

void JsUIServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "%{public}s start.", __func__);
    napi_value result = CallOnDisconnect(want, true);
    bool isPromise = CheckPromise(result);
    if (!isPromise) {
        isAsyncCallback = false;
        return;
    }
    bool callResult = CallPromise(result, callbackInfo);
    if (!callResult) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "error to call promise.");
        isAsyncCallback = false;
    } else {
        isAsyncCallback = true;
    }

    TAG_LOGD(AAFwkTag::UISERVC_EXT, "%{public}s end.", __func__);
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

bool JsUIServiceExtension::HandleInsightIntent(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "called.");
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    callback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Create async callback failed.");
        return false;
    }
    auto executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    bool ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Generate execute param failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT,
        "Insight intent bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64 "",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto asyncCallback = [weak = weak_from_this(), intentId = executeParam->insightIntentId_]
        (AppExecFwk::InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "Execute insight intent finshed, intentId %{public}" PRIu64"", intentId);
        auto extension = weak.lock();
        if (extension == nullptr) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "extension is nullptr.");
            return;
        }
        auto ret = extension->OnInsightIntentExecuteDone(intentId, result);
        if (!ret) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "OnInsightIntentExecuteDone failed.");
        }
    };
    callback->Push(asyncCallback);
    InsightIntentExecutorInfo executorInfo;
    ret = GetInsightIntentExecutorInfo(want, executeParam, executorInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(callback));
    if (!ret) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Execute insight intent failed.");
        return false;
    }
    return true;
}

bool JsUIServiceExtension::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo &executorInfo)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "called.");
    auto context = GetContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Param invalid.");
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

bool JsUIServiceExtension::OnInsightIntentExecuteDone(uint64_t intentId,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "Notify execute done, intentId %{public}" PRIu64"", intentId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "context is null.");
        return false;
    }
    auto token = context->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "token is null.");
        return false;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Notify execute done faild.");
        return false;
    }
    return true;
}

void JsUIServiceExtension::AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName)
{
    FreezeUtil::LifecycleFlow flow = { abilityContext_->GetToken(), state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; JsUIServiceExtension::" + methodName +
        "; the " + methodName + " begin.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}

void JsUIServiceExtension::AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName)
{
    FreezeUtil::LifecycleFlow flow = { abilityContext_->GetToken(), state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; JsUIServiceExtension::" + methodName +
        "; the " + methodName + " end.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
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

napi_value JsUIServiceExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");
    napi_env env = jsRuntime_.GetNapiEnv();

    callbackProxy_ = want.GetRemoteObject("UIServiceStub");
    napi_value napiHostProxy = AAFwk::JsUIServiceHostProxy::CreateJsUIServiceHostProxy(env, callbackProxy_);

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant, napiHostProxy};
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "Not found ServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get onConnect from ServiceExtension object");
        return nullptr;
    }
    
    napi_value result = nullptr;
    napi_call_function(env, obj, method, ARGC_TWO, argv, &result);

    napi_value napiStubObject = CreateJsUndefined(env);
    if (extensionStub_ == nullptr) {
        auto shareThis = std::static_pointer_cast<JsUIServiceExtension>(shared_from_this());
        std::weak_ptr<JsUIServiceExtension> weakThis = shareThis;
        extensionStub_ = std::make_unique<UIServiceExtensionHostStub>(weakThis);
    }

    napiStubObject = NAPI_ohos_rpc_CreateJsRemoteObject(env, extensionStub_->AsObject());
    
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ok");
    return napiStubObject;
}

napi_value JsUIServiceExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    if (extensionStub_) {
        extensionStub_.reset();
    }
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "Not found ServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get ServiceExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get onDisconnect from ServiceExtension object");
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

bool JsUIServiceExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "CheckPromise, result is nullptr, no need to call promise.");
        return false;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "CheckPromise, result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

bool JsUIServiceExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "CallPromise, Error to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "CallPromise, Error to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "CallPromise, Property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "end");
    return true;
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
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, ARGC_ONE);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsUIServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "call");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "Not found ServiceExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get ServiceExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to get onConnect from ServiceExtension object");
            return;
        }
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "success");
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "dumpInfo nullptr.");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "Parse dumpInfoStr failed.");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Dump info size: %{public}zu", info.size());
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

int32_t JsUIServiceExtension::SendData(OHOS::AAFwk::WantParams &data)
{
    TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceExtension::SendData");
    napi_env env = jsRuntime_.GetNapiEnv();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([weak = weak_from_this(), wantParams = data](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto extensionSptr = weak.lock();
            if (!extensionSptr) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "extensionSptr nullptr");
                return;
            }
            auto sptrThis = std::static_pointer_cast<JsUIServiceExtension>(extensionSptr);
            if (!sptrThis) {
                TAG_LOGE(AAFwkTag::UISERVC_EXT, "sptrThis nullptr");
                return;
            }
            sptrThis->HandleSendData(wantParams);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsUIServiceExtension::SendData",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void JsUIServiceExtension::HandleSendData(const OHOS::AAFwk::WantParams &data)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceExtension::HandleSendData, Failed to get ServiceExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onData", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "JsUIServiceExtension::HandleSendData, Failed to get onData from ServiceExtension object");
        return;
    }

    napi_value argv[] = {AppExecFwk::CreateJsWantParams(env, data)};
    napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
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
} // AbilityRuntime
} // OHOS
