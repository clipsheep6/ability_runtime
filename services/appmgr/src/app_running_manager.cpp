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

#include "app_running_manager.h"

#include "app_mgr_service_inner.h"
#include "datetime_ex.h"
#include "iremote_object.h"

#include "appexecfwk_errors.h"
#include "common_event_support.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "os_account_manager_wrapper.h"
#include "perf_profile.h"
#include "parameters.h"
#include "quick_fix_callback_with_record.h"
#include <cstddef>
#ifdef SUPPORT_SCREEN
#include "scene_board_judgement.h"
#include "window_visibility_info.h"
#endif //SUPPORT_SCREEN
#include "app_mgr_service_const.h"
#include "app_mgr_service_dump_error_code.h"
#include "cache_process_manager.h"
#include "res_sched_util.h"
#include "ui_extension_utils.h"


namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr int32_t QUICKFIX_UID = 5524;
    const std::string SHELL_ASSISTANT_BUNDLENAME = "com.huawei.shell_assistant";
    constexpr char DEVELOPER_MODE_STATE[] = "const.security.developermode.state";
}
using EventFwk::CommonEventSupport;

AppRunningManager::AppRunningManager()
    : configuration_(std::make_shared<Configuration>())
{}
AppRunningManager::~AppRunningManager()
{}

std::shared_ptr<AppRunningRecord> AppRunningManager::CreateAppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, const BundleInfo &bundleInfo)
{
    if (!appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return nullptr;
    }

    if (processName.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "processName error");
        return nullptr;
    }

    auto recordId = AppRecordId::Create();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    bool isStageBasedModel = false;
    ClipStringContent(rule, bundleInfo.appId, signCode);
    if (!bundleInfo.hapModuleInfos.empty()) {
        isStageBasedModel = bundleInfo.hapModuleInfos.back().isStageBasedModel;
    }
    TAG_LOGD(AAFwkTag::APPMGR,
        "Create AppRunningRecord, processName: %{public}s, StageBasedModel:%{public}d, recordId: %{public}d",
        processName.c_str(), isStageBasedModel, recordId);

    appRecord->SetStageModelState(isStageBasedModel);
    appRecord->SetSingleton(bundleInfo.singleton);
    appRecord->SetSignCode(signCode);
    appRecord->SetJointUserId(bundleInfo.jointUserId);
    {
        std::lock_guard guard(runningRecordMapMutex_);
        appRunningRecordMap_.emplace(recordId, appRecord);
    }
    {
        std::lock_guard guard(updateConfigurationDelayedLock_);
        updateConfigurationDelayedMap_.emplace(recordId, false);
    }
    return appRecord;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordIsExist(const std::string &appName,
    const std::string &processName, const int uid, const BundleInfo &bundleInfo,
    const std::string &specifiedProcessFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR,
        "appName: %{public}s, processName: %{public}s, uid: %{public}d, specifiedProcessFlag: %{public}s",
        appName.c_str(), processName.c_str(), uid, specifiedProcessFlag.c_str());
    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    auto jointUserId = bundleInfo.jointUserId;
    TAG_LOGD(AAFwkTag::APPMGR, "jointUserId : %{public}s", jointUserId.c_str());
    ClipStringContent(rule, bundleInfo.appId, signCode);

    auto FindSameProcess = [signCode, specifiedProcessFlag, processName, jointUserId](const auto &pair) {
        return (pair.second != nullptr) &&
            (specifiedProcessFlag.empty() ||
            pair.second->GetSpecifiedProcessFlag() == specifiedProcessFlag) &&
            (pair.second->GetSignCode() == signCode) &&
            (pair.second->GetProcessName() == processName) &&
            (pair.second->GetJointUserId() == jointUserId) &&
            !(pair.second->IsTerminating()) &&
            !(pair.second->IsKilling()) && !(pair.second->GetRestartAppFlag());
    };

    auto appRunningMap = GetAppRunningRecordMap();
    if (!jointUserId.empty()) {
        auto iter = std::find_if(appRunningMap.begin(), appRunningMap.end(), FindSameProcess);
        return ((iter == appRunningMap.end()) ? nullptr : iter->second);
    }
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetProcessName() == processName &&
            (specifiedProcessFlag.empty() ||
            appRecord->GetSpecifiedProcessFlag() == specifiedProcessFlag) &&
            !(appRecord->IsTerminating()) && !(appRecord->IsKilling()) && !(appRecord->GetRestartAppFlag())) {
            auto appInfoList = appRecord->GetAppInfoList();
            TAG_LOGD(AAFwkTag::APPMGR,
                "appInfoList: %{public}zu, processName: %{public}s, specifiedProcessFlag: %{public}s",
                appInfoList.size(), appRecord->GetProcessName().c_str(), specifiedProcessFlag.c_str());
            auto isExist = [&appName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
                TAG_LOGD(AAFwkTag::APPMGR, "appInfo->name: %{public}s", appInfo->name.c_str());
                return appInfo->name == appName && appInfo->uid == uid;
            };
            auto appInfoIter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            if (appInfoIter != appInfoList.end()) {
                DelayedSingleton<CacheProcessManager>::GetInstance()->ReuseCachedProcess(appRecord);
                return appRecord;
            }
        }
    }
    return nullptr;
}

#ifdef APP_NO_RESPONSE_DIALOG
bool AppRunningManager::CheckAppRunningRecordIsExist(const std::string &bundleName, const std::string &ablityName)
{
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return false;
    }
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            continue;
        }
        if (appRecord->GetBundleName() != bundleName) {
            continue;
        }
        const auto &abilityRunningRecordMap = appRecord->GetAbilities();
        for (const auto &abilityItem : abilityRunningRecordMap) {
            const auto &abilityRunning = abilityItem.second;
            if (abilityRunning && abilityRunning->GetName() == ablityName) {
                return true;
            }
        }
    }
    return false;
}
#endif

bool AppRunningManager::CheckAppRunningRecordIsExistByBundleName(const std::string &bundleName)
{
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return false;
    }
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && !(appRecord->GetRestartAppFlag())) {
            return true;
        }
    }
    return false;
}

bool AppRunningManager::CheckAppRunningRecordIsExistByUid(int32_t uid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    if (appRunningRecordMap_.empty()) {
        return false;
    }
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetUid() == uid && !(appRecord->GetRestartAppFlag())) {
            return true;
        }
    }
    return false;
}

int32_t AppRunningManager::CheckAppCloneRunningRecordIsExistByBundleName(const std::string &bundleName,
    int32_t appCloneIndex, bool &isRunning)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && !(appRecord->GetRestartAppFlag()) &&
            appRecord->GetAppIndex() == appCloneIndex) {
            isRunning = true;
            break;
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::GetAllAppRunningRecordCountByBundleName(const std::string &bundleName)
{
    int32_t count = 0;
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            count++;
        }
    }

    return count;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        return pair.second->GetPriorityObject()->GetPid() == pid;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityRunningRecordByToken(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

bool AppRunningManager::ProcessExitByBundleName(
    const std::string &bundleName, std::list<pid_t> &pids, const bool clearPageStack)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        // condition [!appRecord->IsKeepAliveApp()] Is to not kill the resident process.
        // Before using this method, consider whether you need.
        if (appRecord && (!appRecord->IsKeepAliveApp() ||
            !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficent())) {
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            auto appInfoList = appRecord->GetAppInfoList();
            auto isExist = [&bundleName](const std::shared_ptr<ApplicationInfo> &appInfo) {
                return appInfo->bundleName == bundleName;
            };
            auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            if (iter == appInfoList.end() || pid <= 0) {
                continue;
            }
            pids.push_back(pid);
            if (clearPageStack) {
                appRecord->ScheduleClearPageStack();
            }
            appRecord->ScheduleProcessSecurityExit();
        }
    }

    return !pids.empty();
}

bool AppRunningManager::GetPidsByUserId(int32_t userId, std::list<pid_t> &pids)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord) {
            int32_t id = -1;
            if ((DelayedSingleton<OsAccountManagerWrapper>::GetInstance()->
                GetOsAccountLocalIdFromUid(appRecord->GetUid(), id) == 0) && (id == userId)) {
                pid_t pid = appRecord->GetPriorityObject()->GetPid();
                if (pid > 0) {
                    pids.push_back(pid);
                    appRecord->ScheduleProcessSecurityExit();
                }
            }
        }
    }

    return (!pids.empty());
}

int32_t AppRunningManager::ProcessUpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    auto appRunningMap = GetAppRunningRecordMap();
    int32_t result = ERR_OK;
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        for (auto iter : appInfoList) {
            if (iter->bundleName == appInfo.bundleName) {
                appRecord->UpdateApplicationInfoInstalled(appInfo);
                break;
            }
        }
    }
    return result;
}

bool AppRunningManager::ProcessExitByBundleNameAndUid(
    const std::string &bundleName, const int uid, std::list<pid_t> &pids, const bool clearPageStack)
{
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        auto appInfoList = appRecord->GetAppInfoList();
        auto isExist = [&bundleName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
            return appInfo->bundleName == bundleName && appInfo->uid == uid;
        };
        auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
        pid_t pid = appRecord->GetPriorityObject()->GetPid();
        if (iter == appInfoList.end() || pid <= 0) {
            continue;
        }
        pids.push_back(pid);
        if (clearPageStack) {
            appRecord->ScheduleClearPageStack();
        }
        appRecord->SetKilling();
        appRecord->ScheduleProcessSecurityExit();
    }

    return (pids.empty() ? false : true);
}

bool AppRunningManager::ProcessExitByPid(pid_t pid)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord != nullptr) {
        appRecord->SetKilling();
        appRecord->ScheduleProcessSecurityExit();
        return true;
    }
    return false;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::OnRemoteDied(const wptr<IRemoteObject> &remote,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remote is null");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::APPMGR, "object is null");
        return nullptr;
    }

    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard guard(runningRecordMapMutex_);
        const auto &iter =
            std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&object](const auto &pair) {
                if (pair.second && pair.second->GetApplicationClient() != nullptr) {
                    return pair.second->GetApplicationClient()->AsObject() == object;
                }
                return false;
            });
        if (iter == appRunningRecordMap_.end()) {
            TAG_LOGE(AAFwkTag::APPMGR, "remote is not exist in the map.");
            return nullptr;
        }
        appRecord = iter->second;
        appRunningRecordMap_.erase(iter);
    }
    if (appRecord != nullptr) {
        {
            std::lock_guard guard(updateConfigurationDelayedLock_);
            updateConfigurationDelayedMap_.erase(appRecord->GetRecordId());
        }
        appRecord->RemoveAppDeathRecipient();
        appRecord->SetApplicationClient(nullptr);
        TAG_LOGI(AAFwkTag::APPMGR, "processName: %{public}s.", appRecord->GetProcessName().c_str());
        auto priorityObject = appRecord->GetPriorityObject();
        if (priorityObject != nullptr) {
            TAG_LOGI(AAFwkTag::APPMGR, "pid: %{public}d.", priorityObject->GetPid());
            if (appMgrServiceInner != nullptr) {
                appMgrServiceInner->KillProcessByPid(priorityObject->GetPid(), "OnRemoteDied");
            }
        }
    }
    if (appRecord != nullptr && appRecord->GetPriorityObject() != nullptr) {
        RemoveUIExtensionLauncherItem(appRecord->GetPriorityObject()->GetPid());
    }

    return appRecord;
}

std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> AppRunningManager::GetAppRunningRecordMap()
{
    std::lock_guard guard(runningRecordMapMutex_);
    return appRunningRecordMap_;
}

void AppRunningManager::RemoveAppRunningRecordById(const int32_t recordId)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    {
        std::lock_guard guard(runningRecordMapMutex_);
        auto it = appRunningRecordMap_.find(recordId);
        if (it != appRunningRecordMap_.end()) {
            appRecord = it->second;
            appRunningRecordMap_.erase(it);
        }
    }
    {
        std::lock_guard guard(updateConfigurationDelayedLock_);
        updateConfigurationDelayedMap_.erase(recordId);
    }

    if (appRecord != nullptr && appRecord->GetPriorityObject() != nullptr) {
        RemoveUIExtensionLauncherItem(appRecord->GetPriorityObject()->GetPid());
    }
}

void AppRunningManager::ClearAppRunningRecordMap()
{
    std::lock_guard guard(runningRecordMapMutex_);
    appRunningRecordMap_.clear();
}

void AppRunningManager::HandleTerminateTimeOut(int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord is nullptr.");
        return;
    }
    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }
    appRecord->AbilityTerminated(abilityToken);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityByTerminateLists(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningManager::GetAbilityRunningRecord(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    for (auto &item : appRunningRecordMap_) {
        if (item.second) {
            auto abilityRecord = item.second->GetAbilityRunningRecord(eventId);
            if (abilityRecord) {
                return abilityRecord;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecord(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&eventId](const auto &pair) {
        return pair.second->GetEventId() == eventId;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

void AppRunningManager::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }

    std::shared_ptr<AbilityRunningRecord> abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        abilityRecord->SetTerminating();
    }

    if (appRecord->IsLastAbilityRecord(token) && (!appRecord->IsKeepAliveApp() ||
        !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficent())) {
        appRecord->SetTerminating();
    }

    auto timeoutTask = [appRecord, token]() {
        if (appRecord) {
            appRecord->TerminateAbility(token, true);
        }
    };
    appRecord->PostTask("DELAY_KILL_ABILITY", AMSEventHandler::KILL_PROCESS_TIMEOUT, timeoutTask);
}

void AppRunningManager::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }

    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        abilityRecord->SetTerminating();
    }

    // set app record terminating when close last page ability
    auto isLastAbility =
        clearMissionFlag ? appRecord->IsLastPageAbilityRecord(token) : appRecord->IsLastAbilityRecord(token);
    if (isLastAbility && (!appRecord->IsKeepAliveApp() ||
        !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficent())) {
        auto cacheProcMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
        if (cacheProcMgr != nullptr && cacheProcMgr->IsAppShouldCache(appRecord)) {
            cacheProcMgr->PenddingCacheProcess(appRecord);
            TAG_LOGI(AAFwkTag::APPMGR, "App %{public}s supports process cache, not terminate record.",
                appRecord->GetBundleName().c_str());
            return;
        }
        TAG_LOGI(AAFwkTag::APPMGR, "The ability is the last in the app:%{public}s.", appRecord->GetName().c_str());
        appRecord->SetTerminating();
    }
}

void AppRunningManager::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }

    auto killProcess = [appRecord, token, inner = appMgrServiceInner]() {
        if (appRecord == nullptr || token == nullptr || inner == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Pointer parameter error.");
            return;
        }
        appRecord->RemoveTerminateAbilityTimeoutTask(token);
        TAG_LOGD(AAFwkTag::APPMGR, "The ability is the last, kill application");
        auto priorityObject = appRecord->GetPriorityObject();
        if (priorityObject == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "priorityObject is nullptr.");
            return;
        }
        auto pid = priorityObject->GetPid();
        if (pid < 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "Pid error.");
            return;
        }
        auto result = inner->KillProcessByPid(pid, "TerminateAbility");
        if (result < 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "Kill application directly failed, pid: %{public}d", pid);
        }
        inner->NotifyAppStatus(appRecord->GetBundleName(), CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        };

    if (clearMissionFlag && appRecord->IsDebugApp()) {
        killProcess();
        return;
    }

    auto isLastAbility =
        clearMissionFlag ? appRecord->IsLastPageAbilityRecord(token) : appRecord->IsLastAbilityRecord(token);
#ifdef SUPPORT_SCREEN
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        appRecord->TerminateAbility(token, true);
    } else {
        appRecord->TerminateAbility(token, false);
    }
#endif //SUPPORT_SCREEN
    auto isLauncherApp = appRecord->GetApplicationInfo()->isLauncherApp;
    if (isLastAbility && (!appRecord->IsKeepAliveApp() ||
        !ExitResidentProcessManager::GetInstance().IsMemorySizeSufficent()) && !isLauncherApp) {
        auto cacheProcMgr = DelayedSingleton<CacheProcessManager>::GetInstance();
        if (cacheProcMgr != nullptr && cacheProcMgr->IsAppShouldCache(appRecord)) {
            TAG_LOGI(AAFwkTag::APPMGR, "App %{public}s is cached, not terminate app.",
                appRecord->GetBundleName().c_str());
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "The ability is the last in the app:%{public}s.", appRecord->GetName().c_str());
        appRecord->SetTerminating();
        if (clearMissionFlag && appMgrServiceInner != nullptr) {
            auto delayTime = appRecord->ExtensionAbilityRecordExists() ?
                AMSEventHandler::DELAY_KILL_EXTENSION_PROCESS_TIMEOUT : AMSEventHandler::DELAY_KILL_PROCESS_TIMEOUT;
            appRecord->PostTask("DELAY_KILL_PROCESS", delayTime, killProcess);
        }
    }
}

void AppRunningManager::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    AssignRunningProcessInfoByAppRecord(appRecord, info);
}

int32_t AppRunningManager::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid process pid:%{public}d", pid);
        return ERR_INVALID_OPERATION;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    return AssignRunningProcessInfoByAppRecord(appRecord, info);
}

int32_t AppRunningManager::AssignRunningProcessInfoByAppRecord(
    std::shared_ptr<AppRunningRecord> appRecord, AppExecFwk::RunningProcessInfo &info) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return ERR_INVALID_OPERATION;
    }

    info.processName_ = appRecord->GetProcessName();
    info.pid_ = appRecord->GetPriorityObject()->GetPid();
    info.uid_ = appRecord->GetUid();
    info.bundleNames.emplace_back(appRecord->GetBundleName());
    info.state_ = static_cast<AppExecFwk::AppProcessState>(appRecord->GetState());
    info.isContinuousTask = appRecord->IsContinuousTask();
    info.isKeepAlive = appRecord->IsKeepAliveApp();
    info.isFocused = appRecord->GetFocusFlag();
    info.isTestProcess = (appRecord->GetUserTestInfo() != nullptr);
    info.startTimeMillis_ = appRecord->GetAppStartTime();
    info.isAbilityForegrounding = appRecord->GetAbilityForegroundingFlag();
    info.isTestMode = info.isTestProcess && system::GetBoolParameter(DEVELOPER_MODE_STATE, false);
    info.extensionType_ = appRecord->GetExtensionType();
    info.processType_ = appRecord->GetProcessType();
    info.isStrictMode = appRecord->IsStrictMode();
    auto appInfo = appRecord->GetApplicationInfo();
    if (appInfo) {
        info.bundleType = static_cast<int32_t>(appInfo->bundleType);
    }
    if (appInfo && (static_cast<int32_t>(appInfo->multiAppMode.multiAppModeType) ==
            static_cast<int32_t>(MultiAppModeType::APP_CLONE))) {
            info.appCloneIndex = appRecord->GetAppIndex();
    }
    return ERR_OK;
}

void AppRunningManager::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }
    appRecord->SetAbilityForegroundingFlag();
}

void AppRunningManager::ClipStringContent(const std::regex &re, const std::string &source, std::string &afterCutStr)
{
    std::smatch basket;
    if (std::regex_search(source, basket, re)) {
        afterCutStr = basket.prefix().str() + basket.suffix().str();
    }
}

void AppRunningManager::GetForegroundApplications(std::vector<AppStateData> &list)
{
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
            return;
        }
        auto state = appRecord->GetState();
        if (state == ApplicationState::APP_STATE_FOREGROUND) {
            AppStateData appData;
            appData.bundleName = appRecord->GetBundleName();
            appData.uid = appRecord->GetUid();
            appData.pid = appRecord->GetPriorityObject()->GetPid();
            appData.state = static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND);
            auto appInfo = appRecord->GetApplicationInfo();
            appData.accessTokenId = appInfo ? appInfo->accessTokenId : 0;
            appData.extensionType = appRecord->GetExtensionType();
            appData.isFocused = appRecord->GetFocusFlag();
            list.push_back(appData);
            TAG_LOGD(AAFwkTag::APPMGR, "bundleName:%{public}s", appData.bundleName.c_str());
        }
    }
}

void AppRunningManager::HandleAddAbilityStageTimeOut(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Handle add ability stage timeout.");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord is nullptr.");
        return;
    }

    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return;
    }

    appRecord->ScheduleProcessSecurityExit();
}

void AppRunningManager::HandleStartSpecifiedAbilityTimeOut(const int64_t eventId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Handle receive multi instances timeout.");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "abilityRecord is nullptr");
        return;
    }

    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
        return;
    }

    appRecord->ScheduleProcessSecurityExit();
}

int32_t AppRunningManager::UpdateConfiguration(const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::vector<std::string> changeKeyV;
    configuration_->CompareDifferent(changeKeyV, config);
    if (!changeKeyV.empty()) {
        configuration_->Merge(changeKeyV, config);
    }

    auto appRunningMap = GetAppRunningRecordMap();
    TAG_LOGD(AAFwkTag::APPMGR, "current app size %{public}zu", appRunningMap.size());
    int32_t result = ERR_OK;
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
            TAG_LOGD(AAFwkTag::APPMGR, "app not ready, appName is %{public}s", appRecord->GetBundleName().c_str());
            continue;
        }
        if (appRecord && !isCollaboratorReserveType(appRecord)) {
            TAG_LOGD(AAFwkTag::APPMGR, "Notification app [%{public}s]", appRecord->GetName().c_str());
            std::lock_guard guard(updateConfigurationDelayedLock_);
            if (appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
                updateConfigurationDelayedMap_[appRecord->GetRecordId()] = false;
                result = appRecord->UpdateConfiguration(config);
            } else {
                updateConfigurationDelayedMap_[appRecord->GetRecordId()] = true;
            }
        }
    }
    return result;
}

int32_t AppRunningManager::UpdateConfigurationByBundleName(const Configuration &config, const std::string &name)
{
    auto appRunningMap = GetAppRunningRecordMap();
    int32_t result = ERR_OK;
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
            TAG_LOGD(AAFwkTag::APPMGR, "app not ready, appName is %{public}s", appRecord->GetBundleName().c_str());
            continue;
        }
        if (appRecord && !isCollaboratorReserveType(appRecord) && appRecord->GetBundleName() == name) {
            TAG_LOGD(AAFwkTag::APPMGR, "Notification app [%{public}s]", appRecord->GetName().c_str());
            result = appRecord->UpdateConfiguration(config);
        }
    }
    return result;
}

bool AppRunningManager::isCollaboratorReserveType(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    std::string bundleName = appRecord->GetApplicationInfo()->name;
    bool isReserveType = bundleName == SHELL_ASSISTANT_BUNDLENAME;
    if (isReserveType) {
        TAG_LOGI(AAFwkTag::APPMGR, "isReserveType app [%{public}s]", appRecord->GetName().c_str());
    }
    return isReserveType;
}

int32_t AppRunningManager::NotifyMemoryLevel(int32_t level)
{
    std::unordered_set<int32_t> frozenPids;
    AAFwk::ResSchedUtil::GetInstance().GetAllFrozenPidsFromRSS(frozenPids);
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        auto priorityObject = appRecord->GetPriorityObject();
        if (!priorityObject) {
            TAG_LOGW(AAFwkTag::APPMGR, "priorityObject null");
            continue;
        }
        auto pid = priorityObject->GetPid();
        if (frozenPids.count(pid) == 0) {
            TAG_LOGD(AAFwkTag::APPMGR, "proc[pid=%{public}d] memory level = %{public}d", pid, level);
            appRecord->ScheduleMemoryLevel(level);
        } else {
            TAG_LOGD(AAFwkTag::APPMGR, "proc[pid=%{public}d] is frozen", pid);
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::NotifyProcMemoryLevel(const std::map<pid_t, MemoryLevel> &procLevelMap)
{
    std::unordered_set<int32_t> frozenPids;
    AAFwk::ResSchedUtil::GetInstance().GetAllFrozenPidsFromRSS(frozenPids);
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord null");
            continue;
        }
        auto priorityObject = appRecord->GetPriorityObject();
        if (!priorityObject) {
            TAG_LOGW(AAFwkTag::APPMGR, "priorityObject null");
            continue;
        }
        auto pid = priorityObject->GetPid();
        if (frozenPids.count(pid) == 0) {
            auto it = procLevelMap.find(pid);
            if (it == procLevelMap.end()) {
                TAG_LOGW(AAFwkTag::APPMGR, "proc[pid=%{public}d] is not found in procLevelMap.", pid);
            } else {
                TAG_LOGD(AAFwkTag::APPMGR, "proc[pid=%{public}d] memory level = %{public}d", pid, it->second);
                appRecord->ScheduleMemoryLevel(it->second);
            }
        } else {
            TAG_LOGD(AAFwkTag::APPMGR, "proc[pid=%{public}d] is frozen", pid);
        }
    }
    return ERR_OK;
}

int32_t AppRunningManager::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    std::shared_ptr<AppRunningRecord> appRecord;
    {
        std::lock_guard guard(runningRecordMapMutex_);
        TAG_LOGI(AAFwkTag::APPMGR, "current app size %{public}zu", appRunningRecordMap_.size());
        auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
            auto priorityObject = pair.second->GetPriorityObject();
            return priorityObject && priorityObject->GetPid() == pid;
        });
        if (iter == appRunningRecordMap_.end()) {
            TAG_LOGE(AAFwkTag::APPMGR, "No matching application was found.");
            return ERR_INVALID_VALUE;
        }
        appRecord = iter->second;
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
            return ERR_INVALID_VALUE;
        }
    }
    appRecord->ScheduleHeapMemory(pid, mallocInfo);
    return ERR_OK;
}

int32_t AppRunningManager::DumpJsHeapMemory(OHOS::AppExecFwk::JsHeapDumpInfo &info)
{
    int32_t pid = static_cast<int32_t>(info.pid);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }
    appRecord->ScheduleJsHeapMemory(info);
    return ERR_OK;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByRenderPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto renderRecordMap = pair.second->GetRenderRecordMap();
        if (renderRecordMap.empty()) {
            return false;
        }
        for (auto it : renderRecordMap) {
            auto renderRecord = it.second;
            if (renderRecord && renderRecord->GetPid() == pid) {
                return true;
            }
        }
        return false;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<RenderRecord> AppRunningManager::OnRemoteRenderDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remote is null");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::APPMGR, "promote failed.");
        return nullptr;
    }

    std::lock_guard guard(runningRecordMapMutex_);
    std::shared_ptr<RenderRecord> renderRecord;
    const auto &it =
        std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(),
            [&object, &renderRecord](const auto &pair) {
            if (!pair.second) {
                return false;
            }

            auto renderRecordMap = pair.second->GetRenderRecordMap();
            if (renderRecordMap.empty()) {
                return false;
            }
            for (auto iter : renderRecordMap) {
                if (iter.second == nullptr) {
                    continue;
                }
                auto scheduler = iter.second->GetScheduler();
                if (scheduler && scheduler->AsObject() == object) {
                    renderRecord = iter.second;
                    return true;
                }
            }
            return false;
        });
    if (it != appRunningRecordMap_.end()) {
        auto appRecord = it->second;
        appRecord->RemoveRenderRecord(renderRecord);
        return renderRecord;
    }
    return nullptr;
}

bool AppRunningManager::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            TAG_LOGD(AAFwkTag::APPMGR, "Process of [%{public}s] is running, processName: %{public}s.",
                bundleName.c_str(), appRecord->GetProcessName().c_str());
            if (IPCSkeleton::GetCallingUid() == QUICKFIX_UID && appRecord->GetPriorityObject() != nullptr) {
                TAG_LOGI(AAFwkTag::APPMGR, "pid: %{public}d.", appRecord->GetPriorityObject()->GetPid());
            }
            return true;
        }
    }
    return false;
}

int32_t AppRunningManager::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    int32_t result = ERR_OK;
    bool loadSucceed = false;
    auto callbackByRecord = sptr<QuickFixCallbackWithRecord>::MakeSptr(callback);
    if (callbackByRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to create callback record.");
        return ERR_INVALID_VALUE;
    }

    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            TAG_LOGD(AAFwkTag::APPMGR, "Notify application [%{public}s] load patch, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyLoadRepairPatch(bundleName, callbackByRecord, recordId);
            if (result == ERR_OK) {
                loadSucceed = true;
            } else {
                callbackByRecord->RemoveRecordId(recordId);
            }
        }
    }
    return loadSucceed == true ? ERR_OK : result;
}

int32_t AppRunningManager::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    int32_t result = ERR_OK;
    bool reloadPageSucceed = false;
    auto callbackByRecord = sptr<QuickFixCallbackWithRecord>::MakeSptr(callback);
    if (callbackByRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to create callback record.");
        return ERR_INVALID_VALUE;
    }

    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            TAG_LOGD(AAFwkTag::APPMGR, "Notify application [%{public}s] reload page, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyHotReloadPage(callbackByRecord, recordId);
            if (result == ERR_OK) {
                reloadPageSucceed = true;
            } else {
                callbackByRecord->RemoveRecordId(recordId);
            }
        }
    }
    return reloadPageSucceed == true ? ERR_OK : result;
}

int32_t AppRunningManager::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    int32_t result = ERR_OK;
    bool unLoadSucceed = false;
    auto callbackByRecord = sptr<QuickFixCallbackWithRecord>::MakeSptr(callback);
    if (callbackByRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to create callback record.");
        return ERR_INVALID_VALUE;
    }

    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            TAG_LOGD(AAFwkTag::APPMGR, "Notify application [%{public}s] unload patch, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyUnLoadRepairPatch(bundleName, callbackByRecord, recordId);
            if (result == ERR_OK) {
                unLoadSucceed = true;
            } else {
                callbackByRecord->RemoveRecordId(recordId);
            }
        }
    }
    return unLoadSucceed == true ? ERR_OK : result;
}

bool AppRunningManager::IsApplicationFirstForeground(const AppRunningRecord &foregroundingRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    if (AAFwk::UIExtensionUtils::IsUIExtension(foregroundingRecord.GetExtensionType())
        || AAFwk::UIExtensionUtils::IsWindowExtension(foregroundingRecord.GetExtensionType())) {
        return false;
    }

    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != foregroundingRecord.GetBundleName()
            || AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType())
            || AAFwk::UIExtensionUtils::IsWindowExtension(appRecord->GetExtensionType())) {
            continue;
        }
        auto state = appRecord->GetState();
        if (state == ApplicationState::APP_STATE_FOREGROUND &&
            appRecord->GetRecordId() != foregroundingRecord.GetRecordId()) {
            return false;
        }
    }
    return true;
}

bool AppRunningManager::IsApplicationBackground(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "function called.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr");
            return false;
        }
        if (AAFwk::UIExtensionUtils::IsUIExtension(appRecord->GetExtensionType())
            || AAFwk::UIExtensionUtils::IsWindowExtension(appRecord->GetExtensionType())) {
            continue;
        }
        auto state = appRecord->GetState();
        if (appRecord && appRecord->GetBundleName() == bundleName &&
            state == ApplicationState::APP_STATE_FOREGROUND) {
            return false;
        }
    }
    return true;
}
#ifdef SUPPORT_SCREEN
void AppRunningManager::OnWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::set<int32_t> pids;
    for (const auto &info : windowVisibilityInfos) {
        if (info == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "Window visibility info is nullptr.");
            continue;
        }
        if (pids.find(info->pid_) != pids.end()) {
            continue;
        }
        auto appRecord = GetAppRunningRecordByPid(info->pid_);
        if (appRecord == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "App running record is nullptr.");
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "The visibility of %{public}s was changed.", appRecord->GetBundleName().c_str());
        appRecord->OnWindowVisibilityChanged(windowVisibilityInfos);
        pids.emplace(info->pid_);
    }
}
#endif //SUPPORT_SCREEN
bool AppRunningManager::IsApplicationFirstFocused(const AppRunningRecord &focusedRecord)
{
    TAG_LOGD(AAFwkTag::APPMGR, "check focus function called.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != focusedRecord.GetBundleName()) {
            continue;
        }
        if (appRecord->GetFocusFlag() && appRecord->GetRecordId() != focusedRecord.GetRecordId()) {
            return false;
        }
    }
    return true;
}

bool AppRunningManager::IsApplicationUnfocused(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "check is application unfocused.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && appRecord->GetFocusFlag()) {
            return false;
        }
    }
    return true;
}

void AppRunningManager::SetAttachAppDebug(const std::string &bundleName, const bool &isAttachDebug)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appRunningMap = GetAppRunningRecordMap();
    for (const auto &item : appRunningMap) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr) {
            continue;
        }
        if (appRecord->GetBundleName() == bundleName) {
            TAG_LOGD(AAFwkTag::APPMGR, "The application: %{public}s will be set debug mode.", bundleName.c_str());
            appRecord->SetAttachDebug(isAttachDebug);
        }
    }
}

std::vector<AppDebugInfo> AppRunningManager::GetAppDebugInfosByBundleName(
    const std::string &bundleName, const bool &isDetachDebug)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard guard(runningRecordMapMutex_);
    std::vector<AppDebugInfo> debugInfos;
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != bundleName ||
            (isDetachDebug && (appRecord->IsDebugApp() || appRecord->IsAssertionPause()))) {
            continue;
        }

        AppDebugInfo debugInfo;
        debugInfo.bundleName = bundleName;
        auto priorityObject = appRecord->GetPriorityObject();
        if (priorityObject) {
            debugInfo.pid = priorityObject->GetPid();
        }
        debugInfo.uid = appRecord->GetUid();
        debugInfo.isDebugStart = (appRecord->IsDebugApp() || appRecord->IsAssertionPause());
        debugInfos.emplace_back(debugInfo);
    }
    return debugInfos;
}

void AppRunningManager::GetAbilityTokensByBundleName(
    const std::string &bundleName, std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != bundleName) {
            continue;
        }

        for (const auto &token : appRecord->GetAbilities()) {
            abilityTokens.emplace_back(token.first);
        }
    }
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByChildProcessPid(const pid_t pid)
{
    std::lock_guard guard(runningRecordMapMutex_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto childProcessRecordMap = pair.second->GetChildProcessRecordMap();
        return childProcessRecordMap.find(pid) != childProcessRecordMap.end();
    });
    if (iter != appRunningRecordMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<ChildProcessRecord> AppRunningManager::OnChildProcessRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGE(AAFwkTag::APPMGR, "On child process remote died.");
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "remote is null");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        TAG_LOGE(AAFwkTag::APPMGR, "promote failed.");
        return nullptr;
    }

    std::lock_guard guard(runningRecordMapMutex_);
    std::shared_ptr<ChildProcessRecord> childRecord;
    const auto &it = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(),
        [&object, &childRecord](const auto &pair) {
            auto appRecord = pair.second;
            if (!appRecord) {
                return false;
            }
            auto childRecordMap = appRecord->GetChildProcessRecordMap();
            if (childRecordMap.empty()) {
                return false;
            }
            for (auto iter : childRecordMap) {
                if (iter.second == nullptr) {
                    continue;
                }
                auto scheduler = iter.second->GetScheduler();
                if (scheduler && scheduler->AsObject() == object) {
                    childRecord = iter.second;
                    return true;
                }
            }
            return false;
        });
    if (it != appRunningRecordMap_.end()) {
        auto appRecord = it->second;
        appRecord->RemoveChildProcessRecord(childRecord);
        return childRecord;
    }
    return nullptr;
}

int32_t AppRunningManager::SignRestartAppFlag(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != bundleName) {
            continue;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "sign");
        appRecord->SetRestartAppFlag(true);
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "Not find apprecord.");
    return ERR_INVALID_VALUE;
}

int32_t AppRunningManager::GetAppRunningUniqueIdByPid(pid_t pid, std::string &appRunningUniqueId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }
    appRunningUniqueId = std::to_string(appRecord->GetAppStartTime());
    TAG_LOGD(AAFwkTag::APPMGR, "appRunningUniqueId = %{public}s.", appRunningUniqueId.c_str());
    return ERR_OK;
}

int32_t AppRunningManager::GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto &item: uiExtensionLauncherMap_) {
        auto temp = item.second.second;
        if (temp == pid) {
            hostPids.emplace_back(item.second.first);
        }
    }
    std::string hostPidStr = std::accumulate(hostPids.begin(), hostPids.end(), std::string(),
        [](const std::string& a, pid_t b) {
            return a + std::to_string(b) + " ";
        });
    TAG_LOGI(AAFwkTag::APPMGR, "pid: %{public}s, hostPid: %{public}s.", std::to_string(pid).c_str(),
        hostPidStr.c_str());
    return ERR_OK;
}

int32_t AppRunningManager::GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
{
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto &item: uiExtensionLauncherMap_) {
        auto temp = item.second.first;
        if (temp == hostPid) {
            providerPids.emplace_back(item.second.second);
        }
    }

    return ERR_OK;
}

int32_t AppRunningManager::AddUIExtensionLauncherItem(int32_t uiExtensionAbilityId, pid_t hostPid, pid_t providerPid)
{
    std::lock_guard guard(uiExtensionMapLock_);
    uiExtensionLauncherMap_.emplace(uiExtensionAbilityId, std::pair<pid_t, pid_t>(hostPid, providerPid));
    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionLauncherItem(pid_t pid)
{
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto it = uiExtensionLauncherMap_.begin(); it != uiExtensionLauncherMap_.end();) {
        if (it->second.first == pid || it->second.second == pid) {
            it = uiExtensionLauncherMap_.erase(it);
            continue;
        }
        it++;
    }

    return ERR_OK;
}

int32_t AppRunningManager::RemoveUIExtensionLauncherItemById(int32_t uiExtensionAbilityId)
{
    std::lock_guard guard(uiExtensionMapLock_);
    for (auto it = uiExtensionLauncherMap_.begin(); it != uiExtensionLauncherMap_.end();) {
        if (it->first == uiExtensionAbilityId) {
            it = uiExtensionLauncherMap_.erase(it);
            continue;
        }
        it++;
    }

    return ERR_OK;
}

int AppRunningManager::DumpIpcAllStart(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    int errCode = DumpErrorCode::ERR_OK;
    for (const auto &item : GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "AppRunningManager::DumpIpcAllStart::pid:%{public}d",
            appRecord->GetPriorityObject()->GetPid());
        std::string currentResult;
        errCode = appRecord->DumpIpcStart(currentResult);
        result += currentResult + "\n";
        if (errCode != DumpErrorCode::ERR_OK) {
            return errCode;
        }
    }
    return errCode;
}

int AppRunningManager::DumpIpcAllStop(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    int errCode = DumpErrorCode::ERR_OK;
    for (const auto &item : GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "AppRunningManager::DumpIpcAllStop::pid:%{public}d",
            appRecord->GetPriorityObject()->GetPid());
        std::string currentResult;
        errCode = appRecord->DumpIpcStop(currentResult);
        result += currentResult + "\n";
        if (errCode != DumpErrorCode::ERR_OK) {
            return errCode;
        }
    }
    return errCode;
}

int AppRunningManager::DumpIpcAllStat(std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    int errCode = DumpErrorCode::ERR_OK;
    for (const auto &item : GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        TAG_LOGD(AAFwkTag::APPMGR, "AppRunningManager::DumpIpcAllStat::pid:%{public}d",
            appRecord->GetPriorityObject()->GetPid());
        std::string currentResult;
        errCode = appRecord->DumpIpcStat(currentResult);
        result += currentResult + "\n";
        if (errCode != DumpErrorCode::ERR_OK) {
            return errCode;
        }
    }
    return errCode;
}

int AppRunningManager::DumpIpcStart(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    const auto& appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        result.append(MSG_DUMP_IPC_START_STAT, strlen(MSG_DUMP_IPC_START_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return appRecord->DumpIpcStart(result);
}

int AppRunningManager::DumpIpcStop(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    const auto& appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        result.append(MSG_DUMP_IPC_STOP_STAT, strlen(MSG_DUMP_IPC_STOP_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return appRecord->DumpIpcStop(result);
}

int AppRunningManager::DumpIpcStat(const int32_t pid, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    const auto& appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        result.append(MSG_DUMP_IPC_STAT, strlen(MSG_DUMP_IPC_STAT))
            .append(MSG_DUMP_FAIL, strlen(MSG_DUMP_FAIL))
            .append(MSG_DUMP_FAIL_REASON_INVALILD_PID, strlen(MSG_DUMP_FAIL_REASON_INVALILD_PID));
        TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    return appRecord->DumpIpcStat(result);
}

int AppRunningManager::DumpFfrt(const std::vector<int32_t>& pids, std::string& result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    int errCode = DumpErrorCode::ERR_OK;
    size_t count = 0;
    for (const auto& pid : pids) {
        TAG_LOGD(AAFwkTag::APPMGR, "DumpFfrt current pid:%{public}d", pid);
        const auto& appRecord = GetAppRunningRecordByPid(pid);
        if (!appRecord) {
            TAG_LOGE(AAFwkTag::APPMGR, "pid %{public}d does not exist", pid);
            ++count;
            continue;
        }
        std::string currentResult;
        errCode = appRecord->DumpFfrt(currentResult);
        if (errCode != DumpErrorCode::ERR_OK) {
            continue;
        }
        result += currentResult + "\n";
    }
    if (count == pids.size()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no valid pid");
        return DumpErrorCode::ERR_INVALID_PID_ERROR;
    }
    if (result.empty()) {
        TAG_LOGE(AAFwkTag::APPMGR, "no ffrt usage is found");
        return DumpErrorCode::ERR_INTERNAL_ERROR;
    }
    return DumpErrorCode::ERR_OK;
}

bool AppRunningManager::IsAppProcessesAllCached(const std::string &bundleName, int32_t uid,
    const std::set<std::shared_ptr<AppRunningRecord>> &cachedSet)
{
    if (cachedSet.size() == 0) {
        TAG_LOGI(AAFwkTag::APPMGR, "empty cache set.");
        return false;
    }
    std::lock_guard guard(runningRecordMapMutex_);
    for (const auto &item : appRunningRecordMap_) {
        auto &itemRecord = item.second;
        if (itemRecord == nullptr) {
            continue;
        }
        if (itemRecord->GetBundleName() == bundleName && itemRecord->GetUid() == uid) {
            auto supportCache =
                DelayedSingleton<CacheProcessManager>::GetInstance()->IsAppSupportProcessCache(itemRecord);
            // need wait for unsupported processes
            if ((cachedSet.find(itemRecord) == cachedSet.end() && supportCache) || !supportCache) {
                return false;
            }
        }
    }
    return true;
}

int32_t AppRunningManager::UpdateConfigurationDelayed(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    std::lock_guard guard(updateConfigurationDelayedLock_);
    int32_t result = ERR_OK;
    auto it = updateConfigurationDelayedMap_.find(appRecord->GetRecordId());
    if (it != updateConfigurationDelayedMap_.end()) {
        result = appRecord->UpdateConfiguration(*configuration_);
        it->second = false;
    }
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
