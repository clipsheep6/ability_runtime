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

#include "js_share_extension.h"

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
NativeValue *AttachShareExtensionContext(NativeEngine *engine, void *value, void*)
{
    HILOG_DEBUG("called.");
    if (value == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }
    NativeValue *object = JsUIExtensionContext::CreateJsUIExtensionContext(*engine, ptr);
    if (object == nullptr) {
        HILOG_ERROR("create context error.");
        return nullptr;
    }
    auto contextObj = JsRuntime::LoadSystemModuleByEngine(engine, "application.UIExtensionContext", &object, 1)->Get();
    if (contextObj == nullptr) {
        HILOG_ERROR("load context error.");
        return nullptr;
    }
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_ERROR("convert context error.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachShareExtensionContext, value, nullptr);

    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    nObject->SetNativePointer(
        workContext,
        [](NativeEngine*, void *data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                HILOG_ERROR("Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr);
    return contextObj;
}

JsShareExtension *JsShareExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new JsShareExtension(static_cast<JsRuntime&>(*runtime));
}

JsShareExtension::JsShareExtension(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime) {}

JsShareExtension::~JsShareExtension()
{
    HILOG_DEBUG("destructor.");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsShareExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("called.");
    ShareExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("JsShareExtension Init abilityInfo srcEntrance is empty");
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);
    auto &engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsShareExtension object");
        return;
    }

    BindContext(engine, obj);

    SetExtensionCommon(
        JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));
}

void JsShareExtension::BindContext(NativeEngine &engine, NativeObject *obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsShareExtension object");
        return;
    }
    HILOG_DEBUG("BindContext CreateJsUIExtensionContext.");
    NativeValue *contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(engine, context);
    if (contextObj == nullptr) {
        HILOG_ERROR("Create js ui extension context error.");
        return;
    }

    shellContextRef_ =
        JsRuntime::LoadSystemModuleByEngine(&engine, "application.UIExtensionContext", &contextObj, ARGC_ONE);
    contextObj = shellContextRef_->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context);
    nativeObj->ConvertToNativeBindingObject(
        &engine, DetachCallbackFunc, AttachShareExtensionContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);

    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine*, void *data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                HILOG_ERROR("Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr);
}

void JsShareExtension::OnStart(const AAFwk::Want &want)
{
    HILOG_DEBUG("called.");
    Extension::OnStart(want);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onCreate");
}

void JsShareExtension::OnStop()
{
    HILOG_DEBUG("called.");
    ShareExtension::OnStop();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
}

void JsShareExtension::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("begin. persistentId: %{private}d, winCmd: %{public}d", sessionInfo->persistentId, winCmd);
    if (sessionInfo == nullptr) {
        HILOG_ERROR("sessionInfo is nullptr.");
        return;
    }
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
            HILOG_DEBUG("unsupported cmd.");
            break;
    }
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
}

void JsShareExtension::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    Extension::OnCommand(want, restart, startId);
    HILOG_DEBUG("begin restart = %{public}s, startId = %{public}d.", restart ? "true" : "false", startId);
    HandleScope handleScope(jsRuntime_);
    NativeEngine *nativeEngine = &jsRuntime_.GetNativeEngine();
    if (nativeEngine == nullptr) {
        HILOG_ERROR("NativeEngine is nullptr.");
        return;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
    NativeValue *nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    if (nativeWant == nullptr) {
        HILOG_ERROR("Failed to get want");
        return;
    }
    napi_value napiStartId = nullptr;
    napi_create_int32(reinterpret_cast<napi_env>(nativeEngine), startId, &napiStartId);
    NativeValue *nativeStartId = reinterpret_cast<NativeValue*>(napiStartId);
    if (nativeStartId == nullptr) {
        HILOG_ERROR("Failed to get startId");
        return;
    }
    NativeValue *argv[] = { nativeWant, nativeStartId };
    CallObjectMethod("onRequest", argv, ARGC_TWO);
}

void JsShareExtension::OnForeground(const Want &want)
{
    HILOG_DEBUG("called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnForeground(want);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
}

void JsShareExtension::OnBackground()
{
    HILOG_DEBUG("called.");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
    Extension::OnBackground();
}

void JsShareExtension::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called.");
    if (sessionInfo == nullptr || sessionInfo->sessionToken == nullptr) {
        HILOG_ERROR("Invalid sessionInfo.");
        return;
    }
    auto obj = sessionInfo->sessionToken;
    if (uiWindowMap_.find(obj) == uiWindowMap_.end()) {
        sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
        auto context = GetContext();
        if (context == nullptr || context->GetAbilityInfo() == nullptr) {
            HILOG_ERROR("Failed to get context");
            return;
        }
        option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
        option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
        option->SetParentId(sessionInfo->hostWindowId);
        auto uiWindow = Rosen::Window::Create(option, GetContext(), sessionInfo->sessionToken);
        if (uiWindow == nullptr) {
            HILOG_ERROR("create ui window error.");
            return;
        }
        HandleScope handleScope(jsRuntime_);
        NativeEngine *nativeEngine = &jsRuntime_.GetNativeEngine();
        if (nativeEngine == nullptr) {
            HILOG_ERROR("NativeEngine is nullptr.");
            return;
        }
        napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(nativeEngine), want);
        NativeValue *nativeWant = reinterpret_cast<NativeValue*>(napiWant);
        if (nativeWant == nullptr) {
            HILOG_ERROR("Failed to get want");
            return;
        }
        NativeValue *nativeContentSession =
            JsUIExtensionContentSession::CreateJsUIExtensionContentSession(*nativeEngine, sessionInfo, uiWindow);
        if (nativeContentSession == nullptr) {
            HILOG_ERROR("Failed to get contentSession");
            return;
        }
        contentSessions_.emplace(
            obj, std::shared_ptr<NativeReference>(nativeEngine->CreateReference(nativeContentSession, 1)));
        NativeValue *argv[] = { nativeWant, nativeContentSession };
        CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
        uiWindowMap_[obj] = uiWindow;
    }
    auto &uiWindow = uiWindowMap_[obj];
    if (uiWindow) {
        uiWindow->Show();
        foregroundWindows_.emplace(obj);
    }
}

void JsShareExtension::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called.");
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

void JsShareExtension::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HILOG_DEBUG("called.");
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
        NativeValue *argv[] = { contentSessions_[obj]->Get() };
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

NativeValue *JsShareExtension::CallObjectMethod(const char *name, NativeValue *const *argv, size_t argc)
{
    HILOG_DEBUG("CallObjectMethod(%{public}s), begin", name);
    if (!jsObj_) {
        HILOG_ERROR("Not found ShareExtension.js");
        return nullptr;
    }
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    NativeValue *value = jsObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ShareExtension object");
        return nullptr;
    }

    NativeValue *method = obj->GetProperty(name);
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from ShareExtension object", name);
        return nullptr;
    }
    HILOG_DEBUG("JsShareExtension CallFunction(%{public}s), success", name);
    return nativeEngine.CallFunction(value, method, argv, argc);
}

void JsShareExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HILOG_DEBUG("called.");
    Extension::OnConfigurationUpdated(configuration);

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    JsExtensionContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration =
        OHOS::AppExecFwk::WrapConfiguration(reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue *jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    if (jsConfiguration == nullptr) {
        HILOG_ERROR("Failed to get configuration.");
        return;
    }
    CallObjectMethod("onConfigurationUpdate", &jsConfiguration, ARGC_ONE);
}

void JsShareExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("called.");
    Extension::Dump(params, info);
    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    NativeValue *arrayValue = nativeEngine.CreateArray(params.size());
    if (arrayValue == nullptr) {
        HILOG_ERROR("create array failed");
        return;
    }
    NativeArray *array = ConvertNativeValueTo<NativeArray>(arrayValue);
    if (array == nullptr) {
        HILOG_ERROR("convert array failed");
        return;
    }
    uint32_t index = 0;
    for (const auto &param : params) {
        array->SetElement(index++, CreateJsValue(nativeEngine, param));
    }
    NativeValue *argv[] = { arrayValue };

    if (!jsObj_) {
        HILOG_ERROR("Not found ShareExtension.js");
        return;
    }

    NativeValue *value = jsObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get ShareExtension object");
        return;
    }

    NativeValue *method = obj->GetProperty("onDump");
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        method = obj->GetProperty("dump");
        if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
            HILOG_ERROR("Failed to get onDump from ShareExtension object");
            return;
        }
    }
    NativeValue *dumpInfo = nativeEngine.CallFunction(value, method, argv, ARGC_ONE);
    if (dumpInfo == nullptr) {
        HILOG_ERROR("dumpInfo is nullptr.");
        return;
    }
    NativeArray *dumpInfoNative = ConvertNativeValueTo<NativeArray>(dumpInfo);
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

void JsShareExtension::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
{
    HILOG_DEBUG("called.");
    Extension::OnAbilityResult(requestCode, resultCode, resultData);
    auto context = GetContext();
    if (context == nullptr) {
        HILOG_WARN("not attached to any runtime context!");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
}
} // namespace AbilityRuntime
} // namespace OHOS