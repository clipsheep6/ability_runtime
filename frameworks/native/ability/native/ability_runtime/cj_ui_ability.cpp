/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cj_ui_ability.h"

#include <dlfcn.h>
#include <regex>
#include <cstdlib>

#include "ability_business_error.h"
#include "ability_delegator_registry.h"
#include "ability_recovery.h"
#include "ability_start_setting.h"
#include "app_recovery.h"
#include "context/application_context.h"
#include "connection_manager.h"
#include "context/context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "insight_intent_execute_param.h"
#include "cj_runtime.h"
#include "cj_ability_object.h"
#include "cj_ability_context.h"
#include "time_util.h"
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#endif
#include "string_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifdef SUPPORT_GRAPHICS
const std::string PAGE_STACK_PROPERTY_NAME = "pageStack";
const std::string METHOD_NAME = "WindowScene::GoForeground";
const std::string SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME = "ohos.extra.param.key.supportContinuePageStack";
#endif
#ifdef SUPPORT_SCREEN
// Numerical base (radix) that determines the valid characters and their interpretation.
const int32_t BASE_DISPLAY_ID_NUM (10);
#endif
const char* CJWINDOW_FFI_LIBNAME = "libcj_window_ffi.z.so";
const char* FUNC_CREATE_CJWINDOWSTAGE = "OHOS_CreateCJWindowStage";
using CFFICreateCJWindowStage = int64_t (*)(std::shared_ptr<Rosen::WindowScene>&);

sptr<Rosen::CJWindowStageImpl> CreateCJWindowStage(std::shared_ptr<Rosen::WindowScene> windowScene)
{
    static void* handle = nullptr;
    if (handle == nullptr) {
        handle = dlopen(CJWINDOW_FFI_LIBNAME, RTLD_LAZY);
        if (handle == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "dlopen failed %{public}s, %{public}s", CJWINDOW_FFI_LIBNAME, dlerror());
            return nullptr;
        }
    }
    // get function
    auto func = reinterpret_cast<CFFICreateCJWindowStage>(dlsym(handle, FUNC_CREATE_CJWINDOWSTAGE));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "dlsym failed %{public}s, %{public}s", FUNC_CREATE_CJWINDOWSTAGE, dlerror());
        dlclose(handle);
        handle = nullptr;
        return nullptr;
    }
    auto id = func(windowScene);
    return OHOS::FFI::FFIData::GetData<Rosen::CJWindowStageImpl>(id);
}
}

UIAbility *CJUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) CJUIAbility(static_cast<CJRuntime &>(*runtime));
}

CJUIAbility::CJUIAbility(CJRuntime &cjRuntime) : cjRuntime_(cjRuntime)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

CJUIAbility::~CJUIAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (abilityContext_ != nullptr) {
        abilityContext_->Unbind();
    }
}

void CJUIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
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
    SetAbilityContext(abilityInfo);
}

void CJUIAbility::SetAbilityContext(
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (!abilityInfo) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo is nullptr");
        return;
    }
    cjAbilityObj_ = CJAbilityObject::LoadModule(abilityInfo->name);
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to get CJAbility object.");
        return;
    }
    cjAbilityObj_->Init(this);
}

void CJUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnStart";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnStart(want, GetLaunchParam());
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformStart.");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    FreezeUtil::LifecycleFlow flow = { AbilityContext::token_, state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; CJUIAbility::" + methodName +
        "; the " + methodName + " begin.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}

void CJUIAbility::AddLifecycleEventAfterCall(FreezeUtil::TimeoutState state, const std::string &methodName) const
{
    FreezeUtil::LifecycleFlow flow = { AbilityContext::token_, state };
    auto entry = std::to_string(TimeUtil::SystemTimeMillisecond()) + "; CJUIAbility::" + methodName +
        "; the " + methodName + " end.";
    FreezeUtil::GetInstance().AddLifecycleEvent(flow, entry);
}

int32_t CJUIAbility::OnShare(WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    return ERR_OK;
}

void CJUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityContext_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set terminating true.");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    cjAbilityObj_->OnStop();
    CJUIAbility::OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void CJUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
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
    cjAbilityObj_->OnStop();
    OnStopCallback();
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void CJUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformStop.");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "The service connection is disconnected.");
    }
    ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
    TAG_LOGD(AAFwkTag::UIABILITY, "The service connection is not disconnected.");
}

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
void CJUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    cjWindowStage_ = CreateCJWindowStage(GetScene());
    if (!cjWindowStage_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to create CJWindowStage object.");
        return;
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageCreate");
        std::string methodName = "OnSceneCreated";
        AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        cjAbilityObj_->OnSceneCreated(cjWindowStage_.GetRefPtr());
        AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformScenceCreated.");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::OnSceneRestored()
{
    UIAbility::OnSceneRestored();
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }

    if (!cjWindowStage_) {
        cjWindowStage_ = CreateCJWindowStage(scene_);
        if (!cjWindowStage_) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Failed to create CJWindowStage object.");
            return;
        }
    }
    cjAbilityObj_->OnSceneRestored(cjWindowStage_.GetRefPtr());

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformScenceRestored.");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }
}

void CJUIAbility::OnSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    cjAbilityObj_->OnSceneDestroyed();

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            TAG_LOGD(AAFwkTag::UIABILITY, "Call window UnregisterDisplayMoveListener.");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call delegator PostPerformScenceDestroyed.");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());

    UIAbility::OnForeground(want);
    CallOnForegroundFunc(want);
}

void CJUIAbility::CallOnForegroundFunc(const Want &want)
{
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnForeground(want);
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformForeground.");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", GetAbilityName().c_str());

    UIAbility::OnBackground();

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnBackground";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    cjAbilityObj_->OnBackground();
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call PostPerformBackground.");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "End ability is %{public}s.", GetAbilityName().c_str());
}

bool CJUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    return true;
}

bool CJUIAbility::OnPrepareTerminate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnPrepareTerminate();

    return true;
}

void CJUIAbility::GetPageStackFromWant(const Want &want, std::string &pageStack)
{
    auto stringObj = AAFwk::IString::Query(want.GetParams().GetParam(PAGE_STACK_PROPERTY_NAME));
    if (stringObj != nullptr) {
        pageStack = AAFwk::String::Unbox(stringObj);
    }
}

bool CJUIAbility::IsRestorePageStack(const Want &want)
{
    return want.GetBoolParam(SUPPORT_CONTINUE_PAGE_STACK_PROPERTY_NAME, true);
}

void CJUIAbility::RestorePageStack(const Want &want)
{
    if (IsRestorePageStack(want)) {
        std::string pageStack;
        GetPageStackFromWant(want, pageStack);
    }
}

void CJUIAbility::AbilityContinuationOrRecover(const Want &want)
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
        OnSceneRestored();
    } else {
        OnSceneCreated();
    }
}

void CJUIAbility::DoOnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (scene_ == nullptr) {
        if ((abilityContext_ == nullptr) || (sceneListener_ == nullptr)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "AbilityContext or sceneListener_ is nullptr .");
            return;
        }
        scene_ = std::make_shared<Rosen::WindowScene>();
        InitSceneDoOnForeground(scene_, want);
    } else {
        auto window = scene_->GetMainWindow();
        if (window  == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "MainWindow is nullptr .");
            return;
        }
        if (want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            TAG_LOGE(AAFwkTag::UIABILITY, "want has parameter PARAM_RESV_WINDOW_MODE.");
            auto windowMode = want.GetIntParam(
                Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            windowMode_ = windowMode;
            TAG_LOGD(AAFwkTag::UIABILITY, "Set window mode is %{public}d .", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window  == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "MainWindow is nullptr .");
        return;
    }
    if (securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    TAG_LOGD(AAFwkTag::UIABILITY, "Move scene to foreground, sceneFlag_: %{public}d .", UIAbility::sceneFlag_);
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void CJUIAbility::InitSceneDoOnForeground(std::shared_ptr<Rosen::WindowScene> scene, const Want &want)
{
    int32_t displayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
    if (setting_ != nullptr) {
        std::string strDisplayId = setting_->GetProperty(OHOS::AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        std::regex formatRegex("[0-9]{0,9}$");
        std::smatch sm;
        bool flag = std::regex_match(strDisplayId, sm, formatRegex);
        if (flag && !strDisplayId.empty()) {
            displayId = strtol(strDisplayId.c_str(), nullptr, BASE_DISPLAY_ID_NUM);
            TAG_LOGD(AAFwkTag::UIABILITY, "Success displayId is %{public}d .", displayId);
        } else {
            TAG_LOGE(AAFwkTag::UIABILITY, "Failed to formatRegex: [%{public}s] .", strDisplayId.c_str());
        }
    }
    Rosen::WMError ret = Rosen::WMError::WM_OK;
    auto option = GetWindowOption(want);
    auto sessionToken = GetSessionToken();
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled() && sessionToken != nullptr) {
        abilityContext_->SetWeakSessionToken(sessionToken);
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option, sessionToken);
    } else {
        ret = scene_->Init(displayId, abilityContext_, sceneListener_, option);
    }
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Failed to init window scene .");
        return;
    }

    AbilityContinuationOrRecover(want);
    auto window = scene_->GetMainWindow();
    if (window) {
        TAG_LOGD(AAFwkTag::UIABILITY,
            "Call RegisterDisplayMoveListener, windowId: %{public}d .", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "abilityDisplayMoveListener_ is nullptr .");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
}

void CJUIAbility::RequestFocus(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: begin .");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "scene_ is nullptr .");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(
            Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::UIABILITY, "Set window mode is %{public}d .", windowMode);
    }
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    TAG_LOGD(AAFwkTag::UIABILITY, "Lifecycle: end .");
}

void CJUIAbility::ContinuationRestore(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called .");
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Is not in continuation or scene_ is nullptr .");
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

const CJRuntime &CJUIAbility::GetCJRuntime()
{
    return cjRuntime_;
}

void CJUIAbility::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called .");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Intention execute param invalid.");
        RequestFocus(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Begin request focus .");
        auto ability = weak.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "ability is nullptr .");
            return;
        }
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get Intention executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
}

void CJUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");
    if (executeParam == nullptr) {
        TAG_LOGW(AAFwkTag::UIABILITY, "Intention execute param invalid.");
        OnForeground(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
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
        TAG_LOGE(AAFwkTag::UIABILITY, "Get Intention executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
}

bool CJUIAbility::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo& executeInfo)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called.");
    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr || cjWindowStage_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Param invalid.");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executeInfo.srcEntry = wantParams.GetStringParam("ohos.insightIntent.srcEntry");
    executeInfo.hapPath = abilityInfo_->hapPath;
    executeInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executeInfo.windowMode = windowMode_;
    executeInfo.token = context->GetToken();
    executeInfo.executeParam = executeParam;
    return true;
}
#endif
#endif
int32_t CJUIAbility::OnContinue(WantParams &wantParams)
{
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }
    auto res = cjAbilityObj_->OnContinue(wantParams);
    TAG_LOGD(AAFwkTag::UIABILITY, "CJAbility::OnContinue end, return value is %{public}d", res);

    return res;
}

int32_t CJUIAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    return 0;
}

void CJUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    UIAbility::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    auto fullConfig = GetAbilityContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::UIABILITY, "configuration is nullptr.");
        return;
    }

    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }

    cjAbilityObj_->OnConfigurationUpdated(fullConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "CJAbility::OnConfigurationUpdated end");
}

void CJUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void CJUIAbility::UpdateContextConfiguration()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void CJUIAbility::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
#ifdef SUPPORT_SCREEN
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif
#endif
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnNewWant";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnNewWant(want, GetLaunchParam());
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void CJUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin .");
    UIAbility::OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is nullptr .");
        return;
    }
    abilityContext_->OnAbilityResult(requestCode, resultCode, resultData);
    TAG_LOGD(AAFwkTag::UIABILITY, "End .");
}

sptr<IRemoteObject> CJUIAbility::CallRequest()
{
    return nullptr;
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> CJUIAbility::CreateADelegatorAbilityProperty()
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
    return property;
}

void CJUIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    UIAbility::Dump(params, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (!cjAbilityObj_) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CJAbility is not loaded.");
        return;
    }
    cjAbilityObj_->Dump(params, info);
    TAG_LOGD(AAFwkTag::UIABILITY, "Dump info size: %{public}zu.", info.size());
}

std::shared_ptr<CJAbilityObject> CJUIAbility::GetCJAbility()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
    if (cjAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "cjAbility object is nullptr.");
    }
    return cjAbilityObj_;
}
} // namespace AbilityRuntime
} // namespace OHOS
