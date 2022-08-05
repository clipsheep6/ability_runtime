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

#include "ability_impl.h"
#include "ability_loader.h"
#include "ability_post_event_timeout.h"
#include "ability_runtime/js_ability.h"
#include "abs_shared_result_set.h"
#include "hitrace_meter.h"
#include "configuration_convertor.h"
#include "connection_manager.h"
#include "context_impl.h"
#include "continuation_manager.h"
#include "continuation_register_manager.h"
#include "continuation_register_manager_proxy.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "data_uri_utils.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "ohos_application.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_replica.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "runtime.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "values_bucket.h"
#include "event_report.h"

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
#include "background_task_mgr_helper.h"
#include "continuous_task_param.h"
#endif

#ifdef DISTRIBUTED_DATA_OBJECT_ENABLE
#include "distributed_objectstore.h"
#endif

#ifdef SUPPORT_GRAPHICS
#include "display_type.h"
#include "form_host_client.h"
#include "form_mgr.h"
#include "form_mgr_errors.h"
#include "form_provider_client.h"
#include "key_event.h"
#endif

namespace OHOS {
namespace AppExecFwk {
// REGISTER_AA(Ability)
const std::string Ability::SYSTEM_UI("com.ohos.systemui");
const std::string Ability::STATUS_BAR("com.ohos.systemui.statusbar.MainAbility");
const std::string Ability::NAVIGATION_BAR("com.ohos.systemui.navigationbar.MainAbility");
const std::string Ability::KEYGUARD("com.ohos.screenlock");
const std::string DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
const std::string DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";
const std::string Ability::DMS_SESSION_ID("sessionId");
const std::string Ability::DMS_ORIGIN_DEVICE_ID("deviceId");
const int Ability::DEFAULT_DMS_SESSION_ID(0);
const std::string PERMISSION_REQUIRE_FORM = "ohos.permission.REQUIRE_FORM";
const std::string LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";
const std::string LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
const std::string DLP_INDEX = "ohos.dlp.params.index";
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";

#ifdef DISTRIBUTED_DATA_OBJECT_ENABLE
constexpr int32_t DISTRIBUTED_OBJECT_TIMEOUT = 10000;
#endif

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
    HILOG_INFO("%{public}s begin.", __func__);
    abilityInfo_ = abilityInfo;
    handler_ = handler;
    AbilityContext::token_ = token;

#ifdef SUPPORT_GRAPHICS
    // page ability only.
    if (abilityInfo_->type == AbilityType::PAGE) {
        if (!abilityInfo_->isStageBasedModel) {
            abilityWindow_ = std::make_shared<AbilityWindow>();
            if (abilityWindow_ != nullptr) {
                HILOG_INFO("%{public}s begin abilityWindow_->Init", __func__);
                abilityWindow_->Init(handler_, shared_from_this());
                HILOG_INFO("%{public}s end abilityWindow_->Init", __func__);
            }
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
        HILOG_INFO("Ability::Init call RegisterDisplayListener");
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

    application_ = application;
    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
    }
    HILOG_INFO("%{public}s end.", __func__);
}

void Ability::AttachAbilityContext(const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    abilityContext_ = abilityContext;
}

/**
 * @brief Obtains a resource manager.
 *
 * @return Returns a ResourceManager object.
 */
std::shared_ptr<Global::Resource::ResourceManager> Ability::GetResourceManager() const
{
    return AbilityContext::GetResourceManager();
}

/**
 * @brief Checks whether the configuration of this ability is changing.
 *
 * @return Returns true if the configuration of this ability is changing and false otherwise.
 */
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
        Rosen::WindowType winType = Rosen::WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
        if (abilityInfo_->bundleName == SYSTEM_UI) {
            if (abilityInfo_->name == STATUS_BAR) {
                winType = Rosen::WindowType::WINDOW_TYPE_STATUS_BAR;
            }
            if (abilityInfo_->name == NAVIGATION_BAR) {
                winType = Rosen::WindowType::WINDOW_TYPE_NAVIGATION_BAR;
            }
        }
        if (abilityInfo_->bundleName == OHOS_REQUEST_PERMISSION_BUNDLENAME &&
            abilityInfo_->name == OHOS_REQUEST_PERMISSION_ABILITY_NAME) {
            winType = Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW;
        }

        if (abilityInfo_->bundleName == KEYGUARD) {
            winType = Rosen::WindowType::WINDOW_TYPE_DRAGGING_EFFECT;
        }

        if (abilityInfo_->bundleName == DEVICE_MANAGER_BUNDLE_NAME && abilityInfo_->name == DEVICE_MANAGER_NAME) {
            winType = Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW;
        }

        if (abilityInfo_->bundleName == LAUNCHER_BUNDLE_NAME && abilityInfo_->name == LAUNCHER_ABILITY_NAME) {
            winType = Rosen::WindowType::WINDOW_TYPE_DESKTOP;
        }

        int defualtDisplayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
        int displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, defualtDisplayId);
        HILOG_INFO("abilityName:%{public}s, windowType:%{public}d, displayId:%{public}d",
            abilityInfo_->name.c_str(), winType, displayId);
        auto option = GetWindowOption(want);
        InitWindow(winType, displayId, option);

        if (abilityWindow_ != nullptr) {
            HILOG_INFO("%{public}s begin abilityWindow_->OnPostAbilityStart.", __func__);
            abilityWindow_->OnPostAbilityStart();
            auto window = abilityWindow_->GetWindow();
            if (window) {
                auto windowId = window->GetWindowId();
                if (winType == Rosen::WindowType::WINDOW_TYPE_APP_MAIN_WINDOW) {
                    HILOG_INFO("Call RegisterDisplayMoveListener, windowId: %{public}d", windowId);
                    std::weak_ptr<Ability> weakAbility = shared_from_this();
                    abilityDisplayMoveListener_ = new AbilityDisplayMoveListener(weakAbility);
                    window->RegisterDisplayMoveListener(abilityDisplayMoveListener_);
                }
            }
            HILOG_INFO("%{public}s end abilityWindow_->OnPostAbilityStart.", __func__);
        }

        // Update resMgr, Configuration
        HILOG_INFO("%{public}s get display by displayId %{public}d.", __func__, displayId);
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
                resConfig->SetScreenDensity(ConvertDensity(density));
                resConfig->SetDirection(ConvertDirection(height, width));
                resourceManager->UpdateResConfig(*resConfig);
                HILOG_INFO("%{public}s Notify ResourceManager, Density: %{public}d, Direction: %{public}d.", __func__,
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
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Called when this ability enters the <b>STATE_STOP</b> state.
 *
 * The ability in the <b>STATE_STOP</b> is being destroyed.
 * You can override this function to implement your own processing logic.
 */
void Ability::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin", __func__);
#ifdef SUPPORT_GRAPHICS
    // Call JS Func(onWindowStageDestroy) and Release the scene.
    if (scene_ != nullptr) {
        scene_->GoDestroy();
        scene_ = nullptr;
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
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Release the window and ability.
 */
void Ability::DestroyInstance()
{
    HILOG_INFO("%{public}s begin", __func__);
#ifdef SUPPORT_GRAPHICS
    // Release the window.
    if (abilityWindow_ != nullptr && abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        abilityWindow_->OnPostAbilityStop(); // Ability instance will been released when window destroy.
    }
#endif
    HILOG_INFO("%{public}s end", __func__);
}

/**
 * @brief Called when this ability enters the <b>STATE_ACTIVE</b> state.
 *
 * The ability in the <b>STATE_ACTIVE</b> state is visible and has focus.
 * You can override this function to implement your own processing logic.
 *
 * @param Want Indicates the {@link Want} structure containing activation information about the ability.
 */
void Ability::OnActive()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
#ifdef SUPPORT_GRAPHICS
    if (abilityWindow_ != nullptr) {
        abilityWindow_->OnPostAbilityActive();
    }
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
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Called when this ability enters the <b>STATE_INACTIVE</b> state.
 *
 * <b>STATE_INACTIVE</b> is an instantaneous state. The ability in this state may be visible but does not have
 * focus.You can override this function to implement your own processing logic.
 */
void Ability::OnInactive()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin", __func__);
#ifdef SUPPORT_GRAPHICS
    if (abilityWindow_ != nullptr && abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        abilityWindow_->OnPostAbilityInactive();
    }
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
    HILOG_INFO("%{public}s end", __func__);
}

bool Ability::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is null");
        return false;
    }

    if (launchParam_.launchReason != LaunchReason::LAUNCHREASON_CONTINUATION) {
        HILOG_INFO("launchReason is %{public}d", launchParam_.launchReason);
        return false;
    }

    if (abilityContext_->GetContentStorage() == nullptr) {
        HILOG_INFO("not Restored In Continuation");
        return false;
    }

    HILOG_INFO("Is Restored In Continuation");
    return true;
}

void Ability::WaitingDistributedObjectSyncComplete(const Want& want)
{
#ifdef DISTRIBUTED_DATA_OBJECT_ENABLE
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);

    HILOG_INFO("continuation WaitingDistributedObjectSyncComplete begin");
    auto timeout = [self = shared_from_this(), sessionId, originDeviceId]() {
        HILOG_INFO("DistributedObject sync timeout");
        self->continuationManager_->NotifyCompleteContinuation(
            originDeviceId, sessionId, false, nullptr);
    };

    // std::shared_ptr<AppExecFwk::EventHandler> handler = handler_;
    auto callback = [self = shared_from_this(), sessionId, originDeviceId]() {
        HILOG_INFO("DistributedObject sync complete");
        if (self->handler_ != nullptr) {
            self->handler_->RemoveTask("Waiting_Sync_Timeout");
        }
        self->continuationManager_->NotifyCompleteContinuation(
            originDeviceId, sessionId, true, nullptr);
    };

    std::string &bundleName = abilityInfo_->bundleName;
    HILOG_INFO("continuation TriggerRestore begin");
    ObjectStore::DistributedObjectStore::GetInstance(bundleName)->TriggerRestore(callback);
    HILOG_INFO("continuation TriggerRestore end");

    if (handler_ != nullptr) {
        HILOG_INFO("continuation set timeout begin");
        handler_->PostTask(timeout, "Waiting_Sync_Timeout", DISTRIBUTED_OBJECT_TIMEOUT);
        HILOG_INFO("continuation set timeout end");
    }
#else
    NotityContinuationResult(want, true);
#endif
}

void Ability::NotityContinuationResult(const Want& want, bool success)
{
    HILOG_INFO("NotityContinuationResult begin");
    std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> ReplicaHandler = continuationHandler_;
    reverseContinuationSchedulerReplica_ = sptr<ReverseContinuationSchedulerReplica>(
        new (std::nothrow) ReverseContinuationSchedulerReplica(handler_, ReplicaHandler));

    if (reverseContinuationSchedulerReplica_ == nullptr) {
        HILOG_ERROR("Ability::NotityContinuationComplete failed, create reverseContinuationSchedulerReplica failed");
        return;
    }
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    HILOG_DEBUG("Ability::NotityContinuationComplete");
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}

/**
 * @brief Called when this Service ability is connected for the first time.
 *
 * You can override this function to implement your own processing logic.
 *
 * @param want Indicates the {@link Want} structure containing connection information about the Service ability.
 * @return Returns a pointer to the <b>sid</b> of the connected Service ability.
 */
sptr<IRemoteObject> Ability::OnConnect(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin", __func__);
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
    HILOG_INFO("%{public}s end", __func__);
    return nullptr;
}

/**
 * @brief Called when all abilities connected to this Service ability are disconnected.
 *
 * You can override this function to implement your own processing logic.
 *
 */
void Ability::OnDisconnect(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s come", __func__);
}

/**
 * Start other ability for result.
 *
 * @param want information of other ability
 * @param requestCode request code for abilityMS to return result
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::StartAbilityForResult(const Want &want, int requestCode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::StartAbilityForResult abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    HILOG_INFO("Ability::StartAbilityForResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode);
    HILOG_INFO("%{public}s end.", __func__);
    return err;
}

/**
 * Starts an ability with specific start settings and returns the execution result when the ability is destroyed.
 * When the ability is destroyed, onAbilityResult(int,int,ohos.aafwk.content.Want) is called and the returned
 * requestCode is transferred to the current method. The given requestCode is customized and cannot be a negative
 * number.
 *
 * @param want Indicates the ability to start.
 * @param requestCode Indicates the request code returned after the ability is started. You can define the request
 * code to identify the results returned by abilities. The value ranges from 0 to 65535.
 * @param abilityStartSetting Indicates the setting ability used to start.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::StartAbilityForResult(const Want &want, int requestCode, AbilityStartSetting abilityStartSetting)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::StartAbilityForResult abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    HILOG_INFO("Ability::StartAbilityForResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        HILOG_ERROR("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode, abilityStartSetting);
    HILOG_INFO("%{public}s end.", __func__);
    return err;
}

/**
 * Starts a new ability with specific start settings.
 * A Page or Service ability uses this method to start a specific ability.
 * The system locates the target ability from installed abilities based on
 * the value of the want parameter and then starts it. You can specify the
 * ability to start using the want parameter.
 *
 * @param want Indicates the ability to start.
 * @param abilityStartSetting Indicates the setting ability used to start.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::StartAbility(const Want &want, AbilityStartSetting abilityStartSetting)
{
    HILOG_INFO("%{public}s beign.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::StartAbility abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    HILOG_INFO("Ability::StartAbility called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE && abilityInfo_->type != AppExecFwk::AbilityType::SERVICE) {
        HILOG_ERROR("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, -1, abilityStartSetting);
    HILOG_INFO("%{public}s end.", __func__);
    return err;
}

/**
 * @brief Called back after permissions are requested by using
 * AbilityContext.requestPermissionsFromUser(java.lang.String[],int).
 *
 * @param requestCode Indicates the request code passed to this method from
 * AbilityContext.requestPermissionsFromUser(java.lang.String[],int).
 * @param permissions Indicates the list of permissions requested by using
 * AbilityContext.requestPermissionsFromUser(java.lang.String[],int). This parameter cannot be null.
 * @param grantResults Indicates the granting results of the corresponding permissions requested using
 * AbilityContext.requestPermissionsFromUser(java.lang.String[],int). The value 0 indicates that a
 * permission is granted, and the value -1 indicates not.
 *
 */
void Ability::OnRequestPermissionsFromUserResult(
    int requestCode, const std::vector<std::string> &permissions, const std::vector<int> &grantResults)
{}

/**
 * @brief Obtains the MIME type matching the data specified by the URI of the Data ability. This method should be
 * implemented by a Data ability. Data abilities supports general data types, including text, HTML, and JPEG.
 *
 * @param uri Indicates the URI of the data.
 *
 * @return Returns the MIME type that matches the data specified by uri.
 */
std::string Ability::GetType(const Uri &uri)
{
    return "";
}

/**
 * @brief Inserts a data record into the database. This method should be implemented by a Data ability.
 *
 * @param uri Indicates the position where the data is to insert.
 * @param value Indicates the data to insert.
 *
 * @return Returns the index of the newly inserted data record.
 */
int Ability::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    return 0;
}

std::shared_ptr<AppExecFwk::PacMap> Ability::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    return nullptr;
}

/**
 * @brief Called when the system configuration is updated.
 *
 * @param configuration Indicates the updated configuration information.
 */
void Ability::OnConfigurationUpdated(const Configuration &configuration)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void Ability::OnConfigurationUpdatedNotify(const Configuration &changeConfiguration)
{
    HILOG_INFO("%{public}s begin.", __func__);

    std::string language;
    std::string colormode;
    std::string hasPointerDevice;
    InitConfigurationProperties(changeConfiguration, language, colormode, hasPointerDevice);
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
            HILOG_INFO("get Locale::forLanguageTag return[%{public}d].", static_cast<int>(status));
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
    OnConfigurationUpdated(changeConfiguration);
    HILOG_INFO("%{public}s Notify Ability Subclass.", __func__);
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

/**
 * @brief Called when the system configuration is updated.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void Ability::OnMemoryLevel(int level)
{
    HILOG_INFO("%{public}s start.", __func__);
    if (scene_ == nullptr) {
        HILOG_DEBUG("WindowScene is null");
        return;
    }
    scene_->NotifyMemoryLevel(level);
}

/**
 * @brief This is like openFile, open a file that need to be able to return sub-sections of files，often assets
 * inside of their .hap.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the file open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing
 * data, or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the RawFileDescriptor object containing file descriptor.
 */
int Ability::OpenRawFile(const Uri &uri, const std::string &mode)
{
    return -1;
}

/**
 * @brief Updates one or more data records in the database. This method should be implemented by a Data ability.
 *
 * @param uri Indicates the database table storing the data to update.
 * @param value Indicates the data to update. This parameter can be null.
 * @param predicates Indicates filter criteria. If this parameter is null, all data records will be updated by
 * default.
 *
 * @return Returns the number of data records updated.
 */
int Ability::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    return 0;
}

/**
 * @brief get application witch the ability belong
 *
 * @return Returns the application ptr
 */
std::shared_ptr<OHOSApplication> Ability::GetApplication()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (application_ == nullptr) {
        HILOG_ERROR("Ability::GetApplication error. application_ == nullptr.");
        return nullptr;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return application_;
}

/**
 * @brief Obtains the class name in this ability name, without the prefixed bundle name.
 *
 * @return Returns the class name of this ability.
 */
std::string Ability::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::GetAbilityName abilityInfo_ is nullptr");
        return "";
    }

    return abilityInfo_->name;
}

/**
 * @brief OChecks whether the current ability is being destroyed.
 * An ability is being destroyed if you called terminateAbility() on it or someone else requested to destroy it.
 *
 * @return Returns true if the current ability is being destroyed; returns false otherwise.
 */
bool Ability::IsTerminating()
{
    return false;
}

/**
 * @brief Called when startAbilityForResult(ohos.aafwk.content.Want,int) is called to start an ability and the
 * result is returned. This method is called only on Page abilities. You can start a new ability to perform some
 * calculations and use setResult (int,ohos.aafwk.content.Want) to return the calculation result. Then the system
 * calls back the current method to use the returned data to execute its own logic.
 *
 * @param requestCode Indicates the request code returned after the ability is started. You can define the request
 * code to identify the results returned by abilities. The value ranges from 0 to 65535.
 * @param resultCode Indicates the result code returned after the ability is started. You can define the result code
 * to identify an error.
 * @param resultData Indicates the data returned after the ability is started. You can define the data returned. The
 * value can be null.
 *
 */
void Ability::OnAbilityResult(int requestCode, int resultCode, const Want &want)
{}

/**
 * @brief Called back when the Back key is pressed.
 * The default implementation destroys the ability. You can override this method.
 *
 */
void Ability::OnBackPressed()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::OnBackPressed abilityInfo_ is nullptr");
        return;
    }

    if (abilityInfo_->isLauncherAbility == false) {
        HILOG_INFO("Ability::OnBackPressed the ability is not Launcher");
        TerminateAbility();
    }
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
 * ability that has been at the top of the ability stack.
 *
 * @param want Indicates the new Want containing information about the ability.
 */
void Ability::OnNewWant(const Want &want)
{
    HILOG_INFO("Ability::OnNewWant called");
}

/**
 * @brief Restores data and states of an ability when it is restored by the system. This method should be
 * implemented by a Page ability. This method is called if an ability was destroyed at a certain time due to
 * resource reclaim or was unexpectedly destroyed and the onSaveAbilityState(ohos.utils.PacMap) method was called to
 * save its user data and states. Generally, this method is called after the onStart(ohos.aafwk.content.Want)
 * method.
 *
 *  @param inState Indicates the PacMap object used for storing data and states. This parameter can not be null.
 *
 */
void Ability::OnRestoreAbilityState(const PacMap &inState)
{
    HILOG_INFO("Ability::OnRestoreAbilityState called");
}

/**
 * @brief Saves temporary data and states of this ability. This method should be implemented by a Page ability.
 * This method is called when the system determines that the ability may be destroyed in an unexpected situation,
 * for example, when the screen orientation changes or the user touches the Home key. Generally, this method is used
 * only to save temporary states.
 *
 *  @param outState Indicates the PacMap object used for storing user data and states. This parameter cannot be
 * null.
 *
 */
void Ability::OnSaveAbilityState(PacMap &outState)
{
    HILOG_INFO("Ability::OnSaveAbilityState called");
}

/**
 * @brief Called every time a key, touch, or trackball event is dispatched to this ability.
 * You can override this callback method if you want to know that the user has interacted with
 * the device in a certain way while this ability is running. This method, together with onLeaveForeground(),
 * is designed to help abilities intelligently manage status bar notifications. Specifically, they help
 * abilities determine when to cancel a notification.
 *
 */
void Ability::OnEventDispatch()
{}

/**
 * @brief Sets the want object that can be obtained by calling getWant().
 * @param Want information of other ability
 */
void Ability::SetWant(const AAFwk::Want &want)
{
    setWant_ = std::make_shared<AAFwk::Want>(want);
}

/**
 * @brief Obtains the Want object that starts this ability.
 *
 * @return Returns the Want object that starts this ability.
 */
std::shared_ptr<AAFwk::Want> Ability::GetWant()
{
    return setWant_;
}

/**
 * @brief Sets the result code and data to be returned by this Page ability to the caller.
 * When a Page ability is destroyed, the caller overrides the AbilitySlice#onAbilityResult(int, int, Want) method to
 * receive the result set in the current method. This method can be called only after the ability has been
 * initialized.
 *
 * @param resultCode Indicates the result code returned after the ability is destroyed. You can define the result
 * code to identify an error.
 * @param resultData Indicates the data returned after the ability is destroyed. You can define the data returned.
 * This parameter can be null.
 */
void Ability::SetResult(int resultCode, const Want &resultData)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::SetResult abilityInfo_ == nullptr");
        return;
    }
    HILOG_INFO("Ability::SetResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        AbilityContext::resultWant_ = resultData;
        AbilityContext::resultCode_ = resultCode;
    }
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Called back when Service is started.
 * This method can be called only by Service. You can use the StartAbility(ohos.aafwk.content.Want) method to start
 * Service. Then the system calls back the current method to use the transferred want parameter to execute its own
 * logic.
 *
 * @param want Indicates the want of Service to start.
 * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 * @param startId Indicates the number of times the Service ability has been started. The startId is incremented by
 * 1 every time the ability is started. For example, if the ability has been started for six times, the value of
 * startId is 6.
 */
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
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief dump ability info
 *
 * @param extra dump ability info
 */
void Ability::Dump(const std::string &extra)
{
    HILOG_INFO("Ability::Dump called");
    // abilityInfo
    HILOG_INFO("==============AbilityInfo==============");
    if (abilityInfo_ != nullptr) {
        HILOG_INFO("abilityInfo: package: %{public}s", abilityInfo_->package.c_str());
        HILOG_INFO("abilityInfo: name: %{public}s", abilityInfo_->name.c_str());
        HILOG_INFO("abilityInfo: label: %{public}s", abilityInfo_->label.c_str());
        HILOG_INFO("abilityInfo: description: %{public}s", abilityInfo_->description.c_str());
        HILOG_INFO("abilityInfo: iconPath: %{public}s", abilityInfo_->iconPath.c_str());
        HILOG_INFO("abilityInfo: visible: %{public}d", abilityInfo_->visible);
        HILOG_INFO("abilityInfo: kind: %{public}s", abilityInfo_->kind.c_str());
        HILOG_INFO("abilityInfo: type: %{public}d", abilityInfo_->type);
        HILOG_INFO("abilityInfo: orientation: %{public}d", abilityInfo_->orientation);
        HILOG_INFO("abilityInfo: launchMode: %{public}d", abilityInfo_->launchMode);
        for (auto permission : abilityInfo_->permissions) {
            HILOG_INFO("abilityInfo: permission: %{public}s", permission.c_str());
        }
        HILOG_INFO("abilityInfo: bundleName: %{public}s", abilityInfo_->bundleName.c_str());
        HILOG_INFO("abilityInfo: applicationName: %{public}s", abilityInfo_->applicationName.c_str());
    } else {
        HILOG_INFO("abilityInfo is nullptr");
    }

    // lifecycle_Event
    HILOG_INFO("==============lifecycle_Event==============");
    if (lifecycle_ != nullptr) {
        HILOG_INFO("lifecycle_Event: launchMode: %{public}d", lifecycle_->GetLifecycleState());
    } else {
        HILOG_INFO("lifecycle is nullptr");
    }

    // lifecycle_State
    HILOG_INFO("==============lifecycle_State==============");
    if (abilityLifecycleExecutor_ != nullptr) {
        HILOG_INFO("lifecycle_State: launchMode: %{public}d", abilityLifecycleExecutor_->GetState());
    } else {
        HILOG_INFO("abilityLifecycleExecutor is nullptr");
    }

    // applicationInfo
    HILOG_INFO("==============applicationInfo==============");
    std::shared_ptr<ApplicationInfo> ApplicationInfoPtr = GetApplicationInfo();
    if (ApplicationInfoPtr != nullptr) {
        HILOG_INFO("applicationInfo: name: %{public}s", ApplicationInfoPtr->name.c_str());
        HILOG_INFO("applicationInfo: bundleName: %{public}s", ApplicationInfoPtr->bundleName.c_str());
    } else {
        HILOG_INFO("ApplicationInfoPtr is nullptr");
    }
}

/**
 * @brief dump ability info
 *
 * @param params dump params that indicate different dump targets
 * @param info dump ability info
*/
void Ability::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{}

/**
 * @brief Keeps this Service ability in the background and displays a notification bar.
 * To use this method, you need to request the ohos.permission.KEEP_BACKGROUND_RUNNING permission from the system.
 * The ohos.permission.KEEP_BACKGROUND_RUNNING permission is of the normal level.
 * This method can be called only by Service abilities after the onStart(ohos.aafwk.content.Want) method is called.
 *
 * @param id Identifies the notification bar information.
 * @param notificationRequest Indicates the NotificationRequest instance containing information for displaying a
 * notification bar.
 */
void Ability::KeepBackgroundRunning(int id, const NotificationRequest &notificationRequest)
{}

/**
 * @brief Cancels background running of this ability to free up system memory.
 * This method can be called only by Service abilities when the onStop() method is called.
 *
 */
void Ability::CancelBackgroundRunning()
{}

/**
 * @brief Converts the given uri that refer to the Data ability into a normalized URI. A normalized URI can be used
 * across devices, persisted, backed up, and restored. It can refer to the same item in the Data ability even if the
 * context has changed. If you implement URI normalization for a Data ability, you must also implement
 * denormalizeUri(ohos.utils.net.Uri) to enable URI denormalization. After this feature is enabled, URIs passed to
 * any method that is called on the Data ability must require normalization verification and denormalization. The
 * default implementation of this method returns null, indicating that this Data ability does not support URI
 * normalization.
 *
 * @param uri Indicates the Uri object to normalize.
 *
 * @return Returns the normalized Uri object if the Data ability supports URI normalization; returns null otherwise.
 */
Uri Ability::NormalizeUri(const Uri &uri)
{
    return uri;
}

/**
 * @brief Deletes one or more data records. This method should be implemented by a Data ability.
 *
 * @param uri Indicates the database table storing the data to delete.
 * @param predicates Indicates filter criteria. If this parameter is null, all data records will be deleted by
 * default.
 *
 * @return Returns the number of data records deleted.
 */
int Ability::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    return 0;
}

/**
 * @brief Obtains the MIME type of files. This method should be implemented by a Data ability.
 *
 * @param uri Indicates the path of the files to obtain.
 * @param mimeTypeFilter Indicates the MIME type of the files to obtain. This parameter cannot be set to null.
 * 1. * / *: Obtains all types supported by a Data ability.
 * 2. image/ *: Obtains files whose main type is image of any subtype.
 * 3. * /jpg: Obtains files whose subtype is JPG of any main type.
 *
 * @return Returns the MIME type of the matched files; returns null if there is no type that matches the Data
 * ability.
 */
std::vector<std::string> Ability::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    return types_;
}

/**
 * @brief Opens a file. This method should be implemented by a Data ability.
 *
 * @param uri Indicates the path of the file to open.
 * @param mode Indicates the open mode, which can be "r" for read-only access, "w" for write-only access
 * (erasing whatever data is currently in the file), "wt" for write access that truncates any existing file,
 * "wa" for write-only access to append to any existing data, "rw" for read and write access on any existing data,
 * or "rwt" for read and write access that truncates any existing file.
 *
 * @return Returns the FileDescriptor object of the file descriptor.
 */
int Ability::OpenFile(const Uri &uri, const std::string &mode)
{
    return -1;
}

/**
 * @brief Queries one or more data records in the database. This method should be implemented by a Data ability.
 *
 * @param uri Indicates the database table storing the data to query.
 * @param columns Indicates the columns to be queried, in array, for example, {"name","age"}. You should define the
 * processing logic when this parameter is null.
 * @param predicates Indicates filter criteria. If this parameter is null, all data records will be queried by
 * default.
 *
 * @return Returns the queried data.
 */
std::shared_ptr<NativeRdb::AbsSharedResultSet> Ability::Query(
    const Uri &uri, const std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    return nullptr;
}

/**
 * @brief Reloads data in the database.
 * The default implementation of this method returns false. You must implement this method in the child class
 * of your Data Ability if you want to use it.
 *
 * @param uri Indicates the position where the data is to reload.
 * @param extras Indicates the additional parameters contained in the PacMap object to be passed in this call.
 *
 * @return Returns true if the data is successfully reloaded; returns false otherwise.
 */
bool Ability::Reload(const Uri &uri, const PacMap &extras)
{
    return false;
}

/**
 * @brief Inserts multiple data records into the database.
 *
 * @param uri Indicates the path of the data to operate.
 * @param values Indicates the data records to insert.
 *
 * @return Returns the number of data records inserted.
 */
int Ability::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("%{public}s begin.", __func__);
    int amount = 0;
    for (auto it = values.begin(); it != values.end(); it++) {
        if (Insert(uri, *it) >= 0) {
            amount++;
        }
    }
    HILOG_INFO("%{public}s end, amount=%{public}d", __func__, amount);
    return amount;
}

/**
 * @brief Migrates this ability to the given device on the same distributed network in a reversible way that allows
 * this ability to be migrated back to the local device through reverseContinueAbility(). The ability to migrate and
 * its ability slices must implement the IAbilityContinuation interface. Otherwise, an exception is thrown,
 * indicating that the ability does not support migration.
 *
 * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
 * is null, this method has the same effect as ContinueAbilityReversibly().
 *
 */
void Ability::ContinueAbilityReversibly(const std::string &deviceId)
{
    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("Ability::ContinueAbilityReversibly(deviceId) failed. VerifySupportForContinuation faled");
        return;
    }
    continuationManager_->ContinueAbility(true, deviceId);
}

/**
 * @brief  public final String getOriginalDeviceId​() throws UnsupportedOperationException
 * Obtains the ID of the source device from which this ability is migrated.
 *
 * @return Returns the source device ID.
 */
std::string Ability::GetOriginalDeviceId()
{
    return "";
}

/**
 * @brief Obtains the migration state of this ability.
 * @return Returns the migration state.
 *
 * @return Returns the source device ID.
 */
ContinuationState Ability::GetContinuationState()
{
    if (!VerifySupportForContinuation()) {
        HILOG_ERROR("Ability::GetContinuationState failed. VerifySupportForContinuation faled");
        return ContinuationState::LOCAL_RUNNING;
    }
    return continuationManager_->GetContinuationState();
}

/**
 * @brief Obtains the singleton AbilityPackage object to which this ability belongs.
 *
 * @return Returns the singleton AbilityPackage object to which this ability belongs.
 */
std::shared_ptr<AbilityPackage> Ability::GetAbilityPackage()
{
    return nullptr;
}

/**
 * @brief Converts the given normalized uri generated by normalizeUri(ohos.utils.net.Uri) into a denormalized one.
 * The default implementation of this method returns the original URI passed to it.
 *
 * @param uri uri Indicates the Uri object to denormalize.
 *
 * @return Returns the denormalized Uri object if the denormalization is successful; returns the original Uri passed
 * to this method if there is nothing to do; returns null if the data identified by the original Uri cannot be found
 * in the current environment.
 */
Uri Ability::DenormalizeUri(const Uri &uri)
{
    return uri;
}

/**
 * @brief Obtains the Lifecycle object of the current ability.
 *
 * @return Returns the Lifecycle object.
 */
std::shared_ptr<LifeCycle> Ability::GetLifecycle()
{
    HILOG_INFO("Ability::GetLifecycle called");
    return lifecycle_;
}

/**
 * @brief Obtains the lifecycle state of this ability.
 *
 * @return Returns the lifecycle state of this ability.
 */
AbilityLifecycleExecutor::LifecycleState Ability::GetState()
{
    HILOG_INFO("Ability::GetState called");

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::GetState error. abilityLifecycleExecutor_ == nullptr.");
        return AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }

    return (AbilityLifecycleExecutor::LifecycleState)abilityLifecycleExecutor_->GetState();
}

/**
 * @brief A Page or Service ability uses this method to start a specific ability. The system locates the target
 * ability from installed abilities based on the value of the want parameter and then starts it. You can specify
 * the ability to start using the want parameter.
 *
 * @param want Indicates the ability to start.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::StartAbility(const Want &want)
{
    HILOG_INFO("%{public}s begin Ability::StartAbility", __func__);
    return AbilityContext::StartAbility(want, -1);
}

/**
 * @brief Destroys this Page or Service ability.
 * After a Page or Service ability performs all operations, it can use this method to destroy itself
 * to free up memory. This method can be called only after the ability is initialized.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::TerminateAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    return AbilityContext::TerminateAbility();
}

/**
 * @brief Destroys ability.
 *
 * @param want Indicates the want containing information about TerminateAbility
 *
 * @return Returns the result of TerminateAbility
 */
int Ability::TerminateAbility(Want &want)
{
    return -1;
}

/**
 * @brief Sets the main route for this ability.
 *
 * The main route, also called main entry, refers to the default <b>AbilitySlice</b> to present for this ability.
 * This function should be called only on Feature Abilities. If this function is not called in the
 * {@link OnStart(const Want &want)} function for a Feature Ability, the Feature Ability will fail to start.
 *
 * @param entry Indicates the main entry, which is the class name of the <b>AbilitySlice</b> instance to start.
 *
 * @return Returns the result of SetMainRouter
 */
void Ability::SetMainRoute(const std::string &entry)
{}

/**
 * @brief By binding an action, you can set different action parameters in Want to present different initial
 * pages. You must register actions in the profile file.
 *
 * @param action Indicates the action to bind.
 *
 * @param entry Indicates the entry, which is the fully qualified name of your AbilitySlice class.
 *
 * @return Returns the result of AddActionRoute
 */
void Ability::AddActionRoute(const std::string &action, const std::string &entry)
{}

/**
 * @brief Connects the current ability to an ability using the AbilityInfo.AbilityType.SERVICE template.
 *
 * @param want Indicates the want containing information about the ability to connect
 *
 * @param conn Indicates the callback object when the target ability is connected.
 *
 * @return True means success and false means failure
 */
bool Ability::ConnectAbility(const Want &want, const sptr<AAFwk::IAbilityConnection> &conn)
{
    return AbilityContext::ConnectAbility(want, conn);
}

/**
 * @brief Disconnects the current ability from an ability
 *
 * @param conn Indicates the IAbilityConnection callback object passed by connectAbility after the connection
 *              is set up. The IAbilityConnection object uniquely identifies a connection between two abilities.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::DisconnectAbility(const sptr<AAFwk::IAbilityConnection> &conn)
{
    return AbilityContext::DisconnectAbility(conn);
}

/**
 * @brief Destroys another ability that uses the AbilityInfo.AbilityType.SERVICE template.
 * The current ability using either the AbilityInfo.AbilityType.SERVICE or AbilityInfo.AbilityType.PAGE
 * template can call this method to destroy another ability that uses the AbilityInfo.AbilityType.SERVICE
 * template. The current ability itself can be destroyed by calling the terminateAbility() method.
 *
 * @param want Indicates the Want containing information about the ability to destroy.
 *
 * @return Returns true if the ability is destroyed successfully; returns false otherwise.
 */
bool Ability::StopAbility(const AAFwk::Want &want)
{
    return AbilityContext::StopAbility(want);
}

/**
 * @brief Posts a scheduled Runnable task to a new non-UI thread.
 * The task posted via this method will be executed in a new thread, which allows you to perform certain
 * time-consuming operations. To use this method, you must also override the supportHighPerformanceUI() method.
 *
 * @param task Indicates the Runnable task to post.
 *
 * @param delayTime Indicates the number of milliseconds after which the task will be executed.
 *
 * @return -
 */
void Ability::PostTask(std::function<void()> task, long delayTime)
{
    HILOG_INFO("%{public}s begin.", __func__);
    TaskHandlerClient::GetInstance()->PostTask(task, delayTime);
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief You can use the IContinuationRegisterManager object to interact with the Device+ control center,
 * including registering and unregistering the ability to migrate, updating the device connection state, and
 * showing the list of devices that can be selected for ability migration.
 *
 * @return Returns true if the migration request is successful; returns false otherwise.
 */
std::weak_ptr<IContinuationRegisterManager> Ability::GetContinuationRegisterManager()
{
    if (abilityInfo_ != nullptr) {
        if ((abilityInfo_->type == AbilityType::PAGE) && (continuationRegisterManager_ == nullptr)) {
            std::weak_ptr<Context> context = shared_from_this();
            std::shared_ptr<ContinuationRegisterManagerProxy> continuationRegisterManagerProxy =
                std::make_shared<ContinuationRegisterManagerProxy>(context);
            continuationRegisterManager_ = std::make_shared<ContinuationRegisterManager>();
            continuationRegisterManager_->Init(continuationRegisterManagerProxy);
        }
    }
    std::weak_ptr<IContinuationRegisterManager> continuationRegisterManager =
        std::weak_ptr<IContinuationRegisterManager>(continuationRegisterManager_);
    return continuationRegisterManager;
}

/**
 * @brief Callback function to ask the user to prepare for the migration .
 *
 * @return If the user allows migration and saves data suscessfully, it returns 0; otherwise, it returns errcode.
 */
int32_t Ability::OnContinue(WantParams &wantParams)
{
    return ContinuationManager::OnContinueResult::Reject;
}

/**
 * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
 * ability slices must implement the IAbilityContinuation interface.
 *
 * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
 * is null, this method has the same effect as continueAbility().
 *
 */
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

/**
 * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
 * ability slices must implement the IAbilityContinuation interface.
 *
 * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
 * is null, this method has the same effect as continueAbility().
 *
 */
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

/**
 * @brief Callback function to ask the user whether to start the migration .
 *
 * @return If the user allows migration, it returns true; otherwise, it returns false.
 */
bool Ability::OnStartContinuation()
{
    return false;
}

/**
 * @brief Save user data of local Ability generated at runtime.
 *
 * @param saveData Indicates the user data to be saved.
 * @return If the data is saved successfully, it returns true; otherwise, it returns false.
 */
bool Ability::OnSaveData(WantParams &saveData)
{
    return false;
}

/**
 * @brief After creating the Ability on the remote device,
 *      immediately restore the user data saved during the migration of the Ability on the remote device.
 * @param restoreData Indicates the user data to be restored.
 * @return If the data is restored successfully, it returns true; otherwise, it returns false .
 */
bool Ability::OnRestoreData(WantParams &restoreData)
{
    return false;
}

/**
 * @brief This function can be used to implement the processing logic after the migration is completed.
 *
 * @param result Migration result code. 0 means the migration was successful, -1 means the migration failed.
 * @return None.
 */
void Ability::OnCompleteContinuation(int result)
{
    HILOG_INFO("Ability::OnCompleteContinuation change continuation state to initial");
    continuationManager_->ChangeProcessStateToInit();
}

/**
 * @brief Used to notify the local Ability that the remote Ability has been destroyed.
 *
 * @return None.
 */
void Ability::OnRemoteTerminated()
{}

void Ability::DispatchLifecycleOnForeground(const Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnForeground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (abilityInfo_->isStageBasedModel) {
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
        HILOG_INFO("Ability::HandleCreateAsContinuation return. This not continuated ability");
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
bool Ability::IsFlagExists(unsigned int flag, unsigned int flagSet)
{
    return (flag & flagSet) == flag;
}

/**
 * @brief Called to set caller information for the application. The default implementation returns null.
 *
 * @return Returns the caller information.
 */
Uri Ability::OnSetCaller()
{
    return Uri("");
}

/**
 * @brief Create a PostEvent timeout task. The default delay is 5000ms
 *
 * @return Return a smart pointer to a timeout object
 */
std::shared_ptr<AbilityPostEventTimeout> Ability::CreatePostEventTimeouter(std::string taskstr)
{
    return std::make_shared<AbilityPostEventTimeout>(taskstr, handler_);
}

/**
 * @brief Keep this Service ability in the background and displays a notification bar.
 *
 * @param wantAgent Indicates which ability to start when user click the notification bar.
 * @return the method result code, 0 means succeed
 */
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

/**
 * @brief Cancel background running of this ability to free up system memory.
 *
 * @return the method result code, 0 means succeed
 */
int Ability::StopBackgroundRunning()
{
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    return BackgroundTaskMgr::BackgroundTaskMgrHelper::RequestStopBackgroundRunning(abilityInfo_->name, GetToken());
#else
    return ERR_INVALID_OPERATION;
#endif
}


/**
 * @brief Acquire a bundle manager, if it not existed.
 * @return returns the bundle manager ipc object, or nullptr for failed.
 */
sptr<IBundleMgr> Ability::GetBundleMgr()
{
    HILOG_INFO("%{public}s called.", __func__);
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

/**
 * @brief Add the bundle manager instance for debug.
 * @param bundleManager the bundle manager ipc object.
 */
void Ability::SetBundleManager(const sptr<IBundleMgr> &bundleManager)
{
    HILOG_INFO("%{public}s called.", __func__);

    iBundleMgr_ = bundleManager;
}

/**
 * @brief Set the start ability setting.
 * @param setting the start ability setting.
 */
void Ability::SetStartAbilitySetting(std::shared_ptr<AbilityStartSetting> setting)
{
    HILOG_INFO("%{public}s called.", __func__);
    setting_ = setting;
}

/**
 * @brief Set the launch param.
 *
 * @param launchParam the launch param.
 */
void Ability::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    HILOG_INFO("%{public}s called.", __func__);
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam& Ability::GetLaunchParam() const
{
    return launchParam_;
}

std::vector<std::shared_ptr<DataAbilityResult>> Ability::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_INFO("Ability::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::ExecuteBatch abilityInfo is nullptr");
        return results;
    }
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Ability::ExecuteBatch data ability type failed, current type: %{public}d", abilityInfo_->type);
        return results;
    }
    size_t len = operations.size();
    HILOG_INFO("Ability::ExecuteBatch operation is nullptr, len %{public}zu", len);
    for (size_t i = 0; i < len; i++) {
        std::shared_ptr<DataAbilityOperation> operation = operations[i];
        if (operation == nullptr) {
            HILOG_INFO("Ability::ExecuteBatch operation is nullptr, create DataAbilityResult");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            continue;
        }
        ExecuteOperation(operation, results, i);
    }
    HILOG_INFO("Ability::ExecuteBatch end, %{public}zu", results.size());
    return results;
}
void Ability::ExecuteOperation(std::shared_ptr<DataAbilityOperation> &operation,
    std::vector<std::shared_ptr<DataAbilityResult>> &results, int index)
{
    HILOG_INFO("Ability::ExecuteOperation start, index=%{public}d", index);
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        HILOG_ERROR("Ability::ExecuteOperation data ability type failed, current type: %{public}d", abilityInfo_->type);
        return;
    }
    if (index < 0) {
        HILOG_ERROR(
            "Ability::ExecuteOperation operation result index should not below zero, current index: %{public}d", index);
        return;
    }
    if (operation == nullptr) {
        HILOG_INFO("Ability::ExecuteOperation operation is nullptr, create DataAbilityResult");
        results.push_back(std::make_shared<DataAbilityResult>(0));
        return;
    }

    int numRows = 0;
    std::shared_ptr<NativeRdb::ValuesBucket> valuesBucket = ParseValuesBucketReference(results, operation, index);
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates =
        ParsePredictionArgsReference(results, operation, index);
    if (operation->IsInsertOperation()) {
        HILOG_INFO("Ability::ExecuteOperation IsInsertOperation");
        numRows = Insert(*(operation->GetUri().get()), *valuesBucket);
    } else if (operation->IsDeleteOperation() && predicates) {
        HILOG_INFO("Ability::ExecuteOperation IsDeleteOperation");
        numRows = Delete(*(operation->GetUri().get()), *predicates);
    } else if (operation->IsUpdateOperation() && predicates) {
        HILOG_INFO("Ability::ExecuteOperation IsUpdateOperation");
        numRows = Update(*(operation->GetUri().get()), *valuesBucket, *predicates);
    } else if (operation->IsAssertOperation() && predicates) {
        HILOG_INFO("Ability::ExecuteOperation IsAssertOperation");
        std::vector<std::string> columns;
        std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult =
            Query(*(operation->GetUri().get()), columns, *predicates);
        if (queryResult == nullptr) {
            HILOG_ERROR("Ability::ExecuteOperation Query retval is nullptr");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            return;
        }
        if (queryResult->GetRowCount(numRows) != 0) {
            HILOG_ERROR("Ability::ExecuteOperation queryResult->GetRowCount(numRows) != E_OK");
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
        HILOG_ERROR("Ability::ExecuteOperation Expected bad type %{public}d", operation->GetType());
    }
    if (operation->GetExpectedCount() != numRows) {
        HILOG_ERROR("Ability::ExecuteOperation Expected %{public}d rows but actual %{public}d",
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
        HILOG_ERROR("Ability::ParsePredictionArgsReference intpur is nullptr");
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
        HILOG_INFO("Ability::ParsePredictionArgsReference operation->GetDataAbilityPredicates is nullptr");
    } else {
        HILOG_INFO("Ability::ParsePredictionArgsReference operation->GetDataAbilityPredicates isn`t nullptr");
        strPredicatesList = predicates->GetWhereArgs();
    }

    if (strPredicatesList.empty()) {
        HILOG_ERROR("Ability::ParsePredictionArgsReference operation->GetDataAbilityPredicates()->GetWhereArgs()"
                 "error strList is empty()");
    }

    for (auto iterMap : predicatesBackReferencesMap) {
        HILOG_INFO(
            "Ability::ParsePredictionArgsReference predicatesBackReferencesMap first:%{public}d second:%{public}d",
            iterMap.first,
            iterMap.second);
        int tempCount = ChangeRef2Value(results, numRefs, iterMap.second);
        if (tempCount < 0) {
            HILOG_ERROR("Ability::ParsePredictionArgsReference tempCount:%{public}d", tempCount);
            continue;
        }
        std::string strPredicates = std::to_string(tempCount);
        HILOG_INFO("Ability::ParsePredictionArgsReference strPredicates:%{public}s", strPredicates.c_str());
        strPredicatesList.push_back(strPredicates);
        HILOG_INFO("Ability::ParsePredictionArgsReference push_back done");
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
        HILOG_ERROR("Ability::ParseValuesBucketReference intpur is nullptr");
        return nullptr;
    }

    if (operation->GetValuesBucketReferences() == nullptr) {
        return operation->GetValuesBucket();
    }

    retValueBucket.Clear();
    if (operation->GetValuesBucket() == nullptr) {
        HILOG_INFO("Ability::ParseValuesBucketReference operation->GetValuesBucket is nullptr");
    } else {
        HILOG_INFO("Ability::ParseValuesBucketReference operation->GetValuesBucket is nullptr");
        retValueBucket = *operation->GetValuesBucket();
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMapReferences;
    operation->GetValuesBucketReferences()->GetAll(valuesMapReferences);

    for (auto itermap : valuesMapReferences) {
        std::string key = itermap.first;
        NativeRdb::ValueObject obj;
        if (!operation->GetValuesBucketReferences()->GetObject(key, obj)) {
            HILOG_ERROR("Ability::ParseValuesBucketReference operation->GetValuesBucketReferences()->GetObject error");
            continue;
        }
        switch (obj.GetType()) {
            case NativeRdb::ValueObjectType::TYPE_INT: {
                int val = 0;
                if (obj.GetInt(val) != 0) {
                    HILOG_ERROR("Ability::ParseValuesBucketReference ValueObject->GetInt() error");
                    break;
                }
                HILOG_INFO("Ability::ParseValuesBucketReference retValueBucket->PutInt(%{public}s, %{public}d)",
                    key.c_str(),
                    val);
                retValueBucket.PutInt(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_DOUBLE: {
                double val = 0.0;
                if (obj.GetDouble(val) != 0) {
                    HILOG_ERROR("Ability::ParseValuesBucketReference ValueObject->GetDouble() error");
                    break;
                }
                HILOG_INFO("Ability::ParseValuesBucketReference retValueBucket->PutDouble(%{public}s, %{public}f)",
                    key.c_str(),
                    val);
                retValueBucket.PutDouble(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_STRING: {
                std::string val = "";
                if (obj.GetString(val) != 0) {
                    HILOG_ERROR("Ability::ParseValuesBucketReference ValueObject->GetString() error");
                    break;
                }
                HILOG_INFO("Ability::ParseValuesBucketReference retValueBucket->PutString(%{public}s, %{public}s)",
                    key.c_str(),
                    val.c_str());
                retValueBucket.PutString(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_BLOB: {
                std::vector<uint8_t> val;
                if (obj.GetBlob(val) != 0) {
                    HILOG_ERROR("Ability::ParseValuesBucketReference ValueObject->GetBlob() error");
                    break;
                }
                HILOG_INFO("Ability::ParseValuesBucketReference retValueBucket->PutBlob(%{public}s, %{public}zu)",
                    key.c_str(),
                    val.size());
                retValueBucket.PutBlob(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_BOOL: {
                bool val = false;
                if (obj.GetBool(val) != 0) {
                    HILOG_ERROR("Ability::ParseValuesBucketReference ValueObject->GetBool() error");
                    break;
                }
                HILOG_INFO("Ability::ParseValuesBucketReference retValueBucket->PutBool(%{public}s, %{public}s)",
                    key.c_str(),
                    val ? "true" : "false");
                retValueBucket.PutBool(key, val);
            } break;
            default: {
                HILOG_INFO("Ability::ParseValuesBucketReference retValueBucket->PutNull(%{public}s)", key.c_str());
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

/**
 * @brief request a remote object of callee from this ability.
 * @return Returns the remote object of callee.
 */
sptr<IRemoteObject> Ability::CallRequest()
{
    return nullptr;
}

ErrCode Ability::StartFeatureAbilityForResult(const Want &want, int requestCode, FeatureAbilityTask &&task)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = StartAbilityForResult(want, requestCode);
    HILOG_INFO("%{public}s end. ret=%{public}d", __func__, err);
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
    HILOG_INFO("%{public}s end.", __func__);
}

#ifdef SUPPORT_GRAPHICS
static std::mutex formLock;
/**
 * @brief Informs the system of the time required for drawing this Page ability.
 *
 * @return Returns the notification is successful or fail
 */
bool Ability::PrintDrawnCompleted()
{
    return AbilityContext::PrintDrawnCompleted();
}

/**
 * @brief Called after instantiating WindowScene.
 *
 *
 * You can override this function to implement your own processing logic.
 */
void Ability::OnSceneCreated()
{
    HILOG_INFO("%{public}s called.", __func__);
}

/**
 * @brief Called after restore WindowScene.
 *
 *
 * You can override this function to implement your own processing logic.
 */
void Ability::OnSceneRestored()
{
    HILOG_INFO("%{public}s called.", __func__);
}

/**
 * @brief Called after ability stoped.
 *
 *
 * You can override this function to implement your own processing logic.
 */
void Ability::onSceneDestroyed()
{
    HILOG_INFO("%{public}s called.", __func__);
}

/**
 * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
 *
 *
 * The ability in the <b>STATE_FOREGROUND</b> state is visible.
 * You can override this function to implement your own processing logic.
 */
void Ability::OnForeground(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    DoOnForeground(want);
    DispatchLifecycleOnForeground(want);
    HILOG_INFO("%{public}s end.", __func__);
    AAFWK::EventInfo eventInfo;
    eventInfo.bundleName = want.GetElement().GetBundleName();
    eventInfo.moduleName = want.GetElement().GetModuleName();
    eventInfo.abilityName = want.GetElement().GetAbilityName();
    AAFWK::EventReport::SendAbilityEvent(AAFWK::ABILITY_ONFOREGROUND,
        HiSysEventType::BEHAVIOR, eventInfo);
}

/**
 * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
 *
 *
 * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
 * You can override this function to implement your own processing logic.
 */
void Ability::OnBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        if (abilityInfo_->isStageBasedModel) {
            if (scene_ == nullptr) {
                HILOG_ERROR("Ability::OnBackground error. scene_ == nullptr.");
                return;
            }
            HILOG_INFO("GoBackground sceneFlag:%{public}d.", sceneFlag_);
            scene_->GoBackground(sceneFlag_);
        } else {
            if (abilityWindow_ == nullptr) {
                HILOG_ERROR("Ability::OnBackground error. abilityWindow_ == nullptr.");
                return;
            }
            HILOG_INFO("OnPostAbilityBackground sceneFlag:%{public}d.", sceneFlag_);
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
    HILOG_INFO("%{public}s end", __func__);
    AAFWK::EventInfo eventInfo;
    eventInfo.bundleName = abilityInfo_->bundleName;
    eventInfo.moduleName = abilityInfo_->moduleName;
    eventInfo.abilityName = abilityInfo_->name;
    AAFWK::EventReport::SendAbilityEvent(AAFWK::ABILITY_ONBACKGROUND,
        HiSysEventType::BEHAVIOR, eventInfo);
}

/**
 * @brief Called when a key is pressed. When any component in the Ability gains focus, the key-down event for
 * the component will be handled first. This callback will not be invoked if the callback triggered for the
 * key-down event of the component returns true. The default implementation of this callback does nothing
 * and returns false.
 *
 * @param keyEvent Indicates the key-down event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event
 * is not handled and should be passed to other handlers.
 */
void Ability::OnKeyDown(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    HILOG_INFO("Ability::OnKeyDown called");
}

/**
 * @brief Called when a key is released. When any component in the Ability gains focus, the key-up event for
 * the component will be handled first. This callback will not be invoked if the callback triggered for the
 * key-up event of the component returns true. The default implementation of this callback does nothing and
 * returns false.
 *
 * @param keyEvent Indicates the key-up event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event
 * is not handled and should be passed to other handlers.
 */
void Ability::OnKeyUp(const std::shared_ptr<MMI::KeyEvent>& keyEvent)
{
    HILOG_INFO("Ability::OnKeyUp called");
    auto code = keyEvent->GetKeyCode();
    if (code == MMI::KeyEvent::KEYCODE_BACK) {
        HILOG_INFO("Ability::OnKey Back key pressed.");
        OnBackPressed();
    }
}

/**
 * @brief Called when a touch event is dispatched to this ability. The default implementation of this callback
 * does nothing and returns false.
 *
 * @param event  Indicates information about the touch event.
 *
 * @return Returns true if the event is handled; returns false otherwise.
 */
void Ability::OnPointerEvent(std::shared_ptr<MMI::PointerEvent>& pointerEvent)
{
    HILOG_INFO("Ability::OnTouchEvent called");
}

/**
 * @brief Inflates UI controls by using ComponentContainer.
 * You can create a ComponentContainer instance that contains multiple components.
 *
 * @param componentContainer Indicates a set of customized components.
 */
void Ability::SetUIContent(const ComponentContainer &componentContainer)
{}

/**
 * @brief Inflates layout resources by using the layout resource ID.
 *
 * @param layoutRes Indicates the layout resource ID, which cannot be a negative number.
 */
void Ability::SetUIContent(int layoutRes)
{}

/**
 * @brief Inflates UI controls by using ComponentContainer.
 * You can create a ComponentContainer instance that contains multiple components.
 *
 * @param componentContainer Indicates the component layout defined by the user.
 * @param context Indicates the context to use.
 * @param typeFlag Indicates the window type.
 */
void Ability::SetUIContent(
    const ComponentContainer &componentContainer, std::shared_ptr<Context> &context, int typeFlag)
{}

/**
 * @brief Inflates layout resources by using the layout resource ID.
 *
 * @param layoutRes Indicates the layout resource ID, which cannot be a negative number.
 * @param context Indicates the context to use.
 * @param typeFlag Indicates the window type.
 */
void Ability::SetUIContent(int layoutRes, std::shared_ptr<Context> &context, int typeFlag)
{}

/**
 * @brief Inflates UI controls by using WindowOption.
 *
 * @param windowOption Indicates the window option defined by the user.
 */
void Ability::InitWindow(Rosen::WindowType winType, int32_t displayId, sptr<Rosen::WindowOption> option)
{
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("Ability::InitWindow abilityWindow_ is nullptr");
        return;
    }
    abilityWindow_->InitWindow(winType, abilityContext_, sceneListener_, displayId, option, securityFlag_);
}

/**
 * @brief Get the window belong to the ability.
 *
 * @return Returns a IWindowsManager object pointer.
 */
const sptr<Rosen::Window> Ability::GetWindow()
{
    if (abilityWindow_ != nullptr) {
        return abilityWindow_->GetWindow();
    } else {
        HILOG_INFO("%{public}s abilityWindow_ is nullptr.", __func__);
        return nullptr;
    }
}

/**
 * @brief get the scene belong to the ability.
 *
 * @return Returns a WindowScene object pointer.
 */
std::shared_ptr<Rosen::WindowScene> Ability::GetScene()
{
    return scene_;
}

/**
 * @brief Obtains the type of audio whose volume is adjusted by the volume button.
 *
 * @return Returns the AudioManager.AudioVolumeType.
 */
int Ability::GetVolumeTypeAdjustedByKey()
{
    return 0;
}

/**
 * @brief Checks whether the main window of this ability has window focus.
 *
 * @return Returns true if this ability currently has window focus; returns false otherwise.
 */
bool Ability::HasWindowFocus()
{
    if (abilityInfo_ == nullptr) {
        HILOG_INFO("Ability::HasWindowFocus abilityInfo_ == nullptr");
        return false;
    }

    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        return bWindowFocus_;
    }

    return false;
}

void Ability::SetShowOnLockScreen(bool showOnLockScreen)
{
    HILOG_INFO("SetShowOnLockScreen come, showOnLockScreen is %{public}d", showOnLockScreen);
    showOnLockScreen_ = showOnLockScreen;
    sptr<Rosen::Window> window = nullptr;
    if (abilityWindow_ == nullptr || (window = abilityWindow_->GetWindow()) == nullptr) {
        HILOG_INFO("SetShowOnLockScreen come, window is null");
        return;
    }
    HILOG_INFO("SetShowOnLockScreen come, addWindowFlag, showOnLockScreen is %{public}d", showOnLockScreen);
    if (showOnLockScreen) {
        window->AddWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    } else {
        window->RemoveWindowFlag(Rosen::WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    }
}

/**
 * @brief Called when a key is lone pressed.
 *
 * @param keyCode Indicates the code of the key long pressed.
 * @param keyEvent Indicates the key-long-press event.
 *
 * @return Returns true if this event is handled and will not be passed further; returns false if this event
 * is not handled and should be passed to other handlers.
 */
bool Ability::OnKeyPressAndHold(int keyCode, const std::shared_ptr<KeyEvent> &keyEvent)
{
    return false;
}

/**
 * @brief Called when this ability is about to leave the foreground and enter the background due to a user
 * operation, for example, when the user touches the Home key.
 *
 */
void Ability::OnLeaveForeground()
{}

/**
 * @brief Sets the type of audio whose volume will be adjusted by the volume button.
 *
 * @param volumeType Indicates the AudioManager.AudioVolumeType to set.
 */
void Ability::SetVolumeTypeAdjustedByKey(int volumeType)
{}

/**
 * @brief Sets the background color of the window in RGB color mode.
 *
 * @param red The value ranges from 0 to 255.
 *
 * @param green The value ranges from 0 to 255.
 *
 * @param blue The value ranges from 0 to 255.
 *
 * @return Returns the result of SetWindowBackgroundColor
 */
int Ability::SetWindowBackgroundColor(int red, int green, int blue)
{
    return -1;
}

/**
 * @brief Get page ability stack info.
 *
 * @return A string represents page ability stack info, empty if failed;
 */
std::string Ability::GetContentInfo()
{
    if (scene_ == nullptr) {
        return "";
    }
    return scene_->GetContentInfo();
}

/**
 * @brief Called when this ability gains or loses window focus.
 *
 * @param hasFocus Specifies whether this ability has focus.
 */
void Ability::OnWindowFocusChanged(bool hasFocus)
{}

/**
 * @brief Called when this ability is moved to or removed from the top of the stack.
 *
 * @param topActive Specifies whether this ability is moved to or removed from the top of the stack. The value true
 * indicates that it is moved to the top, and false indicates that it is removed from the top of the stack.
 */
void Ability::OnTopActiveAbilityChanged(bool topActive)
{}

/**
 * @brief Called to return a FormProviderInfo object.
 *
 * <p>You must override this method if your ability will serve as a form provider to provide a form for clients.
 * The default implementation returns nullptr. </p>
 *
 * @param want   Indicates the detailed information for creating a FormProviderInfo.
 *               The Want object must include the form ID, form name of the form,
 *               which can be obtained from Ability#PARAM_FORM_IDENTITY_KEY,
 *               Ability#PARAM_FORM_NAME_KEY, and Ability#PARAM_FORM_DIMENSION_KEY,
 *               respectively. Such form information must be managed as persistent data for further form
 *               acquisition, update, and deletion.
 *
 * @return Returns the created FormProviderInfo object.
 */
FormProviderInfo Ability::OnCreate(const Want &want)
{
    HILOG_INFO("%{public}s called.", __func__);
    FormProviderInfo formProviderInfo;
    return formProviderInfo;
}

/**
 * @brief Called to notify the form provider that a specified form has been deleted. Override this method if
 * you want your application, as the form provider, to be notified of form deletion.
 *
 * @param formId Indicates the ID of the deleted form.
 * @return None.
 */
void Ability::OnDelete(const int64_t formId)
{}

/**
 * @brief Called to notify the form provider to update a specified form.
 *
 * @param formId Indicates the ID of the form to update.
 * @return none.
 */
void Ability::OnUpdate(const int64_t formId)
{}

/**
 * @brief Called when the form supplier is notified that a temporary form is successfully converted to a normal
 * form.
 *
 * @param formId Indicates the ID of the form.
 * @return None.
 */
void Ability::OnCastTemptoNormal(const int64_t formId)
{}

/**
 * @brief Called when the form supplier receives form events from the fms.
 *
 * @param formEventsMap Indicates the form events occurred. The key in the Map object indicates the form ID,
 *                      and the value indicates the event type, which can be either FORM_VISIBLE
 *                      or FORM_INVISIBLE. FORM_VISIBLE means that the form becomes visible,
 *                      and FORM_INVISIBLE means that the form becomes invisible.
 * @return none.
 */
void Ability::OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap)
{}
/**
 * @brief Called to notify the form supplier to update a specified form.
 *
 * @param formId Indicates the ID of the form to update.
 * @param message Form event message.
 */
void Ability::OnTriggerEvent(const int64_t formId, const std::string &message)
{}

/**
 * @brief Called to notify the form supplier to acquire form state.
 *
 * @param want Indicates the detailed information about the form to be obtained, including
 *             the bundle name, module name, ability name, form name and form dimension.
 */
FormState Ability::OnAcquireFormState(const Want &want)
{
    return FormState::DEFAULT;
}

/**
 * @brief Acquire a form provider remote object.
 * @return Returns form provider remote object.
 */
sptr<IRemoteObject> Ability::GetFormRemoteObject()
{
    HILOG_INFO("%{public}s start", __func__);
    if (providerRemoteObject_ == nullptr) {
        sptr<FormProviderClient> providerClient = new (std::nothrow) FormProviderClient();
        std::shared_ptr<Ability> thisAbility = this->shared_from_this();
        if (thisAbility == nullptr) {
            HILOG_ERROR("%{public}s failed, thisAbility is nullptr", __func__);
        }
        providerClient->SetOwner(thisAbility);
        providerRemoteObject_ = providerClient->AsObject();
    }
    HILOG_INFO("%{public}s end", __func__);
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
    HILOG_INFO("Ability::GetWindowOption window mode is %{public}d.", windowMode);
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
        HILOG_INFO("Set window type for launcher");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_DESKTOP);
    }
    return option;
}

void Ability::DoOnForeground(const Want& want)
{
    if (abilityWindow_ != nullptr) {
        HILOG_INFO("%{public}s begin abilityWindow_->OnPostAbilityForeground, sceneFlag:%{public}d.",
            __func__, sceneFlag_);
        auto window = abilityWindow_->GetWindow();
        if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
            auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
                AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
            window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
            HILOG_INFO("set window mode = %{public}d.", windowMode);
        }
        abilityWindow_->OnPostAbilityForeground(sceneFlag_);
        HILOG_INFO("%{public}s end abilityWindow_->OnPostAbilityForeground.", __func__);
    } else {
        HILOG_INFO("========================abilityWindow_ != nullptr ======================");
    }
}

int Ability::GetCurrentWindowMode()
{
    HILOG_INFO("%{public}s start", __func__);
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

void Ability::OnCreate(Rosen::DisplayId displayId)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void Ability::OnDestroy(Rosen::DisplayId displayId)
{
    HILOG_INFO("%{public}s called.", __func__);
}

void Ability::OnChange(Rosen::DisplayId displayId)
{
    HILOG_INFO("%{public}s start, displayId: %{public}" PRIu64"", __func__,
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
            resConfig->SetScreenDensity(ConvertDensity(density));
            resConfig->SetDirection(ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            HILOG_INFO("%{public}s Notify ResourceManager, Density: %{public}d, Direction: %{public}d.", __func__,
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
    uint32_t size = changeKeyV.size();
    HILOG_INFO("changeKeyV size :%{public}u", size);
    if (!changeKeyV.empty()) {
        configuration->Merge(changeKeyV, newConfig);
        auto task = [ability = shared_from_this(), configuration = *configuration]() {
            ability->OnConfigurationUpdated(configuration);
        };
        handler_->PostTask(task);
    }

    HILOG_INFO("%{public}s end", __func__);
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
            resConfig->SetScreenDensity(ConvertDensity(density));
            resConfig->SetDirection(ConvertDirection(height, width));
            resourceManager->UpdateResConfig(*resConfig);
            HILOG_INFO("%{public}s Notify ResourceManager, Density: %{public}d, Direction: %{public}d.", __func__,
                resConfig->GetScreenDensity(), resConfig->GetDirection());
        }
    }

    Configuration newConfig;
    newConfig.AddItem(ConfigurationInner::APPLICATION_DISPLAYID, std::to_string(to));
    newConfig.AddItem(to, ConfigurationInner::APPLICATION_DIRECTION, GetDirectionStr(height, width));
    newConfig.AddItem(to, ConfigurationInner::APPLICATION_DENSITYDPI, GetDensityStr(density));

    std::vector<std::string> changeKeyV;
    auto configuration = application_->GetConfiguration();
    if (!configuration) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }

    configuration->CompareDifferent(changeKeyV, newConfig);
    uint32_t size = changeKeyV.size();
    HILOG_INFO("changeKeyV size :%{public}u", size);
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
    HILOG_INFO("%{public}s called.", __func__);
    if (abilityWindow_ == nullptr) {
        return;
    }
    auto window = abilityWindow_->GetWindow();
    if (window != nullptr && want.HasParameter(Want::PARAM_RESV_WINDOW_MODE)) {
        auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
            AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
        window->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));
        HILOG_INFO("set window mode = %{public}d.", windowMode);
    }
    abilityWindow_->OnPostAbilityForeground(sceneFlag_);
}

void Ability::SetWakeUpScreen(bool wakeUp)
{
    HILOG_INFO("SetWakeUpScreen wakeUp:%{public}d.", wakeUp);
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("SetWakeUpScreen error. abilityWindow_ == nullptr.");
        return;
    }
    HILOG_DEBUG("FA mode");
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        HILOG_ERROR("window nullptr.");
        return;
    }
    window->SetTurnScreenOn(wakeUp);
}

void Ability::SetDisplayOrientation(int orientation)
{
    HILOG_DEBUG("%{public}s called, orientation: %{public}d", __func__, orientation);
    if (abilityWindow_ == nullptr) {
        HILOG_ERROR("Ability::SetDisplayOrientation error. abilityWindow_ == nullptr.");
        return;
    }
    HILOG_DEBUG("FA mode");
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr.");
        return;
    }
    if (orientation == static_cast<int>(DisplayOrientation::FOLLOWRECENT)) {
        int defualtOrientation = 0;
        if (setWant_) {
            orientation = setWant_->GetIntParam("ohos.aafwk.Orientation", defualtOrientation);
        } else {
            orientation = defualtOrientation;
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
        return 0;
    }
    HILOG_DEBUG("FA mode");
    auto window = abilityWindow_->GetWindow();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr.");
        return 0;
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
    HILOG_INFO("%{public}s called.", __func__);
}
#endif
}  // namespace AppExecFwk
}  // namespace OHOS
