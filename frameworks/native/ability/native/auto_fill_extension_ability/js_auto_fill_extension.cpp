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

#include "js_auto_fill_extension.h"

#include "ability_context.h"
#include "ability_delegator_registry.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ability_start_setting.h"
#include "connection_manager.h"
#include "context.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "int_wrapper.h"
#include "js_auto_fill_extension_util.h"
#include "js_auto_fill_extension_context.h"
#include "js_fill_request_callback.h"
#include "js_save_request_callback.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr const char *WANT_PARAMS_AUTO_FILL_CMD = "ohos.ability.params.autoFillCmd";
constexpr static char WANT_PARAMS_AUTO_FILL_EVENT_KEY[] = "ability.want.params.AutoFillEvent";
constexpr const char *WANT_PARAMS_CUSTOM_DATA = "ohos.ability.params.customData";
}
napi_value AttachAutoFillExtensionContext(napi_env env, void *value, void *)
{
    HILOG_DEBUG("Called.");
    if (value == nullptr) {
        HILOG_ERROR("Invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<AutoFillExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("Invalid context.");
        return nullptr;
    }
    napi_value object = JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(env, ptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AutoFillExtensionContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_ERROR("Load system module failed.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (contextObj == nullptr) {
        HILOG_ERROR("Load context error.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAutoFillExtensionContext, value, nullptr);

    auto workContext = new (std::nothrow) std::weak_ptr<AutoFillExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<AutoFillExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

JsAutoFillExtension *JsAutoFillExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) JsAutoFillExtension(static_cast<JsRuntime&>(*runtime));
}

JsAutoFillExtension::JsAutoFillExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime)
{
}

JsAutoFillExtension::~JsAutoFillExtension()
{
    HILOG_DEBUG("Destructor.");
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
    callbacks_.clear();
}

void JsAutoFillExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("Called.");
    AutoFillExtension::Init(record, application, handler, token);
    if (abilityInfo_ == nullptr || abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("Init ability info failed.");
        return;
    }
    std::string srcPath(abilityInfo_->moduleName + "/");
    srcPath.append(abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    HILOG_DEBUG("LoadModule moduleName:%{public}s, srcPath:%{public}s, hapPath:%{public}s",
        moduleName.c_str(), srcPath.c_str(),  abilityInfo_->hapPath.c_str());
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Js object is nullptr.");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get js auto fill extension object.");
        return;
    }

    BindContext(env, obj);

    SetExtensionCommon(
        JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));
}

void JsAutoFillExtension::BindContext(napi_env env, napi_value obj)
{
    HILOG_DEBUG("Called.");
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context.");
        return;
    }
    HILOG_DEBUG("Create js auto fill extension context.");
    context->SetAutoFillExtensionCallback(std::static_pointer_cast<JsAutoFillExtension>(shared_from_this()));
    napi_value contextObj = JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(env, context);
    if (contextObj == nullptr) {
        HILOG_ERROR("Create js ui extension context failed.");
        return;
    }

    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(
        env, "application.AutoFillExtensionContext", &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Load system module by engine failed.");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object.");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AutoFillExtensionContext>(context);
    if (workContext == nullptr) {
        HILOG_ERROR("workContext is nullptr.");
        return;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAutoFillExtensionContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            delete static_cast<std::weak_ptr<AutoFillExtensionContext>*>(data);
        },
        nullptr, nullptr);
}

void JsAutoFillExtension::OnStart(const AAFwk::Want &want)
{
    HILOG_DEBUG("Called.");
    Extension::OnStart(want);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    CallObjectMethod("onCreate", argv, ARGC_ONE);
}

void JsAutoFillExtension::OnStop()
{
    HILOG_DEBUG("Called.");
    AutoFillExtension::OnStop();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    OnStopCallBack();
}

void JsAutoFillExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HILOG_DEBUG("Called.");
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    AutoFillExtension::OnStop();
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallBack();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<Extension> weakPtr = shared_from_this();
    auto asyncCallback = [extensionWeakPtr = weakPtr]() {
        auto JsAutoFillExtension = extensionWeakPtr.lock();
        if (JsAutoFillExtension == nullptr) {
            HILOG_ERROR("Extension is nullptr.");
            return;
        }
        JsAutoFillExtension->OnStopCallBack();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        HILOG_ERROR("Failed to call promise.");
        OnStopCallBack();
    }
}

void JsAutoFillExtension::OnStopCallBack()
{
    HILOG_DEBUG("Called.");
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context.");
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

bool JsAutoFillExtension::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        HILOG_DEBUG("Result is null, no need to call promise.");
        return false;
    }

    napi_env env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        HILOG_DEBUG("Result is not promise, no need to call promise.");
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

bool JsAutoFillExtension::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
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
        HILOG_ERROR("Property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    return true;
}

void JsAutoFillExtension::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr) {
        HILOG_ERROR("Session info is nullptr.");
        return;
    }
    HILOG_DEBUG("Begin. persistentId: %{private}d, winCmd: %{public}d", sessionInfo->persistentId, winCmd);
    Extension::OnCommandWindow(want, sessionInfo, winCmd);
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
            HILOG_DEBUG("Unsupported cmd.");
            break;
    }
    OnCommandWindowDone(sessionInfo, winCmd);
}

void JsAutoFillExtension::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("Called.");
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context.");
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
    HILOG_DEBUG("End.");
}

void JsAutoFillExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HILOG_DEBUG("Begin.");
    Extension::OnCommand(want, restart, startId);
    HILOG_DEBUG("JsAutoFillExtension OnCommand begin restart= %{public}s, startId= %{public}d.",
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
    HILOG_DEBUG("End.");
}

void JsAutoFillExtension::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HILOG_DEBUG("Called.");
    Extension::OnForeground(want, sessionInfo);
    ForegroundWindow(want, sessionInfo);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
}

void JsAutoFillExtension::OnBackground()
{
    HILOG_DEBUG("Called.");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
    Extension::OnBackground();
}

int32_t JsAutoFillExtension::OnReloadInModal(const sptr<AAFwk::SessionInfo> &sessionInfo, const CustomData &customData)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr) {
        HILOG_ERROR("SessionInfo is nullptr.");
        return ERR_NULL_OBJECT;
    }
    AAFwk::WantParamWrapper wrapper(customData.data);
    auto customDataString = wrapper.ToString();
    auto obj = sessionInfo->sessionToken;
    auto &uiWindow = uiWindowMap_[obj];
    if (uiWindow == nullptr) {
        HILOG_ERROR("UI window is nullptr.");
        return ERR_NULL_OBJECT;
    }
    AAFwk::WantParams wantParams;
    wantParams.SetParam(WANT_PARAMS_AUTO_FILL_CMD,
        AAFwk::Integer::Box(static_cast<int32_t>(AutoFillCommand::RELOAD_IN_MODAL)));
    wantParams.SetParam(WANT_PARAMS_CUSTOM_DATA, AAFwk::String::Box(customDataString));
    auto ret = static_cast<int32_t>(uiWindow->TransferExtensionData(wantParams));
    if (ret != ERR_OK) {
        HILOG_ERROR("Transfer extension data failed.");
        return ERR_INVALID_OPERATION;
    }
    return ERR_OK;
}

void JsAutoFillExtension::UpdateRequest(const AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("Called.");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value request = JsAutoFillExtensionUtil::WrapUpdateRequest(wantParams, env);
    if (request == nullptr) {
        HILOG_ERROR("Failed to create update request.");
        return;
    }
    napi_value argv[] = { request };
    CallObjectMethod("onUpdateRequest", argv, ARGC_ONE);
}

bool JsAutoFillExtension::HandleAutoFillCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid session info.");
        return false;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
        auto context = GetContext();
        if (context == nullptr || context->GetAbilityInfo() == nullptr) {
            HILOG_ERROR("Failed to get context.");
            return false;
        }
        option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
        option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
        option->SetParentId(sessionInfo->hostWindowId);
        auto uiWindow = Rosen::Window::Create(option, GetContext(), sessionInfo->sessionToken);
        if (uiWindow == nullptr) {
            HILOG_ERROR("Create ui window error.");
            return false;
        }
        HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value nativeContentSession =
            JsUIExtensionContentSession::CreateJsUIExtensionContentSession(env, sessionInfo, uiWindow);
        napi_ref ref = nullptr;
        napi_create_reference(env, nativeContentSession, 1, &ref);
        contentSessions_.emplace(
            obj, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
        CallJsOnRequest(want, sessionInfo, uiWindow);
        uiWindowMap_[obj] = uiWindow;
    }
    return true;
}

void JsAutoFillExtension::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr.");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context.");
        return;
    }
    context->SetSessionInfo(sessionInfo);

    if (!HandleAutoFillCreate(want, sessionInfo)) {
        HILOG_ERROR("Handle auto fill create failed.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    auto& uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Show();
        HILOG_DEBUG("UI window show.");
        foregroundWindows_.emplace(obj);

        RegisterTransferComponentDataListener(uiWindow);
        AAFwk::WantParams wantParams;
        wantParams.SetParam(WANT_PARAMS_AUTO_FILL_EVENT_KEY, AAFwk::Integer::Box(
            static_cast<int32_t>(JsAutoFillExtensionUtil::AutoFillResultCode::CALLBACK_REMOVE_TIME_OUT)));
        uiWindow->TransferExtensionData(wantParams);
    }
}

void JsAutoFillExtension::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        HILOG_ERROR("Fail to find ui window.");
        return;
    }
    auto& uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(obj);
    }
}

void JsAutoFillExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        HILOG_ERROR("Wrong to find uiWindow");
        return;
    }
    if (contentSessions_.find(obj) != contentSessions_.end() && contentSessions_[obj] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        napi_value argv[] = {contentSessions_[obj]->GetNapiValue()};
        CallObjectMethod("onSessionDestroy", argv, ARGC_ONE);
    }
    auto& uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(obj);
    foregroundWindows_.erase(obj);
    contentSessions_.erase(obj);
    callbacks_.erase(obj);
}

napi_value JsAutoFillExtension::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    HILOG_DEBUG("Called, name is (%{public}s)", name);
    if (!jsObj_) {
        HILOG_ERROR("Not found AutoFillExtension.js.");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get auto fill extension object.");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        HILOG_ERROR("Failed to get '%{public}s' from auto fill extension object.", name);
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    HILOG_DEBUG("Call function: (%{public}s) success.", name);
    napi_call_function(env, obj, method, argc, argv, nullptr);
    return nullptr;
}

void JsAutoFillExtension::CallJsOnRequest(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    HILOG_DEBUG("Called.");
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr.");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        HILOG_ERROR("Env is nullptr.");
        return;
    }
    napi_value nativeContentSession =
        JsUIExtensionContentSession::CreateJsUIExtensionContentSession(env, sessionInfo, uiWindow);
    if (nativeContentSession == nullptr) {
        HILOG_ERROR("Failed to create session.");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, nativeContentSession, 1, &ref);
    contentSessions_.emplace(
        sessionInfo->sessionToken, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));

    napi_value request = JsAutoFillExtensionUtil::WrapFillRequest(want, env);
    if (request == nullptr) {
        HILOG_ERROR("Fill request is nullptr.");
        return;
    }

    napi_value callback = nullptr;
    auto cmdValue = want.GetIntParam(WANT_PARAMS_AUTO_FILL_CMD, 0);
    if (cmdValue == AutoFillCommand::SAVE) {
        callback = JsSaveRequestCallback::CreateJsSaveRequestCallback(env, sessionInfo, uiWindow);
        napi_value argv[] = { nativeContentSession, request, callback };
        CallObjectMethod("onSaveRequest", argv, ARGC_THREE);
    } else if (cmdValue == AutoFillCommand::FILL || cmdValue == AutoFillCommand::RELOAD_IN_MODAL) {
        callback = JsFillRequestCallback::CreateJsFillRequestCallback(env, sessionInfo, uiWindow);
        napi_value argv[] = { nativeContentSession, request, callback };
        CallObjectMethod("onFillRequest", argv, ARGC_THREE);
    } else {
        HILOG_ERROR("Invalid auto fill request type.");
        return;
    }

    napi_ref callbackRef = nullptr;
    napi_create_reference(env, callback, 1, &callbackRef);
    callbacks_.emplace(sessionInfo->sessionToken,
        std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(callbackRef)));
}

void JsAutoFillExtension::RegisterTransferComponentDataListener(const sptr<Rosen::Window> &uiWindow)
{
    HILOG_DEBUG("Called.");
    if (uiWindow == nullptr) {
        HILOG_ERROR("Invalid ui window object.");
        return;
    }

    auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
    if (handler == nullptr) {
        HILOG_ERROR("Failed to create event handler.");
        return;
    }
    uiWindow->RegisterTransferComponentDataListener([this, handler](
        const AAFwk::WantParams &wantParams) {
            handler->PostTask([this, wantParams]() {
                JsAutoFillExtension::UpdateRequest(wantParams);
                }, "JsAutoFillExtension:UpdateRequest");
    });
}
} // namespace AbilityRuntime
} // namespace OHOS
