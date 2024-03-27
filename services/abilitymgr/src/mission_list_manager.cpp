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

#include "mission_list_manager.h"

#include <cstdlib>

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_exit_reason_data_manager.h"
#include "appfreeze_manager.h"
#include "hitrace_meter.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "mission_info_mgr.h"
#include "in_process_call_wrapper.h"
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
#include "res_sched_client.h"
#include "res_type.h"
#endif // RESOURCE_SCHEDULE_SERVICE_ENABLE

namespace OHOS {
using AbilityRuntime::FreezeUtil;
namespace AAFwk {
namespace {
constexpr uint32_t DELAY_NOTIFY_LABEL_TIME = 30; // 30ms
constexpr uint32_t SCENE_FLAG_KEYGUARD = 1;
constexpr uint32_t ONLY_ONE_ABILITY = 1;
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr int32_t SINGLE_MAX_INSTANCE_COUNT = 128;
constexpr int32_t MAX_INSTANCE_COUNT = 512;
constexpr uint64_t NANO_SECOND_PER_SEC = 1000000000; // ns
const std::string DMS_SRC_NETWORK_ID = "dmsSrcNetworkId";
const std::string DMS_MISSION_ID = "dmsMissionId";
const int DEFAULT_DMS_MISSION_ID = -1;
const std::string DLP_INDEX = "ohos.dlp.params.index";
#ifdef SUPPORT_ASAN
const int KILL_TIMEOUT_MULTIPLE = 45;
#else
const int KILL_TIMEOUT_MULTIPLE = 3;
#endif
constexpr int32_t PREPARE_TERMINATE_ENABLE_SIZE = 6;
const char* PREPARE_TERMINATE_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
const int32_t PREPARE_TERMINATE_TIMEOUT_MULTIPLE = 10;
constexpr int32_t TRACE_ATOMIC_SERVICE_ID = 201;
const std::string TRACE_ATOMIC_SERVICE = "StartAtomicService";
const int GET_TARGET_MISSION_OVER = 200;
std::string GetCurrentTime()
{
    struct timespec tn;
    clock_gettime(CLOCK_REALTIME, &tn);
    uint64_t uTime = static_cast<uint64_t>(tn.tv_sec) * NANO_SECOND_PER_SEC +
        static_cast<uint64_t>(tn.tv_nsec);
    return std::to_string(uTime);
}
const std::unordered_map<uint32_t, FreezeUtil::TimeoutState> stateMap = {
    { AbilityManagerService::LOAD_TIMEOUT_MSG, FreezeUtil::TimeoutState::LOAD },
    { AbilityManagerService::FOREGROUND_TIMEOUT_MSG, FreezeUtil::TimeoutState::FOREGROUND },
    { AbilityManagerService::BACKGROUND_TIMEOUT_MSG, FreezeUtil::TimeoutState::BACKGROUND }
};

auto g_deleteLifecycleEventTask = [](const sptr<Token> &token, FreezeUtil::TimeoutState state) {
    CHECK_POINTER_LOG(token, "token is nullptr.");
    FreezeUtil::LifecycleFlow flow = { token->AsObject(), state };
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);
};
} // namespace

MissionListManager::MissionListManager(int userId) : userId_(userId) {}

MissionListManager::~MissionListManager() {}

void MissionListManager::Init()
{
    launcherList_ = std::make_shared<MissionList>(MissionListType::LAUNCHER);
    defaultStandardList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_STANDARD);
    defaultSingleList_ = std::make_shared<MissionList>(MissionListType::DEFAULT_SINGLE);
    currentMissionLists_.push_front(launcherList_);

    if (!listenerController_) {
        listenerController_ = std::make_shared<MissionListenerController>();
        listenerController_->Init();
    }
#ifdef SUPPORT_GRAPHICS
    InitPrepareTerminateConfig();
#endif
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Init(userId_);
}

std::shared_ptr<Mission> MissionListManager::FindEarliestMission() const
{
    // find the earliest mission of background abilityRecord
    std::shared_ptr<Mission> earliestMission;
    for (const auto& missionList : currentMissionLists_) {
        if (!missionList) {
            continue;
        }
        missionList->FindEarliestMission(earliestMission);
    }
    if (defaultStandardList_) {
        defaultStandardList_->FindEarliestMission(earliestMission);
    }
    if (defaultSingleList_) {
        defaultSingleList_->FindEarliestMission(earliestMission);
    }
    return earliestMission;
}

int32_t MissionListManager::GetMissionCount() const
{
    int32_t missionCount = 0;
    for (const auto& missionList : currentMissionLists_) {
        if (!missionList) {
            continue;
        }
        missionCount += missionList->GetMissionCount();
    }
    if (defaultStandardList_) {
        missionCount += defaultStandardList_->GetMissionCount();
    }
    if (defaultSingleList_) {
        missionCount += defaultSingleList_->GetMissionCount();
    }
    return missionCount;
}

int MissionListManager::StartAbility(AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(managerLock_);
    bool isReachToSingleLimit = CheckSingleLimit(abilityRequest);
    if (isReachToSingleLimit) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "already reach single limit instance. limit is : %{public}d",
            SINGLE_MAX_INSTANCE_COUNT);
        return ERR_REACH_UPPER_LIMIT;
    }

    auto currentTopAbility = GetCurrentTopAbilityLocked();
    if (currentTopAbility && !currentTopAbility->GetRestartAppFlag()) {
        std::string element = currentTopAbility->GetElementName().GetURI();
        auto state = currentTopAbility->GetAbilityState();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "current top: %{public}s, state: %{public}s",
            element.c_str(), AbilityRecord::ConvertAbilityState(state).c_str());
        if (state == FOREGROUNDING) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Top ability:%{public}s is foregrounding, so enqueue ability for waiting.",
                element.c_str());
            EnqueueWaitingAbility(abilityRequest);
            return START_ABILITY_WAITING;
        }
    }

    auto callerAbility = GetAbilityRecordByTokenInner(abilityRequest.callerToken);
    if (callerAbility) {
        std::string element = callerAbility->GetElementName().GetURI();
        auto state = callerAbility->GetAbilityState();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "callerAbility is: %{public}s, state: %{public}s",
            element.c_str(), AbilityRecord::ConvertAbilityState(state).c_str());
    }

    abilityRequest.callerAccessTokenId = IPCSkeleton::GetCallingTokenID();
    int ret = StartAbility(currentTopAbility, callerAbility, abilityRequest);
    if (ret == 0 && !abilityRequest.abilityInfo.visible) {
        SendKeyEvent(abilityRequest);
    }
    NotifyStartAbilityResult(abilityRequest, ret);
    ReportAbilitAssociatedStartInfoToRSS(abilityRequest.abilityInfo, static_cast<int64_t>(
        ResourceSchedule::ResType::AssociatedStartType::MISSION_LIST_START_ABILITY), callerAbility);
    return ret;
}

int MissionListManager::StartAbility(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    auto isSpecified = (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED);
    if (isSpecified) {
        EnqueueWaitingAbilityToFront(abilityRequest);
        DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedAbility(
            abilityRequest.want, abilityRequest.abilityInfo);
        return 0;
    }

    return StartAbilityLocked(currentTopAbility, callerAbility, abilityRequest);
}

int MissionListManager::MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "fromUser:%{public}d.", fromUser);
    std::lock_guard guard(managerLock_);
    // check if ability is in list to avoid user create fake token.
    CHECK_POINTER_AND_RETURN_LOG(GetAbilityRecordByTokenInner(token), INNER_ERR,
        "Minimize ability fail, ability is not in mission list.");
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    return MinimizeAbilityLocked(abilityRecord, fromUser);
}

int MissionListManager::RegisterMissionListener(const sptr<IMissionListener> &listener)
{
    if (!listenerController_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "service not init, try again later.");
        return -1;
    }

    return listenerController_->AddMissionListener(listener);
}

int MissionListManager::UnRegisterMissionListener(const sptr<IMissionListener> &listener)
{
    if (!listenerController_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "service not init, try unregister again later.");
        return -1;
    }

    listenerController_->DelMissionListener(listener);
    return 0;
}

int MissionListManager::GetMissionInfos(int32_t numMax, std::vector<MissionInfo> &missionInfos)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get mission infos.");
    if (numMax < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "numMax is invalid, numMax:%{public}d", numMax);
        return ERR_INVALID_VALUE;
    }

    return DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionInfos(numMax, missionInfos);
}

int MissionListManager::GetMissionInfo(int32_t missionId, MissionInfo &missionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Get mission info by id:%{public}d.", missionId);
    return DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionInfoById(missionId, missionInfo);
}

int MissionListManager::MoveMissionToFront(int32_t missionId, std::shared_ptr<StartOptions> startOptions)
{
    std::lock_guard guard(managerLock_);
    return MoveMissionToFrontInner(missionId, true, true, nullptr, startOptions);
}

int MissionListManager::MoveMissionToFront(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
    std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions)
{
    std::lock_guard guard(managerLock_);
    return MoveMissionToFrontInner(missionId, isCallerFromLauncher, isRecent, callerAbility, startOptions);
}

int MissionListManager::MoveMissionToFrontInner(int32_t missionId, bool isCallerFromLauncher, bool isRecent,
    std::shared_ptr<AbilityRecord> callerAbility, std::shared_ptr<StartOptions> startOptions)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "missionId:%{public}d.", missionId);
    std::shared_ptr<Mission> mission;
    bool isReachToLimit = false;
    auto targetMissionList = GetTargetMissionList(missionId, mission, isReachToLimit);
    if (isReachToLimit) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get target mission list failed, already reach to limit.");
        return ERR_REACH_UPPER_LIMIT;
    }
    if (!targetMissionList || !mission) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get target mission list failed, missionId: %{public}d", missionId);
        return MOVE_MISSION_FAILED;
    }

    MoveMissionToTargetList(isCallerFromLauncher, targetMissionList, mission);
    MoveMissionListToTop(targetMissionList);

    auto targetAbilityRecord = mission->GetAbilityRecord();
    if (!targetAbilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get target ability record failed, missionId: %{public}d", missionId);
        return MOVE_MISSION_FAILED;
    }
    targetAbilityRecord->SetIsNewWant(false);
    targetAbilityRecord->RemoveWindowMode();
    if (startOptions != nullptr) {
        targetAbilityRecord->SetWindowMode(startOptions->GetWindowMode());
    }

#ifdef SUPPORT_GRAPHICS
    AbilityRequest abilityRequest;
    targetAbilityRecord->ProcessForegroundAbility(isRecent, abilityRequest, startOptions, callerAbility);
#else
    targetAbilityRecord->ProcessForegroundAbility(0);
#endif
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SetMovingState, missionId: %{public}d", missionId);
    mission->SetMovingState(true);
    return ERR_OK;
}

void MissionListManager::EnqueueWaitingAbility(const AbilityRequest &abilityRequest)
{
    waitingAbilityQueue_.push(abilityRequest);
    return;
}

void MissionListManager::EnqueueWaitingAbilityToFront(const AbilityRequest &abilityRequest)
{
    std::queue<AbilityRequest> abilityQueue;
    abilityQueue.push(abilityRequest);
    waitingAbilityQueue_.swap(abilityQueue);
    while (!abilityQueue.empty()) {
        AbilityRequest tempAbilityRequest = abilityQueue.front();
        abilityQueue.pop();
        waitingAbilityQueue_.push(tempAbilityRequest);
    }
}

void MissionListManager::StartWaitingAbility()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    std::lock_guard guard(managerLock_);
    auto topAbility = GetCurrentTopAbilityLocked();
    if (topAbility != nullptr && topAbility->IsAbilityState(FOREGROUNDING)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Top ability is foregrounding, must return for start waiting again.");
        return;
    }

    if (!waitingAbilityQueue_.empty()) {
        AbilityRequest abilityRequest = waitingAbilityQueue_.front();
        TAG_LOGI(AAFwkTag::ABILITYMGR, "name:%{public}s", abilityRequest.abilityInfo.name.c_str());
        waitingAbilityQueue_.pop();
        auto callerAbility = GetAbilityRecordByTokenInner(abilityRequest.callerToken);
        if (StartAbility(topAbility, callerAbility, abilityRequest) == 0 && !abilityRequest.abilityInfo.visible) {
            SendKeyEvent(abilityRequest);
        }
        return;
    }
}

void MissionListManager::AddRecord(const AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetAbilityRecord)
{
    std::string srcAbilityId = "";
    if (abilityRequest.want.GetBoolParam(Want::PARAM_RESV_FOR_RESULT, false)) {
        std::string srcDeviceId = abilityRequest.want.GetStringParam(DMS_SRC_NETWORK_ID);
        int missionId = abilityRequest.want.GetIntParam(DMS_MISSION_ID, DEFAULT_DMS_MISSION_ID);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get srcNetWorkId = %s, missionId = %d", srcDeviceId.c_str(), missionId);
        Want* newWant = const_cast<Want*>(&abilityRequest.want);
        newWant->RemoveParam(DMS_SRC_NETWORK_ID);
        newWant->RemoveParam(DMS_MISSION_ID);
        newWant->RemoveParam(Want::PARAM_RESV_FOR_RESULT);
        srcAbilityId = srcDeviceId + "_" + std::to_string(missionId);
    }
    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode, srcAbilityId);
}

int MissionListManager::GetTargetMission(const AbilityRequest &abilityRequest, std::shared_ptr<Mission> &targetMission,
    std::shared_ptr<AbilityRecord> &targetAbilityRecord)
{
    bool isReachToLimit = false;
    GetTargetMissionAndAbility(abilityRequest, targetMission, targetAbilityRecord, isReachToLimit);
    if (isReachToLimit) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get mission and ability, already reach to limit.");
        return ERR_REACH_UPPER_LIMIT;
    }
    if (!targetMission || !targetAbilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get mission or record.");
        return ERR_INVALID_VALUE;
    }

    if (targetAbilityRecord->IsTerminating()) {
        TAG_LOGE(
            AAFwkTag::ABILITYMGR, "%{public}s is terminating.", targetAbilityRecord->GetAbilityInfo().name.c_str());
        return ERR_INVALID_VALUE;
    }

    if (targetAbilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is FOREGROUND.");
        targetAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        return ERR_OK;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is not FOREGROUND.");
        targetAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
        if (targetAbilityRecord->IsLoading()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ability: %{public}s is loading.", abilityRequest.abilityInfo.name.c_str());
            return ERR_OK;
        }
    }

    UpdateAbilityRecordLaunchReason(abilityRequest, targetAbilityRecord);
    AddRecord(abilityRequest, targetAbilityRecord);

    return GET_TARGET_MISSION_OVER;
}

int MissionListManager::StartAbilityLocked(const std::shared_ptr<AbilityRecord> &currentTopAbility,
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    std::string connector = "##";
    auto element = abilityRequest.want.GetElement();
    std::string traceName = __PRETTY_FUNCTION__ + connector + element.GetBundleName() + connector +
        element.GetAbilityName();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, traceName);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Start ability locked.");
    // 1. choose target mission list
    auto targetList = GetTargetMissionList(callerAbility, abilityRequest);
    CHECK_POINTER_AND_RETURN(targetList, ERR_INVALID_CALLER);

    // 2. get target mission
    std::shared_ptr<Mission> targetMission;
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    int ret = GetTargetMission(abilityRequest, targetMission, targetAbilityRecord);
    if (ret != GET_TARGET_MISSION_OVER) {
        return ret;
    }
    
    // 3. move mission to target list
    bool isCallerFromLauncher = (callerAbility && callerAbility->IsLauncherAbility());
    MoveMissionToTargetList(isCallerFromLauncher, targetList, targetMission);

    // 4. move target list to top
    MoveMissionListToTop(targetList);

    // 5. schedule target ability
    if (!currentTopAbility) {
        // top ability is null, then launch the first Ability.
        if (targetAbilityRecord->GetAbilityInfo().applicationInfo.isLauncherApp) {
            targetAbilityRecord->SetLauncherRoot();
        }
    } else {
        // only SA or no Page Ability support back to other mission stack
        auto supportBackToOtherMissionStack =
            (!callerAbility) || (callerAbility->GetAbilityInfo().type != AppExecFwk::AbilityType::PAGE);
        auto needBackToOtherMissionStack =
            abilityRequest.want.GetBoolParam(Want::PARAM_BACK_TO_OTHER_MISSION_STACK, false);
        if (supportBackToOtherMissionStack && needBackToOtherMissionStack) {
            // mark if need back to other mission stack
            targetAbilityRecord->SetNeedBackToOtherMissionStack(true);
            auto focusAbility = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->GetFocusAbility();
            if (focusAbility && (GetMissionIdByAbilityTokenInner(focusAbility->GetToken()) != -1)) {
                targetAbilityRecord->SetOtherMissionStackAbilityRecord(focusAbility);
            } else {
                targetAbilityRecord->SetOtherMissionStackAbilityRecord(currentTopAbility);
            }
        }
    }

    NotifyAbilityToken(targetAbilityRecord->GetToken(), abilityRequest);

    targetAbilityRecord->SetAbilityForegroundingFlag();

#ifdef SUPPORT_GRAPHICS
    std::shared_ptr<StartOptions> startOptions = nullptr;
    targetAbilityRecord->ProcessForegroundAbility(false, abilityRequest, startOptions, callerAbility);
#else
    targetAbilityRecord->ProcessForegroundAbility(0);
#endif
    return ERR_OK;
}

static int32_t CallType2StartMethod(int32_t callType)
{
    switch (callType) {
        case AbilityCallType::INVALID_TYPE:
            return static_cast<int32_t>(StartMethod::START_NORMAL);
        case AbilityCallType::CALL_REQUEST_TYPE:
            return static_cast<int32_t>(StartMethod::START_CALL);
        default:
            break;
    }
    return -1;
}

static bool CallTypeFilter(int32_t callType)
{
    switch (callType) {
        case AbilityCallType::CALL_REQUEST_TYPE:
            return true;
        default:
            break;
    }
    return false;
}

bool MissionListManager::HandleReusedMissionAndAbility(const AbilityRequest &abilityRequest,
    std::shared_ptr<Mission> &targetMission, std::shared_ptr<AbilityRecord> &targetRecord)
{
    auto startMethod = CallType2StartMethod(abilityRequest.callType);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleReusedMissionAndAbility called startMethod is %{public}d.", startMethod);
    auto reUsedMission = GetReusedMission(abilityRequest);
    if (!reUsedMission) {
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "find reused mission in running list.");
    targetMission = reUsedMission;
    targetRecord = targetMission->GetAbilityRecord();
    if (targetRecord) {
        targetRecord->SetWant(abilityRequest.want);
        targetRecord->SetIsNewWant(true);
    }
    /* No need to update condition:
     *      1. not start by call
     *      2. start by call, but call to background again
     * Need to update condition:
     *      1. start by call, but this time is not start by call
     *      2. start by call, and call to foreground again
     */
    if (!(targetMission->IsStartByCall()
        && (!CallTypeFilter(startMethod) ||
            abilityRequest.want.GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)))) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "mission exists. No update required");
        return true;
    }

    auto missionId = targetMission->GetMissionId();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "mission exists. missionId:%{public}d", missionId);
    InnerMissionInfo info;
    if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
        targetMission->GetMissionId(), info) == 0) {
        info.missionInfo.time = GetCurrentTime();
        info.missionInfo.runningState = 0;
        info.startMethod = CallType2StartMethod(abilityRequest.callType);
        DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
        targetMission->UpdateMissionId(missionId, info.startMethod);
        targetMission->UpdateMissionTime(info.missionInfo.time);
    }

    return true;
}

std::string MissionListManager::GetMissionName(const AbilityRequest &abilityRequest) const
{
    int32_t appIndex = abilityRequest.want.GetIntParam(DLP_INDEX, 0);
    return AbilityUtil::ConvertBundleNameSingleton(abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName, appIndex);
}

bool MissionListManager::CreateOrReusedMissionInfo(const AbilityRequest &abilityRequest, InnerMissionInfo &info) const
{
    // get mission name.
    bool reUsedMissionInfo = false;
    bool needFind = false;
    bool isFindRecentStandard = abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD &&
        abilityRequest.startRecent;
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::STANDARD || isFindRecentStandard) {
        needFind = true;
    }

    std::string missionName = GetMissionName(abilityRequest);
    auto mgr = DelayedSingleton<MissionInfoMgr>::GetInstance();
    if (needFind && mgr &&
        mgr->FindReusedMissionInfo(missionName, abilityRequest.specifiedFlag, isFindRecentStandard, info)
        && info.missionInfo.id > 0) {
        reUsedMissionInfo = true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "result:%{public}d", reUsedMissionInfo);

    BuildInnerMissionInfo(info, missionName, abilityRequest);
    return reUsedMissionInfo;
}

void MissionListManager::GetTargetMissionAndAbility(const AbilityRequest &abilityRequest,
    std::shared_ptr<Mission> &targetMission, std::shared_ptr<AbilityRecord> &targetRecord, bool &isReachToLimit)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (HandleReusedMissionAndAbility(abilityRequest, targetMission, targetRecord)) {
        return;
    }

    // try reuse mission info
    InnerMissionInfo info;
    bool findReusedMissionInfo = CreateOrReusedMissionInfo(abilityRequest, info);
    if (!findReusedMissionInfo) {
        if (!DelayedSingleton<MissionInfoMgr>::GetInstance()->GenerateMissionId(info.missionInfo.id)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "failed to generate mission id.");
            return;
        }
    }

    if (targetMission == nullptr) {
        if (CheckLimit()) {
            isReachToLimit = true;
            TAG_LOGE(AAFwkTag::ABILITYMGR, "already reach to limit, not create new mission and ability.");
            return;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Make new mission data.");
        targetRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        if (targetRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "targetRecord is nullptr");
            return;
        }
        targetMission = std::make_shared<Mission>(info.missionInfo.id, targetRecord,
            info.missionName, info.startMethod);
        targetRecord->UpdateRecoveryInfo(info.hasRecoverInfo);
        info.hasRecoverInfo = false;
        targetMission->SetLockedState(info.missionInfo.lockedState);
        targetMission->SetUnclearable(info.missionInfo.unclearable);
        targetMission->UpdateMissionTime(info.missionInfo.time);
        targetRecord->SetMission(targetMission);
        targetRecord->SetOwnerMissionUserId(userId_);
        SetLastExitReason(targetRecord);

        // handle specified
        if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
            targetMission->SetSpecifiedFlag(abilityRequest.specifiedFlag);
            targetRecord->SetSpecifiedFlag(abilityRequest.specifiedFlag);
        }
    }

    if (!findReusedMissionInfo && targetRecord) {
        info.missionInfo.label = targetRecord->GetLabel();
    }

    if (abilityRequest.abilityInfo.excludeFromMissions) {
        return;
    }

    if (findReusedMissionInfo) {
        DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
    } else {
        DelayedSingleton<MissionInfoMgr>::GetInstance()->AddMissionInfo(info);
    }
}

void MissionListManager::EnableRecoverAbility(int32_t missionId)
{
    InnerMissionInfo info;
    if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, info) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get mission info by id.");
        return;
    }
    info.hasRecoverInfo = true;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
}

void MissionListManager::BuildInnerMissionInfo(InnerMissionInfo &info, const std::string &missionName,
    const AbilityRequest &abilityRequest) const
{
    info.missionName = missionName;
    info.launchMode = static_cast<int32_t>(abilityRequest.abilityInfo.launchMode);
    info.startMethod = CallType2StartMethod(abilityRequest.callType);
    info.bundleName = abilityRequest.abilityInfo.bundleName;
    info.uid = abilityRequest.uid;
    info.missionInfo.runningState = 0;
    info.missionInfo.continuable = abilityRequest.abilityInfo.continuable;
    info.missionInfo.time = GetCurrentTime();
    info.missionInfo.iconPath = abilityRequest.appInfo.iconPath;
    info.missionInfo.want = abilityRequest.want;
    info.missionInfo.unclearable = abilityRequest.abilityInfo.unclearableMission;
    info.isTemporary = abilityRequest.abilityInfo.removeMissionAfterTerminate;
    if (abilityRequest.want.GetIntParam(DLP_INDEX, 0) != 0) {
        info.isTemporary = true;
    }
    info.specifiedFlag = abilityRequest.specifiedFlag;

    auto element = info.missionInfo.want.GetElement();
    if (element.GetBundleName().empty() || element.GetAbilityName().empty()) {
        info.missionInfo.want.SetElementName(abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name);
    }
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionList(
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    // priority : starting launcher ability.
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability request is launcher app.");
        return launcherList_;
    }

    // no caller ability, start ability from system service.
    if (!callerAbility) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability request without caller.");
        std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
        return targetMissionList;
    }

    // caller is launcher, new or reuse list.
    if (callerAbility->IsLauncherAbility()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "start ability, caller is launcher app.");
        return GetTargetMissionListByLauncher(abilityRequest);
    }

    // caller is not launcher: refer to the list of the caller ability.
    return GetTargetMissionListByDefault(callerAbility, abilityRequest);
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionListByLauncher(const AbilityRequest &abilityRequest)
{
    auto reUsedMission = GetReusedMission(abilityRequest);
    if (reUsedMission) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTargetMissionListByLauncher find reused mission list.");
        auto missionList = reUsedMission->GetMissionList();
        if (missionList && missionList != defaultSingleList_ && missionList != defaultStandardList_) {
            return missionList;
        }
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTargetMissionListByLauncher do not find reused mission list.");
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    return targetMissionList;
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionListByDefault(
    const std::shared_ptr<AbilityRecord> &callerAbility, const AbilityRequest &abilityRequest)
{
    if (!callerAbility) {
        return nullptr;
    }

    auto callerMission = callerAbility->GetMission();
    auto callerList = callerAbility->GetOwnedMissionList();
    if (!callerMission || !callerList) {
        return nullptr; // invalid status
    }

    // target mission should in caller mission list
    if (callerList != defaultStandardList_ && callerList != defaultSingleList_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTargetMissionListByDefault target is caller list.");
        return callerList;
    }

    // caller is default, need to start a new mission list
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTargetMissionListByDefault target is default list.");
    std::shared_ptr<MissionList> targetMissionList = std::make_shared<MissionList>();
    callerList->RemoveMission(callerMission);
    targetMissionList->AddMissionToTop(callerMission);

    return targetMissionList;
}

std::shared_ptr<Mission> MissionListManager::GetReusedMission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        return GetReusedSpecifiedMission(abilityRequest);
    }

    if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::STANDARD) {
        return GetReusedStandardMission(abilityRequest);
    }

    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON) {
        return nullptr;
    }

    std::shared_ptr<Mission> reUsedMission = nullptr;
    std::string missionName = GetMissionName(abilityRequest);

    // find launcher first.
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        if ((reUsedMission = launcherList_->GetSingletonMissionByName(missionName)) != nullptr) {
            return reUsedMission;
        }
    }

    // current
    for (auto missionList : currentMissionLists_) {
        if (missionList && (reUsedMission = missionList->GetSingletonMissionByName(missionName)) != nullptr) {
            return reUsedMission;
        }
    }

    // default single list
    if ((reUsedMission = defaultSingleList_->GetSingletonMissionByName(missionName)) != nullptr) {
        return reUsedMission;
    }

    return nullptr;
}

std::shared_ptr<Mission> MissionListManager::GetReusedSpecifiedMission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::SPECIFIED) {
        return nullptr;
    }

    // specified mission name format is same as singleton.
    std::shared_ptr<Mission> reUsedMission = nullptr;
    std::string missionName = GetMissionName(abilityRequest);
    std::string flag = abilityRequest.specifiedFlag;

    // find launcher first.
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        if ((reUsedMission = launcherList_->GetSpecifiedMission(missionName, flag)) != nullptr) {
            return reUsedMission;
        }
    }

    // current
    for (auto missionList : currentMissionLists_) {
        if (missionList && (reUsedMission = missionList->GetSpecifiedMission(missionName, flag)) != nullptr) {
            return reUsedMission;
        }
    }

    // default standard list
    if ((reUsedMission = defaultStandardList_->GetSpecifiedMission(missionName, flag)) != nullptr) {
        return reUsedMission;
    }

    return nullptr;
}

std::shared_ptr<Mission> MissionListManager::GetReusedStandardMission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.abilityInfo.launchMode != AppExecFwk::LaunchMode::STANDARD) {
        return nullptr;
    }
    if (!abilityRequest.startRecent) {
        return nullptr;
    }

    // all mission name format is same.
    std::string missionName = GetMissionName(abilityRequest);
    std::shared_ptr<Mission> reUsedMission = nullptr;

    // find launcher first, if find it in launcher list, no need to find in other list.
    if (abilityRequest.abilityInfo.applicationInfo.isLauncherApp) {
        if ((reUsedMission = launcherList_->GetRecentStandardMission(missionName)) != nullptr) {
            return reUsedMission;
        }
    }

    // try find it from current and default standard.
    std::string missionTime = "0";
    for (const auto &missionList : currentMissionLists_) {
        if (!missionList) {
            continue;
        }
        
        auto mission = missionList->GetRecentStandardMission(missionName);
        if (mission && mission->GetMissionTime() >= missionTime) {
            missionTime = mission->GetMissionTime();
            reUsedMission = mission;
        }
    }

    auto mission = defaultStandardList_->GetRecentStandardMission(missionName);
    if (mission && mission->GetMissionTime() >= missionTime) {
        reUsedMission = mission;
    }

    return reUsedMission;
}

void MissionListManager::MoveMissionToTargetList(bool isCallFromLauncher,
    const std::shared_ptr<MissionList> &targetMissionList,
    const std::shared_ptr<Mission> &mission)
{
    auto missionList = mission->GetMissionList();
    // 1. new mission,move to target list.
    if (!missionList) {
        targetMissionList->AddMissionToTop(mission);
        return;
    }

    // 2. launcher call launcher
    if (isCallFromLauncher && targetMissionList == launcherList_) {
        targetMissionList->AddMissionToTop(mission);
        return;
    }

    // 3. reused mission is in default, move from default to target list.
    if (missionList == defaultSingleList_ || missionList == defaultStandardList_) {
        missionList->RemoveMission(mission);
        targetMissionList->AddMissionToTop(mission);
        return;
    }

    // 4. reused mission is in a valid list.
    bool isListChange = !(targetMissionList == missionList);
    if (isListChange) {
        // list change, pop above missions to default.
        MoveNoneTopMissionToDefaultList(mission);
        missionList->RemoveMission(mission);
    } else if (isCallFromLauncher) {
        // list not change, but call from launcher, pop above missions to default.
        MoveNoneTopMissionToDefaultList(mission);
    }
    targetMissionList->AddMissionToTop(mission);

    if (missionList->IsEmpty()) {
        currentMissionLists_.remove(missionList);
    }
}

void MissionListManager::MoveNoneTopMissionToDefaultList(const std::shared_ptr<Mission> &mission)
{
    auto missionList = mission->GetMissionList();
    if (!missionList) {
        return;
    }

    while (!missionList->IsEmpty()) {
        auto item = missionList->GetTopMission();
        if (item == nullptr || item == mission) {
            break;
        }

        missionList->RemoveMission(item);
        if (item->IsSingletonAbility()) {
            defaultSingleList_->AddMissionToTop(item);
        } else {
            defaultStandardList_->AddMissionToTop(item);
        }
    }
}

void MissionListManager::MoveMissionListToTop(const std::shared_ptr<MissionList> &missionList)
{
    if (!missionList) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "mission list is nullptr.");
        return;
    }
    if (!currentMissionLists_.empty() && currentMissionLists_.front() == missionList) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "mission list is at the top of list");
        return;
    }

    currentMissionLists_.remove(missionList);
    currentMissionLists_.push_front(missionList);
}

int MissionListManager::MinimizeAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool fromUser)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Minimize ability fail, ability record is null.");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ability:%{public}s.", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetPendingState(AbilityState::BACKGROUND);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to minimize ability, ability state is not foreground.");
        return ERR_OK;
    }

    abilityRecord->SetMinimizeReason(fromUser);
    MoveToBackgroundTask(abilityRecord);

    return ERR_OK;
}

std::shared_ptr<AbilityRecord> MissionListManager::GetCurrentTopAbilityLocked() const
{
    if (currentMissionLists_.empty()) {
        return nullptr;
    }

    auto& topMissionList = currentMissionLists_.front();
    if (topMissionList) {
        return topMissionList->GetTopAbility();
    }
    return nullptr;
}

int MissionListManager::AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(managerLock_);
    auto abilityRecord = GetAbilityRecordByTokenInner(token);
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityMS attach abilityThread, name is %{public}s.",
        abilityRecord->GetAbilityInfo().name.c_str());

    auto eventHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(eventHandler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    eventHandler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    abilityRecord->SetLoading(false);
    FreezeUtil::LifecycleFlow flow = { token, FreezeUtil::TimeoutState::LOAD };
    FreezeUtil::GetInstance().DeleteLifecycleEvent(flow);

    abilityRecord->SetScheduler(scheduler);

    if (abilityRecord->IsStartedByCall()) {
        if (abilityRecord->GetWant().GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            abilityRecord->SetStartToForeground(true);
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);
        } else {
            abilityRecord->SetStartToBackground(true);
            MoveToBackgroundTask(abilityRecord);
        }
        return ERR_OK;
    }

    if (abilityRecord->IsNeedToCallRequest()) {
        abilityRecord->CallRequest();
    }

    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(taskHandler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    auto taskName = std::to_string(abilityRecord->GetMissionId()) + "_cold";
    taskHandler->CancelTask(taskName);
#ifdef SUPPORT_GRAPHICS
    abilityRecord->PostCancelStartingWindowHotTask();
#endif
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token);

    return ERR_OK;
}

void MissionListManager::OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability request state %{public}d done.", state);
    std::lock_guard guard(managerLock_);
    AppAbilityState abilityState = DelayedSingleton<AppScheduler>::GetInstance()->ConvertToAppAbilityState(state);
    if (abilityState == AppAbilityState::ABILITY_STATE_FOREGROUND) {
        auto abilityRecord = GetAliveAbilityRecordByToken(token);
        CHECK_POINTER(abilityRecord);
        std::string element = abilityRecord->GetElementName().GetURI();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability is %{public}s, start to foreground.", element.c_str());
        abilityRecord->ForegroundAbility(abilityRecord->lifeCycleStateInfo_.sceneFlagBak);
    }
}

void MissionListManager::OnAppStateChanged(const AppInfo &info)
{
    std::lock_guard guard(managerLock_);

    if (info.state == AppState::TERMINATED || info.state == AppState::END) {
        for (const auto& abilityRecord : terminateAbilityList_) {
            if (!abilityRecord) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
                continue;
            }
            if (info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                abilityRecord->SetAppState(info.state);
            }
        }
    } else {
        for (const auto& missionList : currentMissionLists_) {
            auto missions = missionList->GetAllMissions();
            for (const auto& missionInfo : missions) {
                if (!missionInfo) {
                    TAG_LOGE(AAFwkTag::ABILITYMGR, "missionInfo is nullptr.");
                    continue;
                }
                auto abilityRecord = missionInfo->GetAbilityRecord();
                if (info.processName == abilityRecord->GetAbilityInfo().process ||
                    info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                    abilityRecord->SetAppState(info.state);
                }
            }
        }
        auto defaultStandardListmissions = defaultStandardList_->GetAllMissions();
        for (const auto& missionInfo : defaultStandardListmissions) {
            if (!missionInfo) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "defaultStandardListmissions is nullptr.");
                continue;
            }
            auto abilityRecord = missionInfo->GetAbilityRecord();
            if (info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                abilityRecord->SetAppState(info.state);
            }
        }
        auto defaultSingleListmissions = defaultSingleList_->GetAllMissions();
        for (const auto& missionInfo : defaultSingleListmissions) {
            if (!missionInfo) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "defaultSingleListmissions is nullptr.");
                continue;
            }
            auto abilityRecord = missionInfo->GetAbilityRecord();
            if (info.processName == abilityRecord->GetAbilityInfo().process ||
                info.processName == abilityRecord->GetApplicationInfo().bundleName) {
                abilityRecord->SetAppState(info.state);
            }
        }
    }
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    std::lock_guard guard(managerLock_);
    return GetAbilityRecordByTokenInner(token);
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByTokenInner(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        return nullptr;
    }
    // first find in terminating list
    for (auto ability : terminateAbilityList_) {
        if (ability && token == ability->GetToken()->AsObject()) {
            return ability;
        }
    }

    return GetAliveAbilityRecordByToken(token);
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAliveAbilityRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (abilityRecord = missionList->GetAbilityRecordByToken(token)) != nullptr) {
            return abilityRecord;
        }
    }

    if ((abilityRecord = defaultSingleList_->GetAbilityRecordByToken(token)) != nullptr) {
        return abilityRecord;
    }

    return defaultStandardList_->GetAbilityRecordByToken(token);
}

std::shared_ptr<Mission> MissionListManager::GetMissionById(int missionId) const
{
    std::shared_ptr<Mission> mission = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (mission = missionList->GetMissionById(missionId)) != nullptr) {
            return mission;
        }
    }

    if ((mission = defaultSingleList_->GetMissionById(missionId)) != nullptr) {
        return mission;
    }

    if ((mission = launcherList_->GetMissionById(missionId)) != nullptr) {
        return mission;
    }

    return defaultStandardList_->GetMissionById(missionId);
}

int MissionListManager::AbilityTransactionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int targetState = AbilityRecord::ConvertLifeCycleToAbilityState(static_cast<AbilityLifeCycleState>(state));
    std::string abilityState = AbilityRecord::ConvertAbilityState(static_cast<AbilityState>(targetState));
    TAG_LOGI(AAFwkTag::ABILITYMGR, "state: %{public}s.", abilityState.c_str());

    std::lock_guard guard(managerLock_);
    auto abilityRecord = GetAbilityFromTerminateListInner(token);
    if (abilityRecord == nullptr) {
        abilityRecord = GetAbilityRecordByTokenInner(token);
        CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    }

    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability: %{public}s, state: %{public}s", element.c_str(), abilityState.c_str());

    if (targetState == AbilityState::BACKGROUND) {
        abilityRecord->SaveAbilityState(saveData);
    }

    return DispatchState(abilityRecord, targetState);
}

int MissionListManager::DispatchState(const std::shared_ptr<AbilityRecord> &abilityRecord, int state)
{
    switch (state) {
        case AbilityState::INITIAL: {
            return DispatchTerminate(abilityRecord);
        }
        case AbilityState::BACKGROUND:
        case AbilityState::BACKGROUND_FAILED: {
            return DispatchBackground(abilityRecord);
        }
        case AbilityState::FOREGROUND: {
            return DispatchForeground(abilityRecord, true);
        }
        case AbilityState::FOREGROUND_FAILED:
        case AbilityState::FOREGROUND_INVALID_MODE:
        case AbilityState::FOREGROUND_WINDOW_FREEZED:
        case AbilityState::FOREGROUND_DO_NOTHING: {
            return DispatchForeground(abilityRecord, false, static_cast<AbilityState>(state));
        }
        default: {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Don't support transiting state: %{public}d", state);
            return ERR_INVALID_VALUE;
        }
    }
}

int MissionListManager::DispatchForeground(const std::shared_ptr<AbilityRecord> &abilityRecord, bool success,
    AbilityState state)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        PostStartWaitingAbility();
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "DispatchForeground Ability transition life state error. expect %{public}d, actual %{public}d",
            AbilityState::FOREGROUNDING, abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }
    auto eventHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_AND_RETURN_LOG(eventHandler, ERR_INVALID_VALUE, "Fail to get AbilityEventHandler.");
    eventHandler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    g_deleteLifecycleEventTask(abilityRecord->GetToken(), FreezeUtil::TimeoutState::FOREGROUND);
    auto self(weak_from_this());
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(taskHandler, ERR_INVALID_VALUE, "Fail to get AbilityTaskHandler.");
    if (success) {
#ifdef SUPPORT_GRAPHICS
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ok");
        abilityRecord->SetStartingWindow(false);
        auto taskName = std::to_string(abilityRecord->GetMissionId()) + "_hot";
        taskHandler->CancelTask(taskName);
#endif
        auto task = [self, abilityRecord]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "Mission list mgr is invalid.");
                return;
            }
            selfObj->CompleteForegroundSuccess(abilityRecord);
        };
        taskHandler->SubmitTask(task);
    } else {
        auto task = [self, abilityRecord, state]() {
            auto selfObj = self.lock();
            if (!selfObj) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "Mission list mgr is invalid.");
                return;
            }
            selfObj->CompleteForegroundFailed(abilityRecord, state);
        };
        taskHandler->SubmitTask(task);
    }
    return ERR_OK;
}

void MissionListManager::CompleteForegroundSuccess(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(managerLock_);

    CHECK_POINTER(abilityRecord);
    // ability do not save window mode
    abilityRecord->RemoveWindowMode();
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability: %{public}s", element.c_str());

    abilityRecord->SetAbilityState(AbilityState::FOREGROUND);

#if BINDER_IPC_32BIT
    TAG_LOGI(AAFwkTag::ABILITYMGR, "bundle:%{public}s,ability:%{public}s,time:%{public}lld",
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str(),
        AbilityUtil::UTCTimeSeconds());
#else
    TAG_LOGI(AAFwkTag::ABILITYMGR, "bundle:%{public}s,ability:%{public}s,time:%{public}ld",
        abilityRecord->GetAbilityInfo().bundleName.c_str(),
        abilityRecord->GetAbilityInfo().name.c_str(),
        AbilityUtil::UTCTimeSeconds());
#endif

    auto mission = abilityRecord->GetMission();
    if (mission) {
        auto currentTime = GetCurrentTime();
        mission->UpdateMissionTime(currentTime);
        InnerMissionInfo info;
        if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
            mission->GetMissionId(), info) == 0) {
            info.missionInfo.time = currentTime;
            info.missionInfo.runningState = 0;
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
        }
    }

    if (mission && mission->IsMovingState()) {
        mission->SetMovingState(false);
        if (listenerController_ && !(abilityRecord->GetAbilityInfo().excludeFromMissions)) {
            listenerController_->NotifyMissionMovedToFront(mission->GetMissionId());
        }
    }
    PostStartWaitingAbility();
    TerminatePreviousAbility(abilityRecord);

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToForeground() && abilityRecord->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call request after completing foreground state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToForeground(false);
    }

    if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        abilityRecord->SetMinimizeReason(true);
        MoveToBackgroundTask(abilityRecord);
    } else if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }
}

void MissionListManager::TerminatePreviousAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto terminatingAbilityRecord = abilityRecord->GetPreAbilityRecord();
    if (!terminatingAbilityRecord) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "terminatingAbilityRecord is nullptr.");
        return;
    }
    abilityRecord->SetPreAbilityRecord(nullptr);
    auto self(shared_from_this());
    if (terminatingAbilityRecord->GetAbilityState() == AbilityState::FOREGROUND) {
        MoveToBackgroundTask(terminatingAbilityRecord, true);
    }
    if (terminatingAbilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
        auto task = [terminatingAbilityRecord, self]() {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "To terminate terminatingAbilityRecord.");
            self->DelayCompleteTerminate(terminatingAbilityRecord);
        };
        terminatingAbilityRecord->Terminate(task);
    }
}

int MissionListManager::DispatchBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTasktHandler.");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);

    if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability transition life state error. actual %{public}d",
            abilityRecord->GetAbilityState());
        return ERR_INVALID_VALUE;
    }

    // remove background timeout task.
    handler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    g_deleteLifecycleEventTask(abilityRecord->GetToken(), FreezeUtil::TimeoutState::BACKGROUND);
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteBackground(abilityRecord); };
    handler->SubmitTask(task);

    return ERR_OK;
}

void MissionListManager::CompleteBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard guard(managerLock_);
    if (abilityRecord->GetAbilityState() != AbilityState::BACKGROUNDING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability state is %{public}d, it can't complete background.",
            abilityRecord->GetAbilityState());
        return;
    }

    abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
    // send application state to AppMS.
    // notify AppMS to update application state.
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
    } else if (abilityRecord->GetPendingState() == AbilityState::BACKGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not continuous startup.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
    }

    if (abilityRecord->IsSwitchingPause()) {
        abilityRecord->SetSwitchingPause(false);
        return;
    }

    // Abilities ahead of the one started with SingleTask mode were put in terminate list, we need to terminate
    // them.
    auto self(shared_from_this());
    for (auto terminateAbility : terminateAbilityList_) {
        if (terminateAbility->GetAbilityState() == AbilityState::BACKGROUND) {
            auto timeoutTask = [terminateAbility, self]() {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "Terminate ability timeout after background.");
                self->DelayCompleteTerminate(terminateAbility);
            };
            terminateAbility->Terminate(timeoutTask);
        }
    }

    // new version. started by caller, scheduler call request
    if (abilityRecord->IsStartedByCall() && abilityRecord->IsStartToBackground() && abilityRecord->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "call request after completing background state");
        abilityRecord->CallRequest();
        abilityRecord->SetStartToBackground(false);
    }
}

int MissionListManager::MoveAbilityToBackground(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(managerLock_);
    return MoveAbilityToBackgroundLocked(abilityRecord);
}

int MissionListManager::MoveAbilityToBackgroundLocked(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Move ability to background fail, ability record is null.");
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability:%{public}s.", abilityRecord->GetAbilityInfo().name.c_str());
    RemoveBackgroundingAbility(abilityRecord);

    if (abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "current ability is active");
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        auto nextAbilityRecord = abilityRecord->GetNextAbilityRecord();
        if (nextAbilityRecord) {
            nextAbilityRecord->SetPreAbilityRecord(abilityRecord);
#ifdef SUPPORT_GRAPHICS
            nextAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
            nextAbilityRecord->ProcessForegroundAbility(abilityRecord, false);
        } else {
            bool animaEnabled = false;
            if (!abilityRecord->IsClearMissionFlag()) {
                abilityRecord->NotifyAnimationFromMinimizeAbility(animaEnabled);
            }
            if (animaEnabled) {
                return ERR_OK;
            }
#else
            nextAbilityRecord->ProcessForegroundAbility(0);
        } else {
#endif
            MoveToBackgroundTask(abilityRecord, true);
        }
        return ERR_OK;
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Move ability to background fail, ability state is not foreground.");
        return ERR_OK;
    }
}

void MissionListManager::RemoveBackgroundingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_LOG(abilityRecord, "RemoveBackgroundingAbility fail, ability record is null.")
    auto missionList = abilityRecord->GetOwnedMissionList();
    CHECK_POINTER(missionList);
    auto mission = missionList->GetTopMission();
    missionList->RemoveMissionByAbilityRecord(abilityRecord);
    if (mission->IsSingletonAbility()) {
        defaultSingleList_->AddMissionToTop(mission);
    } else {
        defaultStandardList_->AddMissionToTop(mission);
    }

    if (missionList->IsEmpty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Remove backgrounding ability, missionList is empty, remove.");
        RemoveMissionList(missionList);
    }

    abilityRecord->SetNextAbilityRecord(nullptr);
    if (!(abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING))) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability state is %{public}d, just return.", abilityRecord->GetAbilityState());
        return;
    }

    std::shared_ptr<AbilityRecord> needTopAbility;
    if (!missionList->IsEmpty()) {
        needTopAbility = missionList->GetTopAbility();
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "mission list is empty, no next ability.");
    }

    if (!needTopAbility) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The ability needs to top is null.");
        if (!abilityRecord->IsNeedBackToOtherMissionStack()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "This ability doesn't need back to other mission stack.");
            return;
        }
        needTopAbility = abilityRecord->GetOtherMissionStackAbilityRecord();
        CHECK_POINTER_LOG(needTopAbility, "The ability needs back to other mission stack, but needTopAbility is null.")
        abilityRecord->SetNeedBackToOtherMissionStack(false);
    }

    AppExecFwk::ElementName elementName = needTopAbility->GetElementName();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Next top ability is %{public}s, state is %{public}d, minimizeReason is %{public}d!",
        elementName.GetURI().c_str(), needTopAbility->GetAbilityState(), needTopAbility->IsMinimizeFromUser());

    if (elementName.GetBundleName() == AbilityConfig::LAUNCHER_BUNDLE_NAME &&
        elementName.GetAbilityName() == AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Next to need is recent, just to launcher!");
        needTopAbility = launcherList_->GetLauncherRoot();
    }

    CHECK_POINTER_LOG(needTopAbility, "NeedTopAbility of launcherRoot is null.")

    if (!needTopAbility->IsForeground() && !needTopAbility->IsMinimizeFromUser() && needTopAbility->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is need to foreground.", elementName.GetURI().c_str());
        abilityRecord->SetNextAbilityRecord(needTopAbility);
    }
}

int MissionListManager::TerminateAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int resultCode, const Want *resultWant, bool flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(managerLock_);
    return TerminateAbilityInner(abilityRecord, resultCode, resultWant, flag);
}

int MissionListManager::TerminateAbilityInner(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int resultCode, const Want *resultWant, bool flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate ability, ability is %{public}s.", element.c_str());
    if (abilityRecord->IsTerminating() && !abilityRecord->IsForeground()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability is on terminating.");
        return ERR_OK;
    }

    if (abilityRecord->IsTerminating() && abilityRecord->IsForeground()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Ability is on terminating and ability state is foreground, force close");
        flag = false;
    }

    // double check to avoid the ability has been removed
    if (!GetAbilityRecordByTokenInner(abilityRecord->GetToken())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability has already been removed");
        return ERR_OK;
    }

    abilityRecord->SetTerminatingState();
    // save result to caller AbilityRecord
    if (resultWant != nullptr) {
        Want* newWant = const_cast<Want*>(resultWant);
        newWant->RemoveParam(Want::PARAM_RESV_CALLER_TOKEN);
        abilityRecord->SaveResultToCallers(resultCode, newWant);
    } else {
        Want want;
        abilityRecord->SaveResultToCallers(resultCode, &want);
    }

    return TerminateAbilityLocked(abilityRecord, flag);
}

int MissionListManager::TerminateAbilityLocked(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag)
{
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Terminate ability locked, ability is %{public}s.", element.c_str());
    // remove AbilityRecord out of list
    RemoveTerminatingAbility(abilityRecord, flag);
    abilityRecord->SendResultToCallers();

    // 1. if the ability was foreground, first should find whether there is other ability foreground
    if (abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "current ability is active");
        abilityRecord->SetPendingState(AbilityState::BACKGROUND);
        auto nextAbilityRecord = abilityRecord->GetNextAbilityRecord();
        if (nextAbilityRecord) {
            nextAbilityRecord->SetPreAbilityRecord(abilityRecord);
#ifdef SUPPORT_GRAPHICS
            nextAbilityRecord->SetPendingState(AbilityState::FOREGROUND);
            nextAbilityRecord->ProcessForegroundAbility(abilityRecord);
        } else {
            if (!abilityRecord->IsClearMissionFlag()) {
                abilityRecord->NotifyAnimationFromTerminatingAbility();
            }
#else
            nextAbilityRecord->ProcessForegroundAbility(0);
        } else {
#endif
            MoveToBackgroundTask(abilityRecord, true);
        }
        return ERR_OK;
    }

    // 2. if the ability was BACKGROUNDING, waiting for completeBackgroundNew

    // 3. ability on background, schedule to terminate.
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUND) {
        auto self(shared_from_this());
        auto task = [abilityRecord, self]() {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Terminate ability timeout.");
            self->DelayCompleteTerminate(abilityRecord);
        };
        abilityRecord->Terminate(task);
    }
    return ERR_OK;
}

/**
 * @brief This method aims to do things as below
 * 1. remove the mission from the current missionList
 * 2. if the current missionList is empty after, then remove from the manager
 * 3. if the current ability is foreground, then should schedule the next ability to foreground before terminate
 *
 * @param abilityRecord the ability that was terminating
 */
void MissionListManager::RemoveTerminatingAbility(const std::shared_ptr<AbilityRecord> &abilityRecord, bool flag)
{
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Remove terminating ability, ability is %{public}s.", element.c_str());
    if (GetAbilityFromTerminateListInner(abilityRecord->GetToken())) {
        abilityRecord->SetNextAbilityRecord(nullptr);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Find ability in terminating list, return.");
        return;
    }

    auto missionList = abilityRecord->GetOwnedMissionList();
    CHECK_POINTER(missionList);

    missionList->RemoveMissionByAbilityRecord(abilityRecord);
    DelayedSingleton<AppScheduler>::GetInstance()->PrepareTerminate(abilityRecord->GetToken());
    terminateAbilityList_.push_back(abilityRecord);

    if (missionList->IsEmpty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Remove terminating ability, missionList is empty, remove.");
        RemoveMissionList(missionList);
    }

    // 1. clear old
    abilityRecord->SetNextAbilityRecord(nullptr);
    // 2. if the ability to terminate is background, just background
    if (!(abilityRecord->IsAbilityState(FOREGROUND) || abilityRecord->IsAbilityState(FOREGROUNDING))) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability state is %{public}d, just return.", abilityRecord->GetAbilityState());
        return;
    }
    // 3. if close ability, noting to do
    if (!flag) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Close ability schedule.");
        return;
    }

    // 4. the ability should find the next ability to foreground
    std::shared_ptr<AbilityRecord> needTopAbility;
    if (!missionList->IsEmpty()) {
        needTopAbility = missionList->GetTopAbility();
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "mission list is empty, no next ability.");
    }

    if (!needTopAbility) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The ability need to top is null.");
        if (!abilityRecord->IsNeedBackToOtherMissionStack()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "This ability doesn't need back to other mission stack.");
            return;
        }
        needTopAbility = abilityRecord->GetOtherMissionStackAbilityRecord();
        if (!needTopAbility) {
            TAG_LOGE(
                AAFwkTag::ABILITYMGR, "This ability needs back to other mission stack, but needTopAbility is null.");
            return;
        }
        abilityRecord->SetNeedBackToOtherMissionStack(false);
    }
    AppExecFwk::ElementName elementName = needTopAbility->GetElementName();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Next top ability is %{public}s, state is %{public}d, minimizeReason is %{public}d.",
        elementName.GetURI().c_str(), needTopAbility->GetAbilityState(), needTopAbility->IsMinimizeFromUser());

    // 5. if caller is recent, close
    if (elementName.GetBundleName() == AbilityConfig::LAUNCHER_BUNDLE_NAME &&
        elementName.GetAbilityName() == AbilityConfig::LAUNCHER_RECENT_ABILITY_NAME) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Next to need is recent, just to launcher.");
        needTopAbility = launcherList_->GetLauncherRoot();
    }

    if (!needTopAbility) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "NeedTopAbility of launcherRoot is null.");
        return;
    }

    if (!needTopAbility->IsForeground() && !needTopAbility->IsMinimizeFromUser() && needTopAbility->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is need to foreground.", elementName.GetURI().c_str());
        abilityRecord->SetNextAbilityRecord(needTopAbility);
        needTopAbility->SetAbilityForegroundingFlag();
    }
}

void MissionListManager::RemoveMissionList(const std::shared_ptr<MissionList> &missionList)
{
    if (missionList == nullptr) {
        return;
    }

    for (auto iter = currentMissionLists_.begin(); iter != currentMissionLists_.end(); iter++) {
        if ((*iter) == missionList) {
            currentMissionLists_.erase(iter);
            return;
        }
    }
}

int MissionListManager::DispatchTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DispatchTerminate error, ability state is %{public}d",
            abilityRecord->GetAbilityState());
        return INNER_ERR;
    }

    // remove terminate timeout task.
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_AND_RETURN_LOG(handler, ERR_INVALID_VALUE, "Fail to get AbilityTasktHandler.");
    handler->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    auto self(shared_from_this());
    auto task = [self, abilityRecord]() { self->CompleteTerminate(abilityRecord); };
    handler->SubmitTask(task);

    return ERR_OK;
}

void MissionListManager::DelayCompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER(handler);

    PrintTimeOutLog(abilityRecord, AbilityManagerService::TERMINATE_TIMEOUT_MSG);

    auto timeoutTask = [self = shared_from_this(), abilityRecord]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "emit delay complete terminate task.");
        self->CompleteTerminate(abilityRecord);
    };
    int killTimeout = AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * KILL_TIMEOUT_MULTIPLE;
    handler->SubmitTask(timeoutTask, "DELAY_KILL_PROCESS", killTimeout);
}

void MissionListManager::CompleteTerminate(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    std::lock_guard guard(managerLock_);
    if (abilityRecord->GetAbilityState() != AbilityState::TERMINATING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, ability is not terminating.", __func__);
        return;
    }
    abilityRecord->RemoveAbilityDeathRecipient();

    // notify AppMS terminate
    if (abilityRecord->TerminateAbility() != ERR_OK) {
        // Don't return here
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AppMS fail to terminate ability.");
    }

    auto&& preAbilityRecord = abilityRecord->GetPreAbilityRecord();
    if (preAbilityRecord != nullptr) {
        TerminateAbilityLocked(preAbilityRecord, false);
    }

    CompleteTerminateAndUpdateMission(abilityRecord);
}

void MissionListManager::CompleteTerminateAndUpdateMission(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    for (auto it : terminateAbilityList_) {
        if (it == abilityRecord) {
            abilityRecord->RevokeUriPermission();
            terminateAbilityList_.remove(it);
            // update inner mission info time
            bool excludeFromMissions = abilityRecord->GetAbilityInfo().excludeFromMissions;
            if ((abilityRecord->GetAppIndex() != 0) || abilityRecord->GetAbilityInfo().removeMissionAfterTerminate ||
                excludeFromMissions) {
                RemoveMissionLocked(abilityRecord->GetMissionId(), excludeFromMissions);
                return;
            }
            InnerMissionInfo innerMissionInfo;
            auto missionId = abilityRecord->GetMissionId();
            int result = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
                missionId, innerMissionInfo);
            if (result != 0) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Get missionInfo error, result is %{public}d, missionId is %{public}d",
                    result, missionId);
                break;
            }
            innerMissionInfo.hasRecoverInfo = false;
            innerMissionInfo.missionInfo.runningState = -1;
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(innerMissionInfo);
            if (listenerController_) {
                listenerController_->NotifyMissionClosed(missionId);
            }
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Destroy ability record count %{public}ld", abilityRecord.use_count());
            break;
        }
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Can't find ability in terminate list.");
    }
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityFromTerminateList(const sptr<IRemoteObject> &token)
{
    std::lock_guard guard(managerLock_);
    return GetAbilityFromTerminateListInner(token);
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityFromTerminateListInner(const sptr<IRemoteObject> &token)
{
    if (!token) {
        return nullptr;
    }

    for (auto abilityRecord : terminateAbilityList_) {
        // token is type of IRemoteObject, abilityRecord->GetToken() is type of Token extending from IRemoteObject.
        if (abilityRecord && abilityRecord->GetToken() && token == abilityRecord->GetToken()->AsObject()) {
            return abilityRecord;
        }
    }
    return nullptr;
}

int MissionListManager::ClearMission(int missionId)
{
    if (missionId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Mission id is invalid.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard guard(managerLock_);
    auto mission = GetMissionById(missionId);
    if (mission && mission->GetMissionList() && mission->GetMissionList()->GetType() == MissionListType::LAUNCHER) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Mission id is launcher, can not clear.");
        return ERR_INVALID_VALUE;
    }

    if (IsExcludeFromMissions(mission)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "excludeFromMissions is true, not clear by id.");
        return ERR_INVALID_VALUE;
    }

    if (mission && mission->IsUnclearable()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "mission is unclearable.");
        return ERR_INVALID_VALUE;
    }

    if (CheckPrepareTerminateEnable(mission)) {
        return PrepareClearMissionLocked(missionId, mission);
    }

    return ClearMissionLocked(missionId, mission);
}

int MissionListManager::ClearMissionLocking(int missionId, const std::shared_ptr<Mission> &mission)
{
    std::lock_guard guard(managerLock_);
    return ClearMissionLocked(missionId, mission);
}

int MissionListManager::ClearMissionLocked(int missionId, const std::shared_ptr<Mission> &mission)
{
    if (missionId != -1) {
        DelayedSingleton<MissionInfoMgr>::GetInstance()->DeleteMissionInfo(missionId);
        if (listenerController_) {
            listenerController_->NotifyMissionDestroyed(missionId);
        }
    }

    if (mission == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability has already terminate, just remove mission.");
        return ERR_OK;
    }

    auto abilityRecord = mission->GetAbilityRecord();
    if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Ability record is not exist or is on terminating.");
        return ERR_OK;
    }

    abilityRecord->SetTerminatingState();
    abilityRecord->SetClearMissionFlag(true);
    Want want;
    abilityRecord->SaveResultToCallers(-1, &want);
    auto ret = TerminateAbilityLocked(abilityRecord, false);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "clear mission error: %{public}d.", ret);
        return REMOVE_MISSION_FAILED;
    }

    return ERR_OK;
}

int MissionListManager::ClearAllMissions()
{
    std::lock_guard guard(managerLock_);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->DeleteAllMissionInfos(listenerController_);
    std::list<std::shared_ptr<Mission>> foregroundAbilities;
    ClearAllMissionsLocked(defaultStandardList_->GetAllMissions(), foregroundAbilities, false);
    ClearAllMissionsLocked(defaultSingleList_->GetAllMissions(), foregroundAbilities, false);

    for (auto listIter = currentMissionLists_.begin(); listIter != currentMissionLists_.end();) {
        auto missionList = (*listIter);
        listIter++;
        if (!missionList || missionList->GetType() == MissionListType::LAUNCHER) {
            continue;
        }
        ClearAllMissionsLocked(missionList->GetAllMissions(), foregroundAbilities, true);
    }

    ClearAllMissionsLocked(foregroundAbilities, foregroundAbilities, false);
    return ERR_OK;
}

void MissionListManager::ClearAllMissionsLocked(std::list<std::shared_ptr<Mission>> &missionList,
    std::list<std::shared_ptr<Mission>> &foregroundAbilities, bool searchActive)
{
    for (auto listIter = missionList.begin(); listIter != missionList.end();) {
        auto mission = (*listIter);
        listIter++;
        if (!mission || mission->IsLockedState()) {
            continue;
        }

        if (mission && mission->IsUnclearable()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "mission is unclearable.");
            continue;
        }

        auto abilityMs_ = OHOS::DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMs_->IsBackgroundTaskUid(mission->GetAbilityRecord()->GetUid())) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "the mission is background task, do not need clear");
            continue;
        }

        if (searchActive && mission->GetAbilityRecord() && mission->GetAbilityRecord()->IsActiveState()) {
            foregroundAbilities.push_front(mission);
            continue;
        }
        if (CheckPrepareTerminateEnable(mission)) {
            PrepareClearMissionLocked(-1, mission);
        } else {
            ClearMissionLocked(-1, mission);
        }
    }
}

int MissionListManager::SetMissionLockedState(int missionId, bool lockedState)
{
    std::lock_guard guard(managerLock_);
    if (missionId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "param is invalid");
        return MISSION_NOT_FOUND;
    }

    std::shared_ptr<Mission> mission = GetMissionById(missionId);
    if (mission) {
        auto abilityRecord = mission->GetAbilityRecord();
        if (abilityRecord && abilityRecord->GetAbilityInfo().excludeFromMissions) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "excludeFromMissions is true, missionId:%{public}d", missionId);
            return MISSION_NOT_FOUND;
        }
        mission->SetLockedState(lockedState);
    }

    // update inner mission info time
    InnerMissionInfo innerMissionInfo;
    auto ret = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, innerMissionInfo);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "mission is not exist, missionId %{public}d", missionId);
        return MISSION_NOT_FOUND;
    }
    innerMissionInfo.missionInfo.lockedState = lockedState;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(innerMissionInfo);
    return ERR_OK;
}

void MissionListManager::UpdateSnapShot(const sptr<IRemoteObject> &token,
    const std::shared_ptr<Media::PixelMap> &pixelMap)
{
    auto abilityRecord = GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Cannot find AbilityRecord by Token.");
        return;
    }
    if (abilityRecord->GetAbilityInfo().excludeFromMissions) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "excludeFromMissions is true, no need to update mission snapshot.");
        return;
    }
    int32_t missionId = abilityRecord->GetMissionId();
    auto isPrivate = abilityRecord->GetAppIndex() != 0;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionSnapshot(missionId, pixelMap, isPrivate);
    if (listenerController_) {
        listenerController_->NotifyMissionSnapshotChanged(missionId);
    }
}

void MissionListManager::MoveToBackgroundTask(const std::shared_ptr<AbilityRecord> &abilityRecord, bool isClose)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Move the ability to background fail, ability record is null.");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ability:%{public}s.", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->SetIsNewWant(false);
    if (abilityRecord->lifeCycleStateInfo_.sceneFlag != SCENE_FLAG_KEYGUARD &&
        !abilityRecord->IsClearMissionFlag() && !isClose) {
        UpdateMissionSnapshot(abilityRecord);
    }

    auto self(shared_from_this());
    auto task = [abilityRecord, self]() {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Mission list manager move to background timeout.");
        self->PrintTimeOutLog(abilityRecord, AbilityManagerService::BACKGROUND_TIMEOUT_MSG);
        self->CompleteBackground(abilityRecord);
    };
    abilityRecord->BackgroundAbility(task);
}

void  MissionListManager::NotifyMissionCreated(const std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    CHECK_POINTER(abilityRecord);
    auto mission = abilityRecord->GetMission();
    if (mission && mission->NeedNotify() && listenerController_ &&
        !(abilityRecord->GetAbilityInfo().excludeFromMissions)) {
        auto missionId = abilityRecord->GetMissionId();
        listenerController_->NotifyMissionCreated(missionId);
        mission->SetNotifyLabel(false);

        if (mission->NeedNotifyUpdateLabel()) {
            PostMissionLabelUpdateTask(missionId);
            mission->SetNeedNotifyUpdateLabel(false);
        }
    }
}

void MissionListManager::PostMissionLabelUpdateTask(int missionId) const
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to get EventHandler, do not post mission label update message.");
        return;
    }

    std::weak_ptr<MissionListenerController> wpController = listenerController_;
    auto task = [wpController, missionId] {
        auto controller = wpController.lock();
        if (controller == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "controller is nullptr.");
            return;
        }
        controller->NotifyMissionLabelUpdated(missionId);
    };
    handler->SubmitTask(task, "NotifyMissionLabelUpdated.", DELAY_NOTIFY_LABEL_TIME);
}

void MissionListManager::PrintTimeOutLog(const std::shared_ptr<AbilityRecord> &ability, uint32_t msgId, bool isHalf)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability is nullptr");
        return;
    }

    AppExecFwk::RunningProcessInfo processInfo = {};
    DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByToken(ability->GetToken(), processInfo);
    if (processInfo.pid_ == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "error: the ability[%{public}s], app may fork fail or not running.",
            ability->GetAbilityInfo().name.data());
        return;
    }
    int typeId = AppExecFwk::AppfreezeManager::TypeAttribute::NORMAL_TIMEOUT;
    std::string msgContent = "ability:" + ability->GetAbilityInfo().name + " ";
    FreezeUtil::TimeoutState state = FreezeUtil::TimeoutState::UNKNOWN;
    auto search = stateMap.find(msgId);
    if (search != stateMap.end()) {
        state = search->second;
    }
    if (!GetContentAndTypeId(msgId, msgContent, typeId)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "msgId is invalid!");
        return;
    }

    std::string eventName = isHalf ?
        AppExecFwk::AppFreezeType::LIFECYCLE_HALF_TIMEOUT : AppExecFwk::AppFreezeType::LIFECYCLE_TIMEOUT;
    TAG_LOGW(AAFwkTag::ABILITYMGR,
        "%{public}s: uid: %{public}d, pid: %{public}d, bundleName: %{public}s, abilityName: %{public}s,"
        "msg: %{public}s!",
        eventName.c_str(), processInfo.uid_, processInfo.pid_, ability->GetAbilityInfo().bundleName.c_str(),
        ability->GetAbilityInfo().name.c_str(), msgContent.c_str());

    AppExecFwk::AppfreezeManager::ParamInfo info = {
        .typeId = typeId,
        .pid = processInfo.pid_,
        .eventName = eventName,
        .bundleName = ability->GetAbilityInfo().bundleName,
    };
    if (state != FreezeUtil::TimeoutState::UNKNOWN) {
        auto flow = std::make_unique<FreezeUtil::LifecycleFlow>();
        if (ability->GetToken() != nullptr) {
            flow->token = ability->GetToken()->AsObject();
            flow->state = state;
        }
        info.msg = msgContent + "\nserver:\n" + FreezeUtil::GetInstance().GetLifecycleEvent(*flow);
        if (!isHalf) {
            FreezeUtil::GetInstance().DeleteLifecycleEvent(*flow);
        }
        AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info, std::move(flow));
    } else {
        info.msg = msgContent;
        AppExecFwk::AppfreezeManager::GetInstance()->LifecycleTimeoutHandle(info);
    }
}

bool MissionListManager::GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const
{
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent += "load timeout";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            msgContent += "active timeout";
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
            msgContent += "inactive timeout";
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            msgContent += "foreground timeout";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
            msgContent += "background timeout";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent += "terminate timeout";
            break;
        default:
            return false;
    }
    return true;
}

void MissionListManager::UpdateMissionSnapshot(const std::shared_ptr<AbilityRecord>& abilityRecord) const
{
    CHECK_POINTER(abilityRecord);
    if (abilityRecord->GetAbilityInfo().excludeFromMissions) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "excludeFromMissions is true, no need to update mission snapshot.");
        return;
    }
    int32_t missionId = abilityRecord->GetMissionId();
    MissionSnapshot snapshot;
    snapshot.isPrivate = (abilityRecord->GetAppIndex() != 0);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionSnapshot(missionId, abilityRecord->GetToken(),
        snapshot);
    if (listenerController_) {
        listenerController_->NotifyMissionSnapshotChanged(missionId);
    }
}

void MissionListManager::OnTimeOut(uint32_t msgId, int64_t abilityRecordId, bool isHalf)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "On timeout, msgId is %{public}d", msgId);
    std::lock_guard guard(managerLock_);
    auto abilityRecord = GetAbilityRecordById(abilityRecordId);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "MissionListManager on time out event: ability record is nullptr.");
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Ability timeout, name:%{public}s", abilityRecord->GetAbilityInfo().name.c_str());
    abilityRecord->RevokeUriPermission();

#ifdef SUPPORT_GRAPHICS
    if (abilityRecord->IsStartingWindow()) {
        PostCancelStartingWindowTask(abilityRecord);
    }
#endif

    PrintTimeOutLog(abilityRecord, msgId, isHalf);
    if (isHalf) {
        return;
    }
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            abilityRecord->SetLoading(false);
            HandleLoadTimeout(abilityRecord);
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            HandleForegroundTimeout(abilityRecord);
            break;
        default:
            break;
    }
}

void MissionListManager::HandleLoadTimeout(const std::shared_ptr<AbilityRecord> &ability)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "MissionListManager on time out event: ability record is nullptr.");
        return;
    }
    // root launcher load timeout, notify appMs force terminate the ability and restart immediately.
    if (ability->IsLauncherAbility() && ability->IsLauncherRoot()) {
        ability->SetRestarting(true);
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Launcher root load timeout, restart.");
        DelayedStartLauncher();
        return;
    }

    // other
    HandleTimeoutAndResumeAbility(ability);
}

void MissionListManager::HandleForegroundTimeout(const std::shared_ptr<AbilityRecord> &ability, AbilityState state)
{
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "MissionListManager on time out event: ability record is nullptr.");
        return;
    }

    if (ability->GetMission()) {
        ability->GetMission()->SetMovingState(false);
    }

    if (!ability->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "this ability is not foregrounding state.");
        return;
    }

    // root launcher load timeout, notify appMs force terminate the ability and restart immediately.
    if (ability->IsLauncherAbility() && ability->IsLauncherRoot()) {
        DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(ability->GetToken());
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Launcher root load timeout, restart.");
        DelayedStartLauncher();
        return;
    }

    // other
    HandleTimeoutAndResumeAbility(ability, state);
}

void MissionListManager::CompleteForegroundFailed(const std::shared_ptr<AbilityRecord> &abilityRecord,
    AbilityState state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CompleteForegroundFailed come, state: %{public}d.", static_cast<int32_t>(state));
    std::lock_guard guard(managerLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CompleteForegroundFailed, ability is nullptr.");
        return;
    }
    if (state == AbilityState::FOREGROUND_WINDOW_FREEZED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Window was freezed.");
        abilityRecord->SetPendingState(AbilityState::INITIAL);
        abilityRecord->SetAbilityState(AbilityState::BACKGROUND);
        DelayedSingleton<AppScheduler>::GetInstance()->MoveToBackground(abilityRecord->GetToken());
        TerminatePreviousAbility(abilityRecord);
        return;
    }
    if (state == AbilityState::FOREGROUND_DO_NOTHING) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ForegroundFailed. WMS return do_nothing");
        abilityRecord->SetAbilityState(AbilityState::FOREGROUND);
        MoveToBackgroundTask(abilityRecord);
        return;
    }
#ifdef SUPPORT_GRAPHICS
    if (state == AbilityState::FOREGROUND_INVALID_MODE) {
        abilityRecord->SetStartingWindow(false);
    }
    if (abilityRecord->IsStartingWindow()) {
        PostCancelStartingWindowTask(abilityRecord);
    }
#endif

    HandleForegroundTimeout(abilityRecord, state);
    TerminatePreviousAbility(abilityRecord);
    PostStartWaitingAbility();
}

void MissionListManager::HandleTimeoutAndResumeAbility(const std::shared_ptr<AbilityRecord> &timeOutAbilityRecord,
    AbilityState state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleTimeoutAndResumeTopAbility start");
    if (timeOutAbilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "LoadAndForeGroundCommon: timeOutAbilityRecord is nullptr.");
        return;
    }

    // complete mission list moving
    MoveToTerminateList(timeOutAbilityRecord);

    // load and foreground timeout, notify appMs force terminate the ability.
    DelayedSingleton<AppScheduler>::GetInstance()->AttachTimeOut(timeOutAbilityRecord->GetToken());

    // caller not exist or caller is service or timeout ability is launcher, back to launcher
    auto callerAbility = timeOutAbilityRecord->GetCallerRecord();
    if ((callerAbility == nullptr) ||
        (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::SERVICE) ||
        (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::EXTENSION) ||
        timeOutAbilityRecord->IsLauncherAbility() ||
        callerAbility->IsLauncherAbility()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability timeout, back to launcher.");
        DelayedStartLauncher();
        return;
    }

    if (state != AbilityState::FOREGROUND_INVALID_MODE) {
        DelayedResumeTimeout(callerAbility);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ok");
}

void MissionListManager::DelayedResumeTimeout(const std::shared_ptr<AbilityRecord> &callerAbility)
{
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetTaskHandler();
    CHECK_POINTER(handler);
    std::weak_ptr<MissionListManager> wpListMgr = shared_from_this();
    auto timeoutTask = [wpListMgr, callerAbility]() {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The caller ability need to resume.");
        auto listMgr = wpListMgr.lock();
        if (listMgr) {
            listMgr->BackToCaller(callerAbility);
        }
    };
    handler->SubmitTask(timeoutTask, "Caller_Restart");
}

void MissionListManager::BackToCaller(const std::shared_ptr<AbilityRecord> &callerAbility)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Back to Caller.");
    std::lock_guard guard(managerLock_);

    // caller is already the top ability and foregroundnew.
    auto topAbility = GetCurrentTopAbilityLocked();
    if (callerAbility == topAbility && topAbility->IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "caller is already the top ability and foregroundnew.");
        return;
    }

    // other , resume caller ability to top and foreground.
    MoveMissionToFrontInner(callerAbility->GetMissionId(), false, false, nullptr);
}

void MissionListManager::MoveToTerminateList(const std::shared_ptr<AbilityRecord>& abilityRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "timeout ability record is nullptr.");
        return;
    }
    auto missionList = abilityRecord->GetOwnedMissionList();
    if (missionList == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "timeout missionList is nullptr.");
        return;
    }
    auto selMission = abilityRecord->GetMission();
    if (selMission == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "timeout mission is nullptr.");
        return;
    }

    missionList->RemoveMission(selMission);
    if (missionList->GetType() == MissionListType::CURRENT && missionList->IsEmpty()) {
        RemoveMissionList(missionList);
    }

    // load timeout will not wait for died event, directly remove.
    if (abilityRecord->IsAbilityState(AbilityState::INITIAL)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "load timeout will not wait for died event, directly remove.");
        // update running state.
        InnerMissionInfo info;
        auto missionId = selMission->GetMissionId();
        if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, info) == 0) {
            info.missionInfo.runningState = -1;
            if (listenerController_) {
                listenerController_->NotifyMissionClosed(missionId);
            }
            DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
        }
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "success move timeout ability to terminate mission list.");
    // other remove to terminate list.
    abilityRecord->SetTerminatingState();
    terminateAbilityList_.push_back(abilityRecord);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "MoveToDefaultList end");
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByCaller(
    const std::shared_ptr<AbilityRecord> &caller, int requestCode)
{
    if (!caller) {
        return nullptr;
    }

    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (abilityRecord = missionList->GetAbilityRecordByCaller(caller, requestCode)) != nullptr) {
            return abilityRecord;
        }
    }

    if ((abilityRecord = defaultSingleList_->GetAbilityRecordByCaller(caller, requestCode)) != nullptr) {
        return abilityRecord;
    }

    return defaultStandardList_->GetAbilityRecordByCaller(caller, requestCode);
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordById(int64_t abilityRecordId) const
{
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (abilityRecord = missionList->GetAbilityRecordById(abilityRecordId)) != nullptr) {
            return abilityRecord;
        }
    }

    if ((abilityRecord = defaultSingleList_->GetAbilityRecordById(abilityRecordId)) != nullptr) {
        return abilityRecord;
    }

    return defaultStandardList_->GetAbilityRecordById(abilityRecordId);
}

void MissionListManager::OnAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord, int32_t currentUserId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "On ability died.");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnAbilityDied come, abilityRecord is nullptr.");
        return;
    }
    std::string element = abilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityDied come, ability is %{public}s", element.c_str());
    if (abilityRecord->GetAbilityInfo().type != AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability type is not page.");
        return;
    }

    std::lock_guard guard(managerLock_);
#ifdef SUPPORT_GRAPHICS
    if (abilityRecord->IsStartingWindow()) {
        PostCancelStartingWindowTask(abilityRecord);
    }
#endif

    if (abilityRecord->IsLauncherRoot() && currentUserId != userId_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "launcher root Ability died, state: INITIAL, %{public}d", __LINE__);
        abilityRecord->SetAbilityState(AbilityState::INITIAL);
        abilityRecord->SetRestarting(true);
        return;
    }

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    CHECK_POINTER_LOG(handler, "Get AbilityEventHandler failed.");
    if (abilityRecord->GetAbilityState() == AbilityState::INITIAL) {
        handler->RemoveEvent(AbilityManagerService::LOAD_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
        abilityRecord->SetLoading(false);
    }
    if (abilityRecord->GetAbilityState() == AbilityState::FOREGROUNDING) {
        handler->RemoveEvent(AbilityManagerService::FOREGROUND_TIMEOUT_MSG, abilityRecord->GetAbilityRecordId());
    }
    auto taskHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(taskHandler, "Fail to get AbilityTaskHandler.");
    if (abilityRecord->GetAbilityState() == AbilityState::BACKGROUNDING) {
        taskHandler->CancelTask("background_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    }
    if (abilityRecord->GetAbilityState() == AbilityState::TERMINATING) {
        taskHandler->CancelTask("terminate_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    }

    HandleAbilityDied(abilityRecord);
}

std::shared_ptr<MissionList> MissionListManager::GetTargetMissionList(int missionId, std::shared_ptr<Mission> &mission,
    bool &isReachToLimit)
{
    mission = GetMissionById(missionId);
    if (mission) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "get mission by id successfully, missionId: %{public}d", missionId);
        auto missionList = mission->GetMissionList();
        if (!missionList) {
            // mission is not null ptr, so its missionList ptr should be not null ptr too.
            TAG_LOGE(AAFwkTag::ABILITYMGR, "mission list ptr is null ptr");
            return nullptr;
        }

        auto missionType = missionList->GetType();
        std::shared_ptr<MissionList> targetMissionList = nullptr;
        switch (missionType) {
            case LAUNCHER:
                // not support move launcher to front.
                TAG_LOGE(AAFwkTag::ABILITYMGR, "get launcher mission list, missionId: %{public}d", missionId);
                break;
            case CURRENT:
                targetMissionList = mission->GetMissionList();
                break;
            case DEFAULT_STANDARD:
            case DEFAULT_SINGLE:
                // generate a new missionList
                targetMissionList = std::make_shared<MissionList>();
                break;
            default:
                TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid missionType: %{public}d", missionType);
        }
        return targetMissionList;
    }

    // cannot find mission, may reasons: system restart or mission removed by system.
    TAG_LOGI(AAFwkTag::ABILITYMGR, "cannot find mission missionId: %{public}d", missionId);

    InnerMissionInfo innerMissionInfo;
    int getMission = DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(
        missionId, innerMissionInfo);
    if (getMission != ERR_OK) {
        TAG_LOGE(
            AAFwkTag::ABILITYMGR, "cannot find mission info from MissionInfoList by missionId: %{public}d", missionId);
        return nullptr;
    }

    // generate a new mission and missionList
    AbilityRequest abilityRequest;
    int generateAbility = DelayedSingleton<AbilityManagerService>::GetInstance()->GenerateAbilityRequest(
        innerMissionInfo.missionInfo.want, DEFAULT_INVAL_VALUE, abilityRequest, nullptr, userId_);
    if (generateAbility != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "cannot find generate ability request, missionId: %{public}d", missionId);
        return nullptr;
    }

    if (CheckLimit()) {
        isReachToLimit = true;
        TAG_LOGE(AAFwkTag::ABILITYMGR, "already reach to limit, not create new mission list.");
        return nullptr;
    }

    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    mission = std::make_shared<Mission>(innerMissionInfo.missionInfo.id, abilityRecord, innerMissionInfo.missionName);
    abilityRecord->UpdateRecoveryInfo(innerMissionInfo.hasRecoverInfo);
    innerMissionInfo.hasRecoverInfo = false;
    mission->SetLockedState(innerMissionInfo.missionInfo.lockedState);
    mission->SetUnclearable(innerMissionInfo.missionInfo.unclearable);
    abilityRecord->SetMission(mission);
    abilityRecord->SetOwnerMissionUserId(userId_);
    SetLastExitReason(abilityRecord);
    std::shared_ptr<MissionList> newMissionList = std::make_shared<MissionList>();
    return newMissionList;
}

int32_t MissionListManager::GetMissionIdByAbilityToken(const sptr<IRemoteObject> &token)
{
    std::lock_guard guard(managerLock_);
    return GetMissionIdByAbilityTokenInner(token);
}

int32_t MissionListManager::GetMissionIdByAbilityTokenInner(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = GetAbilityRecordByTokenInner(token);
    if (!abilityRecord) {
        return -1;
    }
    auto mission = abilityRecord->GetMission();
    if (!mission) {
        return -1;
    }
    return mission->GetMissionId();
}

sptr<IRemoteObject> MissionListManager::GetAbilityTokenByMissionId(int32_t missionId)
{
    std::lock_guard guard(managerLock_);
    sptr<IRemoteObject> result = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (result = missionList->GetAbilityTokenByMissionId(missionId)) != nullptr) {
            return result;
        }
    }

    if ((result = defaultSingleList_->GetAbilityTokenByMissionId(missionId)) != nullptr) {
        return result;
    }

    return defaultStandardList_->GetAbilityTokenByMissionId((missionId));
}

void MissionListManager::PostStartWaitingAbility()
{
    auto self(shared_from_this());
    auto startWaitingAbilityTask = [self]() { self->StartWaitingAbility(); };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    CHECK_POINTER_LOG(handler, "Fail to get AbilityTaskHandler.");

    /* PostTask to trigger start Ability from waiting queue */
    handler->SubmitTask(startWaitingAbilityTask, "startWaitingAbility");
}

void MissionListManager::HandleAbilityDied(std::shared_ptr<AbilityRecord> abilityRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle Ability Died.");
    CHECK_POINTER(abilityRecord);

    if (abilityRecord->GetAbilityInfo().type != AbilityType::PAGE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Ability type is not page.");
        return;
    }

    if (abilityRecord->IsLauncherAbility()) {
        HandleLauncherDied(abilityRecord);
        return;
    }

    HandleAbilityDiedByDefault(abilityRecord);
}

void MissionListManager::HandleLauncherDied(std::shared_ptr<AbilityRecord> ability)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle launcher Ability Died.");
    auto mission = ability->GetMission();
    CHECK_POINTER_LOG(mission, "Fail to get launcher mission.");
    auto missionList = mission->GetMissionList();
    if (launcherList_ != missionList) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not launcher missionList.");
        return;
    }

    bool isForeground = ability->IsAbilityState(FOREGROUND) || ability->IsAbilityState(FOREGROUNDING);
    if (ability->IsLauncherRoot()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "launcher root Ability died, state: INITIAL");
        ability->SetAbilityState(AbilityState::INITIAL);
        ability->SetRestarting(true);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "launcher Ability died, remove");
        missionList->RemoveMission(mission);
    }
    if (isForeground) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "active launcher ability died, start launcher");
        DelayedStartLauncher();
    }
}

void MissionListManager::HandleAbilityDiedByDefault(std::shared_ptr<AbilityRecord> ability)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle Ability DiedByDefault.");
    CHECK_POINTER_LOG(ability, "ability is null.");
    if (ability->IsTerminating()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle Ability DiedByTerminating.");
        CompleteTerminateAndUpdateMission(ability);
        return;
    }

    auto mission = ability->GetMission();
    CHECK_POINTER_LOG(mission, "Fail to get mission.");
    auto missionList = mission->GetMissionList();
    CHECK_POINTER_LOG(missionList, "Fail to get mission list.");

    std::shared_ptr<AbilityRecord> launcherRoot = launcherList_->GetLauncherRoot();
    bool isLauncherActive = (launcherRoot &&
        (launcherRoot->IsAbilityState(FOREGROUND) || launcherRoot->IsAbilityState(FOREGROUNDING)));
    bool isForeground = ability->IsAbilityState(FOREGROUND) || ability->IsAbilityState(FOREGROUNDING);

    // remove from mission list.
    missionList->RemoveMission(mission);
    if (missionList->GetType() == MissionListType::CURRENT && missionList->IsEmpty()) {
        RemoveMissionList(missionList);
    }

    // update running state.
    auto missionId = mission->GetMissionId();
    if (!ability->IsUninstallAbility()) {
        if ((ability->GetAppIndex() != 0) || ability->GetAbilityInfo().removeMissionAfterTerminate ||
            ability->GetAbilityInfo().excludeFromMissions) {
            RemoveMissionLocked(missionId, ability->GetAbilityInfo().excludeFromMissions);
        } else {
            InnerMissionInfo info;
            if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, info) == 0) {
                info.missionInfo.runningState = -1;
                DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionInfo(info);
            }

            if (listenerController_) {
                listenerController_->NotifyMissionClosed(missionId);
            }
        }
    }

    // start launcher
    if (isForeground && !isLauncherActive) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "active ability died, start launcher later");
        DelayedStartLauncher();
    }
}

void MissionListManager::DelayedStartLauncher()
{
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetTaskHandler();
    CHECK_POINTER(handler);
    std::weak_ptr<MissionListManager> wpListMgr = shared_from_this();
    auto timeoutTask = [wpListMgr]() {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The launcher needs to be restarted.");
        auto listMgr = wpListMgr.lock();
        if (listMgr) {
            listMgr->BackToLauncher();
        }
    };
    handler->SubmitTask(timeoutTask, "Launcher_Restart");
}

void MissionListManager::BackToLauncher()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Back to launcher.");
    std::lock_guard guard(managerLock_);
    CHECK_POINTER(launcherList_);

    auto launcherRootAbility = launcherList_->GetLauncherRoot();
    if (!launcherRootAbility) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "no root launcher ability, no need back to launcher.");
        return;
    }

    if (launcherRootAbility->GetAbilityInfo().bundleName != AbilityConfig::LAUNCHER_BUNDLE_NAME) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not launcher mission, no need back to launcher.");
        return;
    }

    auto launcherRootMission = launcherRootAbility->GetMission();
    if (!launcherRootMission) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "no root launcher mission, no need back to launcher.");
        return;
    }

    std::queue<AbilityRequest> emptyQueue;
    std::swap(waitingAbilityQueue_, emptyQueue);

    launcherList_->AddMissionToTop(launcherRootMission);
    MoveMissionListToTop(launcherList_);
    launcherRootAbility->ProcessForegroundAbility(0);
}

int MissionListManager::SetMissionContinueState(const sptr<IRemoteObject> &token, const int32_t missionId,
    const AAFwk::ContinueState &state)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "SetMissionContinueState start. Mission id: %{public}d, state: %{public}d",
        missionId, state);
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "SetMissionContinueState token is nullptr. Mission id: %{public}d, state: %{public}d", missionId, state);
        return -1;
    }

    return DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionContinueState(missionId, state);
}

#ifdef SUPPORT_GRAPHICS
int MissionListManager::SetMissionLabel(const sptr<IRemoteObject> &token, const std::string &label)
{
    if (!token) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "SetMissionLabel token is nullptr.");
        return -1;
    }

    auto missionId = GetMissionIdByAbilityToken(token);
    if (missionId <= 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "SetMissionLabel find mission failed.");
        return -1;
    }

    // store label if not notify mission created.
    auto abilityRecord = GetAbilityRecordByToken(token);
    if (abilityRecord) {
        auto mission = abilityRecord->GetMission();
        if (mission && mission->NeedNotify()) {
            mission->SetNeedNotifyUpdateLabel(true);
        }
    }

    auto ret = DelayedSingleton<MissionInfoMgr>::GetInstance()->UpdateMissionLabel(missionId, label);
    if (ret == 0 && listenerController_) {
        listenerController_->NotifyMissionLabelUpdated(missionId);
    }

    return ret;
}

int MissionListManager::SetMissionIcon(const sptr<IRemoteObject> &token, const std::shared_ptr<Media::PixelMap> &icon)
{
    if (!token) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "SetMissionIcon token is nullptr.");
        return -1;
    }

    std::lock_guard guard(managerLock_);
    auto missionId = GetMissionIdByAbilityTokenInner(token);
    if (missionId <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon find mission failed.");
        return -1;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(token);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon find ability failed.");
        return -1;
    }

    if (listenerController_ && !(abilityRecord->GetAbilityInfo().excludeFromMissions)) {
        listenerController_->NotifyMissionIconChanged(missionId, icon);
    }

    return 0;
}

void MissionListManager::CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) const
{
    FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, TRACE_ATOMIC_SERVICE, TRACE_ATOMIC_SERVICE_ID);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CompleteFirstFrameDrawing called.");
    if (!abilityToken) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s ability token is nullptr.", __func__);
        return;
    }

    auto abilityRecord = GetAbilityRecordByToken(abilityToken);
    if (!abilityRecord) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s get AbilityRecord by token failed.", __func__);
        return;
    }

    if (abilityRecord->IsCompleteFirstFrameDrawing()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "First frame drawing has completed.");
        return;
    }
    abilityRecord->SetCompleteFirstFrameDrawing(true);

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to get Ability task handler.");
        return;
    }

    auto task = [owner = weak_from_this(), abilityRecord] {
        auto mgr = owner.lock();
        if (mgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "MissionListManager is nullptr.");
            return;
        }
        mgr->NotifyMissionCreated(abilityRecord);
        if (DelayedSingleton<AbilityManagerService>::GetInstance()->IsDmsAlive()) {
            mgr->UpdateMissionSnapshot(abilityRecord);
        }
    };
    handler->SubmitTask(task, "FirstFrameDrawing");
    auto preloadTask = [owner = weak_from_this(), abilityRecord] {
        auto mgr = owner.lock();
        if (mgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "MissionListManager is nullptr.");
            return;
        }
        mgr->ProcessPreload(abilityRecord);
    };
    handler->SubmitTask(preloadTask);
}

void MissionListManager::ProcessPreload(const std::shared_ptr<AbilityRecord> &record) const
{
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER(bundleMgrHelper);
    auto abilityInfo = record->GetAbilityInfo();
    Want want;
    want.SetElementName(abilityInfo.deviceId, abilityInfo.bundleName, abilityInfo.name, abilityInfo.moduleName);
    auto uid = record->GetUid();
    want.SetParam("uid", uid);
    bundleMgrHelper->ProcessPreload(want);
}

Closure MissionListManager::GetCancelStartingWindowTask(const std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    auto windowHandler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetWMSHandler();
    if (!windowHandler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, Get WMS handler failed.", __func__);
        return nullptr;
    }

    return [windowHandler, abilityRecord] {
        if (windowHandler && abilityRecord && abilityRecord->IsStartingWindow()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s, call windowHandler CancelStartingWindow.", __func__);
            windowHandler->CancelStartingWindow(abilityRecord->GetToken());
            abilityRecord->SetStartingWindow(false);
        }
    };
}

void MissionListManager::PostCancelStartingWindowTask(const std::shared_ptr<AbilityRecord> &abilityRecord) const
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to get AbilityTaskHandler.");
        return;
    }

    auto task = GetCancelStartingWindowTask(abilityRecord);
    if (!task) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Fail to get CancelStartingWindow task.");
        return;
    }
    handler->SubmitTask(task, TaskQoS::USER_INTERACTIVE);
}

void MissionListManager::InitPrepareTerminateConfig()
{
    char value[PREPARE_TERMINATE_ENABLE_SIZE] = "false";
    int retSysParam = GetParameter(PREPARE_TERMINATE_ENABLE_PARAMETER, "false", value, PREPARE_TERMINATE_ENABLE_SIZE);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CheckPrepareTerminateEnable, %{public}s value is %{public}s.",
        PREPARE_TERMINATE_ENABLE_PARAMETER, value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        isPrepareTerminateEnable_ = true;
    }
}
#endif

void MissionListManager::Dump(std::vector<std::string> &info)
{
    std::lock_guard guard(managerLock_);
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    dumpInfo = " current mission lists:{";
    info.push_back(dumpInfo);
    for (const auto& missionList : currentMissionLists_) {
        if (missionList) {
            missionList->Dump(info);
        }
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " default stand mission list:{";
    info.push_back(dumpInfo);
    if (defaultStandardList_) {
        defaultStandardList_->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " default single mission list:{";
    info.push_back(dumpInfo);
    if (defaultSingleList_) {
        defaultSingleList_->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);

    dumpInfo = " launcher mission list:{";
    info.push_back(dumpInfo);
    if (launcherList_) {
        launcherList_->Dump(info);
    }
    dumpInfo = " }";
    info.push_back(dumpInfo);
}

void MissionListManager::DumpMissionListByRecordId(
    std::vector<std::string> &info, bool isClient, int32_t abilityRecordId, const std::vector<std::string> &params)
{
    std::list<std::unique_ptr<MissionList>> currentMissionListsBackup;
    std::unique_ptr<MissionList> defaultStandardListBackup;
    std::unique_ptr<MissionList> defaultSingleListBackup;
    std::unique_ptr<MissionList> launcherListBackup;
    {
        std::lock_guard guard(managerLock_);
        for (const auto& missionList : currentMissionLists_) {
            if (missionList != nullptr) {
                currentMissionListsBackup.emplace_back(std::make_unique<MissionList>(*missionList));
            }
        }
        defaultStandardListBackup = std::make_unique<MissionList>(*defaultStandardList_);
        defaultSingleListBackup = std::make_unique<MissionList>(*defaultSingleList_);
        launcherListBackup = std::make_unique<MissionList>(*launcherList_);
    }

    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    for (const auto& missionList : currentMissionListsBackup) {
        if (missionList && missionList != launcherListBackup) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "missionList");
            missionList->DumpStateByRecordId(info, isClient, abilityRecordId, params);
        }
    }

    if (defaultStandardListBackup) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "defaultStandardList");
        defaultStandardListBackup->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    }

    if (defaultSingleListBackup) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "defaultSingleList");
        defaultSingleListBackup->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    }

    if (launcherListBackup) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "launcherList");
        launcherListBackup->DumpStateByRecordId(info, isClient, abilityRecordId, params);
    }
}

void MissionListManager::DumpMissionList(std::vector<std::string> &info, bool isClient, const std::string &args)
{
    std::list<std::unique_ptr<MissionList>> currentMissionListsBackup;
    std::unique_ptr<MissionList> defaultStandardListBackup;
    std::unique_ptr<MissionList> defaultSingleListBackup;
    std::unique_ptr<MissionList> launcherListBackup;
    {
        std::lock_guard guard(managerLock_);
        for (const auto& missionList : currentMissionLists_) {
            if (missionList != nullptr) {
                currentMissionListsBackup.emplace_back(std::make_unique<MissionList>(*missionList));
            }
        }
        defaultStandardListBackup = std::make_unique<MissionList>(*defaultStandardList_);
        defaultSingleListBackup = std::make_unique<MissionList>(*defaultSingleList_);
        launcherListBackup = std::make_unique<MissionList>(*launcherList_);
    }

    if (args.size() != 0 &&
        args != "NORMAL" &&
        args != "DEFAULT_STANDARD" &&
        args != "DEFAULT_SINGLE" &&
        args != "LAUNCHER") {
        info.emplace_back("MissionList Type NORMAL|DEFAULT_STANDARD|DEFAULT_SINGLE|LAUNCHER");
        return;
    }

    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    if (args.size() == 0 || args == "NORMAL") {
        dumpInfo = "  Current mission lists:";
        info.push_back(dumpInfo);
        for (const auto& missionList : currentMissionListsBackup) {
            if (missionList) {
                missionList->DumpList(info, isClient);
            }
        }
    }

    if (args.size() == 0 || args == "DEFAULT_STANDARD") {
        dumpInfo = "  default stand mission list:";
        info.push_back(dumpInfo);
        defaultStandardListBackup->DumpList(info, isClient);
    }

    if (args.size() == 0 || args == "DEFAULT_SINGLE") {
        dumpInfo = "  default single mission list:";
        info.push_back(dumpInfo);
        defaultSingleListBackup->DumpList(info, isClient);
    }
    if (args.size() == 0 || args == "LAUNCHER") {
        dumpInfo = "  launcher mission list:";
        info.push_back(dumpInfo);
        launcherListBackup->DumpList(info, isClient);
    }
}

void MissionListManager::DumpMissionInfos(std::vector<std::string> &info)
{
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    DelayedSingleton<MissionInfoMgr>::GetInstance()->Dump(info);
}

void MissionListManager::DumpMission(int missionId, std::vector<std::string> &info)
{
    std::string dumpInfo = "User ID #" + std::to_string(userId_);
    info.push_back(dumpInfo);
    InnerMissionInfo innerMissionInfo;
    if (DelayedSingleton<MissionInfoMgr>::GetInstance()->GetInnerMissionInfoById(missionId, innerMissionInfo) != 0) {
        info.push_back("error: invalid mission number, please see 'aa dump --mission-list'.");
        return;
    }
    innerMissionInfo.Dump(info);
}

int MissionListManager::ResolveLocked(const AbilityRequest &abilityRequest)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ability_name:%{public}s", abilityRequest.want.GetElement().GetURI().c_str());

    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, resolve ability_name:", __func__);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    return CallAbilityLocked(abilityRequest);
}

bool MissionListManager::IsAbilityStarted(AbilityRequest &abilityRequest,
    std::shared_ptr<AbilityRecord> &targetRecord)
{
    std::shared_ptr<Mission> targetMission;

    return HandleReusedMissionAndAbility(abilityRequest, targetMission, targetRecord);
}

int MissionListManager::CallAbilityLocked(const AbilityRequest &abilityRequest)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call ability.");
    std::lock_guard guard(managerLock_);

    // allow to start ability by called type without loading ui.
    if (!abilityRequest.IsCallType(AbilityCallType::CALL_REQUEST_TYPE)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "start ability not by call.");
        return ERR_INVALID_VALUE;
    }

    // Get target mission and ability record.
    std::shared_ptr<AbilityRecord> targetAbilityRecord;
    std::shared_ptr<Mission> targetMission;
    bool isReachToLimit = false;
    GetTargetMissionAndAbility(abilityRequest, targetMission, targetAbilityRecord, isReachToLimit);
    if (isReachToLimit) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get mission or record, already reach to limit.");
        return ERR_REACH_UPPER_LIMIT;
    }
    if (!targetMission || !targetAbilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get mission or record.");
        return ERR_INVALID_VALUE;
    }

    targetAbilityRecord->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode);
    targetAbilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CALL);

    // mission is first created, add mission to default call mission list.
    // other keep in current mission list.
    if (!targetMission->GetMissionList()) {
        if (abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SINGLETON) {
            defaultSingleList_->AddMissionToTop(targetMission);
        } else {
            defaultStandardList_->AddMissionToTop(targetMission);
        }
    }

    NotifyAbilityToken(targetAbilityRecord->GetToken(), abilityRequest);

    // new version started by call type
    auto ret = ResolveAbility(targetAbilityRecord, abilityRequest);
    if (ret == ResolveResultType::OK_HAS_REMOTE_OBJ) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "target ability has been resolved.");
        if (targetAbilityRecord->GetWant().GetBoolParam(Want::PARAM_RESV_CALL_TO_FOREGROUND, false)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "target ability needs to be switched to foreground.");
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(targetAbilityRecord->GetToken());
        }
        return ERR_OK;
    } else if (ret == ResolveResultType::NG_INNER_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resolve failed, error: %{public}d.", RESOLVE_CALL_ABILITY_INNER_ERR);
        return RESOLVE_CALL_ABILITY_INNER_ERR;
    }

    // schedule target ability
    std::string element = targetAbilityRecord->GetElementName().GetURI();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "load ability record: %{public}s", element.c_str());

    // flag the first ability.
    auto currentTopAbility = GetCurrentTopAbilityLocked();
    if (!currentTopAbility) {
        if (targetAbilityRecord->GetAbilityInfo().applicationInfo.isLauncherApp) {
            targetAbilityRecord->SetLauncherRoot();
        }
    }

    return targetAbilityRecord->LoadAbility();
}

int MissionListManager::ReleaseCallLocked(
    const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "release call ability.");

    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    std::lock_guard guard(managerLock_);

    auto abilityRecords = GetAbilityRecordsByName(element);
    auto isExist = [connect] (const std::shared_ptr<AbilityRecord> &abilityRecord) {
        return abilityRecord->IsExistConnection(connect);
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found ability record by callback!");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER_AND_RETURN(abilityRecord, RELEASE_CALL_ABILITY_INNER_ERR);

    if (!abilityRecord->ReleaseCall(connect)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability release call record failed!");
        return RELEASE_CALL_ABILITY_INNER_ERR;
    }
    return ERR_OK;
}

int MissionListManager::ResolveAbility(
    const std::shared_ptr<AbilityRecord> &targetAbility, const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbilityRecord resolve call record.");
    CHECK_POINTER_AND_RETURN(targetAbility, ResolveResultType::NG_INNER_ERROR);

    ResolveResultType result = targetAbility->Resolve(abilityRequest);
    switch (result) {
        case ResolveResultType::NG_INNER_ERROR:
        case ResolveResultType::OK_HAS_REMOTE_OBJ:
            return result;
        default:
            break;
    }

    if (targetAbility->IsReady()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbility is ready, directly scheduler call request.");
        targetAbility->CallRequest();
        return ResolveResultType::OK_HAS_REMOTE_OBJ;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "targetAbility need to call request after lifecycle.");
    return result;
}

std::shared_ptr<AbilityRecord> MissionListManager::GetAbilityRecordByName(const AppExecFwk::ElementName &element)
{
    // find in currentMissionLists_
    for (auto missionList : currentMissionLists_) {
        if (missionList != nullptr) {
            auto ability = missionList->GetAbilityRecordByName(element);
            if (ability != nullptr) {
                return ability;
            }
        }
    }

    // find in launcherMissionList_
    auto ability = launcherList_->GetAbilityRecordByName(element);
    if (ability != nullptr) {
        return ability;
    }

    // find in default singlelist_
    return defaultSingleList_->GetAbilityRecordByName(element);
}

std::vector<std::shared_ptr<AbilityRecord>> MissionListManager::GetAbilityRecordsByName(
    const AppExecFwk::ElementName &element)
{
    std::vector<std::shared_ptr<AbilityRecord>> records;
    for (auto missionList : currentMissionLists_) {
        if (missionList != nullptr) {
            missionList->GetAbilityRecordsByName(element, records);
        }
    }

    // find in launcherMissionList_
    if (launcherList_ != nullptr) {
        launcherList_->GetAbilityRecordsByName(element, records);
    }

    // find in defaultStandardList_
    if (defaultStandardList_ != nullptr) {
        defaultStandardList_->GetAbilityRecordsByName(element, records);
    }

    if (!records.empty()) {
        return records;
    }

    // find in default singlelist_
    if (defaultSingleList_ != nullptr) {
        defaultSingleList_->GetAbilityRecordsByName(element, records);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "records is %{public}s.", records.empty() ? "empty" : "not empty");
    return records;
}

void MissionListManager::OnCallConnectDied(const std::shared_ptr<CallRecord> &callRecord)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "On callConnect died.");
    CHECK_POINTER(callRecord);
    std::lock_guard guard(managerLock_);

    AppExecFwk::ElementName element = callRecord->GetTargetServiceName();
    auto abilityRecords = GetAbilityRecordsByName(element);
    auto isExist = [callRecord] (const std::shared_ptr<AbilityRecord> &abilityRecord) {
        return abilityRecord->IsExistConnection(callRecord->GetConCallBack());
    };
    auto findRecord = std::find_if(abilityRecords.begin(), abilityRecords.end(), isExist);
    if (findRecord == abilityRecords.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not found ability record by callback");
        return;
    }
    auto abilityRecord = *findRecord;
    CHECK_POINTER(abilityRecord);
    abilityRecord->ReleaseCall(callRecord->GetConCallBack());
}
void MissionListManager::OnAcceptWantResponse(const AAFwk::Want &want, const std::string &flag)
{
    std::lock_guard guard(managerLock_);
    if (waitingAbilityQueue_.empty()) {
        return;
    }

    AbilityRequest abilityRequest = waitingAbilityQueue_.front();
    waitingAbilityQueue_.pop();

    auto currentTopAbility = GetCurrentTopAbilityLocked();
    auto callerAbility = GetAbilityRecordByTokenInner(abilityRequest.callerToken);

    if (!flag.empty()) {
        auto mission = GetMissionBySpecifiedFlag(want, flag);
        if (mission) {
            auto ability = mission->GetAbilityRecord();
            if (!ability) {
                return;
            }
            ability->SetWant(abilityRequest.want);
            ability->SetIsNewWant(true);
            UpdateAbilityRecordLaunchReason(abilityRequest, ability);
            if (callerAbility == nullptr) {
                callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
            }
            auto isCallerFromLauncher = (callerAbility && callerAbility->IsLauncherAbility());
            MoveMissionToFrontInner(mission->GetMissionId(), isCallerFromLauncher, false, callerAbility);
            NotifyRestartSpecifiedAbility(abilityRequest, ability->GetToken());
            return;
        }
    }

    abilityRequest.specifiedFlag = flag;
    NotifyStartSpecifiedAbility(abilityRequest, want);
    StartAbilityLocked(currentTopAbility, callerAbility, abilityRequest);
}

void MissionListManager::NotifyRestartSpecifiedAbility(AbilityRequest &request, const sptr<IRemoteObject> &token)
{
    if (request.abilityInfoCallback == nullptr) {
        return;
    }
    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (request.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s called.", __func__);
        abilityInfoCallback->NotifyRestartSpecifiedAbility(token);
    }
}

void MissionListManager::NotifyStartSpecifiedAbility(AbilityRequest &abilityRequest, const AAFwk::Want &want)
{
    if (abilityRequest.abilityInfoCallback == nullptr) {
        return;
    }

    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        Want newWant = want;
        int32_t type = static_cast<int32_t>(abilityRequest.abilityInfo.type);
        newWant.SetParam("abilityType", type);
        sptr<Want> extraParam = new (std::nothrow) Want();
        abilityInfoCallback->NotifyStartSpecifiedAbility(abilityRequest.callerToken, newWant,
            abilityRequest.requestCode, extraParam);
        int32_t procCode = extraParam->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
        if (procCode != 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_REQUEST_PROC_CODE, procCode);
        }
        int32_t tokenCode = extraParam->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0);
        if (tokenCode != 0) {
            abilityRequest.want.SetParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, tokenCode);
        }
    }
}

void MissionListManager::OnStartSpecifiedAbilityTimeoutResponse(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s called.", __func__);
    std::lock_guard guard(managerLock_);
    if (waitingAbilityQueue_.empty()) {
        return;
    }
    waitingAbilityQueue_.pop();

    if (waitingAbilityQueue_.empty()) {
        return;
    }
    AbilityRequest abilityRequest = waitingAbilityQueue_.front();
    waitingAbilityQueue_.pop();

    auto currentTopAbility = GetCurrentTopAbilityLocked();
    auto callerAbility = GetAbilityRecordByTokenInner(abilityRequest.callerToken);
    if (StartAbility(currentTopAbility, callerAbility, abilityRequest) == 0 && !abilityRequest.abilityInfo.visible) {
        SendKeyEvent(abilityRequest);
    }
}

std::shared_ptr<Mission> MissionListManager::GetMissionBySpecifiedFlag(
    const AAFwk::Want &want, const std::string &flag) const
{
    std::shared_ptr<Mission> mission = nullptr;
    for (auto missionList : currentMissionLists_) {
        if (missionList && (mission = missionList->GetMissionBySpecifiedFlag(want, flag)) != nullptr) {
            return mission;
        }
    }

    if ((mission = defaultSingleList_->GetMissionBySpecifiedFlag(want, flag)) != nullptr) {
        return mission;
    }

    if ((mission = launcherList_->GetMissionBySpecifiedFlag(want, flag)) != nullptr) {
        return mission;
    }

    return defaultStandardList_->GetMissionBySpecifiedFlag(want, flag);
}

bool MissionListManager::CheckSingleLimit(const AbilityRequest &abilityRequest)
{
    auto reUsedMission = GetReusedMission(abilityRequest);
    if (!reUsedMission) {
        bool isSingleMaxLimit = IsReachToSingleLimitLocked(abilityRequest.uid);
        if (isSingleMaxLimit) {
            return true;
        }
    }
    return false;
}

bool MissionListManager::CheckLimit()
{
    bool isAllMaxLimit = IsReachToLimitLocked();
    if (isAllMaxLimit) {
        auto earliestMission = FindEarliestMission();
        if (earliestMission) {
            if (TerminateAbilityInner(earliestMission->GetAbilityRecord(), DEFAULT_INVAL_VALUE,
                nullptr, true) != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR,
                    "already reach limit instance. limit: %{public}d, and terminate earliestAbility failed.",
                    MAX_INSTANCE_COUNT);
                return true;
            }
            if (IsAppLastAbility(earliestMission->GetAbilityRecord())) {
                ExitReason exitReason = { REASON_RESOURCE_CONTROL,
                    "Already reach ability max limit, terminate earliest ability." };
                OHOS::DelayedSingleton<AbilityManagerService>::GetInstance()->RecordAppExitReason(exitReason);
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR,
                "already reach limit instance. limit: %{public}d, and terminate earliestAbility success.",
                MAX_INSTANCE_COUNT);
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "current is not reach limit instance.");
    return false;
}

bool MissionListManager::IsReachToLimitLocked() const
{
    auto missionCount = GetMissionCount();
    if (missionCount >= MAX_INSTANCE_COUNT) {
        return true;
    }
    return false;
}

bool MissionListManager::IsReachToSingleLimitLocked(const int32_t uid) const
{
    int32_t singleAppMissionCount = 0;
    for (const auto& missionList : currentMissionLists_) {
        if (!missionList) {
            continue;
        }
        singleAppMissionCount += missionList->GetMissionCountByUid(uid);
        if (singleAppMissionCount >= SINGLE_MAX_INSTANCE_COUNT) {
            return true;
        }
    }
    singleAppMissionCount += defaultStandardList_->GetMissionCountByUid(uid);
    if (singleAppMissionCount >= SINGLE_MAX_INSTANCE_COUNT) {
        return true;
    }
    singleAppMissionCount += defaultSingleList_->GetMissionCountByUid(uid);
    if (singleAppMissionCount >= SINGLE_MAX_INSTANCE_COUNT) {
        return true;
    }
    return false;
}

bool MissionListManager::MissionDmInitCallback::isInit_ = false;
void MissionListManager::MissionDmInitCallback::OnRemoteDied()
{
    isInit_ = false;
    TAG_LOGW(AAFwkTag::ABILITYMGR, "DeviceManager died.");
}

void MissionListManager::RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler)
{
    DelayedSingleton<MissionInfoMgr>::GetInstance()->RegisterSnapshotHandler(handler);
}

bool MissionListManager::GetMissionSnapshot(int32_t missionId, const sptr<IRemoteObject>& abilityToken,
    MissionSnapshot& missionSnapshot, bool isLowResolution)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "snapshot: Start get mission snapshot.");
    bool forceSnapshot = false;
    {
        std::lock_guard guard(managerLock_);
        auto abilityRecord = GetAbilityRecordByTokenInner(abilityToken);
        if (abilityRecord && abilityRecord->IsAbilityState(FOREGROUND)) {
            forceSnapshot = true;
            missionSnapshot.isPrivate = (abilityRecord->GetAppIndex() != 0);
        }
    }
    return DelayedSingleton<MissionInfoMgr>::GetInstance()->GetMissionSnapshot(
        missionId, abilityToken, missionSnapshot, isLowResolution, forceSnapshot);
}

void MissionListManager::GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm)
{
    std::lock_guard guard(managerLock_);

    auto func = [&info, isPerm](const std::shared_ptr<Mission> &mission) {
        if (!mission) {
            return;
        }

        auto ability = mission->GetAbilityRecord();
        if (!ability) {
            return;
        }

        if (isPerm) {
            DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, ability);
        } else {
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenID = ability->GetApplicationInfo().accessTokenId;
            if (callingTokenId == tokenID) {
                DelayedSingleton<AbilityManagerService>::GetInstance()->GetAbilityRunningInfo(info, ability);
            }
        }
    };
    if (!(defaultStandardList_->GetAllMissions().empty())) {
        auto list = defaultStandardList_->GetAllMissions();
        std::for_each(list.begin(), list.end(), func);
    }
    if (!(defaultSingleList_->GetAllMissions().empty())) {
        auto list = defaultSingleList_->GetAllMissions();
        std::for_each(list.begin(), list.end(), func);
    }
    for (auto missionList : currentMissionLists_) {
        if (!(missionList->GetAllMissions().empty())) {
            auto list = missionList->GetAllMissions();
            std::for_each(list.begin(), list.end(), func);
        }
    }
}

void MissionListManager::UninstallApp(const std::string &bundleName, int32_t uid)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Uninstall app, bundleName: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
    auto abilityManagerService = DelayedSingleton<AbilityManagerService>::GetInstance();
    CHECK_POINTER(abilityManagerService);
    auto handler = abilityManagerService->GetTaskHandler();
    CHECK_POINTER(handler);
    std::weak_ptr<MissionListManager> wpMgr = shared_from_this();
    auto task = [wpMgr, bundleName, uid]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle Uninstall app, bundleName: %{public}s, uid:%{public}d",
            bundleName.c_str(), uid);
        auto mgr = wpMgr.lock();
        if (mgr) {
            mgr->AddUninstallTags(bundleName, uid);
        }
    };
    handler->SubmitTask(task);
}

void MissionListManager::AddUninstallTags(const std::string &bundleName, int32_t uid)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AddUninstallTags, bundleName: %{public}s, uid:%{public}d", bundleName.c_str(), uid);
    std::lock_guard guard(managerLock_);
    for (auto it = currentMissionLists_.begin(); it != currentMissionLists_.end();) {
        auto missionList = *it;
        if (missionList) {
            missionList->HandleUnInstallApp(bundleName, uid); // add tag here.
            if (missionList->IsEmpty()) {
                currentMissionLists_.erase(it++);
                continue;
            }
        }
        it++;
    }
    defaultSingleList_->HandleUnInstallApp(bundleName, uid);
    defaultStandardList_->HandleUnInstallApp(bundleName, uid);
    std::list<int32_t> matchedMissions;
    DelayedSingleton<MissionInfoMgr>::GetInstance()->HandleUnInstallApp(bundleName, uid, matchedMissions);
    if (listenerController_) {
        listenerController_->HandleUnInstallApp(matchedMissions);
    }

    EraseWaitingAbility(bundleName, uid);
}

void MissionListManager::EraseWaitingAbility(const std::string &bundleName, int32_t uid)
{
    std::queue<AbilityRequest> abilityQueue;
    waitingAbilityQueue_.swap(abilityQueue);
    while (!abilityQueue.empty()) {
        AbilityRequest tempAbilityRequest = abilityQueue.front();
        abilityQueue.pop();
        if (tempAbilityRequest.abilityInfo.bundleName == bundleName && tempAbilityRequest.uid == uid) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "AddUninstallTags, erase AbilityRequest from waitingAbilityQueue.");
        } else {
            waitingAbilityQueue_.push(tempAbilityRequest);
        }
    }
}

bool MissionListManager::IsStarted()
{
    std::lock_guard guard(managerLock_);
    auto launcherRoot = launcherList_->GetLauncherRoot();
    return launcherRoot != nullptr;
}

void MissionListManager::PauseManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "MissionListManager PauseManager. move foreground to background.");
    std::lock_guard guard(managerLock_);
    std::list<std::shared_ptr<AbilityRecord>> foregroundAbilities;
    GetAllForegroundAbilities(foregroundAbilities);

    for (auto& abilityRecord : foregroundAbilities) {
        if (!abilityRecord) {
            continue;
        }
        abilityRecord->SetSwitchingPause(true);
        MoveToBackgroundTask(abilityRecord);
    }
}

void MissionListManager::ResumeManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ResumeManager, back to launcher.");
    BackToLauncher();
}

void MissionListManager::GetAllForegroundAbilities(std::list<std::shared_ptr<AbilityRecord>>& foregroundList)
{
    for (auto& missionList : currentMissionLists_) {
        GetForegroundAbilities(missionList, foregroundList);
    }
    GetForegroundAbilities(defaultSingleList_, foregroundList);
    GetForegroundAbilities(defaultStandardList_, foregroundList);
}

void MissionListManager::GetForegroundAbilities(const std::shared_ptr<MissionList>& missionList,
    std::list<std::shared_ptr<AbilityRecord>>& foregroundList)
{
    if (!missionList || missionList->IsEmpty()) {
        return;
    }

    for (auto& mission : missionList->GetAllMissions()) {
        if (!mission) {
            continue;
        }

        auto abilityRecord = mission->GetAbilityRecord();
        if (!abilityRecord) {
            continue;
        }

        if (abilityRecord->IsActiveState()) {
            foregroundList.emplace_back(abilityRecord);
        }
    }
}

void MissionListManager::RemoveMissionLocked(int32_t missionId, bool excludeFromMissions)
{
    if (missionId <= 0) {
        return;
    }

    DelayedSingleton<MissionInfoMgr>::GetInstance()->DeleteMissionInfo(missionId);
    if (listenerController_ && !excludeFromMissions) {
        listenerController_->NotifyMissionDestroyed(missionId);
    }
}

bool MissionListManager::IsExcludeFromMissions(const std::shared_ptr<Mission> &mission)
{
    if (!mission) {
        return false;
    }

    auto abilityRecord = mission->GetAbilityRecord();
    return abilityRecord && abilityRecord->GetAbilityInfo().excludeFromMissions;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int MissionListManager::BlockAbility(int32_t abilityRecordId)
{
    int ret = -1;
    for (const auto &missionList : currentMissionLists_) {
        if (missionList && missionList != launcherList_) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "missionList begin to call BlockAbilityByRecordId %{public}s", __func__);
            if (missionList->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "missionList call BlockAbilityByRecordId success");
                ret = ERR_OK;
            }
        }
    }

    if (defaultStandardList_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "defaultStandardList begin to call BlockAbilityByRecordId %{public}s", __func__);
        if (defaultStandardList_->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "defaultStandardList call BlockAbilityByRecordId success");
            ret = ERR_OK;
        }
    }

    if (defaultSingleList_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "defaultSingleList begin to call BlockAbilityByRecordId %{public}s", __func__);
        if (defaultSingleList_->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "defaultSingleList_ call BlockAbilityByRecordId success");
            ret = ERR_OK;
        }
    }

    if (launcherList_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "launcherList begin to call BlockAbilityByRecordId %{public}s", __func__);
        if (launcherList_->BlockAbilityByRecordId(abilityRecordId) == ERR_OK) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "launcherList_ call BlockAbilityByRecordId success");
            ret = ERR_OK;
        }
    }
    return ret;
}
#endif

void MissionListManager::SetMissionANRStateByTokens(const std::vector<sptr<IRemoteObject>> &tokens)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    for (auto &item : tokens) {
        auto abilityRecord = GetAbilityRecordByToken(item);
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
            continue;
        }
        auto mission = abilityRecord->GetMission();
        if (mission == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "mission is nullptr.");
            continue;
        }
        mission->SetANRState(true);
    }
}

int32_t MissionListManager::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    constexpr int32_t searchCount = 20;
    auto callerUid = IPCSkeleton::GetCallingUid();
    auto missionInfoMgr = DelayedSingleton<MissionInfoMgr>::GetInstance();
    if (missionInfoMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionInfoMgr is nullptr.");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard guard(managerLock_);
    for (auto i = 0; i < searchCount && i < static_cast<int32_t>(missionIds.size()); ++i) {
        MissionValidResult missionResult = {};
        missionResult.missionId = missionIds.at(i);
        InnerMissionInfo info;
        if (missionInfoMgr->GetInnerMissionInfoById(missionResult.missionId, info) != ERR_OK) {
            results.push_back(missionResult);
            continue;
        }

        if (callerUid != info.uid) {
            results.push_back(missionResult);
            continue;
        }

        missionResult.isValid = true;
        results.push_back(missionResult);
    }

    return ERR_OK;
}

bool MissionListManager::UpdateAbilityRecordLaunchReason(
    const AbilityRequest &abilityRequest, std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "input record is nullptr.");
        return false;
    }

    if (abilityRequest.IsContinuation()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_CONTINUATION);
        return true;
    }

    if (abilityRequest.IsAppRecovery() || abilityRecord->GetRecoveryInfo()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_APP_RECOVERY);
        return true;
    }

    if (abilityRequest.IsAcquireShareData()) {
        abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_SHARE);
        return true;
    }

    abilityRecord->SetLaunchReason(LaunchReason::LAUNCHREASON_START_ABILITY);
    return true;
}

void MissionListManager::NotifyMissionFocused(const int32_t missionId)
{
    if (listenerController_) {
        listenerController_->NotifyMissionFocused(missionId);
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "listener controller is null");
    }
}

void MissionListManager::NotifyMissionUnfocused(const int32_t missionId)
{
    if (listenerController_) {
        listenerController_->NotifyMissionUnfocused(missionId);
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "listener controller is null");
    }
}

void MissionListManager::NotifyAbilityToken(const sptr<IRemoteObject> &token, const AbilityRequest &abilityRequest)
{
    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyAbilityToken(token, abilityRequest.want);
    }
}

void MissionListManager::NotifyStartAbilityResult(const AbilityRequest &abilityRequest, int result)
{
    sptr<AppExecFwk::IAbilityInfoCallback> abilityInfoCallback
        = iface_cast<AppExecFwk::IAbilityInfoCallback> (abilityRequest.abilityInfoCallback);
    if (abilityInfoCallback != nullptr) {
        abilityInfoCallback->NotifyStartAbilityResult(abilityRequest.want, result);
    }
}

int MissionListManager::DoAbilityForeground(std::shared_ptr<AbilityRecord> &abilityRecord, uint32_t flag)
{
    std::lock_guard guard(managerLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DoAbilityForeground failed, ability record is null.");
        return ERR_INVALID_VALUE;
    }
    if (abilityRecord->GetPendingState() == AbilityState::FOREGROUND) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is FOREGROUND.");
        abilityRecord->SetPendingState(AbilityState::FOREGROUND);
        return ERR_OK;
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pending state is not FOREGROUND.");
        abilityRecord->SetPendingState(AbilityState::FOREGROUND);
    }
    abilityRecord->ProcessForegroundAbility(0, flag);
    return ERR_OK;
}

void MissionListManager::GetActiveAbilityList(const std::string &bundleName, std::vector<std::string> &abilityList,
    int32_t pid)
{
    std::lock_guard guard(managerLock_);
    for (auto missionList : currentMissionLists_) {
        if (missionList != nullptr) {
            std::vector<std::string> currentActiveAbilities;
            missionList->GetActiveAbilityList(bundleName, currentActiveAbilities, pid);
            if (!currentActiveAbilities.empty()) {
                abilityList.insert(abilityList.end(), currentActiveAbilities.begin(), currentActiveAbilities.end());
            }
        }
    }

    if (defaultStandardList_ != nullptr) {
        std::vector<std::string> defaultActiveStandardList;
        defaultStandardList_->GetActiveAbilityList(bundleName, defaultActiveStandardList, pid);
        if (!defaultActiveStandardList.empty()) {
            abilityList.insert(abilityList.end(), defaultActiveStandardList.begin(), defaultActiveStandardList.end());
        }
    }

    if (defaultSingleList_ != nullptr) {
        std::vector<std::string> defaultActiveSingleList;
        defaultSingleList_->GetActiveAbilityList(bundleName, defaultActiveSingleList, pid);
        if (!defaultActiveSingleList.empty()) {
            abilityList.insert(abilityList.end(), defaultActiveSingleList.begin(), defaultActiveSingleList.end());
        }
    }

    if (!abilityList.empty()) {
        sort(abilityList.begin(), abilityList.end());
        abilityList.erase(unique(abilityList.begin(), abilityList.end()), abilityList.end());
    }
}

void MissionListManager::SetLastExitReason(std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
        return;
    }

    if (abilityRecord->GetAbilityInfo().bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName is empty.");
        return;
    }

    ExitReason exitReason;
    bool isSetReason;
    DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        abilityRecord->GetAbilityInfo().bundleName, abilityRecord->GetAbilityInfo().name, isSetReason, exitReason);

    if (isSetReason) {
        abilityRecord->SetLastExitReason(exitReason);
    }
}

bool MissionListManager::IsAppLastAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityRecord is nullptr.");
        return false;
    }

    std::string bundleName = abilityRecord->GetAbilityInfo().bundleName;
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName is empty.");
        return false;
    }

    std::vector<std::string> abilityList;
    for (auto missionList : currentMissionLists_) {
        if (missionList != nullptr) {
            missionList->GetActiveAbilityList(bundleName, abilityList);
        }
    }

    if (abilityList.size() == ONLY_ONE_ABILITY) {
        return true;
    }
    return false;
}

int MissionListManager::PrepareClearMissionLocked(int missionId, const std::shared_ptr<Mission> &mission)
{
    if (mission == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability has already terminate, just remove mission.");
        return ERR_OK;
    }
    auto abilityRecord = mission->GetAbilityRecord();
    if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Ability record is not exist or is on terminating.");
        return ERR_OK;
    }

    // terminate on timeout
    std::weak_ptr<MissionListManager> wpMgr = shared_from_this();
    auto terminateTask = [wpMgr, missionId, mission]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle terminate task: %{public}d", missionId);
        auto mgr = wpMgr.lock();
        if (mgr) {
            mgr->ClearMissionLocking(missionId, mission);
        }
    };
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    int prepareTerminateTimeout =
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime() * PREPARE_TERMINATE_TIMEOUT_MULTIPLE;
    if (handler) {
        handler->SubmitTask(terminateTask, "PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()),
            prepareTerminateTimeout);
    }

    bool res = abilityRecord->PrepareTerminateAbility();
    if (res) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "stop terminating.");
        handler->CancelTask("PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()));
        return ERR_OK;
    }
    handler->CancelTask("PrepareTermiante_" + std::to_string(abilityRecord->GetAbilityRecordId()));
    return ClearMissionLocked(missionId, mission);
}

bool MissionListManager::CheckPrepareTerminateEnable(const std::shared_ptr<Mission> &mission)
{
    if (!isPrepareTerminateEnable_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Only support PC.");
        return false;
    }
    if (mission == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability has already terminate, just remove mission.");
        return false;
    }
    auto abilityRecord = mission->GetAbilityRecord();
    if (abilityRecord == nullptr || abilityRecord->IsTerminating()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability record is not exist or is on terminating.");
        return false;
    }
    auto type = abilityRecord->GetAbilityInfo().type;
    bool isStageBasedModel = abilityRecord->GetAbilityInfo().isStageBasedModel;
    if (!isStageBasedModel || type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ability mode not support.");
        return false;
    }
    auto tokenId = abilityRecord->GetApplicationInfo().accessTokenId;
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyPrepareTerminatePermission(tokenId)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "failed, please apply permission ohos.permission.PREPARE_APP_TERMINATE");
        return false;
    }
    return true;
}

void MissionListManager::CallRequestDone(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<IRemoteObject> &callStub)
{
    std::lock_guard guard(managerLock_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ability record is null.");
        return;
    }
    if (callStub == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "call stub is null.");
        return;
    }
    abilityRecord->CallRequestDone(callStub);
}

void MissionListManager::SendKeyEvent(const AbilityRequest &abilityRequest)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    EventInfo eventInfo;
    eventInfo.abilityName = abilityInfo.name;
    eventInfo.bundleName = abilityInfo.bundleName;
    eventInfo.moduleName = abilityInfo.moduleName;
    EventReport::SendKeyEvent(EventName::START_PRIVATE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
}

void MissionListManager::ReportAbilitAssociatedStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo,
    int64_t type, const std::shared_ptr<AbilityRecord> &callerAbility)
{
#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE
    if (callerAbility == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "associated start caller record is nullptr");
        return;
    }
    int32_t callerUid = callerAbility->GetUid();
    int32_t callerPid = callerAbility->GetPid();
    std::unordered_map<std::string, std::string> eventParams {
        { "name", "associated_start" },
        { "caller_uid", std::to_string(callerUid) },
        { "caller_pid", std::to_string(callerPid) },
        { "callee_uid", std::to_string(abilityInfo.applicationInfo.uid) },
        { "callee_bundle_name", abilityInfo.applicationInfo.bundleName }
    };
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(
        ResourceSchedule::ResType::RES_TYPE_APP_ASSOCIATED_START, type, eventParams);
#endif
}

void MissionListManager::SignRestartAppFlag(const std::string &bundleName)
{
    for (const auto& missionList : currentMissionLists_) {
        if (!missionList) {
            continue;
        }
        missionList->SignRestartAppFlag(bundleName);
    }
    if (defaultStandardList_) {
        defaultStandardList_->SignRestartAppFlag(bundleName);
    }
    if (defaultSingleList_) {
        defaultSingleList_->SignRestartAppFlag(bundleName);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
