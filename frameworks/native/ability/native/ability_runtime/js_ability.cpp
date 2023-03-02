/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <regex>

#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "ability_delegator_registry.h"
#include "ability_runtime/js_ability.h"

#include "ability_runtime/js_ability_context.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "app_recovery.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_configuration.h"
#ifdef SUPPORT_GRAPHICS
#include "js_window_stage.h"
#endif
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "string_wrapper.h"
#include "context/context.h"
#include "context/application_context.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
NativeValue *PromiseCallback(NativeEngine *engine, NativeCallbackInfo *info)
{
    if (info == nullptr || info->functionInfo == nullptr || info->functionInfo->data == nullptr) {
        HILOG_ERROR("Invalid input info.");
        return nullptr;
    }
    void *data = info->functionInfo->data;
    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(data);
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    info->functionInfo->data = nullptr;
    return nullptr;
}
}

NativeValue *AttachJsAbilityContext(NativeEngine *engine, void *value, void *)
{
    HILOG_DEBUG("AttachJsAbilityContext");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::AbilityContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue *object = CreateJsAbilityContext(*engine, ptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.AbilityContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachJsAbilityContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void * data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        }, nullptr);
    return contextObj;
}

Ability *JsAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new JsAbility(static_cast<JsRuntime &>(*runtime));
}

JsAbility::JsAbility(JsRuntime &jsRuntime) : jsRuntime_(jsRuntime)
{}
JsAbility::~JsAbility() = default;

void JsAbility::Init(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<OHOSApplication> application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    Ability::Init(abilityInfo, application, handler, token);

    if (!abilityInfo) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->type == AppExecFwk::AbilityType::PAGE && abilityInfo->isStageBasedModel
        && abilityContext_ != nullptr) {
            AppExecFwk::AppRecovery::GetInstance().AddAbility(shared_from_this(), abilityContext_->GetAbilityInfo(),
            abilityContext_->GetToken());
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
            HILOG_ERROR("abilityInfo srcEntrance is empty");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        HILOG_INFO("JsAbility srcPath is %{public}s", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);

    HandleScope handleScope(jsRuntime_);
    auto &engine = jsRuntime_.GetNativeEngine();

    jsAbilityObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo->hapPath, abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE);
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(jsAbilityObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to convert AbilityStage object");
        return;
    }

    auto context = GetAbilityContext();
    NativeValue *contextObj = CreateJsAbilityContext(engine, context);
    shellContextRef_ = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(&engine, "application.AbilityContext", &contextObj, 1).release());
    contextObj = shellContextRef_->Get();
    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get ability native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::AbilityContext>(context);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachJsAbilityContext,
        workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);
    HILOG_DEBUG("Set ability context");

    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->SetJsAbility(reinterpret_cast<uintptr_t>(workContext));
    }

    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::AbilityContext> *>(data);
        },
        nullptr);
}

void JsAbility::OnStart(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStart begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::OnStart(want);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityCreate(jsAbilityObj_);
    }

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue *jsWant = reinterpret_cast<NativeValue *>(napiWant);

    obj->SetProperty("launchWant", jsWant);
    obj->SetProperty("lastRequestWant", jsWant);

    NativeValue *argv[] = {
        jsWant,
        CreateJsLaunchParam(nativeEngine, GetLaunchParam()),
    };
    CallObjectMethod("onCreate", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformStart");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    HILOG_DEBUG("OnStart end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStop begin.");
    if (abilityContext_) {
        HILOG_DEBUG("OnStop, set terminating true.");
        abilityContext_->SetTerminating(true);
    }
    Ability::OnStop();
    CallObjectMethod("onDestroy");
    OnStopCallback();
    HILOG_DEBUG("OnStop end.");
}

void JsAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStop begin.");
    if (abilityContext_) {
        HILOG_DEBUG("OnStop, set terminating true.");
        abilityContext_->SetTerminating(true);
    }

    Ability::OnStop();

    HandleScope handleScope(jsRuntime_);
    NativeValue *result = CallObjectMethod("onDestroy", nullptr, 0, true);
    if (!CheckPromise(result)) {
        OnStopCallback();
        isAsyncCallback = false;
        return;
    }

    std::weak_ptr<Ability> weakPtr = shared_from_this();
    auto asyncCallback = [abilityWeakPtr = weakPtr]() {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr.");
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
    HILOG_DEBUG("OnStop end.");
}

void JsAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformStop");
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
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";

void JsAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnSceneCreated begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::OnSceneCreated();
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue *argv[] = {jsAppWindowStage->Get()};
    CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformScenceCreated");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageCreate(jsAbilityObj_, jsWindowStageObj_);
    }

    HILOG_DEBUG("OnSceneCreated end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnSceneRestored()
{
    Ability::OnSceneRestored();
    HILOG_DEBUG("OnSceneRestored");
    auto jsAppWindowStage = CreateAppWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue *argv[] = {jsAppWindowStage->Get()};
    CallObjectMethod("onWindowStageRestore", argv, ArraySize(argv));

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformScenceRestored");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }

    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
}

void JsAbility::onSceneDestroyed()
{
    HILOG_DEBUG("onSceneDestroyed begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::onSceneDestroyed();

    CallObjectMethod("onWindowStageDestroy");

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            HILOG_DEBUG("Call UnregisterDisplayMoveListener");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformScenceDestroyed");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnWindowStageDestroy(jsAbilityObj_, jsWindowStageObj_);
    }
    HILOG_DEBUG("onSceneDestroyed end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnForeground begin, ability is %{public}s.", GetAbilityName().c_str());
    Ability::OnForeground(want);

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }
    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue *jsWant = reinterpret_cast<NativeValue *>(napiWant);

    obj->SetProperty("lastRequestWant", jsWant);

    CallObjectMethod("onForeground", &jsWant, 1);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformForeground");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityForeground(jsAbilityObj_);
    }
    HILOG_DEBUG("OnForeground end, ability is %{public}s.", GetAbilityName().c_str());
}

void JsAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnBackground begin, ability is %{public}s.", GetAbilityName().c_str());
    CallObjectMethod("onBackground");

    Ability::OnBackground();

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformBackground");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityBackground(jsAbilityObj_);
    }
    HILOG_DEBUG("OnBackground end, ability is %{public}s.", GetAbilityName().c_str());
}

std::unique_ptr<NativeReference> JsAbility::CreateAppWindowStage()
{
    HandleScope handleScope(jsRuntime_);
    auto &engine = jsRuntime_.GetNativeEngine();
    NativeValue *jsWindowStage = Rosen::CreateJsWindowStage(engine, GetScene());
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(&engine, "application.WindowStage", &jsWindowStage, 1);
}

void JsAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool JsAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void JsAbility::RestorePageStack(const Want &want)
{
    if (IsRestorePageStack(want)) {
        std::string pageStack;
        GetPageStackFromWant(want, pageStack);
        HandleScope handleScope(jsRuntime_);
        auto &engine = jsRuntime_.GetNativeEngine();
        if (abilityContext_->GetContentStorage()) {
            scene_->GetMainWindow()->SetUIContent(pageStack, &engine,
                abilityContext_->GetContentStorage()->Get(), true);
        } else {
            HILOG_ERROR("restore: content storage is nullptr");
        }
    }
}

void JsAbility::AbilityContinuationOrRecover(const Want &want)
{
    // multi-instance ability continuation
    HILOG_DEBUG("launch reason = %{public}d", launchParam_.launchReason);
    if (IsRestoredInContinuation()) {
        RestorePageStack(want);
        OnSceneRestored();
        NotifyContinuationResult(want, true);
    } else if (ShouldRecoverState(want)) {
        std::string pageStack = abilityRecovery_->GetSavedPageStack(AppExecFwk::StateReason::DEVELOPER_REQUEST);
        HandleScope handleScope(jsRuntime_);
        auto &engine = jsRuntime_.GetNativeEngine();
        auto mainWindow = scene_->GetMainWindow();
        if (mainWindow != nullptr) {
            mainWindow->SetUIContent(pageStack, &engine, abilityContext_->GetContentStorage()->Get(), true);
        } else {
            HILOG_ERROR("AppRecovery:AbilityContinuationOrRecover mainWindow nullptr");
        }
        OnSceneRestored();
    } else {
        OnSceneCreated();
    }
}

void JsAbility::DoOnForeground(const Want &want)
{
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            HILOG_ERROR("Ability::OnForeground error. abilityContext_ or sceneListener_ is nullptr!");
            return;
        }
        scene_ = std::make_shared<Rosen::WindowScene>();
        int32_t displayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
        if (setting_ != nullptr) {
            std::string strDisplayId =
                setting_->GetProperty(OHOS::AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
            std::regex formatRegex("[0-9]{0,9}$");
            std::smatch sm;
            bool flag = std::regex_match(strDisplayId, sm, formatRegex);
            if (flag && !strDisplayId.empty()) {
                int base = 10; // Numerical base (radix) that determines the valid characters and their interpretation.
                displayId = strtol(strDisplayId.c_str(), nullptr, base);
                HILOG_DEBUG("%{public}s success. displayId is %{public}d", __func__, displayId);
            } else {
                HILOG_WARN("%{public}s failed to formatRegex:[%{public}s]", __func__, strDisplayId.c_str());
            }
        }
        auto option = GetWindowOption(want);
        Rosen::WMError ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
        if (ret != Rosen::WMError::WM_OK) {
            HILOG_ERROR("%{public}s error. failed to init window scene!", __func__);
            return;
        }

        AbilityContinuationOrRecover(want);
        auto window = scene_->GetMainWindow();
        if (window) {
            HILOG_DEBUG("Call RegisterDisplayMoveListener, windowId: %{public}d", window->GetWindowId());
            abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
            window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    } else {
        auto window = scene_->GetMainWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
                AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            HILOG_DEBUG("set window mode = %{public}d.", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window != nullptr && securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    HILOG_DEBUG("%{public}s begin scene_->GoForeground, sceneFlag_:%{public}d.", __func__, Ability::sceneFlag_);
    scene_->GoForeground(Ability::sceneFlag_);
    HILOG_DEBUG("%{public}s end scene_->GoForeground.", __func__);
}

void JsAbility::RequestFocus(const Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (scene_ == nullptr) {
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        HILOG_DEBUG("set window mode = %{public}d.", windowMode);
    }
    scene_->GoForeground(Ability::sceneFlag_);
}

void JsAbility::ContinuationRestore(const Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

std::shared_ptr<NativeReference> JsAbility::GetJsWindowStage()
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (jsWindowStageObj_ == nullptr) {
        HILOG_ERROR("jsWindowSatge is nullptr");
    }
    return jsWindowStageObj_;
}

const JsRuntime& JsAbility::GetJsRuntime()
{
    return jsRuntime_;
}

#endif

int32_t JsAbility::OnContinue(WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return AppExecFwk::ContinuationManager::OnContinueResult::REJECT;
    }
    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return AppExecFwk::ContinuationManager::OnContinueResult::REJECT;
    }

    NativeValue *methodOnCreate = obj->GetProperty("onContinue");
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onContinue' from Ability object");
        return AppExecFwk::ContinuationManager::OnContinueResult::REJECT;
    }

    napi_value napiWantParams = OHOS::AppExecFwk::WrapWantParams(reinterpret_cast<napi_env>(&nativeEngine), wantParams);
    NativeValue *jsWantParams = reinterpret_cast<NativeValue *>(napiWantParams);

    NativeValue *result = nativeEngine.CallFunction(value, methodOnCreate, &jsWantParams, 1);

    napi_value new_napiWantParams = reinterpret_cast<napi_value>(jsWantParams);
    OHOS::AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(&nativeEngine), new_napiWantParams, wantParams);

    NativeNumber *numberResult = ConvertNativeValueTo<NativeNumber>(result);
    if (numberResult == nullptr) {
        HILOG_ERROR("'onContinue' is not implemented");
        return AppExecFwk::ContinuationManager::OnContinueResult::REJECT;
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext != nullptr) {
        applicationContext->DispatchOnAbilityContinue(jsAbilityObj_);
    }

    return *numberResult;
}

int32_t JsAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("AppRecoveryFailed to get AbilityStage object");
        return -1;
    }
    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("AppRecovery Failed to get Ability object");
        return -1;
    }

    NativeValue *methodOnSaveState = obj->GetProperty("onSaveState");
    if (methodOnSaveState == nullptr) {
        HILOG_ERROR("AppRecovery Failed to get 'onSaveState' from Ability object");
        return -1;
    }

    napi_value napiWantParams = OHOS::AppExecFwk::WrapWantParams(reinterpret_cast<napi_env>(&nativeEngine), wantParams);
    NativeValue* jsReason = CreateJsValue(nativeEngine, reason);
    NativeValue* jsWantParams = reinterpret_cast<NativeValue *>(napiWantParams);
    NativeValue* args[] = { jsReason, jsWantParams };
    NativeValue* result = nativeEngine.CallFunction(value, methodOnSaveState, args, 2); // 2:args size
    napi_value newNapiWantParams = reinterpret_cast<napi_value>(jsWantParams);
    OHOS::AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(&nativeEngine), newNapiWantParams, wantParams);

    NativeNumber *numberResult = ConvertNativeValueTo<NativeNumber>(result);
    if (numberResult == nullptr) {
        HILOG_ERROR("AppRecovery no result return from onSaveState");
        return -1;
    }
    return *numberResult;
}

void JsAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    Ability::OnConfigurationUpdated(configuration);
    HILOG_DEBUG("%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    auto fullConfig = GetAbilityContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), configuration);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, 1);
    CallObjectMethod("onConfigurationUpdate", &jsConfiguration, 1);
    JsAbilityContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);
}

void JsAbility::OnMemoryLevel(int level)
{
    Ability::OnMemoryLevel(level);
    HILOG_DEBUG("%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }
    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    NativeValue *jslevel = CreateJsValue(nativeEngine, level);
    NativeValue *argv[] = {
        jslevel,
    };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

void JsAbility::UpdateContextConfiguration()
{
    HILOG_DEBUG("%{public}s called.", __func__);
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    JsAbilityContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, GetAbilityContext()->GetConfiguration());
}

void JsAbility::OnNewWant(const Want &want)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    Ability::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }
    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&nativeEngine), want);
    NativeValue *jsWant = reinterpret_cast<NativeValue *>(napiWant);

    obj->SetProperty("lastRequestWant", jsWant);

    NativeValue *argv[] = {
        jsWant,
        CreateJsLaunchParam(nativeEngine, GetLaunchParam()),
    };
    CallObjectMethod("onNewWant", argv, ArraySize(argv));

    HILOG_DEBUG("%{public}s end.", __func__);
}

void JsAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    Ability::OnAbilityResult(requestCode, resultCode, resultData);
    std::shared_ptr<AbilityRuntime::AbilityContext> context = GetAbilityContext();
    if (context == nullptr) {
        HILOG_WARN("JsAbility not attached to any runtime context!");
        return;
    }
    context->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_DEBUG("%{public}s end.", __func__);
}

sptr<IRemoteObject> JsAbility::CallRequest()
{
    HILOG_DEBUG("JsAbility::CallRequest begin.");
    if (jsAbilityObj_ == nullptr) {
        HILOG_WARN("JsAbility::CallRequest Obj is nullptr");
        return nullptr;
    }

    if (remoteCallee_ != nullptr) {
        HILOG_DEBUG("JsAbility::CallRequest get Callee remoteObj.");
        return remoteCallee_;
    }

    HandleScope handleScope(jsRuntime_);
    HILOG_DEBUG("JsAbility::CallRequest set runtime scope.");
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    auto value = jsAbilityObj_->Get();
    if (value == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest value is nullptr");
        return nullptr;
    }

    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest obj is nullptr");
        return nullptr;
    }

    auto method = obj->GetProperty("onCallRequest");
    if (method == nullptr || !method->IsCallable()) {
        HILOG_ERROR("JsAbility::CallRequest method is %{public}s", method == nullptr ? "nullptr" : "not func");
        return nullptr;
    }

    auto remoteJsObj = nativeEngine.CallFunction(value, method, nullptr, 0);
    if (remoteJsObj == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest JsObj is nullptr");
        return nullptr;
    }

    remoteCallee_ = SetNewRuleFlagToCallee(nativeEngine, remoteJsObj);
    HILOG_DEBUG("JsAbility::CallRequest end.");
    return remoteCallee_;
}

NativeValue *JsAbility::CallObjectMethod(const char *name, NativeValue *const *argv, size_t argc, bool withResult)
{
    HILOG_DEBUG("JsAbility::CallObjectMethod(%{public}s", name);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return nullptr;
    }

    HandleEscape handleEscape(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsAbilityObj_->Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return nullptr;
    }

    NativeValue *methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return nullptr;
    }
    if (withResult) {
        return handleEscape.Escape(nativeEngine.CallFunction(value, methodOnCreate, argv, argc));
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
    return nullptr;
}

bool JsAbility::CheckPromise(NativeValue *result)
{
    if (result == nullptr) {
        HILOG_DEBUG("result is null, no need to call promise.");
        return false;
    }
    if (!result->IsPromise()) {
        HILOG_DEBUG("result is not promise, no need to call promise.");
        return false;
    }
    return true;
}

bool JsAbility::CallPromise(NativeValue *result, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo)
{
    auto *retObj = ConvertNativeValueTo<NativeObject>(result);
    if (retObj == nullptr) {
        HILOG_ERROR("Failed to convert native value to NativeObject.");
        return false;
    }
    NativeValue *then = retObj->GetProperty("then");
    if (then == nullptr) {
        HILOG_ERROR("Failed to get property: then.");
        return false;
    }
    if (!then->IsCallable()) {
        HILOG_ERROR("property then is not callable.");
        return false;
    }
    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();
    auto promiseCallback = nativeEngine.CreateFunction("promiseCallback", strlen("promiseCallback"), PromiseCallback,
        callbackInfo);
    NativeValue *argv[1] = { promiseCallback };
    nativeEngine.CallFunction(result, then, argv, 1);
    return true;
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> JsAbility::CreateADelegatorAbilityProperty()
{
    auto property = std::make_shared<AppExecFwk::ADelegatorAbilityProperty>();
    property->token_          = GetAbilityContext()->GetToken();
    property->name_           = GetAbilityName();
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
    property->object_         = jsAbilityObj_;

    return property;
}

void JsAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Ability::Dump(params, info);
    HILOG_DEBUG("%{public}s called.", __func__);
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    // create js array object of params
    NativeValue* argv[] = { CreateNativeArray(nativeEngine, params) };

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found .js");
        return;
    }

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty("dump");
    NativeValue* onDumpMethod = obj->GetProperty("onDump");

    NativeValue* dumpInfo = nullptr;
    if (method != nullptr) {
        dumpInfo = nativeEngine.CallFunction(value, method, argv, 1);
    }

    NativeValue* onDumpInfo = nullptr;
    if (onDumpMethod != nullptr) {
        onDumpInfo = nativeEngine.CallFunction(value, onDumpMethod, argv, 1);
    }

    NativeArray* dumpInfoNative = nullptr;
    if (dumpInfo != nullptr) {
        dumpInfoNative = ConvertNativeValueTo<NativeArray>(dumpInfo);
    }

    NativeArray* onDumpInfoNative = nullptr;
    if (onDumpInfo != nullptr) {
        onDumpInfoNative = ConvertNativeValueTo<NativeArray>(onDumpInfo);
    }

    if (dumpInfoNative != nullptr) {
        for (uint32_t i = 0; i < dumpInfoNative->GetLength(); i++) {
            std::string dumpInfoStr;
            if (!ConvertFromJsValue(nativeEngine, dumpInfoNative->GetElement(i), dumpInfoStr)) {
                HILOG_ERROR("Parse dumpInfoStr failed");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }

    if (onDumpInfoNative != nullptr) {
        for (uint32_t i = 0; i < onDumpInfoNative->GetLength(); i++) {
            std::string dumpInfoStr;
            if (!ConvertFromJsValue(nativeEngine, onDumpInfoNative->GetElement(i), dumpInfoStr)) {
                HILOG_ERROR("Parse dumpInfoStr from onDumpInfoNative failed");
                return;
            }
            info.push_back(dumpInfoStr);
        }
    }

    HILOG_DEBUG("Dump info size: %{public}zu", info.size());
}

std::shared_ptr<NativeReference> JsAbility::GetJsAbility()
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("jsAbility object is nullptr");
    }
    return jsAbilityObj_;
}

sptr<IRemoteObject> JsAbility::SetNewRuleFlagToCallee(NativeEngine &nativeEngine, NativeValue* remoteJsObj)
{
    NativeObject* calleeObj = ConvertNativeValueTo<NativeObject>(remoteJsObj);
    if (calleeObj == nullptr) {
        HILOG_ERROR("JsAbility::SetNewRuleFlagToCallee calleeObj is nullptr");
        return nullptr;
    }
    auto setFlagMethod = calleeObj->GetProperty("setNewRuleFlag");
    if (setFlagMethod == nullptr || !setFlagMethod->IsCallable()) {
        HILOG_ERROR("JsAbility::SetNewRuleFlagToCallee setFlagMethod is %{public}s",
            setFlagMethod == nullptr ? "nullptr" : "not func");
        return nullptr;
    }
    auto flag = nativeEngine.CreateBoolean(IsUseNewStartUpRule());
    NativeValue *argv[1] = { flag };
    nativeEngine.CallFunction(remoteJsObj, setFlagMethod, argv, 1);

    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(
        reinterpret_cast<napi_env>(&nativeEngine), reinterpret_cast<napi_value>(remoteJsObj));
    if (remoteObj == nullptr) {
        HILOG_ERROR("JsAbility::CallRequest obj is nullptr");
        return nullptr;
    }
    return remoteObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
