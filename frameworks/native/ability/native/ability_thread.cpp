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

#include "ability_thread.h"

#include <chrono>
#include <functional>
#include <thread>

#include "ability_context_impl.h"
#include "ability_impl.h"
#include "ability_impl_factory.h"
#include "ability_loader.h"
#include "abs_shared_result_set.h"
#include "application_impl.h"
#include "hitrace_meter.h"
#include "context_deal.h"
#include "data_ability_predicates.h"
#include "dataobs_mgr_client.h"
#include "hilog_wrapper.h"
#include "ohos_application.h"
#ifdef SUPPORT_GRAPHICS
#include "page_ability_impl.h"
#endif
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
using namespace std::chrono_literals;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
#ifdef ABILITY_COMMAND_FOR_TEST
const int32_t BLOCK_ABILITY_TIME = 20;
#endif
constexpr static char ACE_SERVICE_ABILITY_NAME[] = "AceServiceAbility";
constexpr static char ACE_DATA_ABILITY_NAME[] = "AceDataAbility";
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "Ability";
constexpr static char ACE_ABILITY_NAME[] = "AceAbility";
constexpr static char ACE_FORM_ABILITY_NAME[] = "AceFormAbility";
constexpr static char FORM_EXTENSION[] = "FormExtension";
constexpr static char UI_EXTENSION[] = "UIExtensionAbility";
#endif
constexpr static char BASE_SERVICE_EXTENSION[] = "ServiceExtension";
constexpr static char BASE_DRIVER_EXTENSION[] = "DriverExtension";
constexpr static char STATIC_SUBSCRIBER_EXTENSION[] = "StaticSubscriberExtension";
constexpr static char DATA_SHARE_EXT_ABILITY[] = "DataShareExtAbility";
constexpr static char WORK_SCHEDULER_EXTENSION[] = "WorkSchedulerExtension";
constexpr static char ACCESSIBILITY_EXTENSION[] = "AccessibilityExtension";
constexpr static char WALLPAPER_EXTENSION[] = "WallpaperExtension";
constexpr static char FILEACCESS_EXT_ABILITY[] = "FileAccessExtension";
constexpr static char ENTERPRISE_ADMIN_EXTENSION[] = "EnterpriseAdminExtension";
constexpr static char INPUTMETHOD_EXTENSION[] = "InputMethodExtensionAbility";
constexpr static char APP_ACCOUNT_AUTHORIZATION_EXTENSION[] = "AppAccountAuthorizationExtension";

const int32_t PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS = 3000;

AbilityThread::AbilityThread()
    : abilityImpl_(nullptr), token_(nullptr), currentAbility_(nullptr), abilityHandler_(nullptr), runner_(nullptr)
{}

AbilityThread::~AbilityThread()
{
    if (isExtension_) {
        if (currentExtension_) {
            currentExtension_.reset();
        }
    } else {
        if (currentAbility_) {
            currentAbility_->DetachBaseContext();
            currentAbility_.reset();
        }
    }

    DelayedSingleton<AbilityImplFactory>::DestroyInstance();
}

std::string AbilityThread::CreateAbilityName(const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    std::shared_ptr<OHOSApplication> &application)
{
    std::string abilityName;
    if (abilityRecord == nullptr || application == nullptr) {
        HILOG_ERROR("AbilityThread::CreateAbilityName failed,abilityRecord or app is nullptr");
        return abilityName;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,abilityInfo is nullptr");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        HILOG_DEBUG("Create ability name success, name is %{public}s.", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->type == AbilityType::PAGE) {
        if (abilityInfo->isStageBasedModel) {
            abilityName = ABILITY_NAME;
        } else {
            abilityName = ACE_ABILITY_NAME;
        }
    } else if (abilityInfo->type == AbilityType::SERVICE) {
#else
    if (abilityInfo->type == AbilityType::SERVICE) {
#endif
#ifdef SUPPORT_GRAPHICS
        if (abilityInfo->formEnabled == true) {
            abilityName = ACE_FORM_ABILITY_NAME;
        } else {
#endif
            abilityName = ACE_SERVICE_ABILITY_NAME;
#ifdef SUPPORT_GRAPHICS
        }
#endif
    } else if (abilityInfo->type == AbilityType::DATA) {
        abilityName = ACE_DATA_ABILITY_NAME;
    } else if (abilityInfo->type == AbilityType::EXTENSION) {
        application->GetExtensionNameByType(static_cast<int32_t>(abilityInfo->extensionAbilityType), abilityName);
        if (abilityName.length() > 0) {
            HILOG_DEBUG("Get extension name by plugin success, name: %{public}s", abilityName.c_str());
            return abilityName;
        }
        abilityName = BASE_SERVICE_EXTENSION;
#ifdef SUPPORT_GRAPHICS
        if (abilityInfo->formEnabled || abilityInfo->extensionAbilityType == ExtensionAbilityType::FORM) {
            abilityName = FORM_EXTENSION;
        }
#endif
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::STATICSUBSCRIBER) {
            abilityName = STATIC_SUBSCRIBER_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::DRIVER) {
            abilityName = BASE_DRIVER_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::DATASHARE) {
            abilityName = DATA_SHARE_EXT_ABILITY;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::WORK_SCHEDULER) {
            abilityName = WORK_SCHEDULER_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::ACCESSIBILITY) {
            abilityName = ACCESSIBILITY_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::WALLPAPER) {
            abilityName = WALLPAPER_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::FILEACCESS_EXTENSION) {
            abilityName = FILEACCESS_EXT_ABILITY;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::ENTERPRISE_ADMIN) {
            abilityName = ENTERPRISE_ADMIN_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::INPUTMETHOD) {
            abilityName = INPUTMETHOD_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::UI) {
            abilityName = UI_EXTENSION;
        }
        if (abilityInfo->extensionAbilityType == ExtensionAbilityType::APP_ACCOUNT_AUTHORIZATION) {
            abilityName = APP_ACCOUNT_AUTHORIZATION_EXTENSION;
        }
        HILOG_DEBUG("CreateAbilityName extension type, abilityName:%{public}s", abilityName.c_str());
    } else {
        abilityName = abilityInfo->name;
    }

    HILOG_DEBUG("Create ability name success, name is %{public}s.", abilityName.c_str());
    return abilityName;
}

std::shared_ptr<ContextDeal> AbilityThread::CreateAndInitContextDeal(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<AbilityContext> &abilityObject)
{
    HILOG_DEBUG("AbilityThread::CreateAndInitContextDeal.");
    std::shared_ptr<ContextDeal> contextDeal = nullptr;
    if ((application == nullptr) || (abilityRecord == nullptr) || (abilityObject == nullptr)) {
        HILOG_ERROR("AbilityThread::ability attach failed,context or record or abilityObject is nullptr");
        return contextDeal;
    }

    contextDeal = std::make_shared<ContextDeal>();
    contextDeal->SetAbilityInfo(abilityRecord->GetAbilityInfo());
    contextDeal->SetApplicationInfo(application->GetApplicationInfo());
    abilityObject->SetProcessInfo(application->GetProcessInfo());

    std::shared_ptr<Context> tmpContext = application->GetApplicationContext();
    contextDeal->SetApplicationContext(tmpContext);

    contextDeal->SetBundleCodePath(abilityRecord->GetAbilityInfo()->codePath);
    contextDeal->SetContext(abilityObject);
    return contextDeal;
}

void AbilityThread::Attach(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("Attach ability failed, context or record is nullptr.");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName == "") {
        HILOG_ERROR("Attach ability failed, abilityInfo is nullptr.");
        return;
    }
    HILOG_DEBUG("Attach ability begin, ability:%{public}s.", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("Attach ability failed, abilityHandler_ is nullptr.");
        return;
    }

    // 2.new ability
    auto ability = AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("Attach ability failed, load ability failed.");
        return;
    }

    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<ContextDeal> contextDeal = CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_,
        stageContext));

    // 3.new abilityImpl
    abilityImpl_ =
        DelayedSingleton<AbilityImplFactory>::GetInstance()->MakeAbilityImplObject(abilityRecord->GetAbilityInfo());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("Attach ability failed, abilityImpl_ == nullptr.");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);
    // 4. ability attach : ipc
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Attach ability failed, err = %{public}d.", err);
        return;
    }
}

void AbilityThread::AttachExtension(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("Attach extension failed, context or record is nullptr.");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName == "") {
        HILOG_ERROR("Attach ability failed, abilityInfo is nullptr.");
        return;
    }
    HILOG_DEBUG("Attach extension begin, extension:%{public}s.", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("Attach extension failed, abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        HILOG_ERROR("Attach extension failed, load ability failed");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("Attach extension failed, extensionImpl_ == nullptr");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Attach extension failed, err = %{public}d", err);
        return;
    }
}

void AbilityThread::AttachExtension(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::AttachExtension begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,context or record is nullptr");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,create runner failed");
        return;
    }
    abilityHandler_ = std::make_shared<AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto extension = AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        HILOG_ERROR("AbilityThread::AttachExtension failed,load extension failed");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<AbilityRuntime::ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::extension extensionImpl_ == nullptr");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: AttachExtension failed err = %{public}d", err);
        return;
    }
    HILOG_DEBUG("AbilityThread::AttachExtension end");
}

void AbilityThread::Attach(
    std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::Attach begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("AbilityThread::ability attach failed,context or record is nullptr");
        return;
    }
    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,create runner failed");
        return;
    }
    abilityHandler_ = std::make_shared<AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto ability = AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("AbilityThread::ability attach failed,load ability failed");
        return;
    }

    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<ContextDeal> contextDeal = CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);

    // new hap requires
    ability->AttachAbilityContext(BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_,
        stageContext));

    // 3.new abilityImpl
    abilityImpl_ =
        DelayedSingleton<AbilityImplFactory>::GetInstance()->MakeAbilityImplObject(abilityRecord->GetAbilityInfo());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ability abilityImpl_ == nullptr");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);
    // 4. ability attach : ipc
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: attach success failed err = %{public}d", err);
        return;
    }

    HILOG_DEBUG("AbilityThread::Attach end");
}

void AbilityThread::HandleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
    sptr<SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    HILOG_DEBUG("Handle ability transaction begin, name is %{public}s.", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("Handle ability transaction error, abilityImpl_ == nullptr.");
        return;
    }

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId,
        lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName,
        lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    HILOG_DEBUG("Handle ability transaction success.");
}

void AbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("share data error, abilityImpl_ == nullptr.");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
    HILOG_DEBUG("Handle share data success.");
}

void AbilityThread::HandleExtensionTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
    sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::HandleExtensionTransaction begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleExtensionTransaction extensionImpl_ == nullptr");
        return;
    }
    extensionImpl_->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
    HILOG_DEBUG("AbilityThread::HandleAbilityTransaction end");
}

void AbilityThread::HandleConnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::HandleConnectAbility begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleConnectAbility abilityImpl_ == nullptr");
        return;
    }

    sptr<IRemoteObject> service = abilityImpl_->ConnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: HandleConnectAbility failed err = %{public}d", err);
    }
    HILOG_DEBUG("AbilityThread::HandleConnectAbility end");
}

void AbilityThread::HandleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Handle disconnect ability begin.");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("Handle disconnect ability error, abilityImpl_ == nullptr.");
        return;
    }

    abilityImpl_->DisconnectAbility(want);
    HILOG_DEBUG("Handle disconnect ability done, notify ability manager service.");
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("Handle disconnect ability error, err = %{public}d.", err);
    }
}

void AbilityThread::HandleCommandAbility(const Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::HandleCommandAbility begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleCommandAbility failed. abilityImpl_ == nullptr");
        return;
    }
    abilityImpl_->CommandAbility(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread:: HandleCommandAbility  failed err = %{public}d", err);
    }
    HILOG_DEBUG("AbilityThread::HandleCommandAbility end");
}

void AbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::HandleConnectExtension begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleConnectExtension extensionImpl_ == nullptr");
        return;
    }
    bool isAsyncCallback = false;
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->ConnectExtensionCallback(service);
    }
    HILOG_DEBUG("AbilityThread::HandleConnectExtension end");
}

void AbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::HandleDisconnectExtension begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleDisconnectExtension extensionImpl_ == nullptr");
        return;
    }

    bool isAsyncCallback = false;
    extensionImpl_->DisconnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->DisconnectExtensionCallback();
    }
    HILOG_DEBUG("AbilityThread::HandleDisconnectExtension end");
}

void AbilityThread::HandleCommandExtension(const Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread::HandleCommandExtension begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleCommandExtension extensionImpl_ == nullptr");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("AbilityThread::HandleCommandExtension failed err = %{public}d", err);
    }
    HILOG_DEBUG("AbilityThread::HandleCommandExtension end");
}

void AbilityThread::HandleCommandExtensionWindow(const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ == nullptr");
        return;
    }
    extensionImpl_->CommandExtensionWindow(sessionInfo, winCmd);
    HILOG_DEBUG("end");
}

void AbilityThread::HandleRestoreAbilityState(const PacMap &state)
{
    HILOG_DEBUG("AbilityThread::HandleRestoreAbilityState begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleRestoreAbilityState abilityImpl_ == nullptr");
        return;
    }

    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("AbilityThread::HandleRestoreAbilityState end");
}

void AbilityThread::ScheduleSaveAbilityState()
{
    HILOG_DEBUG("AbilityThread::ScheduleSaveAbilityState begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleSaveAbilityState abilityImpl_ == nullptr");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
    HILOG_DEBUG("AbilityThread::ScheduleSaveAbilityState end");
}

void AbilityThread::ScheduleRestoreAbilityState(const PacMap &state)
{
    HILOG_DEBUG("AbilityThread::ScheduleRestoreAbilityState begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleRestoreAbilityState abilityImpl_ == nullptr");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("AbilityThread::ScheduleRestoreAbilityState end");
}

void AbilityThread::ScheduleUpdateConfiguration(const Configuration &config)
{
    HILOG_DEBUG("AbilityThread::ScheduleUpdateConfiguration begin");
    wptr<AbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleUpdateConfiguration failed.");
            return;
        }

        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionUpdateConfiguration(config);
        } else {
            abilityThread->HandleUpdateConfiguration(config);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleUpdateConfiguration abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleUpdateConfiguration PostTask error");
    }
    HILOG_DEBUG("AbilityThread::ScheduleUpdateConfiguration end");
}

void AbilityThread::HandleUpdateConfiguration(const Configuration &config)
{
    HILOG_DEBUG("AbilityThread::HandleUpdateConfiguration begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::HandleUpdateConfiguration abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("AbilityThread::HandleUpdateConfiguration end");
}

void AbilityThread::HandleExtensionUpdateConfiguration(const Configuration &config)
{
    HILOG_DEBUG("AbilityThread::HandleExtensionUpdateConfiguration begin");
    if (!extensionImpl_) {
        HILOG_ERROR("AbilityThread::HandleExtensionUpdateConfiguration extensionImpl_ is nullptr");
        return;
    }

    extensionImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("AbilityThread::HandleExtensionUpdateConfiguration success");
}

void AbilityThread::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo,
    sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);

    if (token_ == nullptr) {
        HILOG_ERROR("ScheduleAbilityTransaction::failed, token_  nullptr");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleAbilityTransaction failed.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
        } else {
            abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleAbilityTransaction abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleAbilityTransaction PostTask error");
    }
}

void AbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!token_) {
        HILOG_ERROR("token_  is nullptr.");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (!abilityThread) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleShareData failed.");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };

    if (!abilityHandler_) {
        HILOG_ERROR("abilityHandler_ is nullptr.");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("postTask error.");
    }
}

void AbilityThread::ScheduleConnectAbility(const Want &want)
{
    HILOG_DEBUG("AbilityThread::ScheduleConnectAbility begin, isExtension_:%{public}d", isExtension_);
    wptr<AbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleConnectAbility failed.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleConnectExtension(want);
        } else {
            abilityThread->HandleConnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleConnectAbility abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleConnectAbility PostTask error");
    }
    HILOG_DEBUG("AbilityThread::ScheduleConnectAbility end");
}

void AbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Schedule disconnect ability begin, isExtension:%{public}d.", isExtension_);
    wptr<AbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("Schedule disconnect ability error, abilityThread is nullptr.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleDisconnectExtension(want);
        } else {
            abilityThread->HandleDisconnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("Schedule disconnect ability error, abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("Schedule disconnect ability error, PostTask error");
    }
}

void AbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    HILOG_DEBUG("AbilityThread::ScheduleCommandAbility begin. startId:%{public}d", startId);
    wptr<AbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleCommandAbility failed.");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleCommandExtension(want, restart, startId);
        } else {
            abilityThread->HandleCommandAbility(want, restart, startId);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleCommandAbility abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleCommandAbility PostTask error");
    }
    HILOG_DEBUG("AbilityThread::ScheduleCommandAbility end");
}

bool AbilityThread::SchedulePrepareTerminateAbility()
{
    HILOG_DEBUG("call");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr.");
        return true;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        HILOG_DEBUG("end, ret = %{public}d", ret);
        return ret;
    } else {
        wptr<AbilityThread> weak = this;
        auto task = [weak]() {
            auto abilityThread = weak.promote();
            if (abilityThread == nullptr) {
                HILOG_ERROR("abilityThread is nullptr, ScheduleCommandAbility failed.");
                return;
            }
            abilityThread->HandlePrepareTermianteAbility();
        };

        if (abilityHandler_ == nullptr) {
            HILOG_ERROR("abilityHandler_ is nullptr");
            return false;
        }

        bool ret = abilityHandler_->PostTask(task);
        if (!ret) {
            HILOG_ERROR("PostTask error");
            return false;
        }

        std::unique_lock<std::mutex> lock(mutex_);
        HILOG_ERROR("[DongLin] begin to wait");
        auto condition = [this] {
            if (this->isPrepareTerminateAbilityDone_) {
                HILOG_ERROR("[DongLin]isPrepareTerminateAbilityDone_ is true");
                return true;
            } else {
                HILOG_ERROR("[DongLin]isPrepareTerminateAbilityDone_ is false");
                return false;
            }
        };
        if(!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
            HILOG_WARN("[DongLin]Wait timeout.");
        }
        HILOG_DEBUG("end, ret = %{public}d", isPrepareTerminate_);
        return isPrepareTerminate_;
    }
}

void AbilityThread::ScheduleCommandAbilityWindow(const sptr<AAFwk::SessionInfo> &sessionInfo,
    AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("begin.");
    wptr<AbilityThread> weak = this;
    auto task = [weak, sessionInfo, winCmd]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        abilityThread->HandleCommandExtensionWindow(sessionInfo, winCmd);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void AbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    HILOG_DEBUG("AbilityThread::SendResult begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::SendResult abilityImpl_ == nullptr");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr || abilityThread->abilityImpl_ == nullptr) {
            HILOG_ERROR("abilityThread or abilityImpl is nullptr, SendResult failed.");
            return;
        }
        if (requestCode != -1) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::SendResult abilityHandler_ == nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::SendResult PostTask error");
    }
    HILOG_DEBUG("AbilityThread::SendResult end");
}

std::vector<std::string> AbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_DEBUG("AbilityThread::GetFileTypes begin");
    std::vector<std::string> types;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::GetFileTypes abilityImpl_ is nullptr");
        return types;
    }

    types = abilityImpl_->GetFileTypes(uri, mimeTypeFilter);
    HILOG_DEBUG("AbilityThread::GetFileTypes end");
    return types;
}

int AbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_DEBUG("AbilityThread::OpenFile begin");
    int fd = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::OpenFile abilityImpl_ is nullptr");
        return fd;
    }

    fd = abilityImpl_->OpenFile(uri, mode);
    HILOG_DEBUG("AbilityThread::OpenFile end");
    return fd;
}

int AbilityThread::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_DEBUG("AbilityThread::OpenRawFile begin");
    int fd = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::OpenRawFile abilityImpl_ is nullptr");
        return fd;
    }

    fd = abilityImpl_->OpenRawFile(uri, mode);
    HILOG_DEBUG("AbilityThread::OpenRawFile end");
    return fd;
}

int AbilityThread::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_DEBUG("AbilityThread::Insert begin");
    int index = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Insert abilityImpl_ is nullptr");
        return index;
    }

    index = abilityImpl_->Insert(uri, value);
    HILOG_DEBUG("AbilityThread::Insert end");
    return index;
}

std::shared_ptr<AppExecFwk::PacMap> AbilityThread::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    HILOG_DEBUG("AbilityThread::Call begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Insert abilityImpl_ is nullptr");
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::PacMap> result = abilityImpl_->Call(uri, method, arg, pacMap);
    HILOG_DEBUG("AbilityThread::Call end");
    return result;
}

int AbilityThread::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_DEBUG("AbilityThread::Update begin");
    int index = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Update abilityImpl_ is nullptr");
        return index;
    }

    index = abilityImpl_->Update(uri, value, predicates);
    HILOG_DEBUG("AbilityThread::Update end");
    return index;
}

int AbilityThread::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_DEBUG("AbilityThread::Delete begin");
    int index = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Delete abilityImpl_ is nullptr");
        return index;
    }
    index = abilityImpl_->Delete(uri, predicates);
    HILOG_DEBUG("AbilityThread::Delete end");
    return index;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilityThread::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_DEBUG("AbilityThread::Query begin");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Query abilityImpl_ is nullptr");
        return resultSet;
    }

    resultSet = abilityImpl_->Query(uri, columns, predicates);
    HILOG_DEBUG("AbilityThread::Query end");
    return resultSet;
}

std::string AbilityThread::GetType(const Uri &uri)
{
    HILOG_DEBUG("AbilityThread::GetType begin");
    std::string type;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::GetType abilityImpl_ is nullptr");
        return type;
    }

    type = abilityImpl_->GetType(uri);
    HILOG_DEBUG("AbilityThread::GetType end");
    return type;
}

bool AbilityThread::Reload(const Uri &uri, const PacMap &extras)
{
    HILOG_DEBUG("AbilityThread::Reload begin");
    bool ret = false;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::Reload abilityImpl_ is nullptr");
        return ret;
    }
    ret = abilityImpl_->Reload(uri, extras);
    HILOG_DEBUG("AbilityThread::Reload end");
    return ret;
}

int AbilityThread::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_DEBUG("AbilityThread::BatchInsert begin");
    int ret = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::BatchInsert abilityImpl_ is nullptr");
        return ret;
    }

    ret = abilityImpl_->BatchInsert(uri, values);
    HILOG_DEBUG("AbilityThread::BatchInsert end");
    return ret;
}

void AbilityThread::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    HILOG_DEBUG("ContinueAbility");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ContinueAbility abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->ContinueAbility(deviceId, versionCode);
}

void AbilityThread::NotifyContinuationResult(int32_t result)
{
    HILOG_DEBUG("NotifyContinuationResult, result:%{public}d", result);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::NotifyContinuationResult abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->NotifyContinuationResult(result);
}

void AbilityThread::NotifyMemoryLevel(int32_t level)
{
    HILOG_DEBUG("NotifyMemoryLevel, result:%{public}d", level);

    if (isExtension_) {
        HILOG_DEBUG("AbilityThread is an extension ability");
        if (extensionImpl_ == nullptr) {
            HILOG_ERROR("AbilityThread::NotifyMemoryLevel extensionImpl_ is nullptr");
            return;
        }
        extensionImpl_->NotifyMemoryLevel(level);
    } else {
        HILOG_DEBUG("AbilityThread is an ability");
        if (abilityImpl_ == nullptr) {
            HILOG_ERROR("AbilityThread::NotifyMemoryLevel abilityImpl_ is nullptr");
            return;
        }
        abilityImpl_->NotifyMemoryLevel(level);
    }
}

void AbilityThread::AbilityThreadMain(std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread main start.");
    sptr<AbilityThread> thread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    if (thread == nullptr) {
        HILOG_ERROR("AbilityThread::AbilityThreadMain failed,thread  is nullptr");
        return;
    }
    thread->InitExtensionFlag(abilityRecord);
    if (thread->isExtension_) {
        thread->AttachExtension(application, abilityRecord, mainRunner);
    } else {
        thread->Attach(application, abilityRecord, mainRunner, stageContext);
    }
    HILOG_DEBUG("AbilityThread main end.");
}

void AbilityThread::AbilityThreadMain(
    std::shared_ptr<OHOSApplication> &application, const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("AbilityThread main start.");
    sptr<AbilityThread> thread = sptr<AbilityThread>(new (std::nothrow) AbilityThread());
    if (thread == nullptr || abilityRecord == nullptr) {
        HILOG_ERROR("AbilityThread::AbilityThreadMain failed, thread is nullptr");
        return;
    }
    thread->InitExtensionFlag(abilityRecord);
    if (thread->isExtension_) {
        thread->AttachExtension(application, abilityRecord);
    } else {
        thread->Attach(application, abilityRecord, stageContext);
    }
    HILOG_DEBUG("AbilityThread main end.");
}

void AbilityThread::InitExtensionFlag(const std::shared_ptr<AbilityLocalRecord> &abilityRecord)
{
    HILOG_DEBUG("AbilityThread::InitExtensionFlag start");
    if (abilityRecord == nullptr) {
        HILOG_ERROR("AbilityThread::InitExtensionFlag abilityRecord null");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("AbilityThread::InitExtensionFlag abilityInfo null");
        return;
    }
    if (abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        HILOG_DEBUG("AbilityThread::InitExtensionFlag true");
        isExtension_ = true;
    } else {
        isExtension_ = false;
    }
}

Uri AbilityThread::NormalizeUri(const Uri &uri)
{
    HILOG_DEBUG("AbilityThread::NormalizeUri begin");
    Uri urivalue("");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("DataAbilityHelper::normalizeUri failed dataAbility == nullptr");
        return urivalue;
    }

    urivalue = abilityImpl_->NormalizeUri(uri);
    HILOG_DEBUG("AbilityThread::NormalizeUri end");
    return urivalue;
}

Uri AbilityThread::DenormalizeUri(const Uri &uri)
{
    HILOG_DEBUG("AbilityThread::DenormalizeUri begin");
    Uri urivalue("");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("DataAbilityHelper::denormalizeUri failed dataAbility == nullptr");
        return urivalue;
    }

    urivalue = abilityImpl_->DenormalizeUri(uri);
    HILOG_DEBUG("AbilityThread::DenormalizeUri end");
    return urivalue;
}

bool AbilityThread::HandleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->RegisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->RegisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool AbilityThread::HandleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->UnregisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->UnregisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool AbilityThread::HandleNotifyChange(const Uri &uri)
{
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->NotifyChange error return %{public}d", __func__, ret);
        return false;
    }
    return true;
}

bool AbilityThread::CheckObsPermission()
{
    HILOG_DEBUG("%{public}s CheckObsPermission() run Permission Checkout", __func__);
    return true;
}

bool AbilityThread::ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (!CheckObsPermission()) {
        HILOG_ERROR("%{public}s CheckObsPermission() return false", __func__);
        return false;
    }

    wptr<AbilityThread> weak = this;
    auto task = [weak, uri, dataObserver]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleRegisterObserver failed.");
            return;
        }
        abilityThread->HandleRegisterObserver(uri, dataObserver);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleRegisterObserver abilityHandler_ == nullptr");
        return false;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleRegisterObserver PostTask error");
    }
    return ret;
}

bool AbilityThread::ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (!CheckObsPermission()) {
        HILOG_ERROR("%{public}s CheckObsPermission() return false", __func__);
        return false;
    }

    wptr<AbilityThread> weak = this;
    auto task = [weak, uri, dataObserver]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleUnregisterObserver failed.");
            return;
        }
        abilityThread->HandleUnregisterObserver(uri, dataObserver);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleUnregisterObserver abilityHandler_ == nullptr");
        return false;
    }

    bool ret = abilityHandler_->PostSyncTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleUnregisterObserver PostTask error");
    }
    return ret;
}

bool AbilityThread::ScheduleNotifyChange(const Uri &uri)
{
    HILOG_DEBUG("%{public}s called", __func__);
    if (!CheckObsPermission()) {
        HILOG_ERROR("%{public}s CheckObsPermission() return false", __func__);
        return false;
    }

    wptr<AbilityThread> weak = this;
    auto task = [weak, uri]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleNotifyChange failed.");
            return;
        }
        abilityThread->HandleNotifyChange(uri);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleNotifyChange abilityHandler_ == nullptr");
        return false;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("AbilityThread::ScheduleNotifyChange PostTask error");
    }
    return ret;
}

std::vector<std::shared_ptr<DataAbilityResult>> AbilityThread::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    HILOG_DEBUG("AbilityThread::ExecuteBatch start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("AbilityThread::ExecuteBatch abilityImpl_ is nullptr");
        results.clear();
        return results;
    }
    results = abilityImpl_->ExecuteBatch(operations);
    HILOG_DEBUG("AbilityThread::ExecuteBatch end");
    return results;
}

std::shared_ptr<AbilityRuntime::AbilityContext> AbilityThread::BuildAbilityContext(
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<OHOSApplication> &application,
    const sptr<IRemoteObject> &token, const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    auto abilityContextImpl = std::make_shared<AbilityRuntime::AbilityContextImpl>();
    abilityContextImpl->SetStageContext(stageContext);
    abilityContextImpl->SetToken(token);
    abilityContextImpl->SetAbilityInfo(abilityInfo);
    abilityContextImpl->SetConfiguration(application->GetConfiguration());
    return abilityContextImpl;
}

void AbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (token_ == nullptr) {
        HILOG_ERROR("DumpAbilityInfo::failed, token_  nullptr");
        return;
    }
    wptr<AbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr, ScheduleAbilityTransaction failed.");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            HILOG_ERROR("AbilityThread:: DumpAbilityInfo failed err = %{public}d", err);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("AbilityThread::ScheduleAbilityTransaction abilityHandler_ == nullptr");
        return;
    }

    abilityHandler_->PostTask(task);
}

#ifdef SUPPORT_GRAPHICS
void AbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (currentAbility_ == nullptr && currentExtension_ == nullptr) {
        HILOG_DEBUG("currentAbility and currentExtension_ is nullptr.");
        return;
    }
    if (currentAbility_ != nullptr) {
        if (abilityImpl_->IsStageBasedModel()) {
            auto scene = currentAbility_->GetScene();
            if (scene == nullptr) {
                HILOG_ERROR("DumpAbilityInfo scene == nullptr");
                return;
            }
            auto window = scene->GetMainWindow();
            if (window == nullptr) {
                HILOG_ERROR("DumpAbilityInfo window == nullptr");
                return;
            }
            window->DumpInfo(params, info);
        }
        currentAbility_->Dump(params, info);
    }
    if (currentExtension_ != nullptr) {
        currentExtension_->Dump(params, info);
    }
    if (params.empty()) {
        DumpOtherInfo(info);
        return;
    }
    HILOG_DEBUG("%{public}s end.", __func__);
}
#else
void AbilityThread::DumpAbilityInfoInner(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    HILOG_DEBUG("%{public}s begin.", __func__);
    if (currentAbility_ != nullptr) {
        currentAbility_->Dump(params, info);
    }

    if (currentExtension_ != nullptr) {
        currentExtension_->Dump(params, info);
    }
    DumpOtherInfo(info);
}
#endif

void AbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (!abilityHandler_) {
        HILOG_DEBUG("abilityHandler_ is nullptr.");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (!runner) {
        HILOG_DEBUG("runner_ is nullptr.");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
    if (currentAbility_ != nullptr) {
        const auto ablityContext = currentAbility_->GetAbilityContext();
        if (!ablityContext) {
            HILOG_DEBUG("current ability context is nullptr.");
            return;
        }
        const auto localCallContainer = ablityContext->GetLocalCallContainer();
        if (!localCallContainer) {
            HILOG_DEBUG("current ability context locall call container is nullptr.");
            return;
        }
        localCallContainer->DumpCalls(info);
    }
}

void AbilityThread::CallRequest()
{
    HILOG_DEBUG("AbilityThread::CallRequest begin");

    if (!currentAbility_) {
        HILOG_ERROR("ability is nullptr.");
        return;
    }

    sptr<IRemoteObject> retval = nullptr;
    std::weak_ptr<OHOS::AppExecFwk::Ability> weakAbility = currentAbility_;
    auto syncTask = [ability = weakAbility, &retval] () {
        auto currentAbility = ability.lock();
        if (currentAbility == nullptr) {
            HILOG_ERROR("ability is nullptr.");
            return;
        }

        retval = currentAbility->CallRequest();
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("ability Handler is nullptr.");
        return;
    }

    abilityHandler_->PostSyncTask(syncTask);
    AbilityManagerClient::GetInstance()->CallRequestDone(token_, retval);
    HILOG_DEBUG("AbilityThread::CallRequest end");
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityThread::BlockAbility()
{
    HILOG_DEBUG("AbilityThread::BlockAblity begin");
    if (abilityHandler_) {
        auto task = []() {
            while (1) {
                std::this_thread::sleep_for(BLOCK_ABILITY_TIME*1s);
            }
        };
        abilityHandler_->PostTask(task);
        HILOG_DEBUG("AbilityThread::BlockAblity end");
        return ERR_OK;
    }
    return ERR_NO_INIT;
}
#endif

void AbilityThread::HandlePrepareTermianteAbility()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("share data error, abilityImpl_ == nullptr.");
        return;
    }
    isPrepareTerminate_ = abilityImpl_->PrepareTerminateAbility();
    HILOG_DEBUG("end, ret = %{public}d", isPrepareTerminate_);
    isPrepareTerminateAbilityDone_.store(true); 
    cv_.notify_all();
    HILOG_ERROR("[DongLin] notify");
}
}  // namespace AppExecFwk
}  // namespace OHOS
