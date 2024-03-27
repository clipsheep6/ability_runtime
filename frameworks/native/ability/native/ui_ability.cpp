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

#include "ui_ability.h"

#include "ability_lifecycle.h"
#include "ability_recovery.h"
#include "configuration_convertor.h"
#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_ui_ability.h"
#include "ohos_application.h"
#include "reverse_continuation_scheduler_primary_stage.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char DMS_SESSION_ID[] = "sessionId";
constexpr char DMS_ORIGIN_DEVICE_ID[] = "deviceId";
constexpr int32_t DEFAULT_DMS_SESSION_ID = 0;
constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
constexpr char LAUNCHER_ABILITY_NAME[] = "com.ohos.launcher.MainAbility";
constexpr char SHOW_ON_LOCK_SCREEN[] = "ShowOnLockScreen";
constexpr char DLP_INDEX[] = "ohos.dlp.params.index";
constexpr char DLP_PARAMS_SECURITY_FLAG[] = "ohos.dlp.params.securityFlag";
constexpr char COMPONENT_STARTUP_NEW_RULES[] = "component.startup.newRules";
constexpr int32_t ERR_INVALID_VALUE = -1;
}
UIAbility *UIAbility::Create(const std::unique_ptr<Runtime> &runtime)
{
    if (!runtime) {
        return new (std::nothrow) UIAbility;
    }

    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsUIAbility::Create(runtime);
        default:
            return new (std::nothrow) UIAbility();
    }
}

void UIAbility::Init(std::shared_ptr<AppExecFwk::AbilityLocalRecord> record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityLocalRecord is nullptr.");
        return;
    }
    application_ = application;
    abilityInfo_ = record->GetAbilityInfo();
    handler_ = handler;
    token_ = token;
#ifdef SUPPORT_GRAPHICS
    continuationManager_ = std::make_shared<AppExecFwk::ContinuationManagerStage>();
    std::weak_ptr<AppExecFwk::ContinuationManagerStage> continuationManager = continuationManager_;
    continuationHandler_ =
        std::make_shared<AppExecFwk::ContinuationHandlerStage>(continuationManager, weak_from_this());
    if (!continuationManager_->Init(shared_from_this(), GetToken(), GetAbilityInfo(), continuationHandler_)) {
        continuationManager_.reset();
    } else {
        std::weak_ptr<AppExecFwk::ContinuationHandlerStage> continuationHandler = continuationHandler_;
        sptr<AppExecFwk::ReverseContinuationSchedulerPrimaryStage> primary =
            new (std::nothrow) AppExecFwk::ReverseContinuationSchedulerPrimaryStage(continuationHandler, handler_);
        if (primary == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "Primary is nullptr.");
        } else {
            continuationHandler_->SetPrimaryStub(primary);
            continuationHandler_->SetAbilityInfo(abilityInfo_);
        }
    }
    // register displayid change callback
    TAG_LOGD(AAFwkTag::UIABILITY, "Call RegisterDisplayListener.");
    abilityDisplayListener_ = new (std::nothrow) UIAbilityDisplayListener(weak_from_this());
    if (abilityDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityDisplayListener_ is nullptr.");
        return;
    }
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(abilityDisplayListener_);
#endif
    lifecycle_ = std::make_shared<AppExecFwk::LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AppExecFwk::AbilityLifecycleExecutor>();
    abilityLifecycleExecutor_->DispatchLifecycleState(AppExecFwk::AbilityLifecycleExecutor::LifecycleState::INITIAL);
    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

std::shared_ptr<OHOS::AppExecFwk::LifeCycle> UIAbility::GetLifecycle()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    return lifecycle_;
}

void UIAbility::RegisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "register UIAbility lifecycle observer failed, observer is nullptr.");
        return;
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "register UIAbility lifecycle observer failed, lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->AddObserver(observer);
}

void UIAbility::UnregisterAbilityLifecycleObserver(const std::shared_ptr<AppExecFwk::ILifecycleObserver> &observer)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "unregister UIAbility lifecycle observer failed, observer is nullptr.");
        return;
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "unregister UIAbility lifecycle observer failed, lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->RemoveObserver(observer);
}

void UIAbility::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
}

void UIAbility::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AbilityInfo_ is nullptr.");
        return;
    }

    appIndex_ = want.GetIntParam(DLP_INDEX, 0);
    (const_cast<AAFwk::Want &>(want)).RemoveParam(DLP_INDEX);
    securityFlag_ = want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false);
    (const_cast<AAFwk::Want &>(want)).RemoveParam(DLP_PARAMS_SECURITY_FLAG);
    SetWant(want);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin ability is %{public}s.", abilityInfo_->name.c_str());
#ifdef SUPPORT_GRAPHICS
    if (sessionInfo != nullptr) {
        SetSessionToken(sessionInfo->sessionToken);
    }
    OnStartForSupportGraphics(want);
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_START, want);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
#ifdef SUPPORT_GRAPHICS
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleSaveAbilityState(AppExecFwk::StateReason::LIFECYCLE);
    }
    (void)Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(abilityDisplayListener_);
    auto &&window = GetWindow();
    if (window != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Call UnregisterDisplayMoveListener.");
        window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
    // Call JS Func(onWindowStageDestroy) and Release the scene.
    if (scene_ != nullptr) {
        scene_->GoDestroy();
        onSceneDestroyed();
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AppExecFwk::AbilityLifecycleExecutor::LifecycleState::INITIAL);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_STOP);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    isAsyncCallback = false;
    OnStop();
}

void UIAbility::OnStopCallback()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::DestroyInstance()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

bool UIAbility::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is null.");
        return false;
    }

    if (launchParam_.launchReason != AAFwk::LaunchReason::LAUNCHREASON_CONTINUATION) {
        TAG_LOGD(AAFwkTag::UIABILITY, "LaunchReason is %{public}d.", launchParam_.launchReason);
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Get content failed.");
        return false;
    }
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
    return true;
}

bool UIAbility::ShouldRecoverState(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AppRecovery not recovery restart.");
        return false;
    }

    if (abilityRecovery_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityRecovery_ is null.");
        return false;
    }

    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext_ is null.");
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get content failed.");
        return false;
    }
    return true;
}

void UIAbility::NotifyContinuationResult(const AAFwk::Want &want, bool success)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);

    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "continuationManager_ is null.");
        return;
    }
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

void UIAbility::OnConfigurationUpdatedNotify(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "begin");
    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    InitConfigurationProperties(configuration, language, colormode, hasPointerDevice);
    // Notify ResourceManager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Create res config failed.");
        return;
    }
    auto resourceManager = GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
#ifdef SUPPORT_GRAPHICS
        if (!language.empty()) {
            UErrorCode status = U_ZERO_ERROR;
            icu::Locale locale = icu::Locale::forLanguageTag(language, status);
            TAG_LOGD(AAFwkTag::UIABILITY, "Get forLanguageTag return[%{public}d].", static_cast<int>(status));
            if (status == U_ZERO_ERROR) {
                resConfig->SetLocaleInfo(locale);
            }
        }
#endif
        if (!colormode.empty()) {
            resConfig->SetColorMode(AppExecFwk::ConvertColorMode(colormode));
        }
        if (!hasPointerDevice.empty()) {
            resConfig->SetInputDevice(AppExecFwk::ConvertHasPointerDevice(hasPointerDevice));
        }
        resourceManager->UpdateResConfig(*resConfig);
        TAG_LOGD(AAFwkTag::UIABILITY, "Current colorMode: %{public}d, hasPointerDevice: %{public}d.",
            resConfig->GetColorMode(), resConfig->GetInputDevice());
    }

    if (abilityContext_ != nullptr && application_ != nullptr) {
        abilityContext_->SetConfiguration(application_->GetConfiguration());
    }
    // Notify Ability Subclass
    OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbility::InitConfigurationProperties(const AppExecFwk::Configuration &changeConfiguration, std::string &language,
    std::string &colormode, std::string &hasPointerDevice)
{
    if (setting_) {
        auto displayId =
            std::atoi(setting_->GetProperty(AppExecFwk::AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str());
        language = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        TAG_LOGD(AAFwkTag::UIABILITY, "displayId: [%{public}d], language: [%{public}s], colormode: [%{public}s], "
                    "hasPointerDevice: [%{public}s].",
            displayId, language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    } else {
        language = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        TAG_LOGD(AAFwkTag::UIABILITY,
            "Language: [%{public}s], colormode: [%{public}s], hasPointerDevice: [%{public}s].",
            language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    }
}

void UIAbility::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "WindowScene is null.");
        return;
    }
    scene_->NotifyMemoryLevel(level);
}

std::string UIAbility::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo_ is nullptr");
        return "";
    }
    return abilityInfo_->name;
}

std::string UIAbility::GetModuleName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo_ is nullptr.");
        return "";
    }

    return abilityInfo_->moduleName;
}

void UIAbility::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnNewWant(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnRestoreAbilityState(const AppExecFwk::PacMap &inState)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::SetWant(const AAFwk::Want &want)
{
    setWant_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> UIAbility::GetWant()
{
    return setWant_;
}

void UIAbility::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

AppExecFwk::AbilityLifecycleExecutor::LifecycleState UIAbility::GetState()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return AppExecFwk::AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }
    return static_cast<AppExecFwk::AbilityLifecycleExecutor::LifecycleState>(abilityLifecycleExecutor_->GetState());
}

int32_t UIAbility::OnContinue(AAFwk::WantParams &wantParams)
{
    return AppExecFwk::ContinuationManagerStage::OnContinueResult::REJECT;
}

void UIAbility::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "DeviceId is empty.");
        return;
    }

    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "continuationManager_ is nullptr.");
        return;
    }
    continuationManager_->ContinueAbilityWithStack(deviceId, versionCode);
}

bool UIAbility::OnStartContinuation()
{
    return false;
}

bool UIAbility::OnSaveData(AAFwk::WantParams &saveData)
{
    return false;
}

bool UIAbility::OnRestoreData(AAFwk::WantParams &restoreData)
{
    return false;
}

int32_t UIAbility::OnSaveState(int32_t reason, AAFwk::WantParams &wantParams)
{
    return ERR_OK;
}

void UIAbility::OnCompleteContinuation(int result)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Continuation manager is nullptr.");
        return;
    }

    continuationManager_->ChangeProcessStateToInit();
}

void UIAbility::OnRemoteTerminated()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::DispatchLifecycleOnForeground(const AAFwk::Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_FOREGROUND, want);
}

void UIAbility::HandleCreateAsRecovery(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "AppRecovery not recovery restart.");
        return;
    }

    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleRestoreAbilityState(AppExecFwk::StateReason::DEVELOPER_REQUEST, want);
    }
}

void UIAbility::SetStartAbilitySetting(std::shared_ptr<AppExecFwk::AbilityStartSetting> setting)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    setting_ = setting;
}

void UIAbility::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam &UIAbility::GetLaunchParam() const
{
    return launchParam_;
}

std::shared_ptr<AbilityRuntime::AbilityContext> UIAbility::GetAbilityContext()
{
    return abilityContext_;
}

sptr<IRemoteObject> UIAbility::CallRequest()
{
    return nullptr;
}

bool UIAbility::IsUseNewStartUpRule()
{
    if (!isNewRuleFlagSetted_ && setWant_) {
        startUpNewRule_ = setWant_->GetBoolParam(COMPONENT_STARTUP_NEW_RULES, false);
        isNewRuleFlagSetted_ = true;
    }
    return startUpNewRule_;
}

void UIAbility::EnableAbilityRecovery(const std::shared_ptr<AppExecFwk::AbilityRecovery> &abilityRecovery)
{
    abilityRecovery_ = abilityRecovery;
}

int32_t UIAbility::OnShare(AAFwk::WantParams &wantParams)
{
    return ERR_OK;
}

bool UIAbility::CheckIsSilentForeground() const
{
    return isSilentForeground_;
}

void UIAbility::SetIsSilentForeground(bool isSilentForeground)
{
    isSilentForeground_ = isSilentForeground;
}

#ifdef SUPPORT_GRAPHICS
void UIAbility::OnSceneCreated()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnSceneRestored()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnForeground(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    DoOnForeground(want);
    if (isSilentForeground_) {
        TAG_LOGD(AAFwkTag::UIABILITY, "silent foreground, return");
        return;
    }
    DispatchLifecycleOnForeground(want);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    eventInfo.callerBundleName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    if (abilityInfo_ != nullptr) {
        eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    } else {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo_ is nullptr.");
    }
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONFOREGROUND, HiSysEventType::BEHAVIOR, eventInfo);
}

void UIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin.");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo_ is nullptr.");
        return;
    }

    if (scene_ != nullptr) {
        TAG_LOGD(AAFwkTag::UIABILITY, "GoBackground sceneFlag: %{public}d.", sceneFlag_);
        scene_->GoBackground(sceneFlag_);
    }
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleSaveAbilityState(AppExecFwk::StateReason::LIFECYCLE);
    }

    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_BACKGROUND);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONBACKGROUND, HiSysEventType::BEHAVIOR, eventInfo);
}

bool UIAbility::OnPrepareTerminate()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    return false;
}

const sptr<Rosen::Window> UIAbility::GetWindow()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    return nullptr;
}

std::shared_ptr<Rosen::WindowScene> UIAbility::GetScene()
{
    return scene_;
}

void UIAbility::OnLeaveForeground()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

std::string UIAbility::GetContentInfo()
{
    if (scene_ == nullptr) {
        return "";
    }
    return scene_->GetContentInfo();
}

void UIAbility::SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    sceneListener_ = listener;
}

void UIAbility::DoOnForeground(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

int32_t UIAbility::GetCurrentWindowMode()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    auto windowMode = static_cast<int>(Rosen::WindowMode::WINDOW_MODE_UNDEFINED);
    if (scene_ == nullptr) {
        return windowMode;
    }
    auto window = scene_->GetMainWindow();
    if (window != nullptr) {
        windowMode = static_cast<int>(window->GetMode());
    }
    return windowMode;
}

ErrCode UIAbility::SetMissionLabel(const std::string &label)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Invalid ability info.");
        return ERR_INVALID_VALUE;
    }

    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Scene is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get window scene failed.");
        return ERR_INVALID_VALUE;
    }

    if (window->SetAPPWindowLabel(label) != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "SetAPPWindowLabel failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

ErrCode UIAbility::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityInfo_ is nullptr or not page type.");
        return ERR_INVALID_VALUE;
    }

    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Scene_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Window is nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (window->SetAPPWindowIcon(icon) != OHOS::Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "SetAPPWindowIcon failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void UIAbility::GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Scene is nullptr.");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Window is nullptr.");
        return;
    }
    left = window->GetRect().posX_;
    top = window->GetRect().posY_;
    width = static_cast<int32_t>(window->GetRect().width_);
    height = static_cast<int32_t>(window->GetRect().height_);
    TAG_LOGD(AAFwkTag::UIABILITY, "left: %{public}d, top: %{public}d, width: %{public}d, height: %{public}d.",
        left, top, width, height);
}

Ace::UIContent *UIAbility::GetUIContent()
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get window scene failed.");
        return nullptr;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get window failed.");
        return nullptr;
    }
    return window->GetUIContent();
}

void UIAbility::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Begin displayId: %{public}" PRIu64 "", displayId);
    // Get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get display by displayId %{public}" PRIu64 " failed.", displayId);
        return;
    }

    // Notify ResourceManager
    float density = display->GetVirtualPixelRatio();
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig != nullptr) {
        auto resourceManager = GetResourceManager();
        if (resourceManager != nullptr) {
            resourceManager->GetResConfig(*resConfig);
            resConfig->SetScreenDensity(density);
            resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            TAG_LOGD(AAFwkTag::UIABILITY, "Notify ResourceManager, Density: %{public}f, Direction: %{public}d",
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    // Notify ability
    Configuration newConfig;
    newConfig.AddItem(
        displayId, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, AppExecFwk::GetDirectionStr(height, width));
    newConfig.AddItem(
        displayId, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, AppExecFwk::GetDensityStr(density));

    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "application_ is nullptr.");
        return;
    }

    OnChangeForUpdateConfiguration(newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "End.");
}

void UIAbility::OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "From displayId %{public}" PRIu64 " to %{public}" PRIu64 "", from, to);
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(to);
    if (!display) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Get display by displayId %{public}" PRIu64 " failed.", to);
        return;
    }
    // Get new display config
    float density = display->GetVirtualPixelRatio();
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig != nullptr) {
        auto resourceManager = GetResourceManager();
        if (resourceManager != nullptr) {
            resourceManager->GetResConfig(*resConfig);
            resConfig->SetScreenDensity(density);
            resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            TAG_LOGD(AAFwkTag::UIABILITY,
                "Density: %{public}f, Direction: %{public}d", resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    AppExecFwk::Configuration newConfig;
    newConfig.AddItem(AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(to));
    newConfig.AddItem(
        to, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, AppExecFwk::GetDirectionStr(height, width));
    newConfig.AddItem(to, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, AppExecFwk::GetDensityStr(density));
    if (application_ == nullptr || handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "application_ or handler_ is nullptr.");
        return;
    }
    std::vector<std::string> changeKeyV;
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Configuration is nullptr.");
        return;
    }

    configuration->CompareDifferent(changeKeyV, newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "changeKeyV size: %{public}zu.", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [abilityWptr = weak_from_this(), configuration = *configuration]() {
            auto ability = abilityWptr.lock();
            if (ability == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "ability is nullptr.");
                return;
            }
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);
    }
}

void UIAbility::RequestFocus(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

sptr<Rosen::WindowOption> UIAbility::GetWindowOption(const AAFwk::Want &want)
{
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Option is null.");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(
        AAFwk::Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    TAG_LOGD(AAFwkTag::UIABILITY, "Window mode is %{public}d.", windowMode);
    option->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
    bool showOnLockScreen = false;
    if (abilityInfo_) {
        std::vector<AppExecFwk::CustomizeData> datas = abilityInfo_->metaData.customizeData;
        for (AppExecFwk::CustomizeData data : datas) {
            if (data.name == SHOW_ON_LOCK_SCREEN) {
                showOnLockScreen = true;
            }
        }
    }
    if (showOnLockScreen_ || showOnLockScreen) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Add window flag WINDOW_FLAG_SHOW_WHEN_LOCKED.");
        option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        TAG_LOGD(AAFwkTag::UIABILITY, "Set window type for launcher.");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    return option;
}

void UIAbility::ContinuationRestore(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "Called.");
}

void UIAbility::OnStartForSupportGraphics(const AAFwk::Want &want)
{
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        int32_t defualtDisplayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
        int32_t displayId = want.GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        TAG_LOGD(AAFwkTag::UIABILITY, "abilityName: %{public}s, displayId: %{public}d.",
            abilityInfo_->name.c_str(), displayId);
        auto option = GetWindowOption(want);
        InitWindow(displayId, option);

        // Update resMgr, Configuration
        TAG_LOGD(AAFwkTag::UIABILITY, "DisplayId is %{public}d.", displayId);
        auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
        if (display) {
            float density = display->GetVirtualPixelRatio();
            int32_t width = display->GetWidth();
            int32_t height = display->GetHeight();
            std::shared_ptr<AppExecFwk::Configuration> configuration = nullptr;
            if (application_) {
                configuration = application_->GetConfiguration();
            }
            if (configuration) {
                std::string direction = AppExecFwk::GetDirectionStr(height, width);
                configuration->AddItem(displayId, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, direction);
                configuration->AddItem(displayId, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI,
                    AppExecFwk::GetDensityStr(density));
                configuration->AddItem(
                    AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(displayId));
                UpdateContextConfiguration();
            }

            std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
            if (resConfig == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "ResConfig is nullptr.");
                return;
            }
            auto resourceManager = GetResourceManager();
            if (resourceManager != nullptr) {
                resourceManager->GetResConfig(*resConfig);
                resConfig->SetScreenDensity(density);
                resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
                resourceManager->UpdateResConfig(*resConfig);
                TAG_LOGD(AAFwkTag::UIABILITY, "Density: %{public}f, Direction: %{public}d",
                    resConfig->GetScreenDensity(), resConfig->GetDirection());
            }
        }
    }
}

void UIAbility::OnChangeForUpdateConfiguration(const AppExecFwk::Configuration &newConfig)
{
    if (application_ == nullptr || handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "application_ or handler_ is nullptr.");
        return;
    }
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        TAG_LOGE(AAFwkTag::UIABILITY, "Configuration is nullptr.");
        return;
    }

    std::vector<std::string> changeKeyV;
    configuration->CompareDifferent(changeKeyV, newConfig);
    TAG_LOGD(AAFwkTag::UIABILITY, "ChangeKeyV size: %{public}zu.", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [abilityWptr = weak_from_this(), configuration = *configuration]() {
            auto ability = abilityWptr.lock();
            if (ability == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "ability is nullptr.");
                return;
            }
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);

        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
        TAG_LOGD(AAFwkTag::UIABILITY, "Update display config %{public}s for all windows.",
            diffConfiguration->GetName().c_str());
        Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
    }
}

void UIAbility::CallOnForegroundFunc(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::ExecuteInsightIntentRepeateForeground(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::ExecuteInsightIntentMoveToForeground(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

void UIAbility::ExecuteInsightIntentBackground(const AAFwk::Want &want,
    const std::shared_ptr<InsightIntentExecuteParam> &executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "called");
}

int UIAbility::CreateModalUIExtension(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "call");
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityContextImpl->CreateModalUIExtensionWithApp(want);
}

void UIAbility::SetSessionToken(sptr<IRemoteObject> sessionToken)
{
    std::lock_guard lock(sessionTokenMutex_);
    sessionToken_ = sessionToken;
}

void UIAbility::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    SetSessionToken(sessionToken);
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext is nullptr");
        return;
    }
    abilityContextImpl->SetWeakSessionToken(sessionToken);
}

void UIAbility::EraseUIExtension(int32_t sessionId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "call");
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "abilityContext is nullptr");
        return;
    }
    abilityContextImpl->EraseUIExtension(sessionId);
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
