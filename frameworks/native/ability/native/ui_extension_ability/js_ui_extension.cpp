/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_embeddable_ui_ability_context.h"
#include "js_embeddable_window_stage.h"
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
    TAG_LOGD(AAFwkTag::UI_EXT, "AttachUIExtensionContext");
    if (value == nullptr || extValue == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid context.");
        return nullptr;
    }
    auto screenModePtr = reinterpret_cast<std::weak_ptr<int32_t> *>(extValue)->lock();
    if (screenModePtr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid screenModePtr.");
        return nullptr;
    }
    napi_value contextObj = nullptr;
    if (*screenModePtr == AAFwk::IDLE_SCREEN_MODE) {
        auto uiExtObject = JsUIExtensionContext::CreateJsUIExtensionContext(env, ptr);
        CHECK_POINTER_AND_RETURN(uiExtObject, nullptr);
        auto contextRef = JsRuntime::LoadSystemModuleByEngine(env, "application.UIExtensionContext",
            &uiExtObject, 1);
        CHECK_POINTER_AND_RETURN(contextRef, nullptr);
        contextObj = contextRef->GetNapiValue();
    } else {
        auto emUIObject = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            nullptr, ptr, *screenModePtr);
        CHECK_POINTER_AND_RETURN(emUIObject, nullptr);
        auto contextRef = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext",
            &emUIObject, 1);
        CHECK_POINTER_AND_RETURN(contextRef, nullptr);
        contextObj = contextRef->GetNapiValue();
    }
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "load context error.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc,
        AttachUIExtensionContext, value, extValue);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
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
    TAG_LOGD(AAFwkTag::UI_EXT, "Js ui extension destructor.");
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension begin init");
    CHECK_POINTER(record);
    UIExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtension Init abilityInfo error");
        return;
    }

    if (record != nullptr) {
        token_ = record->GetToken();
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
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get jsObj_");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get JsUIExtension object");
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
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "BindContext CreateJsUIExtensionContext.");
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Want info is null.");
        return;
    }
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    napi_value contextObj = nullptr;
    CreateJSContext(env, contextObj, context, screenMode);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get LoadSystemModuleByEngine.");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context native object");
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
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    TAG_LOGD(AAFwkTag::UI_EXT, "Init end.");
}

void JsUIExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnStart begin.");
    Extension::OnStart(want);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnStart end.");
}

void JsUIExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIExtension::OnStop();
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnStop begin.");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    OnStopCallBack();
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnStop end.");
}
void JsUIExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStop begin.");
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
            TAG_LOGE(AAFwkTag::UI_EXT, "extension is nullptr.");
            return;
        }
        jsUIExtension->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to call promise.");
        OnStopCallBack();
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "OnStop end.");
}

void JsUIExtension::OnStopCallBack()
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
        return;
    }
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(context->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UI_EXT, "The service connection is not disconnected.");
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
        TAG_LOGD(AAFwkTag::UI_EXT, "result is null, no need to call promise.");
        return false;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::UI_EXT, "result is not promise, no need to call promise.");
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
        TAG_LOGD(AAFwkTag::UI_EXT, "Invalid input info.");
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
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UI_EXT, "property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::UI_EXT, "exit");
    return true;
}

sptr<IRemoteObject> JsUIExtension::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "remoteObj is nullptr.");
    }
    return remoteObj;
}

void JsUIExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnDisconnect begin.");
    HandleScope handleScope(jsRuntime_);
    CallOnDisconnect(want, false);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnDisconnect end.");
}

void JsUIExtension::OnCommandWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "sessionInfo is nullptr.");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "begin. persistentId: %{private}d, winCmd: %{public}d",
        sessionInfo->persistentId, winCmd);
    Extension::OnCommandWindow(want, sessionInfo, winCmd);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want) && winCmd == AAFwk::WIN_CMD_FOREGROUND) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, false);
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
            TAG_LOGD(AAFwkTag::UI_EXT, "unsupported cmd.");
            break;
    }
    OnCommandWindowDone(sessionInfo, winCmd);
}

bool JsUIExtension::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called.");
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed.");
        return false;
    }

    std::unique_ptr<InsightIntentExecutorAsyncCallback> executorCallback = nullptr;
    executorCallback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (executorCallback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Create async callback failed.");
        return false;
    }

    auto uiExtension = std::static_pointer_cast<JsUIExtension>(shared_from_this());
    executorCallback->Push([uiExtension, sessionInfo, needForeground](AppExecFwk::InsightIntentExecuteResult result) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Execute post insightintent.");
        if (uiExtension == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "UI extension is nullptr.");
            return;
        }

        uiExtension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    });

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
        return false;
    }
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
    TAG_LOGD(AAFwkTag::UI_EXT, "executorInfo, insightIntentId: %{public}" PRIu64,
        executorInfo.executeParam->insightIntentId_);
    int32_t ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(executorCallback));
    if (!ret) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Execute insight intent failed.");
        // callback has removed, release in insight intent executor.
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
    return true;
}

void JsUIExtension::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "Post insightintent executed.");
    if (needForeground) {
        // If uiextensionability is started for the first time or need move background to foreground.
        HandleScope handleScope(jsRuntime_);
        CallObjectMethod("onForeground");
    }

    OnInsightIntentExecuteDone(sessionInfo, result);

    if (needForeground) {
        // If need foreground, that means triggered by onForeground.
        TAG_LOGI(AAFwkTag::UI_EXT, "call abilityms");
        AAFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, AAFwk::ABILITY_STATE_FOREGROUND_NEW,
            restoreData);
    } else {
        // If uiextensionability has displayed in the foreground.
        OnCommandWindowDone(sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    }
}

void JsUIExtension::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called.");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
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
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}

void JsUIExtension::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo.");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
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
            TAG_LOGD(AAFwkTag::UI_EXT, "TransferExtensionData success");
        } else {
            TAG_LOGE(AAFwkTag::UI_EXT, "TransferExtensionData failed, ret=%{public}d", ret);
        }

        res->second->Show();
        foregroundWindows_.emplace(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}

void JsUIExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnCommand begin restart=%{public}s,startId=%{public}d.",
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
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnCommand end.");
}

void JsUIExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnForeground begin.");
    Extension::OnForeground(want, sessionInfo);

    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, true);
        if (finish) {
            return;
        }
    }

    ForegroundWindow(want, sessionInfo);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnForeground end.");
}

void JsUIExtension::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnBackground begin.");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
    Extension::OnBackground();
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnBackground end.");
}

bool JsUIExtension::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo.");
        return false;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetURI().c_str());
    auto compId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(compId) == uiWindowMap_.end()) {
        auto context = GetContext();
        auto uiWindow = CreateUIWindow(context, sessionInfo);
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "create ui window error.");
            return false;
        }
        HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
        napi_value nativeContentSession = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
            env, sessionInfo, uiWindow, context, abilityResultListeners_);
        napi_ref ref = nullptr;
        napi_create_reference(env, nativeContentSession, 1, &ref);
        contentSessions_.emplace(compId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
        int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
        if (screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
            screenMode_ = AAFwk::EMBEDDED_FULL_SCREEN_MODE;
            auto jsAppWindowStage = CreateAppWindowStage(uiWindow, sessionInfo);
            if (jsAppWindowStage == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "JsAppWindowStage is nullptr.");
                return false;
            }
            napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
            CallObjectMethod("onWindowStageCreate", argv, ARGC_ONE);
        } else {
            napi_value argv[] = {napiWant, nativeContentSession};
            CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
        }
        uiWindowMap_[compId] = uiWindow;
        if (context->GetWindow() == nullptr) {
            context->SetWindow(uiWindow);
        }
    }
    return true;
}

sptr<Rosen::Window> JsUIExtension::CreateUIWindow(const std::shared_ptr<UIExtensionContext> context,
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    sptr<Rosen::WindowOption> option = new (std::nothrow) Rosen::WindowOption();
    if (context == nullptr || context->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
        return nullptr;
    }
    option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    return Rosen::Window::Create(option, GetContext(), sessionInfo->sessionToken);
}

std::unique_ptr<NativeReference> JsUIExtension::CreateAppWindowStage(sptr<Rosen::Window> uiWindow,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::JsEmbeddableWindowStage::CreateJsEmbeddableWindowStage(
        env, uiWindow, sessionInfo);
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.embeddablewindowstage", &jsWindowStage, 1);
}

void JsUIExtension::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto& uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Show();
        foregroundWindows_.emplace(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}

void JsUIExtension::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo.");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Fail to find uiWindow");
        return;
    }
    auto& uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(componentId);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}

void JsUIExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo.");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Wrong to find uiWindow");
        return;
    }
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        if (screenMode_ == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
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
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}

napi_value JsUIExtension::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension CallObjectMethod(%{public}s), begin", name);

    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get UIExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get '%{public}s' from UIExtension object", name);
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension CallFunction(%{public}s), success", name);
    napi_call_function(env, obj, method, argc, argv, nullptr);
    return nullptr;
}

napi_value JsUIExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension CallOnConnect begin.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get UIExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onConnect from UIExtension object");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (remoteNative == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "remoteNative is nullptr.");
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension CallOnConnect end.");
    return remoteNative;
}

napi_value JsUIExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get UIExtension object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onDisconnect from UIExtension object");
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension OnConfigurationUpdated called.");

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context->GetResourceManager());

    auto fullConfig = context->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UI_EXT, "configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
}

void JsUIExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtension Dump called.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found UIExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get UIExtension object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onDump from UIExtension object");
            return;
        }
    }
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "dumpInfo is nullptr.");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Parse dumpInfoStr fail.");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Dump info size: %{public}zu", info.size());
}

void JsUIExtension::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin.");
    Extension::OnAbilityResult(requestCode, resultCode, resultData);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "not attached to any runtime context!");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "abilityResultListensers is nullptr");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}
}
}
