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
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "insight_intent_execute_param.h"
#include "cj_runtime.h"
#include "cj_ability_object.h"
#include "time_util.h"
#include "scene_board_judgement.h"
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
// Numerical base (radix) that determines the valid characters and their interpretation.
const int32_t BASE_DISPLAY_ID_NUM (10);
}

UIAbility *CJUIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new (std::nothrow) CJUIAbility(static_cast<CJRuntime &>(*runtime));
}

CJUIAbility::CJUIAbility(CJRuntime &cjRuntime) : cjRuntime_(cjRuntime)
{
    HILOG_DEBUG("Called.");
}

CJUIAbility::~CJUIAbility()
{
    HILOG_DEBUG("Called.");
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
    SetAbilityContext(abilityInfo);
}

void CJUIAbility::SetAbilityContext(
    const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (!abilityInfo) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    cjAbilityObj_ = CJAbilityObject::LoadModule(abilityInfo->name);
    if (!cjAbilityObj_) {
        HILOG_ERROR("Failed to get CJAbility object.");
        return;
    }
    cjAbilityObj_->Init(this);
}

void CJUIAbility::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnStart(want, sessionInfo);

    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnStart";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnStart(want, GetLaunchParam());
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformStart.");
        delegator->PostPerformStart(CreateADelegatorAbilityProperty());
    }
    HILOG_INFO("End ability is %{public}s.", GetAbilityName().c_str());
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
    HILOG_DEBUG("Begin.");
    return ERR_OK;
}

void CJUIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin.");
    if (abilityContext_) {
        HILOG_DEBUG("Set terminating true.");
        abilityContext_->SetTerminating(true);
    }
    UIAbility::OnStop();
    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    cjAbilityObj_->OnStop();
    CJUIAbility::OnStopCallback();
    HILOG_DEBUG("End.");
}

void CJUIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
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
    cjAbilityObj_->OnStop();
    OnStopCallback();
    HILOG_DEBUG("End.");
}

void CJUIAbility::OnStopCallback()
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformStop.");
        delegator->PostPerformStop(CreateADelegatorAbilityProperty());
    }

    bool ret = ConnectionManager::GetInstance().DisconnectCaller(AbilityContext::token_);
    if (!ret) {
        HILOG_ERROR("The service connection is disconnected.");
    }
    ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
    HILOG_DEBUG("The service connection is not disconnected.");
}

#ifdef SUPPORT_GRAPHICS
void CJUIAbility::OnSceneCreated()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::OnSceneCreated();

    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }

    cjWindowStage_ = OHOS::Rosen::CJWindowStageImpl::CreateCJWindowStage(GetScene());
    if (!cjWindowStage_) {
        HILOG_ERROR("Failed to create CJWindowStage object.");
        return;
    }

    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "onWindowStageCreate");
        std::string methodName = "OnSceneCreated";
        AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
        cjAbilityObj_->OnSceneCreated(cjWindowStage_.get());
        AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformScenceCreated.");
        delegator->PostPerformScenceCreated(CreateADelegatorAbilityProperty());
    }

    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::OnSceneRestored()
{
    UIAbility::OnSceneRestored();
    HILOG_DEBUG("called.");

    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }

    if (!cjWindowStage_) {
        cjWindowStage_ = OHOS::Rosen::CJWindowStageImpl::CreateCJWindowStage(scene_);
        if (!cjWindowStage_) {
            HILOG_ERROR("Failed to create CJWindowStage object.");
            return;
        }
    }

    cjAbilityObj_->OnSceneRestored(cjWindowStage_.get());

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformScenceRestored.");
        delegator->PostPerformScenceRestored(CreateADelegatorAbilityProperty());
    }
}

void CJUIAbility::OnSceneDestroyed()
{
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());
    UIAbility::onSceneDestroyed();

    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    cjAbilityObj_->OnSceneDestroyed();

    if (scene_ != nullptr) {
        auto window = scene_->GetMainWindow();
        if (window != nullptr) {
            HILOG_DEBUG("Call window UnregisterDisplayMoveListener.");
            window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call delegator PostPerformScenceDestroyed.");
        delegator->PostPerformScenceDestroyed(CreateADelegatorAbilityProperty());
    }
    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());

    UIAbility::OnForeground(want);
    CallOnForegroundFunc(want);
}

void CJUIAbility::CallOnForegroundFunc(const Want &want)
{
    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnForeground";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnForeground(want);
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformForeground.");
        delegator->PostPerformForeground(CreateADelegatorAbilityProperty());
    }

    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

void CJUIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability is %{public}s.", GetAbilityName().c_str());

    UIAbility::OnBackground();

    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnBackground";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);
    cjAbilityObj_->OnBackground();
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::BACKGROUND, methodName);

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call PostPerformBackground.");
        delegator->PostPerformBackground(CreateADelegatorAbilityProperty());
    }

    HILOG_DEBUG("End ability is %{public}s.", GetAbilityName().c_str());
}

bool CJUIAbility::OnBackPress()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability: %{public}s.", GetAbilityName().c_str());
    UIAbility::OnBackPress();
    return true;
}

bool CJUIAbility::OnPrepareTerminate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Begin ability: %{public}s.", GetAbilityName().c_str());
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
    HILOG_DEBUG("Launch reason is %{public}d.", launchParam_.launchReason);
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
            HILOG_ERROR("AbilityContext or sceneListener_ is nullptr .");
            return;
        }
        scene_ = std::make_shared<Rosen::WindowScene>();
        InitSceneDoOnForeground(scene_, want);
    } else {
        auto window = scene_->GetMainWindow();
        if (window  == nullptr) {
            HILOG_ERROR("MainWindow is nullptr .");
            return;
        }
        if (want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            HILOG_ERROR("want has parameter PARAM_RESV_WINDOW_MODE.");
            auto windowMode = want.GetIntParam(
                Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            windowMode_ = windowMode;
            HILOG_DEBUG("Set window mode is %{public}d .", windowMode);
        }
    }

    auto window = scene_->GetMainWindow();
    if (window  == nullptr) {
        HILOG_ERROR("MainWindow is nullptr .");
        return;
    }
    if (securityFlag_) {
        window->SetSystemPrivacyMode(true);
    }

    HILOG_INFO("Move scene to foreground, sceneFlag_: %{public}d .", UIAbility::sceneFlag_);
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    HILOG_DEBUG("End.");
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
            HILOG_DEBUG("Success displayId is %{public}d .", displayId);
        } else {
            HILOG_ERROR("Failed to formatRegex: [%{public}s] .", strDisplayId.c_str());
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
        HILOG_ERROR("Failed to init window scene .");
        return;
    }

    AbilityContinuationOrRecover(want);
    auto window = scene_->GetMainWindow();
    if (window) {
        HILOG_DEBUG("Call RegisterDisplayMoveListener, windowId: %{public}d .", window->GetWindowId());
        abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
        if (abilityDisplayMoveListener_ == nullptr) {
            HILOG_ERROR("abilityDisplayMoveListener_ is nullptr .");
            return;
        }
        window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
}

void CJUIAbility::RequestFocus(const Want &want)
{
    HILOG_INFO("Lifecycle: begin .");
    if (scene_ == nullptr) {
        HILOG_ERROR("scene_ is nullptr .");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(
            Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        HILOG_DEBUG("Set window mode is %{public}d .", windowMode);
    }
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, METHOD_NAME);
    scene_->GoForeground(UIAbility::sceneFlag_);
    HILOG_INFO("Lifecycle: end .");
}

void CJUIAbility::ContinuationRestore(const Want &want)
{
    HILOG_DEBUG("Called .");
    if (!IsRestoredInContinuation() || scene_ == nullptr) {
        HILOG_ERROR("Is not in continuation or scene_ is nullptr .");
        return;
    }
    RestorePageStack(want);
    OnSceneRestored();
    NotifyContinuationResult(want, true);
}

std::shared_ptr<Rosen::CJWindowStageImpl> CJUIAbility::GetCJWindowStage()
{
    HILOG_DEBUG("Called.");
    if (cjWindowStage_ == nullptr) {
        HILOG_ERROR("CJWindowSatge is nullptr .");
    }
    return cjWindowStage_;
}

const CJRuntime &CJUIAbility::GetCJRuntime()
{
    return cjRuntime_;
}

void CJUIAbility::ExecuteInsightIntentRepeateForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called .");
    if (executeParam == nullptr) {
        HILOG_WARN("Intention execute param invalid.");
        RequestFocus(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
    }

    auto asyncCallback = [weak = weak_from_this(), want](InsightIntentExecuteResult result) {
        HILOG_DEBUG("Begin request focus .");
        auto ability = weak.lock();
        if (ability == nullptr) {
            HILOG_ERROR("ability is nullptr .");
            return;
        }
        ability->RequestFocus(want);
    };
    callback->Push(asyncCallback);

    InsightIntentExecutorInfo executeInfo;
    auto ret = GetInsightIntentExecutorInfo(want, executeParam, executeInfo);
    if (!ret) {
        HILOG_ERROR("Get Intention executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
}

void CJUIAbility::ExecuteInsightIntentMoveToForeground(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    HILOG_DEBUG("called.");
    if (executeParam == nullptr) {
        HILOG_WARN("Intention execute param invalid.");
        OnForeground(want);
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback), ERR_OK);
        return;
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
        HILOG_ERROR("Get Intention executor failed.");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return;
    }
}

bool CJUIAbility::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo& executeInfo)
{
    HILOG_DEBUG("called.");
    auto context = GetAbilityContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr || cjWindowStage_ == nullptr) {
        HILOG_ERROR("Param invalid.");
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

int32_t CJUIAbility::OnContinue(WantParams &wantParams)
{
    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
    }
    auto res = cjAbilityObj_->OnContinue(wantParams);
    HILOG_INFO("CJAbility::OnContinue end, return value is %{public}d", res);

    return res;
}

int32_t CJUIAbility::OnSaveState(int32_t reason, WantParams &wantParams)
{
    return 0;
}

void CJUIAbility::OnConfigurationUpdated(const Configuration &configuration)
{
    UIAbility::OnConfigurationUpdated(configuration);
    HILOG_DEBUG("Called.");
    auto fullConfig = GetAbilityContext()->GetConfiguration();
    if (!fullConfig) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }

    cjAbilityObj_->OnConfigurationUpdated(fullConfig);
    HILOG_INFO("CJAbility::OnConfigurationUpdated end");
}

void CJUIAbility::OnMemoryLevel(int level)
{
    UIAbility::OnMemoryLevel(level);
    HILOG_DEBUG("Called.");
}

void CJUIAbility::UpdateContextConfiguration()
{
    HILOG_DEBUG("Called.");
}

void CJUIAbility::OnNewWant(const Want &want)
{
    HILOG_DEBUG("Begin.");
    UIAbility::OnNewWant(want);

#ifdef SUPPORT_GRAPHICS
    if (scene_) {
        scene_->OnNewWant(want);
    }
#endif
    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    std::string methodName = "OnNewWant";
    AddLifecycleEventBeforeCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);
    cjAbilityObj_->OnNewWant(want, GetLaunchParam());
    AddLifecycleEventAfterCall(FreezeUtil::TimeoutState::FOREGROUND, methodName);

    HILOG_DEBUG("End.");
}

void CJUIAbility::OnAbilityResult(int requestCode, int resultCode, const Want &resultData)
{
    HILOG_DEBUG("Begin .");
    UIAbility::OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is nullptr .");
        return;
    }
    abilityContext_->OnAbilityResult(requestCode, resultCode, resultData);
    HILOG_DEBUG("End .");
}

sptr<IRemoteObject> CJUIAbility::CallRequest()
{
    return nullptr;
}

std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> CJUIAbility::CreateADelegatorAbilityProperty()
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
    return property;
}

void CJUIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    UIAbility::Dump(params, info);
    HILOG_DEBUG("Called.");
    if (!cjAbilityObj_) {
        HILOG_ERROR("CJAbility is not loaded.");
        return;
    }
    cjAbilityObj_->Dump(params, info);
    HILOG_DEBUG("Dump info size: %{public}zu.", info.size());
}

std::shared_ptr<CJAbilityObject> CJUIAbility::GetCJAbility()
{
    HILOG_DEBUG("Called.");
    if (cjAbilityObj_ == nullptr) {
        HILOG_ERROR("cjAbility object is nullptr.");
    }
    return cjAbilityObj_;
}
} // namespace AbilityRuntime
} // namespace OHOS
