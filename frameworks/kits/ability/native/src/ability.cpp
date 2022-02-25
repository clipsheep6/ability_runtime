/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "app_log_wrapper.h"
#include "bytrace.h"
#include "connection_manager.h"
#include "context_impl.h"
#include "continuation_manager.h"
#include "continuation_register_manager.h"
#include "continuation_register_manager_proxy.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "data_uri_utils.h"
#include "display_type.h"
#include "distributed_objectstore.h"
#include "form_host_client.h"
#include "form_mgr.h"
#include "form_provider_client.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "key_event.h"
#include "ohos_application.h"
#include "permission/permission.h"
#include "permission/permission_kit.h"
#include "reverse_continuation_scheduler_primary.h"
#include "reverse_continuation_scheduler_replica.h"
#include "reverse_continuation_scheduler_replica_handler_interface.h"
#include "runtime.h"
#include "string_wrapper.h"
#include "system_ability_definition.h"
#include "task_handler_client.h"
#include "touch_event.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using PermissionKit = OHOS::Security::Permission::PermissionKit;
using PermissionState = OHOS::Security::Permission::PermissionState;

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

static std::mutex formLock;

constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MILLISEC_TO_NANOSEC = 1000000;
constexpr int32_t DISTRIBUTED_OBJECT_TIMEOUT = 3000;

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

void Ability::Init(const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler, const sptr<IRemoteObject> &token)
{
    APP_LOGI("%{public}s begin.", __func__);
    abilityInfo_ = abilityInfo;
    handler_ = handler;
    AbilityContext::token_ = token;

    // page ability only.
    if (abilityInfo_->type == AbilityType::PAGE) {
        if (!abilityInfo_->isStageBasedModel) {
            abilityWindow_ = std::make_shared<AbilityWindow>();
            if (abilityWindow_ != nullptr) {
                APP_LOGI("%{public}s begin abilityWindow_->Init", __func__);
                abilityWindow_->Init(handler_, shared_from_this());
                APP_LOGI("%{public}s end abilityWindow_->Init", __func__);
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
            sptr<ReverseContinuationSchedulerPrimary> Primary = sptr<ReverseContinuationSchedulerPrimary>(
                new (std::nothrow) ReverseContinuationSchedulerPrimary(continuationHandler, handler_));
            if (Primary == nullptr) {
                APP_LOGE("Ability::Init failed,Primary create failed");
            } else {
                continuationHandler_->SetPrimaryStub(Primary);
                continuationHandler_->SetAbilityInfo(abilityInfo_);
            }
        }
    }
    lifecycle_ = std::make_shared<LifeCycle>();
    abilityLifecycleExecutor_ = std::make_shared<AbilityLifecycleExecutor>();
    if (abilityLifecycleExecutor_ != nullptr) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);
    } else {
        APP_LOGE("%{public}s abilityLifecycleExecutor_ make failed.", __func__);
    }

    application_ = application;
    if (abilityContext_ != nullptr) {
        abilityContext_->RegisterAbilityCallback(weak_from_this());
    }
    APP_LOGI("%{public}s end.", __func__);
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
 * Will be called when ability start. You should override this function
 *
 * @param want ability start information
 */
void Ability::OnStart(const Want &want)
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::OnStart falied abilityInfo_ is nullptr.");
        return;
    }

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
            winType = Rosen::WindowType::WINDOW_TYPE_WALLPAPER;
        }

        int defualtDisplayId = Rosen::WindowScene::DEFAULT_DISPLAY_ID;
        int displayId = want.GetIntParam(StartOptions::STRING_DISPLAY_ID, defualtDisplayId);
        APP_LOGI("Ability::OnStart bundleName:%{public}s, abilityName:%{public}s, windowType:%{public}d, "
            "displayId:%{public}d",
            abilityInfo_->bundleName.c_str(),
            abilityInfo_->name.c_str(),
            winType,
            displayId);
        auto option = GetWindowOption(want);
        InitWindow(winType, displayId, option);

        if (abilityWindow_ != nullptr) {
            APP_LOGI("%{public}s begin abilityWindow_->OnPostAbilityStart.", __func__);
            abilityWindow_->OnPostAbilityStart();
            if (abilityWindow_->GetWindow()) {
                auto windowId = abilityWindow_->GetWindow()->GetWindowId();
                APP_LOGI("Ability::OnStart: add window info  = %{public}d", windowId);
                OHOS::AAFwk::AbilityManagerClient::GetInstance()->AddWindowInfo(AbilityContext::GetToken(), windowId);
            }
            APP_LOGI("%{public}s end abilityWindow_->OnPostAbilityStart.", __func__);
        }
    }

    SetWant(want);
    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnStart error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (!abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);
    }

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnStart error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_START, want);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Called when this ability enters the <b>STATE_STOP</b> state.
 *
 * The ability in the <b>STATE_STOP</b> is being destroyed.
 * You can override this function to implement your own processing logic.
 */
void Ability::OnStop()
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnStop error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnStop error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_STOP);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Release the window and ability.
 */
void Ability::Destroy()
{
    APP_LOGI("%{public}s begin.", __func__);
    // Release the scene.
    if (scene_ != nullptr) {
        scene_->GoDestroy();
        scene_ = nullptr;
        onSceneDestroyed();
    }

    // Release the window.
    if (abilityWindow_ != nullptr && abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        abilityWindow_->OnPostAbilityStop(); // Ability will been released when window destroy.
    }
    APP_LOGI("%{public}s end.", __func__);
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
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityWindow_ != nullptr) {
        APP_LOGI("%{public}s begin abilityWindow_->OnPostAbilityActive.", __func__);
        abilityWindow_->OnPostAbilityActive();
        APP_LOGI("%{public}s end abilityWindow_->OnPostAbilityActive.", __func__);
    }
    bWindowFocus_ = true;
    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnActive error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnActive error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Called when this ability enters the <b>STATE_INACTIVE</b> state.
 *
 * <b>STATE_INACTIVE</b> is an instantaneous state. The ability in this state may be visible but does not have
 * focus.You can override this function to implement your own processing logic.
 */
void Ability::OnInactive()
{
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityWindow_ != nullptr && abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        APP_LOGI("%{public}s begin abilityWindow_->OnPostAbilityInactive.", __func__);
        abilityWindow_->OnPostAbilityInactive();
        APP_LOGI("%{public}s end abilityWindow_->OnPostAbilityInactive.", __func__);
    }
    bWindowFocus_ = false;
    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnInactive error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnInactive error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_INACTIVE);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Called after instantiating WindowScene.
 *
 *
 * You can override this function to implement your own processing logic.
 */
void Ability::OnSceneCreated()
{
    APP_LOGI("%{public}s called.", __func__);
}

/**
 * @brief Called after restore WindowScene.
 *
 *
 * You can override this function to implement your own processing logic.
 */
void Ability::OnSceneRestored()
{
    APP_LOGI("%{public}s called.", __func__);
}

/**
 * @brief Called after ability stoped.
 *
 *
 * You can override this function to implement your own processing logic.
 */
void Ability::onSceneDestroyed()
{
    APP_LOGI("%{public}s called.", __func__);
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
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    DoOnForeground(want);
    DispatchLifecycleOnForeground(want);
    APP_LOGI("%{public}s end.", __func__);
}

bool Ability::IsRestoredInContinuation() const
{
    if (abilityContext_ == nullptr) {
        APP_LOGE("abilityContext_ is null");
        return false;
    }
    if (launchParam_.launchReason == LaunchReason::LAUNCHREASON_CONTINUATION
        && abilityInfo_->launchMode == LaunchMode::STANDARD
        && abilityContext_->GetContentStorage() != nullptr) {
        APP_LOGI("Is Restored In Continuation");
        return true;
    }
    APP_LOGI("not Restored In Continuation");
    return false;
}

void Ability::WaitingDistributedObjectSyncComplete(const Want& want)
{
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);

    APP_LOGI("continuation WaitingDistributedObjectSyncComplete begin");
    auto timeout = [self = shared_from_this(), sessionId, originDeviceId]() {
        APP_LOGI("DistributedObject sync timeout");
        self->continuationManager_->NotifyCompleteContinuation(
            originDeviceId, sessionId, false, nullptr);
    };

    // std::shared_ptr<AppExecFwk::EventHandler> handler = handler_;
    auto callback = [self = shared_from_this(), sessionId, originDeviceId]() {
        APP_LOGI("DistributedObject sync complete");
        if (self->handler_ != nullptr) {
            self->handler_->RemoveTask("Waiting_Sync_Timeout");
        }
        self->continuationManager_->NotifyCompleteContinuation(
            originDeviceId, sessionId, true, nullptr);
    };

    std::string &bundleName = abilityInfo_->bundleName;
    APP_LOGI("continuation TriggerRestore begin");
    ObjectStore::DistributedObjectStore::GetInstance(bundleName)->TriggerRestore(callback);
    APP_LOGI("continuation TriggerRestore end");

    if (handler_ != nullptr) {
        APP_LOGI("continuation set timeout begin");
        handler_->PostTask(timeout, "Waiting_Sync_Timeout", DISTRIBUTED_OBJECT_TIMEOUT);
        APP_LOGI("continuation set timeout end");
    }
}

void Ability::NotityContinuationResult(const Want& want, bool success)
{
    APP_LOGI("NotityContinuationResult begin");
    std::weak_ptr<IReverseContinuationSchedulerReplicaHandler> ReplicaHandler = continuationHandler_;
    reverseContinuationSchedulerReplica_ = sptr<ReverseContinuationSchedulerReplica>(
        new (std::nothrow) ReverseContinuationSchedulerReplica(handler_, ReplicaHandler));

    if (reverseContinuationSchedulerReplica_ == nullptr) {
        APP_LOGE("Ability::NotityContinuationComplete failed, create reverseContinuationSchedulerReplica failed");
        return;
    }
    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    APP_LOGI("Ability::NotityContinuationComplete DeviceId: %{public}s", originDeviceId.c_str());
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
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
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        APP_LOGI("%{public}s begin OnPostAbilityBackground.", __func__);
        if (abilityInfo_->isStageBasedModel) {
            if (scene_ == nullptr) {
                APP_LOGE("Ability::OnBackground error. scene_ == nullptr.");
                return;
            }
            APP_LOGI("GoBackground sceneFlag:%{public}d.", sceneFlag_);
            scene_->GoBackground(sceneFlag_);
        } else {
            if (abilityWindow_ == nullptr) {
                APP_LOGE("Ability::OnBackground error. abilityWindow_ == nullptr.");
                return;
            }
            APP_LOGI("OnPostAbilityBackground sceneFlag:%{public}d.", sceneFlag_);
            abilityWindow_->OnPostAbilityBackground(sceneFlag_);
        }
        APP_LOGI("%{public}s end OnPostAbilityBackground.", __func__);
    }

    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnBackground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }

    if (abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND);
    }

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnBackground error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_BACKGROUND);
    APP_LOGI("%{public}s end.", __func__);
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
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnConnect error. abilityLifecycleExecutor_ == nullptr.");
        return nullptr;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnConnect error. lifecycle_ == nullptr.");
        return nullptr;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    APP_LOGI("%{public}s end.", __func__);
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
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
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
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::StartAbilityForResult abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    APP_LOGI("Ability::StartAbilityForResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        APP_LOGE("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode);
    APP_LOGI("%{public}s end.", __func__);
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
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::StartAbilityForResult abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    APP_LOGI("Ability::StartAbilityForResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE) {
        APP_LOGE("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, requestCode, abilityStartSetting);
    APP_LOGI("%{public}s end.", __func__);
    return err;
}

/**
 * Starts a new ability with specific start settings.
 * A Page or Service ability uses this method to start a specific ability.
 * The system locates the target ability from installed abilities based on
 * the value of the intent parameter and then starts it. You can specify the
 * ability to start using the intent parameter.
 *
 * @param want Indicates the ability to start.
 * @param abilityStartSetting Indicates the setting ability used to start.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::StartAbility(const Want &want, AbilityStartSetting abilityStartSetting)
{
    APP_LOGI("%{public}s beign.", __func__);
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::StartAbility abilityInfo_ == nullptr");
        return ERR_NULL_OBJECT;
    }
    APP_LOGI("Ability::StartAbility called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type != AppExecFwk::AbilityType::PAGE && abilityInfo_->type != AppExecFwk::AbilityType::SERVICE) {
        APP_LOGE("Ability::StartAbility ability type: %{public}d", abilityInfo_->type);
        return ERR_INVALID_VALUE;
    }
    ErrCode err = AbilityContext::StartAbility(want, -1, abilityStartSetting);
    APP_LOGI("%{public}s end.", __func__);
    return err;
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
    APP_LOGI("Ability::OnKeyDown called");
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
    APP_LOGI("Ability::OnKeyUp called");
    auto code = keyEvent->GetKeyCode();
    if (code == MMI::KeyEvent::KEYCODE_BACK) {
        APP_LOGI("Ability::OnKey Back key pressed.");
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
    APP_LOGI("Ability::OnTouchEvent called");
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
        APP_LOGE("Ability::InitWindow abilityWindow_ is nullptr");
        return;
    }
    bool useNewMission = AbilityImpl::IsUseNewMission();
    APP_LOGI("%{public}s beign abilityWindow_->InitWindow.", __func__);
    if (useNewMission) {
        abilityWindow_->InitWindow(winType, abilityContext_, sceneListener_, displayId, option);
    } else {
        std::shared_ptr<AbilityRuntime::AbilityContext> context = nullptr;
        sptr<Rosen::IWindowLifeCycle> listener = nullptr;
        abilityWindow_->InitWindow(winType, context, listener, displayId, option);
    }
    APP_LOGI("%{public}s end abilityWindow_->InitWindow.", __func__);
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
        APP_LOGI("%{public}s abilityWindow_ is nullptr.", __func__);
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
        APP_LOGI("Ability::HasWindowFocus abilityInfo_ == nullptr");
        return false;
    }

    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        return bWindowFocus_;
    }

    return false;
}

/**
 * @description: Obtains api version based on ability.
 * @return api version.
 */
int Ability::GetCompatibleVersion()
{
    return compatibleVersion_;
}

/**
 * @description: Set api version in an ability.
 * @param compatibleVersion api version
 * @return None.
 */
void Ability::SetCompatibleVersion(int32_t compatibleVersion)
{
    compatibleVersion_ = compatibleVersion;
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
 * @brief Called when this ability is about to leave the foreground and enter the background due to a user
 * operation, for example, when the user touches the Home key.
 *
 */
void Ability::OnLeaveForeground()
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

/**
 * @brief Called when the system configuration is updated.
 *
 * @param configuration Indicates the updated configuration information.
 */
void Ability::OnConfigurationUpdated(const Configuration &configuration)
{
    APP_LOGI("%{public}s called.", __func__);
}

void Ability::OnConfigurationUpdatedNotify(const Configuration &changeConfiguration)
{
    APP_LOGI("%{public}s begin.", __func__);

    std::string language;
    if (setting_) {
        auto displayId = std::atoi(setting_->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str());
        language = changeConfiguration.GetItem(displayId, GlobalConfigurationKey::SYSTEM_LANGUAGE);
        APP_LOGI("displayId :[%{public}d] | language :[%{public}s]", displayId, language.c_str());
    } else {
        language = changeConfiguration.GetItem(GlobalConfigurationKey::SYSTEM_LANGUAGE);
        APP_LOGI("language :[%{public}s]", language.c_str());
    }

    // Notify ResourceManager
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig != nullptr && !language.empty()) {
        APP_LOGE("make resource mgr date");
        UErrorCode status = U_ZERO_ERROR;
        icu::Locale locale = icu::Locale::forLanguageTag(language, status);
        APP_LOGI("get Locale::forLanguageTag return[%{public}d].", static_cast<int>(status));
        if (status == U_ZERO_ERROR) {
            resConfig->SetLocaleInfo(locale);
            auto resourceManager = GetResourceManager();
            if (resourceManager != nullptr) {
                resourceManager->UpdateResConfig(*resConfig);
                APP_LOGI("%{public}s Notify ResourceManager.", __func__);
            }
        }
    }

    // Notify WindowScene
    if (scene_ != nullptr && !language.empty()) {
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(changeConfiguration);
        scene_->UpdateConfiguration(diffConfiguration);
        APP_LOGE("%{public}s scene_ -> UpdateConfiguration success.", __func__);
    }

    if (abilityContext_ != nullptr && application_ != nullptr) {
        abilityContext_->SetConfiguration(application_->GetConfiguration());
    }
    // Notify Ability Subclass
    OnConfigurationUpdated(changeConfiguration);
    APP_LOGI("%{public}s Notify Ability Subclass.", __func__);
}

/**
 * @brief Called when the system configuration is updated.
 *
 * @param level Indicates the memory trim level, which shows the current memory usage status.
 *
 */
void Ability::OnMemoryLevel(int level)
{}

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
    APP_LOGI("%{public}s begin.", __func__);
    if (application_ == nullptr) {
        APP_LOGE("Ability::GetApplication error. application_ == nullptr.");
        return nullptr;
    }
    APP_LOGI("%{public}s end.", __func__);
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
        APP_LOGE("Ability::GetAbilityName abilityInfo_ is nullptr");
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
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::OnBackPressed abilityInfo_ is nullptr");
        return;
    }

    if (abilityInfo_->isLauncherAbility == false) {
        APP_LOGI("Ability::OnBackPressed the ability is not Launcher");
        TerminateAbility();
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
 * ability that has been at the top of the ability stack.
 *
 * @param want Indicates the new Want containing information about the ability.
 */
void Ability::OnNewWant(const Want &want)
{
    APP_LOGI("Ability::OnNewWant called");
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
    APP_LOGI("Ability::OnRestoreAbilityState called");
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
    APP_LOGI("Ability::OnSaveAbilityState called");
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
    APP_LOGI("%{public}s begin.", __func__);
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::SetResult abilityInfo_ == nullptr");
        return;
    }
    APP_LOGI("Ability::SetResult called type = %{public}d", abilityInfo_->type);
    if (abilityInfo_->type == AppExecFwk::AbilityType::PAGE) {
        AbilityContext::resultWant_ = resultData;
        AbilityContext::resultCode_ = resultCode;
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Sets the type of audio whose volume will be adjusted by the volume button.
 *
 * @param volumeType Indicates the AudioManager.AudioVolumeType to set.
 */
void Ability::SetVolumeTypeAdjustedByKey(int volumeType)
{}

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
    BYTRACE_NAME(BYTRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    APP_LOGI("%{public}s begin restart=%{public}s,startId=%{public}d.", __func__, restart ? "true" : "false", startId);
    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::OnCommand error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::ACTIVE);

    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnCommand error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_ACTIVE);
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief dump ability info
 *
 * @param extra dump ability info
 */
void Ability::Dump(const std::string &extra)
{
    APP_LOGI("Ability::Dump called");
    // abilityInfo
    APP_LOGI("==============AbilityInfo==============");
    if (abilityInfo_ != nullptr) {
        APP_LOGI("abilityInfo: package: %{public}s", abilityInfo_->package.c_str());
        APP_LOGI("abilityInfo: name: %{public}s", abilityInfo_->name.c_str());
        APP_LOGI("abilityInfo: label: %{public}s", abilityInfo_->label.c_str());
        APP_LOGI("abilityInfo: description: %{public}s", abilityInfo_->description.c_str());
        APP_LOGI("abilityInfo: iconPath: %{public}s", abilityInfo_->iconPath.c_str());
        APP_LOGI("abilityInfo: visible: %{public}d", abilityInfo_->visible);
        APP_LOGI("abilityInfo: kind: %{public}s", abilityInfo_->kind.c_str());
        APP_LOGI("abilityInfo: type: %{public}d", abilityInfo_->type);
        APP_LOGI("abilityInfo: orientation: %{public}d", abilityInfo_->orientation);
        APP_LOGI("abilityInfo: launchMode: %{public}d", abilityInfo_->launchMode);
        for (auto permission : abilityInfo_->permissions) {
            APP_LOGI("abilityInfo: permission: %{public}s", permission.c_str());
        }
        APP_LOGI("abilityInfo: bundleName: %{public}s", abilityInfo_->bundleName.c_str());
        APP_LOGI("abilityInfo: applicationName: %{public}s", abilityInfo_->applicationName.c_str());
    } else {
        APP_LOGI("abilityInfo is nullptr");
    }

    // lifecycle_Event
    APP_LOGI("==============lifecycle_Event==============");
    if (lifecycle_ != nullptr) {
        APP_LOGI("lifecycle_Event: launchMode: %{public}d", lifecycle_->GetLifecycleState());
    } else {
        APP_LOGI("lifecycle is nullptr");
    }

    // lifecycle_State
    APP_LOGI("==============lifecycle_State==============");
    if (abilityLifecycleExecutor_ != nullptr) {
        APP_LOGI("lifecycle_State: launchMode: %{public}d", abilityLifecycleExecutor_->GetState());
    } else {
        APP_LOGI("abilityLifecycleExecutor is nullptr");
    }

    // applicationInfo
    APP_LOGI("==============applicationInfo==============");
    std::shared_ptr<ApplicationInfo> ApplicationInfoPtr = GetApplicationInfo();
    if (ApplicationInfoPtr != nullptr) {
        APP_LOGI("applicationInfo: name: %{public}s", ApplicationInfoPtr->name.c_str());
        APP_LOGI("applicationInfo: bundleName: %{public}s", ApplicationInfoPtr->bundleName.c_str());
    } else {
        APP_LOGI("ApplicationInfoPtr is nullptr");
    }
}

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
    APP_LOGI("%{public}s begin.", __func__);
    int amount = 0;
    for (auto it = values.begin(); it != values.end(); it++) {
        if (Insert(uri, *it) >= 0) {
            amount++;
        }
    }
    APP_LOGI("%{public}s end, amount=%{public}d", __func__, amount);
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
        APP_LOGE("Ability::ContinueAbilityReversibly(deviceId) failed. VerifySupportForContinuation faled");
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
        APP_LOGE("Ability::GetContinuationState failed. VerifySupportForContinuation faled");
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
    APP_LOGI("Ability::GetLifecycle called");
    return lifecycle_;
}

/**
 * @brief Obtains the lifecycle state of this ability.
 *
 * @return Returns the lifecycle state of this ability.
 */
AbilityLifecycleExecutor::LifecycleState Ability::GetState()
{
    APP_LOGI("Ability::GetState called");

    if (abilityLifecycleExecutor_ == nullptr) {
        APP_LOGE("Ability::GetState error. abilityLifecycleExecutor_ == nullptr.");
        return AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }

    return (AbilityLifecycleExecutor::LifecycleState)abilityLifecycleExecutor_->GetState();
}

/**
 * @brief A Page or Service ability uses this method to start a specific ability. The system locates the target
 * ability from installed abilities based on the value of the intent parameter and then starts it. You can specify
 * the ability to start using the intent parameter.
 *
 * @param intent Indicates the ability to start.
 *
 * @return errCode ERR_OK on success, others on failure.
 */
ErrCode Ability::StartAbility(const Want &want)
{
    APP_LOGI("%{public}s begin Ability::StartAbility", __func__);
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
    APP_LOGI("%{public}s begin Ability::TerminateAbility", __func__);
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
 * @brief By binding an action, you can set different action parameters in Intent to present different initial
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
    APP_LOGI("%{public}s begin.", __func__);
    TaskHandlerClient::GetInstance()->PostTask(task, delayTime);
    APP_LOGI("%{public}s end.", __func__);
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
 * @return If the user allows migration and saves data suscessfully, it returns true; otherwise, it returns false.
 */
bool Ability::OnContinue(WantParams &wantParams)
{
    return false;
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
 * @brief Migrates this ability to the given device on the same distributed network. The ability to migrate and its
 * ability slices must implement the IAbilityContinuation interface.
 *
 * @param deviceId Indicates the ID of the target device where this ability will be migrated to. If this parameter
 * is null, this method has the same effect as continueAbility().
 *
 */
void Ability::ContinueAbilityWithStack(const std::string &deviceId)
{
    if (deviceId.empty()) {
        APP_LOGE("ContinueAbilityWithStack(deviceId) failed. deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        APP_LOGE("ContinueAbilityWithStack(deviceId) failed. VerifySupportForContinuation failed");
        return;
    }
    continuationManager_->ContinueAbilityWithStack(deviceId);
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
        APP_LOGE("Ability::ContinueAbility(deviceId) failed. deviceId is empty");
        return;
    }

    if (!VerifySupportForContinuation()) {
        APP_LOGE("Ability::ContinueAbility(deviceId) failed. VerifySupportForContinuation failed");
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
    APP_LOGI("Ability::OnCompleteContinuation change continuation state to initial");
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
        APP_LOGE("Ability::OnForeground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    }
    if (lifecycle_ == nullptr) {
        APP_LOGE("Ability::OnForeground error. lifecycle_ == nullptr.");
        return;
    }
    lifecycle_->DispatchLifecycle(LifeCycle::Event::ON_FOREGROUND, want);
}

bool Ability::VerifySupportForContinuation()
{
    if (continuationManager_ == nullptr) {
        APP_LOGE("Ability::VerifySupportForContinuation failed. continuationManager_ is nullptr");
        return false;
    }
    return true;
}

void Ability::HandleCreateAsContinuation(const Want &want)
{
    if (!IsFlagExists(Want::FLAG_ABILITY_CONTINUATION, want.GetFlags())) {
        APP_LOGI("Ability::HandleCreateAsContinuation return. This not continuated ability");
        return;
    }

    // check whether it needs reversible
    bool reversible = false;
    reversible = IsFlagExists(Want::FLAG_ABILITY_CONTINUATION_REVERSIBLE, want.GetFlags());
    if (!VerifySupportForContinuation()) {
        APP_LOGE("Ability::HandleCreateAsContinuation failed. VerifySupportForContinuation failed");
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
            APP_LOGE("Ability::HandleCreateAsContinuation failed, create reverseContinuationSchedulerReplica failed");
            return;
        }
    }

    int sessionId = want.GetIntParam(DMS_SESSION_ID, DEFAULT_DMS_SESSION_ID);
    std::string originDeviceId = want.GetStringParam(DMS_ORIGIN_DEVICE_ID);
    APP_LOGE("Ability::HandleCreateAsContinuationoriginDeviceId: %{public}s", originDeviceId.c_str());
    continuationManager_->NotifyCompleteContinuation(
        originDeviceId, sessionId, success, reverseContinuationSchedulerReplica_);
}
bool Ability::IsFlagExists(unsigned int flag, unsigned int flagSet)
{
    return (flag & flagSet) == flag;
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
 * @brief Called to set caller information for the application. The default implementation returns null.
 *
 * @return Returns the caller information.
 */
Uri Ability::OnSetCaller()
{
    return Uri("");
}

/**
 * @brief Call this when your ability should be closed and the mission should be completely removed as a part of
 * finishing the root ability of the mission.
 */
void Ability::TerminateAndRemoveMission()
{
    APP_LOGI("%{public}s begin.", __func__);
    auto state = GetState();
    if (state > AbilityLifecycleExecutor::LifecycleState::INITIAL) {
        APP_LOGI("Ability::TerminateAndRemoveMission the GetState retval is %d", state);
        return;
    }
    AbilityContext::TerminateAndRemoveMission();
    APP_LOGI("%{public}s end.", __func__);
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
 * Releases an obtained form by its ID.
 *
 * <p>After this method is called, the form won't be available for use by the application, but the Form Manager
 * Service still keeps the cache information about the form, so that the application can quickly obtain it based on
 * the {@code formId}.</p>
 * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
 *
 * @param formId Indicates the form ID.
 * @return Returns {@code true} if the form is successfully released; returns {@code false} otherwise.
 *
 * <ul>
 * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
 * <li>The specified form has not been added by the application.</li>
 * <li>An error occurred when connecting to the Form Manager Service.</li>
 * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
 * <li>The form has been obtained by another application and cannot be released by the current application.</li>
 * <li>The form is being restored.</li>
 * </ul>
 */
bool Ability::ReleaseForm(const int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);
    // release form by formId and do not release cache
    return ReleaseForm(formId, false);
}

/**
 * Releases an obtained form by its ID.
 *
 * <p>After this method is called, the form won't be available for use by the application, if isReleaseCache is
 * false, this method is same as {@link #releaseForm(int)}, otherwise the Form Manager Service still store this
 * form in the cache.</p>
 * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
 *
 * @param formId Indicates the form ID.
 * @param isReleaseCache Indicates whether to clear cache in service.
 * @return Returns {@code true} if the form is successfully released; returns {@code false} otherwise.
 *
 * <ul>
 * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
 * <li>The specified form has not been added by the application.</li>
 * <li>An error occurred when connecting to the Form Manager Service.</li>
 * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
 * <li>The form has been obtained by another application and cannot be released by the current application.</li>
 * <li>The form is being restored.</li>
 * </ul>
 */
bool Ability::ReleaseForm(const int64_t formId, const bool isReleaseCache)
{
    APP_LOGI("%{public}s called.", __func__);
    // release form with formId and specifies whether to release the cache
    return DeleteForm(formId, isReleaseCache ? RELEASE_CACHED_FORM : RELEASE_FORM);
}

/**
 * Deletes an obtained form by its ID.
 *
 * <p>After this method is called, the form won't be available for use by the application and the Form Manager
 * Service no longer keeps the cache information about the form.</p>
 * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
 *
 * @param formId Indicates the form ID.
 * @return Returns {@code true} if the form is successfully deleted; returns {@code false} otherwise.
 *
 * <ul>
 * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
 * <li>The specified form has not been added by the application.</li>
 * <li>An error occurred when connecting to the Form Manager Service.</li>
 * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
 * <li>The form has been obtained by another application and cannot be deleted by the current application.</li>
 * <li>The form is being restored.</li>
 * </ul>
 */
bool Ability::DeleteForm(const int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);
    // delete form with formId
    return DeleteForm(formId, DELETE_FORM);
}

/**
 * @brief Cast temp form with formId.
 *
 * @param formId Indicates the form's ID.
 *
 * @return Returns {@code true} if the form is successfully casted; returns {@code false} otherwise.
 */
bool Ability::CastTempForm(const int64_t formId)
{
    APP_LOGI("%{public}s start", __func__);
    if (formId <= 0) {
        APP_LOGE("%{public}s error, passing in form id can't be negative.", __func__);
        return false;
    }

    APP_LOGI("%{public}s, castTempForm begin of temp form %{public}" PRId64, __func__, formId);
    int result = FormMgr::GetInstance().CastTempForm(formId, FormHostClient::GetInstance());

    if (result != ERR_OK) {
        APP_LOGE("%{public}s error, some internal server occurs, error code is %{public}d.", __func__, result);
        return false;
    }

    userReqParams_[formId].SetParam(Constants::PARAM_FORM_TEMPORARY_KEY, false);

    APP_LOGI("%{public}s end", __func__);
    return true;
}

/**
 * @brief Obtains a specified form that matches the application bundle name, module name, form name, and
 * other related information specified in the passed {@code Want}.
 *
 * <p>This method is asynchronous. After the {@link FormJsInfo} instance is obtained.
 *
 * @param formId Indicates the form ID.
 * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
 *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
 * @param callback Indicates the callback to be invoked whenever the {@link FormJsInfo} instance is obtained.
 * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
 */
bool Ability::AcquireForm(const int64_t formId, const Want &want, const std::shared_ptr<FormCallback> callback)
{
    APP_LOGI("%{public}s called.", __func__);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;
    APP_LOGI("%{public}s begin, current time: %{public}ld", __func__, currentTime);

    // check fms recover status
    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // check form id
    APP_LOGD("%{public}s, param of formId %{public}" PRId64 ".", __func__, formId);
    if (formId < 0) {
        APP_LOGE("%{public}s error, form id should not be negative.", __func__);
        return false;
    }

    // check param of want
    if (!CheckWantValid(formId, want)) {
        APP_LOGE("%{public}s error, failed to check param of want.", __func__);
        return false;
    };

    ElementName elementName = want.GetElement();
    std::string bundleName = elementName.GetBundleName();
    std::string abilityName = elementName.GetAbilityName();
    APP_LOGI("%{public}s, begin to acquire form, bundleName is %{public}s, abilityName is %{public}s, formId is "
             "%{public}" PRId64 ".",
        __func__,
        bundleName.c_str(),
        abilityName.c_str(),
        formId);

    // hostClient init
    sptr<FormHostClient> formHostClient = FormHostClient::GetInstance();
    if (formHostClient == nullptr) {
        APP_LOGE("%{public}s error, formHostClient == nullptr.", __func__);
        return false;
    }

    // acquire form request to fms
    FormJsInfo formJsInfo;
    if (FormMgr::GetInstance().AddForm(formId, want, formHostClient, formJsInfo) != ERR_OK) {
        APP_LOGE("%{public}s error, acquire form for fms failed.", __func__);
        return false;
    }
    APP_LOGI("%{public}s, end to acquire form, the formId returned from the fms is %{public}" PRId64 ".",
        __func__,
        formJsInfo.formId);

    // check for form presence in hostForms
    if (formHostClient->ContainsForm(formJsInfo.formId)) {
        APP_LOGE("%{public}s error, form has already acquired, do not support acquire twice.", __func__);
        return false;
    }

    // add ability of form to hostForms
    std::shared_ptr<Ability> thisAbility = this->shared_from_this();
    formHostClient->AddForm(thisAbility, formJsInfo.formId);
    // post the async task of handleAcquireResult
    PostTask([this, want, formJsInfo, callback]() { HandleAcquireResult(want, formJsInfo, callback); }, 0L);
    // the acquire form is successfully
    return true;
}

/**
 * @brief Updates the content of a specified JS form.
 *
 * <p>This method is called by a form provider to update JS form data as needed.
 *
 * @param formId Indicates the form ID.
 * @param formProviderData The data used to update the JS form displayed on the client.
 * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
 */
bool Ability::UpdateForm(const int64_t formId, const FormProviderData &formProviderData)
{
    APP_LOGI("%{public}s called.", __func__);
    // check fms recover status
    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // check formId
    if (formId <= 0) {
        APP_LOGE("%{public}s error, the passed in formId can't be negative or zero.", __func__);
        return false;
    }

    // check formProviderData
    if (formProviderData.GetDataString().empty()) {
        APP_LOGE("%{public}s error, the formProviderData is null.", __func__);
        return false;
    }

    // update form request to fms
    if (FormMgr::GetInstance().UpdateForm(formId, abilityInfo_->bundleName, formProviderData) != ERR_OK) {
        APP_LOGE("%{public}s error, update form for fms failed.", __func__);
        return false;
    }

    // the update form is successfully
    return true;
}

/**
 * @brief Sends a notification to the form framework to make the specified forms visible.
 *
 * <p>After this method is successfully called, {@link Ability#OnVisibilityChanged(std::map<int64_t, int>)}
 * will be called to notify the form provider of the form visibility change event.</p>
 *
 * @param formIds Indicates the IDs of the forms to be made visible.
 * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
 */
bool Ability::NotifyVisibleForms(const std::vector<int64_t> &formIds)
{
    APP_LOGI("%{public}s called.", __func__);
    return NotifyWhetherVisibleForms(formIds, Constants::FORM_VISIBLE);
}

/**
 * @brief Sends a notification to the form framework to make the specified forms invisible.
 *
 * <p>After this method is successfully called, {@link Ability#OnVisibilityChanged(std::map<int64_t, int>)}
 * will be called to notify the form provider of the form visibility change event.</p>
 *
 * @param formIds Indicates the IDs of the forms to be made invisible.
 * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
 */
bool Ability::NotifyInvisibleForms(const std::vector<int64_t> &formIds)
{
    APP_LOGI("%{public}s called.", __func__);
    return NotifyWhetherVisibleForms(formIds, Constants::FORM_INVISIBLE);
}

/**
 * @brief Set form next refresh time.
 *
 * <p>This method is called by a form provider to set refresh time.
 *
 * @param formId Indicates the ID of the form to set refresh time.
 * @param nextTime Indicates the next time gap now in seconds, can not be litter than 300 seconds.
 * @return Returns {@code true} if seting succeed; returns {@code false} otherwise.
 */
bool Ability::SetFormNextRefreshTime(const int64_t formId, const int64_t nextTime)
{
    APP_LOGI("%{public}s called.", __func__);
    if (nextTime < MIN_NEXT_TIME) {
        APP_LOGE("next time litte than 300 seconds.");
        return false;
    }

    if (FormMgr::GetInstance().GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s, formManager is in recovering", __func__);
        return false;
    }

    int result = FormMgr::GetInstance().SetNextRefreshTime(formId, nextTime);
    if (result != ERR_OK) {
        APP_LOGE("%{public}s, internal error:[%{public}d]", __func__, result);
        return false;
    }

    return true;
}
/**
 * @brief Requests for form data update.
 *
 * This method must be called when the application has detected that a system setting item (such as the language,
 * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
 * provider automatically updates the form data (if there is any update) through the form framework, with the update
 * process being unperceivable by the application.
 *
 * @param formId Indicates the ID of the form to update.
 * @return Returns true if the update request is successfully initiated, returns false otherwise.
 */
bool Ability::RequestForm(const int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);
    Want want;
    return RequestForm(formId, want);
}

/**
 * @brief Update form.
 *
 * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
 */
void Ability::ProcessFormUpdate(const FormJsInfo &formJsInfo)
{
    APP_LOGI("%{public}s called.", __func__);
    // post the async task of handleFormMessage
    int32_t msgCode = OHOS_FORM_UPDATE_FORM;
    PostTask([this, msgCode, formJsInfo]() { HandleFormMessage(msgCode, formJsInfo); }, 0L);
}
/**
 * @brief Uninstall form.
 *
 * @param formId Indicates the ID of the form to uninstall.
 */
void Ability::ProcessFormUninstall(const int64_t formId)
{
    APP_LOGI("%{public}s start.", __func__);
    // check formId
    if (formId <= 0) {
        APP_LOGE("%{public}s error, the passed in formId can't be negative or zero.", __func__);
        return;
    }

    std::shared_ptr<FormCallback> formCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(formLock);
        // get callback iterator by formId
        std::map<int64_t, std::shared_ptr<FormCallback>>::iterator appCallbackIterator = appCallbacks_.find(formId);

        // call the callback function when you need to be notified
        if (appCallbackIterator == appCallbacks_.end()) {
            APP_LOGE("%{public}s failed, callback not find, formId: %{public}" PRId64 ".", __func__, formId);
            return;
        }
        formCallback = appCallbackIterator->second;
        CleanFormResource(formId);
    }
    if (formCallback == nullptr) {
        APP_LOGE("%{public}s failed, callback is nullptr.", __func__);
        return;
    }

    formCallback->OnFormUninstall(formId);

    APP_LOGI("%{public}s end.", __func__);
}

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
    APP_LOGI("%{public}s called.", __func__);
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
 * @brief Delete or release form with formId.
 *
 * @param formId Indicates the form's ID.
 * @param deleteType Indicates the type of delete or release.
 * @return Returns {@code true} if the form is successfully deleted; returns {@code false} otherwise.
 */
bool Ability::DeleteForm(const int64_t formId, const int32_t deleteType)
{
    APP_LOGI("%{public}s called.", __func__);
    // check fms recover status
    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }
    // check formId
    if (formId <= 0) {
        APP_LOGE("%{public}s error, the passed in formId can't be negative or zero.", __func__);
        return false;
    }

    APP_LOGI("%{public}s, delete form begin, formId is %{public}" PRId64 " and deleteType is %{public}d.",
        __func__,
        formId,
        deleteType);
    {
        // form lock
        std::lock_guard<std::mutex> lock(formLock);
        // clean form resource when form is temp form
        if (std::find(lostedByReconnectTempForms_.begin(), lostedByReconnectTempForms_.end(), formId) !=
            lostedByReconnectTempForms_.end()) {
            CleanFormResource(formId);
            // the delete temp form is successfully
            return true;
        }
    }

    // hostClient init
    sptr<FormHostClient> formHostClient = FormHostClient::GetInstance();
    // delete or release request to fms
    int result;
    if (deleteType == DELETE_FORM) {
        result = FormMgr::GetInstance().DeleteForm(formId, formHostClient);
    } else {
        result = FormMgr::GetInstance().ReleaseForm(
            formId, formHostClient, (deleteType == RELEASE_CACHED_FORM) ? true : false);
    }
    if (result != ERR_OK) {
        APP_LOGE("%{public}s error, some internal server occurs, error code is %{public}d.", __func__, result);
        return false;
    }
    {
        // form lock
        std::lock_guard<std::mutex> lock(formLock);
        // clean form resource
        CleanFormResource(formId);
    }
    // the delete form is successfully
    return true;
}

/**
 * @brief Clean form resource with formId.
 *
 * @param formId Indicates the form's ID.
 */
void Ability::CleanFormResource(const int64_t formId)
{
    APP_LOGI("%{public}s called.", __func__);
    // compatible with int form id
    int64_t cleanId {-1L};
    for (auto param : userReqParams_) {
        uint64_t unsignedFormId = static_cast<uint64_t>(formId);
        uint64_t unsignedParamFirst = static_cast<uint64_t>(param.first);
        if ((unsignedParamFirst & 0x00000000ffffffffL) == (unsignedFormId & 0x00000000ffffffffL)) {
            cleanId = param.first;
            break;
        }
    }
    if (cleanId == -1L) {
        return;
    }

    APP_LOGD("%{public}s. clean id is %{public}" PRId64 ".", __func__, cleanId);
    // remove wantParam, callback and lostedByReconnectTempForms
    appCallbacks_.erase(cleanId);
    userReqParams_.erase(cleanId);
    auto tempForm = std::find(lostedByReconnectTempForms_.begin(), lostedByReconnectTempForms_.end(), cleanId);
    if (tempForm != lostedByReconnectTempForms_.end()) {
        lostedByReconnectTempForms_.erase(tempForm);
    }

    // remove ability
    std::shared_ptr<Ability> thisAbility = this->shared_from_this();
    FormHostClient::GetInstance()->RemoveForm(thisAbility, cleanId);

    // unregister death callback when appCallbacks is empty
    if (appCallbacks_.empty()) {
        std::shared_ptr<Ability> thisAbility = this->shared_from_this();
        FormMgr::GetInstance().UnRegisterDeathCallback(thisAbility);
    }
    APP_LOGI("%{public}s end.", __func__);
}

/**
 * @brief Handle acquire result of the obtained form instance.
 *
 * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
 *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
 * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
 * @param callback Indicates the callback to be invoked whenever the {@link FormJsInfo} instance is obtained.
 */
void Ability::HandleAcquireResult(
    const Want &want, const FormJsInfo &formJsInfo, const std::shared_ptr<FormCallback> callback)
{
    APP_LOGI("%{public}s called.", __func__);
    {
        // form lock
        std::lock_guard<std::mutex> lock(formLock);

        // register death when userReqParams is empty
        if (userReqParams_.empty()) {
            std::shared_ptr<Ability> thisAbility = this->shared_from_this();
            FormMgr::GetInstance().RegisterDeathCallback(thisAbility);
        }

        // save wantParam and callback
        userReqParams_.insert(std::make_pair(formJsInfo.formId, want));
        appCallbacks_.insert(std::make_pair(formJsInfo.formId, callback));
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long currentTime = ts.tv_sec * SEC_TO_MILLISEC + ts.tv_nsec / MILLISEC_TO_NANOSEC;
    APP_LOGI("%{public}s, AcquireForm end, current time: %{public}ld", __func__, currentTime);

    // handle acquire message of the obtained form instance
    callback->OnAcquired(FormCallback::OHOS_FORM_ACQUIRE_SUCCESS, formJsInfo);
}

/**
 * @brief Handle acquire message of the obtained form instance.
 *
 * @param msgCode Indicates the code of message type.
 * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
 */
void Ability::HandleFormMessage(const int32_t msgCode, const FormJsInfo &formJsInfo)
{
    APP_LOGI("%{public}s called.", __func__);
    std::shared_ptr<FormCallback> formCallback = nullptr;
    {
        std::lock_guard<std::mutex> lock(formLock);
        // get callback iterator by formId
        std::map<int64_t, std::shared_ptr<FormCallback>>::iterator appCallbackIterator =
            appCallbacks_.find(formJsInfo.formId);

        // call the callback function when you need to be notified
        if (appCallbackIterator == appCallbacks_.end()) {
            APP_LOGE("%{public}s failed, callback not find, formId: %{public}" PRId64 ".", __func__, formJsInfo.formId);
            return;
        }
        formCallback = appCallbackIterator->second;
    }
    if (formCallback == nullptr) {
        APP_LOGE("%{public}s failed, callback is nullptr.", __func__);
        return;
    }

    APP_LOGI("%{public}s, call user implement of form %{public}" PRId64 ".", __func__, formJsInfo.formId);

    if (msgCode == OHOS_FORM_ACQUIRE_FORM) {
        formCallback->OnAcquired(FormCallback::OHOS_FORM_ACQUIRE_SUCCESS, formJsInfo);
    } else {
        formCallback->OnUpdate(FormCallback::OHOS_FORM_UPDATE_SUCCESS, formJsInfo);
    }
}

/**
 * @brief Notify the forms visibility change event.
 *
 * @param formIds Indicates the IDs of the forms to be made visible or invisible.
 * @param eventType Indicates the form events occurred. FORM_VISIBLE means that the form becomes visible,
 *                  and FORM_INVISIBLE means that the form becomes invisible.
 * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
 */
bool Ability::NotifyWhetherVisibleForms(const std::vector<int64_t> &formIds, int32_t eventType)
{
    APP_LOGI("%{public}s called.", __func__);
    if (formIds.empty() || formIds.size() > Constants::MAX_VISIBLE_NOTIFY_LIST) {
        APP_LOGE("%{public}s, formIds is empty or exceed 32.", __func__);
        return false;
    }

    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    int resultCode =
        FormMgr::GetInstance().NotifyWhetherVisibleForms(formIds, FormHostClient::GetInstance(), eventType);
    if (resultCode != ERR_OK) {
        APP_LOGE("%{public}s error, internal error occurs, error code:%{public}d.", __func__, resultCode);
        return false;
    }
    return true;
}

/**
 * @brief Check the param of want.
 *
 * @param formId Indicates the form's ID.
 * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
 *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
 * @return Returns {@code true} if the check result is ok; returns {@code false} ng.
 */
bool Ability::CheckWantValid(const int64_t formId, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);
    // get want parameters
    int32_t formDimension = want.GetIntParam(Constants::PARAM_FORM_DIMENSION_KEY, 1);
    std::string moduleName = want.GetStringParam(Constants::PARAM_MODULE_NAME_KEY);

    ElementName elementName = want.GetElement();
    std::string bundleName = elementName.GetBundleName();
    std::string abilityName = elementName.GetAbilityName();

    bool tempFormFlg = want.GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, false);

    APP_LOGD("%{public}s, param of formDimension %{public}d in want.", __func__, formDimension);
    APP_LOGD("%{public}s, param of moduleName %{public}s in want.", __func__, moduleName.c_str());
    APP_LOGD("%{public}s, param of bundleName %{public}s in want.", __func__, bundleName.c_str());
    APP_LOGD("%{public}s, param of abilityName %{public}s in want.", __func__, abilityName.c_str());
    APP_LOGD("%{public}s, param of tempFormFlg %{public}d in want.", __func__, tempFormFlg);

    // check want parameters
    if (bundleName.empty() || abilityName.empty() || moduleName.empty()) {
        APP_LOGE("%{public}s error, bundleName or abilityName or moduleName is not set in want.", __func__);
        return false;
    }
    if (FormHostClient::GetInstance()->ContainsForm(formId)) {
        APP_LOGE("%{public}s error, form has already acquired, do not support acquire twice.", __func__);
        return false;
    }
    if (formDimension <= 0) {
        APP_LOGE("%{public}s error, dimension should not be zero or negative in want.", __func__);
        return false;
    }
    if (tempFormFlg && formId != 0) {
        APP_LOGE("%{public}s error, can not select form id when acquire temporary form.", __func__);
        return false;
    }

    // the check is successfully
    return true;
}

/**
 * @brief Enable form update.
 *
 * @param formIds FormIds of hostclient.
 */
bool Ability::EnableUpdateForm(const std::vector<int64_t> &formIds)
{
    APP_LOGI("%{public}s called.", __func__);
    return LifecycleUpdate(formIds, ENABLE_FORM_UPDATE);
}

/**
 * @brief Disable form update.
 *
 * @param formIds FormIds of hostclient.
 */
bool Ability::DisableUpdateForm(const std::vector<int64_t> &formIds)
{
    APP_LOGI("%{public}s called.", __func__);
    return LifecycleUpdate(formIds, DISABLE_FORM_UPDATE);
}

bool Ability::LifecycleUpdate(std::vector<int64_t> formIds, int32_t updateType)
{
    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // hostClient init
    sptr<FormHostClient> formHostClient = FormHostClient::GetInstance();
    if (formHostClient == nullptr) {
        APP_LOGE("%{public}s error, formHostClient == nullptr.", __func__);
        return false;
    }

    int result = FormMgr::GetInstance().LifecycleUpdate(formIds, formHostClient, updateType);
    if (result != ERR_OK) {
        APP_LOGE("%{public}s error, internal error.", __func__);
        return false;
    }

    return true;
}

/**
 * @brief Requests for form data update, by passing a set of parameters (using Want) to the form provider.
 *
 * This method must be called when the application has detected that a system setting item (such as the language,
 * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
 * supplier automatically updates the form data (if there is any update) through the form framework, with the update
 * process being unperceivable by the application.
 *
 * @param formId Indicates the ID of the form to update.
 * @param want Indicates a set of parameters to be transparently passed to the form provider.
 * @return Returns true if the update request is successfully initiated, returns false otherwise.
 */
bool Ability::RequestForm(const int64_t formId, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);
    if (formId <= 0) {
        APP_LOGE("%{public}s error, The passed formid is invalid. Its value must be larger than 0.", __func__);
        return false;
    }

    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // requestForm request to fms
    int resultCode = FormMgr::GetInstance().RequestForm(formId, FormHostClient::GetInstance(), want);
    if (resultCode != ERR_OK) {
        APP_LOGE("%{public}s error, failed to notify the form service that the form user's lifecycle is updated, error "
                 "code is %{public}d.",
            __func__,
            resultCode);
        return false;
    }

    return true;
}

/**
 * @brief Called to reacquire form and update the form host after the death callback is received.
 *
 */
void Ability::OnDeathReceived()
{
    APP_LOGI("%{public}s called.", __func__);
    int64_t formId;
    std::map<int64_t, Want> &userReqParams = userReqParams_;
    std::vector<int64_t> &lostedTempForms = lostedByReconnectTempForms_;
    for (const auto &userReqRaram : userReqParams) {
        formId = userReqRaram.first;
        Want want;
        {
            std::lock_guard<std::mutex> lock(formLock);
            want = userReqRaram.second;
            if (want.GetBoolParam(Constants::PARAM_FORM_TEMPORARY_KEY, false) &&
                std::find(lostedTempForms.begin(), lostedTempForms.end(), formId) == lostedTempForms.end()) {
                lostedTempForms.emplace_back(formId);
                continue;
            }
        }

        bool result = ReAcquireForm(formId, want);
        if (!result) {
            APP_LOGI("%{public}s error, reacquire form failed, formId:%{public}" PRId64 ".", __func__, formId);
            std::shared_ptr<FormCallback> formCallback = nullptr;
            {
                std::lock_guard<std::mutex> lock(formLock);
                // get callback iterator by formId
                std::map<int64_t, std::shared_ptr<FormCallback>>::iterator appCallbackIterator =
                    appCallbacks_.find(formId);

                if (appCallbackIterator == appCallbacks_.end()) {
                    APP_LOGW("%{public}s error, lack of form callback for form, formId:%{public}" PRId64 ".",
                        __func__,
                        formId);
                    continue;
                }
                formCallback = appCallbackIterator->second;
            }
            if (formCallback == nullptr) {
                APP_LOGW("%{public}s failed, callback is nullptr.", __func__);
                continue;
            }

            FormJsInfo formJsInfo;
            formJsInfo.formId = formId;
            formCallback->OnAcquired(FormCallback::OHOS_FORM_RESTORE_FAILURE, formJsInfo);
        }
    }
}

/**
 * @brief Reacquire a specified form when the death callback is received.
 *
 * @param formId Indicates the form ID.
 * @param want Indicates the detailed information about the form to be obtained.
 * @return Returns true if the request is successfully initiated; returns false otherwise.
 */
bool Ability::ReAcquireForm(const int64_t formId, const Want &want)
{
    APP_LOGI("%{public}s called.", __func__);

    // get the form host client
    sptr<FormHostClient> formHostClient = FormHostClient::GetInstance();
    if (formHostClient == nullptr) {
        APP_LOGE("%{public}s error, formHostClient is nullptr, formId:%{public}" PRId64 ".", __func__, formId);
        return false;
    }

    // reacquire form
    FormJsInfo formJsInfo;
    if (FormMgr::GetInstance().AddForm(formId, want, formHostClient, formJsInfo) != ERR_OK || formJsInfo.formId <= 0 ||
        formJsInfo.formId != formId) {
        APP_LOGE("%{public}s error, fms reacquire form failed, formId:%{public}" PRId64 ".", __func__, formId);
        return false;
    }

    // handle update message of the obtained form instance.
    ProcessFormUpdate(formJsInfo);

    return true;
}

/**
 * @brief Check form manager service ready.
 *
 * @return Return true if form manager service ready; returns false otherwise.
 */
bool Ability::CheckFMSReady()
{
    APP_LOGI("%{public}s called.", __func__);

    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto remoteObject = systemAbilityManager->GetSystemAbility(FORM_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        APP_LOGI("%{public}s, form manager service is not ready.", __func__);
        return false;
    }

    return true;
}

/**
 * @brief Get All FormsInfo.
 *
 * @param formInfos Return the forms' information of all forms provided.
 * @return Return true if the request is successfully initiated; return false otherwise.
 */
bool Ability::GetAllFormsInfo(std::vector<FormInfo> &formInfos)
{
    APP_LOGI("%{public}s called.", __func__);

    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // GetAllFormsInfo request to fms
    int resultCode = FormMgr::GetInstance().GetAllFormsInfo(formInfos);
    if (resultCode != ERR_OK) {
        APP_LOGE("%{public}s error, failed to GetAllFormsInfo, error code is %{public}d.", __func__, resultCode);
        return false;
    }

    return !formInfos.empty();
}

/**
 * @brief Get forms info by bundle name .
 *
 * @param bundleName Application name.
 * @param formInfos Return the forms' information of the specify application name.
 * @return Return true if the request is successfully initiated; return false otherwise.
 */
bool Ability::GetFormsInfoByApp(std::string &bundleName, std::vector<FormInfo> &formInfos)
{
    APP_LOGI("%{public}s called.", __func__);
    if (bundleName.empty()) {
        APP_LOGW("Failed to Get forms info, because empty bundle name");
        return false;
    }

    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // GetFormsInfoByApp request to fms
    int resultCode = FormMgr::GetInstance().GetFormsInfoByApp(bundleName, formInfos);
    if (resultCode != ERR_OK) {
        APP_LOGE("%{public}s error, failed to GetFormsInfoByApp, error code is %{public}d.", __func__, resultCode);
        return false;
    }

    return !formInfos.empty();
}

/**
 * @brief Get forms info by bundle name and module name.
 *
 * @param bundleName bundle name.
 * @param moduleName Module name of hap.
 * @param formInfos Return the forms' information of the specify bundle name and module name.
 * @return Return true if the request is successfully initiated; return false otherwise.
 */
bool Ability::GetFormsInfoByModule(std::string &bundleName, std::string &moduleName, std::vector<FormInfo> &formInfos)
{
    APP_LOGI("%{public}s called.", __func__);
    if (bundleName.empty() || moduleName.empty()) {
        APP_LOGW("Failed to Get forms info, because empty bundleName or moduleName");
        return false;
    }

    if (FormMgr::GetRecoverStatus() == Constants::IN_RECOVERING) {
        APP_LOGE("%{public}s error, form is in recover status, can't do action on form.", __func__);
        return false;
    }

    // GetFormsInfoByModule request to fms
    int resultCode = FormMgr::GetInstance().GetFormsInfoByModule(bundleName, moduleName, formInfos);
    if (resultCode != ERR_OK) {
        APP_LOGE("%{public}s error, failed to GetFormsInfoByModule, error code is %{public}d.", __func__, resultCode);
        return false;
    }

    return !formInfos.empty();
}

/**
 * @brief Acquire a bundle manager, if it not existed.
 * @return returns the bundle manager ipc object, or nullptr for failed.
 */
sptr<IBundleMgr> Ability::GetBundleMgr()
{
    APP_LOGI("%{public}s called.", __func__);
    if (iBundleMgr_ == nullptr) {
        sptr<ISystemAbilityManager> systemAbilityManager =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (remoteObject == nullptr) {
            APP_LOGE("%{public}s error, failed to get bundle manager service.", __func__);
            return nullptr;
        }

        iBundleMgr_ = iface_cast<IBundleMgr>(remoteObject);
        if (iBundleMgr_ == nullptr) {
            APP_LOGE("%{public}s error, failed to get bundle manager service", __func__);
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
    APP_LOGI("%{public}s called.", __func__);

    iBundleMgr_ = bundleManager;
}

/**
 * @brief Acquire a form provider remote object.
 * @return Returns form provider remote object.
 */
sptr<IRemoteObject> Ability::GetFormRemoteObject()
{
    APP_LOGI("%{public}s start", __func__);
    if (providerRemoteObject_ == nullptr) {
        sptr<FormProviderClient> providerClient = new (std::nothrow) FormProviderClient();
        std::shared_ptr<Ability> thisAbility = this->shared_from_this();
        if (thisAbility == nullptr) {
            APP_LOGE("%{public}s failed, thisAbility is nullptr", __func__);
        }
        providerClient->SetOwner(thisAbility);
        providerRemoteObject_ = providerClient->AsObject();
    }
    APP_LOGI("%{public}s end", __func__);
    return providerRemoteObject_;
}

/**
 * @brief Set the start ability setting.
 * @param setting the start ability setting.
 */
void Ability::SetStartAbilitySetting(std::shared_ptr<AbilityStartSetting> setting)
{
    APP_LOGI("%{public}s called.", __func__);
    setting_ = setting;
}

/**
 * @brief Set the launch param.
 *
 * @param launchParam the launch param.
 */
void Ability::SetLaunchParam(const AAFwk::LaunchParam &launchParam)
{
    APP_LOGI("%{public}s called.", __func__);
    launchParam_ = launchParam;
}

const AAFwk::LaunchParam& Ability::GetLaunchParam() const
{
    return launchParam_;
}

void Ability::SetSceneListener(const sptr<Rosen::IWindowLifeCycle> &listener)
{
    sceneListener_ = listener;
}

std::vector<std::shared_ptr<DataAbilityResult>> Ability::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    APP_LOGI("Ability::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (abilityInfo_ == nullptr) {
        APP_LOGE("Ability::ExecuteBatch abilityInfo is nullptr");
        return results;
    }
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        APP_LOGE("Ability::ExecuteBatch data ability type failed, current type: %{public}d", abilityInfo_->type);
        return results;
    }
    size_t len = operations.size();
    APP_LOGI("Ability::ExecuteBatch operation is nullptr, len %{public}zu", len);
    for (size_t i = 0; i < len; i++) {
        std::shared_ptr<DataAbilityOperation> operation = operations[i];
        if (operation == nullptr) {
            APP_LOGI("Ability::ExecuteBatch operation is nullptr, create DataAbilityResult");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            continue;
        }
        ExecuteOperation(operation, results, i);
    }
    APP_LOGI("Ability::ExecuteBatch end, %{public}zu", results.size());
    return results;
}
void Ability::ExecuteOperation(std::shared_ptr<DataAbilityOperation> &operation,
    std::vector<std::shared_ptr<DataAbilityResult>> &results, int index)
{
    APP_LOGI("Ability::ExecuteOperation start, index=%{public}d", index);
    if (abilityInfo_->type != AppExecFwk::AbilityType::DATA) {
        APP_LOGE("Ability::ExecuteOperation data ability type failed, current type: %{public}d", abilityInfo_->type);
        return;
    }
    if (index < 0) {
        APP_LOGE(
            "Ability::ExecuteOperation operation result index should not below zero, current index: %{public}d", index);
        return;
    }
    if (operation == nullptr) {
        APP_LOGI("Ability::ExecuteOperation operation is nullptr, create DataAbilityResult");
        results.push_back(std::make_shared<DataAbilityResult>(0));
        return;
    }

    int numRows = 0;
    std::shared_ptr<NativeRdb::ValuesBucket> valuesBucket = ParseValuesBucketReference(results, operation, index);
    std::shared_ptr<NativeRdb::DataAbilityPredicates> predicates =
        ParsePredictionArgsReference(results, operation, index);
    if (operation->IsInsertOperation()) {
        APP_LOGI("Ability::ExecuteOperation IsInsertOperation");
        numRows = Insert(*(operation->GetUri().get()), *valuesBucket);
    } else if (operation->IsDeleteOperation()) {
        APP_LOGI("Ability::ExecuteOperation IsDeleteOperation");
        numRows = Delete(*(operation->GetUri().get()), *predicates);
    } else if (operation->IsUpdateOperation()) {
        APP_LOGI("Ability::ExecuteOperation IsUpdateOperation");
        numRows = Update(*(operation->GetUri().get()), *valuesBucket, *predicates);
    } else if (operation->IsAssertOperation()) {
        APP_LOGI("Ability::ExecuteOperation IsAssertOperation");
        std::vector<std::string> columns;
        std::shared_ptr<NativeRdb::AbsSharedResultSet> queryResult =
            Query(*(operation->GetUri().get()), columns, *predicates);
        if (queryResult == nullptr) {
            APP_LOGE("Ability::ExecuteOperation Query retval is nullptr");
            results.push_back(std::make_shared<DataAbilityResult>(0));
            return;
        }
        if (queryResult->GetRowCount(numRows) != 0) {
            APP_LOGE("Ability::ExecuteOperation queryResult->GetRowCount(numRows) != E_OK");
        }
        if (!CheckAssertQueryResult(queryResult, operation->GetValuesBucket())) {
            if (queryResult != nullptr) {
                queryResult->Close();
            }
            APP_LOGE("Query Result is not equal to expected value.");
        }

        if (queryResult != nullptr) {
            queryResult->Close();
        }
    } else {
        APP_LOGE("Ability::ExecuteOperation Expected bad type %{public}d", operation->GetType());
    }
    if (operation->GetExpectedCount() != numRows) {
        APP_LOGE("Ability::ExecuteOperation Expected %{public}d rows but actual %{public}d",
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
        APP_LOGE("Ability::ParsePredictionArgsReference intpur is nullptr");
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
        APP_LOGI("Ability::ParsePredictionArgsReference operation->GetDataAbilityPredicates is nullptr");
    } else {
        APP_LOGI("Ability::ParsePredictionArgsReference operation->GetDataAbilityPredicates isn`t nullptr");
        strPredicatesList = predicates->GetWhereArgs();
    }

    if (strPredicatesList.empty()) {
        APP_LOGE("Ability::ParsePredictionArgsReference operation->GetDataAbilityPredicates()->GetWhereArgs()"
                 "error strList is empty()");
    }

    for (auto iterMap : predicatesBackReferencesMap) {
        APP_LOGI("Ability::ParsePredictionArgsReference predicatesBackReferencesMap first:%{public}d second:%{public}d",
            iterMap.first,
            iterMap.second);
        int tempCount = ChangeRef2Value(results, numRefs, iterMap.second);
        if (tempCount < 0) {
            APP_LOGE("Ability::ParsePredictionArgsReference tempCount:%{public}d", tempCount);
            continue;
        }
        std::string strPredicates = std::to_string(tempCount);
        APP_LOGI("Ability::ParsePredictionArgsReference strPredicates:%{public}s", strPredicates.c_str());
        strPredicatesList.push_back(strPredicates);
        APP_LOGI("Ability::ParsePredictionArgsReference push_back done");
    }

    predicates->SetWhereArgs(strPredicatesList);

    return predicates;
}

std::shared_ptr<NativeRdb::ValuesBucket> Ability::ParseValuesBucketReference(
    std::vector<std::shared_ptr<DataAbilityResult>> &results, std::shared_ptr<DataAbilityOperation> &operation,
    int numRefs)
{
    NativeRdb::ValuesBucket retValueBucket;
    if (operation == nullptr) {
        APP_LOGE("Ability::ParseValuesBucketReference intpur is nullptr");
        return nullptr;
    }

    if (operation->GetValuesBucketReferences() == nullptr) {
        return operation->GetValuesBucket();
    }

    retValueBucket.Clear();
    if (operation->GetValuesBucket() == nullptr) {
        APP_LOGI("Ability::ParseValuesBucketReference operation->GetValuesBucket is nullptr");
    } else {
        APP_LOGI("Ability::ParseValuesBucketReference operation->GetValuesBucket is nullptr");
        retValueBucket = *operation->GetValuesBucket();
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMapReferences;
    operation->GetValuesBucketReferences()->GetAll(valuesMapReferences);

    for (auto itermap : valuesMapReferences) {
        std::string key = itermap.first;
        NativeRdb::ValueObject obj;
        if (!operation->GetValuesBucketReferences()->GetObject(key, obj)) {
            APP_LOGE("Ability::ParseValuesBucketReference operation->GetValuesBucketReferences()->GetObject error");
            continue;
        }
        switch (obj.GetType()) {
            case NativeRdb::ValueObjectType::TYPE_INT: {
                int val = 0;
                if (obj.GetInt(val) != 0) {
                    APP_LOGE("Ability::ParseValuesBucketReference ValueObject->GetInt() error");
                    break;
                }
                APP_LOGI("Ability::ParseValuesBucketReference retValueBucket->PutInt(%{public}s, %{public}d)",
                    key.c_str(),
                    val);
                retValueBucket.PutInt(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_DOUBLE: {
                double val = 0.0;
                if (obj.GetDouble(val) != 0) {
                    APP_LOGE("Ability::ParseValuesBucketReference ValueObject->GetDouble() error");
                    break;
                }
                APP_LOGI("Ability::ParseValuesBucketReference retValueBucket->PutDouble(%{public}s, %{public}f)",
                    key.c_str(),
                    val);
                retValueBucket.PutDouble(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_STRING: {
                std::string val = "";
                if (obj.GetString(val) != 0) {
                    APP_LOGE("Ability::ParseValuesBucketReference ValueObject->GetString() error");
                    break;
                }
                APP_LOGI("Ability::ParseValuesBucketReference retValueBucket->PutString(%{public}s, %{public}s)",
                    key.c_str(),
                    val.c_str());
                retValueBucket.PutString(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_BLOB: {
                std::vector<uint8_t> val;
                if (obj.GetBlob(val) != 0) {
                    APP_LOGE("Ability::ParseValuesBucketReference ValueObject->GetBlob() error");
                    break;
                }
                APP_LOGI("Ability::ParseValuesBucketReference retValueBucket->PutBlob(%{public}s, %{public}zu)",
                    key.c_str(),
                    val.size());
                retValueBucket.PutBlob(key, val);
            } break;
            case NativeRdb::ValueObjectType::TYPE_BOOL: {
                bool val = false;
                if (obj.GetBool(val) != 0) {
                    APP_LOGE("Ability::ParseValuesBucketReference ValueObject->GetBool() error");
                    break;
                }
                APP_LOGI("Ability::ParseValuesBucketReference retValueBucket->PutBool(%{public}s, %{public}s)",
                    key.c_str(),
                    val ? "true" : "false");
                retValueBucket.PutBool(key, val);
            } break;
            default: {
                APP_LOGI("Ability::ParseValuesBucketReference retValueBucket->PutNull(%{public}s)", key.c_str());
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
        APP_LOGE("Ability::ChangeRef2Value index >= numRefs");
        return retval;
    }

    if (index >= static_cast<int>(results.size())) {
        APP_LOGE("Ability::ChangeRef2Value index:%{public}d >= results.size():%{public}zu", index, results.size());
        return retval;
    }

    std::shared_ptr<DataAbilityResult> refResult = results[index];
    if (refResult == nullptr) {
        APP_LOGE("Ability::ChangeRef2Value No.%{public}d refResult is null", index);
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
        APP_LOGE("Ability::CheckAssertQueryResult intput queryResult is null");
        return true;
    }

    if (valuesBucket == nullptr) {
        APP_LOGE("Ability::CheckAssertQueryResult intput valuesBucket is null");
        return true;
    }

    std::map<std::string, NativeRdb::ValueObject> valuesMap;
    valuesBucket->GetAll(valuesMap);
    if (valuesMap.empty()) {
        APP_LOGE("Ability::CheckAssertQueryResult valuesMap is empty");
        return true;
    }
    int count = 0;
    if (queryResult->GetRowCount(count) != 0) {
        APP_LOGE("Ability::CheckAssertQueryResult GetRowCount is 0");
        return true;
    }

    for (auto iterMap : valuesMap) {
        std::string strObject;
        if (iterMap.second.GetString(strObject) != 0) {
            APP_LOGE("Ability::CheckAssertQueryResult GetString strObject is error");
            continue;
        }
        if (strObject.empty()) {
            APP_LOGE("Ability::CheckAssertQueryResult strObject is empty");
            continue;
        }
        for (int i = 0; i < count; ++i) {
            std::string strName;
            if (queryResult->GetString(i, strName) != 0) {
                APP_LOGE("Ability::CheckAssertQueryResult GetString strName is error");
                continue;
            }
            if (strName.empty()) {
                APP_LOGE("Ability::CheckAssertQueryResult strName is empty");
                continue;
            }
            if (strName.c_str() == strObject.c_str()) {
                APP_LOGE("Ability::CheckAssertQueryResult strName same to strObject");
                continue;
            }

            return false;
        }
    }

    return true;
}

sptr<Rosen::WindowOption> Ability::GetWindowOption(const Want &want)
{
    APP_LOGI("%{public}s start", __func__);
    sptr<Rosen::WindowOption> option = new Rosen::WindowOption();
    if (option == nullptr) {
        APP_LOGE("Ability::GetWindowOption option is null.");
        return nullptr;
    }
    auto windowMode = want.GetIntParam(Want::PARAM_RESV_WINDOW_MODE,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED);
    APP_LOGI("Ability::GetWindowOption window mode is %{public}d.", windowMode);
    option->SetWindowMode(static_cast<Rosen::WindowMode>(windowMode));

    if (want.GetElement().GetBundleName() == LAUNCHER_BUNDLE_NAME &&
        want.GetElement().GetAbilityName() == LAUNCHER_ABILITY_NAME) {
        APP_LOGI("Set window type for launcher");
        option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_WALLPAPER);
    }

    APP_LOGI("%{public}s end", __func__);
    return option;
}

void Ability::DoOnForeground(const Want& want)
{
    if (abilityWindow_ != nullptr) {
        APP_LOGI("%{public}s begin abilityWindow_->OnPostAbilityForeground, sceneFlag:%{public}d.",
            __func__, sceneFlag_);
        abilityWindow_->OnPostAbilityForeground(sceneFlag_);
        APP_LOGI("%{public}s end abilityWindow_->OnPostAbilityForeground.", __func__);
    } else {
        APP_LOGI("========================abilityWindow_ != nullptr ======================");

    }
}

/**
 * @brief request a remote object of callee from this ability.
 * @return Returns the remote object of callee.
 */
sptr<IRemoteObject> Ability::CallRequest()
{
    return nullptr;
}

int Ability::GetCurrentWindowMode()
{
    APP_LOGI("%{public}s start", __func__);
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
}  // namespace AppExecFwk
}  // namespace OHOS
