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
} // namespace

napi_value AttachJsAbilityContext(napi_env env, void *value, void *)
{
    HILOG_DEBUG("Begin.");
    if (value == nullptr) {
        HILOG_ERROR("Invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("Invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsAbilityContext(env, ptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_ERROR("Invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

UIAbility *JsUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) JsUIAbility(static_cast<JsRuntime &>(*runtime));
}

JsUIAbility::JsUIAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{
    HILOG_DEBUG("Called.");
}

JsUIAbility::~JsUIAbility()
{
    HILOG_DEBUG("Called.");
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
        HILOG_ERROR("AbilityLocalRecord is nullptr.");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityInfo is nullptr.");
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
            HILOG_ERROR("SrcEntrance is empty.");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        HILOG_DEBUG("JsAbility srcPath is %{public}s.", srcPath.c_str());
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
        HILOG_ERROR("jsAbilityObj_ or abilityContext_ or want is nullptr.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to check type");
        return;
    }
    napi_value contextObj = nullptr;
    int32_t screenMode = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    CreateAbilityContext(env, contextObj, screenMode);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("shellContextRef_ is nullptr.");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get ability native object.");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(abilityContext_);
    if (workContext == nullptr) {
        HILOG_ERROR("workContext is nullptr.");
        return;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachJsAbilityContext, workContext, nullptr);
    abilityContext_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    HILOG_DEBUG("Set ability context");
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->SetJsAbility(reinterpret_cast<uintptr_t>(workContext));
    }
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        }, nullptr, nullptr);
}

void JsUIAbility::CreateAbilityContext(napi_env env, napi_value &contextObj, int32_t screenMode)
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
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!jsAbilityObj_) {
        HILOG_ERROR("Not found Ability.js.");
        return;
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityCreate(jsAbilityObj_);
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Error to get Ability object");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        HILOG_ERROR("JsWant is nullptr.");
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
        HILOG_DEBUG("Call PostPerformStart.");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
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
    HILOG_DEBUG("Begin.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object.");
        return ERR_INVALID_VALUE;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Ability object.");
        return ERR_INVALID_VALUE;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParam);
    napi_value argv[] = {
        jsWantParams,
    };
    CallObjectMethod("onShare", argv, ArraySize(argv));
    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParam);
    HILOG_DEBUG("End.");
    return ERR_OK;
}

void JsUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin.");
    if (abilityContext_) {
        HILOG_DEBUG("Set terminating true.");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onDestroy");
    OnStopCallback();
    HILOG_DEBUG("End.");
}

void JsUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin");
    if (abilityContext_) {
        HILOG_DEBUG("Set terminating true.");
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
            HILOG_ERROR("Ability is nullptr.");
            return;
        }
        ability->OnStopCallback();
    };
    callbackInfo->Push(asyncCallback);
    isAsyncCallback = CallPromise(result, callbackInfo);
    if (!isAsyncCallback) {
        HILOG_ERROR("Failed to call promise.");
        OnStopCallback();
    }
    HILOG_DEBUG("End.");
}

void JsUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformStop.");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        HILOG_DEBUG("The service connection is not disconnected.");
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
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("JsAppWindowStage is nullptr.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
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
        HILOG_DEBUG("Call PostPerformScenceCreated.");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageCreate(jsAbilityObj_, jsWindowStageObj_);
    }

    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::OnSceneRestored()
{
    UIAbility::OnSceneRestored();
    HILOG_DEBUG("called.");
    HandleScope handleScope(jsRuntime_);
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("JsAppWindowStage is nullptr.");
        return;
    }
    napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformScenceRestored.");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
}

void JsUIAbility::onSceneDestroyed()
{
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();
    HandleScope handleScope(jsRuntime_);
    CallObjectMethod("onWindowStageDestroy");

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            HILOG_DEBUG("Call UnregisterDisplayMoveListener.");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformScenceDestroyed.");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageDestroy(jsAbilityObj_, jsWindowStageObj_);
    }
    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    UIAbility::OnForeground(want);
    CallOnForegroundFunc(want);
}

void JsUIAbility::CallOnForegroundFunc(const Want &want)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("JsAbilityObj_ is nullptr.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Ability object.");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        HILOG_ERROR("jsWant is nullptr.");
        return;
    }

    napi_set_named_property(env, obj, "lastRequestWant", jsWant);
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    CallObjectMethod("onForeground", &jsWant, 1);
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformForeground.");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityForeground(jsAbilityObj_);
    }
    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

void JsUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    std::string methodName = "OnBackground";
    HandleScope handleScope(jsRuntime_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    CallObjectMethod("onBackground");
    AddLifecycleEventAfterJSCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    UIAbility::OnBackground();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformBackground.");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityBackground(jsAbilityObj_);
    }
    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

bool JsUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onBackPressed", nullptr, 0, true);
    bool ret = false;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        HILOG_ERROR("Get js value failed.");
        return false;
    }
    HILOG_DEBUG("End ret is %{public}d.", ret);
    return ret;
}

bool JsUIAbility::OnPrepareTerminate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnPrepareTerminate();
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsValue = CallObjectMethod("onPrepareToTerminate", nullptr, 0, true);
    bool ret = false;
    if (!ConvertFromJsValue(env, jsValue, ret)) {
        HILOG_ERROR("Get js value failed.");
        return false;
    }
    HILOG_DEBUG("End ret is %{public}d.", ret);
    return ret;
}

std::unique_ptr<NativeReference> JsUIAbility::CreateAppWindowStage()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::CreateJsWindowStage(env, GetScene());
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object.");
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
            HILOG_ERROR("Content storage is nullptr.");
        }
    }
}

void JsUIAbility::AbilityContinuationOrRecover(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // multi-instance ability continuation
    HILOG_DEBUG("Launch reason is %{public}d.", launchParam_.launchReason);
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
            HILOG_ERROR("MainWindow is nullptr.");
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
            HILOG_ERROR("AbilityContext or sceneListener_ is nullptr.");
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
            HILOG_DEBUG("Set window mode is %{public}d.", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    HILOG_INFO("Move scene to foreground, sceneFlag_: %{public}d.", UIAbility::sceneFlag_);
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    HILOG_DEBUG("End.");
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
            HILOG_DEBUG("Success displayId is %{public}d.", displayId);
        } else {
            HILOG_ERROR("Failed to formatRegex: [%{public}s].", strDisplayId.c_str());
        }
    }
    auto option = GetWindowOption(want);
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionInfo_ != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionInfo_->sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionInfo_->sessionToken);
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        HILOG_ERROR("Failed to init window scene.");
        return;
    }

    AbilityContinuationOrRecover(want);
    auto window = scene_->GetMainWindow();
    if (window) {
        HILOG_DEBUG("Call RegisterDisplayMoveListener, windowId: %{public}d.", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            HILOG_ERROR("abilityDisplayMoveListener_ is nullptr.");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
}

void JsUIAbility::RequestFocus(const Want &want)
{
    HILOG_INFO("Lifecycle: begin.");
    if (scene_ == nullptr) {
        HILOG_ERROR("scene_ is nullptr.");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(
            Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        HILOG_DEBUG("Set window mode is %{public}d.", windowMode);
    }
    AddLifecycleEventBeforeJSCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    HILOG_INFO("Lifecycle: end.");
}

void JsUIAbility::ContinuationRestore(const Want &want)
{
    HILOG_DEBUG("Called.");
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        HILOG_ERROR("Is not in continuation or scene_ is nullptr.");
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

std::shared_ptr<NativeReference> JsUIAbility::GetJsWindowStage()
{
    HILOG_DEBUG("Called.");
    if (jsWindowStageObj_ == nullptr) {
        HILOG_ERROR("JsWindowSatge is nullptr.");
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
    HILOG_DEBUG("called.");
    if (executeParam == nullptr) {
        HILOG_WARN("Intent execute param invalid.");
        RequestFocus(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        HILOG_DEBUG("Begin request focus.");
        auto ability = weak.lock();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr.");
            return;
        }
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        HILOG_ERROR("Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        HILOG_ERROR("Execute insight intent failed.");
    }
}

void JsUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");
    if (executeParam == nullptr) {
        HILOG_WARN("Intent execute param invalid.");
        OnForeground(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }
    UIAbility::OnForeground(want);

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        HILOG_DEBUG("Begin call onForeground.");
        auto ability = weak.lock();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr.");
            return;
        }
        ability->CallOnForegroundFunc(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        HILOG_ERROR("Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        HILOG_ERROR("Execute insight intent failed.");
    }
}

void JsUIAbility::ExecuteInsightIntentBackground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");
    if (executeParam == nullptr) {
        HILOG_WARN("Intent execute param invalid.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    if (abilityInfo_) {
        jsRuntime_.UpdateModuleNameAndAssetPath(abilityInfo_->moduleName);
    }

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        HILOG_ERROR("Get Intent executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }

    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executeInfo, std::move(callback));
    if (!ret) {
        // callback has removed, release in insight intent executor.
        HILOG_ERROR("Execute insight intent failed.");
    }
}

bool JsUIAbility::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo& executeInfo)
{
    HILOG_DEBUG("called.");

    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        HILOG_ERROR("Param invalid.");
        return false;
    }

    if (executeParam->executeMode_ == AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND
        && jsWindowStageObj_ == nullptr) {
        HILOG_ERROR("Param invalid.");
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
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Ability object.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onContinue", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onContinue' from Ability object.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }

    napi_value jsWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    napi_value result = nullptr;
    napi_call_function(env, obj, methodOnCreate, 1, &jsWantParams, &result);

    OHOS::AppExecFwk::UnwrapWantParams(env, jsWantParams, wantParams);

    int32_t numberResult = 0;
    if (!ConvertFromJsValue(env, result, numberResult)) {
        HILOG_ERROR("'onContinue' is not implemented.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityContinue(jsAbilityObj_);
    }

    return numberResult;
}

int32_t JsUIAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("AppRecoveryFailed to get AbilityStage object.");
        return -1;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("AppRecovery Failed to get Ability object.");
        return -1;
    }

    napi_value methodOnSaveState = nullptr;
    napi_get_named_property(env, obj, "onSaveState", &methodOnSaveState);
    if (methodOnSaveState == nullptr) {
        HILOG_ERROR("AppRecovery Failed to get 'onSaveState' from Ability object.");
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
        HILOG_ERROR("AppRecovery no result return from onSaveState.");
        return -1;
    }
    return numberResult;
}

void JsUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    UIAbility::OnConfigurationUpdated(configuration);
    HILOG_DEBUG("Called.");
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is nullptr.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    auto fullConfig = abilityContext_->GetConfiguration();
    if (fullConfig == nullptr) {
        HILOG_ERROR("Configuration is nullptr.");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, configuration);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, 1);
    JsAbilityContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    HILOG_DEBUG("Called.");

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Ability object.");
        return;
    }

    napi_value jslevel = CreateJsValue(env, level);
    napi_value argv[] = { jslevel };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

void JsUIAbility::UpdateContextConfiguration()
{
    HILOG_DEBUG("Called.");
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is nullptr.");
        return;
    }
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    JsAbilityContext::ConfigurationUpdated(env, shellContextRef_, abilityContext_->GetConfiguration());
}

void JsUIAbility::OnNewWant(const Want &want)
{
    HILOG_DEBUG("Begin.");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object.");
        return;
    }
    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Ability object.");
        return;
    }

    napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, want);
    if (jsWant == nullptr) {
        HILOG_ERROR("Failed to get want.");
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
    HILOG_DEBUG("End.");
}

void JsUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("Begin.");
    UIAbility::OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is nullptr.");
        return;
    }
    abilityContext_->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_DEBUG("End.");
}

sptr<IRemoteObject> JsUIAbility::CallRequest()
{
    HILOG_DEBUG("Begin.");
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Obj is nullptr.");
        return nullptr;
    }

    if (remoteCallee_ != nullptr) {
        HILOG_ERROR("RemoteCallee is nullptr.");
        return remoteCallee_;
    }

    HandleScope handleScope(jsRuntime_);
    HILOG_DEBUG("Set runtime scope.");
    auto env = jsRuntime_.GetNapiEnv();
    auto obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Value is nullptr.");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onCallRequest", &method);
    bool isCallable = false;
    napi_is_callable(env, method, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("Method is %{public}s.", method == nullptr ? "nullptr" : "not func");
        return nullptr;
    }

    napi_value remoteJsObj = nullptr;
    napi_call_function(env, obj, method, 0, nullptr, &remoteJsObj);
    if (remoteJsObj == nullptr) {
        HILOG_ERROR("JsObj is nullptr.");
        return nullptr;
    }

    remoteCallee_ = SetNewRuleFlagToCallee(env, remoteJsObj);
    HILOG_DEBUG("End.");
    return remoteCallee_;
}

napi_value JsUIAbility::CallObjectMethod(const char *name, napi_value const *argv, size_t argc, bool withResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Lifecycle: the begin of %{public}s", name);
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Not found Ability.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Ability object.");
        return nullptr;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object.", name);
        return nullptr;
    }
    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, methodOnCreate, argc, argv, &result);
        return handleEscape.Escape(result);
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
    HILOG_INFO("Lifecycle: the end of %{public}s", name);
    return nullptr;
}

bool JsUIAbility::CheckPromise(napi_value result)
{
    if (result == nullptr) {
        HILOG_DEBUG("Result is null.");
        return false;
    }
    auto env = jsRuntime_.GetNapiEnv();
    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        HILOG_DEBUG("Result is not promise.");
        return false;
    }
    return true;
}

bool JsUIAbility::CallPromise(napi_value result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
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
    HILOG_DEBUG("CallPromise complete");
    return true;
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> JsUIAbility::CreateADelegatorAbilityProperty()
{
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is nullptr.");
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
    HILOG_DEBUG("Called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found .js");
        return;
    }

    napi_value obj = jsAbilityObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get object.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "dump", &method);
    napi_value onDumpMethod = nullptr;
    napi_get_named_property(env, obj, "onDump", &onDumpMethod);

    napi_value dumpInfo = nullptr;
    if (method != nullptr) {
        napi_call_function(env, obj, method, 1, argv, &dumpInfo);
    }

    napi_value onDumpInfo = nullptr;
    if (onDumpMethod != nullptr) {
        napi_call_function(env, obj, onDumpMethod, 1, argv, &onDumpInfo);
    }

    GetDumpInfo(env, dumpInfo, onDumpInfo, info);
    HILOG_DEBUG("Dump info size: %{public}zu.", info.size());
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
                HILOG_ERROR("Parse dumpInfoStr failed.");
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
                HILOG_ERROR("Parse dumpInfoStr from onDumpInfoNative failed");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }
}

std::shared_ptr<NativeReference> JsUIAbility::GetJsAbility()
{
    HILOG_DEBUG("Called.");
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("JsAbility object is nullptr.");
    }
    return jsAbilityObj_;
}

sptr<IRemoteObject> JsUIAbility::SetNewRuleFlagToCallee(napi_env env, napi_value remoteJsObj)
{
    if (!CheckTypeForNapiValue(env, remoteJsObj, napi_object)) {
        HILOG_ERROR("CalleeObj is nullptr.");
        return nullptr;
    }
    napi_value setFlagMethod = nullptr;
    napi_get_named_property(env, remoteJsObj, "setNewRuleFlag", &setFlagMethod);
    bool isCallable = false;
    napi_is_callable(env, setFlagMethod, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("SetFlagMethod is %{public}s", setFlagMethod == nullptr ? "nullptr" : "not func");
        return nullptr;
    }
    auto flag = CreateJsValue(env, IsUseNewStartUpRule());
    napi_value argv[1] = { flag };
    napi_call_function(env, remoteJsObj, setFlagMethod, 1, argv, nullptr);

    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, remoteJsObj);
    if (remoteObj == nullptr) {
        HILOG_ERROR("Obj is nullptr.");
        return nullptr;
    }
    return remoteObj;
}
} // namespace AbilityRuntime
} // namespace OHOS
