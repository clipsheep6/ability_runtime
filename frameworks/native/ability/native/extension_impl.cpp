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

#include "extension_impl.h"

#include "ability_manager_client.h"
#include "ability_local_record.h"
#include "ability_transaction_callback_info.h"
#include "hitrace_meter.h"
#include "extension_context.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void ExtensionImpl::Init(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    std::shared_ptr<Extension> &extension,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("ExtensionImpl Init begin.");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr) ||
        extension == nullptr) {
        HILOG_ERROR("ExtensionImpl::init failed, some object is nullptr");
        return;
    }

    token_ = record->GetToken();
    extension_ = extension;
    if (record->GetAbilityInfo() != nullptr &&
        record->GetAbilityInfo()->extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI) {
        extension_->SetSceneSessionStageListener(
            std::make_shared<ExtensionSessionStateLifeCycleImpl>(token_, shared_from_this()));
    }
    extension_->Init(record, application, handler, token);
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    HILOG_INFO("ExtensionImpl Init end.");
}

/**
 * @brief Handling the life cycle switching of Extension.
 *
 * @param want Indicates the structure containing information about the extension.
 * @param targetState The life cycle state to switch to.
 * @param sessionInfo  Indicates the sessionInfo.
 *
 */
void ExtensionImpl::HandleExtensionTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    HILOG_INFO("ExtensionImpl::HandleExtensionTransaction begin sourceState:%{public}d; targetState: %{public}d; "
             "isNewWant: %{public}d",
        lifecycleState_,
        targetState.state,
        targetState.isNewWant);
    if (lifecycleState_ == targetState.state) {
        HILOG_ERROR("Org lifeCycleState equals to Dst lifeCycleState.");
        return;
    }

    bool ret = true;

    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_INITIAL: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_ACTIVE) {
                Stop();
            }
            break;
        }
        case AAFwk::ABILITY_STATE_INACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
                Start(want, sessionInfo);
            }
            break;
        }
        default: {
            ret = false;
            HILOG_ERROR("ExtensionImpl::HandleExtensionTransaction state is error");
            break;
        }
    }

    if (ret) {
        HILOG_INFO("ExtensionImpl::HandleExtensionTransaction before AbilityManagerClient->AbilityTransitionDone");
        AAFwk::PacMap restoreData;
        AAFwk::AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, restoreData);
        HILOG_INFO("ExtensionImpl::HandleExtensionTransaction after AbilityManagerClient->AbilityTransitionDone");
    }
    HILOG_INFO("ExtensionImpl::HandleExtensionTransaction end");
}

void ExtensionImpl::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_INFO("%{public}s begin.", __func__);

    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::ScheduleUpdateConfiguration extension_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        extension_->OnConfigurationUpdated(config);
    }

    HILOG_INFO("%{public}s end.", __func__);
}

void ExtensionImpl::NotifyMemoryLevel(int level)
{
    HILOG_INFO("%{public}s begin.", __func__);

    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::NotifyMemoryLevel extension_ is nullptr");
        return;
    }

    if (lifecycleState_ != AAFwk::ABILITY_STATE_INITIAL) {
        extension_->OnMemoryLevel(level);
    }

    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INACTIVE. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 * @param want  The Want object to switch the life cycle.
 * @param sessionInfo  Indicates the sessionInfo.
 */
void ExtensionImpl::Start(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::Start extension_ is nullptr");
        return;
    }

    HILOG_INFO("ExtensionImpl::Start");
    if (extension_->abilityInfo_->extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI) {
        extension_->OnStart(want, sessionInfo);
    } else {
        extension_->OnStart(want);
    }
    lifecycleState_ = AAFwk::ABILITY_STATE_INACTIVE;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Toggles the lifecycle status of Extension to AAFwk::ABILITY_STATE_INITIAL. And notifies the application
 * that it belongs to of the lifecycle status.
 *
 */
void ExtensionImpl::Stop()
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::Stop extension_ is nullptr");
        return;
    }

    HILOG_INFO("ExtensionImpl::Stop");
    extension_->OnStop();
    lifecycleState_ = AAFwk::ABILITY_STATE_INITIAL;
    HILOG_INFO("%{public}s end.", __func__);
}

/**
 * @brief Connect the extension. and Calling information back to Extension.
 *
 * @param want The Want object to connect to.
 *
 */
sptr<IRemoteObject> ExtensionImpl::ConnectExtension(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::ConnectAbility extension_ is nullptr");
        return nullptr;
    }

    HILOG_INFO("ExtensionImpl:: ConnectAbility");
    sptr<IRemoteObject> object = extension_->OnConnect(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    HILOG_INFO("%{public}s end.", __func__);

    return object;
}

sptr<IRemoteObject> ExtensionImpl::ConnectExtension(const Want &want, bool &isAsyncCallback)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::ConnectAbility extension_ is nullptr");
        isAsyncCallback = false;
        return nullptr;
    }

    HILOG_INFO("ExtensionImpl:: ConnectAbility");
    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Create();
    if (callbackInfo == nullptr) {
        sptr<IRemoteObject> object = extension_->OnConnect(want);
        lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        isAsyncCallback = false;
        HILOG_INFO("%{public}s end.", __func__);
        return object;
    }

    std::weak_ptr<ExtensionImpl> weakPtr = shared_from_this();
    auto asyncCallback = [extensionImplWeakPtr = weakPtr](sptr<IRemoteObject> &service) {
        auto extensionImpl = extensionImplWeakPtr.lock();
        if (extensionImpl == nullptr) {
            HILOG_ERROR("extensionImpl is nullptr.");
            return;
        }
        extensionImpl->lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        extensionImpl->ConnectExtensionCallback(service);
    };
    callbackInfo->Push(asyncCallback);

    sptr<IRemoteObject> object = extension_->OnConnect(want, callbackInfo, isAsyncCallback);
    if (!isAsyncCallback) {
        lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
    }
    // else: callbackInfo will be destroyed after the async callback
    HILOG_INFO("%{public}s end.", __func__);
    return object;
}

void ExtensionImpl::ConnectExtensionCallback(sptr<IRemoteObject> &service)
{
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread::HandleConnectExtension failed err = %{public}d", err);
    }
}

/**
 * @brief Disconnects the connected object.
 *
 * @param want The Want object to disconnect to.
 */
void ExtensionImpl::DisconnectExtension(const Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::DisconnectAbility extension_ is nullptr");
        return;
    }

    extension_->OnDisconnect(want);
    HILOG_INFO("%{public}s end.", __func__);
}

void ExtensionImpl::DisconnectExtension(const Want &want, bool &isAsyncCallback)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("Failed to disconnect, extension_ is nullptr");
        isAsyncCallback = false;
        return;
    }

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    if (callbackInfo == nullptr) {
        extension_->OnDisconnect(want);
        isAsyncCallback = false;
        return;
    }
    std::weak_ptr<ExtensionImpl> weakPtr = shared_from_this();
    auto asyncCallback = [extensionImplWeakPtr = weakPtr]() {
        auto extensionImpl = extensionImplWeakPtr.lock();
        if (extensionImpl == nullptr) {
            HILOG_ERROR("extensionImpl is nullptr.");
            return;
        }
        extensionImpl->DisconnectExtensionCallback();
    };
    callbackInfo->Push(asyncCallback);

    extension_->OnDisconnect(want, callbackInfo, isAsyncCallback);
    if (!isAsyncCallback) {
        AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
    }
    // else: callbackInfo will be destroyed after the async callback
    HILOG_DEBUG("%{public}s end.", __func__);
}

void ExtensionImpl::DisconnectExtensionCallback()
{
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("ExtensionImpl::DisconnectExtensionCallback failed err = %{public}d", err);
    }
}

/**
 * @brief Command the Extension. and Calling information back to Extension.
 *
 * @param want The Want object to command to.
 *
 * * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
 * destroyed, and the value false indicates a normal startup.
 *
 * @param startId Indicates the number of times the Service Extension has been started. The startId is incremented by 1
 * every time the Extension is started. For example, if the Extension has been started for six times,
 * the value of startId is 6.
 */
void ExtensionImpl::CommandExtension(const Want &want, bool restart, int startId)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::CommandAbility extension_ is nullptr");
        return;
    }

    HILOG_INFO("ExtensionImpl:: CommandAbility");
    extension_->OnCommand(want, restart, startId);
    lifecycleState_ = AAFwk::ABILITY_STATE_ACTIVE;
    HILOG_INFO("%{public}s end.", __func__);
}

void ExtensionImpl::Foreground(const Want &want)
{
    HILOG_DEBUG("ExtensionImpl::Foreground begin");
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::Foreground ability is nullptr");
        return;
    }

    extension_->OnForeground(want);
    lifecycleState_ = AAFwk::ABILITY_STATE_FOREGROUND_NEW;
}

void ExtensionImpl::Background()
{
    HILOG_DEBUG("ExtensionImpl::Background begin");
    if (extension_ == nullptr) {
        HILOG_ERROR("ExtensionImpl::Background ability is nullptr");
        return;
    }
    extension_->OnBackground();
    lifecycleState_ = AAFwk::ABILITY_STATE_BACKGROUND_NEW;
}

void ExtensionImpl::ExtensionSessionStateLifeCycleImpl::AfterForeground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ExtensionSessionStateLifeCycleImpl AfterForeground called.");
    auto owner = owner_.lock();
    if (owner) {
        AAFwk::Want want = *(owner->extension_->GetLaunchWant());
        owner->Foreground(want);
    }
    HILOG_DEBUG("ExtensionSessionStateLifeCycleImpl AfterForeground end.");
}

void ExtensionImpl::ExtensionSessionStateLifeCycleImpl::AfterBackground()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("ExtensionSessionStateLifeCycleImpl AfterBackground called.");
    auto owner = owner_.lock();
    if (owner) {
        owner->Background();
    }
    HILOG_DEBUG("ExtensionSessionStateLifeCycleImpl AfterBackground end.");
}

void ExtensionImpl::ExtensionSessionStateLifeCycleImpl::AfterActive()
{
    HILOG_DEBUG("ExtensionSessionStateLifeCycleImpl AfterActive called.");
}

void ExtensionImpl::ExtensionSessionStateLifeCycleImpl::AfterInactive()
{
    HILOG_DEBUG("ExtensionSessionStateLifeCycleImpl AfterInactive called.");
}
}
}
