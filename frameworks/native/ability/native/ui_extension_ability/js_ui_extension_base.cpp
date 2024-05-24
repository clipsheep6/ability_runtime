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

#include "js_ui_extension_base.h"

#include <type_traits>
#include <vector>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_data_struct_converter.h"
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
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace
napi_value AttachUIExtensionBaseContext(napi_env env, void *value, void*)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid parameter.");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext>*>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "invalid context.");
        return nullptr;
    }
    napi_value object = JsUIExtensionContext::CreateJsUIExtensionContext(env, ptr);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "create context error.");
        return nullptr;
    }
    auto contextRef = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &object, 1);
    if (contextRef == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Failed to get LoadSystemModuleByEngine");
        return nullptr;
    }
    auto contextObj = contextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "load context error.");
        return nullptr;
    }
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not object.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

JsUIExtensionBase::JsUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : jsRuntime_(static_cast<JsRuntime&>(*runtime))
{
    abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
}

JsUIExtensionBase::~JsUIExtensionBase()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "destructor.");
    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
    for (auto &item : contentSessions_) {
        jsRuntime_.FreeNativeReference(std::move(item.second));
    }
    contentSessions_.clear();
}

std::shared_ptr<ExtensionCommon> JsUIExtensionBase::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "abilityInfo is nullptr");
        return nullptr;
    }
    if (abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "abilityInfo srcEntrance is empty");
        return nullptr;
    }

    if (record != nullptr) {
        token_ = record->GetToken();
    }
    std::string srcPath(abilityInfo_->moduleName + "/");
    srcPath.append(abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HandleScope handleScope(jsRuntime_);

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsObj_ is nullptr");
        return nullptr;
    }

    BindContext();

    return JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_);
}

void JsUIExtensionBase::BindContext()
{
    HandleScope handleScope(jsRuntime_);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "jsObj_ is nullptr");
        return;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "obj is not object");
        return;
    }
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context_ is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "BindContext CreateJsUIExtensionContext.");
    napi_value contextObj = JsUIExtensionContext::CreateJsUIExtensionContext(env, context_);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Create js ui extension context error.");
        return;
    }
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(
        env, "application.UIExtensionContext", &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context_);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachUIExtensionBaseContext, workContext, nullptr);
    context_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void*) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Finalizer for weak_ptr ui extension context is called");
            if (data == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Finalizer for weak_ptr is nullptr");
                return;
            }
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
        },
        nullptr, nullptr);
}

void JsUIExtensionBase::OnStart(const AAFwk::Want &want, AAFwk::LaunchParam &launchParam)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = {
        CreateJsLaunchParam(env, launchParam),
        napiWant
    };
    CallObjectMethod("onCreate", argv, ARGC_TWO);
}

void JsUIExtensionBase::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
}

void JsUIExtensionBase::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "sessionInfo is nullptr.");
        return;
    }
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

bool JsUIExtensionBase::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
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
    executorCallback->Push(
        [weak = weak_from_this(), sessionInfo, needForeground](AppExecFwk::InsightIntentExecuteResult result) {
            TAG_LOGD(AAFwkTag::UI_EXT, "Begin UI extension transaction callback.");
            auto extension = weak.lock();
            if (extension == nullptr) {
                TAG_LOGE(AAFwkTag::UI_EXT, "UI extension is nullptr.");
                return;
            }

            extension->PostInsightIntentExecuted(sessionInfo, result, needForeground);
        });

    InsightIntentExecutorInfo executorInfo;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = context_->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        executorInfo.hapPath = abilityInfo->hapPath;
        executorInfo.windowMode = abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    }
    executorInfo.token = context_->GetToken();
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

void JsUIExtensionBase::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
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

void JsUIExtensionBase::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called.");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Error to get context");
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
    TAG_LOGD(AAFwkTag::UI_EXT, "end.");
}

void JsUIExtensionBase::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
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

void JsUIExtensionBase::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env is nullptr.");
        return;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get want");
        return;
    }
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    if (napiStartId == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get startId");
        return;
    }
    napi_value argv[] = { napiWant, napiStartId };
    CallObjectMethod("onRequest", argv, ARGC_TWO);
}

void JsUIExtensionBase::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        bool finish = ForegroundWindowWithInsightIntent(want, sessionInfo, true);
        if (finish) {
            return;
        }
    }

    ForegroundWindow(want, sessionInfo);
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onForeground");
}

void JsUIExtensionBase::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onBackground");
}

bool JsUIExtensionBase::CallJsOnSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow, const uint64_t &uiExtensionComponentId)
{
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "env is nullptr.");
        return false;
    }
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get want");
        return false;
    }
    napi_value nativeContentSession = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
        env, sessionInfo, uiWindow, context_, abilityResultListeners_);
    if (nativeContentSession == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get contentSession");
        return false;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, nativeContentSession, 1, &ref);
    contentSessions_.emplace(
        uiExtensionComponentId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    napi_value argv[] = { napiWant, nativeContentSession };
    CallObjectMethod("onSessionCreate", argv, ARGC_TWO);
    return true;
}

bool JsUIExtensionBase::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo.");
        return false;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ", element: %{public}s.",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetURI().c_str());
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo.");
        return false;
    }
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
        if (context_ == nullptr || context_->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get context");
            return false;
        }
        option->SetWindowName(context_->GetBundleName() + context_->GetAbilityInfo()->name);
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
        option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
        option->SetParentId(sessionInfo->hostWindowId);
        auto uiWindow = Rosen::Window::Create(option, context_, sessionInfo->sessionToken);
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "create ui window error.");
            return false;
        }
        if (!CallJsOnSessionCreate(want, sessionInfo, uiWindow, componentId)) {
            return false;
        }
        uiWindowMap_[componentId] = uiWindow;
        if (context_->GetWindow() == nullptr) {
            context_->SetWindow(uiWindow);
        }
    }
    return true;
}

void JsUIExtensionBase::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ".", sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Show();
        foregroundWindows_.emplace(componentId);
    }
}

void JsUIExtensionBase::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
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
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(componentId);
    }
}

void JsUIExtensionBase::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
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
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        napi_value argv[] = { contentSessions_[componentId]->GetNapiValue() };
        CallObjectMethod("onSessionDestroy", argv, ARGC_ONE);
    }
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(componentId);
    foregroundWindows_.erase(componentId);
    contentSessions_.erase(componentId);
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
}

napi_value JsUIExtensionBase::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CallObjectMethod(%{public}s), begin", name);
    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found .js file");
        return nullptr;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get '%{public}s' object", name);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "CallFunction(%{public}s), success", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}

void JsUIExtensionBase::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called.");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is nullptr");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateGlobalConfig(configuration, context_->GetResourceManager());

    HandleScope handleScope(jsRuntime_);
    auto fullConfig = context_->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UI_EXT, "configuration is nullptr.");
        return;
    }
    napi_env env = jsRuntime_.GetNapiEnv();
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration =
        OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    if (napiConfiguration == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get configuration.");
        return;
    }
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
}

void JsUIExtensionBase::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not found .js file");
        return;
    }
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get object");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Failed to get onDump");
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
            TAG_LOGE(AAFwkTag::UI_EXT, "Parse dumpInfoStr error");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "Dump info size: %{public}zu", info.size());
}

void JsUIExtensionBase::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "not attached to any runtime context!");
        return;
    }
    context_->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "abilityResultListensers is nullptr");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
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
