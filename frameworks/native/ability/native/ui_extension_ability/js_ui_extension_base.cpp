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

#include "js_ui_extension_base.h"

#include <type_traits>
#include <vector>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"
#include "js_ui_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "ui_extension_window_command.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace
napi_value AttachUIExtensionBaseContext(napi_env env, void *value, void*)
{
    HILOG_DEBUG("called");
    if (value == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }
    napi_value object = JsUIExtensionContext::CreateJsUIExtensionContext(env, ptr);
    if (object == nullptr) {
        HILOG_ERROR("create context error.");
        return nullptr;
    }
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &object, 1)->GetNapiValue();
    if (contextObj == nullptr) {
        HILOG_ERROR("load context error.");
        return nullptr;
    }
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("not object.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                HILOG_ERROR("Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

JsUIExtensionBase::JsUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : jsRuntime_(static_cast<JsRuntime&>(*runtime))
{}

JsUIExtensionBase::~JsUIExtensionBase()
{
    HILOG_DEBUG("destructor.");
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
    for (auto &item : contentSessions_) {
        jsRuntime_.FreeNativeReference(std::move(item.second));
    }
    contentSessions_.clear();
}

std::shared_ptr<JsExtensionCommon> JsUIExtensionBase::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("called");
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return nullptr;
    }
    if (abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("abilityInfo srcEntrance is empty");
        return nullptr;
    }
    std::string srcPath(abilityInfo_->moduleName + "/");
    srcPath.append(abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("jsObj_ is nullptr");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("obj is not object");
        return nullptr;
    }

    BindContext(env, obj);

    return JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_);
}

void JsUIExtensionBase::BindContext(napi_env env, napi_value obj)
{
    if (context_ == nullptr) {
        HILOG_ERROR("context_ is nullptr");
        return;
    }
    if (obj == nullptr) {
        HILOG_ERROR("obj is nullptr");
        return;
    }
    HILOG_DEBUG("BindContext CreateJsUIExtensionContext.");
    napi_value contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(env, context_);
    if (contextObj == nullptr) {
        HILOG_ERROR("Create js ui extension context error.");
        return;
    }

    shellContextRef_ =
        JsRuntime::LoadSystemModuleByEngine(env, "application.UIExtensionContext", &contextObj, ARGC_ONE);
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context_);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, workContext, nullptr);
    context_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                HILOG_ERROR("Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
}

void JsUIExtensionBase::OnStart(const AAFwk::Want &want)
{
    HILOG_DEBUG("called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onCreate");
}

void JsUIExtensionBase::OnStop()
{
    HILOG_DEBUG("called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
}

void JsUIExtensionBase::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("called");
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr.");
        return;
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
    if (context_ == nullptr) {
        HILOG_ERROR("Error to get context");
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
        context_->GetToken(), sessionInfo, winCmd, abilityCmd);
}

void JsUIExtensionBase::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    HILOG_DEBUG("called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        HILOG_ERROR("env is nullptr.");
        return;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (napiWant == nullptr) {
        HILOG_ERROR("Failed to get want");
        return;
    }
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    if (napiStartId == nullptr) {
        HILOG_ERROR("Failed to get startId");
        return;
    }
    napi_value argv[] = { napiWant, napiStartId };
    CallObjectMethod("onRequest", argv, ARGC_TWO);
}

void JsUIExtensionBase::OnForeground(const Want &want)
{
    HILOG_DEBUG("called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
}

void JsUIExtensionBase::OnBackground()
{
    HILOG_DEBUG("called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
}

bool JsUIExtensionBase::CallJsOnSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow, const sptr<IRemoteObject> &sessionToken)
{
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        HILOG_ERROR("env is nullptr.");
        return false;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (napiWant == nullptr) {
        HILOG_ERROR("Failed to get want");
        return false;
    }
    napi_value nativeContentSession =
        JsUIExtensionContentSession::CreateJsUIExtensionContentSession(env, sessionInfo, uiWindow);
    if (nativeContentSession == nullptr) {
        HILOG_ERROR("Failed to get contentSession");
        return false;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, nativeContentSession, 1, &ref);
    contentSessions_.emplace(
        sessionToken, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    napi_value argv[] = { napiWant, nativeContentSession };
    CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
    return true;
}

void JsUIExtensionBase::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
        if (context_ == nullptr || context_->GetAbilityInfo() == nullptr) {
            HILOG_ERROR("Failed to get context");
            return;
        }
        option->SetWindowName(context_->GetBundleName() + context_->GetAbilityInfo()->name);
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
        option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
        option->SetParentId(sessionInfo->hostWindowId);
        auto uiWindow = Rosen::Window::Create(option, context_, sessionInfo->sessionToken);
        if (uiWindow == nullptr) {
            HILOG_ERROR("create ui window error.");
            return;
        }
        if (!CallJsOnSessionCreate(want, sessionInfo, uiWindow, obj)) {
            return;
        }
        uiWindowMap_[obj] = uiWindow;
    }
    auto &uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Show();
        foregroundWindows_.emplace(obj);
    }
}

void JsUIExtensionBase::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        HILOG_ERROR("Fail to find uiWindow");
        return;
    }
    auto &uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(obj);
    }
}

void JsUIExtensionBase::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        HILOG_ERROR("Fail to find uiWindow");
        return;
    }
    if (contentSessions_.find(obj) != contentSessions_.end() && contentSessions_[obj] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        napi_value argv[] = { contentSessions_[obj]->GetNapiValue() };
        CallObjectMethod("onSessionDestroy", argv, ARGC_ONE);
    }
    auto &uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(obj);
    foregroundWindows_.erase(obj);
    contentSessions_.erase(obj);
}

napi_value JsUIExtensionBase::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    HILOG_DEBUG("CallObjectMethod(%{public}s), begin", name);
    if (!jsObj_) {
        HILOG_ERROR("Not found .js file");
        return nullptr;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        HILOG_ERROR("Failed to get '%{public}s' object", name);
        return nullptr;
    }
    HILOG_DEBUG("CallFunction(%{public}s), success", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsUIExtensionBase::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HILOG_DEBUG("called.");
    if (context_ == nullptr) {
        HILOG_ERROR("context is nullptr");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    auto fullConfig = context_->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration =
        OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    if (napiConfiguration == nullptr) {
        HILOG_ERROR("Failed to get configuration.");
        return;
    }
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
}

void JsUIExtensionBase::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        HILOG_ERROR("Not found .js file");
        return;
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            HILOG_ERROR("Failed to get onDump");
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
            HILOG_ERROR("Parse dumpInfoStr error");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}

void JsUIExtensionBase::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
{
    HILOG_DEBUG("called");
    if (context_ == nullptr) {
        HILOG_WARN("not attached to any runtime context!");
        return;
    }
    context_->OnAbilityResult(requestCode, resultCode, resultData);
}

void JsUIExtensionBase::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

void JsUIExtensionBase::SetContext(const std::shared_ptr<UIExtensionContext> &context)
{
    context_ = context;
}
} // namespace AbilityRuntime
} // namespace OHOS