/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "constants.h"
#include "ability_event_handler.h"
#include "ability_manager_service.h"
#include "ability_scheduler_stub.h"
#include "ability_util.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_client.h"
#include "connection_state_manager.h"
#include "hitrace_meter.h"
#include "image_source.h"
#include "in_process_call_wrapper.h"
#include "errors.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "os_account_manager_wrapper.h"
#include "parameters.h"
#include "system_ability_token_callback.h"
#include "uri_permission_manager_client.h"
#ifdef SUPPORT_GRAPHICS
#include "image_source.h"
#include "locale_config.h"
#include "mission_info_mgr.h"
#endif
#ifdef EFFICIENCY_MANAGER_ENABLE
#include "suspend_manager_client.h"
#endif // EFFICIENCY_MANAGER_ENABLE


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
const std::string DLP_INDEX = "ohos.dlp.params.index";
const std::string DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
const std::string COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
const std::string NEED_STARTINGWINDOW = "ohos.ability.NeedStartingWindow";
const uint32_t RELEASE_STARTING_BG_TIMEOUT = 15000; // release starting window resource timeout.
int64_t AbilityRecord::abilityRecordId = 0;
int64_t AbilityRecord::g_abilityRecordEventId_ = 0;
const int32_t DEFAULT_USER_ID = 0;
const int32_t SEND_RESULT_CANCELED = -1;
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
    std::map<AbilityState, std::string>::value_type(FOREGROUND_INVALID_MODE, "FOREGROUND_INVALID_MODE"),
    std::map<AbilityState, std::string>::value_type(FOREGROUND_WINDOW_FREEZED, "FOREGROUND_WINDOW_FREEZED"),
};
const std::map<AppState, std::string> AbilityRecord::appStateToStrMap_ = {
    std::map<AppState, std::string>::value_type(AppState::BEGIN, "BEGIN"),
    std::map<AppState, std::string>::value_type(AppState::READY, "READY"),
    std::map<AppState, std::string>::value_type(AppState::FOREGROUND, "FOREGROUND"),
    std::map<AppState, std::string>::value_type(AppState::BACKGROUND, "BACKGROUND"),
    std::map<AppState, std::string>::value_type(AppState::SUSPENDED, "SUSPENDED"),
    std::map<AppState, std::string>::value_type(AppState::TERMINATED, "TERMINATED"),
    std::map<AppState, std::string>::value_type(AppState::END, "END"),
    std::map<AppState, std::string>::value_type(AppState::FOCUS, "FOCUS"),
};
const std::map<AbilityLifeCycleState, AbilityState> AbilityRecord::convertStateMap = {
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INITIAL, INITIAL),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INACTIVE, INACTIVE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_ACTIVE, ACTIVE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_FOREGROUND_NEW, FOREGROUND),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_BACKGROUND_NEW, BACKGROUND),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_FOREGROUND_FAILED, FOREGROUND_FAILED),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_INVALID_WINDOW_MODE,
        FOREGROUND_INVALID_MODE),
    std::map<AbilityLifeCycleState, AbilityState>::value_type(ABILITY_STATE_WINDOW_FREEZED,
        FOREGROUND_WINDOW_FREEZED),
};

Token::Token(std::weak_ptr<AbilityRecord> abilityRecord) : abilityRecord_(abilityRecord)
{}

Token::~Token()
{}

std::shared_ptr<AbilityRecord> Token::GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
{
    CHECK_POINTER_AND_RETURN(token, nullptr);

    std::string descriptor = Str16ToStr8(token->GetObjectDescriptor());
    if (descriptor != "ohos.aafwk.AbilityToken") {
        HILOG_ERROR("Input token is not an AbilityToken, token->GetObjectDescriptor(): %{public}s",
            descriptor.c_str());
        return nullptr;
    }

    // Double check if token is valid
    sptr<IAbilityToken> theToken = iface_cast<IAbilityToken>(token);
    if (!theToken) {
        return nullptr;
    }
    if (theToken->GetDescriptor() != u"ohos.aafwk.AbilityToken") {
        return nullptr;
    }

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
        bool isRootLauncher = (applicationInfo_.bundleName == LAUNCHER_BUNDLE_NAME);
        abilityMgr->GetMaxRestartNum(restartMax_, isRootLauncher);
        bool flag = abilityMgr->GetStartUpNewRuleFlag();
        want_.SetParam(COMPONENT_STARTUP_NEW_RULES, flag);
    }
    restartCount_ = restartMax_;
    appIndex_ = want.GetIntParam(DLP_INDEX, 0);
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
    abilityRecord->SetAppIndex(abilityRequest.want.GetIntParam(DLP_INDEX, 0));
    abilityRecord->SetCallerAccessTokenId(abilityRequest.callerAccessTokenId);
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

int32_t AbilityRecord::GetPid()
{
    return pid_;
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

    if (isRestarting_) {
        restartTime_ = AbilityUtil::SystemTimeMillis();
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
    if (isLauncherRoot_ && isRestarting_ && IsLauncherAbility() && (restartCount_ < 0)) {
        HILOG_ERROR("Root launcher restart is out of max count.");
        return false;
    }
    return true;
}

bool AbilityRecord::CanRestartResident()
{
    HILOG_DEBUG("isKeepAlive: %{public}d, isRestarting: %{public}d, restartCount: %{public}d",
        isKeepAlive_, isRestarting_, restartCount_);
    if (isKeepAlive_ && isRestarting_ && (restartCount_ < 0)) {
        int restartIntervalTime = 0;
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr) {
            abilityMgr->GetRestartIntervalTime(restartIntervalTime);
        }
        HILOG_DEBUG("restartTime: %{public}lld, now: %{public}lld, intervalTine:%{public}d",
            static_cast<unsigned long long>(restartTime_),
            static_cast<unsigned long long>(AbilityUtil::SystemTimeMillis()), restartIntervalTime);
        if ((AbilityUtil::SystemTimeMillis() - restartTime_) < restartIntervalTime) {
            HILOG_ERROR("Resident restart is out of max count");
            return false;
        }
    }
    return true;
}

void AbilityRecord::ForegroundAbility(uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Start to foreground ability, name is %{public}s.", abilityInfo_.name.c_str());
    CHECK_POINTER(lifecycleDeal_);

    SendEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, AbilityManagerService::FOREGROUND_TIMEOUT);

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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string element = GetWant().GetElement().GetURI();
    HILOG_DEBUG("ability record: %{public}s", element.c_str());

    if (isReady_) {
        if (IsAbilityState(AbilityState::FOREGROUND)) {
            HILOG_DEBUG("Activate %{public}s", element.c_str());
            ForegroundAbility(sceneFlag);
        } else {
            // background to active state
            HILOG_DEBUG("MoveToForeground, %{public}s", element.c_str());
            lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
#ifdef EFFICIENCY_MANAGER_ENABLE
            std::string bundleName = GetAbilityInfo().bundleName;
            int32_t uid = GetUid();
            SuspendManager::SuspendManagerClient::GetInstance().ThawOneApplication(
                uid, bundleName, "THAW_BY_FOREGROUND_ABILITY");
#endif // EFFICIENCY_MANAGER_ENABLE
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
        }
    } else {
        HILOG_INFO("To load ability.");
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        LoadAbility();
    }
}

std::string AbilityRecord::GetLabel()
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::string strLabel = applicationInfo_.label;

    if (abilityInfo_.resourcePath.empty()) {
        HILOG_WARN("resource path is empty.");
        return strLabel;
    }

#ifdef SUPPORT_GRAPHICS
    auto resourceMgr = CreateResourceManager();
    if (!resourceMgr) {
        return strLabel;
    }

    auto result = resourceMgr->GetStringById(applicationInfo_.labelId, strLabel);
    if (result != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_WARN("%{public}s. Failed to GetStringById.", __func__);
    }

    InitColdStartingWindowResource(resourceMgr);
#endif

    return strLabel;
}

#ifdef SUPPORT_GRAPHICS
void AbilityRecord::ProcessForegroundAbility(const std::shared_ptr<AbilityRecord> &callerAbility, uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string element = GetWant().GetElement().GetURI();
    HILOG_DEBUG("SUPPORT_GRAPHICS: ability record: %{public}s", element.c_str());

    StartingWindowHot();
    auto flag = !IsForeground();
    NotifyAnimationFromTerminatingAbility(callerAbility, flag);
    PostCancelStartingWindowHotTask();

    if (IsAbilityState(AbilityState::FOREGROUND)) {
        HILOG_DEBUG("Activate %{public}s", element.c_str());
        ForegroundAbility(sceneFlag);
    } else {
        // background to active state
        HILOG_DEBUG("MoveToForeground, %{public}s", element.c_str());
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
    }
}

void AbilityRecord::NotifyAnimationFromTerminatingAbility(const std::shared_ptr<AbilityRecord>& callerAbility,
    bool flag)
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("Get WMS handler failed.");
        return;
    }

    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    if (callerAbility) {
        auto callerAbilityInfo = callerAbility->GetAbilityInfo();
        SetAbilityTransitionInfo(callerAbilityInfo, fromInfo);
        fromInfo->abilityToken_ = callerAbility->GetToken();
    }

    if (flag) {
        fromInfo->reason_ = TransitionReason::BACK_TRANSITION;
    } else {
        fromInfo->reason_ = TransitionReason::CLOSE;
    }

    auto toInfo = CreateAbilityTransitionInfo();
    SetAbilityTransitionInfo(abilityInfo_, toInfo);

    windowHandler->NotifyWindowTransition(fromInfo, toInfo);
}

void AbilityRecord::NotifyAnimationFromTerminatingAbility() const
{
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("Get WMS handler failed.");
        return;
    }

    sptr<AbilityTransitionInfo> fromInfo = new AbilityTransitionInfo();
    SetAbilityTransitionInfo(fromInfo);
    fromInfo->reason_ = TransitionReason::CLOSE;
    windowHandler->NotifyWindowTransition(fromInfo, nullptr);
}

void AbilityRecord::SetAbilityTransitionInfo(sptr<AbilityTransitionInfo>& info) const
{
    info->abilityToken_ = token_;
    info->missionId_ = missionId_;
    info->abilityName_ = abilityInfo_.name;
    info->bundleName_ = abilityInfo_.bundleName;
    info->windowModes_ = abilityInfo_.windowModes;
    info->maxWindowRatio_ = abilityInfo_.maxWindowRatio;
    info->minWindowRatio_ = abilityInfo_.minWindowRatio;
    info->maxWindowWidth_ = abilityInfo_.maxWindowWidth;
    info->minWindowWidth_ = abilityInfo_.minWindowWidth;
    info->maxWindowHeight_ = abilityInfo_.maxWindowHeight;
    info->minWindowHeight_ = abilityInfo_.minWindowHeight;
    info->orientation_ = abilityInfo_.orientation;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo()
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    SetAbilityTransitionInfo(info);
    SetStartingWindow(true);
    return info;
}

void AbilityRecord::StartingWindowHot()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("Get WMS handler failed.");
        return;
    }

    auto pixelMap = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetSnapshot(missionId_);
    if (!pixelMap) {
        HILOG_WARN("Get snapshot failed.");
    }

    auto info = CreateAbilityTransitionInfo();
    HILOG_INFO("Notify wms to start StartingWindow.");
    windowHandler->StartingWindow(info, pixelMap);
}

void AbilityRecord::ProcessForegroundAbility(bool isRecent, const AbilityRequest &abilityRequest,
    std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<AbilityRecord> &callerAbility,
    uint32_t sceneFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string element = GetWant().GetElement().GetURI();
    HILOG_DEBUG("SUPPORT_GRAPHICS: ability record: %{public}s", element.c_str());
    GrantUriPermission(want_, GetCurrentAccountId(), applicationInfo_.accessTokenId);

    if (isReady_) {
        auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
        if (!handler) {
            HILOG_ERROR("Fail to get AbilityEventHandler.");
            return;
        }
        auto taskName = std::to_string(missionId_) + "_hot";
        handler->RemoveTask(taskName);
        StartingWindowTask(isRecent, false, abilityRequest, startOptions);
        AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
        PostCancelStartingWindowHotTask();

        if (IsAbilityState(AbilityState::FOREGROUND)) {
            HILOG_DEBUG("Activate %{public}s", element.c_str());
            ForegroundAbility(sceneFlag);
        } else {
            // background to active state
            HILOG_DEBUG("MoveToForeground, %{public}s", element.c_str());
            lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
        }
    } else {
        HILOG_INFO("SUPPORT_GRAPHICS: to load ability.");
        lifeCycleStateInfo_.sceneFlagBak = sceneFlag;
        auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
        auto needStartingWindow = abilityRequest.want.GetBoolParam(NEED_STARTINGWINDOW, true);
        if (!isSaCall || needStartingWindow) {
            StartingWindowTask(isRecent, true, abilityRequest, startOptions);
            AnimationTask(isRecent, abilityRequest, startOptions, callerAbility);
            PostCancelStartingWindowColdTask();
        }
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
    if (isRecent) {
        auto want = GetWantFromMission();
        NotifyAnimationFromRecentTask(startOptions, want);
    } else {
        if (!IsForeground()) {
            NotifyAnimationFromStartingAbility(callerAbility, abilityRequest);
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
    info->windowModes_ = abilityInfo.windowModes;
    info->orientation_ = abilityInfo.orientation;
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

    auto toInfo = CreateAbilityTransitionInfo(startOptions, want);
    toInfo->abilityToken_ = token_;
    toInfo->missionId_ = missionId_;
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

    auto toInfo = CreateAbilityTransitionInfo(abilityRequest);
    toInfo->abilityToken_ = token_;
    toInfo->missionId_ = missionId_;
    SetAbilityTransitionInfo(abilityInfo_, toInfo);

    windowHandler->NotifyWindowTransition(fromInfo, toInfo);
}

void AbilityRecord::StartingWindowTask(bool isRecent, bool isCold, const AbilityRequest &abilityRequest,
    std::shared_ptr<StartOptions> &startOptions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s was called.", __func__);
    if (isRecent) {
        auto want = GetWantFromMission();
        if (isCold) {
            StartingWindowCold(startOptions, want, abilityRequest);
        } else {
            StartingWindowHot(startOptions, want, abilityRequest);
        }
    } else {
        std::shared_ptr<Want> want = nullptr;
        if (isCold) {
            StartingWindowCold(startOptions, want, abilityRequest);
        } else {
            StartingWindowHot(startOptions, want, abilityRequest);
        }
    }
}

void AbilityRecord::PostCancelStartingWindowHotTask()
{
    if (want_.GetBoolParam(DEBUG_APP, false)) {
        HILOG_INFO("PostCancelStartingWindowHotTask was called, debug mode, just return.");
        return;
    }
    HILOG_INFO("PostCancelStartingWindowHotTask was called.");
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");

    auto windowHandler = GetWMSHandler();
    CHECK_POINTER_LOG(windowHandler, "PostCancelStartingWindowColdTask, Get WMS handler failed.");

    auto abilityRecord(shared_from_this());
    auto delayTask = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING) {
            HILOG_INFO("PostCancelStartingWindowHotTask, call windowHandler CancelStartingWindow.");
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    auto taskName = std::to_string(missionId_) + "_hot";
    handler->PostTask(delayTask, taskName, AbilityManagerService::FOREGROUND_TIMEOUT);
}

void AbilityRecord::PostCancelStartingWindowColdTask()
{
    if (want_.GetBoolParam(DEBUG_APP, false)) {
        HILOG_INFO("PostCancelStartingWindowColdTask was called, debug mode, just return.");
        return;
    }
    HILOG_INFO("PostCancelStartingWindowColdTask was called.");
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityEventHandler.");

    auto windowHandler = GetWMSHandler();
    CHECK_POINTER_LOG(windowHandler, "PostCancelStartingWindowColdTask, Get WMS handler failed.");

    auto abilityRecord(shared_from_this());
    auto delayTask = [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow() &&
            (abilityRecord->GetScheduler() == nullptr ||
            abilityRecord->GetAbilityState() != AbilityState::FOREGROUNDING)) {
            HILOG_INFO("PostCancelStartingWindowColdTask, call windowHandler CancelStartingWindow.");
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
    auto taskName = std::to_string(missionId_) + "_cold";
    handler->PostTask(delayTask, taskName, AbilityManagerService::LOAD_TIMEOUT);
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
        HILOG_INFO("%{public}s: origin window mode is %{public}d.", __func__, mode);
        info->mode_ = static_cast<uint32_t>(mode);
    }
    if (displayId != -1) {
        info->displayId_ = static_cast<uint64_t>(displayId);
    }
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    if (startOptions != nullptr) {
        info->mode_ = static_cast<uint32_t>(startOptions->GetWindowMode());
        HILOG_INFO("%{public}s: window mode is %{public}d.", __func__, info->mode_);
        info->displayId_ = static_cast<uint64_t>(startOptions->GetDisplayID());
    } else {
        SetWindowModeAndDisplayId(info, want);
    }
    return info;
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(const AbilityRequest &abilityRequest) const
{
    sptr<AbilityTransitionInfo> info = new AbilityTransitionInfo();
    auto abilityStartSetting = abilityRequest.startSetting;
    if (abilityStartSetting) {
        auto windowMode = abilityStartSetting->GetProperty(AbilityStartSetting::WINDOW_MODE_KEY);
        auto displayId = abilityStartSetting->GetProperty(AbilityStartSetting::WINDOW_DISPLAY_ID_KEY);
        try {
            info->mode_ = static_cast<uint32_t>(std::stoi(windowMode));
            info->displayId_ = static_cast<uint64_t>(std::stoi(displayId));
        } catch (...) {
            HILOG_WARN("windowMode: stoi(%{public}s) failed", windowMode.c_str());
            HILOG_WARN("displayId: stoi(%{public}s) failed", displayId.c_str());
        }
    } else {
        SetWindowModeAndDisplayId(info, std::make_shared<Want>(abilityRequest.want));
    }
    return info;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityRecord::CreateResourceManager() const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(Global::I18n::LocaleConfig::GetSystemLanguage(), status);
    resConfig->SetLocaleInfo(locale);

    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr(Global::Resource::CreateResourceManager());
    resourceMgr->UpdateResConfig(*resConfig);

    std::string loadPath;
    if (!abilityInfo_.hapPath.empty()) {
        loadPath = abilityInfo_.hapPath;
    } else {
        loadPath = abilityInfo_.resourcePath;
    }

    if (loadPath.empty()) {
        HILOG_WARN("Invalid app resource.");
        return nullptr;
    }

    if (!resourceMgr->AddResource(loadPath.c_str())) {
        HILOG_WARN("%{public}s AddResource failed.", __func__);
        return nullptr;
    }
    return resourceMgr;
}

std::shared_ptr<Media::PixelMap> AbilityRecord::GetPixelMap(const uint32_t windowIconId,
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (resourceMgr == nullptr) {
        HILOG_WARN("%{public}s resource manager does not exist.", __func__);
        return nullptr;
    }

    Media::SourceOptions opts;
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource;
    if (!abilityInfo_.hapPath.empty()) { // hap is not unzip
        std::unique_ptr<uint8_t[]> iconOut;
        size_t len;
        if (resourceMgr->GetMediaDataById(windowIconId, len, iconOut) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(iconOut.get(), len, opts, errorCode);
    } else { // already unzip hap
        std::string iconPath;
        if (resourceMgr->GetMediaById(windowIconId, iconPath) != Global::Resource::RState::SUCCESS) {
            return nullptr;
        }
        imageSource = Media::ImageSource::CreateImageSource(iconPath, opts, errorCode);
    }

    if (errorCode != 0 || imageSource == nullptr) {
        HILOG_ERROR("Failed to create icon id %{private}d err %{public}d", windowIconId, errorCode);
        return nullptr;
    }

    Media::DecodeOptions decodeOpts;
    auto pixelMapPtr = imageSource->CreatePixelMap(decodeOpts, errorCode);
    if (errorCode != 0) {
        HILOG_ERROR("Failed to create PixelMap id %{private}d err %{public}d", windowIconId, errorCode);
        return nullptr;
    }
    HILOG_DEBUG("%{public}s OUT.", __func__);
    return std::shared_ptr<Media::PixelMap>(pixelMapPtr.release());
}

sptr<AbilityTransitionInfo> AbilityRecord::CreateAbilityTransitionInfo(
    const std::shared_ptr<StartOptions> &startOptions, const std::shared_ptr<Want> &want,
    const AbilityRequest &abilityRequest)
{
    sptr<AbilityTransitionInfo> info;
    if (startOptions) {
        info = CreateAbilityTransitionInfo(startOptions, want);
    } else {
        info = CreateAbilityTransitionInfo(abilityRequest);
    }

    SetAbilityTransitionInfo(info);
    SetStartingWindow(true);
    return info;
}

void AbilityRecord::StartingWindowHot(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s was called.", __func__);
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    auto pixelMap = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetSnapshot(missionId_);
    if (!pixelMap) {
        HILOG_WARN("%{public}s, Get snapshot failed.", __func__);
    }

    auto info = CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
    windowHandler->StartingWindow(info, pixelMap);
}

void AbilityRecord::StartingWindowCold(const std::shared_ptr<StartOptions> &startOptions,
    const std::shared_ptr<Want> &want, const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s was called.", __func__);
    auto windowHandler = GetWMSHandler();
    if (!windowHandler) {
        HILOG_WARN("%{public}s, Get WMS handler failed.", __func__);
        return;
    }

    // get bg pixelmap and color.
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    uint32_t bgColor = 0;
    GetColdStartingWindowResource(pixelMap, bgColor);

    // start window
    auto info = CreateAbilityTransitionInfo(startOptions, want, abilityRequest);
    windowHandler->StartingWindow(info, pixelMap, bgColor);
    startingWindowBg_.reset();
}

void AbilityRecord::GetColdStartingWindowResource(std::shared_ptr<Media::PixelMap> &bg, uint32_t &bgColor)
{
    bg = startingWindowBg_;
    bgColor = bgColor_;
    if (bg) {
        return;
    }

    auto resourceMgr = CreateResourceManager();
    if (!resourceMgr) {
        HILOG_WARN("%{public}s, Get resourceMgr failed.", __func__);
        return;
    }

    auto windowIconId = static_cast<uint32_t>(abilityInfo_.startWindowIconId);
    bg = GetPixelMap(windowIconId, resourceMgr);

    auto colorId = static_cast<uint32_t>(abilityInfo_.startWindowBackgroundId);
    auto colorErrval = resourceMgr->GetColorById(colorId, bgColor);
    if (colorErrval != OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_WARN("%{public}s. Failed to GetColorById.", __func__);
        bgColor = 0xdfffffff;
    }
    HILOG_DEBUG("%{public}s colorId is %{public}u, bgColor is %{public}u.", __func__, colorId, bgColor);
}

void AbilityRecord::InitColdStartingWindowResource(
    const std::shared_ptr<Global::Resource::ResourceManager> &resourceMgr)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!resourceMgr) {
        HILOG_ERROR("invalid resourceManager.");
        return;
    }

    startingWindowBg_ = GetPixelMap(static_cast<uint32_t>(abilityInfo_.startWindowIconId), resourceMgr);
    if (resourceMgr->GetColorById(static_cast<uint32_t>(abilityInfo_.startWindowBackgroundId), bgColor_) !=
        OHOS::Global::Resource::RState::SUCCESS) {
        HILOG_WARN("%{public}s. Failed to GetColorById.", __func__);
        bgColor_ = 0xdfffffff;
    }

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (startingWindowBg_ && handler) {
        auto delayTask = [me = weak_from_this()] {
            auto self = me.lock();
            if (!self || !self->startingWindowBg_) {
                return;
            }
            self->startingWindowBg_.reset();
        };
        handler->PostTask(delayTask, "release_bg", RELEASE_STARTING_BG_TIMEOUT);
    }
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
            handler->PostTask(task, std::to_string(eventId_), AbilityManagerService::BACKGROUND_TIMEOUT);
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
    HandleDlpClosed();
    AAFwk::EventInfo eventInfo;
    eventInfo.bundleName = GetAbilityInfo().bundleName;
    eventInfo.abilityName = GetAbilityInfo().name;
    AAFwk::EventReport::SendAbilityEvent(AAFwk::EventName::TERMINATE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
    eventInfo.errCode = DelayedSingleton<AppScheduler>::GetInstance()->TerminateAbility(token_, clearMissionFlag_);
    if (eventInfo.errCode != ERR_OK) {
        AAFwk::EventReport::SendAbilityEvent(
            AAFwk::EventName::TERMINATE_ABILITY_ERROR, HiSysEventType::FAULT, eventInfo);
    }
    return eventInfo.errCode;
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
    if (state == AbilityState::FOREGROUND || state == AbilityState::ACTIVE || state == AbilityState::BACKGROUND) {
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
        pid_ = static_cast<int32_t>(IPCSkeleton::GetCallingPid()); // set pid when ability attach to service.
        HandleDlpAttached();
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
        pid_ = 0;
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
    GrantUriPermission(result_->resultWant_, GetCurrentAccountId(), applicationInfo_.accessTokenId);
    scheduler_->SendResult(result_->requestCode_, result_->resultCode_, result_->resultWant_);
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
    auto latestCaller = callerRecordList.back();
    for (auto caller : callerRecordList) {
        if (caller == nullptr) {
            HILOG_WARN("Caller record is nullptr.");
            continue;
        }
        if (caller == latestCaller) {
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
    HILOG_DEBUG("Get srcDeviceId = %{public}s", srcDeviceId.c_str());
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
    if (!srcAbilityId.empty() && IsSystemAbilityCall(callerToken)) {
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
    std::string isKeepAlive = isKeepAlive_ ? "true" : "false";
    dumpInfo = "        isKeepAlive: " + isKeepAlive;
    info.push_back(dumpInfo);
    // get ability type(unknown/page/service/provider)
    std::string typeStr;
    GetAbilityTypeString(typeStr);
    dumpInfo = "        ability type [" + typeStr + "]";
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> preAbility = GetPreAbilityRecord();
    if (preAbility == nullptr) {
        dumpInfo = "        previous ability app name [NULL]";
        dumpInfo.append("\n");
        dumpInfo += "        previous ability file name [NULL]";
    } else {
        dumpInfo =
            "        previous ability app name [" + preAbility->GetAbilityInfo().applicationName + "]";
        dumpInfo.append("\n");
        dumpInfo += "        previous ability file name [" + preAbility->GetAbilityInfo().name + "]";
    }
    info.push_back(dumpInfo);
    std::shared_ptr<AbilityRecord> nextAbility = GetNextAbilityRecord();
    if (nextAbility == nullptr) {
        dumpInfo = "        next ability app name [NULL]";
        dumpInfo.append("\n");
        dumpInfo += "        next ability file name [NULL]";
    } else {
        dumpInfo =
            "        next ability app name [" + nextAbility->GetAbilityInfo().applicationName + "]";
        dumpInfo.append("\n");
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
    
    std::string isKeepAlive = isKeepAlive_ ? "true" : "false";
    dumpInfo = "        isKeepAlive: " + isKeepAlive;
    info.push_back(dumpInfo);
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
    
    std::string isKeepAlive = isKeepAlive_ ? "true" : "false";
    info.emplace_back("        isKeepAlive: " + isKeepAlive);
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

    RemoveUriPermission();
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
    HandleDlpClosed();
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
    if ((isLauncherRoot_ && IsLauncherAbility()) || isKeepAlive_) {
        restartCount_ = isRestart ? (--restartCount_) : restartMax_;
        HILOG_INFO("root launcher or resident process's restart count: %{public}d", restartCount_);
    }
}

void AbilityRecord::SetRestarting(const bool isRestart, int32_t canRestartCount)
{
    isRestarting_ = isRestart;
    HILOG_DEBUG("SetRestarting: %{public}d, restart count: %{public}d", isRestarting_, canRestartCount);

    if ((isLauncherRoot_ && IsLauncherAbility()) || isKeepAlive_) {
        restartCount_ = isRestart ? canRestartCount : restartMax_;
        HILOG_INFO("root launcher or resident process's restart count: %{public}d", restartCount_);
    }
}

int32_t AbilityRecord::GetRestartCount() const
{
    return restartCount_;
}

void AbilityRecord::SetRestartCount(int32_t restartCount)
{
    restartCount_ = restartCount;
}

bool AbilityRecord::IsRestarting() const
{
    return isRestarting_;
}

void AbilityRecord::SetKeepAlive()
{
    isKeepAlive_ = true;
}

int64_t AbilityRecord::GetRestartTime()
{
    return restartTime_;
}

void AbilityRecord::SetRestartTime(const int64_t restartTime)
{
    restartTime_ = restartTime;
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

void AbilityRecord::SetAppIndex(const int32_t appIndex)
{
    appIndex_ = appIndex;
}

int32_t AbilityRecord::GetAppIndex() const
{
    return appIndex_;
}

bool AbilityRecord::IsMinimizeFromUser() const
{
    return minimizeReason_;
}

void AbilityRecord::SetClearMissionFlag(bool clearMissionFlag)
{
    clearMissionFlag_= clearMissionFlag;
}

bool AbilityRecord::IsClearMissionFlag()
{
    return clearMissionFlag_;
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

bool AbilityRecord::IsStartToForeground() const
{
    return isStartToForeground_;
}

void AbilityRecord::SetStartToForeground(const bool flag)
{
    isStartToForeground_ = flag;
}

void AbilityRecord::CallRequest()
{
    HILOG_INFO("Call Request.");
    CHECK_POINTER(scheduler_);

    GrantUriPermission(want_, GetCurrentAccountId(), applicationInfo_.accessTokenId);
    // Async call request
    scheduler_->CallRequest();
}

bool AbilityRecord::CallRequestDone(const sptr<IRemoteObject> &callStub) const
{
    CHECK_POINTER_RETURN_BOOL(callContainer_);
    if (!callContainer_->CallRequestDone(callStub)) {
        HILOG_ERROR("Call request failed.");
        return false;
    }
    return true;
}

ResolveResultType AbilityRecord::Resolve(const AbilityRequest &abilityRequest)
{
    auto callback = abilityRequest.connect;
    if (abilityRequest.callType != AbilityCallType::CALL_REQUEST_TYPE || !callback) {
        HILOG_ERROR("only start by call type can create a call record.");
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

bool AbilityRecord::ReleaseCall(const sptr<IAbilityConnection>& connect)
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

void AbilityRecord::GrantUriPermission(const Want &want, int32_t userId, uint32_t targetTokenId)
{
    if ((want.GetFlags() & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("Do not call uriPermissionMgr.");
        return;
    }

    auto bms = AbilityUtil::GetBundleManager();
    CHECK_POINTER_IS_NULLPTR(bms);
    auto&& uriVec = want.GetStringArrayParam(AbilityConfig::PARAMS_STREAM);
    HILOG_DEBUG("GrantUriPermission uriVec size: %{public}zu", uriVec.size());
    auto upmClient = AAFwk::UriPermissionManagerClient::GetInstance();
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    auto fromTokenId = IPCSkeleton::GetCallingTokenID();
    for (auto&& str : uriVec) {
        Uri uri(str);
        auto&& scheme = uri.GetScheme();
        HILOG_INFO("uri scheme is %{public}s.", scheme.c_str());
        // only support file scheme
        if (scheme != "file") {
            HILOG_WARN("only support file uri.");
            continue;
        }
        auto&& authority = uri.GetAuthority();
        HILOG_INFO("uri authority is %{public}s.", authority.c_str());
        AppExecFwk::BundleInfo uriBundleInfo;
        if (!IN_PROCESS_CALL(bms->GetBundleInfo(authority, bundleFlag, uriBundleInfo, userId))) {
            HILOG_WARN("To fail to get bundle info according to uri.");
            continue;
        }
        if (uriBundleInfo.applicationInfo.accessTokenId != fromTokenId &&
            uriBundleInfo.applicationInfo.accessTokenId != callerAccessTokenId_) {
            HILOG_ERROR("the uri does not belong to caller.");
            continue;
        }
        auto ret = IN_PROCESS_CALL(upmClient->GrantUriPermission(uri, want.GetFlags(),
            callerAccessTokenId_, targetTokenId));
        if (ret) {
            isGrantedUriPermission_ = true;
        }
    }
}

void AbilityRecord::RemoveUriPermission()
{
    if (isGrantedUriPermission_) {
        HILOG_DEBUG("To remove uri permission.");
        auto upmClient = AAFwk::UriPermissionManagerClient::GetInstance();
        upmClient->RemoveUriPermission(applicationInfo_.accessTokenId);
        isGrantedUriPermission_ = false;
    }
}

void AbilityRecord::HandleDlpAttached()
{
    if (abilityInfo_.bundleName == DLP_BUNDLE_NAME) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->AddDlpManager(shared_from_this());
    }

    if (appIndex_ > 0) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->AddDlpAbility(shared_from_this());
    }
}

void AbilityRecord::HandleDlpClosed()
{
    if (abilityInfo_.bundleName == DLP_BUNDLE_NAME) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->RemoveDlpManager(shared_from_this());
    }

    if (appIndex_ > 0) {
        DelayedSingleton<ConnectionStateManager>::GetInstance()->RemoveDlpAbility(shared_from_this());
    }
}

int32_t AbilityRecord::GetCurrentAccountId() const
{
    std::vector<int32_t> osActiveAccountIds;
    ErrCode ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
            QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        HILOG_ERROR("QueryActiveOsAccountIds failed.");
        return DEFAULT_USER_ID;
    }
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

void AbilityRecord::SetPendingState(AbilityState state)
{
    pendingState_.store(state);
}

AbilityState AbilityRecord::GetPendingState() const
{
    return pendingState_.load();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AbilityRecord::BlockAbility()
{
    HILOG_INFO("BlockAbility.");
    if (scheduler_) {
        HILOG_INFO("scheduler_ begin to call BlockAbility %{public}s", __func__);
        return scheduler_->BlockAbility();
    }
    return ERR_NO_INIT;
}
#endif

bool AbilityRecord::IsNeedBackToOtherMissionStack()
{
    return isNeedBackToOtherMissionStack_;
}

void AbilityRecord::SetNeedBackToOtherMissionStack(bool isNeedBackToOtherMissionStack)
{
    isNeedBackToOtherMissionStack_ = isNeedBackToOtherMissionStack;
}

std::shared_ptr<AbilityRecord> AbilityRecord::GetOtherMissionStackAbilityRecord() const
{
    return otherMissionStackAbilityRecord_.lock();
}

void AbilityRecord::SetOtherMissionStackAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    otherMissionStackAbilityRecord_ = abilityRecord;
}

void AbilityRecord::UpdateRecoveryInfo(bool hasRecoverInfo){
    if (hasRecoverInfo == true) {
        want_.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
