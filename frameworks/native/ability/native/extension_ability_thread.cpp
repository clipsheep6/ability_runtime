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

#include "extension_ability_thread.h"

#include "ability_context_impl.h"
#include "ability_handler.h"
#include "ability_loader.h"
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
namespace {
#ifdef SUPPORT_GRAPHICS
constexpr static char FORM_EXTENSION[] = "FormExtension";
constexpr static char UI_EXTENSION[] = "UIExtensionAbility";
constexpr static char CUSTOM_EXTENSION[] = "ExtensionAbility";
constexpr static char MEDIA_CONTROL_EXTENSION[] = "MediaControlExtensionAbility";
constexpr static char USER_AUTH_EXTENSION[] = "UserAuthExtensionAbility";
constexpr static char ACTION_EXTENSION[] = "ActionExtensionAbility";
constexpr static char SHARE_EXTENSION[] = "ShareExtensionAbility";
constexpr static char AUTO_FILL_EXTENSION[] = "AutoFillExtensionAbility";
constexpr static char EMBEDDED_UI_EXTENSION[] = "EmbeddedUIExtensionAbility";
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
constexpr static char VPN_EXTENSION[] = "VpnExtension";
}

const std::map<AppExecFwk::ExtensionAbilityType, std::string> UI_EXTENSION_NAME_MAP = {
    { AppExecFwk::ExtensionAbilityType::SHARE, SHARE_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::ACTION, ACTION_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD, AUTO_FILL_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART, AUTO_FILL_EXTENSION },
    { AppExecFwk::ExtensionAbilityType::EMBEDDED_UI, EMBEDDED_UI_EXTENSION }
};

ExtensionAbilityThread::ExtensionAbilityThread() : extensionImpl_(nullptr), currentExtension_(nullptr) {}

ExtensionAbilityThread::~ExtensionAbilityThread()
{
    if (currentExtension_ != nullptr) {
        currentExtension_.reset();
    }
}

std::string ExtensionAbilityThread::CreateAbilityName(
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application)
{
    std::string abilityName;
    if (abilityRecord == nullptr || application == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "AbilityRecord or application is nullptr.");
        return abilityName;
    }

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "AbilityInfo is nullptr.");
        return abilityName;
    }

    if (abilityInfo->isNativeAbility || abilityInfo->type != AppExecFwk::AbilityType::EXTENSION) {
        TAG_LOGD(AAFwkTag::EXT, "Ability info name is %{public}s", abilityInfo->name.c_str());
        return abilityInfo->name;
    }

    application->GetExtensionNameByType(static_cast<int32_t>(abilityInfo->extensionAbilityType), abilityName);
    if (!abilityName.empty()) {
        TAG_LOGD(AAFwkTag::EXT, "Get extension name: %{public}s success.", abilityName.c_str());
        return abilityName;
    }

    abilityName = BASE_SERVICE_EXTENSION;
#ifdef SUPPORT_GRAPHICS
    if (abilityInfo->formEnabled || abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::FORM) {
        abilityName = FORM_EXTENSION;
    }
#endif
    if (AAFwk::UIExtensionUtils::IsUIExtension(abilityInfo->extensionAbilityType)) {
        auto iter = UI_EXTENSION_NAME_MAP.find(abilityInfo->extensionAbilityType);
        if (iter != UI_EXTENSION_NAME_MAP.end()) {
            abilityName = iter->second;
        } else {
            abilityName = UI_EXTENSION;
        }
    }
    CreateExtensionAbilityName(abilityInfo, abilityName);
    TAG_LOGD(AAFwkTag::EXT, "Ability name is %{public}s.", abilityName.c_str());
    return abilityName;
}

void ExtensionAbilityThread::CreateExtensionAbilityName(
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, std::string &abilityName)
{
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
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSPICKER_MEDIACONTROL) {
        abilityName = MEDIA_CONTROL_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::APP_ACCOUNT_AUTHORIZATION) {
        abilityName = APP_ACCOUNT_AUTHORIZATION_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::SYSDIALOG_USERAUTH) {
        abilityName = USER_AUTH_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::VPN) {
        abilityName = VPN_EXTENSION;
    }
    if (abilityInfo->extensionAbilityType == AppExecFwk::ExtensionAbilityType::UNSPECIFIED &&
        abilityInfo->type == AppExecFwk::AbilityType::EXTENSION) {
        abilityName = abilityInfo->extensionTypeName + CUSTOM_EXTENSION;
    }
}

void ExtensionAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner,
    [[maybe_unused]] const std::shared_ptr<Context> &appContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (application == nullptr || abilityRecord == nullptr || mainRunner == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "application or abilityRecord or mainRunner is nullptr.");
        return;
    }
    HandleAttach(application, abilityRecord, mainRunner);
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::Attach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    [[maybe_unused]] const std::shared_ptr<Context> &appContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if ((application == nullptr) || (abilityRecord == nullptr)) {
        TAG_LOGE(AAFwkTag::EXT, "application or abilityRecord is nullptr.");
        return;
    }
    HandleAttach(application, abilityRecord, nullptr);
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleAttach(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AppExecFwk::EventRunner> &mainRunner)
{
    if (application == nullptr || abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Application or abilityRecord is nullptr.");
        return;
    }

    // 1.new AbilityHandler
    std::string abilityName = CreateAbilityName(abilityRecord, application);
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "AbilityName is empty.");
        return;
    }

    TAG_LOGD(AAFwkTag::EXT, "Begin, extension: %{public}s.", abilityName.c_str());
    if (mainRunner == nullptr) {
        runner_ = AppExecFwk::EventRunner::Create(abilityName);
        if (runner_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "runner_ is nullptr.");
            return;
        }
        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
        if (abilityHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
            return;
        }
    } else {
        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(mainRunner);
        if (abilityHandler_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
            return;
        }
    }

    // 2.new ability
    auto extension = AppExecFwk::AbilityLoader::GetInstance().GetExtensionByName(abilityName);
    if (extension == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Extension is nullptr.");
        return;
    }
    currentExtension_.reset(extension);
    token_ = abilityRecord->GetToken();
    abilityRecord->SetEventHandler(abilityHandler_);
    mainRunner == nullptr ? abilityRecord->SetEventRunner(runner_) : abilityRecord->SetEventRunner(mainRunner);
    abilityRecord->SetAbilityThread(this);
    HandleAttachInner(application, abilityRecord);
}

void ExtensionAbilityThread::HandleAttachInner(const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &abilityRecord)
{
    extensionImpl_ = std::make_shared<ExtensionImpl>();
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr");
        return;
    }

    // 3.new init
    extensionImpl_->Init(application, abilityRecord, currentExtension_, abilityHandler_, token_);
    // 4.ipc attach init
    ErrCode err = AbilityManagerClient::GetInstance()->AttachAbilityThread(this, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Attach failed err is %{public}d", err);
    }
}

void ExtensionAbilityThread::HandleExtensionTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    extensionImpl_->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleConnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    bool isAsyncCallback = false;
    sptr<IRemoteObject> service = extensionImpl_->ConnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->ConnectExtensionCallback(service);
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleDisconnectExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }

    bool isAsyncCallback = false;
    extensionImpl_->DisconnectExtension(want, isAsyncCallback);
    if (!isAsyncCallback) {
        extensionImpl_->DisconnectExtensionCallback();
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleCommandExtension(const Want &want, bool restart, int32_t startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    extensionImpl_->CommandExtension(want, restart, startId);
    ErrCode err = AbilityManagerClient::GetInstance()->ScheduleCommandAbilityDone(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Failed err is %{public}d.", err);
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleInsightIntent(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    auto ret = extensionImpl_->HandleInsightIntent(want);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ HandleInsightIntent failed.");
        return;
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleCommandExtensionWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    extensionImpl_->CommandExtensionWindow(want, sessionInfo, winCmd);
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::ScheduleUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, config]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "Ability thread is nullptr.");
            return;
        }
        abilityThread->HandleExtensionUpdateConfiguration(config);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error");
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::HandleExtensionUpdateConfiguration(const AppExecFwk::Configuration &config)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    extensionImpl_->ScheduleUpdateConfiguration(config);
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::ScheduleAbilityTransaction(
    const Want &want, const LifeCycleStateInfo &lifeCycleStateInfo, sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Name is %{public}s, targeState is %{public}d, isNewWant is %{public}d",
        want.GetElement().GetAbilityName().c_str(), lifeCycleStateInfo.state, lifeCycleStateInfo.isNewWant);
    if (token_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "token_ is nullptr.");
        return;
    }
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want, lifeCycleStateInfo, sessionInfo]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "Ability thread is nullptr.");
            return;
        }
        abilityThread->HandleExtensionTransaction(want, lifeCycleStateInfo, sessionInfo);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
}

void ExtensionAbilityThread::ScheduleConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "Called.");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandleConnectExtension(want);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
}

void ExtensionAbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "Called.");
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandleDisconnectExtension(want);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
}

void ExtensionAbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int32_t startId)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin startId: %{public}d", startId);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    ScheduleCommandAbilityInner(want, restart, startId);
    if (AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        ScheduleInsightIntentInner(want);
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::ScheduleCommandAbilityInner(const Want &want, bool restart, int32_t startId)
{
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want, restart, startId]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandleCommandExtension(want, restart, startId);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
}

void ExtensionAbilityThread::ScheduleInsightIntentInner(const Want &want)
{
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandleInsightIntent(want);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
}

void ExtensionAbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin, winCmd: %{public}d.", winCmd);
    if (abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, want, sessionInfo, winCmd]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }
        abilityThread->HandleCommandExtensionWindow(want, sessionInfo, winCmd);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::SendResult(int requestCode, int resultCode, const Want &want)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (abilityHandler_ == nullptr || requestCode == -1) {
        TAG_LOGE(AAFwkTag::EXT, "abilityHandler_ is nullptr or requestCode is -1.");
        return;
    }

    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, requestCode, resultCode, want]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }

        if (abilityThread->extensionImpl_ == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "ExtensionImpl is nullptr.");
            return;
        }
        abilityThread->extensionImpl_->SendResult(requestCode, resultCode, want);
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::NotifyMemoryLevel(int32_t level)
{
    TAG_LOGD(AAFwkTag::EXT, "called, result: %{public}d.", level);
    if (extensionImpl_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "extensionImpl_ is nullptr.");
        return;
    }
    extensionImpl_->NotifyMemoryLevel(level);
}

void ExtensionAbilityThread::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (token_ == nullptr || abilityHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "token_ or abilityHandler_ is nullptr.");
        return;
    }
    wptr<ExtensionAbilityThread> weak = this;
    auto task = [weak, params, token = token_]() {
        auto abilityThread = weak.promote();
        if (abilityThread == nullptr) {
            TAG_LOGE(AAFwkTag::EXT, "AbilityThread is nullptr.");
            return;
        }
        std::vector<std::string> dumpInfo;
        abilityThread->DumpAbilityInfoInner(params, dumpInfo);
        ErrCode err = AbilityManagerClient::GetInstance()->DumpAbilityInfoDone(dumpInfo, token);
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::EXT, "Dump ability info failed err is %{public}d.", err);
        }
    };
    bool ret = abilityHandler_->PostTask(task);
    if (!ret) {
        TAG_LOGE(AAFwkTag::EXT, "PostTask error.");
    }
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::DumpAbilityInfoInner(
    const std::vector<std::string> &params, std::vector<std::string> &info)
{
    TAG_LOGD(AAFwkTag::EXT, "Begin.");
    if (currentExtension_ == nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "currentExtension_ is nullptr.");
        return;
    }
    currentExtension_->Dump(params, info);

#ifdef SUPPORT_GRAPHICS
    if (params.empty()) {
        DumpOtherInfo(info);
        return;
    }
#else
    DumpOtherInfo(info);
#endif
    TAG_LOGD(AAFwkTag::EXT, "End.");
}

void ExtensionAbilityThread::DumpOtherInfo(std::vector<std::string> &info)
{
    std::string dumpInfo = "        event:";
    info.push_back(dumpInfo);
    if (abilityHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "abilityHandler_ is nullptr.");
        return;
    }
    auto runner = abilityHandler_->GetEventRunner();
    if (runner == nullptr) {
        TAG_LOGD(AAFwkTag::EXT, "runner_ is nullptr.");
        return;
    }
    dumpInfo = "";
    runner->DumpRunnerInfo(dumpInfo);
    info.push_back(dumpInfo);
}
} // namespace AbilityRuntime
} // namespace OHOS
