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

#include "ui_ability.h"

#include "ability_lifecycle.h"
#include "ability_recovery.h"
#include "ability_runtime/js_ui_ability.h"
#include "configuration_convertor.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#include "reverse_continuation_scheduler_primary_stage.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
const char *UIAbility::DMS_SESSION_ID = "sessionId";
const char *UIAbility::DMS_ORIGIN_DEVICE_ID = "deviceId";
constexpr int32_t UIAbility::DEFAULT_DMS_SESSION_ID = 0;
constexpr char LAUNCHER_BUNDLE_NAME[] = "com.ohos.launcher";
constexpr char LAUNCHER_ABILITY_NAME[] = "com.ohos.launcher.MainAbility";
constexpr char SHOW_ON_LOCK_SCREEN[] = "ShowOnLockScreen";
constexpr char DLP_INDEX[] = "ohos.dlp.params.index";
constexpr char DLP_PARAMS_SECURITY_FLAG[] = "ohos.dlp.params.securityFlag";
constexpr char COMPONENT_STARTUP_NEW_RULES[] = "component.startup.newRules";
constexpr int32_t ERR_INVALID_VALUE = -1;
UIAbility *UIAbility::Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime)
{
    if (!runtime) {
        return new UIAbility;
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return AbilityRuntime::JsUIAbility::Create(runtime);

        default:
            return new UIAbility();
    }
}

void UIAbility::Init(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    const std::shared_ptr<AppExecFwk::OHOSApplication> application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    application_ = application;
    abilityInfo_ = abilityInfo;
    handler_ = handler;
    AbilityContext::token_ = token;
#ifdef SUPPORT_GRAPHICS
    continuationManager_ = std::make_shared<AppExecFwk::ContinuationManagerStage>();
    std::weak_ptr<UIAbility> ability = shared_from_this();
    std::weak_ptr<AppExecFwk::ContinuationManagerStage> continuationManager = continuationManager_;
    continuationHandler_ = std::make_shared<AppExecFwk::ContinuationHandlerStage>(continuationManager, ability);
    if (!continuationManager_->Init(shared_from_this(), GetToken(), GetAbilityInfo(), continuationHandler_)) {
        continuationManager_.reset();
    } else {
        std::weak_ptr<AppExecFwk::ContinuationHandlerStage> continuationHandler = continuationHandler_;
        sptr<AppExecFwk::ReverseContinuationSchedulerPrimaryStage> primary =
            new (std::nothrow) AppExecFwk::ReverseContinuationSchedulerPrimaryStage(continuationHandler, handler_);
        if (primary == nullptr) {
            HILOG_ERROR("primary is nullptr");
        } else {
            continuationHandler_->SetPrimaryStub(primary);
            continuationHandler_->SetAbilityInfo(abilityInfo_);
        }
    }
    // register displayid change callback
    HILOG_DEBUG("call RegisterDisplayListener");
    abilityDisplayListener_ = new UIAbilityDisplayListener(ability);
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(abilityDisplayListener_);
#endif
    lifecycle_ = std::make_shared<AppExecFwk::LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AppExecFwk::AbilityLifecycleExecutor>();
    if (abilityLifecycleExecutor_ != nullptr) {
        abilityLifecycleExecutor_->DispatchLifecycleState(
            AppExecFwk::AbilityLifecycleExecutor::LifecycleState::INITIAL);
    } else {
        HILOG_ERROR("DispatchLifecycleState failed");
    }

    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
    }
    HILOG_DEBUG("end");
}

void UIAbility::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
}

void UIAbility::OnStart(const AAFwk::Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo_ is nullptr");
        return;
    }

    appIndex_ = want.GetIntParam(DLP_INDEX, 0);
    (const_cast<AAFwk::Want &>(want)).RemoveParam(DLP_INDEX);
    securityFlag_ = want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false);
    (const_cast<AAFwk::Want &>(want)).RemoveParam(DLP_PARAMS_SECURITY_FLAG);
    SetWant(want);
    sessionInfo_ = sessionInfo;
    HILOG_DEBUG("begin ability is %{public}s", abilityInfo_->name.c_str());
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        int32_t defualtDisplayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
        int32_t displayId = want.GetIntParam(AAFwk::Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        HILOG_DEBUG("abilityName:%{public}s, displayId:%{public}d", abilityInfo_->name.c_str(), displayId);
        auto option = GetWindowOption(want);
        InitWindow(displayId, option);

        // Update resMgr, Configuration
        HILOG_DEBUG("displayId %{public}d", displayId);
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
                HILOG_ERROR("resConfig is nullptr");
                return;
            }
            auto resourceManager = GetResourceManager();
            if (resourceManager != nullptr) {
                resourceManager->GetResConfig(*resConfig);
                resConfig->SetScreenDensity(density);
                resConfig->SetDirection(AppExecFwk::ConvertDirection(height, width));
                resourceManager->UpdateResConfig(*resConfig);
                HILOG_DEBUG("Density: %{public}f, Direction: %{public}d", resConfig->GetScreenDensity(),
                    resConfig->GetDirection());
            }
        }
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("abilityLifecycleExecutor_ is nullptr");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("lifecycle_ is nullptr");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_START, want);
    HILOG_DEBUG("end");
}

void UIAbility::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
#ifdef SUPPORT_GRAPHICS
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleSaveAbilityState(AppExecFwk::StateReason::LIFECYCLE);
    }
    (void)Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(abilityDisplayListener_);
    auto &&window = GetWindow();
    if (window != nullptr) {
        HILOG_DEBUG("Call UnregisterDisplayMoveListener");
        window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
    // Call JS Func(onWindowStageDestroy) and Release the scene.
    if (scene_ != nullptr) {
        scene_->GoDestroy();
        onSceneDestroyed();
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("abilityLifecycleExecutor_ is nullptr");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AppExecFwk::AbilityLifecycleExecutor::LifecycleState::INITIAL);
    if (lifecycle_ == nullptr) {
        HILOG_ERROR("lifecycle_ is nullptr");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_STOP);
    HILOG_DEBUG("end");
}

void UIAbility::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    isAsyncCallback = false;
    OnStop();
}

void UIAbility::OnStopCallback()
{
    HILOG_DEBUG("called");
}

void UIAbility::DestroyInstance()
{
    HILOG_DEBUG("called");
}

bool UIAbility::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is null");
        return false;
    }

    if (launchParam_.launchReason != AAFwk::LaunchReason::LAUNCHREASON_CONTINUATION) {
        HILOG_DEBUG("launchReason is %{public}d", launchParam_.launchReason);
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        HILOG_DEBUG("not Restored In Continuation");
        return false;
    }

    HILOG_DEBUG("end");
    return true;
}

bool UIAbility::ShouldRecoverState(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        HILOG_ERROR("AppRecovery not recovery restart");
        return false;
    }

    if (abilityRecovery_ == nullptr) {
        HILOG_WARN("abilityRecovery_ is null");
        return false;
    }

    if (abilityContext_ == nullptr) {
        HILOG_WARN("abilityContext_ is null");
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        HILOG_WARN("GetContentStorage is null");
        return false;
    }

    return true;
}

void UIAbility::NotifyContinuationResult(const AAFwk::Want &want, bool success)
{
    HILOG_DEBUG("begin");
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);

    if (continuationManager_ == nullptr) {
        HILOG_WARN("continuationManager_ is null");
        return;
    }
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

void UIAbility::OnConfigurationUpdatedNotify(const AppExecFwk::Configuration &configuration)
{
    HILOG_DEBUG("begin");
    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    InitConfigurationProperties(configuration, language, colormode, hasPointerDevice);
    // Notify ResourceManager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("Create res config failed");
        return;
    }
    auto resourceManager = GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
#ifdef SUPPORT_GRAPHICS
        if (!language.empty()) {
            UErrorCode status = U_ZERO_ERROR;
            icu::Locale locale = icu::Locale::forLanguageTag(language, status);
            HILOG_DEBUG("get forLanguageTag return[%{public}d]", static_cast<int>(status));
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
        HILOG_DEBUG("Current colorMode: %{public}d, hasPointerDevice: %{publis}d", resConfig->GetColorMode(),
            resConfig->GetInputDevice());
    }

    if (abilityContext_ != nullptr && application_ != nullptr) {
        abilityContext_->SetConfiguration(application_->GetConfiguration());
    }
    // Notify Ability Subclass
    OnConfigurationUpdated(configuration);
    HILOG_DEBUG("end");
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
        HILOG_DEBUG("displayId: [%{public}d], language: [%{public}s], colormode: [%{public}s], "
                    "hasPointerDevice: [%{public}s]",
            displayId, language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    } else {
        language = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        HILOG_DEBUG("language: [%{public}s], colormode: [%{public}s], hasPointerDevice: [%{public}s]", language.c_str(),
            colormode.c_str(), hasPointerDevice.c_str());
    }
}

void UIAbility::OnMemoryLevel(int level)
{
    HILOG_DEBUG("begin");
    if (scene_ == nullptr) {
        HILOG_ERROR("WindowScene is null");
        return;
    }
    scene_->NotifyMemoryLevel(level);
}

std::string UIAbility::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo_ is nullptr");
        return "";
    }
    return abilityInfo_->name;
}

void UIAbility::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &want)
{
    HILOG_DEBUG("called");
}

void UIAbility::OnNewWant(const AAFwk::Want &want)
{
    HILOG_DEBUG("called");
}

void UIAbility::OnRestoreAbilityState(const AppExecFwk::PacMap &inState)
{
    HILOG_DEBUG("called");
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
    HILOG_DEBUG("called");
}

void UIAbility::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("called");
}

AppExecFwk::AbilityLifecycleExecutor::LifecycleState UIAbility::GetState()
{
    HILOG_DEBUG("called");
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("abilityLifecycleExecutor_ is nullptr");
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
        HILOG_ERROR("deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("VerifySupportForContinuation failed");
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
    HILOG_DEBUG("begin");
    if (continuationManager_ == nullptr) {
        HILOG_ERROR("Continuation manager is nullptr");
        return;
    }

    continuationManager_->ChangeProcessStateToInit();
}

void UIAbility::OnRemoteTerminated()
{
    HILOG_DEBUG("called");
}

void UIAbility::DispatchLifecycleOnForeground(const AAFwk::Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("abilityLifecycleExecutor_ is nullptr");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("lifecycle_ is nullptr");
        return;
    }
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_FOREGROUND, want);
}

bool UIAbility::VerifySupportForContinuation()
{
    if (continuationManager_ == nullptr) {
        HILOG_ERROR("continuationManager_ is nullptr");
        return false;
    }
    return true;
}

void UIAbility::HandleCreateAsRecovery(const AAFwk::Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        HILOG_ERROR("AppRecovery not recovery restart");
        return;
    }

    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleRestoreAbilityState(AppExecFwk::StateReason::DEVELOPER_REQUEST, want);
    }
}

void UIAbility::SetStartAbilitySetting(std::shared_ptr<AppExecFwk::AbilityStartSetting> setting)
{
    HILOG_DEBUG("called");
    setting_ = setting;
}

void UIAbility::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    HILOG_DEBUG("called");
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

std::shared_ptr<Global::Resource::ResourceManager> UIAbility::GetResourceManager() const
{
    return AppExecFwk::AbilityContext::GetResourceManager();
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

#ifdef SUPPORT_GRAPHICS
void UIAbility::OnSceneCreated()
{
    HILOG_DEBUG("called");
}

void UIAbility::OnSceneRestored()
{
    HILOG_DEBUG("called");
}

void UIAbility::onSceneDestroyed()
{
    HILOG_DEBUG("called");
}

void UIAbility::OnForeground(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    DoOnForeground(want);
    DispatchLifecycleOnForeground(want);
    HILOG_DEBUG("end");
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    eventInfo.callerBundleName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    if (abilityInfo_ != nullptr) {
        eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    } else {
        HILOG_ERROR("abilityInfo_ is nullptr");
    }
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONFOREGROUND, HiSysEventType::BEHAVIOR, eventInfo);
}

void UIAbility::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo_ is nullptr");
        return;
    }

    if (scene_ != nullptr) {
        HILOG_DEBUG("GoBackground sceneFlag:%{public}d", sceneFlag_);
        scene_->GoBackground(sceneFlag_);
    }
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleSaveAbilityState(AppExecFwk::StateReason::LIFECYCLE);
    }

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("abilityLifecycleExecutor_ is nullptr");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(
        AppExecFwk::AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("lifecycle_ is nullptr");
        return;
    }
#ifdef IMAGE_PURGEABLE_PIXELMAP
    PurgeableMem::PurgeableResourceManager::GetInstance().EndAccessPurgeableMem();
#endif
    lifecycle_->DispatchLifecycle(AppExecFwk::LifeCycle::Event::ON_BACKGROUND);
    HILOG_DEBUG("end");
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONBACKGROUND, HiSysEventType::BEHAVIOR, eventInfo);
}

bool UIAbility::OnPrepareTerminate()
{
    HILOG_DEBUG("called");
    return false;
}

const sptr<Rosen::Window> UIAbility::GetWindow()
{
    HILOG_DEBUG("called");
    return nullptr;
}

std::shared_ptr<Rosen::WindowScene> UIAbility::GetScene()
{
    return scene_;
}

void UIAbility::OnLeaveForeground()
{
    HILOG_DEBUG("called");
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
    HILOG_DEBUG("called");
}

int32_t UIAbility::GetCurrentWindowMode()
{
    HILOG_DEBUG("called");
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
    HILOG_DEBUG("begin");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("invalid ability info");
        return ERR_INVALID_VALUE;
    }

    if (scene_ == nullptr) {
        HILOG_ERROR("scene is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("get window scene failed");
        return ERR_INVALID_VALUE;
    }

    if (window->SetAPPWindowLabel(label) != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("SetAPPWindowLabel failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

ErrCode UIAbility::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    HILOG_DEBUG("called");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("abilityInfo_ is nullptr or not page type");
        return ERR_INVALID_VALUE;
    }

    if (scene_ == nullptr) {
        HILOG_ERROR("scene_ is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (window->SetAPPWindowIcon(icon) != OHOS::Rosen::WMError::WM_OK) {
        HILOG_ERROR("SetAPPWindowIcon failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

void UIAbility::GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
{
    HILOG_DEBUG("called");
    if (scene_ == nullptr) {
        HILOG_ERROR("scene is nullptr");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr");
        return;
    }
    left = window->GetRect().posX_;
    top = window->GetRect().posY_;
    width = static_cast<int32_t>(window->GetRect().width_);
    height = static_cast<int32_t>(window->GetRect().height_);
    HILOG_DEBUG("left: %{public}d, top: %{public}d, width: %{public}d, height: %{public}d", left, top, width, height);
}

Ace::UIContent *UIAbility::GetUIContent()
{
    HILOG_DEBUG("call");
    if (scene_ == nullptr) {
        HILOG_ERROR("get window scene failed");
        return nullptr;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("get window failed");
        return nullptr;
    }
    return window->GetUIContent();
}

void UIAbility::OnCreate(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("called");
}

void UIAbility::OnDestroy(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("called");
}

void UIAbility::OnChange(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("start, displayId: %{public}" PRIu64 "", displayId);

    // Get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        HILOG_ERROR("Get display by displayId %{public}" PRIu64 " failed", displayId);
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
            HILOG_DEBUG("Notify ResourceManager, Density: %{public}f, Direction: %{public}d",
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
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        HILOG_ERROR("configuration is nullptr");
        return;
    }

    std::vector<std::string> changeKeyV;
    configuration->CompareDifferent(changeKeyV, newConfig);
    HILOG_DEBUG("changeKeyV size :%{public}zu", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [ability = shared_from_this(), configuration = *configuration]() {
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);

        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
        HILOG_DEBUG("Update display config %{public}s for all windows", diffConfiguration->GetName().c_str());
        Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
    }

    HILOG_DEBUG("end");
}

void UIAbility::OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to)
{
    HILOG_DEBUG("from displayId %{public}" PRIu64 " to %{public}" PRIu64 ".", from, to);

    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(to);
    if (!display) {
        HILOG_ERROR("Get display by displayId %{public}" PRIu64 " failed.", to);
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
            HILOG_DEBUG("Density: %{public}f, Direction: %{public}d", resConfig->GetScreenDensity(),
                resConfig->GetDirection());
        }
    }

    AppExecFwk::Configuration newConfig;
    newConfig.AddItem(AppExecFwk::ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(to));
    newConfig.AddItem(
        to, AppExecFwk::ConfigurationInner::APPLICATION_DIRECTION, AppExecFwk::GetDirectionStr(height, width));
    newConfig.AddItem(to, AppExecFwk::ConfigurationInner::APPLICATION_DENSITYDPI, AppExecFwk::GetDensityStr(density));

    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    std::vector<std::string> changeKeyV;
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        HILOG_ERROR("configuration is nullptr");
        return;
    }

    configuration->CompareDifferent(changeKeyV, newConfig);
    HILOG_DEBUG("changeKeyV size :%{public}zu", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [ability = shared_from_this(), configuration = *configuration]() {
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);
    }
}

void UIAbility::RequestFocus(const AAFwk::Want &want)
{
    HILOG_DEBUG("called");
}

void UIAbility::InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option)
{
    HILOG_DEBUG("called");
}

sptr<Rosen::WindowOption> UIAbility::GetWindowOption(const AAFwk::Want &want)
{
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    if (option == nullptr) {
        HILOG_ERROR("option is null");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(
        AAFwk::Want::PARAM_RESV_WINDOW_MODE, AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    HILOG_DEBUG("window mode is %{public}d", windowMode);
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
        HILOG_DEBUG("add window flag WINDOW_FLAG_SHOW_WHEN_LOCKED");
        option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        HILOG_DEBUG("Set window type for launcher");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    return option;
}

void UIAbility::ContinuationRestore(const AAFwk::Want &want)
{
    HILOG_DEBUG("called");
}
#endif
} // namespace AbilityRuntime
} // namespace OHOS
