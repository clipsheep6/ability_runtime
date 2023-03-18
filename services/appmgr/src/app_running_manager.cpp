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

#include "app_running_manager.h"

#include "app_mgr_service_inner.h"
#include "datetime_ex.h"
#include "iremote_object.h"

#include "appexecfwk_errors.h"
#include "common_event_support.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "os_account_manager_wrapper.h"
#include "perf_profile.h"
#include "quick_fix_callback_with_record.h"

namespace OHOS {
namespace AppExecFwk {
AppRunningManager::AppRunningManager()
{}
AppRunningManager::~AppRunningManager()
{}

std::shared_ptr<AppRunningRecord> AppRunningManager::CreateAppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::string &processName, const BundleInfo &bundleInfo)
{
    if (!appInfo) {
        HILOG_ERROR("param error");
        return nullptr;
    }

    if (processName.empty()) {
        HILOG_ERROR("processName error");
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
    HILOG_DEBUG("Create AppRunningRecord, processName: %{public}s, StageBasedModel:%{public}d, recordId: %{public}d",
        processName.c_str(), isStageBasedModel, recordId);

    appRecord->SetStageModelState(isStageBasedModel);
    appRecord->SetSignCode(signCode);
    appRecord->SetJointUserId(bundleInfo.jointUserId);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    appRunningRecordMap_.emplace(recordId, appRecord);
    return appRecord;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::CheckAppRunningRecordIsExist(const std::string &appName,
    const std::string &processName, const int uid, const BundleInfo &bundleInfo)
{
    HILOG_INFO("appName: %{public}s, processName: %{public}s, uid : %{public}d",
        appName.c_str(), processName.c_str(), uid);

    std::regex rule("[a-zA-Z.]+[-_#]{1}");
    std::string signCode;
    auto jointUserId = bundleInfo.jointUserId;
    HILOG_INFO("jointUserId : %{public}s", jointUserId.c_str());
    ClipStringContent(rule, bundleInfo.appId, signCode);

    auto FindSameProcess = [signCode, processName, jointUserId](const auto &pair) {
            return ((pair.second->GetSignCode() == signCode) &&
                    (pair.second->GetProcessName() == processName) &&
                    (pair.second->GetJointUserId() == jointUserId) &&
                    !(pair.second->IsTerminating()) &&
                    !(pair.second->IsKilling()));
    };

    // If it is not empty, look for whether it can come in the same process
    std::lock_guard<std::recursive_mutex> guard(lock_);
    if (jointUserId.empty()) {
        for (const auto &item : appRunningRecordMap_) {
            const auto &appRecord = item.second;
            if (appRecord && appRecord->GetProcessName() == processName &&
                !(appRecord->IsTerminating()) && !(appRecord->IsKilling())) {
                HILOG_INFO("appRecord->GetProcessName() : %{public}s", appRecord->GetProcessName().c_str());
                auto appInfoList = appRecord->GetAppInfoList();
                HILOG_INFO("appInfoList : %{public}zu", appInfoList.size());
                auto isExist = [&appName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
                    HILOG_INFO("appInfo->name : %{public}s", appInfo->name.c_str());
                    return appInfo->name == appName && appInfo->uid == uid;
                };
                auto appInfoIter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
                if (appInfoIter != appInfoList.end()) {
                    return appRecord;
                }
            }
        }
        return nullptr;
    }

    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), FindSameProcess);
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByPid(const pid_t pid)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        return pair.second->GetPriorityObject()->GetPid() == pid;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetAbilityRunningRecordByToken(abilityToken)) {
            return appRecord;
        }
    }
    return nullptr;
}

bool AppRunningManager::ProcessExitByBundleName(const std::string &bundleName, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        // condition [!appRecord->IsKeepAliveApp()] Is to not kill the resident process.
        // Before using this method, consider whether you need.
        if (appRecord && !appRecord->IsKeepAliveApp()) {
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            auto appInfoList = appRecord->GetAppInfoList();
            auto isExist = [&bundleName](const std::shared_ptr<ApplicationInfo> &appInfo) {
                return appInfo->bundleName == bundleName;
            };
            auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            if (iter != appInfoList.end() && pid > 0) {
                pids.push_back(pid);
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return !pids.empty();
}

bool AppRunningManager::GetPidsByUserId(int32_t userId, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
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
    std::lock_guard<std::recursive_mutex> guard(lock_);
    int32_t result = ERR_OK;
    for (const auto &item : appRunningRecordMap_) {
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
    const std::string &bundleName, const int uid, std::list<pid_t> &pids)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord) {
            auto appInfoList = appRecord->GetAppInfoList();
            auto isExist = [&bundleName, &uid](const std::shared_ptr<ApplicationInfo> &appInfo) {
                return appInfo->bundleName == bundleName && appInfo->uid == uid;
            };
            auto iter = std::find_if(appInfoList.begin(), appInfoList.end(), isExist);
            pid_t pid = appRecord->GetPriorityObject()->GetPid();
            if (iter != appInfoList.end() && pid > 0) {
                pids.push_back(pid);

                appRecord->SetKilling();
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }

    return (pids.empty() ? false : true);
}

bool AppRunningManager::ProcessExitByPid(pid_t pid)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord) {
            pid_t appPid = appRecord->GetPriorityObject()->GetPid();
            if (appPid == pid) {
                appRecord->SetKilling();
                appRecord->ScheduleProcessSecurityExit();
                return true;
            }
        }
    }

    return false;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_INFO("On remot died.");
    if (remote == nullptr) {
        HILOG_ERROR("remote is null");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        HILOG_ERROR("object is null");
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> guard(lock_);
    const auto &iter =
        std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&object](const auto &pair) {
            if (pair.second && pair.second->GetApplicationClient() != nullptr) {
                return pair.second->GetApplicationClient()->AsObject() == object;
            }
            return false;
        });
    if (iter == appRunningRecordMap_.end()) {
        HILOG_ERROR("remote is not exist in the map.");
        return nullptr;
    }
    auto appRecord = iter->second;
    if (appRecord != nullptr) {
        appRecord->SetApplicationClient(nullptr);
    }
    appRunningRecordMap_.erase(iter);
    return appRecord;
}

std::map<const int32_t, const std::shared_ptr<AppRunningRecord>> AppRunningManager::GetAppRunningRecordMap()
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    return appRunningRecordMap_;
}

void AppRunningManager::RemoveAppRunningRecordById(const int32_t recordId)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    appRunningRecordMap_.erase(recordId);
}

void AppRunningManager::ClearAppRunningRecordMap()
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    appRunningRecordMap_.clear();
}

void AppRunningManager::HandleTerminateTimeOut(int64_t eventId)
{
    HILOG_INFO("Handle terminate timeout.");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is nullptr.");
        return;
    }
    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr.");
        return;
    }
    appRecord->AbilityTerminated(abilityToken);
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &abilityToken)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
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
    HILOG_INFO("Get ability running record by eventId.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
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
    HILOG_INFO("Get app running record by eventId.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&eventId](const auto &pair) {
        return pair.second->GetEventId() == eventId;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

void AppRunningManager::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("Handle ability attach timeOut.");
    if (token == nullptr) {
        HILOG_ERROR("token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr.");
        return;
    }

    std::shared_ptr<AbilityRunningRecord> abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        abilityRecord->SetTerminating();
    }

    if (appRecord->IsLastAbilityRecord(token) && !appRecord->IsKeepAliveApp()) {
        appRecord->SetTerminating();
    }

    auto timeoutTask = [appRecord, token]() {
        if (appRecord) {
            appRecord->TerminateAbility(token, true);
        }
    };
    appRecord->PostTask("DELAY_KILL_ABILITY", AMSEventHandler::KILL_PROCESS_TIMEOUT, timeoutTask);
}

void AppRunningManager::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        HILOG_ERROR("token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr.");
        return;
    }

    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityRecord) {
        abilityRecord->SetTerminating();
    }

    if (appRecord->IsLastAbilityRecord(token) && !appRecord->IsKeepAliveApp()) {
        HILOG_INFO("The ability is the last in the app:%{public}s.", appRecord->GetName().c_str());
        appRecord->SetTerminating();
    }
}

void AppRunningManager::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag,
    std::shared_ptr<AppMgrServiceInner> appMgrServiceInner)
{
    if (!token) {
        HILOG_ERROR("token is nullptr.");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr.");
        return;
    }
    auto isLastAbility =
        clearMissionFlag ? appRecord->IsLastPageAbilityRecord(token) : appRecord->IsLastAbilityRecord(token);
    appRecord->TerminateAbility(token, false);

    auto isKeepAliveApp = appRecord->IsKeepAliveApp();
    auto isLauncherApp = appRecord->GetApplicationInfo()->isLauncherApp;
    if (isLastAbility && !isKeepAliveApp && !isLauncherApp) {
        HILOG_DEBUG("The ability is the last in the app:%{public}s.", appRecord->GetName().c_str());
        appRecord->SetTerminating();
        if (clearMissionFlag && appMgrServiceInner != nullptr) {
            appRecord->RemoveTerminateAbilityTimeoutTask(token);
            HILOG_DEBUG("The ability is the last, kill application");
            auto pid = appRecord->GetPriorityObject()->GetPid();
            auto result = appMgrServiceInner->KillProcessByPid(pid);
            if (result < 0) {
                HILOG_WARN("Kill application directly failed, pid: %{public}d", pid);
            }
            appMgrServiceInner->NotifyAppStatus(appRecord->GetBundleName(),
                EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        }
    }
}

void AppRunningManager::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto appRecord = GetAppRunningRecordByAbilityToken(token);

    AssignRunningProcessInfoByAppRecord(appRecord, info);
}

void AppRunningManager::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto appRecord = GetAppRunningRecordByPid(pid);

    AssignRunningProcessInfoByAppRecord(appRecord, info);
}

void AppRunningManager::AssignRunningProcessInfoByAppRecord(
    std::shared_ptr<AppRunningRecord> appRecord, AppExecFwk::RunningProcessInfo &info) const
{
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
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
    HILOG_INFO("%{public}s, begin.", __func__);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (!appRecord) {
            HILOG_ERROR("appRecord is nullptr");
            return;
        }
        auto state = appRecord->GetState();
        if (state == ApplicationState::APP_STATE_FOREGROUND) {
            AppStateData appData;
            appData.bundleName = appRecord->GetBundleName();
            appData.uid = appRecord->GetUid();
            appData.state = static_cast<int32_t>(ApplicationState::APP_STATE_FOREGROUND);
            appData.isFocused = appRecord->GetFocusFlag();
            list.push_back(appData);
            HILOG_INFO("%{public}s, bundleName:%{public}s", __func__, appData.bundleName.c_str());
        }
    }
}

void AppRunningManager::HandleAddAbilityStageTimeOut(const int64_t eventId)
{
    HILOG_DEBUG("Handle add ability stage timeout.");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }

    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }

    appRecord->ScheduleProcessSecurityExit();
}

void AppRunningManager::HandleStartSpecifiedAbilityTimeOut(const int64_t eventId)
{
    HILOG_DEBUG("Handle receive multi instances timeout.");
    auto abilityRecord = GetAbilityRunningRecord(eventId);
    if (!abilityRecord) {
        HILOG_ERROR("abilityRecord is nullptr");
        return;
    }

    auto abilityToken = abilityRecord->GetToken();
    auto appRecord = GetTerminatingAppRunningRecord(abilityToken);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }

    appRecord->ScheduleProcessSecurityExit();
}

int32_t AppRunningManager::UpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("call %{public}s", __func__);
    std::lock_guard<std::recursive_mutex> guard(lock_);
    HILOG_INFO("current app size %{public}zu", appRunningRecordMap_.size());
    int32_t result = ERR_OK;
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord) {
            HILOG_INFO("Notification app [%{public}s]", appRecord->GetName().c_str());
            result = appRecord->UpdateConfiguration(config);
        }
    }
    return result;
}

int32_t AppRunningManager::NotifyMemoryLevel(int32_t level)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    HILOG_INFO("call %{public}s, current app size %{public}zu", __func__, appRunningRecordMap_.size());
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        HILOG_INFO("Notification app [%{public}s]", appRecord->GetName().c_str());
        appRecord->ScheduleMemoryLevel(level);
    }
    return ERR_OK;
}

int32_t AppRunningManager::DumpHeapMemory(std::vector<int32_t> &mallinfo)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    int32_t pid = mallinfo.front();
    HILOG_INFO("call %{public}s, current app size %{public}zu", __func__, appRunningRecordMap_.size());
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto priorityObject = pair.second->GetPriorityObject();
        return priorityObject && priorityObject->GetPid() == pid;
    });
    if (iter == appRunningRecordMap_.end()) {
        HILOG_ERROR("No matching application was found.");
        return ERR_OK;
    }
    iter.second->ScheduleHeapMemory(mallinfo);
    return ERR_OK;
}

std::shared_ptr<AppRunningRecord> AppRunningManager::GetAppRunningRecordByRenderPid(const pid_t pid)
{
    std::lock_guard<std::recursive_mutex> guard(lock_);
    auto iter = std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&pid](const auto &pair) {
        auto renderRecord = pair.second->GetRenderRecord();
        return renderRecord && renderRecord->GetPid() == pid;
    });
    return ((iter == appRunningRecordMap_.end()) ? nullptr : iter->second);
}

std::shared_ptr<RenderRecord> AppRunningManager::OnRemoteRenderDied(const wptr<IRemoteObject> &remote)
{
    if (remote == nullptr) {
        HILOG_ERROR("remote is null");
        return nullptr;
    }
    sptr<IRemoteObject> object = remote.promote();
    if (!object) {
        HILOG_ERROR("promote failed.");
        return nullptr;
    }

    std::lock_guard<std::recursive_mutex> guard(lock_);
    const auto &it =
        std::find_if(appRunningRecordMap_.begin(), appRunningRecordMap_.end(), [&object](const auto &pair) {
            if (!pair.second) {
                return false;
            }

            auto renderRecord = pair.second->GetRenderRecord();
            if (!renderRecord) {
                return false;
            }

            auto scheduler = renderRecord->GetScheduler();
            return scheduler && scheduler->AsObject() == object;
        });
    if (it != appRunningRecordMap_.end()) {
        auto appRecord = it->second;
        auto renderRecord = appRecord->GetRenderRecord();
        appRecord->SetRenderRecord(nullptr);
        return renderRecord;
    }
    return nullptr;
}

bool AppRunningManager::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            HILOG_DEBUG("Process of [%{public}s] is running, processName: %{public}s.",
                bundleName.c_str(), appRecord->GetProcessName().c_str());
            return true;
        }
    }
    return false;
}

int32_t AppRunningManager::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    int32_t result = ERR_OK;
    bool loadSucceed = false;
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new (std::nothrow) QuickFixCallbackWithRecord(callback);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            HILOG_DEBUG("Notify application [%{public}s] load patch, record id %{public}d.",
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
    HILOG_DEBUG("function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    int32_t result = ERR_OK;
    bool reloadPageSucceed = false;
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new (std::nothrow) QuickFixCallbackWithRecord(callback);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            HILOG_DEBUG("Notify application [%{public}s] reload page, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyHotReloadPage(callback, recordId);
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
    HILOG_DEBUG("function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    int32_t result = ERR_OK;
    bool unLoadSucceed = false;
    sptr<QuickFixCallbackWithRecord> callbackByRecord = new (std::nothrow) QuickFixCallbackWithRecord(callback);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName) {
            auto recordId = appRecord->GetRecordId();
            HILOG_DEBUG("Notify application [%{public}s] unload patch, record id %{public}d.",
                appRecord->GetProcessName().c_str(), recordId);
            callbackByRecord->AddRecordId(recordId);
            result = appRecord->NotifyUnLoadRepairPatch(bundleName, callback, recordId);
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
    HILOG_DEBUG("function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord == nullptr || appRecord->GetBundleName() != foregroundingRecord.GetBundleName()) {
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
    HILOG_DEBUG("function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        auto state = appRecord->GetState();
        if (appRecord && appRecord->GetBundleName() == bundleName &&
            state == ApplicationState::APP_STATE_FOREGROUND) {
            return false;
        }
    }
    return true;
}

bool AppRunningManager::IsApplicationFirstFocused(const AppRunningRecord &focusedRecord)
{
    HILOG_DEBUG("check focus function called.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
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
    HILOG_DEBUG("check is application unfocused.");
    std::lock_guard<std::recursive_mutex> guard(lock_);
    for (const auto &item : appRunningRecordMap_) {
        const auto &appRecord = item.second;
        if (appRecord && appRecord->GetBundleName() == bundleName && appRecord->GetFocusFlag()) {
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
