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

#include "ability.h"

#include <cinttypes>
#include <thread>

#include "ability_post_event_timeout.h"
#include "ability_recovery.h"
#include "ability_runtime/js_ability.h"
#include "abs_shared_result_set.h"
#include "configuration_convertor.h"
#include "connection_manager.h"
#include "continuation_manager.h"
#include "continuation_register_manager.h"
#include "continuation_register_manager_proxy.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "data_uri_utils.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ohos_application.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_replica.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "runtime.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "values_bucket.h"
#include "continuation_handler.h"

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_mgr_helper.h"
#include "continuous_task_param.h"
#endif

#ifdef SUPPORT_GRAPHICS
#include "display_type.h"
#include "form_provider_client.h"
#include "key_event.h"
#endif

namespace OHOS {
namespace AppExecFwk {
const std::string Ability::SYSTEM_UI("com.ohos.systemui");
const std::string Ability::STATUS_BAR("com.ohos.systemui.statusbar.MainAbility");
const std::string Ability::NAVIGATION_BAR("com.ohos.systemui.navigationbar.MainAbility");
const std::string Ability::KEYGUARD("com.ohos.screenlock");
const std::string DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
const std::string DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";
const std::string Ability::DMS_SESSION_ID("sessionId");
const std::string Ability::DMS_ORIGIN_DEVICE_ID("deviceId");
const int Ability::DEFAULT_DMS_SESSION_ID(0);
const std::string LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";
const std::string LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
const std::string DLP_INDEX = "ohos.dlp.params.index";
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";

Ability* Ability::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    if (!runtime) {
        return new Ability;
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return AbilityRuntime::JsAbility::Create(runtime);

        default:
            return new Ability();
    }
}

void Ability::Init(const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> application,
    std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    application_ = application;
    abilityInfo_ = abilityInfo;
    handler_ = handler;
    AbilityContext::token_ = token;

#ifdef SUPPORT_GRAPHICS
    // page ability only.
    if (abilityInfo_->type == AbilityType::PAGE) {
        if (!abilityInfo_->isStageBasedModel) {
            abilityWindow_ = std::make_shared<AbilityWindow>();
            abilityWindow_->Init(handler_, shared_from_this());
        }
        continuationManager_ = std::make_shared<ContinuationManager>();
        std::weak_ptr<Ability> ability = shared_from_this();
        std::weak_ptr<ContinuationManager> continuationManager = continuationManager_;
        continuationHandler_ = std::make_shared<ContinuationHandler>(continuationManager, ability);
        if (!continuationManager_->Init(shared_from_this(), GetToken(), GetAbilityInfo(), continuationHandler_)) {
            continuationManager_.reset();
        } else {
            std::weak_ptr<ContinuationHandler> continuationHandler = continuationHandler_;
            sptr<ReverseContinuationSchedulerPrimary> primary = sptr<ReverseContinuationSchedulerPrimary>(
                new (std::nothrow) ReverseContinuationSchedulerPrimary(continuationHandler, handler_));
            if (primary == nullptr) {
                HILOG_ERROR("Ability::Init failed,primary create failed");
            } else {
                continuationHandler_->SetPrimaryStub(primary);
                continuationHandler_->SetAbilityInfo(abilityInfo_);
            }
        }

        // register displayid change callback
        HILOG_DEBUG("Ability::Init call RegisterDisplayListener");
        abilityDisplayListener_ = new AbilityDisplayListener(ability);
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(abilityDisplayListener_);
    }
#endif
    lifecycle_ = std::make_shared<LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AbilityLifecycleExecutor>();
    if (abilityLifecycleExecutor_ != nullptr) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);
    } else {
        HILOG_ERROR("%{public}s abilityLifecycleExecutor_ make failed.", __func__);
    }

    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

void Ability::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
}

std::shared_ptr<Global::Resource::ResourceManager> Ability::GetResourceManager() const
{
    return AbilityContext::GetResourceManager();
}

bool Ability::IsUpdatingConfigurations()
{
    return AbilityContext::IsUpdatingConfigurations();
}

void Ability::OnStart(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::OnStart failed abilityInfo_ is nullptr.");
        return;
    }

    appIndex_ = want.GetIntParam(DLP_INDEX, 0);
    (const_cast<Want &>(want)).RemoveParam(DLP_INDEX);
    securityFlag_ = want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false);
    (const_cast<Want &>(want)).RemoveParam(DLP_PARAMS_SECURITY_FLAG);
    SetWant(want);
    HILOG_INFO("%{public}s begin, ability is %{public}s.", __func__, abilityInfo_->name.c_str());
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        int defualtDisplayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
        int displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        HILOG_DEBUG("abilityName:%{public}s, displayId:%{public}d", abilityInfo_->name.c_str(), displayId);
        auto option = GetWindowOption(want);
        InitWindow(displayId, option);

        if (abilityWindow_ != nullptr) {
            HILOG_DEBUG("%{public}s get window from abilityWindow.", __func__);
            auto window = abilityWindow_->GetWindow();
            if (window) {
                HILOG_DEBUG("Call RegisterDisplayMoveListener, windowId: %{public}d", window->GetWindowId());
                abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
                window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
            }
        }

        // Update resMgr, Configuration
        HILOG_DEBUG("%{public}s get display by displayId %{public}d.", __func__, displayId);
        auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
        if (display) {
            float density = display->GetVirtualPixelRatio();
            int32_t width = display->GetWidth();
            int32_t height = display->GetHeight();
            std::shared_ptr<Configuration> configuration = nullptr;
            if (application_) {
                configuration = application_->GetConfiguration();
            }
            if (configuration) {
                std::string direction = GetDirectionStr(height, width);
                configuration->AddItem(displayId, ConfigurationInner::APPLICATION_DIRECTION, direction);
                configuration->AddItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));
                configuration->AddItem(ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(displayId));
                UpdateContextConfiguration();
            }

            std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
            if (resConfig == nullptr) {
                HILOG_ERROR("%{public}s error, resConfig is nullptr.", __func__);
                return;
            }
            auto resourceManager = GetResourceManager();
            if (resourceManager != nullptr) {
                resourceManager->GetResConfig(*resConfig);
                resConfig->SetScreenDensity(density);
                resConfig->SetDirection(ConvertDirection(height, width));
                resourceManager->UpdateResConfig(*resConfig);
                HILOG_DEBUG("%{public}s Notify ResourceManager, Density: %{public}f, Direction: %{public}d.", __func__,
                    resConfig->GetScreenDensity(), resConfig->GetDirection());
            }
        }
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnStart error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (!abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);
    }

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnStart error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_START, want);
    HILOG_DEBUG("%{public}s end", __func__);
}

void Ability::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin", __func__);
#ifdef SUPPORT_GRAPHICS
    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleSaveAbilityState(StateReason::LIFECYCLE);
    }
    (void)Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(abilityDisplayListener_);
    auto && window = GetWindow();
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
        HILOG_ERROR("Ability::OnStop error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);
    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnStop error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_STOP);
    HILOG_DEBUG("%{public}s end", __func__);
}

void Ability::OnStop(AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    isAsyncCallback = false;
    OnStop();
}

void Ability::OnStopCallback()
{
}

void Ability::DestroyInstance()
{
    HILOG_DEBUG("%{public}s begin", __func__);
#ifdef SUPPORT_GRAPHICS
    // Release the window.
    if (abilityWindow_ != nullptr && abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        abilityWindow_->OnPostAbilityStop(); // Ability instance will been released when window destroy.
    }
#endif
    HILOG_DEBUG("%{public}s end", __func__);
}

void Ability::OnActive()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin.", __func__);
#ifdef SUPPORT_GRAPHICS
    bWindowFocus_ = true;
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnActive error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnActive error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.abilityType = static_cast<int32_t>(abilityInfo_->type);
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONACTIVE,
        HiSysEventType::BEHAVIOR, eventInfo);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void Ability::OnInactive()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin", __func__);
#ifdef SUPPORT_GRAPHICS
    bWindowFocus_ = false;
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnInactive error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnInactive error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_INACTIVE);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONINACTIVE,
        HiSysEventType::BEHAVIOR, eventInfo);
    HILOG_DEBUG("%{public}s end", __func__);
}

bool Ability::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is null");
        return false;
    }

    if (launchParam_.launchReason != LaunchReason::LAUNCHREASON_CONTINUATION) {
        HILOG_DEBUG("launchReason is %{public}d", launchParam_.launchReason);
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        HILOG_DEBUG("not Restored In Continuation");
        return false;
    }

    HILOG_DEBUG("Is Restored In Continuation");
    return true;
}

bool Ability::ShouldRecoverState(const Want& want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        HILOG_INFO("AppRecovery not recovery restart");
        return false;
    }

    if (abilityRecovery_ == nullptr) {
        HILOG_WARN("AppRecovery Not enable");
        return false;
    }

    if (abilityContext_ == nullptr) {
        HILOG_WARN("AppRecovery abilityContext_ is null");
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        HILOG_WARN("AppRecovery abilityContext_ GetContentStorage is null");
        return false;
    }

    return true;
}

void Ability::NotifyContinuationResult(const Want& want, bool success)
{
    HILOG_INFO("NotifyContinuationResult begin");

    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    HILOG_DEBUG("Ability::NotifyContinuationComplete");
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

sptr<IRemoteObject> Ability::OnConnect(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin", __func__);
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnConnect error. abilityLifecycleExecutor_ == nullptr.");
        return nullptr;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnConnect error. lifecycle_ == nullptr.");
        return nullptr;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    HILOG_DEBUG("%{public}s end", __func__);
    return nullptr;
}

void Ability::OnDisconnect(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s come", __func__);
}

ErrCode Ability::StartAbilityForResult(const Want &want, int requestCode)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::StartAbilityForResult abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    HILOG_DEBUG("Ability::StartAbilityForResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode);
    HILOG_DEBUG("%{public}s end.", __func__);
    return err;
}

ErrCode Ability::StartAbilityForResult(const Want &want, int requestCode, AbilityStartSetting abilityStartSetting)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::StartAbilityForResult abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    HILOG_DEBUG("Ability::StartAbilityForResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode, abilityStartSetting);
    HILOG_DEBUG("%{public}s end.", __func__);
    return err;
}

ErrCode Ability::StartAbility(const Want &want, AbilityStartSetting abilityStartSetting)
{
    HILOG_DEBUG("%{public}s beign.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::StartAbility abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    HILOG_DEBUG("Ability::StartAbility called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE && abilityInfo_->type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, -1, abilityStartSetting);
    HILOG_DEBUG("%{public}s end.", __func__);
    return err;
}

std::string Ability::GetType(const Uri &uri)
{
    return "";
}

int Ability::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    return 0;
}

std::shared_ptr<AppExecFwk::PacMap> Ability::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    return nullptr;
}

void Ability::OnConfigurationUpdated(const Configuration &configuration)
{
    HILOG_DEBUG("%{public}s called.", __func__);
}

void Ability::OnConfigurationUpdatedNotify(const Configuration &configuration)
{
    HILOG_DEBUG("%{public}s begin.", __func__);

    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    InitConfigurationProperties(configuration, language, colormode, hasPointerDevice);
    // Notify ResourceManager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("Create res config failed.");
        return;
    }
    auto resourceManager = GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
#ifdef SUPPORT_GRAPHICS
        if (!language.empty()) {
            UErrorCode status = U_ZERO_ERROR;
            icu::Locale locale = icu::Locale::forLanguageTag(language, status);
            HILOG_DEBUG("get Locale::forLanguageTag return[%{public}d].", static_cast<int>(status));
            if (status == U_ZERO_ERROR) {
                resConfig->SetLocaleInfo(locale);
            }
        }
#endif
        if (!colormode.empty()) {
            resConfig->SetColorMode(ConvertColorMode(colormode));
        }
        if (!hasPointerDevice.empty()) {
            resConfig->SetInputDevice(ConvertHasPointerDevice(hasPointerDevice));
        }
        resourceManager->UpdateResConfig(*resConfig);
        HILOG_INFO("Notify ResourceManager, current colorMode: %{public}d, hasPointerDevice: %{publis}d.",
            resConfig->GetColorMode(), resConfig->GetInputDevice());
    }

    if (abilityContext_ != nullptr && application_ != nullptr) {
        abilityContext_->SetConfiguration(application_->GetConfiguration());
    }
    // Notify Ability Subclass
    OnConfigurationUpdated(configuration);
    HILOG_DEBUG("%{public}s Notify Ability Subclass.", __func__);
}

void Ability::InitConfigurationProperties(const Configuration& changeConfiguration, std::string& language,
    std::string& colormode, std::string& hasPointerDevice)
{
    if (setting_) {
        auto displayId = std::atoi(setting_->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str());
        language = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        HILOG_INFO("displayId: [%{public}d], language: [%{public}s], colormode: [%{public}s], "
            "hasPointerDevice: [%{public}s]", displayId, language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    } else {
        language = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        HILOG_INFO("language: [%{public}s], colormode: [%{public}s], hasPointerDevice: [%{public}s]",
            language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    }
}

void Ability::OnMemoryLevel(int level)
{
    HILOG_INFO("%{public}s start.", __func__);
#ifdef SUPPORT_GRAPHICS
    if (scene_ == nullptr) {
        HILOG_DEBUG("WindowScene is null");
        return;
    }
    scene_->NotifyMemoryLevel(level);
#endif
}

int Ability::OpenRawFile(const Uri &uri, const std::string &mode)
{
    return -1;
}

int Ability::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    return 0;
}

std::shared_ptr<OHOSApplication> Ability::GetApplication()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (application_ == nullptr) {
        HILOG_ERROR("Ability::GetApplication error. application_ == nullptr.");
        return nullptr;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return application_;
}

std::string Ability::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::GetAbilityName abilityInfo_ is nullptr");
        return "";
    }

    return abilityInfo_->name;
}

bool Ability::IsTerminating()
{
    return false;
}

void Ability::OnAbilityResult(int requestCode, int resultCode, const Want &want)
{}

void Ability::OnBackPressed()
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::OnBackPressed abilityInfo_ is nullptr");
        return;
    }

    if (abilityInfo_->isLauncherAbility == false) {
        HILOG_DEBUG("Ability::OnBackPressed the ability is not Launcher");
        TerminateAbility();
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

void Ability::OnNewWant(const Want &want)
{
    HILOG_DEBUG("Ability::OnNewWant called");
}

void Ability::OnRestoreAbilityState(const PacMap &inState)
{
    HILOG_DEBUG("Ability::OnRestoreAbilityState called");
}

void Ability::OnSaveAbilityState(PacMap &outState)
{
    HILOG_DEBUG("Ability::OnSaveAbilityState called");
}

void Ability::OnEventDispatch()
{}

void Ability::SetWant(const AAFwk::Want &want)
{
    setWant_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> Ability::GetWant()
{
    return setWant_;
}

void Ability::SetResult(int resultCode, const Want &resultData)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::SetResult abilityInfo_ == nullptr");
        return;
    }
    HILOG_DEBUG("Ability::SetResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        AbilityContext::resultWant_ = resultData;
        AbilityContext::resultCode_ = resultCode;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

void Ability::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO(
        "%{public}s begin restart=%{public}s,startId=%{public}d.", __func__, restart ? "true" : "false", startId);
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnCommand error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnCommand error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    HILOG_DEBUG("%{public}s end.", __func__);
}

void Ability::Dump(const std::string &extra)
{
    HILOG_DEBUG("Ability::Dump called");
    // abilityInfo
    HILOG_DEBUG("==============AbilityInfo==============");
    if (abilityInfo_ != nullptr) {
        HILOG_DEBUG("abilityInfo: package: %{public}s", abilityInfo_->package.c_str());
        HILOG_DEBUG("abilityInfo: name: %{public}s", abilityInfo_->name.c_str());
        HILOG_DEBUG("abilityInfo: label: %{public}s", abilityInfo_->label.c_str());
        HILOG_DEBUG("abilityInfo: description: %{public}s", abilityInfo_->description.c_str());
        HILOG_DEBUG("abilityInfo: iconPath: %{public}s", abilityInfo_->iconPath.c_str());
        HILOG_DEBUG("abilityInfo: visible: %{public}d", abilityInfo_->visible);
        HILOG_DEBUG("abilityInfo: kind: %{public}s", abilityInfo_->kind.c_str());
        HILOG_DEBUG("abilityInfo: type: %{public}d", abilityInfo_->type);
        HILOG_DEBUG("abilityInfo: orientation: %{public}d", abilityInfo_->orientation);
        HILOG_DEBUG("abilityInfo: launchMode: %{public}d", abilityInfo_->launchMode);
        for (auto permission : abilityInfo_->permissions) {
            HILOG_DEBUG("abilityInfo: permission: %{public}s", permission.c_str());
        }
        HILOG_DEBUG("abilityInfo: bundleName: %{public}s", abilityInfo_->bundleName.c_str());
        HILOG_DEBUG("abilityInfo: applicationName: %{public}s", abilityInfo_->applicationName.c_str());
    } else {
        HILOG_DEBUG("abilityInfo is nullptr");
    }

    // lifecycle_Event
    HILOG_DEBUG("==============lifecycle_Event==============");
    if (lifecycle_ != nullptr) {
        HILOG_DEBUG("lifecycle_Event: launchMode: %{public}d", lifecycle_->GetLifecycleState());
    } else {
        HILOG_DEBUG("lifecycle is nullptr");
    }

    // lifecycle_State
    HILOG_DEBUG("==============lifecycle_State==============");
    if (abilityLifecycleExecutor_ != nullptr) {
        HILOG_DEBUG("lifecycle_State: launchMode: %{public}d", abilityLifecycleExecutor_->GetState());
    } else {
        HILOG_DEBUG("abilityLifecycleExecutor is nullptr");
    }

    // applicationInfo
    HILOG_DEBUG("==============applicationInfo==============");
    std::shared_ptr<ApplicationInfo> ApplicationInfoPtr = GetApplicationInfo();
    if (ApplicationInfoPtr != nullptr) {
        HILOG_DEBUG("applicationInfo: name: %{public}s", ApplicationInfoPtr->name.c_str());
        HILOG_DEBUG("applicationInfo: bundleName: %{public}s", ApplicationInfoPtr->bundleName.c_str());
    } else {
        HILOG_DEBUG("ApplicationInfoPtr is nullptr");
    }
}

void Ability::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{}

void Ability::KeepBackgroundRunning(int id, const NotificationRequest &notificationRequest)
{}

void Ability::CancelBackgroundRunning()
{}

Uri Ability::NormalizeUri(const Uri &uri)
{
    return uri;
}

int Ability::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    return 0;
}

std::vector<std::string> Ability::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    return types_;
}

int Ability::OpenFile(const Uri &uri, const std::string &mode)
{
    return -1;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> Ability::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    return nullptr;
}

bool Ability::Reload(const Uri &uri, const PacMap &extras)
{
    return false;
}

int Ability::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    int amount = 0;
    for (auto it = values.begin(); it != values.end(); it++) {
        if (Insert(uri, *it) >= 0) {
            amount++;
        }
    }
    HILOG_DEBUG("%{public}s end, amount=%{public}d", __func__, amount);
    return amount;
}

void Ability::ContinueAbilityReversibly(const std::string &deviceId)
{
    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("Ability::ContinueAbilityReversibly(deviceId) failed. VerifySupportForContinuation faled");
        return;
    }
    continuationManager_->ContinueAbility(true, deviceId);
}

std::string Ability::GetOriginalDeviceId()
{
    return "";
}

ContinuationState Ability::GetContinuationState()
{
    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("Ability::GetContinuationState failed. VerifySupportForContinuation faled");
        return ContinuationState::LOCAL_RUNNING;
    }
    return continuationManager_->GetContinuationState();
}

Uri Ability::DenormalizeUri(const Uri &uri)
{
    return uri;
}

std::shared_ptr<LifeCycle> Ability::GetLifecycle()
{
    HILOG_DEBUG("Ability::GetLifecycle called");
    return lifecycle_;
}

AbilityLifecycleExecutor::LifecycleState Ability::GetState()
{
    HILOG_DEBUG("Ability::GetState called");

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::GetState error. abilityLifecycleExecutor_ == nullptr.");
        return AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }

    return (AbilityLifecycleExecutor::LifecycleState)abilityLifecycleExecutor_->GetState();
}

ErrCode Ability::StartAbility(const Want &want)
{
    HILOG_DEBUG("%{public}s begin Ability::StartAbility", __func__);
    return AbilityContext::StartAbility(want, -1);
}

void Ability::PostTask(std::function<void()> task, long delayTime)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    TaskHandlerClient::GetInstance()->PostTask(task, delayTime);
    HILOG_DEBUG("%{public}s end.", __func__);
}

int32_t Ability::OnContinue(WantParams &wantParams)
{
    return ContinuationManager::OnContinueResult::REJECT;
}

void Ability::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    if (deviceId.empty()) {
        HILOG_ERROR("ContinueAbilityWithStack(deviceId) failed. deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("ContinueAbilityWithStack(deviceId) failed. VerifySupportForContinuation failed");
        return;
    }
    continuationManager_->ContinueAbilityWithStack(deviceId, versionCode);
}

void Ability::ContinueAbility(const std::string &deviceId)
{
    if (deviceId.empty()) {
        HILOG_ERROR("Ability::ContinueAbility(deviceId) failed. deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("Ability::ContinueAbility(deviceId) failed. VerifySupportForContinuation failed");
        return;
    }
    continuationManager_->ContinueAbility(false, deviceId);
}

bool Ability::OnStartContinuation()
{
    return false;
}

bool Ability::OnSaveData(WantParams &saveData)
{
    return false;
}

bool Ability::OnRestoreData(WantParams &restoreData)
{
    return false;
}

int32_t Ability::OnSaveState(int32_t reason, WantParams &wantParams)
{
    return 0;
}

void Ability::OnCompleteContinuation(int result)
{
    HILOG_DEBUG("Ability::OnCompleteContinuation change continuation state to initial");
    if (continuationManager_ == nullptr) {
        HILOG_ERROR("Continuation manager is nullptr.");
        return;
    }

    continuationManager_->ChangeProcessStateToInit();
}

void Ability::OnRemoteTerminated()
{}

void Ability::DispatchLifecycleOnForeground(const Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnForeground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (abilityInfo_ != nullptr && abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    }
    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnForeground error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_FOREGROUND, want);
}

bool Ability::VerifySupportForContinuation()
{
    if (continuationManager_ == nullptr) {
        HILOG_ERROR("Ability::VerifySupportForContinuation failed. continuationManager_ is nullptr");
        return false;
    }
    return true;
}

void Ability::HandleCreateAsContinuation(const Want &want)
{
    if (!IsFlagExists(Want::FLAG_ABILITY_CONTINUATION, want.GetFlags())) {
        HILOG_DEBUG("Ability::HandleCreateAsContinuation return. This not continuated ability");
        return;
    }

    // check whether it needs reversible
    bool reversible = false;
    reversible = IsFlagExists(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE, want.GetFlags());
    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("Ability::HandleCreateAsContinuation failed. VerifySupportForContinuation failed");
        return;
    }
    bool success = continuationManager_->RestoreData(
        want.GetParams(), reversible, want.GetStringParam(ContinuationHandler::ORIGINAL_DEVICE_ID));
    if (success && reversible) {
        // Register this ability to receive reverse continuation callback.
        std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> ReplicaHandler = continuationHandler_;
        reverseContinuationSchedulerReplica_ = sptr<ReverseContinuationSchedulerReplica>(
            new (std::nothrow) ReverseContinuationSchedulerReplica(handler_, ReplicaHandler));

        if (reverseContinuationSchedulerReplica_ == nullptr) {
            HILOG_ERROR(
                "Ability::HandleCreateAsContinuation failed, create reverseContinuationSchedulerReplica failed");
            return;
        }
    }

    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    HILOG_DEBUG("Ability::HandleCreateAsContinuation");
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

void Ability::HandleCreateAsRecovery(const Want &want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        HILOG_ERROR("AppRecovery not recovery restart");
        return;
    }

    if (abilityRecovery_ != nullptr) {
        abilityRecovery_->ScheduleRestoreAbilityState(StateReason::DEVELOPER_REQUEST, want);
    }
}

bool Ability::IsFlagExists(unsigned int flag, unsigned int flagSet)
{
    return (flag & flagSet) == flag;
}

Uri Ability::OnSetCaller()
{
    return Uri("");
}

std::shared_ptr<AbilityPostEventTimeout> Ability::CreatePostEventTimeouter(std::string taskstr)
{
    return std::make_shared<AbilityPostEventTimeout>(taskstr, handler_);
}

int Ability::StartBackgroundRunning(const AbilityRuntime::WantAgent::WantAgent &wantAgent)
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    auto bundleMgr = GetBundleMgr();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("Ability::GetBundleMgr failed");
        return ERR_NULL_OBJECT;
    }
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("ability info is null");
        return ERR_INVALID_VALUE;
    }
    Want want;
    want.SetAction("action.system.home");
    want.AddEntity("entity.system.home");
    want.SetElementName("", abilityInfo_->bundleName, "", "");
    AppExecFwk::AbilityInfo abilityInfo;
    bundleMgr->QueryAbilityInfo(want, abilityInfo);
    std::string appName = bundleMgr->GetAbilityLabel(abilityInfo_->bundleName, abilityInfo.name);
    uint32_t defaultBgMode = 0;
    BackgroundTaskMgr::ContinuousTaskParam taskParam = BackgroundTaskMgr::ContinuousTaskParam(false, defaultBgMode,
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>(wantAgent), abilityInfo_->name, GetToken(), appName);
    return BackgroundTaskMgr::BackgroundTaskMgrHelper::RequestStartBackgroundRunning(taskParam);
#else
    return ERR_INVALID_OPERATION;
#endif
}

int Ability::StopBackgroundRunning()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    return BackgroundTaskMgr::BackgroundTaskMgrHelper::RequestStopBackgroundRunning(abilityInfo_->name, GetToken());
#else
    return ERR_INVALID_OPERATION;
#endif
}

sptr<IBundleMgr> Ability::GetBundleMgr()
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (iBundleMgr_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject == nullptr) {
            HILOG_ERROR("%{public}s error, failed to get bundle manager service.", __func__);
            return nullptr;
        }

        iBundleMgr_ = iface_cast<IBundleMgr>(remoteObject);
        if (iBundleMgr_ == nullptr) {
            HILOG_ERROR("%{public}s error, failed to get bundle manager service", __func__);
            return nullptr;
        }
    }

    return iBundleMgr_;
}

void Ability::SetBundleManager(const sptr<IBundleMgr> &bundleManager)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    iBundleMgr_ = bundleManager;
}

void Ability::SetStartAbilitySetting(std::shared_ptr<AbilityStartSetting> setting)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    setting_ = setting;
}

void Ability::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam& Ability::GetLaunchParam() const
{
    return launchParam_;
}

std::vector<std::shared_ptr<DataAbilityResult>> Ability::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_DEBUG("start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return results;
    }
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("data ability type failed, current type: %{public}d", abilityInfo_->type);
        return results;
    }
    size_t len = operations.size();
    HILOG_DEBUG("operation is nullptr, len %{public}zu", len);
    for (size_t i = 0; i < len; i++) {
        std::shared_ptr<DataAbilityOperation> operation = operations[i];
        if (operation == nullptr) {
            HILOG_DEBUG("operation is nullptr, create DataAbilityResult");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            continue;
        }
        ExecuteOperation(operation, results, i);
    }
    HILOG_DEBUG("end, %{public}zu", results.size());
    return results;
}
void Ability::ExecuteOperation(std::shared_ptr<DataAbilityOperation> &operation,
    std::vector<std::shared_ptr<DataAbilityResult>> &results, int index)
{
    HILOG_DEBUG("exec start, index=%{public}d", index);
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("exec data ability type failed, current type: %{public}d", abilityInfo_->type);
        return;
    }
    if (index < 0) {
        HILOG_ERROR(
            "exec operation result index should not below zero, current index: %{public}d", index);
        return;
    }
    if (operation == nullptr) {
        HILOG_WARN("exec operation is nullptr, create DataAbilityResult");
        results.push_back(std::make_shared<DataAbilityResult>(0));
        return;
    }

    int numRows = 0;
    std::shared_ptr<NativeRdb::ValuesBucket> valuesBucket = ParseValuesBucketReference(results, operation, index);
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates =
        ParsePredictionArgsReference(results, operation, index);
    if (operation->IsInsertOperation()) {
        HILOG_DEBUG("exec IsInsertOperation");
        numRows = Insert(*(operation->GetUri().get()), *valuesBucket);
    } else if (operation->IsDeleteOperation() && predicates) {
        HILOG_DEBUG("exec IsDeleteOperation");
        numRows = Delete(*(operation->GetUri().get()), *predicates);
    } else if (operation->IsUpdateOperation() && predicates) {
        HILOG_DEBUG("exec IsUpdateOperation");
        numRows = Update(*(operation->GetUri().get()), *valuesBucket, *predicates);
    } else if (operation->IsAssertOperation() && predicates) {
        HILOG_DEBUG("exec IsAssertOperation");
        std::vector<std::string> columns;
        std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult =
            Query(*(operation->GetUri().get()), columns, *predicates);
        if (queryResult == nullptr) {
            HILOG_ERROR("exec Query retval is nullptr");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            return;
        }
        if (queryResult->GetRowCount(numRows) != 0) {
            HILOG_ERROR("exec queryResult->GetRowCount(numRows) != E_OK");
        }
        if (!CheckAssertQueryResult(queryResult, operation->GetValuesBucket())) {
            if (queryResult != nullptr) {
                queryResult->Close();
            }
            HILOG_ERROR("Query Result is not equal to expected value.");
        }

        if (queryResult != nullptr) {
            queryResult->Close();
        }
    } else {
        HILOG_ERROR("exec Expected bad type %{public}d", operation->GetType());
    }
    if (operation->GetExpectedCount() != numRows) {
        HILOG_ERROR("exec Expected %{public}d rows but actual %{public}d",
            operation->GetExpectedCount(),
            numRows);
    } else {
        if (operation->GetUri() != nullptr) {
            results.push_back(std::make_shared<DataAbilityResult>(*operation->GetUri(), numRows));
        } else {
            results.push_back(std::make_shared<DataAbilityResult>(Uri(std::string("")), numRows));
        }
    }
}

std::shared_ptr<NativeRdb::DataAbilityPredicates> Ability::ParsePredictionArgsReference(
    std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
    int numRefs)
{
    if (operation == nullptr) {
        HILOG_ERROR("intput is nullptr");
        return nullptr;
    }

    std::map<int, int> predicatesBackReferencesMap = operation->GetDataAbilityPredicatesBackReferences();
    if (predicatesBackReferencesMap.empty()) {
        return operation->GetDataAbilityPredicates();
    }

    std::vector<std::string> strPredicatesList;
    strPredicatesList.clear();
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates = operation->GetDataAbilityPredicates();
    if (predicates == nullptr) {
        HILOG_DEBUG("operation->GetDataAbilityPredicates is nullptr");
    } else {
        HILOG_DEBUG("operation->GetDataAbilityPredicates isn`t nullptr");
        strPredicatesList = predicates->GetWhereArgs();
    }

    if (strPredicatesList.empty()) {
        HILOG_ERROR("operation->GetDataAbilityPredicates()->GetWhereArgs()"
                 "error strList is empty()");
    }

    for (auto iterMap : predicatesBackReferencesMap) {
        HILOG_DEBUG("predicatesBackReferencesMap first:%{public}d second:%{public}d",
            iterMap.first,
            iterMap.second);
        int tempCount = ChangeRef2Value(results, numRefs, iterMap.second);
        if (tempCount < 0) {
            HILOG_ERROR("tempCount:%{public}d", tempCount);
            continue;
        }
        std::string strPredicates = std::to_string(tempCount);
        strPredicatesList.push_back(strPredicates);
    }

    if (predicates) {
        predicates->SetWhereArgs(strPredicatesList);
    }

    return predicates;
}

std::shared_ptr<NativeRdb::ValuesBucket> Ability::ParseValuesBucketReference(
    std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
    int numRefs)
{
    NativeRdb::ValuesBucket retValueBucket;
    if (operation == nullptr) {
        HILOG_ERROR("intput is nullptr");
        return nullptr;
    }

    if (operation->GetValuesBucketReferences() == nullptr) {
        return operation->GetValuesBucket();
    }

    retValueBucket.Clear();
    if (operation->GetValuesBucket() == nullptr) {
        HILOG_DEBUG("operation->GetValuesBucket is nullptr");
    } else {
        HILOG_DEBUG("operation->GetValuesBucket is nullptr");
        retValueBucket = *operation->GetValuesBucket();
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMapReferences;
    operation->GetValuesBucketReferences()->GetAll(valuesMapReferences);

    for (auto itermap : valuesMapReferences) {
        std::string key = itermap.first;
        NativeRdb::ValueObject obj;
        if (!operation->GetValuesBucketReferences()->GetObject(key, obj)) {
            HILOG_ERROR("operation->GetValuesBucketReferences()->GetObject error");
            continue;
        }
        switch (obj.GetType()) {
            case NativeRdb::ValueObjectType::TYPE_INT: {
                int val = 0;
                if (obj.GetInt(val) != 0) {
                    HILOG_ERROR("ValueObject->GetInt() error");
                    break;
                }
                HILOG_DEBUG("retValueBucket->PutInt(%{public}s, %{public}d)",
                    key.c_str(),
                    val);
                retValueBucket.PutInt(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_DOUBLE: {
                double val = 0.0;
                if (obj.GetDouble(val) != 0) {
                    HILOG_ERROR("ValueObject->GetDouble() error");
                    break;
                }
                HILOG_DEBUG("retValueBucket->PutDouble(%{public}s, %{public}f)",
                    key.c_str(),
                    val);
                retValueBucket.PutDouble(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_STRING: {
                std::string val = "";
                if (obj.GetString(val) != 0) {
                    HILOG_ERROR("ValueObject->GetString() error");
                    break;
                }
                HILOG_DEBUG("retValueBucket->PutString(%{public}s, %{public}s)",
                    key.c_str(),
                    val.c_str());
                retValueBucket.PutString(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_BLOB: {
                std::vector<uint8_t> val;
                if (obj.GetBlob(val) != 0) {
                    HILOG_ERROR("ValueObject->GetBlob() error");
                    break;
                }
                HILOG_DEBUG("retValueBucket->PutBlob(%{public}s, %{public}zu)",
                    key.c_str(),
                    val.size());
                retValueBucket.PutBlob(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_BOOL: {
                bool val = false;
                if (obj.GetBool(val) != 0) {
                    HILOG_ERROR("ValueObject->GetBool() error");
                    break;
                }
                HILOG_DEBUG("retValueBucket->PutBool(%{public}s, %{public}s)",
                    key.c_str(),
                    val ? "true" : "false");
                retValueBucket.PutBool(key, val);
            } break;
            default: {
                HILOG_DEBUG("retValueBucket->PutNull(%{public}s)", key.c_str());
                retValueBucket.PutNull(key);
            } break;
        }
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    retValueBucket.GetAll(valuesMap);

    return std::make_shared<NativeRdb::ValuesBucket>(valuesMap);
}

int Ability::ChangeRef2Value(std::vector<std::shared_ptr<DataAbilityResult>> &results, int numRefs, int index)
{
    int retval = -1;
    if (index >= numRefs) {
        HILOG_ERROR("Ability::ChangeRef2Value index >= numRefs");
        return retval;
    }

    if (index >= static_cast<int>(results.size())) {
        HILOG_ERROR("Ability::ChangeRef2Value index:%{public}d >= results.size():%{public}zu", index, results.size());
        return retval;
    }

    std::shared_ptr<DataAbilityResult> refResult = results[index];
    if (refResult == nullptr) {
        HILOG_ERROR("Ability::ChangeRef2Value No.%{public}d refResult is null", index);
        return retval;
    }

    if (refResult->GetUri().ToString().empty()) {
        retval = refResult->GetCount();
    } else {
        retval = DataUriUtils::GetId(refResult->GetUri());
    }

    return retval;
}

bool Ability::CheckAssertQueryResult(std::shared_ptr<NativeRdb::AbsSharedResultSet> &queryResult,
    std::shared_ptr<NativeRdb::ValuesBucket> &&valuesBucket)
{
    if (queryResult == nullptr) {
        HILOG_ERROR("Ability::CheckAssertQueryResult intput queryResult is null");
        return true;
    }

    if (valuesBucket == nullptr) {
        HILOG_ERROR("Ability::CheckAssertQueryResult intput valuesBucket is null");
        return true;
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    valuesBucket->GetAll(valuesMap);
    if (valuesMap.empty()) {
        HILOG_ERROR("Ability::CheckAssertQueryResult valuesMap is empty");
        return true;
    }
    int count = 0;
    if (queryResult->GetRowCount(count) != 0) {
        HILOG_ERROR("Ability::CheckAssertQueryResult GetRowCount is 0");
        return true;
    }

    for (auto iterMap : valuesMap) {
        std::string strObject;
        if (iterMap.second.GetString(strObject) != 0) {
            HILOG_ERROR("Ability::CheckAssertQueryResult GetString strObject is error");
            continue;
        }
        if (strObject.empty()) {
            HILOG_ERROR("Ability::CheckAssertQueryResult strObject is empty");
            continue;
        }
        for (int i = 0; i < count; ++i) {
            std::string strName;
            if (queryResult->GetString(i, strName) != 0) {
                HILOG_ERROR("Ability::CheckAssertQueryResult GetString strName is error");
                continue;
            }
            if (strName.empty()) {
                HILOG_ERROR("Ability::CheckAssertQueryResult strName is empty");
                continue;
            }
            if (strName == strObject) {
                HILOG_ERROR("Ability::CheckAssertQueryResult strName same to strObject");
                continue;
            }

            return false;
        }
    }

    return true;
}

sptr<IRemoteObject> Ability::CallRequest()
{
    return nullptr;
}

ErrCode Ability::StartFeatureAbilityForResult(const Want &want, int requestCode, FeatureAbilityTask &&task)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = StartAbilityForResult(want, requestCode);
    HILOG_DEBUG("%{public}s end. ret=%{public}d", __func__, err);
    return err;
}

void Ability::OnFeatureAbilityResult(int requestCode, int resultCode, const Want &want)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, want);
        }
        resultCallbacks_.erase(requestCode);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}

bool Ability::IsUseNewStartUpRule()
{
    if (!isNewRuleFlagSetted_ && setWant_) {
        startUpNewRule_ = setWant_->GetBoolParam(COMPONENT_STARTUP_NEW_RULES, false);
        isNewRuleFlagSetted_ = true;
    }
    return startUpNewRule_;
}

void Ability::EnableAbilityRecovery(const std::shared_ptr<AbilityRecovery>& abilityRecovery)
{
    abilityRecovery_ = abilityRecovery;
}

#ifdef SUPPORT_GRAPHICS
bool Ability::PrintDrawnCompleted()
{
    return AbilityContext::PrintDrawnCompleted();
}

void Ability::OnSceneCreated()
{
    HILOG_DEBUG("%{public}s called.", __func__);
}

void Ability::OnSceneRestored()
{
    HILOG_DEBUG("%{public}s called.", __func__);
}

void Ability::onSceneDestroyed()
{
    HILOG_DEBUG("%{public}s called.", __func__);
}

void Ability::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin.", __func__);
    DoOnForeground(want);
    DispatchLifecycleOnForeground(want);
    HILOG_DEBUG("%{public}s end.", __func__);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONFOREGROUND,
        HiSysEventType::BEHAVIOR, eventInfo);
}

void Ability::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo_ is nullptr.");
        return;
    }
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        if (abilityInfo_->isStageBasedModel) {
            if (scene_ == nullptr) {
                HILOG_ERROR("Ability::OnBackground error. scene_ == nullptr.");
                return;
            }
            HILOG_DEBUG("GoBackground sceneFlag:%{public}d.", sceneFlag_);
            scene_->GoBackground(sceneFlag_);
            if (abilityRecovery_ != nullptr) {
                abilityRecovery_->ScheduleSaveAbilityState(StateReason::LIFECYCLE);
            }
        } else {
            if (abilityWindow_ == nullptr) {
                HILOG_ERROR("Ability::OnBackground error. abilityWindow_ == nullptr.");
                return;
            }
            HILOG_DEBUG("OnPostAbilityBackground sceneFlag:%{public}d.", sceneFlag_);
            abilityWindow_->OnPostAbilityBackground(sceneFlag_);
        }
    }

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnBackground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }

    if (abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND);
    }

    if (lifecycle_ == nullptr) {
        HILOG_ERROR("Ability::OnBackground error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_BACKGROUND);
    HILOG_DEBUG("%{public}s end", __func__);
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONBACKGROUND,
        HiSysEventType::BEHAVIOR, eventInfo);
}

void Ability::OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    HILOG_DEBUG("Ability::OnKeyDown called");
}

void Ability::OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    HILOG_DEBUG("Ability::OnKeyUp called");
    auto code = keyEvent->GetKeyCode();
    if (code == MMI::KeyEvent::KEYCODE_BACK) {
        HILOG_DEBUG("Ability::OnKey Back key pressed.");
        OnBackPressed();
    }
}

void Ability::OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    HILOG_DEBUG("Ability::OnTouchEvent called");
}

void Ability::InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option)
{
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("Ability::InitWindow abilityWindow_ is nullptr");
        return;
    }
    abilityWindow_->InitWindow(abilityContext_, sceneListener_, displayId, option, securityFlag_);
}

const sptr<Rosen::Window> Ability::GetWindow()
{
    if (abilityWindow_ != nullptr) {
        return abilityWindow_->GetWindow();
    } else {
        HILOG_DEBUG("%{public}s abilityWindow_ is nullptr.", __func__);
        return nullptr;
    }
}

std::shared_ptr<Rosen::WindowScene> Ability::GetScene()
{
    return scene_;
}

bool Ability::HasWindowFocus()
{
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::HasWindowFocus abilityInfo_ == nullptr");
        return false;
    }

    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        return bWindowFocus_;
    }

    return false;
}

void Ability::SetShowOnLockScreen(bool showOnLockScreen)
{
    HILOG_DEBUG("SetShowOnLockScreen come, showOnLockScreen is %{public}d", showOnLockScreen);
    showOnLockScreen_ = showOnLockScreen;
    sptr<Rosen::Window> window = nullptr;
    if (abilityWindow_ == nullptr || (window = abilityWindow_->GetWindow()) == nullptr) {
        HILOG_ERROR("SetShowOnLockScreen come, window is null");
        return;
    }
    HILOG_DEBUG("SetShowOnLockScreen come, addWindowFlag, showOnLockScreen is %{public}d", showOnLockScreen);
    if (showOnLockScreen) {
        window->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    } else {
        window->RemoveWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }
}

void Ability::OnLeaveForeground()
{}

void Ability::SetVolumeTypeAdjustedByKey(int volumeType)
{}

int Ability::SetWindowBackgroundColor(int red, int green, int blue)
{
    return -1;
}

std::string Ability::GetContentInfo()
{
    if (scene_ == nullptr) {
        return "";
    }
    return scene_->GetContentInfo();
}

void Ability::OnWindowFocusChanged(bool hasFocus)
{}

void Ability::OnTopActiveAbilityChanged(bool topActive)
{}

FormProviderInfo Ability::OnCreate(const Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    FormProviderInfo formProviderInfo;
    return formProviderInfo;
}

bool Ability::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    return false;
}

void Ability::OnDelete(const int64_t formId)
{}

void Ability::OnUpdate(const int64_t formId)
{}

void Ability::OnCastTemptoNormal(const int64_t formId)
{}

void Ability::OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap)
{}

void Ability::OnTriggerEvent(const int64_t formId, const std::string &message)
{}

FormState Ability::OnAcquireFormState(const Want &want)
{
    return FormState::DEFAULT;
}

sptr<IRemoteObject> Ability::GetFormRemoteObject()
{
    HILOG_DEBUG("%{public}s start", __func__);
    if (providerRemoteObject_ == nullptr) {
        sptr<FormProviderClient> providerClient = new (std::nothrow) FormProviderClient();
        std::shared_ptr<Ability> thisAbility = this->shared_from_this();
        if (thisAbility == nullptr) {
            HILOG_ERROR("%{public}s failed, thisAbility is nullptr", __func__);
        }
        providerClient->SetOwner(thisAbility);
        providerRemoteObject_ = providerClient->AsObject();
    }
    HILOG_DEBUG("%{public}s end", __func__);
    return providerRemoteObject_;
}

void Ability::SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    sceneListener_ = listener;
}

sptr<Rosen::WindowOption> Ability::GetWindowOption(const Want &want)
{
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    if (option == nullptr) {
        HILOG_ERROR("Ability::GetWindowOption option is null.");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    HILOG_DEBUG("Ability::GetWindowOption window mode is %{public}d.", windowMode);
    option->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
    bool showOnLockScreen = false;
    if (abilityInfo_) {
        std::vector<CustomizeData> datas = abilityInfo_->metaData.customizeData;
        for (CustomizeData data : datas) {
            if (data.name == SHOW_ON_LOCK_SCREEN) {
                showOnLockScreen = true;
            }
        }
    }
    if (showOnLockScreen_ || showOnLockScreen) {
        HILOG_DEBUG("Ability::GetWindowOption come, add window flag WINDOW_FLAG_SHOW_WHEN_LOCKED.");
        option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        HILOG_DEBUG("Set window type for launcher");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    return option;
}

void Ability::DoOnForeground(const Want& want)
{
    if (abilityWindow_ != nullptr) {
        HILOG_DEBUG("%{public}s begin abilityWindow_->OnPostAbilityForeground, sceneFlag:%{public}d.",
            __func__, sceneFlag_);
        auto window = abilityWindow_->GetWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
                AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            HILOG_DEBUG("set window mode = %{public}d.", windowMode);
        }
        abilityWindow_->OnPostAbilityForeground(sceneFlag_);
        HILOG_DEBUG("%{public}s end abilityWindow_->OnPostAbilityForeground.", __func__);
    } else {
        HILOG_DEBUG("abilityWindow_ != nullptr");
    }
}

int Ability::GetCurrentWindowMode()
{
    HILOG_DEBUG("%{public}s start", __func__);
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

ErrCode Ability::SetMissionLabel(const std::string &label)
{
    HILOG_DEBUG("%{public}s start", __func__);
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("invalid ability info.");
        return -1;
    }

    // stage mode
    if (abilityInfo_->isStageBasedModel) {
        if (scene_ == nullptr) {
            HILOG_ERROR("get window scene failed.");
            return -1;
        }
        auto window = scene_->GetMainWindow();
        if (window == nullptr) {
            HILOG_ERROR("get window scene failed.");
            return -1;
        }

        if (window->SetAPPWindowLabel(label) != OHOS::Rosen::WMError::WM_OK) {
            HILOG_ERROR("SetAPPWindowLabel failed.");
            return -1;
        }
        return ERR_OK;
    }

    // fa mode
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("abilityWindow is nullptr.");
        return -1;
    }
    return abilityWindow_->SetMissionLabel(label);
}

ErrCode Ability::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    HILOG_DEBUG("%{public}s start", __func__);
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("invalid ability info, can not set mission icon.");
        return -1;
    }

    // stage mode
    if (abilityInfo_->isStageBasedModel) {
        if (scene_ == nullptr) {
            HILOG_ERROR("get window scene failed, can not set mission icon.");
            return -1;
        }
        auto window = scene_->GetMainWindow();
        if (window == nullptr) {
            HILOG_ERROR("get window scene failed, can not set mission icon.");
            return -1;
        }

        if (window->SetAPPWindowIcon(icon) != OHOS::Rosen::WMError::WM_OK) {
            HILOG_ERROR("SetAPPWindowIcon failed.");
            return -1;
        }
        return ERR_OK;
    }

    // fa mode
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("abilityWindow is nullptr, can not set mission icon.");
        return -1;
    }
    return abilityWindow_->SetMissionIcon(icon);
}

void Ability::OnCreate(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("%{public}s called.", __func__);
}

void Ability::OnDestroy(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("%{public}s called.", __func__);
}

void Ability::OnChange(Rosen::DisplayId displayId)
{
    HILOG_DEBUG("%{public}s start, displayId: %{public}" PRIu64"", __func__,
        displayId);

    // Get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        HILOG_ERROR("Get display by displayId %{public}" PRIu64" failed.", displayId);
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
            resConfig->SetDirection(ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            HILOG_INFO("%{public}s Notify ResourceManager, Density: %{public}f, Direction: %{public}d.", __func__,
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    // Notify ability
    Configuration newConfig;
    newConfig.AddItem(displayId, ConfigurationInner::APPLICATION_DIRECTION, GetDirectionStr(height, width));
    newConfig.AddItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));

    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr.");
        return;
    }

    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        HILOG_ERROR("configuration is nullptr.");
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
    }

    HILOG_DEBUG("%{public}s end", __func__);
}

void Ability::OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to)
{
    HILOG_INFO("%{public}s called, from displayId %{public}" PRIu64" to %{public}" PRIu64".", __func__, from, to);

    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(to);
    if (!display) {
        HILOG_ERROR("Get display by displayId %{public}" PRIu64" failed.", to);
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
            resConfig->SetDirection(ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            HILOG_INFO("%{public}s Notify ResourceManager, Density: %{public}f, Direction: %{public}d.", __func__,
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    Configuration newConfig;
    newConfig.AddItem(ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(to));
    newConfig.AddItem(to, ConfigurationInner::APPLICATION_DIRECTION, GetDirectionStr(height, width));
    newConfig.AddItem(to, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));

    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr.");
        return;
    }

    std::vector<std::string> changeKeyV;
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        HILOG_ERROR("configuration is nullptr.");
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

void Ability::RequestFocus(const Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (abilityWindow_ == nullptr) {
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        HILOG_DEBUG("set window mode = %{public}d.", windowMode);
    }
    abilityWindow_->OnPostAbilityForeground(sceneFlag_);
}

void Ability::SetWakeUpScreen(bool wakeUp)
{
    HILOG_DEBUG("FA mode::SetWakeUpScreen wakeUp:%{public}d.", wakeUp);
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("SetWakeUpScreen error. abilityWindow_ == nullptr.");
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        HILOG_ERROR("window nullptr.");
        return;
    }
    window->SetTurnScreenOn(wakeUp);
}

void Ability::SetDisplayOrientation(int orientation)
{
    HILOG_DEBUG("FA mode::%{public}s called, orientation: %{public}d", __func__, orientation);
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("Ability::SetDisplayOrientation error. abilityWindow_ == nullptr.");
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr.");
        return;
    }
    if (orientation == static_cast<int>(DisplayOrientation::FOLLOWRECENT)) {
        int defaultOrientation = 0;
        if (setWant_) {
            orientation = setWant_->GetIntParam("ohos.aafwk.Orientation", defaultOrientation);
        } else {
            orientation = defaultOrientation;
        }
    }
    if (orientation == static_cast<int>(DisplayOrientation::LANDSCAPE)) {
        HILOG_DEBUG("%{public}s, to set LANDSCAPE", __func__);
        window->SetRequestedOrientation(Rosen::Orientation::HORIZONTAL);
    } else if (orientation == static_cast<int>(DisplayOrientation::PORTRAIT)) {
        HILOG_DEBUG("%{public}s, to set PORTRAIT", __func__);
        window->SetRequestedOrientation(Rosen::Orientation::VERTICAL);
    } else {
        HILOG_DEBUG("%{public}s, to set UNSPECIFIED", __func__);
        window->SetRequestedOrientation(Rosen::Orientation::UNSPECIFIED);
    }
}

int Ability::GetDisplayOrientation()
{
    HILOG_DEBUG("%{public}s called.", __func__);
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("Ability::GetDisplayOrientation error. abilityWindow_ == nullptr.");
        return -1;
    }
    HILOG_DEBUG("FA mode");
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr.");
        return -1;
    }
    auto orientation = window->GetRequestedOrientation();
    if (orientation == Rosen::Orientation::HORIZONTAL) {
        HILOG_DEBUG("%{public}s, get window orientation: LANDSCAPE", __func__);
        return static_cast<int>(DisplayOrientation::LANDSCAPE);
    }
    if (orientation == Rosen::Orientation::VERTICAL) {
        HILOG_DEBUG("%{public}s, get window orientation: PORTRAIT", __func__);
        return static_cast<int>(DisplayOrientation::PORTRAIT);
    }
    HILOG_DEBUG("%{public}s, get window orientation: UNSPECIFIED", __func__);
    return 0;
}

void Ability::ContinuationRestore(const Want &want)
{
    HILOG_DEBUG("%{public}s called.", __func__);
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
