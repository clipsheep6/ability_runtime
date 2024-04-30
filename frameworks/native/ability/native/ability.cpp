/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "ability_runtime/js_ability.h"
#include "abs_shared_result_set.h"
#include "bundle_mgr_helper.h"
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
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "ohos_application.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_replica.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "runtime.h"
#include "scene_board_judgement.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "values_bucket.h"

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_mgr_helper.h"
#include "continuous_task_param.h"
#endif

#ifdef SUPPORT_GRAPHICS
#include "display_type.h"
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
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
                TAG_LOGE(AAFwkTag::ABILITY, "Create primary failed");
            } else {
                continuationHandler_->SetPrimaryStub(primary);
                continuationHandler_->SetAbilityInfo(abilityInfo_);
            }
        }

        // register displayid change callback
        TAG_LOGD(AAFwkTag::ABILITY, "Start RegisterDisplayListener");
        abilityDisplayListener_ = new AbilityDisplayListener(ability);
        Rosen::DisplayManager::GetInstance().RegisterDisplayListener(abilityDisplayListener_);
    }
#endif
    lifecycle_ = std::make_shared<LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AbilityLifecycleExecutor>();
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);

    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
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

void Ability::OnStart(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::OnStart failed abilityInfo_ is nullptr.");
        return;
    }

    appIndex_ = want.GetIntParam(DLP_INDEX, 0);
    (const_cast<Want &>(want)).RemoveParam(DLP_INDEX);
    securityFlag_ = want.GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false);
    (const_cast<Want &>(want)).RemoveParam(DLP_PARAMS_SECURITY_FLAG);
    SetWant(want);
    if (sessionInfo != nullptr) {
        SetSessionToken(sessionInfo->sessionToken);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "AbilityName is %{public}s.", abilityInfo_->name.c_str());
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        int32_t defualtDisplayId = static_cast<int32_t>(Rosen::DisplayManager::GetInstance().GetDefaultDisplayId());
        int32_t displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        TAG_LOGD(AAFwkTag::ABILITY, "abilityName:%{public}s, displayId:%{public}d",
            abilityInfo_->name.c_str(), displayId);
        InitFAWindow(want, displayId);

        if (!UpdateResMgrAndConfiguration(displayId)) {
            return;
        }
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    if (!abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);
    }

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_START, want);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
#ifdef SUPPORT_GRAPHICS
    (void)Rosen::DisplayManager::GetInstance().UnregisterDisplayListener(abilityDisplayListener_);
    auto && window = GetWindow();
    if (window != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "Call UnregisterDisplayMoveListener");
        window->UnregisterDisplayMoveListener(abilityDisplayMoveListener_);
    }
    // Call JS Func(onWindowStageDestroy) and Release the scene.
    if (scene_ != nullptr) {
        scene_->GoDestroy();
        onSceneDestroyed();
    }
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_STOP);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
#ifdef SUPPORT_GRAPHICS
    // Release the window.
    if (abilityWindow_ != nullptr && abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        abilityWindow_->OnPostAbilityStop(); // Ability instance will been released when window destroy.
    }
#endif
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::OnActive()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
#ifdef SUPPORT_GRAPHICS
    bWindowFocus_ = true;
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is null.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is null.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is null.");
        return;
    }
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.abilityType = static_cast<int32_t>(abilityInfo_->type);
    eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    if (setWant_ != nullptr) {
        eventInfo.callerBundleName = setWant_->GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
        TAG_LOGI(AAFwkTag::ABILITY, "callerBundleName is %{public}s", eventInfo.callerBundleName.c_str());
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "setWant_ is nullptr, can not get callerBundleName.");
    }
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONACTIVE,
        HiSysEventType::BEHAVIOR, eventInfo);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::OnInactive()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
#ifdef SUPPORT_GRAPHICS
    bWindowFocus_ = false;
#endif
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_INACTIVE);
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr.");
        return;
    }
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONINACTIVE,
        HiSysEventType::BEHAVIOR, eventInfo);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

bool Ability::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityContext_ is null");
        return false;
    }

    if (launchParam_.launchReason != LaunchReason::LAUNCHREASON_CONTINUATION) {
        TAG_LOGD(AAFwkTag::ABILITY, "launchReason is %{public}d", launchParam_.launchReason);
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "not Restored In Continuation");
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITY, "Is Restored In Continuation");
    return true;
}

bool Ability::ShouldRecoverState(const Want& want)
{
    if (!want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false)) {
        TAG_LOGI(AAFwkTag::ABILITY, "AppRecovery not recovery restart");
        return false;
    }

    if (abilityContext_ == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "AppRecovery abilityContext_ is null");
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "AppRecovery abilityContext_ GetContentStorage is null");
        return false;
    }

    return true;
}

void Ability::NotifyContinuationResult(const Want& want, bool success)
{
    TAG_LOGI(AAFwkTag::ABILITY, "called");

    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    TAG_LOGD(AAFwkTag::ABILITY, "Notify complete continuation.");
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

sptr<IRemoteObject> Ability::OnConnect(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return nullptr;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is nullptr.");
        return nullptr;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return nullptr;
}

void Ability::OnDisconnect(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

ErrCode Ability::StartAbilityForResult(const Want &want, int requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "start");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is null");
        return ERR_NULL_OBJECT;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Ability type is %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability type %{public}d mismatch.", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return err;
}

ErrCode Ability::StartAbilityForResult(const Want &want, int requestCode, AbilityStartSetting abilityStartSetting)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr");
        return ERR_NULL_OBJECT;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Ability type is %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability type %{public}d mismatch.", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode, abilityStartSetting);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return err;
}

ErrCode Ability::StartAbility(const Want &want, AbilityStartSetting abilityStartSetting)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr");
        return ERR_NULL_OBJECT;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Ability type is %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE && abilityInfo_->type != AppExecFwk::AbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability type %{public}d mismatch.", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, -1, abilityStartSetting);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnConfigurationUpdatedNotify(const Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");

    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    InitConfigurationProperties(configuration, language, colormode, hasPointerDevice);
    // Notify ResourceManager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Create res config failed.");
        return;
    }
    auto resourceManager = GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
#ifdef SUPPORT_GRAPHICS
        if (!language.empty()) {
            UErrorCode status = U_ZERO_ERROR;
            icu::Locale locale = icu::Locale::forLanguageTag(language, status);
            TAG_LOGD(AAFwkTag::ABILITY, "get Locale::forLanguageTag return[%{public}d].", static_cast<int>(status));
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
        TAG_LOGI(AAFwkTag::ABILITY,
            "Notify ResourceManager, current colorMode: %{public}d, hasPointerDevice: %{publis}d.",
            resConfig->GetColorMode(), resConfig->GetInputDevice());
    }

    if (abilityContext_ != nullptr && application_ != nullptr) {
        abilityContext_->SetConfiguration(application_->GetConfiguration());
    }
    // Notify Ability Subclass
    OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::InitConfigurationProperties(const Configuration& changeConfiguration, std::string& language,
    std::string& colormode, std::string& hasPointerDevice)
{
    if (setting_) {
        auto displayId = std::atoi(setting_->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str());
        language = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(displayId, AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        TAG_LOGI(AAFwkTag::ABILITY, "displayId: [%{public}d], language: [%{public}s], colormode: [%{public}s], "
            "hasPointerDevice: [%{public}s]", displayId, language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    } else {
        language = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE);
        colormode = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
        hasPointerDevice = changeConfiguration.GetItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE);
        TAG_LOGI(AAFwkTag::ABILITY, "language: [%{public}s], colormode: [%{public}s], hasPointerDevice: [%{public}s]",
            language.c_str(), colormode.c_str(), hasPointerDevice.c_str());
    }
}

void Ability::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (scene_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "WindowScene is null");
        return;
    }
    scene_->NotifyMemoryLevel(level);
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "application_ is nullptr.");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    return application_;
}

std::string Ability::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr");
        return "";
    }

    return abilityInfo_->name;
}

std::string Ability::GetModuleName()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr");
        return "";
    }

    return abilityInfo_->moduleName;
}

bool Ability::IsTerminating()
{
    return false;
}

void Ability::OnAbilityResult(int requestCode, int resultCode, const Want &want)
{}

void Ability::OnBackPressed()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr");
        return;
    }

    if (abilityInfo_->isLauncherAbility == false) {
        TAG_LOGD(AAFwkTag::ABILITY, "The ability is not Launcher");
        TerminateAbility();
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::OnNewWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnRestoreAbilityState(const PacMap &inState)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnSaveAbilityState(PacMap &outState)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Ability type: %{public}d", abilityInfo_->type);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        AbilityContext::resultWant_ = resultData;
        AbilityContext::resultCode_ = resultCode;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::ABILITY, "restart: %{public}s, startId: %{public}d.", restart ? "true" : "false", startId);
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::Dump(const std::string &extra)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Ability::Dump called");
    // abilityInfo
    TAG_LOGD(AAFwkTag::ABILITY, "==============AbilityInfo==============");
    if (abilityInfo_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: package: %{public}s", abilityInfo_->package.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: name: %{public}s", abilityInfo_->name.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: label: %{public}s", abilityInfo_->label.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: description: %{public}s", abilityInfo_->description.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: iconPath: %{public}s", abilityInfo_->iconPath.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: visible: %{public}d", abilityInfo_->visible);
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: kind: %{public}s", abilityInfo_->kind.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: type: %{public}d", abilityInfo_->type);
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: orientation: %{public}d", abilityInfo_->orientation);
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: launchMode: %{public}d", abilityInfo_->launchMode);
        for (auto permission : abilityInfo_->permissions) {
            TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: permission: %{public}s", permission.c_str());
        }
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: bundleName: %{public}s", abilityInfo_->bundleName.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo: applicationName: %{public}s", abilityInfo_->applicationName.c_str());
    } else {
        TAG_LOGD(AAFwkTag::ABILITY, "abilityInfo is nullptr");
    }

    // lifecycle_Event
    TAG_LOGD(AAFwkTag::ABILITY, "==============lifecycle_Event==============");
    if (lifecycle_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "lifecycle_Event: launchMode: %{public}d", lifecycle_->GetLifecycleState());
    } else {
        TAG_LOGD(AAFwkTag::ABILITY, "lifecycle is nullptr");
    }

    // lifecycle_State
    TAG_LOGD(AAFwkTag::ABILITY, "==============lifecycle_State==============");
    if (abilityLifecycleExecutor_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "lifecycle_State: launchMode: %{public}d", abilityLifecycleExecutor_->GetState());
    } else {
        TAG_LOGD(AAFwkTag::ABILITY, "abilityLifecycleExecutor is nullptr");
    }

    // applicationInfo
    TAG_LOGD(AAFwkTag::ABILITY, "==============applicationInfo==============");
    std::shared_ptr<ApplicationInfo> ApplicationInfoPtr = GetApplicationInfo();
    if (ApplicationInfoPtr != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "applicationInfo: name: %{public}s", ApplicationInfoPtr->name.c_str());
        TAG_LOGD(AAFwkTag::ABILITY, "applicationInfo: bundleName: %{public}s", ApplicationInfoPtr->bundleName.c_str());
    } else {
        TAG_LOGD(AAFwkTag::ABILITY, "ApplicationInfoPtr is nullptr");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    int amount = 0;
    for (auto it = values.begin(); it != values.end(); it++) {
        if (Insert(uri, *it) >= 0) {
            amount++;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Insert amount: %{public}d", amount);
    return amount;
}

void Ability::ContinueAbilityReversibly(const std::string &deviceId)
{
    if (!VerifySupportForContinuation()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Don't support continuation.");
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
        TAG_LOGE(AAFwkTag::ABILITY, "Don't support continuation.");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return lifecycle_;
}

void Ability::RegisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "register ability lifecycle observer failed, observer is nullptr.");
        return;
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "register ability lifecycle observer failed, lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->AddObserver(observer);
}

void Ability::UnregisterAbilityLifecycleObserver(const std::shared_ptr<ILifecycleObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "unregister ability lifecycle observer failed, observer is nullptr.");
        return;
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "unregister ability lifecycle observer failed, lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->RemoveObserver(observer);
}

AbilityLifecycleExecutor::LifecycleState Ability::GetState()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");

    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }

    return (AbilityLifecycleExecutor::LifecycleState)abilityLifecycleExecutor_->GetState();
}

ErrCode Ability::StartAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return AbilityContext::StartAbility(want, -1);
}

void Ability::PostTask(std::function<void()> task, long delayTime)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    TaskHandlerClient::GetInstance()->PostTask(task, delayTime);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

int32_t Ability::OnContinue(WantParams &wantParams)
{
    return ContinuationManager::OnContinueResult::REJECT;
}

void Ability::ContinueAbilityWithStack(const std::string &deviceId, uint32_t versionCode)
{
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Continue ability failed. deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Don't support continuation.");
        return;
    }
    continuationManager_->ContinueAbilityWithStack(deviceId, versionCode);
}

void Ability::ContinueAbility(const std::string &deviceId)
{
    if (deviceId.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Continue ability failed. deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Don't support continuation.");
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
    TAG_LOGD(AAFwkTag::ABILITY, "Change continuation state to initial");
    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Continuation manager is nullptr.");
        return;
    }

    continuationManager_->ChangeProcessStateToInit();
}

void Ability::OnRemoteTerminated()
{}

void Ability::DispatchLifecycleOnForeground(const Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityLifecycleExecutor_ is nullptr.");
        return;
    }
    if (abilityInfo_ != nullptr && abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    }
    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "lifecycle_ is nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_FOREGROUND, want);
}

bool Ability::VerifySupportForContinuation()
{
    if (continuationManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Continuation manager is nullptr.");
        return false;
    }
    return true;
}

void Ability::HandleCreateAsContinuation(const Want &want)
{
    if (!IsFlagExists(Want::FLAG_ABILITY_CONTINUATION, want.GetFlags())) {
        TAG_LOGD(AAFwkTag::ABILITY, "This's not continuated ability");
        return;
    }

    // check whether it needs reversible
    bool reversible = false;
    reversible = IsFlagExists(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE, want.GetFlags());
    if (!VerifySupportForContinuation()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Don't support continuation.");
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
            TAG_LOGE(AAFwkTag::ABILITY, "Create reverseContinuationSchedulerReplica failed");
            return;
        }
    }

    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    TAG_LOGD(AAFwkTag::ABILITY, "Notify complete continuation.");
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

void Ability::HandleCreateAsRecovery(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
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
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to get bundleMgrHelper.");
        return ERR_NULL_OBJECT;
    }
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "The ability info is nullptr.");
        return ERR_INVALID_VALUE;
    }
    Want want;
    want.SetAction("action.system.home");
    want.AddEntity("entity.system.home");
    want.SetElementName("", abilityInfo_->bundleName, "", "");
    AppExecFwk::AbilityInfo abilityInfo;
    bundleMgrHelper->QueryAbilityInfo(want, abilityInfo);
    std::string appName = bundleMgrHelper->GetAbilityLabel(abilityInfo_->bundleName, abilityInfo.name);
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

void Ability::SetStartAbilitySetting(std::shared_ptr<AbilityStartSetting> setting)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    setting_ = setting;
}

void Ability::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam& Ability::GetLaunchParam() const
{
    return launchParam_;
}

std::vector<std::shared_ptr<DataAbilityResult>> Ability::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo is nullptr");
        return results;
    }
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITY, "data ability type failed, current type: %{public}d", abilityInfo_->type);
        return results;
    }
    size_t len = operations.size();
    TAG_LOGD(AAFwkTag::ABILITY, "operation is nullptr, len %{public}zu", len);
    for (size_t i = 0; i < len; i++) {
        std::shared_ptr<DataAbilityOperation> operation = operations[i];
        if (operation == nullptr) {
            TAG_LOGD(AAFwkTag::ABILITY, "operation is nullptr, create DataAbilityResult");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            continue;
        }
        ExecuteOperation(operation, results, i);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end, %{public}zu", results.size());
    return results;
}
void Ability::ExecuteOperation(std::shared_ptr<DataAbilityOperation> &operation,
    std::vector<std::shared_ptr<DataAbilityResult>> &results, int index)
{
    TAG_LOGD(AAFwkTag::ABILITY, "exec start, index=%{public}d", index);
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITY, "exec data ability type failed, current type: %{public}d", abilityInfo_->type);
        return;
    }
    if (index < 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "exec operation result index should not below zero, current index: %{public}d",
            index);
        return;
    }
    if (operation == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITY, "exec operation is nullptr, create DataAbilityResult");
        results.push_back(std::make_shared<DataAbilityResult>(0));
        return;
    }

    int numRows = 0;
    std::shared_ptr<NativeRdb::ValuesBucket> valuesBucket = ParseValuesBucketReference(results, operation, index);
    auto predicates = ParsePredictionArgsReference(results, operation, index);
    if (operation->IsInsertOperation()) {
        TAG_LOGD(AAFwkTag::ABILITY, "exec IsInsertOperation");
        numRows = Insert(*(operation->GetUri().get()), *valuesBucket);
    } else if (operation->IsDeleteOperation() && predicates) {
        TAG_LOGD(AAFwkTag::ABILITY, "exec IsDeleteOperation");
        numRows = Delete(*(operation->GetUri().get()), *predicates);
    } else if (operation->IsUpdateOperation() && predicates) {
        TAG_LOGD(AAFwkTag::ABILITY, "exec IsUpdateOperation");
        numRows = Update(*(operation->GetUri().get()), *valuesBucket, *predicates);
    } else if (operation->IsAssertOperation() && predicates) {
        TAG_LOGD(AAFwkTag::ABILITY, "exec IsAssertOperation");
        std::vector<std::string> columns;
        auto queryResult = Query(*(operation->GetUri().get()), columns, *predicates);
        if (queryResult == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "exec Query retval is nullptr");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            return;
        }
        (void)CheckAssertQueryResult(queryResult, operation->GetValuesBucket());
        queryResult->Close();
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "exec Expected bad type %{public}d", operation->GetType());
    }
    if (operation->GetExpectedCount() == numRows) {
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
        TAG_LOGE(AAFwkTag::ABILITY, "intput is nullptr");
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
        TAG_LOGD(AAFwkTag::ABILITY, "operation->GetDataAbilityPredicates is nullptr");
    } else {
        TAG_LOGD(AAFwkTag::ABILITY, "operation->GetDataAbilityPredicates isn`t nullptr");
        strPredicatesList = predicates->GetWhereArgs();
    }

    if (strPredicatesList.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "operation->GetDataAbilityPredicates()->GetWhereArgs()"
                 "error strList is empty()");
    }

    for (auto iterMap : predicatesBackReferencesMap) {
        TAG_LOGD(AAFwkTag::ABILITY, "predicatesBackReferencesMap first:%{public}d second:%{public}d",
            iterMap.first,
            iterMap.second);
        int tempCount = ChangeRef2Value(results, numRefs, iterMap.second);
        if (tempCount < 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "tempCount:%{public}d", tempCount);
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
    if (operation == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "intput is nullptr");
        return nullptr;
    }
    if (operation->GetValuesBucketReferences() == nullptr) {
        return operation->GetValuesBucket();
    }

    NativeRdb::ValuesBucket retValueBucket;
    retValueBucket.Clear();
    if (operation->GetValuesBucket() != nullptr) {
        retValueBucket = *operation->GetValuesBucket();
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMapReferences;
    operation->GetValuesBucketReferences()->GetAll(valuesMapReferences);

    for (auto itermap : valuesMapReferences) {
        std::string key = itermap.first;
        TAG_LOGD(AAFwkTag::ABILITY, "key is %{public}s", key.c_str());
        NativeRdb::ValueObject obj;
        if (!operation->GetValuesBucketReferences()->GetObject(key, obj)) {
            TAG_LOGE(AAFwkTag::ABILITY, "operation->GetValuesBucketReferences()->GetObject error");
            continue;
        }
        switch (obj.GetType()) {
            case NativeRdb::ValueObjectType::TYPE_INT:
                ParseIntValue(obj, key, retValueBucket);
                break;
            case NativeRdb::ValueObjectType::TYPE_DOUBLE:
                ParseDoubleValue(obj, key, retValueBucket);
                break;
            case NativeRdb::ValueObjectType::TYPE_STRING:
                ParseStringValue(obj, key, retValueBucket);
                break;
            case NativeRdb::ValueObjectType::TYPE_BLOB:
                ParseBlobValue(obj, key, retValueBucket);
                break;
            case NativeRdb::ValueObjectType::TYPE_BOOL:
                ParseBoolValue(obj, key, retValueBucket);
                break;
            default:
                retValueBucket.PutNull(key);
                break;
        }
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    retValueBucket.GetAll(valuesMap);
    return std::make_shared<NativeRdb::ValuesBucket>(valuesMap);
}

void Ability::ParseIntValue(const NativeRdb::ValueObject &obj, const std::string &key,
    NativeRdb::ValuesBucket &retValueBucket) const
{
    int val = 0;
    if (obj.GetInt(val) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "ValueObject->GetInt() error");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "retValueBucket->PutInt(%{public}s, %{public}d)", key.c_str(), val);
    retValueBucket.PutInt(key, val);
}

void Ability::ParseDoubleValue(const NativeRdb::ValueObject &obj, const std::string &key,
    NativeRdb::ValuesBucket &retValueBucket) const
{
    double val = 0.0;
    if (obj.GetDouble(val) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "ValueObject->GetDouble() error");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "retValueBucket->PutDouble(%{public}s, %{public}f)", key.c_str(), val);
    retValueBucket.PutDouble(key, val);
}

void Ability::ParseStringValue(const NativeRdb::ValueObject &obj, const std::string &key,
    NativeRdb::ValuesBucket &retValueBucket) const
{
    std::string val = "";
    if (obj.GetString(val) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "ValueObject->GetString() error");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "retValueBucket->PutString(%{public}s, %{public}s)", key.c_str(), val.c_str());
    retValueBucket.PutString(key, val);
}

void Ability::ParseBlobValue(const NativeRdb::ValueObject &obj, const std::string &key,
    NativeRdb::ValuesBucket &retValueBucket) const
{
    std::vector<uint8_t> val;
    if (obj.GetBlob(val) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "ValueObject->GetBlob() error");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "retValueBucket->PutBlob(%{public}s, %{public}zu)", key.c_str(), val.size());
    retValueBucket.PutBlob(key, val);
}

void Ability::ParseBoolValue(const NativeRdb::ValueObject &obj, const std::string &key,
    NativeRdb::ValuesBucket &retValueBucket) const
{
    bool val = false;
    if (obj.GetBool(val) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "ValueObject->GetBool() error");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "retValueBucket->PutBool(%{public}s, %{public}s)", key.c_str(), val ? "true" : "false");
    retValueBucket.PutBool(key, val);
}

int Ability::ChangeRef2Value(std::vector<std::shared_ptr<DataAbilityResult>> &results, int numRefs, int index)
{
    int retval = -1;
    if (index >= numRefs) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::ChangeRef2Value index >= numRefs");
        return retval;
    }

    if (index >= static_cast<int>(results.size())) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::ChangeRef2Value index:%{public}d >= results.size():%{public}zu",
            index, results.size());
        return retval;
    }

    std::shared_ptr<DataAbilityResult> refResult = results[index];
    if (refResult == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::ChangeRef2Value No.%{public}d refResult is null", index);
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
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult intput queryResult is null");
        return true;
    }

    if (valuesBucket == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult intput valuesBucket is null");
        return true;
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    valuesBucket->GetAll(valuesMap);
    if (valuesMap.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult valuesMap is empty");
        return true;
    }
    int count = 0;
    if (queryResult->GetRowCount(count) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult GetRowCount is 0");
        return true;
    }

    for (auto iterMap : valuesMap) {
        std::string strObject;
        if (iterMap.second.GetString(strObject) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult GetString strObject is error");
            continue;
        }
        if (strObject.empty()) {
            TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult strObject is empty");
            continue;
        }
        for (int i = 0; i < count; ++i) {
            std::string strName;
            if (queryResult->GetString(i, strName) != 0) {
                TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult GetString strName is error");
                continue;
            }
            if (strName.empty()) {
                TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult strName is empty");
                continue;
            }
            if (strName == strObject) {
                TAG_LOGE(AAFwkTag::ABILITY, "Ability::CheckAssertQueryResult strName same to strObject");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = StartAbilityForResult(want, requestCode);
    TAG_LOGD(AAFwkTag::ABILITY, "ret: %{public}d", err);
    return err;
}

void Ability::OnFeatureAbilityResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, want);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "end");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

int32_t Ability::OnShare(WantParams &wantParams)
{
    return ERR_OK;
}

#ifdef SUPPORT_GRAPHICS
bool Ability::PrintDrawnCompleted()
{
    return AbilityContext::PrintDrawnCompleted();
}

void Ability::OnSceneCreated()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnSceneRestored()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::onSceneDestroyed()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    DoOnForeground(want);
    DispatchLifecycleOnForeground(want);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    eventInfo.callerBundleName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    if (abilityInfo_ != nullptr) {
        eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr, can not get bundleType.");
    }
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONFOREGROUND,
        HiSysEventType::BEHAVIOR, eventInfo);
}

void Ability::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityInfo_ is nullptr.");
        return;
    }
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        if (abilityInfo_->isStageBasedModel) {
            if (scene_ != nullptr) {
                TAG_LOGD(AAFwkTag::ABILITY, "GoBackground sceneFlag:%{public}d.", sceneFlag_);
                scene_->GoBackground(sceneFlag_);
            }
        } else {
            if (abilityWindow_ == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITY, "Ability::OnBackground error. abilityWindow_ == nullptr.");
                return;
            }
            TAG_LOGD(AAFwkTag::ABILITY, "OnPostAbilityBackground sceneFlag:%{public}d.", sceneFlag_);
            abilityWindow_->OnPostAbilityBackground(sceneFlag_);
        }
    }

    if (abilityLifecycleExecutor_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::OnBackground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }

    if (abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND);
    }

    if (lifecycle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::OnBackground error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_BACKGROUND);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    eventInfo.bundleType = static_cast<int32_t>(abilityInfo_->applicationInfo.bundleType);
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::ABILITY_ONBACKGROUND,
        HiSysEventType::BEHAVIOR, eventInfo);
}

bool Ability::OnBackPress()
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    return false;
}

bool Ability::OnPrepareTerminate()
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    return false;
}

void Ability::OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Ability::OnKeyDown called");
}

void Ability::OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Ability::OnKeyUp called");
    auto code = keyEvent->GetKeyCode();
    if (code == MMI::KeyEvent::KEYCODE_BACK) {
        TAG_LOGD(AAFwkTag::ABILITY, "Ability::OnKey Back key pressed.");
        OnBackPressed();
    }
}

void Ability::OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "Ability::OnTouchEvent called");
}

void Ability::InitWindow(int32_t displayId, sptr<Rosen::WindowOption> option)
{
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability window is nullptr.");
        return;
    }
    abilityWindow_->SetSessionToken(sessionToken_);
    abilityWindow_->InitWindow(abilityContext_, sceneListener_, displayId, option, securityFlag_);
}

const sptr<Rosen::Window> Ability::GetWindow()
{
    if (abilityWindow_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "Ability window is nullptr.");
        return nullptr;
    }
    return abilityWindow_->GetWindow();
}

std::shared_ptr<Rosen::WindowScene> Ability::GetScene()
{
    return scene_;
}

bool Ability::HasWindowFocus()
{
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::HasWindowFocus abilityInfo_ == nullptr");
        return false;
    }

    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        return bWindowFocus_;
    }

    return false;
}

void Ability::SetShowOnLockScreen(bool showOnLockScreen)
{
    TAG_LOGD(AAFwkTag::ABILITY, "SetShowOnLockScreen come, showOnLockScreen is %{public}d", showOnLockScreen);
    showOnLockScreen_ = showOnLockScreen;
    sptr<Rosen::Window> window = nullptr;
    if (abilityWindow_ == nullptr || (window = abilityWindow_->GetWindow()) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "SetShowOnLockScreen come, window is null");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "SetShowOnLockScreen come, addWindowFlag, showOnLockScreen is %{public}d",
        showOnLockScreen);
    if (showOnLockScreen) {
        window->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
        if (abilityInfo_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "Ability::SetShowOnLockScreen abilityInfo_ == nullptr");
            return;
        }
        AAFwk::EventInfo eventInfo;
        eventInfo.bundleName = abilityInfo_->bundleName;
        eventInfo.moduleName = abilityInfo_->moduleName;
        eventInfo.abilityName = abilityInfo_->name;
        AAFwk::EventReport::SendKeyEvent(AAFwk::EventName::FA_SHOW_ON_LOCK, HiSysEventType::BEHAVIOR, eventInfo);
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    FormProviderInfo formProviderInfo;
    return formProviderInfo;
}

bool Ability::OnShare(int64_t formId, AAFwk::WantParams &wantParams)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return false;
}

void Ability::OnDelete(const int64_t formId)
{}

void Ability::OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams)
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

void Ability::SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    sceneListener_ = listener;
}

sptr<Rosen::WindowOption> Ability::GetWindowOption(const Want &want)
{
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::GetWindowOption option is null.");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    TAG_LOGD(AAFwkTag::ABILITY, "Ability::GetWindowOption window mode is %{public}d.", windowMode);
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
        TAG_LOGD(AAFwkTag::ABILITY, "Ability::GetWindowOption come, add window flag WINDOW_FLAG_SHOW_WHEN_LOCKED.");
        option->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        TAG_LOGD(AAFwkTag::ABILITY, "Set window type for launcher");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    return option;
}

void Ability::DoOnForeground(const Want& want)
{
    if (abilityWindow_ == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "Ability window is nullptr.");
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITY, "sceneFlag: %{public}d.", sceneFlag_);
    auto window = abilityWindow_->GetWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::ABILITY, "set window mode = %{public}d.", windowMode);
    }
    abilityWindow_->OnPostAbilityForeground(sceneFlag_);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

int Ability::GetCurrentWindowMode()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
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
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid ability info.");
        return -1;
    }

    // stage mode
    if (abilityInfo_->isStageBasedModel) {
        if (scene_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed.");
            return -1;
        }
        auto window = scene_->GetMainWindow();
        if (window == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed.");
            return -1;
        }

        if (window->SetAPPWindowLabel(label) != OHOS::Rosen::WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::ABILITY, "SetAPPWindowLabel failed.");
            return -1;
        }
        return ERR_OK;
    }

    // fa mode
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityWindow is nullptr.");
        return -1;
    }
    return abilityWindow_->SetMissionLabel(label);
}

ErrCode Ability::SetMissionIcon(const std::shared_ptr<OHOS::Media::PixelMap> &icon)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (!abilityInfo_ || abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid ability info, can not set mission icon.");
        return -1;
    }

    // stage mode
    if (abilityInfo_->isStageBasedModel) {
        if (scene_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed, can not set mission icon.");
            return -1;
        }
        auto window = scene_->GetMainWindow();
        if (window == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed, can not set mission icon.");
            return -1;
        }

        if (window->SetAPPWindowIcon(icon) != OHOS::Rosen::WMError::WM_OK) {
            TAG_LOGE(AAFwkTag::ABILITY, "SetAPPWindowIcon failed.");
            return -1;
        }
        return ERR_OK;
    }

    // fa mode
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityWindow is nullptr, can not set mission icon.");
        return -1;
    }
    return abilityWindow_->SetMissionIcon(icon);
}

void Ability::GetWindowRect(int32_t &left, int32_t &top, int32_t &width, int32_t &height)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed.");
        return;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed.");
        return;
    }
    left = window->GetRect().posX_;
    top = window->GetRect().posY_;
    width = static_cast<int32_t>(window->GetRect().width_);
    height = static_cast<int32_t>(window->GetRect().height_);
    TAG_LOGI(AAFwkTag::ABILITY, "left: %{public}d, top: %{public}d, width: %{public}d, height: %{public}d",
        left, top, width, height);
}

Ace::UIContent* Ability::GetUIContent()
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (scene_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "get window scene failed.");
        return nullptr;
    }
    auto window = scene_->GetMainWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "get window failed.");
        return nullptr;
    }
    return window->GetUIContent();
}

void Ability::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

void Ability::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "displayId: %{public}" PRIu64"", displayId);

    // Get display
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        TAG_LOGE(AAFwkTag::ABILITY, "Get display by displayId %{public}" PRIu64" failed.", displayId);
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
            TAG_LOGI(AAFwkTag::ABILITY, "Notify ResourceManager, Density: %{public}f, Direction: %{public}d",
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    // Notify ability
    Configuration newConfig;
    newConfig.AddItem(displayId, ConfigurationInner::APPLICATION_DIRECTION, GetDirectionStr(height, width));
    newConfig.AddItem(displayId, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));

    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "application_ is nullptr.");
        return;
    }

    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        TAG_LOGE(AAFwkTag::ABILITY, "configuration is nullptr.");
        return;
    }

    std::vector<std::string> changeKeyV;
    configuration->CompareDifferent(changeKeyV, newConfig);
    TAG_LOGD(AAFwkTag::ABILITY, "changeKeyV size :%{public}zu", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [ability = shared_from_this(), configuration = *configuration]() {
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task, "Ability:OnChange");

        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
        TAG_LOGI(AAFwkTag::ABILITY, "Update display config %{public}s for all windows.",
            diffConfiguration->GetName().c_str());
        Rosen::Window::UpdateConfigurationForAll(diffConfiguration);
    }

    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void Ability::OnDisplayMove(Rosen::DisplayId from, Rosen::DisplayId to)
{
    TAG_LOGI(AAFwkTag::ABILITY, "from displayId %{public}" PRIu64" to %{public}" PRIu64".", from, to);

    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(to);
    if (!display) {
        TAG_LOGE(AAFwkTag::ABILITY, "Get display by displayId %{public}" PRIu64" failed.", to);
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
            TAG_LOGI(AAFwkTag::ABILITY, "Notify ResourceManager, Density: %{public}f, Direction: %{public}d.",
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    Configuration newConfig;
    newConfig.AddItem(ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(to));
    newConfig.AddItem(to, ConfigurationInner::APPLICATION_DIRECTION, GetDirectionStr(height, width));
    newConfig.AddItem(to, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));

    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "application_ is nullptr.");
        return;
    }

    std::vector<std::string> changeKeyV;
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        TAG_LOGE(AAFwkTag::ABILITY, "configuration is nullptr.");
        return;
    }

    configuration->CompareDifferent(changeKeyV, newConfig);
    TAG_LOGD(AAFwkTag::ABILITY, "changeKeyV size :%{public}zu", changeKeyV.size());
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [ability = shared_from_this(), configuration = *configuration]() {
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task, "Ability:OnDisplayMove");
    }
}

void Ability::RequestFocus(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityWindow_ == nullptr) {
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        TAG_LOGD(AAFwkTag::ABILITY, "set window mode = %{public}d.", windowMode);
    }
    abilityWindow_->OnPostAbilityForeground(sceneFlag_);
}

void Ability::SetWakeUpScreen(bool wakeUp)
{
    TAG_LOGD(AAFwkTag::ABILITY, "FA mode::SetWakeUpScreen wakeUp:%{public}d.", wakeUp);
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "SetWakeUpScreen error. abilityWindow_ == nullptr.");
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window nullptr.");
        return;
    }
    window->SetTurnScreenOn(wakeUp);
}

void Ability::SetDisplayOrientation(int orientation)
{
    TAG_LOGD(AAFwkTag::ABILITY, "FA mode, orientation: %{public}d", orientation);
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability window is nullptr.");
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window is nullptr.");
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
        TAG_LOGD(AAFwkTag::ABILITY, "Set LANDSCAPE");
        window->SetRequestedOrientation(Rosen::Orientation::HORIZONTAL);
    } else if (orientation == static_cast<int>(DisplayOrientation::PORTRAIT)) {
        TAG_LOGD(AAFwkTag::ABILITY, "Set PORTRAIT");
        window->SetRequestedOrientation(Rosen::Orientation::VERTICAL);
    } else {
        TAG_LOGD(AAFwkTag::ABILITY, "Set UNSPECIFIED");
        window->SetRequestedOrientation(Rosen::Orientation::UNSPECIFIED);
    }
}

int Ability::GetDisplayOrientation()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability::GetDisplayOrientation error. abilityWindow_ == nullptr.");
        return -1;
    }
    TAG_LOGD(AAFwkTag::ABILITY, "FA mode");
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window is nullptr.");
        return -1;
    }
    auto orientation = window->GetRequestedOrientation();
    if (orientation == Rosen::Orientation::HORIZONTAL) {
        TAG_LOGD(AAFwkTag::ABILITY, "Get window orientation: LANDSCAPE");
        return static_cast<int>(DisplayOrientation::LANDSCAPE);
    }
    if (orientation == Rosen::Orientation::VERTICAL) {
        TAG_LOGD(AAFwkTag::ABILITY, "Get window orientation: PORTRAIT");
        return static_cast<int>(DisplayOrientation::PORTRAIT);
    }
    TAG_LOGD(AAFwkTag::ABILITY, "Get window orientation: UNSPECIFIED");
    return 0;
}

void Ability::ContinuationRestore(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
}

int Ability::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilitycontext is nullptr");
        return ERR_INVALID_VALUE;
    }
    return abilityContextImpl->CreateModalUIExtensionWithApp(want);
}

void Ability::SetSessionToken(sptr<IRemoteObject> sessionToken)
{
    std::lock_guard lock(sessionTokenMutex_);
    sessionToken_ = sessionToken;
}

void Ability::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    SetSessionToken(sessionToken);
    if (abilityWindow_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Ability window is nullptr.");
        return;
    }
    abilityWindow_->SetSessionToken(sessionToken);
}

void Ability::InitFAWindow(const Want &want, int32_t displayId)
{
    if (!abilityInfo_->isStageBasedModel) {
        auto option = GetWindowOption(want);
        InitWindow(displayId, option);
    }

    if (abilityWindow_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "Get window from abilityWindow.");
        auto window = abilityWindow_->GetWindow();
        if (window) {
            TAG_LOGD(AAFwkTag::ABILITY, "Call RegisterDisplayMoveListener, windowId: %{public}d",
                window->GetWindowId());
            abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weak_from_this());
            window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
        }
    }
}

bool Ability::UpdateResMgrAndConfiguration(int32_t displayId)
{
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (!display) {
        TAG_LOGI(AAFwkTag::ABILITY, "The display is invliad.");
        return true;
    }
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
        TAG_LOGE(AAFwkTag::ABILITY, "Create resource config failed.");
        return false;
    }
    auto resourceManager = GetResourceManager();
    if (resourceManager != nullptr) {
        resourceManager->GetResConfig(*resConfig);
        resConfig->SetScreenDensity(density);
        resConfig->SetDirection(ConvertDirection(height, width));
        resourceManager->UpdateResConfig(*resConfig);
        TAG_LOGD(AAFwkTag::ABILITY, "Notify ResourceManager, Density: %{public}f, Direction: %{public}d.",
            resConfig->GetScreenDensity(), resConfig->GetDirection());
    }
    return true;
}

void Ability::EraseUIExtension(int32_t sessionId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    auto abilityContextImpl = GetAbilityContext();
    if (abilityContextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "abilityContext is nullptr");
        return;
    }
    abilityContextImpl->EraseUIExtension(sessionId);
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
