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

#include "fa_ability_thread.h"

#include <chrono>
#include <functional>
#include <thread>

#include "ability_context_impl.h"
#include "ability_impl.h"
#include "ability_impl_factory.h"
#include "ability_loader.h"
#include "abs_shared_result_set.h"
#include "application_impl.h"
#include "context_deal.h"
#include "data_ability_predicates.h"
#include "dataobs_mgr_client.h"
#ifdef WITH_DLP
#include "dlp_file_kits.h"
#endif // WITH_DLP
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#ifdef SUPPORT_GRAPHICS
#include "page_ability_impl.h"
#endif
#include "ui_extension_utils.h"
#include "values_bucket.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace std::chrono_literals;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;
namespace {
constexpr static char ACE_SERVICE_ABILITY_NAME[] = "AceServiceAbility";
constexpr static char ACE_DATA_ABILITY_NAME[] = "AceDataAbility";
#ifdef SUPPORT_GRAPHICS
constexpr static char ABILITY_NAME[] = "Ability";
constexpr static char ACE_ABILITY_NAME[] = "AceAbility";
constexpr static char ACE_FORM_ABILITY_NAME[] = "AceFormAbility";
constexpr static char FORM_EXTENSION[] = "FormExtension";
constexpr static char UI_EXTENSION[] = "UIExtensionAbility";
constexpr static char MEDIA_CONTROL_EXTENSION[] = "MediaControlExtensionAbility";
constexpr static char USER_AUTH_EXTENSION[] = "UserAuthExtensionAbility";
constexpr static char ACTION_EXTENSION[] = "ActionExtensionAbility";
constexpr static char SHARE_EXTENSION[] = "ShareExtensionAbility";
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
#ifdef WITH_DLP
constexpr static char DLP_PARAMS_SANDBOX[] = "ohos.dlp.params.sandbox";
#endif // WITH_DLP
const int32_t PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS = 3000;
}

FAAbilityThread::FAAbilityThread() : abilityImpl_(nullptr), currentAbility_(nullptr) {}

FAAbilityThread::~FAAbilityThread()
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

    DelayedSingleton<AppExecFwk::AbilityImplFactory>::DestroyInstance();
}

std::string FAAbilityThread::CreateAbilityName(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    std::shared_ptr<AppExecFwk::OHOSApplication> &application)
{
    std::string abilityName;
    if (abilityRecord == nullptr || application == nullptr) {
        HILOG_ERROR("abilityRecord or application is nullptr");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility) {
        HILOG_DEBUG("AbilityInfo name is %{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->type == AppExecFwk::AbilityType::PAGE) {
        if (abilityInfo->isStageBasedModel) {
            abilityName = ABILITY_NAME;
        } else {
            abilityName = ACE_ABILITY_NAME;
        }
    } else if (abilityInfo->type == AppExecFwk::AbilityType::SERVICE) {
#else
    if (abilityInfo->type == AppExecFwk::AbilityType::SERVICE) {
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
    } else if (abilityInfo->type == AppExecFwk::AbilityType::DATA) {
        abilityName = ACE_DATA_ABILITY_NAME;
    } else if (abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        CreateExtensionAbilityName(application, abilityInfo, abilityName);
    } else {
        abilityName = abilityInfo->name;
    }

    HILOG_DEBUG("ability name is %{public}s", abilityName.c_str());
    return abilityName;
}

void FAAbilityThread::CreateExtensionAbilityName(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, std::string &abilityName)
{
    application->GetExtensionNameByType(static_cast<int32_t>(abilityInfo->extensionAbilityType), abilityName);
    if (abilityName.length() > 0) {
        HILOG_DEBUG("extension name: %{public}s", abilityName.c_str());
        return;
    }
    abilityName = BASE_SERVICE_EXTENSION;
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER) {
        abilityName = STATIC_SUBSCRIBER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::DRIVER) {
        abilityName = BASE_DRIVER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::DATASHARE) {
        abilityName = DATA_SHARE_EXT_ABILITY;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER) {
        abilityName = WORK_SCHEDULER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ACCESSIBILITY) {
        abilityName = ACCESSIBILITY_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::WALLPAPER) {
        abilityName = WALLPAPER_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION) {
        abilityName = FILEACCESS_EXT_ABILITY;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN) {
        abilityName = ENTERPRISE_ADMIN_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
        abilityName = INPUTMETHOD_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::APP_ACCOUNT_AUTHORIZATION) {
        abilityName = APP_ACCOUNT_AUTHORIZATION_EXTENSION;
    }
    CreateExtensionAbilityNameSupportGraphics(abilityInfo, abilityName);
    HILOG_DEBUG("extension abilityName: %{public}s", abilityName.c_str());
}

void FAAbilityThread::CreateExtensionAbilityNameSupportGraphics(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    std::string &abilityName)
{
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->formEnabled || abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FORM) {
        abilityName = FORM_EXTENSION;
    }
#endif
    if (AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType)) {
        if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SHARE) {
            abilityName = SHARE_EXTENSION;
        } else if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::ACTION) {
            abilityName = ACTION_EXTENSION;
        } else {
            abilityName = UI_EXTENSION;
        }
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH) {
        abilityName = USER_AUTH_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL) {
        abilityName = MEDIA_CONTROL_EXTENSION;
    }
}

std::shared_ptr<AppExecFwk::ContextDeal> FAAbilityThread::CreateAndInitContextDeal(
    std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::AbilityContext> &abilityObject)
{
    HILOG_DEBUG("begin");
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal = nullptr;
    if ((application == nullptr) || (abilityRecord == nullptr) || (abilityObject == nullptr)) {
        HILOG_ERROR("application or abilityRecord or abilityObject is nullptr");
        return contextDeal;
    }

    contextDeal = std::make_shared<AppExecFwk::ContextDeal>();
    contextDeal->SetAbilityInfo(abilityRecord->GetAbilityInfo());
    contextDeal->SetApplicationInfo(application->GetApplicationInfo());
    abilityObject->SetProcessInfo(application->GetProcessInfo());

    std::shared_ptr<AppExecFwk::Context> tmpContext = application->GetApplicationContext();
    contextDeal->SetApplicationContext(tmpContext);

    contextDeal->SetBundleCodePath(abilityRecord->GetAbilityInfo()->codePath);
    contextDeal->SetContext(abilityObject);
    return contextDeal;
}

void FAAbilityThread::Attach(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner, const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("application or abilityRecord or mainRunner is nullptr");
        return;
    }
    InitExtensionFlag(abilityRecord);
    if (isExtension_) {
        AttachExtension(application, abilityRecord, mainRunner);
        HILOG_DEBUG("Execute AttachExtension");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName.empty()) {
        HILOG_ERROR("abilityName is empty");
        return;
    }
    HILOG_DEBUG("begin ability: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);
    // new hap requires
    ability->AttachAbilityContext(
        BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_, stageContext));

    AttachInner(application, abilityRecord, stageContext);
}

void FAAbilityThread::AttachInner(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<Context> &stageContext)
{
    // 3.new abilityImpl
    abilityImpl_ = DelayedSingleton<AppExecFwk::AbilityImplFactory>::GetInstance()->MakeAbilityImplObject(
        abilityRecord->GetAbilityInfo());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->Init(application, abilityRecord, currentAbility_, abilityHandler_, token_);
    // 4. ability attach : ipc
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("err = %{public}d", err);
    }
}

void FAAbilityThread::AttachExtension(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if ((application == nullptr) || (abilityRecord == nullptr) || (mainRunner == nullptr)) {
        HILOG_ERROR("application or abilityRecord or mainRunner is nullptr");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName.empty()) {
        HILOG_ERROR("abilityName is empty");
        return;
    }
    HILOG_DEBUG("extension: %{public}s", abilityRecord->GetAbilityInfo()->name.c_str());
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto extension = AppExecFwk::AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        HILOG_ERROR("extension is nullptr");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("err = %{public}d", err);
    }
}

void FAAbilityThread::AttachExtension(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("application or abilityRecord is nullptr");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("runner is nullptr");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto extension = AppExecFwk::AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        HILOG_ERROR("extension is nullptr");
        return;
    }

    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    extensionImpl_ = std::make_shared<ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
        return;
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::Attach(std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord, const std::shared_ptr<Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        HILOG_ERROR("application or abilityRecord is nullptr");
        return;
    }
    InitExtensionFlag(abilityRecord);
    if (isExtension_) {
        AttachExtension(application, abilityRecord);
        HILOG_DEBUG("Execute AttachExtension");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    runner_ = AppExecFwk::EventRunner::Create(abilityName);
    if (runner_ == nullptr) {
        HILOG_ERROR("runner_ is nullptr");
        return;
    }
    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    // 2.new ability
    auto ability = AppExecFwk::AbilityLoader::GetInstance().GetAbilityByName(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    currentAbility_.reset(ability);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    abilityRecord->SetEventRunner(runner_);
    abilityRecord->SetAbilityThread(this);
    std::shared_ptr<AppExecFwk::AbilityContext> abilityObject = currentAbility_;
    std::shared_ptr<AppExecFwk::ContextDeal> contextDeal =
        CreateAndInitContextDeal(application, abilityRecord, abilityObject);
    ability->AttachBaseContext(contextDeal);
    // new hap requires
    ability->AttachAbilityContext(
        BuildAbilityContext(abilityRecord->GetAbilityInfo(), application, token_, stageContext));

    AttachInner(application, abilityRecord, stageContext);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    std::string connector = "##";
    std::string traceName = __PRETTY_FUNCTION__ + connector + want.GetElement().GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    HILOG_DEBUG("begin, name is %{public}s", want.GetElement().GetAbilityName().c_str());
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->SetCallingContext(lifeCycleStateInfo.caller.deviceId, lifeCycleStateInfo.caller.bundleName,
        lifeCycleStateInfo.caller.abilityName, lifeCycleStateInfo.caller.moduleName);
    abilityImpl_->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->HandleShareData(uniqueId);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleExtensionTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    extensionImpl_->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleConnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    sptr<IRemoteObject> service = abilityImpl_->ConnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleConnectAbilityDone(token_, service);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DisconnectAbility(want);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleDisconnectAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("err = %{public}d", err);
    }
}

void FAAbilityThread::HandleCommandAbility(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->CommandAbility(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    bool isAsyncCallback = false;
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->ConnectExtensionCallback(service);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }

    bool isAsyncCallback = false;
    extensionImpl_->DisconnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->DisconnectExtensionCallback();
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleCommandExtension(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        HILOG_ERROR("failed err = %{public}d", err);
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleCommandExtensionWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin");
    if (extensionImpl_ == nullptr) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }
    extensionImpl_->CommandExtensionWindow(want, sessionInfo, winCmd);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleSaveAbilityState()
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->DispatchSaveAbilityState();
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleRestoreAbilityState(const AppExecFwk::PacMap &state)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }
    abilityImpl_->DispatchRestoreAbilityState(state);
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }

        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionUpdateConfiguration(config);
        } else {
            abilityThread->HandleUpdateConfiguration(config);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return;
    }

    abilityImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("end");
}

void FAAbilityThread::HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    HILOG_DEBUG("begin");
    if (!extensionImpl_) {
        HILOG_ERROR("extensionImpl_ is nullptr");
        return;
    }

    extensionImpl_->ScheduleUpdateConfiguration(config);
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AppExecFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("name:%{public}s,targeState:%{public}d,isNewWant:%{public}d",
        want.GetElement().GetAbilityName().c_str(),
        lifeCycleStateInfo.state,
        lifeCycleStateInfo.isNewWant);

    if (token_ == nullptr) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
            Want newWant(want);
            newWant.CloseAllFd();
        } else {
            abilityThread->HandleAbilityTransaction(want, lifeCycleStateInfo, sessionInfo);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
}

void FAAbilityThread::ScheduleShareData(const int32_t &uniqueId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!token_) {
        HILOG_ERROR("token_ is nullptr");
        return;
    }
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, uniqueId]() {
        auto abilityThread = weak.promote();
        if (!abilityThread) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        abilityThread->HandleShareData(uniqueId);
    };

    if (!abilityHandler_) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("postTask error");
    }
}

void FAAbilityThread::ScheduleConnectAbility(const Want &want)
{
    HILOG_DEBUG("begin, isExtension_: %{public}d", isExtension_);
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleConnectExtension(want);
        } else {
            abilityThread->HandleConnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("begin, isExtension: %{public}d", isExtension_);
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            abilityThread->HandleDisconnectExtension(want);
        } else {
            abilityThread->HandleDisconnectAbility(want);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
}

void FAAbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    HILOG_DEBUG("begin. startId: %{public}d", startId);
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (abilityThread->isExtension_) {
            Want newWant(want);
#ifdef WITH_DLP
            bool sandboxFlag = Security::DlpPermission::DlpFileKits::GetSandboxFlag(newWant);
            newWant.SetParam(DLP_PARAMS_SANDBOX, sandboxFlag);
            if (sandboxFlag) {
                newWant.CloseAllFd();
            }
#endif // WITH_DLP
            abilityThread->HandleCommandExtension(newWant, restart, startId);
            newWant.CloseAllFd();
        } else {
            abilityThread->HandleCommandAbility(want, restart, startId);
        }
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

bool FAAbilityThread::SchedulePrepareTerminateAbility()
{
    HILOG_DEBUG("begin");
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return true;
    }
    if (getpid() == gettid()) {
        bool ret = abilityImpl_->PrepareTerminateAbility();
        HILOG_DEBUG("end, ret = %{public}d", ret);
        return ret;
    } else {
        wptr<FAAbilityThread> weak = this;
        auto task = [weak]() {
            auto abilityThread = weak.promote();
            if (abilityThread == nullptr) {
                HILOG_ERROR("abilityThread is nullptr");
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
        auto condition = [weak] {
            auto abilityThread = weak.promote();
            if (abilityThread == nullptr) {
                HILOG_ERROR("abilityThread is nullptr");
                return false;
            }
            return abilityThread->isPrepareTerminateAbilityDone_;
        };
        if (!cv_.wait_for(lock, std::chrono::milliseconds(PREPARE_TO_TERMINATE_TIMEOUT_MILLISECONDS), condition)) {
            HILOG_WARN("Wait timeout");
        }
        HILOG_DEBUG("end, ret = %{public}d", isPrepareTerminate_);
        return isPrepareTerminate_;
    }
}

void FAAbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HILOG_DEBUG("begin");
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, want, sessionInfo, winCmd]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        abilityThread->HandleCommandExtensionWindow(want, sessionInfo, winCmd);
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

void FAAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    HILOG_DEBUG("begin");
    wptr<FAAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            HILOG_ERROR("abilityThread is nullptr");
            return;
        }
        if (requestCode == -1) {
            HILOG_ERROR("requestCode is -1");
            return;
        }
        if (abilityThread->isExtension_ && abilityThread->extensionImpl_ != nullptr) {
            abilityThread->extensionImpl_->SendResult(requestCode, resultCode, want);
            return;
        } else if (!abilityThread->isExtension_ && abilityThread->abilityImpl_ != nullptr) {
            abilityThread->abilityImpl_->SendResult(requestCode, resultCode, want);
            return;
        }
        HILOG_ERROR("%{public}simpl is nullptr", abilityThread->isExtension_ ? "extension" : "ability");
    };

    if (abilityHandler_ == nullptr) {
        HILOG_ERROR("abilityHandler_ is nullptr");
        return;
    }

    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        HILOG_ERROR("PostTask error");
    }
    HILOG_DEBUG("end");
}

std::vector<std::string> FAAbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_DEBUG("begin");
    std::vector<std::string> types;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return types;
    }

    types = abilityImpl_->GetFileTypes(uri, mimeTypeFilter);
    HILOG_DEBUG("end");
    return types;
}

int FAAbilityThread::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_DEBUG("begin");
    int32_t fd = -1;
    if (abilityImpl_ == nullptr) {
        HILOG_ERROR("abilityImpl_ is nullptr");
        return fd;
    }

    fd = abilityImpl_->OpenFile(uri, mode);
    HILOG_DEBUG("end");
    return fd;
}
