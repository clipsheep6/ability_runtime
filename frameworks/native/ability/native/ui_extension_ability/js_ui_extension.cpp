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

#include "js_ui_extension.h"
#include "ability_context.h"
#include "ability_delegator_registry.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ability_start_setting.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_embeddable_ui_ability_context.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"
#include "js_ui_extension_context.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

napi_value AttachUIExtensionContext(napi_env env, void *value, void *extValue)
{
    HILOG_DEBUG("AttachUIExtensionContext");
    if (value == nullptr || extValue == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }
    auto screenModePtr = reinterpret_cast<std::weak_ptr<int32_t> *>(extValue)->lock();
    if (screenModePtr == nullptr) {
        HILOG_ERROR("Invalid screenModePtr.");
        return nullptr;
    }
    napi_value contextObj = nullptr;
    if (*screenModePtr == AAFwk::IDLE_SCREEN_MODE) {
        auto uiExtObject = JsUIExtensionContext::CreateJsUIExtensionContext(env, ptr);
        CHECK_POINTER_AND_RETURN(uiExtObject, nullptr);
        contextObj = JsRuntime::LoadSystemModuleByEngine(env, "application.UIExtensionContext",
            &uiExtObject, 1)->GetNapiValue();
    } else {
        auto emUIObject = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            nullptr, ptr, *screenModePtr);
        CHECK_POINTER_AND_RETURN(emUIObject, nullptr);
        contextObj = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext",
            &emUIObject, 1)->GetNapiValue();
    }
    if (contextObj == nullptr) {
        HILOG_ERROR("load context error.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc,
        AttachUIExtensionContext, value, extValue);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

void AbilityResultListeners::AddListener(const uint64_t &uiExtensionComponentId,
    std::shared_ptr<AbilityResultListener> listener)
{
    if (uiExtensionComponentId == 0) {
        HILOG_ERROR("Invalid session.");
        return;
    }
    listeners_[uiExtensionComponentId] = listener;
}

void AbilityResultListeners::RemoveListener(const uint64_t &uiExtensionComponentId)
{
    if (listeners_.find(uiExtensionComponentId) != listeners_.end()) {
        listeners_.erase(uiExtensionComponentId);
    }
}

void AbilityResultListeners::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    for (auto item:listeners_) {
        if (item.second && item.second->IsMatch(requestCode)) {
            item.second->OnAbilityResult(requestCode, resultCode, resultData);
            return;
        }
    }
}

JsUIExtension* JsUIExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new (std::nothrow) JsUIExtension(static_cast<JsRuntime&>(*runtime));
}

JsUIExtension::JsUIExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime)
{
    abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
}

JsUIExtension::~JsUIExtension()
{
    HILOG_DEBUG("Js ui extension destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
    for (auto &item : contentSessions_) {
        jsRuntime_.FreeNativeReference(std::move(item.second));
    }
    contentSessions_.clear();
}

void JsUIExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("JsUIExtension begin init");
    CHECK_POINTER(record);
    UIExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("JsUIExtension Init abilityInfo error");
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get JsUIExtension object");
        return;
    }

    BindContext(env, obj, record->GetWant());

    SetExtensionCommon(
        JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));
}

void JsUIExtension::CreateJSContext(napi_env env, napi_value &contextObj,
    std::shared_ptr<UIExtensionContext> context, int32_t screenMode)
{
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(env, context);
        CHECK_POINTER(contextObj);
        shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.UIExtensionContext",
            &contextObj, ARGC_ONE);
    } else {
        contextObj = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            nullptr, context, screenMode);
        CHECK_POINTER(contextObj);
        shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext",
            &contextObj, ARGC_ONE);
    }
}

void JsUIExtension::BindContext(napi_env env, napi_value obj, std::shared_ptr<AAFwk::Want> want)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_DEBUG("BindContext CreateJsUIExtensionContext.");
    if (want == nullptr) {
        HILOG_ERROR("Want info is null.");
        return;
    }
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    napi_value contextObj = nullptr;
    CreateJSContext(env, contextObj, context, screenMode);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine.");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context);
    CHECK_POINTER(workContext);
    screenModePtr_ = std::make_shared<int32_t>(screenMode);
    auto workScreenMode = new (std::nothrow) std::weak_ptr<int32_t>(screenModePtr_);
    CHECK_POINTER(workScreenMode);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionContext, workContext, workScreenMode);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    HILOG_DEBUG("Init end.");
}

void JsUIExtension::OnStart(const AAFwk::Want &want)
{
    HILOG_DEBUG("JsUIExtension OnStart begin.");
    Extension::OnStart(want);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    HILOG_DEBUG("JsUIExtension OnStart end.");
}

void JsUIExtension::OnStop()
{
    UIExtension::OnStop();
    HILOG_DEBUG("JsUIExtension OnStop begin.");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    OnStopCallBack();
    HILOG_DEBUG("JsUIExtension OnStop end.");
}
void JsUIExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HILOG_DEBUG("OnStop begin.");
    UIExtension::OnStop();
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallBack();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<Extension> weakPtr = shared_from_this();
    auto asyncCallback = [extensionWeakPtr = weakPtr]() {
        auto jsUIExtension = extensionWeakPtr.lock();
        if (jsUIExtension == nullptr) {
            HILOG_ERROR("extension is nullptr.");
            return;
        }
        jsUIExtension->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        HILOG_ERROR("Failed to call promise.");
        OnStopCallBack();
    }
    HILOG_DEBUG("OnStop end.");
}

void JsUIExtension::OnStopCallBack()
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(context->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_DEBUG("The service connection is not disconnected.");
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        std::shared_ptr<NativeReference> sharedJsObj = std::move(jsObj_);
        applicationContext->DispatchOnAbilityDestroy(sharedJsObj);
    }
}

bool JsUIExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        HILOG_DEBUG("result is null, no need to call promise.");
        return false;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        HILOG_DEBUG("result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

napi_value PromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data), nullptr);
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    if (callbackInfo == nullptr) {
        HILOG_DEBUG("Invalid input info.");
        return nullptr;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    data = nullptr;
    return nullptr;
}

bool JsUIExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        HILOG_ERROR("Failed to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        HILOG_ERROR("Failed to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    HILOG_DEBUG("exit");
    return true;
}

sptr<IRemoteObject> JsUIExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
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
    HandleScope handleScope(jsRuntime_);
    CallOnDisconnect(want, false);
    HILOG_DEBUG("JsUIExtension OnDisconnect end.");
}

void JsUIExtension::OnCommandWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr.");
        return;
    }
    HILOG_DEBUG("begin. persistentId: %{private}d, winCmd: %{public}d", sessionInfo->persistentId, winCmd);
    Extension::OnCommandWindow(want, sessionInfo, winCmd);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want) && winCmd == AAFwk::WIN_CMD_FOREGROUND) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo);
        if (finish) {
            return;
        }
    }
    switch (winCmd) {
        case AAFwk::WIN_CMD_FOREGROUND:
            ForegroundWindow(want, sessionInfo);
            break;
        case AAFwk::WIN_CMD_BACKGROUND:
            BackgroundWindow(sessionInfo);
            break;
        case AAFwk::WIN_CMD_DESTROY:
            DestroyWindow(sessionInfo);
            break;
        default:
            HILOG_DEBUG("unsupported cmd.");
            break;
    }
    OnCommandWindowDone(sessionInfo, winCmd);
}

bool JsUIExtension::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called.");
    if (!HandleSessionCreate(want, sessionInfo)) {
        HILOG_ERROR("HandleSessionCreate failed.");
        return false;
    }

    std::unique_ptr<InsightIntentExecutorAsyncCallback> executorCallback = nullptr;
    executorCallback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (executorCallback == nullptr) {
        HILOG_ERROR("Create async callback failed.");
        return false;
    }
    executorCallback->Push([weak = weak_from_this(), sessionInfo](AppExecFwk::InsightIntentExecuteResult result) {
        auto extension = weak.lock();
        if (extension == nullptr) {
            HILOG_ERROR("UI extension is nullptr.");
            return;
        }
        extension->OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
        extension->OnInsightIntentExecuteDone(sessionInfo, result);
    });

    auto context = GetContext();
    InsightIntentExecutorInfo executorInfo;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        executorInfo.hapPath = abilityInfo->hapPath;
        executorInfo.windowMode = abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    }
    executorInfo.token = context->GetToken();
    executorInfo.pageLoader = contentSessions_[sessionInfo->uiExtensionComponentId];
    executorInfo.executeParam = std::make_shared<InsightIntentExecuteParam>();
    InsightIntentExecuteParam::GenerateFromWant(want, *executorInfo.executeParam);
    executorInfo.executeParam->executeMode_ = UI_EXTENSION_ABILITY;
    executorInfo.srcEntry = want.GetStringParam(INSIGHT_INTENT_SRC_ENTRY);
    HILOG_DEBUG("executorInfo, insightIntentId: %{public}" PRIu64, executorInfo.executeParam->insightIntentId_);
    int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(executorCallback));
    if (!ret) {
        HILOG_ERROR("Execute insight intent failed.");
        // callback has removed, release in insight intent executor.
    }
    HILOG_DEBUG("end.");
    return true;
}

void JsUIExtension::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("called.");
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    AAFwk::AbilityCommand abilityCmd;
    if (uiWindowMap_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_DESTROY;
    } else if (foregroundWindows_.empty()) {
        abilityCmd = AAFwk::ABILITY_CMD_BACKGROUND;
    } else {
        abilityCmd = AAFwk::ABILITY_CMD_FOREGROUND;
    }
    AAFwk::AbilityManagerClient::GetInstance()->ScheduleCommandAbilityWindowDone(
        context->GetToken(), sessionInfo, winCmd, abilityCmd);
    HILOG_DEBUG("end.");
}

void JsUIExtension::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    if (sessionInfo == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }

    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto res = uiWindowMap_.find(componentId);
    if (res != uiWindowMap_.end() && res->second != nullptr) {
        WantParams params;
        params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT_CODE, Integer::Box(result.innerErr));
        WantParams resultParams;
        resultParams.SetParam("code", Integer::Box(result.code));
        if (result.result != nullptr) {
            sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(*result.result);
            if (pWantParams != nullptr) {
                resultParams.SetParam("result", pWantParams);
            }
        }
        sptr<AAFwk::IWantParams> pWantParams = WantParamWrapper::Box(resultParams);
        if (pWantParams != nullptr) {
            params.SetParam(INSIGHT_INTENT_EXECUTE_RESULT, pWantParams);
        }

        Rosen::WMError ret = res->second->TransferExtensionData(params);
        if (ret == Rosen::WMError::WM_OK) {
            HILOG_DEBUG("TransferExtensionData success");
        } else {
            HILOG_ERROR("TransferExtensionData failed, ret=%{public}d", ret);
        }

        res->second->Show();
        foregroundWindows_.emplace(componentId);
    }
    HILOG_DEBUG("end.");
}

void JsUIExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    HILOG_DEBUG("JsUIExtension OnCommand begin restart=%{public}s,startId=%{public}d.",
        restart ? "true" : "false", startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = {napiWant, napiStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    HILOG_DEBUG("JsUIExtension OnCommand end.");
}

void JsUIExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("JsUIExtension OnForeground begin.");
    Extension::OnForeground(want, sessionInfo);

    ForegroundWindow(want, sessionInfo);

    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
    HILOG_DEBUG("JsUIExtension OnForeground end.");
}

void JsUIExtension::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("JsUIExtension OnBackground begin.");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
    Extension::OnBackground();
    HILOG_DEBUG("JsUIExtension OnBackground end.");
}

bool JsUIExtension::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        HILOG_ERROR("Invalid sessionInfo.");
        return false;
    }

    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetURI().c_str());
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
        auto context = GetContext();
        if (context == nullptr || context->GetAbilityInfo() == nullptr) {
            HILOG_ERROR("Failed to get context");
            return false;
        }
        option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
        option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
        option->SetParentId(sessionInfo->hostWindowId);
        auto uiWindow = Rosen::Window::Create(option, GetContext(), sessionInfo->sessionToken);
        if (uiWindow == nullptr) {
            HILOG_ERROR("create ui window error.");
            return false;
        }
        HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
        napi_value nativeContentSession =
            JsUIExtensionContentSession::CreateJsUIExtensionContentSession(env, sessionInfo,
                uiWindow, GetContext(), abilityResultListeners_);
        napi_ref ref = nullptr;
        napi_create_reference(env, nativeContentSession, 1, &ref);
        contentSessions_.emplace(
            componentId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
        int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
        if (screenMode == AAFwk::HALF_SCREEN_MODE) {
            screenMode_ = AAFwk::HALF_SCREEN_MODE;
            napi_value argv[] = {nullptr};
            CallObjectMethod("onWindowStageCreate", argv, ARGC_ONE);
        } else {
            napi_value argv[] = {napiWant, nativeContentSession};
            CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
        }
        uiWindowMap_[componentId] = uiWindow;
        if (context->GetWindow() == nullptr) {
            context->SetWindow(uiWindow);
        }
    }
    return true;
}

void JsUIExtension::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (!HandleSessionCreate(want, sessionInfo)) {
        HILOG_ERROR("HandleSessionCreate failed.");
        return;
    }
    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto& uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Show();
        foregroundWindows_.emplace(componentId);
    }
    HILOG_DEBUG("end.");
}

void JsUIExtension::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }

    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        HILOG_ERROR("Fail to find uiWindow");
        return;
    }
    auto& uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(componentId);
    }
    HILOG_DEBUG("end.");
}

void JsUIExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }

    HILOG_DEBUG("UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        HILOG_ERROR("Wrong to find uiWindow");
        return;
    }
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        if (screenMode_ == AAFwk::HALF_SCREEN_MODE) {
            screenMode_ = AAFwk::IDLE_SCREEN_MODE;
            CallObjectMethod("onWindowStageDestroy");
        } else {
            napi_value argv[] = {contentSessions_[componentId]->GetNapiValue()};
            CallObjectMethod("onSessionDestroy", argv, ARGC_ONE);
        }
    }
    auto& uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(componentId);
    auto context = GetContext();
    if (context != nullptr && context->GetWindow() == uiWindow) {
        context->SetWindow(nullptr);
        for (auto it : uiWindowMap_) {
            context->SetWindow(it.second);
            break;
        }
    }
    foregroundWindows_.erase(componentId);
    contentSessions_.erase(componentId);
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
    HILOG_DEBUG("end.");
}

napi_value JsUIExtension::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    HILOG_DEBUG("JsUIExtension CallObjectMethod(%{public}s), begin", name);

    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get UIExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        HILOG_ERROR("Failed to get '%{public}s' from UIExtension object", name);
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    HILOG_DEBUG("JsUIExtension CallFunction(%{public}s), success", name);
    napi_call_function(env, obj, method, argc, argv, nullptr);
    return nullptr;
}

napi_value JsUIExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    HILOG_DEBUG("JsUIExtension CallOnConnect begin.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get UIExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onConnect from UIExtension object");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (remoteNative == nullptr) {
        HILOG_ERROR("remoteNative is nullptr.");
    }
    HILOG_DEBUG("JsUIExtension CallOnConnect end.");
    return remoteNative;
}

napi_value JsUIExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get UIExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onDisconnect from UIExtension object");
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

void JsUIExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    Extension::OnConfigurationUpdated(configuration);
    HILOG_DEBUG("JsUIExtension OnConfigurationUpdated called.");

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());

    auto fullConfig = context->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
}

void JsUIExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    HILOG_DEBUG("JsUIExtension Dump called.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        HILOG_ERROR("Not found UIExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get UIExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            HILOG_ERROR("Failed to get onDump from UIExtension object");
            return;
        }
    }
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        HILOG_ERROR("dumpInfo is nullptr.");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            HILOG_ERROR("Parse dumpInfoStr fail.");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}

void JsUIExtension::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("begin.");
    Extension::OnAbilityResult(requestCode, resultCode, resultData);
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_WARN("not attached to any runtime context!");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        HILOG_WARN("abilityResultListensers is nullptr");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_DEBUG("end.");
}
}
}
