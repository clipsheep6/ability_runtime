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

#include "js_ui_ability.h"

#include <cstdlib>
#include <regex>

#include "ability_business_error.h"
#include "ability_delegator_registry.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "app_recovery.h"
#include "connection_manager.h"
#include "context/application_context.h"
#include "context/context.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "insight_intent_execute_param.h"
#include "js_ability_context.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "js_window_stage.h"
#endif
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "scene_board_judgement.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
const std::string METHOD_NAME = "WindowScene::GoForeground";
#endif
// Numerical base (radix) that determines the valid characters and their interpretation.
const int32_t BASE_DISPLAY_ID_NUM (10);
constexpr const int32_t API12 = 12;
constexpr const int32_t API_VERSION_MOD = 100;

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

napi_value OnContinuePromiseCallback(napi_env env, napi_callback_info info)
{
    void *data = nullptr;
    size_t argc = 1;
    napi_value argv = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, &argv, nullptr, &data), nullptr);
    int32_t onContinueRes = 0;
    if (!ConvertFromJsValue(env, argv, onContinueRes)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get js return value failed.");
        return nullptr;
    }
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<int32_t> *>(data);
    callbackInfo->Call(onContinueRes);
    AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Destroy(callbackInfo);
    data = nullptr;

    return nullptr;
}
} // namespace

napi_value AttachJsAbilityContext(napi_env env, void *value, void *extValue)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (value == nullptr || extValue == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Invalid context.");
        return nullptr;
    }
    std::shared_ptr<NativeReference> systemModule = nullptr;
    auto screenModePtr = reinterpret_cast<std::weak_ptr<int32_t> *>(extValue)->lock();
    if (screenModePtr == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Invalid screenModePtr.");
        return nullptr;
    }
    if (*screenModePtr == AAFwk::IDLE_SCREEN_MODE) {
        auto uiAbiObject = CreateJsAbilityContext(env, ptr);
        CHECK_POINTER_AND_RETURN(uiAbiObject, nullptr);
        systemModule = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(env,
            "application.AbilityContext", &uiAbiObject, 1).release());
    } else {
        auto emUIObject = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            ptr, nullptr, *screenModePtr);
        CHECK_POINTER_AND_RETURN(emUIObject, nullptr);
        systemModule = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(env,
            "application.EmbeddableUIAbilityContext", &emUIObject, 1).release());
    }
    CHECK_POINTER_AND_RETURN(systemModule, nullptr);
    auto contextObj = systemModule->GetNapiValue();
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, value, extValue);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(ptr);
    if (workContext != nullptr) {
        napi_wrap(env, contextObj, workContext,
            [](napi_env, void* data, void*) {
              TAG_LOGD(AAFwkTag::UIABILITY, "Finalizer for weak_ptr ability context is called");
              delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
            },
            nullptr, nullptr);
    }
    return contextObj;
}

UIAbility *JsUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) JsUIAbility(static_cast<JsRuntime &>(*runtime));
}

JsUIAbility::JsUIAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

JsUIAbility::~JsUIAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (abilityContext_ != nullptr) {
        abilityContext_->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsAbilityObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
#ifdef SUPPORT_GRAPHICS
    jsRuntime_.FreeNativeReference(std::move(jsWindowStageObj_));
#endif
}

void JsUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityLocalRecord is nullptr.");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityInfo is nullptr.");
        return;
    }
    UIAbility::Init(record, application, handler, token);
#ifdef SUPPORT_GRAPHICS
    if (abilityContext_ != nullptr) {
        AppExecFwk::AppRecovery::GetInstance().AddAbility(
            shared_from_this(), abilityContext_->GetAbilityInfo(), abilityContext_->GetToken());
    }
#endif
    std::string srcPath(abilityInfo->package);
    if (!abilityInfo->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append("/assets/js/");
        if (!abilityInfo->srcPath.empty()) {
            srcPath.append(abilityInfo->srcPath);
        }
        srcPath.append("/").append(abilityInfo->name).append(".abc");
    } else {
        if (abilityInfo->srcEntrance.empty()) {
            TAG_LOGE(AAFwkTag::UIABILITY, "SrcEntrance is empty.");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        TAG_LOGD(AAFwkTag::UIABILITY, "JsAbility srcPath is %{public}s.", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    SetAbilityContext(abilityInfo, record->GetWant(), moduleName, srcPath);
}

void JsUIAbility::SetAbilityContext(std::shared_ptr<AbilityInfo> abilityInfo,
    std::shared_ptr<AAFwk::Want> want, const std::string &moduleName, const std::string &srcPath)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    jsAbilityObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo->hapPath, abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE);
    if (jsAbilityObj_ == nullptr || abilityContext_ == nullptr || want == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "jsAbilityObj_ or abilityContext_ or want is nullptr.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to check type");
        return;
    }
    napi_value contextObj = nullptr;
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::ScreenMode::IDLE_SCREEN_MODE);
    CreateJSContext(env, contextObj, screenMode);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "shellContextRef_ is nullptr.");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get ability native object.");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(abilityContext_);
    CHECK_POINTER(workContext);
    screenModePtr_ = std::make_shared<int32_t>(screenMode);
    auto workScreenMode = new (std::nothrow) std::weak_ptr<int32_t>(screenModePtr_);
    CHECK_POINTER(workScreenMode);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, workContext, workScreenMode);
    abilityContext_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    TAG_LOGD(AAFwkTag::UIABILITY, "Set ability context");
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->SetJsAbility(reinterpret_cast<uintptr_t>(workContext));
    }
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        }, nullptr, nullptr);
    TAG_LOGD(AAFwkTag::UIABILITY, "Init end.");
}

void JsUIAbility::CreateJSContext(napi_env env, napi_value &contextObj, int32_t screenMode)
{
    if (screenMode == AAFwk::IDLE_SCREEN_MODE) {
        contextObj = CreateJsAbilityContext(env, abilityContext_);
        CHECK_POINTER(contextObj);
        shellContextRef_ = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(
            env, "application.AbilityContext", &contextObj, 1).release());
    } else {
        contextObj = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
            abilityContext_, nullptr, screenMode);
        CHECK_POINTER(contextObj);
        shellContextRef_ = std::shared_ptr<NativeReference>(JsRuntime::LoadSystemModuleByEngine(
            env, "application.EmbeddableUIAbilityContext", &contextObj, 1).release());
    }
}

void JsUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!jsAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Not found Ability.js.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Error to get Ability object");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsWant is nullptr.");
        return;
    }

    napi_set_named_property(env, obj, "launchWant", jsWant);
    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    auto launchParam = GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = {
        jsWant,
        CreateJsLaunchParam(env, launchParam),
    };
    std::string methodName = "OnStart";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onCreate", argv, ArraySize(argv));
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformStart.");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityCreate(jsAbilityObj_);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    FreezeUtil::LifecycleFlow flow = { AbilityContext::token_, state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; JsUIAbility::" + methodName +
        "; the " + methodName + " begin.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}

void JsUIAbility::AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    FreezeUtil::LifecycleFlow flow = { AbilityContext::token_, state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; JsUIAbility::" + methodName +
        "; the " + methodName + " end.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}

int32_t JsUIAbility::OnShare(WantParams &wantParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object.");
        return ERR_INVALID_VALUE;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get Ability object.");
        return ERR_INVALID_VALUE;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParam);
    napi_value argv[] = {
        jsWantParams,
    };
    CallObjectMethod("onShare", argv, ArraySize(argv));
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParam);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
    return ERR_OK;
}

void JsUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set terminating true.");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void JsUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set terminating true.");
        abilityContext_->SetTerminating(true);
    }

    UIAbility::OnStop();

    HandleScope handleScope(jsRuntime_);
    napi_value result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallback();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<UIAbility> weakPtr = shared_from_this();
    auto asyncCallback = [abilityWeakPtr = weakPtr]() {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Ability is nullptr.");
            return;
        }
        ability->OnStopCallback();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to call promise.");
        OnStopCallback();
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void JsUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformStop.");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::UIABILITY, "The service connection is not disconnected.");
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityDestroy(jsAbilityObj_);
    }
}

#ifdef SUPPORT_GRAPHICS
void JsUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsAppWindowStage is nullptr.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageCreate");
        std::string methodName = "OnSceneCreated";
        AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));
        AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformScenceCreated.");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageCreate(jsAbilityObj_, jsWindowStageObj_);
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::OnSceneRestored()
{
    UIAbility::OnSceneRestored();
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");
    HandleScope handleScope(jsRuntime_);
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsAppWindowStage is nullptr.");
        return;
    }
    UpdateJsWindowStage(jsAppWindowStage->GetNapiValue());
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformScenceRestored.");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
}

void JsUIAbility::OnSceneWillDestroy()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    HandleScope handleScope(jsRuntime_);
    if (jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "jsWindowStageObj_ is nullptr.");
        return;
    }
    napi_value argv[] = {jsWindowStageObj_->GetNapiValue()};
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageWillDestroy");
        std::string methodName = "onWindowStageWillDestroy";
        CallObjectMethod("onWindowStageWillDestroy", argv, ArraySize(argv));
    }
}

void JsUIAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();
    HandleScope handleScope(jsRuntime_);
    UpdateJsWindowStage(nullptr);
    CallObjectMethod("onWindowStageDestroy");

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Call UnregisterDisplayMoveListener.");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformScenceDestroyed.");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageDestroy(jsAbilityObj_, jsWindowStageObj_);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    UIAbility::OnForeground(want);
    if (CheckIsSilentForeground()) {
        TAG_LOGD(AAFwkTag::UIABILITY, "silent foreground, do not call 'onForeground'");
        return;
    }
    CallOnForegroundFunc(want);
}

void JsUIAbility::CallOnForegroundFunc(const Want &want)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsAbilityObj_ is nullptr.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get Ability object.");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "jsWant is nullptr.");
        return;
    }

    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onForeground", &jsWant, 1);
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformForeground.");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityForeground(jsAbilityObj_);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    std::string methodName = "OnBackground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    CallObjectMethod("onBackground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    UIAbility::OnBackground();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformBackground.");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityBackground(jsAbilityObj_);
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

bool JsUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onBackPressed", nullptr, 0, true, false);
    bool defaultRet = BackPressDefaultValue();
    if (jsValue == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "jsValue is nullptr, return defaultRet %{public}d.", defaultRet);
        return defaultRet;
    }
    bool ret = defaultRet;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get js value failed.");
        return defaultRet;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ret is %{public}d.", ret);
    return ret;
}

bool JsUIAbility::OnPrepareTerminate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnPrepareTerminate();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onPrepareToTerminate", nullptr, 0, true);
    bool ret = false;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get js value failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ret is %{public}d.", ret);
    return ret;
}

std::unique_ptr<NativeReference> JsUIAbility::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to create jsWindowSatge object.");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.WindowStage", &jsWindowStage, 1);
}

void JsUIAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool JsUIAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void JsUIAbility::RestorePageStack(const Want &want)
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
            TAG_LOGE(AAFwkTag::UIABILITY, "Content storage is nullptr.");
        }
    }
}

void JsUIAbility::AbilityContinuationOrRecover(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // multi-instance ability continuation
    TAG_LOGD(AAFwkTag::UIABILITY, "Launch reason is %{public}d.", launchParam_.launchReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
        NotifyContinuationResult(want, true);
    } else if (ShouldRecoverState(want)) {
        std::string pageStack = abilityRecovery_->GetSavedPageStack(AppExecFwk::StateReason::DEVELOPER_REQUEST);
        HandleScope handleScope(jsRuntime_);
        auto env = jsRuntime_.GetNapiEnv();
        auto mainWindow = scene_->GetMainWindow();
        if (mainWindow != nullptr) {
            mainWindow->NapiSetUIContent(pageStack, env, abilityContext_->GetContentStorage()->GetNapiValue(), true);
        } else {
            TAG_LOGE(AAFwkTag::UIABILITY, "MainWindow is nullptr.");
        }
        OnSceneRestored();
    } else {
        OnSceneCreated();
    }
}

void JsUIAbility::DoOnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityContext or sceneListener_ is nullptr.");
            return;
        }
        DoOnForegroundForSceneIsNull(want);
    } else {
        auto window = scene_->GetMainWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(
                Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            windowMode_ = windowMode;
            TAG_LOGD(AAFwkTag::UIABILITY, "Set window mode is %{public}d.", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    if (CheckIsSilentForeground()) {
        TAG_LOGI(AAFwkTag::UIABILITY, "silent foreground, do not show window");
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "Move scene to foreground, sceneFlag_: %{public}d.", UIAbility::sceneFlag_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void JsUIAbility::DoOnForegroundForSceneIsNull(const Want &want)
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
            TAG_LOGD(AAFwkTag::UIABILITY, "Success displayId is %{public}d.", displayId);
        } else {
            TAG_LOGE(AAFwkTag::UIABILITY, "Failed to formatRegex: [%{public}s].", strDisplayId.c_str());
        }
    }
    auto option = GetWindowOption(want);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto sessionToken = GetSessionToken();
    auto identityToken = GetIdentityToken();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken, identityToken);
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to init window scene.");
        return;
    }

    AbilityContinuationOrRecover(want);
    auto window = scene_->GetMainWindow();
    if (window) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call RegisterDisplayMoveListener, windowId: %{public}d.", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "abilityDisplayMoveListener_ is nullptr.");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
}

void JsUIAbility::RequestFocus(const Want &want)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle: begin.");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "scene_ is nullptr.");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(
            Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::UIABILITY, "Set window mode is %{public}d.", windowMode);
    }
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle: end.");
}

void JsUIAbility::ContinuationRestore(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Is not in continuation or scene_ is nullptr.");
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

std::shared_ptr<NativeReference> JsUIAbility::GetJsWindowStage()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsWindowSatge is nullptr.");
    }
    return jsWindowStageObj_;
}

const JsRuntime &JsUIAbility::GetJsRuntime()
{
    return jsRuntime_;
}

void JsUIAbility::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Intent execute param invalid.");
        RequestFocus(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Begin request focus.");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "ability is nullptr.");
            return;
        }
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "Execute insight intent failed.");
    }
}

void JsUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Intent execute param invalid.");
        OnForeground(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }
    UIAbility::OnForeground(want);

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Begin call onForeground.");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "ability is nullptr.");
            return;
        }
        ability->CallOnForegroundFunc(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "Execute insight intent failed.");
    }
}

void JsUIAbility::ExecuteInsightIntentBackground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Intent execute param invalid.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        TAG_LOGE(AAFwkTag::UIABILITY, "Execute insight intent failed.");
    }
}

bool JsUIAbility::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo& executeInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");

    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Param invalid.");
        return false;
    }

    if (executeParam->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND
        && jsWindowStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Param invalid.");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executeInfo.srcEntry = wantParams.GetStringParam("ohos.insightIntent.srcEntry");
    executeInfo.hapPath = abilityInfo_->hapPath;
    executeInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executeInfo.windowMode = windowMode_;
    executeInfo.token = context->GetToken();
    if (jsWindowStageObj_ != nullptr) {
        executeInfo.pageLoader = jsWindowStageObj_;
    }
    executeInfo.executeParam = executeParam;
    return true;
}
#endif

int32_t JsUIAbility::OnContinue(WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::UIABILITY, "OnContinue called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get Ability object.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }
    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value result = CallObjectMethod("onContinue", &jsWantParams, 1, true);
    int32_t onContinueRes = 0;
    if (!CheckPromise(result)) {
        if (!ConvertFromJsValue(env, result, onContinueRes)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "'onContinue' is not implemented");
            return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
        }
    } else {
        if (!CallPromise(result, onContinueRes)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Failed to call promise.");
            return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
        }
    }
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityContinue(jsAbilityObj_);
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "OnContinue end.");
    return onContinueRes;
}

int32_t JsUIAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AppRecoveryFailed to get AbilityStage object.");
        return -1;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AppRecovery Failed to get Ability object.");
        return -1;
    }

    napi_value methodOnSaveState = nullptr;
    napi_get_named_property(env, obj, "onSaveState", &methodOnSaveState);
    if (methodOnSaveState == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AppRecovery Failed to get 'onSaveState' from Ability object.");
        return -1;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value jsReason = CreateJsValue(env, reason);
    napi_value args[] = { jsReason, jsWantParams };
    napi_value result = nullptr;
    napi_call_function(env, obj, methodOnSaveState, 2, args, &result); // 2:args size
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);

    int32_t numberResult = 0;
    if (!ConvertFromJsValue(env, result, numberResult)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AppRecovery no result return from onSaveState.");
        return -1;
    }
    return numberResult;
}

void JsUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    UIAbility::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is nullptr.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto fullConfig = abilityContext_->GetConfiguration();
    if (fullConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Configuration is nullptr.");
        return;
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "fullConfig: %{public}s", fullConfig->GetName().c_str());
    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
    JsAbilityContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get Ability object.");
        return;
    }

    napi_value jslevel = CreateJsValue(env, level);
    napi_value argv[] = { jslevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

void JsUIAbility::UpdateContextConfiguration()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is nullptr.");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    JsAbilityContext::ConfigurationUpdated(env, shellContextRef_, abilityContext_->GetConfiguration());
}

void JsUIAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get AbilityStage object.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get Ability object.");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get want.");
        return;
    }

    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    auto launchParam = GetLaunchParam();
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = {
        jsWant,
        CreateJsLaunchParam(env, launchParam),
    };
    std::string methodName = "OnNewWant";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onNewWant", argv, ArraySize(argv));
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void JsUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    UIAbility::OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is nullptr.");
        return;
    }
    abilityContext_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

sptr<IRemoteObject> JsUIAbility::CallRequest()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Obj is nullptr.");
        return nullptr;
    }

    if (remoteCallee_ != nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "RemoteCallee is nullptr.");
        return remoteCallee_;
    }

    HandleScope handleScope(jsRuntime_);
    TAG_LOGD(AAFwkTag::UIABILITY, "Set runtime scope.");
    auto env = jsRuntime_.GetNapiEnv();
    auto obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Value is nullptr.");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onCallRequest", &method);
    bool isCallable = false;
    napi_is_callable(env, method, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Method is %{public}s.", method == nullptr ? "nullptr" : "not func");
        return nullptr;
    }

    napi_value remoteJsObj = nullptr;
    napi_call_function(env, obj, method, 0, nullptr, &remoteJsObj);
    if (remoteJsObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsObj is nullptr.");
        return nullptr;
    }

    remoteCallee_ = SetNewRuleFlagToCallee(env, remoteJsObj);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
    return remoteCallee_;
}

napi_value JsUIAbility::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult,
    bool showMethodNotFoundLog)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: the begin of %{public}s", name);
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Not found Ability.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get Ability object.");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        if (showMethodNotFoundLog) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get '%{public}s' from Ability object.", name);
        }
        return nullptr;
    }
    TryCatch tryCatch(env);
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, methodOnCreate, argc, argv, &result);
        if (tryCatch.HasCaught()) {
            reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
        }
        return handleEscape.Escape(result);
    }
    int64_t timeStart = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    int64_t timeEnd = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    if (tryCatch.HasCaught()) {
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Lifecycle: the end of %{public}s, time: %{public}s",
        name, std::to_string(timeEnd - timeStart).c_str());
    return nullptr;
}

bool JsUIAbility::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Result is null.");
        return false;
    }
    auto env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Result is not promise.");
        return false;
    }
    return true;
}

bool JsUIAbility::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, "promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::UIABILITY, "CallPromise complete");
    return true;
}

bool JsUIAbility::CallPromise(napi_value result, int32_t &onContinueRes)
{
    auto env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Error to convert native value to NativeObject.");
        return false;
    }
    napi_value then = nullptr;
    napi_get_named_property(env, result, "then", &then);
    if (then == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Error to get property: then.");
        return false;
    }
    bool isCallable = false;
    napi_is_callable(env, then, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "property then is not callable");
        return false;
    }

    std::weak_ptr<UIAbility> weakPtr = shared_from_this();
    auto asyncCallback = [abilityWeakPtr = weakPtr, this, &onContinueRes](int32_t &result) {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Ability is nullptr.");
            return;
        }
        onContinueRes = result;
    };
    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<int32_t>::Create();
    if (callbackInfo != nullptr) {
        callbackInfo->Push(asyncCallback);
    }

    HandleScope handleScope(jsRuntime_);
    napi_value promiseCallback = nullptr;
    napi_create_function(env, nullptr, NAPI_AUTO_LENGTH, OnContinuePromiseCallback,
        callbackInfo, &promiseCallback);
    napi_value argv[1] = { promiseCallback };
    napi_call_function(env, result, then, 1, argv, nullptr);
    TAG_LOGD(AAFwkTag::UIABILITY, "CallPromise complete");
    return true;
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> JsUIAbility::CreateADelegatorAbilityProperty()
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is nullptr.");
        return nullptr;
    }
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    property->token_ = abilityContext_->GetToken();
    property->name_ = GetAbilityName();
    property->moduleName_ = GetModuleName();
    if (GetApplicationInfo() == nullptr || GetApplicationInfo()->bundleName.empty()) {
        property->fullName_ = GetAbilityName();
    } else {
        std::string::size_type pos = GetAbilityName().find(GetApplicationInfo()->bundleName);
        if (pos == std::string::npos || pos != 0) {
            property->fullName_ = GetApplicationInfo()->bundleName + "." + GetAbilityName();
        } else {
            property->fullName_ = GetAbilityName();
        }
    }
    property->lifecycleState_ = GetState();
    property->object_ = jsAbilityObj_;
    return property;
}

void JsUIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    UIAbility::Dump(params, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };
    napi_value dumpInfo = CallObjectMethod("dump", argv, ArraySize(argv), true);
    napi_value onDumpInfo = CallObjectMethod("onDump", argv, ArraySize(argv), true);

    GetDumpInfo(env, dumpInfo, onDumpInfo, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "Dump info size: %{public}zu.", info.size());
}

void JsUIAbility::GetDumpInfo(
    napi_env env, napi_value dumpInfo, napi_value onDumpInfo, std::vector<std::string> &info)
{
    if (dumpInfo != nullptr) {
        uint32_t len = 0;
        napi_get_array_length(env, dumpInfo, &len);
        for (uint32_t i = 0; i < len; i++) {
            std::string dumpInfoStr;
            napi_value element = nullptr;
            napi_get_element(env, dumpInfo, i, &element);
            if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
                TAG_LOGE(AAFwkTag::UIABILITY, "Parse dumpInfoStr failed.");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }

    if (onDumpInfo != nullptr) {
        uint32_t len = 0;
        napi_get_array_length(env, onDumpInfo, &len);
        for (uint32_t i = 0; i < len; i++) {
            std::string dumpInfoStr;
            napi_value element = nullptr;
            napi_get_element(env, onDumpInfo, i, &element);
            if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
                TAG_LOGE(AAFwkTag::UIABILITY, "Parse dumpInfoStr from onDumpInfoNative failed");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }
}

std::shared_ptr<NativeReference> JsUIAbility::GetJsAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (jsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "JsAbility object is nullptr.");
    }
    return jsAbilityObj_;
}

sptr<IRemoteObject> JsUIAbility::SetNewRuleFlagToCallee(napi_env env, napi_value remoteJsObj)
{
    if (!CheckTypeForNapiValue(env, remoteJsObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CalleeObj is nullptr.");
        return nullptr;
    }
    napi_value setFlagMethod = nullptr;
    napi_get_named_property(env, remoteJsObj, "setNewRuleFlag", &setFlagMethod);
    bool isCallable = false;
    napi_is_callable(env, setFlagMethod, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::UIABILITY, "SetFlagMethod is %{public}s", setFlagMethod == nullptr ? "nullptr" : "not func");
        return nullptr;
    }
    auto flag = CreateJsValue(env, IsUseNewStartUpRule());
    napi_value argv[1] = { flag };
    napi_call_function(env, remoteJsObj, setFlagMethod, 1, argv, nullptr);

    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, remoteJsObj);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Obj is nullptr.");
        return nullptr;
    }
    return remoteObj;
}

void JsUIAbility::UpdateJsWindowStage(napi_value windowStage)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "shellContextRef_ is nullptr.");
        return;
    }
    napi_value contextObj = shellContextRef_->GetNapiValue();
    napi_env env = jsRuntime_.GetNapiEnv();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get context native object.");
        return;
    }
    if (windowStage == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set context windowStage is undefined.");
        napi_set_named_property(env, contextObj, "windowStage", CreateJsUndefined(env));
        return;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "Set context windowStage object.");
    napi_set_named_property(env, contextObj, "windowStage", windowStage);
}

bool JsUIAbility::CheckSatisfyTargetAPIVersion(int32_t version)
{
    auto applicationInfo = GetApplicationInfo();
    if (!applicationInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CheckTargetAPIVersion applicationInfo is nullptr.");
        return false;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "TargetAPIVersion: %{public}d.", applicationInfo->apiTargetVersion);
    return applicationInfo->apiTargetVersion % API_VERSION_MOD >= version;
}

bool JsUIAbility::BackPressDefaultValue()
{
    return CheckSatisfyTargetAPIVersion(API12) ? true : false;
}
} // namespace AbilityRuntime
} // namespace OHOS
