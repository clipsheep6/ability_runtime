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

#include "ability_record.h"

#include <singleton.h>
#include <vector>

#include "ability_event_handler.h"
#include "ability_manager_service.h"
#include "ability_scheduler_stub.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_client.h"
#include "hitrace_meter.h"
#include "errors.h"
#include "hilog_wrapper.h"
#ifdef OS_ACCOUNT_PART_ENABLED
#include "os_account_manager.h"
#endif // OS_ACCOUNT_PART_ENABLED
#include "system_ability_token_callback.h"
#include "uri_permission_manager_client.h"
#ifdef SUPPORT_GRAPHICS
#include "image_source.h"
#include "locale_config.h"
#include "mission_info_mgr.h"
#endif

namespace OHOS {
namespace AAFwk {
using namespace OHOS::Security;
const std::string DEBUG_APP = "debugApp";
const std::string DMS_PROCESS_NAME = "distributedsched";
const std::string DMS_MISSION_ID = "dmsMissionId";
const std::string DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";
const std::string ABILITY_OWNER_USERID = "AbilityMS_Owner_UserId";
const std::u16string SYSTEM_ABILITY_TOKEN_CALLBACK = u"ohos.aafwk.ISystemAbilityTokenCallback";
const std::string SHOW_ON_LOCK_SCREEN = "ShowOnLockScreen";
int64_t AbilityRecord::abilityRecordId = 0;
int64_t AbilityRecord::g_abilityRecordEventId_ = 0;
const int32_t DEFAULT_USER_ID = 0;
const int32_t SEND_RESULT_CANCELED = -1;
const int DEFAULT_REQUEST_CODE = -1;
const int VECTOR_SIZE = 2;
const std::map<AbilityState, std::string> AbilityRecord::stateToStrMap = {
    std::map<AbilityState, std::string>::value_type(INITIAL, "INITIAL"),
    std::map<AbilityState, std::string>::value_type(INACTIVE, "INACTIVE"),
    std::map<AbilityState, std::string>::value_type(ACTIVE, "ACTIVE"),
    std::map<AbilityState, std::string>::value_type(INACTIVATING, "INACTIVATING"),
    std::map<AbilityState, std::string>::value_type(ACTIVATING, "ACTIVATING"),
    std::map<AbilityState, std::string>::value_type(TERMINATING, "TERMINATING"),
    std::map<AbilityState, std::string>::value_type(FOREGROUND, "FOREGROUND"),
    std::map<AbilityState, std::string>::value_type(BACKGROUND, "BACKGROUND"),
    std::map<AbilityState, std::string>::value_type(FOREGROUNDING, "FOREGROUNDING"),
    std::map<AbilityState, std::string>::value_type(BACKGROUNDING, "BACKGROUNDING"),
    std::map<AbilityState, std::string>::value_type(FOREGROUND_FAILED, "FOREGROUND_FAILED"),
};
const std::map<AppState, std::string> AbilityRecord::appStateToStrMap_ = {
    std::map<AppState, std::string>::value_type(AppState::BEGIN, "BEGIN"),
    std::map<AppState, std::string>::value_type(AppState::READY, "READY"),
    std::map<AppState, std::string>::value_type(AppState::FOREGROUND, "FOREGROUND"),
    std::map<AppState, std::string>::value_type(AppState::BACKGROUND, "BACKGROUND"),
    std::map<AppState, std::string>::value_type(AppState::SUSPENDED, "SUSPENDED"),
    std::map<AppState, std::string>::value_type(AppState::TERMINATED, "TERMINATED"),
    std::map<AppState, std::string>::value_type(AppState::END, "END"),
};
const std::map<AbilityLifeCycleState, AbilityState> AbilityRecord::convertStateMap = {
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INITIAL, INITIAL),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INACTIVE, INACTIVE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_ACTIVE, ACTIVE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_FOREGROUND_NEW, FOREGROUND),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_BACKGROUND_NEW, BACKGROUND),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_FOREGROUND_FAILED, FOREGROUND_FAILED),
};
#ifndef OS_ACCOUNT_PART_ENABLED
const int32_t DEFAULT_OS_ACCOUNT_ID = 0; // 0 is the default id when there is no os_account part
#endif // OS_ACCOUNT_PART_ENABLED

Token::Token(std::weak_ptr<AbilityRecord> abilityRecord) : abilityRecord_(abilityRecord)
{}

Token::~Token()
{}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(token, nullptr);
    return (static_cast<Token *>(token.GetRefPtr()))->GetAbilityRecord();
}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecord() const
{
    return abilityRecord_.lock();
}

AbilityRecord::AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : want_(want), abilityInfo_(abilityInfo), applicationInfo_(applicationInfo), requestCode_(requestCode)
{
    recordId_ = abilityRecordId++;
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (abilityMgr) {
        abilityMgr->GetMaxRestartNum(restratMax_);
    }
    restartCount_ = restratMax_;
}

AbilityRecord::~AbilityRecord()
{
    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto object = scheduler_->AsObject();
        if (object != nullptr) {
            object->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
}

std::shared_ptr<AbilityRecord> AbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    std::shared_ptr<AbilityRecord> abilityRecord = std::make_shared<AbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    CHECK_POINTER_AND_RETURN(abilityRecord, nullptr);
    abilityRecord->SetUid(abilityRequest.uid);
    if (!abilityRecord->Init()) {
        HILOG_ERROR("failed to init new ability record");
        return nullptr;
    }
    if (abilityRequest.startSetting != nullptr) {
        HILOG_INFO("abilityRequest.startSetting...");
        abilityRecord->SetStartSetting(abilityRequest.startSetting);
    }
    if (abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        HILOG_INFO("abilityRequest.callType is CALL_REQUEST_TYPE.");
        abilityRecord->SetStartedByCall(true);
    }
    return abilityRecord;
}

bool AbilityRecord::Init()
{
    lifecycleDeal_ = std::make_unique<LifecycleDeal>();
    CHECK_POINTER_RETURN_BOOL(lifecycleDeal_);

    token_ = new (std::nothrow) Token(weak_from_this());
    CHECK_POINTER_RETURN_BOOL(token_);

    if (applicationInfo_.isLauncherApp) {
        isLauncherAbility_ = true;
    }
    return true;
}

void AbilityRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t AbilityRecord::GetUid()
{
    return uid_;
}

int AbilityRecord::LoadAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Start load ability, name is %{public}s.", abilityInfo_.name.c_str());
    if (abilityInfo_.type != AppExecFwk::AbilityType::DATA) {
        auto loadTimeOut = want_.GetBoolParam("coldStart", false) ?
            AbilityManagerService::COLDSTART_LOAD_TIMEOUT : AbilityManagerService::LOAD_TIMEOUT;
        SendEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, loadTimeOut);
    }

    startTime_ = AbilityUtil::SystemTimeMillis();
    CHECK_POINTER_AND_RETURN(token_, ERR_INVALID_VALUE);
    std::string appName = applicationInfo_.name;
    if (appName.empty()) {
        HILOG_ERROR("app name is empty");
        return ERR_INVALID_VALUE;
    }

    if (!CanRestartRootLauncher()) {
        HILOG_ERROR("Root launcher restart is out of max count.");
        return ERR_INVALID_VALUE;
    }

    sptr<Token> callerToken_ = nullptr;
    if (!callerList_.empty() && callerList_.back()) {
        auto caller = callerList_.back()->GetCaller();
        if (caller) {
            callerToken_ = caller->GetToken();
        }
    }
    want_.SetParam(ABILITY_OWNER_USERID, ownerMissionUserId_);
    auto result = DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        token_, callerToken_, abilityInfo_, applicationInfo_, want_);
    want_.RemoveParam(ABILITY_OWNER_USERID);
    return result;
}

bool AbilityRecord::CanRestartRootLauncher()
{
    if (isLauncherRoot_ && isRestarting_ && IsLauncherAbility() && (restartCount_ <= 0)) {
        HILOG_ERROR("Root launcher restart is out of max count.");
        return false;
    }
    return true;
}

void AbilityRecord::ForegroundAbility(const Closure &task, uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Start to foreground ability, name is %{public}s.", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::FOREGROUNDNEW_TIMEOUT_MSG, AbilityManagerService::FOREGROUNDNEW_TIMEOUT);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        handler->PostTask(task, "CancelStartingWindow", AbilityManagerService::FOREGROUNDNEW_TIMEOUT);
    }

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::FOREGROUNDING;
    lifeCycleStateInfo_.sceneFlag = sceneFlag;
    lifecycleDeal_->ForegroundNew(want_, lifeCycleStateInfo_);
    lifeCycleStateInfo_.sceneFlag = 0;
    lifeCycleStateInfo_.sceneFlagBak = 0;

    // update ability state to appMgr service when restart
    if (IsNewWant()) {
        sptr<Token> preToken = nullptr;
        if (GetPreAbilityRecord()) {
            preToken = GetPreAbilityRecord()->GetToken();
        }
        DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token_, preToken, 1, 1, 1);
    }
}

void AbilityRecord::ProcessForegroundAbility(uint32_t sceneFlag)
{
    std::string element = GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability record: %{public}s", element.c_str());

    if (isReady_) {
        if (IsAbilityState(AbilityState::BACKGROUND)) {
            // background to activte state
            HILOG_DEBUG("MoveToForground, %{public}s", element.c_str());
            lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token_);
        } else {
            HILOG_DEBUG("Activate %{public}s", element.c_str());
            ForegroundAbility(nullptr, sceneFlag);
        }
    } else {
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        LoadAbility();
    }
}

#ifdef SUPPORT_GRAPHICS
void AbilityRecord::ProcessForegroundAbility(bool isRecent, const AbilityRequest &abilityRequest,
    std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility,
    uint32_t sceneFlag)
{
    std::string element = GetWant().GetElement().GetURI();
    HILOG_INFO("ability record: %{public}s", element.c_str());

    if (isReady_) {
        if (IsAbilityState(AbilityState::BACKGROUND)) {
            // background to activte state
            HILOG_DEBUG("MoveToForground, %{public}s", element.c_str());
            lifeCycleStateInfo_.sceneFlagBak = sceneFlag;

            StartingWindowTask(isRecent, false, abilityRequest, startOptions);
            AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
            CancelStartingWindowHotTask();

            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForground(token_);
        } else {
            HILOG_DEBUG("Activate %{public}s", element.c_str());
            ForegroundAbility(nullptr, sceneFlag);
        }
    } else {
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        StartingWindowTask(isRecent, true, abilityRequest, startOptions);
        AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
        CancelStartingWindowColdTask();
        LoadAbility();
    }
}

std::shared_ptr<Want> AbilityRecord::GetWantFromMission() const
{
    InnerMissionInfo innerMissionInfo;
    int getMission = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
        missionId_, innerMissionInfo);
    if (getMission != ERR_OK) {
        HILOG_ERROR("cannot find mission info from MissionInfoList by missionId: %{public}d", missionId_);
        return nullptr;
    }

    return std::make_shared<Want>(innerMissionInfo.missionInfo.want);
}

void AbilityRecord::AnimationTask(bool isRecent, const AbilityRequest &abilityRequest,
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility)
{
    HILOG_INFO("%{public}s was called.", __func__);
    if (abilityInfo_.name == AbilityConfig::GRANT_ABILITY_ABILITY_NAME &&
        abilityInfo_.bundleName == AbilityConfig::GRANT_ABILITY_BUNDLE_NAME) {
        HILOG_INFO("%{public}s, ignore GrantAbility.", __func__);
        return;
    }

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (!handler) {
        HILOG_ERROR("Fail to get AbilityEventHandler.");
        return;
    }

    auto self(weak_from_this());
    if (isRecent) {
        auto want = GetWantFromMission();
        auto task = [self, startOptions, want] {
            auto ability = self.lock();
            if (ability) {
                ability->NotifyAnimationFromRecentTask(startOptions, want);
            }
        };
        handler->PostTask(task, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        if (!IsForeground()) {
            auto task = [self, callerAbility, abilityRequest] {
                auto ability = self.lock();
                if (ability) {
                    ability->NotifyAnimationFromStartingAbility(callerAbility, abilityRequest);
                }
            };
            handler->PostTask(task, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        }
    }
}

void AbilityRecord::SetShowWhenLocked(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<AbilityTransitionInfo> &info) const
{
    std::vector<AppExecFwk::CustomizeData> datas = abilityInfo.metaData.customizeData;
    for (AppExecFwk::CustomizeData data : datas) {
        if (data.name == SHOW_ON_LOCK_SCREEN) {
            info->isShowWhenLocked_ = true;
            break;
        }
    }
}

void AbilityRecord::SetAbilityTransitionInfo(const AppExecFwk::AbilityInfo &abilityInfo,
    sptr<AbilityTransitionInfo> &info) const
{
    info->abilityName_ = abilityInfo.name;
    info->bundleName_ = abilityInfo.bundleName;
    SetShowWhenLocked(abilityInfo, info);
}

void AbilityRecord::NotifyAnimationFromRecentTask(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want) const
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    auto toInfo = CreateAbilityTransitionInfo(token_, startOptions, want);
    SetAbilityTransitionInfo(abilityInfo_, toInfo);
    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    fromInfo->isRecent_ = true;
    windowHandler->NotifyWindowTransition(fromInfo, toInfo);
}

void AbilityRecord::NotifyAnimationFromStartingAbility(const std::shared_ptr<AbilityRecord> &callerAbility,
    const AbilityRequest &abilityRequest) const
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    if (callerAbility) {
        auto callerAbilityInfo = callerAbility->GetAbilityInfo();
        SetAbilityTransitionInfo(callerAbilityInfo, fromInfo);
        fromInfo->abilityToken_ = callerAbility->GetToken();
    } else {
        fromInfo->abilityToken_ = abilityRequest.callerToken;
    }

    auto toInfo = CreateAbilityTransitionInfo(abilityRequest, token_);
    SetAbilityTransitionInfo(abilityInfo_, toInfo);

    windowHandler->NotifyWindowTransition(fromInfo, toInfo);
}

void AbilityRecord::StartingWindowTask(bool isRecent, bool isCold, const AbilityRequest &abilityRequest,
    std::shared_ptr<StartOptions> &startOptions)
{
    HILOG_INFO("%{public}s was called.", __func__);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (!handler) {
        HILOG_ERROR("Fail to get AbilityEventHandler.");
        return;
    }

    auto self(weak_from_this());
    if (isRecent) {
        auto want = GetWantFromMission();
        auto task = [self, startOptions, want, isCold] {
            auto ability = self.lock();
            if (ability) {
                AbilityRequest abilityRequest;
                if (isCold) {
                    ability->StartingWindowCold(startOptions, want, abilityRequest);
                } else {
                    ability->StartingWindowHot(startOptions, want, abilityRequest);
                }
            }
        };
        handler->PostTask(task, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    } else {
        auto task = [self, abilityRequest, isCold] {
            auto ability = self.lock();
            if (ability) {
                std::shared_ptr<StartOptions> startOptions = nullptr;
                std::shared_ptr<Want> want = nullptr;
                if (isCold) {
                    ability->StartingWindowCold(startOptions, want, abilityRequest);
                } else {
                    ability->StartingWindowHot(startOptions, want, abilityRequest);
                }
            }
        };
        handler->PostTask(task, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}

void AbilityRecord::CancelStartingWindowHotTask()
{
    HILOG_INFO("%{public}s was called.", __func__);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (!handler) {
        HILOG_ERROR("Fail to get AbilityEventHandler.");
        return;
    }

    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_ERROR("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    auto abilityRecord(shared_from_this());
    auto delayTask = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING) {
            HILOG_INFO("%{public}s, call windowHandler CancelStartingWindow.", __func__);
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    handler->PostTask(delayTask, "CancelStartingWindowHot", AbilityManagerService::FOREGROUNDNEW_TIMEOUT);
}

void AbilityRecord::CancelStartingWindowColdTask()
{
    HILOG_INFO("%{public}s was called.", __func__);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (!handler) {
        HILOG_ERROR("Fail to get AbilityEventHandler.");
        return;
    }

    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_ERROR("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    auto abilityRecord(shared_from_this());
    auto delayTask = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            (abilityRecord->GetScheduler() == nullptr ||
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING)) {
            HILOG_INFO("%{public}s, call windowHandler CancelStartingWindow.", __func__);
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    handler->PostTask(delayTask, "CancelStartingWindowCold", AbilityManagerService::FOREGROUNDNEW_TIMEOUT);
}

sptr<IWindowManagerServiceHandler> AbilityRecord::GetWMSHandler() const
{
    auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
    if (!abilityMgr) {
        HILOG_WARN("%{public}s, Get Ability Manager Service failed.", __func__);
        return nullptr;
    }
    return abilityMgr->GetWMSHandler();
}

void AbilityRecord::SetWindowModeAndDisplayId(sptr<AbilityTransitionInfo> &info,
    const std::shared_ptr<Want> &want) const
{
    if (!want) {
        HILOG_WARN("%{public}s, want is invalid.", __func__);
        return;
    }
    auto mode = want->GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
    auto displayId = want->GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -1);
    if (mode != -1) {
        info->mode_ = static_cast<uint32_t>(mode);
    }
    if (displayId != -1) {
        info->displayId_ = static_cast<uint64_t>(displayId);
    }
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(const sptr<IRemoteObject> abilityToken,
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    if (startOptions != nullptr) {
        info->mode_ = static_cast<uint32_t>(startOptions->GetWindowMode());
        info->displayId_ = static_cast<uint64_t>(startOptions->GetDisplayID());
    } else {
        SetWindowModeAndDisplayId(info, want);
    }
    info->abilityToken_ = abilityToken;
    return info;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(const AbilityRequest &abilityRequest,
    const sptr<IRemoteObject> abilityToken) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    auto abilityStartSetting = abilityRequest.startSetting;
    if (abilityStartSetting) {
        int base = 10; // Numerical base (radix) that determines the valid characters and their interpretation.
        auto mode =
            strtol(abilityStartSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY).c_str(), nullptr, base);
        info->mode_ = static_cast<uint32_t>(mode);
        auto displayId =
            strtol(abilityStartSetting->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY).c_str(), nullptr, base);
        info->displayId_ = static_cast<uint64_t>(displayId);
    } else {
        SetWindowModeAndDisplayId(info, std::make_shared<Want>(abilityRequest.want));
    }
    info->abilityToken_ = abilityToken;
    return info;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityRecord::CreateResourceManager(
    const AppExecFwk::AbilityInfo &abilityInfo) const
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    if (!resourceMgr->AddResource(abilityInfo.resourcePath.c_str())) {
        HILOG_WARN("%{public}s AddResource failed.", __func__);
    }

    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);
    resourceMgr->UpdateResConfig(*resConfig);
    return resourceMgr;
}

sptr<Media::PixelMap> AbilityRecord::GetPixelMap(const uint32_t windowIconId,
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr) const
{
    std::string iconPath;
    auto iconPathErrval = resourceMgr->GetMediaById(windowIconId, iconPath);
    if (iconPathErrval != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_ERROR("GetMediaById iconPath failed");
        return nullptr;
    }
    HILOG_DEBUG("GetMediaById iconPath: %{private}s", iconPath.c_str());

    uint32_t errorCode = 0;
    Media::SourceOptions opts;
    auto imageSource = Media::ImageSource::CreateImageSource(iconPath, opts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("Failed to create image source path %{private}s err %{public}d", iconPath.c_str(), errorCode);
        return nullptr;
    }

    Media::DecodeOptions decodeOpts;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("Failed to create pixelmap path %{private}s err %{public}d", iconPath.c_str(), errorCode);
        return nullptr;
    }
    HILOG_DEBUG("%{public}s OUT.", __func__);
    return sptr<Media::PixelMap>(pixelMapPtr.release());
}

void AbilityRecord::StartingWindowHot(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest)
{
    if (abilityInfo_.name == AbilityConfig::GRANT_ABILITY_ABILITY_NAME &&
        abilityInfo_.bundleName == AbilityConfig::GRANT_ABILITY_BUNDLE_NAME) {
        HILOG_INFO("%{public}s, ignore GrantAbility.", __func__);
        return;
    }

    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    auto pixelMap = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetSnapshot(missionId_);
    if (!pixelMap) {
        HILOG_WARN("%{public}s, Get snapshot failed.", __func__);
        return;
    }

    sptr<AbilityTransitionInfo> info;
    if (startOptions) {
        info = CreateAbilityTransitionInfo(token_, startOptions, want);
    } else {
        info = CreateAbilityTransitionInfo(abilityRequest, token_);
    }

    SetStartingWindow(true);
    windowHandler->StartingWindow(info, pixelMap);
}

void AbilityRecord::StartingWindowCold(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest)
{
    if (abilityInfo_.name == AbilityConfig::GRANT_ABILITY_ABILITY_NAME &&
        abilityInfo_.bundleName == AbilityConfig::GRANT_ABILITY_BUNDLE_NAME) {
        HILOG_INFO("%{public}s, ignore GrantAbility.", __func__);
        return;
    }

    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    auto resourceMgr = CreateResourceManager(abilityInfo_);
    if (!resourceMgr) {
        HILOG_WARN("%{public}s, Get resourceMgr failed.", __func__);
        return;
    }

    auto windowIconId = static_cast<uint32_t>(abilityInfo_.startWindowIconId);
    auto pixelMap = GetPixelMap(windowIconId, resourceMgr);

    uint32_t bgColor = 0;
    auto colorId = static_cast<uint32_t>(abilityInfo_.startWindowBackgroundId);
    auto colorErrval = resourceMgr->GetColorById(colorId, bgColor);
    if (colorErrval != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_WARN("%{public}s. Failed to GetColorById.", __func__);
        bgColor = 0xdfffffff;
    }
    HILOG_DEBUG("%{public}s colorId is %{public}u, bgColor is %{public}u.", __func__, colorId, bgColor);

    sptr<AbilityTransitionInfo> info;
    if (startOptions) {
        info = CreateAbilityTransitionInfo(token_, startOptions, want);
    } else {
        info = CreateAbilityTransitionInfo(abilityRequest, token_);
    }

    SetStartingWindow(true);
    windowHandler->StartingWindow(info, pixelMap, bgColor);
}
#endif

void AbilityRecord::BackgroundAbility(const Closure &task)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Move the ability to background, ability:%{public}s.", abilityInfo_.name.c_str());
    if (lifecycleDeal_ == nullptr) {
        HILOG_ERROR("Move the ability to background fail, lifecycleDeal_ is null.");
        return;
    }
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        if (!want_.GetBoolParam(DEBUG_APP, false)) {
            g_abilityRecordEventId_++;
            eventId_ = g_abilityRecordEventId_;
            // eventId_ is a unique id of the task.
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::BACKGROUNDNEW_TIMEOUT);
        } else {
            HILOG_INFO("Is debug mode, no need to handle time out.");
        }
    }

    if (!IsTerminating() || IsRestarting()) {
        // schedule save ability state before moving to background.
        SaveAbilityState();
    }

    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::BACKGROUNDING;
    lifecycleDeal_->BackgroundNew(want_, lifeCycleStateInfo_);
}

int AbilityRecord::TerminateAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Schedule terminate ability to AppMs, ability:%{public}s.", abilityInfo_.name.c_str());
    return DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token_);
}

const AppExecFwk::AbilityInfo &AbilityRecord::GetAbilityInfo() const
{
    return abilityInfo_;
}

const AppExecFwk::ApplicationInfo &AbilityRecord::GetApplicationInfo() const
{
    return applicationInfo_;
}

AbilityState AbilityRecord::GetAbilityState() const
{
    return currentState_;
}

bool AbilityRecord::IsForeground() const
{
    return currentState_ == AbilityState::FOREGROUND || currentState_ == AbilityState::FOREGROUNDING;
}

void AbilityRecord::SetAbilityState(AbilityState state)
{
    currentState_ = state;
    if (state == AbilityState::FOREGROUND || state == AbilityState::ACTIVE) {
        SetRestarting(false);
    }
}

void AbilityRecord::SetScheduler(const sptr<IAbilityScheduler> &scheduler)
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER(lifecycleDeal_);
    if (scheduler != nullptr) {
        if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
            auto schedulerObject = scheduler_->AsObject();
            if (schedulerObject != nullptr) {
                schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
            }
        }
        if (schedulerDeathRecipient_ == nullptr) {
            std::weak_ptr<AbilityRecord> thisWeakPtr(shared_from_this());
            schedulerDeathRecipient_ =
                new AbilitySchedulerRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                    auto abilityRecord = thisWeakPtr.lock();
                    if (abilityRecord) {
                        abilityRecord->OnSchedulerDied(remote);
                    }
                });
        }
        isReady_ = true;
        scheduler_ = scheduler;
        lifecycleDeal_->SetScheduler(scheduler);
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->AddDeathRecipient(schedulerDeathRecipient_);
        }
    } else {
        HILOG_ERROR("scheduler is nullptr");
        isReady_ = false;
        isWindowAttached_ = false;
        SetIsNewWant(false);
        if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
            auto schedulerObject = scheduler_->AsObject();
            if (schedulerObject != nullptr) {
                schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
            }
        }
        scheduler_ = scheduler;
    }
}

sptr<Token> AbilityRecord::GetToken() const
{
    return token_;
}

void AbilityRecord::SetPreAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    preAbilityRecord_ = abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetPreAbilityRecord() const
{
    return preAbilityRecord_.lock();
}

void AbilityRecord::SetNextAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    nextAbilityRecord_ = abilityRecord;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetNextAbilityRecord() const
{
    return nextAbilityRecord_.lock();
}

void AbilityRecord::SetEventId(int64_t eventId)
{
    eventId_ = eventId;
}

int64_t AbilityRecord::GetEventId() const
{
    return eventId_;
}

bool AbilityRecord::IsReady() const
{
    return isReady_;
}

#ifdef SUPPORT_GRAPHICS
bool AbilityRecord::IsWindowAttached() const
{
    return isWindowAttached_;
}
#endif

bool AbilityRecord::IsLauncherAbility() const
{
    return isLauncherAbility_;
}

bool AbilityRecord::IsTerminating() const
{
    return isTerminating_;
}

void AbilityRecord::SetTerminatingState()
{
    isTerminating_ = true;
}

bool AbilityRecord::IsNewWant() const
{
    return lifeCycleStateInfo_.isNewWant;
}

void AbilityRecord::SetIsNewWant(bool isNewWant)
{
    lifeCycleStateInfo_.isNewWant = isNewWant;
}

bool AbilityRecord::IsCreateByConnect() const
{
    return isCreateByConnect_;
}

void AbilityRecord::SetCreateByConnectMode()
{
    isCreateByConnect_ = true;
}

void AbilityRecord::Activate()
{
    HILOG_INFO("Activate.");
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::ACTIVE_TIMEOUT_MSG, AbilityManagerService::ACTIVE_TIMEOUT);

    // schedule active after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::ACTIVATING;
    lifecycleDeal_->Activate(want_, lifeCycleStateInfo_);

    // update ability state to appMgr service when restart
    if (IsNewWant()) {
        sptr<Token> preToken = nullptr;
        if (GetPreAbilityRecord()) {
            preToken = GetPreAbilityRecord()->GetToken();
        }
        DelayedSingleton<AppScheduler>::GetInstance()->AbilityBehaviorAnalysis(token_, preToken, 1, 1, 1);
    }
}

void AbilityRecord::Inactivate()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Inactivate ability start, ability:%{public}s.", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::INACTIVE_TIMEOUT_MSG, AbilityManagerService::INACTIVE_TIMEOUT);

    // schedule inactive after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::INACTIVATING;
    lifecycleDeal_->Inactivate(want_, lifeCycleStateInfo_);
}

void AbilityRecord::Terminate(const Closure &task)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Begin to terminate ability, ability:%{public}s.", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler && task) {
        if (!want_.GetBoolParam(DEBUG_APP, false)) {
            g_abilityRecordEventId_++;
            eventId_ = g_abilityRecordEventId_;
            // eventId_ is a unique id of the task.
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::TERMINATE_TIMEOUT);
        } else {
            HILOG_INFO("Is debug mode, no need to handle time out.");
        }
    }
    // schedule background after updating AbilityState and sending timeout message to avoid ability async callback
    // earlier than above actions.
    currentState_ = AbilityState::TERMINATING;
    lifecycleDeal_->Terminate(want_, lifeCycleStateInfo_);
}

void AbilityRecord::ConnectAbility()
{
    HILOG_INFO("Connect ability.");
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->ConnectAbility(want_);
}

void AbilityRecord::DisconnectAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Disconnect ability, ability:%{public}s.", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->DisconnectAbility(want_);
}

void AbilityRecord::CommandAbility()
{
    HILOG_INFO("Command ability, startId_:%{public}d.", startId_);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->CommandAbility(want_, false, startId_);
}

void AbilityRecord::SaveAbilityState()
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->SaveAbilityState();
}

void AbilityRecord::SaveAbilityState(const PacMap &inState)
{
    HILOG_INFO("%{public}s : pacmap save", __func__);
    stateDatas_ = inState;
}

void AbilityRecord::RestoreAbilityState()
{
    HILOG_INFO("%{public}s", __func__);
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->RestoreAbilityState(stateDatas_);
    stateDatas_.Clear();
    isRestarting_ = false;
}

void AbilityRecord::SetWant(const Want &want)
{
    want_ = want;
}

const Want &AbilityRecord::GetWant() const
{
    return want_;
}

int AbilityRecord::GetRequestCode() const
{
    return requestCode_;
}

void AbilityRecord::SetResult(const std::shared_ptr<AbilityResult> &result)
{
    result_ = result;
}

std::shared_ptr<AbilityResult> AbilityRecord::GetResult() const
{
    return result_;
}

void AbilityRecord::SendResult()
{
    HILOG_INFO("Send result to the caller, ability:%{public}s.", abilityInfo_.name.c_str());
    std::lock_guard<std::mutex> guard(lock_);
    CHECK_POINTER(scheduler_);
    CHECK_POINTER(result_);
    scheduler_->SendResult(result_->requestCode_, result_->resultCode_, result_->resultWant_);
    GrantUriPermission(result_->resultWant_);
    // reset result to avoid send result next time
    result_.reset();
}

void AbilityRecord::SendResultToCallers()
{
    for (auto caller : GetCallerRecordList()) {
        if (caller == nullptr) {
            HILOG_WARN("Caller record is nullptr.");
            continue;
        }
        std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
        if (callerAbilityRecord != nullptr && callerAbilityRecord->GetResult() != nullptr) {
            callerAbilityRecord->SendResult();
        } else {
            std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord = caller->GetSaCaller();
            if (callerSystemAbilityRecord != nullptr) {
                HILOG_INFO("Send result to system ability.");
                callerSystemAbilityRecord->SendResultToSystemAbility(caller->GetRequestCode(),
                    callerSystemAbilityRecord->GetResultCode(), callerSystemAbilityRecord->GetResultWant(),
                    callerSystemAbilityRecord->GetCallerToken());
            }
        }
    }
}

void AbilityRecord::SaveResultToCallers(const int resultCode, const Want *resultWant)
{
    auto callerRecordList = GetCallerRecordList();
    if (callerRecordList.empty()) {
        HILOG_WARN("callerRecordList is empty.");
        return;
    }
    auto lastestCaller = callerRecordList.back();
    for (auto caller : callerRecordList) {
        if (caller == nullptr) {
            HILOG_WARN("Caller record is nullptr.");
            continue;
        }
        if (caller == lastestCaller) {
            HILOG_INFO("Caller record is the latest.");
            SaveResult(resultCode, resultWant, caller);
            continue;
        }
        SaveResult(SEND_RESULT_CANCELED, resultWant, caller);
    }
}

void AbilityRecord::SaveResult(int resultCode, const Want *resultWant, std::shared_ptr<CallerRecord> caller)
{
    std::shared_ptr<AbilityRecord> callerAbilityRecord = caller->GetCaller();
    if (callerAbilityRecord != nullptr) {
        callerAbilityRecord->SetResult(
            std::make_shared<AbilityResult>(caller->GetRequestCode(), resultCode, *resultWant));
    } else {
        std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord = caller->GetSaCaller();
        if (callerSystemAbilityRecord != nullptr) {
            HILOG_INFO("Caller is system ability.");
            Want* newWant = const_cast<Want*>(resultWant);
            callerSystemAbilityRecord->SetResultToSystemAbility(callerSystemAbilityRecord, *newWant,
                resultCode);
        }
    }
}

void SystemAbilityCallerRecord::SetResultToSystemAbility(
    std::shared_ptr<SystemAbilityCallerRecord> callerSystemAbilityRecord,
    Want &resultWant, int resultCode)
{
    std::vector<std::string> data;
    std::string srcAbilityId = callerSystemAbilityRecord->GetSrcAbilityId();
    SplitStr(srcAbilityId, "_", data);
    if (data.size() != VECTOR_SIZE) {
        HILOG_ERROR("Check data size failed");
        return;
    }
    std::string srcDeviceId = data[0];
    HILOG_INFO("Get srcDeviceId = %{public}s", srcDeviceId.c_str());
    int missionId = atoi(data[1].c_str());
    HILOG_INFO("Get missionId = %{public}d", missionId);
    resultWant.SetParam(DMS_SRC_NETWORK_ID, srcDeviceId);
    resultWant.SetParam(DMS_MISSION_ID, missionId);
    callerSystemAbilityRecord->SetResult(resultWant, resultCode);
}

void SystemAbilityCallerRecord::SendResultToSystemAbility(int requestCode, int resultCode, Want &resultWant,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_INFO("%{public}s", __func__);
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    uint32_t accessToken = IPCSkeleton::GetCallingTokenID();
    HILOG_INFO("Try to SendResult, callerUid = %{public}d, AccessTokenId = %{public}u",
        callerUid, accessToken);
    if (callerToken == nullptr) {
        HILOG_ERROR("CallerToken is nullptr");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(SYSTEM_ABILITY_TOKEN_CALLBACK)) {
        HILOG_ERROR("SendResultToSystemAbility Write interface token failed.");
        return;
    }
    if (!data.WriteParcelable(&resultWant)) {
        HILOG_ERROR("fail to WriteParcelable");
        return;
    }
    data.WriteInt32(callerUid);
    data.WriteInt32(requestCode);
    data.WriteUint32(accessToken);
    data.WriteInt32(resultCode);
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int result = callerToken->SendRequest(ISystemAbilityTokenCallback::SEND_RESULT, data, reply, option);
    if (result != ERR_OK) {
        HILOG_ERROR("SendResultToSystemAbility error = %{public}d", result);
    }
}

void AbilityRecord::AddConnectRecordToList(const std::shared_ptr<ConnectionRecord> &connRecord)
{
    CHECK_POINTER(connRecord);
    auto it = std::find(connRecordList_.begin(), connRecordList_.end(), connRecord);
    // found it
    if (it != connRecordList_.end()) {
        HILOG_DEBUG("Found it in list, so no need to add same connection");
        return;
    }
    // no found then add new connection to list
    HILOG_DEBUG("No found in list, so add new connection to list");
    connRecordList_.push_back(connRecord);
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityRecord::GetConnectRecordList() const
{
    return connRecordList_;
}

void AbilityRecord::RemoveConnectRecordFromList(const std::shared_ptr<ConnectionRecord> &connRecord)
{
    CHECK_POINTER(connRecord);
    connRecordList_.remove(connRecord);
}

void AbilityRecord::AddCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode, std::string srcAbilityId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Add caller record.");
    if (requestCode != DEFAULT_REQUEST_CODE && IsSystemAbilityCall(callerToken)) {
        AddSystemAbilityCallerRecord(callerToken, requestCode, srcAbilityId);
        return;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    CHECK_POINTER(abilityRecord);

    auto isExist = [&abilityRecord](const std::shared_ptr<CallerRecord> &callerRecord) {
        return (callerRecord->GetCaller() == abilityRecord);
    };

    auto record = std::find_if(callerList_.begin(), callerList_.end(), isExist);
    if (record != callerList_.end()) {
        callerList_.erase(record);
    }

    callerList_.emplace_back(std::make_shared<CallerRecord>(requestCode, abilityRecord));

    lifeCycleStateInfo_.caller.requestCode = requestCode;
    lifeCycleStateInfo_.caller.deviceId = abilityRecord->GetAbilityInfo().deviceId;
    lifeCycleStateInfo_.caller.bundleName = abilityRecord->GetAbilityInfo().bundleName;
    lifeCycleStateInfo_.caller.abilityName = abilityRecord->GetAbilityInfo().name;
    HILOG_INFO("caller %{public}s, %{public}s",
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str());
}

bool AbilityRecord::IsSystemAbilityCall(const sptr<IRemoteObject> &callerToken)
{
    if (callerToken == nullptr) {
        return false;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord != nullptr) {
        return false;
    }
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(IPCSkeleton::GetCallingTokenID());
    bool isNativeCall = tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    if (!isNativeCall) {
        HILOG_INFO("Is not native call.");
        return false;
    }
    AccessToken::NativeTokenInfo nativeTokenInfo;
    int32_t result = AccessToken::AccessTokenKit::GetNativeTokenInfo(IPCSkeleton::GetCallingTokenID(),
        nativeTokenInfo);
    if (result == ERR_OK && nativeTokenInfo.processName == DMS_PROCESS_NAME) {
        HILOG_INFO("Is system ability call.");
        return true;
    }
    return false;
}

void AbilityRecord::AddSystemAbilityCallerRecord(const sptr<IRemoteObject> &callerToken, int requestCode,
    std::string srcAbilityId)
{
    HILOG_INFO("Add system ability caller record.");
    std::shared_ptr<SystemAbilityCallerRecord> systemAbilityRecord =
        std::make_shared<SystemAbilityCallerRecord>(srcAbilityId, callerToken);
    auto isExist = [&srcAbilityId](const std::shared_ptr<CallerRecord> &callerRecord) {
        std::shared_ptr<SystemAbilityCallerRecord> saCaller = callerRecord->GetSaCaller();
        return (saCaller != nullptr && saCaller->GetSrcAbilityId() == srcAbilityId);
    };
    auto record = std::find_if(callerList_.begin(), callerList_.end(), isExist);
    if (record != callerList_.end()) {
        HILOG_INFO("Find same system ability caller record.");
        callerList_.erase(record);
    }
    callerList_.emplace_back(std::make_shared<CallerRecord>(requestCode, systemAbilityRecord));
    HILOG_INFO("Add system ability record end.");
}

std::list<std::shared_ptr<CallerRecord>> AbilityRecord::GetCallerRecordList() const
{
    return callerList_;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetCallerRecord() const
{
    if (callerList_.empty()) {
        return nullptr;
    }
    return callerList_.back()->GetCaller();
}

bool AbilityRecord::IsConnectListEmpty()
{
    return connRecordList_.empty();
}

std::shared_ptr<ConnectionRecord> AbilityRecord::GetConnectingRecord() const
{
    auto connect =
        std::find_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
            return record->GetConnectState() == ConnectionState::CONNECTING;
        });
    return (connect != connRecordList_.end()) ? *connect : nullptr;
}

std::list<std::shared_ptr<ConnectionRecord>> AbilityRecord::GetConnectingRecordList()
{
    std::list<std::shared_ptr<ConnectionRecord>> connectingList;
    for (auto record : connRecordList_) {
        if (record && record->GetConnectState() == ConnectionState::CONNECTING) {
            connectingList.push_back(record);
        }
    }
    return connectingList;
}

std::shared_ptr<ConnectionRecord> AbilityRecord::GetDisconnectingRecord() const
{
    auto connect =
        std::find_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
            return record->GetConnectState() == ConnectionState::DISCONNECTING;
        });
    return (connect != connRecordList_.end()) ? *connect : nullptr;
}

void AbilityRecord::GetAbilityTypeString(std::string &typeStr)
{
    AppExecFwk::AbilityType type = GetAbilityInfo().type;
    switch (type) {
#ifdef SUPPORT_GRAPHICS
        case AppExecFwk::AbilityType::PAGE: {
            typeStr = "PAGE";
            break;
        }
#endif
        case AppExecFwk::AbilityType::SERVICE: {
            typeStr = "SERVICE";
            break;
        }
        // for config.json type
        case AppExecFwk::AbilityType::DATA: {
            typeStr = "DATA";
            break;
        }
        default: {
            typeStr = "UNKNOWN";
            break;
        }
    }
}

std::string AbilityRecord::ConvertAbilityState(const AbilityState &state)
{
    auto it = stateToStrMap.find(state);
    if (it != stateToStrMap.end()) {
        return it->second;
    }
    return "INVALIDSTATE";
}

std::string AbilityRecord::ConvertAppState(const AppState &state)
{
    auto it = appStateToStrMap_.find(state);
    if (it != appStateToStrMap_.end()) {
        return it->second;
    }
    return "INVALIDSTATE";
}

int AbilityRecord::ConvertLifeCycleToAbilityState(const AbilityLifeCycleState &state)
{
    auto it = convertStateMap.find(state);
    if (it != convertStateMap.end()) {
        return it->second;
    }
    return DEFAULT_INVAL_VALUE;
}

void AbilityRecord::Dump(std::vector<std::string> &info)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    // get ability type(unknown/page/service/provider)
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> preAbility = GetPreAbilityRecord();
    if (preAbility == nullptr) {
        dumpInfo = "        previous ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [NULL]";
    } else {
        dumpInfo =
            "        previous ability app name [" + preAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [" + preAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> nextAbility = GetNextAbilityRecord();
    if (nextAbility == nullptr) {
        dumpInfo = "        next ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        next ability file name [NULL]";
    } else {
        dumpInfo =
            "        next ability app name [" + nextAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        next ability main name [" + nextAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);

    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }
}

void AbilityRecord::DumpAbilityState(
    std::vector<std::string> &info, bool isClient, const std::vector<std::string> &params)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);

    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);
    dumpInfo = "        callee connections: ";
    info.push_back(dumpInfo);
    if (callContainer_) {
        callContainer_->Dump(info);
    }

    if (isLauncherRoot_) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }

    // add dump client info
    DumpClientInfo(info, params, isClient, params.empty());
}

void AbilityRecord::SetStartTime()
{
    if (startTime_ == 0) {
        startTime_ = AbilityUtil::SystemTimeMillis();
    }
}

int64_t AbilityRecord::GetStartTime() const
{
    return startTime_;
}

void AbilityRecord::DumpService(std::vector<std::string> &info, bool isClient) const
{
    std::vector<std::string> params;
    DumpService(info, params, isClient);
}

void AbilityRecord::DumpService(std::vector<std::string> &info, std::vector<std::string> &params, bool isClient) const
{
    info.emplace_back("      AbilityRecord ID #" + std::to_string(GetRecordId()) + "   state #" +
                      AbilityRecord::ConvertAbilityState(GetAbilityState()) + "   start time [" +
                      std::to_string(GetStartTime()) + "]");
    info.emplace_back("      main name [" + GetAbilityInfo().name + "]");
    info.emplace_back("      bundle name [" + GetAbilityInfo().bundleName + "]");
    info.emplace_back("      ability type [SERVICE]");
    info.emplace_back("      app state #" + AbilityRecord::ConvertAppState(appState_));

    if (isLauncherRoot_) {
        info.emplace_back("      can restart num #" + std::to_string(restartCount_));
    }

    info.emplace_back("      Connections: " + std::to_string(connRecordList_.size()));
    for (auto &&conn : connRecordList_) {
        if (conn) {
            conn->Dump(info);
        }
    }
    // add dump client info
    DumpClientInfo(info, params, isClient);
}

void AbilityRecord::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    HILOG_WARN("On scheduler died.");
    auto mission = GetMission();
    if (mission) {
        HILOG_WARN("On scheduler died. Is app not response Reason:%{public}d", mission->IsANRState());
    }
    std::lock_guard<std::mutex> guard(lock_);
    CHECK_POINTER(scheduler_);

    auto object = remote.promote();
    CHECK_POINTER(object);

    if (object != scheduler_->AsObject()) {
        HILOG_ERROR("Ability on scheduler died: scheduler is not matches with remote.");
        return;
    }

    if (scheduler_ != nullptr && schedulerDeathRecipient_ != nullptr) {
        auto schedulerObject = scheduler_->AsObject();
        if (schedulerObject != nullptr) {
            schedulerObject->RemoveDeathRecipient(schedulerDeathRecipient_);
        }
    }
    scheduler_.clear();
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->SetScheduler(nullptr);
    isWindowAttached_ = false;

    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);

    auto handler = abilityManagerService->GetEventHandler();
    CHECK_POINTER(handler);

    HILOG_INFO("Ability on scheduler died: '%{public}s'", abilityInfo_.name.c_str());
    auto task = [abilityManagerService, ability = shared_from_this()]() {
        abilityManagerService->OnAbilityDied(ability);
    };
    handler->PostTask(task);
    auto uriTask = [want = want_, ability = shared_from_this()]() {
        ability->SaveResultToCallers(-1, &want);
        ability->SendResultToCallers();
    };
    handler->PostTask(uriTask);
}

void AbilityRecord::SetConnRemoteObject(const sptr<IRemoteObject> &remoteObject)
{
    connRemoteObject_ = remoteObject;
}

sptr<IRemoteObject> AbilityRecord::GetConnRemoteObject() const
{
    return connRemoteObject_;
}

void AbilityRecord::AddStartId()
{
    startId_++;
}
int AbilityRecord::GetStartId() const
{
    return startId_;
}

void AbilityRecord::SetIsUninstallAbility()
{
    isUninstall_ = true;
}

bool AbilityRecord::IsUninstallAbility() const
{
    return isUninstall_;
}

void AbilityRecord::SetLauncherRoot()
{
    isLauncherRoot_ = true;
}

bool AbilityRecord::IsLauncherRoot() const
{
    return isLauncherRoot_;
}

bool AbilityRecord::IsAbilityState(const AbilityState &state) const
{
    return (currentState_ == state);
}

bool AbilityRecord::IsActiveState() const
{
    return (IsAbilityState(AbilityState::ACTIVE) || IsAbilityState(AbilityState::ACTIVATING) ||
            IsAbilityState(AbilityState::INITIAL) || IsAbilityState(AbilityState::FOREGROUND) ||
            IsAbilityState(AbilityState::FOREGROUNDING));
}

void AbilityRecord::SendEvent(uint32_t msg, uint32_t timeOut)
{
    if (want_.GetBoolParam(DEBUG_APP, false)) {
        HILOG_INFO("Is debug mode, no need to handle time out.");
        return;
    }
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER(handler);

    g_abilityRecordEventId_++;
    eventId_ = g_abilityRecordEventId_;
    handler->SendEvent(msg, eventId_, timeOut);
}

void AbilityRecord::SetStartSetting(const std::shared_ptr<AbilityStartSetting> &setting)
{
    lifeCycleStateInfo_.setting = setting;
}

std::shared_ptr<AbilityStartSetting> AbilityRecord::GetStartSetting() const
{
    return lifeCycleStateInfo_.setting;
}

void AbilityRecord::SetRestarting(const bool isRestart)
{
    isRestarting_ = isRestart;
    HILOG_DEBUG("SetRestarting: %{public}d", isRestarting_);

    if (isLauncherRoot_ && IsLauncherAbility()) {
        restartCount_ = isRestart ? (--restartCount_) : restratMax_;
        HILOG_INFO("root launcher restart count: %{public}d", restartCount_);
    }
}

void AbilityRecord::SetRestarting(const bool isRestart, int32_t canReStartCount)
{
    isRestarting_ = isRestart;
    HILOG_DEBUG("SetRestarting: %{public}d, restart count: %{public}d", isRestarting_, canReStartCount);

    if (isLauncherRoot_ && IsLauncherAbility()) {
        restartCount_ = isRestart ? canReStartCount : restratMax_;
        HILOG_INFO("root launcher restart count: %{public}d", restartCount_);
    }
}

int32_t AbilityRecord::GetRestartCount() const
{
    return restartCount_;
}

bool AbilityRecord::IsRestarting() const
{
    return isRestarting_;
}

void AbilityRecord::SetAppState(const AppState &state)
{
    appState_ = state;
}

AppState AbilityRecord::GetAppState() const
{
    return appState_;
}

void AbilityRecord::SetLaunchReason(const LaunchReason &reason)
{
    lifeCycleStateInfo_.launchParam.launchReason = reason;
}

void AbilityRecord::SetLastExitReason(const LastExitReason &reason)
{
    lifeCycleStateInfo_.launchParam.lastExitReason = reason;
}

void AbilityRecord::NotifyContinuationResult(int32_t result)
{
    HILOG_INFO("NotifyContinuationResult.");
    CHECK_POINTER(lifecycleDeal_);

    lifecycleDeal_->NotifyContinuationResult(result);
}

std::shared_ptr<MissionList> AbilityRecord::GetOwnedMissionList() const
{
    return missionList_.lock();
}

void AbilityRecord::SetMissionList(const std::shared_ptr<MissionList> &missionList)
{
    missionList_ = missionList;
}

void AbilityRecord::SetMission(const std::shared_ptr<Mission> &mission)
{
    if (mission) {
        missionId_ = mission->GetMissionId();
        HILOG_INFO("SetMission come, missionId is %{public}d.", missionId_);
    }
    mission_ = mission;
}

void AbilityRecord::SetMinimizeReason(bool fromUser)
{
    minimizeReason_ = fromUser;
}

void AbilityRecord::SetDlp(bool isDlp)
{
    isDlp_ = isDlp;
}

bool AbilityRecord::IsDlp() const
{
    return isDlp_;
}

bool AbilityRecord::IsMinimizeFromUser() const
{
    return minimizeReason_;
}

std::shared_ptr<Mission> AbilityRecord::GetMission() const
{
    return mission_.lock();
}

int32_t AbilityRecord::GetMissionId() const
{
    return missionId_;
}

void AbilityRecord::SetSpecifiedFlag(const std::string &flag)
{
    specifiedFlag_ = flag;
}

std::string AbilityRecord::GetSpecifiedFlag() const
{
    return specifiedFlag_;
}

// new version  --start
bool AbilityRecord::IsStartedByCall() const
{
    return isStartedByCall_;
}

void AbilityRecord::SetStartedByCall(const bool isFlag)
{
    isStartedByCall_ = isFlag;
}

bool AbilityRecord::IsStartToBackground() const
{
    return isStartToBackground_;
}

void AbilityRecord::SetStartToBackground(const bool flag)
{
    isStartToBackground_ = flag;
}

bool AbilityRecord::CallRequest()
{
    HILOG_INFO("Call Request.");
    CHECK_POINTER_RETURN_BOOL(scheduler_);
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    // sync call request
    sptr<IRemoteObject> callStub = scheduler_->CallRequest();
    if (!callStub) {
        HILOG_ERROR("call request failed, callstub is nullptr.");
        return false;
    }

    // complete call request
    return callContainer_->CallRequestDone(callStub);
}

ResolveResultType AbilityRecord::Resolve(const AbilityRequest &abilityRequest)
{
    auto callback = abilityRequest.connect;
    if (abilityRequest.callType != AbilityCallType::CALL_REQUEST_TYPE || !callback) {
        HILOG_ERROR("only startd by call type can create a call record.");
        return ResolveResultType::NG_INNER_ERROR;
    }
    if (!callContainer_) {
        callContainer_ = std::make_shared<CallContainer>();
        if (!callContainer_) {
            HILOG_ERROR("mark_shared error.");
            return ResolveResultType::NG_INNER_ERROR;
        }
    }

    HILOG_DEBUG("create call record for this resolve. callerUid:%{public}d ,targetname:%{public}s",
        abilityRequest.callerUid,
        abilityRequest.abilityInfo.name.c_str());

    std::shared_ptr<CallRecord> callRecord = callContainer_->GetCallRecord(callback);
    if (!callRecord) {
        callRecord = CallRecord::CreateCallRecord(
            abilityRequest.callerUid, shared_from_this(), callback, abilityRequest.callerToken);
        if (!callRecord) {
            HILOG_ERROR("mark_shared error.");
            return ResolveResultType::NG_INNER_ERROR;
        }
    }

    callContainer_->AddCallRecord(callback, callRecord);

    if (callRecord->IsCallState(CallState::REQUESTED) && callRecord->GetCallStub()) {
        HILOG_DEBUG("this record has requested.");
        if (!callRecord->SchedulerConnectDone()) {
            HILOG_DEBUG("this callrecord has requested, but callback failed.");
            return ResolveResultType::NG_INNER_ERROR;
        }
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    callRecord->SetCallState(CallState::REQUESTING);
    return ResolveResultType::OK_NO_REMOTE_OBJ;
}

bool AbilityRecord::Release(const sptr<IAbilityConnection> & connect)
{
    HILOG_DEBUG("ability release call record by callback.");
    CHECK_POINTER_RETURN_BOOL(callContainer_);

    return callContainer_->RemoveCallRecord(connect);
}

bool AbilityRecord::IsNeedToCallRequest() const
{
    HILOG_DEBUG("ability release call record by callback.");
    if (callContainer_ == nullptr) {
        return false;
    }

    return callContainer_->IsNeedToCallRequest();
}

void AbilityRecord::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    HILOG_INFO("ContinueAbility.");
    CHECK_POINTER(lifecycleDeal_);

    lifecycleDeal_->ContinueAbility(deviceId, versionCode);
}

void AbilityRecord::SetSwitchingPause(bool state)
{
    isSwitchingPause_ = state;
}

bool AbilityRecord::IsSwitchingPause()
{
    return isSwitchingPause_;
}

void AbilityRecord::SetOwnerMissionUserId(int32_t userId)
{
    ownerMissionUserId_ = userId;
}

int32_t AbilityRecord::GetOwnerMissionUserId()
{
    return ownerMissionUserId_;
}

void AbilityRecord::DumpSys(std::vector<std::string> &info, bool isClient)
{
    std::string dumpInfo = "      AbilityRecord ID #" + std::to_string(recordId_);
    info.push_back(dumpInfo);
    dumpInfo = "        app name [" + GetAbilityInfo().applicationName + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        main name [" + GetAbilityInfo().name + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        bundle name [" + GetAbilityInfo().bundleName + "]";
    info.push_back(dumpInfo);
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> preAbility = GetPreAbilityRecord();
    if (preAbility == nullptr) {
        dumpInfo = "        previous ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [NULL]";
    } else {
        dumpInfo =
            "        previous ability app name [" + preAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        previous ability file name [" + preAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> nextAbility = GetNextAbilityRecord();
    if (nextAbility == nullptr) {
        dumpInfo = "        next ability app name [NULL]" + LINE_SEPARATOR;
        dumpInfo += "        next ability file name [NULL]";
    } else {
        dumpInfo =
            "        next ability app name [" + nextAbility->GetAbilityInfo().applicationName + "]" + LINE_SEPARATOR;
        dumpInfo += "        next ability main name [" + nextAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    dumpInfo = "        state #" + AbilityRecord::ConvertAbilityState(GetAbilityState()) + "  start time [" +
               std::to_string(startTime_) + "]";
    info.push_back(dumpInfo);
    dumpInfo = "        app state #" + AbilityRecord::ConvertAppState(appState_);
    info.push_back(dumpInfo);
    dumpInfo = "        ready #" + std::to_string(isReady_) + "  window attached #" +
               std::to_string(isWindowAttached_) + "  launcher #" + std::to_string(isLauncherAbility_);
    info.push_back(dumpInfo);

    if (isLauncherRoot_ && abilityInfo_.isStageBasedModel) {
        dumpInfo = "        can restart num #" + std::to_string(restartCount_);
        info.push_back(dumpInfo);
    }
    const std::vector<std::string> params;
    DumpClientInfo(info, params, isClient);
}

void AbilityRecord::DumpClientInfo(std::vector<std::string> &info, const std::vector<std::string> &params,
    bool isClient, bool dumpConfig) const
{
    if (!isClient || !scheduler_ || !isReady_) {
        HILOG_ERROR("something nullptr.");
        return;
    }
    std::unique_lock<std::mutex> lock(dumpLock_);
    scheduler_->DumpAbilityInfo(params, info);

    HILOG_INFO("Dump begin wait.");
    isDumpTimeout_ = false;
    std::chrono::milliseconds timeout { AbilityManagerService::DUMP_TIMEOUT };
    if (dumpCondition_.wait_for(lock, timeout) == std::cv_status::timeout) {
        isDumpTimeout_ = true;
    }
    HILOG_INFO("Dump done and begin parse.");
    if (!isDumpTimeout_) {
        std::lock_guard<std::mutex> infoLock(dumpInfoLock_);
        for (auto one : dumpInfos_) {
            info.emplace_back(one);
        }
    }

    if (!dumpConfig) {
        HILOG_INFO("not dumpConfig.");
        return;
    }
    AppExecFwk::Configuration config;
    if (DelayedSingleton<AppScheduler>::GetInstance()->GetConfiguration(config) == ERR_OK) {
        info.emplace_back("          configuration: " + config.GetName());
    }
}

void AbilityRecord::DumpAbilityInfoDone(std::vector<std::string> &infos)
{
    HILOG_INFO("DumpAbilityInfoDone begin.");
    if (isDumpTimeout_) {
        HILOG_WARN("%{public}s, dump time out.", __func__);
        return;
    }
    {
        std::lock_guard<std::mutex> infoLock(dumpInfoLock_);
        dumpInfos_.clear();
        for (auto info : infos) {
            dumpInfos_.emplace_back(info);
        }
    }
    dumpCondition_.notify_all();
}

void AbilityRecord::GrantUriPermission(const Want &want)
{
    HILOG_DEBUG("AbilityRecord::GrantUriPermission is called.");
    auto flags = want.GetFlags();
    if (flags & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) {
        HILOG_INFO("Want to grant r/w permission of the uri");
        auto targetTokenId = abilityInfo_.applicationInfo.accessTokenId;
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr) {
            abilityMgr->GrantUriPermission(want, GetCurrentAccountId(), targetTokenId);
        }
    }
}

int AbilityRecord::GetCurrentAccountId()
{
    std::vector<int32_t> osActiveAccountIds;
#ifdef OS_ACCOUNT_PART_ENABLED
    ErrCode ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        HILOG_ERROR("QueryActiveOsAccountIds failed.");
        return DEFAULT_USER_ID;
    }
#else // OS_ACCOUNT_PART_ENABLED
    osActiveAccountIds.push_back(DEFAULT_OS_ACCOUNT_ID);
    HILOG_DEBUG("AbilityRecord::GetCurrentAccountId, do not have os account part, use default id.");
#endif // OS_ACCOUNT_PART_ENABLED

    if (osActiveAccountIds.empty()) {
        HILOG_ERROR("QueryActiveOsAccountIds is empty, no accounts.");
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}

void AbilityRecord::SetWindowMode(int32_t windowMode)
{
    want_.SetParam(Want::PARAM_RESV_WINDOW_MODE, windowMode);
}

void AbilityRecord::RemoveWindowMode()
{
    want_.RemoveParam(Want::PARAM_RESV_WINDOW_MODE);
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityRecord::BlockAbility()
{
    HILOG_INFO("BlockAbility.");
    if (scheduler_) {
        HILOG_INFO("scheduler_ begain to call BlockAbility %{public}s", __func__);
        return scheduler_->BlockAbility();
    }
    return ERR_NO_INIT;
}
#endif
}  // namespace AAFwk
}  // namespace OHOS
