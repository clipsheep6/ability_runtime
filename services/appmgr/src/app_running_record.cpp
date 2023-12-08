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

#include "ability_window_configuration.h"
#include "app_running_record.h"
#include "app_mgr_service_inner.h"
#include "event_report.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
constexpr int32_t MAX_RESTART_COUNT = 3;
constexpr int32_t RESTART_INTERVAL_TIME = 120000;
const std::string LAUNCHER_NAME = "com.ohos.sceneboard";
}

int64_t AppRunningRecord::appEventId_ = 0;

RenderRecord::RenderRecord(pid_t hostPid, const std::string &renderParam,
                           int32_t ipcFd, int32_t sharedFd, int32_t crashFd,
                           const std::shared_ptr<AppRunningRecord> &host)
    : hostPid_(hostPid), renderParam_(renderParam), ipcFd_(ipcFd),
      sharedFd_(sharedFd), crashFd_(crashFd), host_(host) {}

RenderRecord::~RenderRecord()
{
    close(sharedFd_);
    close(ipcFd_);
    close(crashFd_);
}

std::shared_ptr<RenderRecord> RenderRecord::CreateRenderRecord(
    pid_t hostPid, const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, int32_t crashFd,
    const std::shared_ptr<AppRunningRecord> &host)
{
    if (hostPid <= 0 || renderParam.empty() || ipcFd <= 0 || sharedFd <= 0 ||
        crashFd <= 0 || !host) {
        return nullptr;
    }

    auto renderRecord = std::make_shared<RenderRecord>(
        hostPid, renderParam, ipcFd, sharedFd, crashFd, host);
    renderRecord->SetHostUid(host->GetUid());
    renderRecord->SetHostBundleName(host->GetBundleName());
    renderRecord->SetProcessName(host->GetProcessName());
    return renderRecord;
}

void RenderRecord::SetPid(pid_t pid)
{
    pid_ = pid;
}

pid_t RenderRecord::GetPid() const
{
    return pid_;
}

pid_t RenderRecord::GetHostPid() const
{
    return hostPid_;
}

void RenderRecord::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t RenderRecord::GetUid() const
{
    return uid_;
}

void RenderRecord::SetHostUid(const int32_t hostUid)
{
    hostUid_ = hostUid;
}

int32_t RenderRecord::GetHostUid() const
{
    return hostUid_;
}

void RenderRecord::SetHostBundleName(const std::string &hostBundleName)
{
    hostBundleName_ = hostBundleName;
}

std::string RenderRecord::GetHostBundleName() const
{
    return hostBundleName_;
}

void RenderRecord::SetProcessName(const std::string &hostProcessName)
{
    processName_ = hostProcessName;
}

std::string RenderRecord::GetProcessName() const
{
    return processName_;
}

std::string RenderRecord::GetRenderParam() const
{
    return renderParam_;
}

int32_t RenderRecord::GetIpcFd() const
{
    return ipcFd_;
}

int32_t RenderRecord::GetSharedFd() const
{
    return sharedFd_;
}

int32_t RenderRecord::GetCrashFd() const
{
    return crashFd_;
}

ProcessType RenderRecord::GetProcessType() const
{
    return processType_;
}

std::shared_ptr<AppRunningRecord> RenderRecord::GetHostRecord() const
{
    return host_.lock();
}

sptr<IRenderScheduler> RenderRecord::GetScheduler() const
{
    return renderScheduler_;
}

void RenderRecord::SetScheduler(const sptr<IRenderScheduler> &scheduler)
{
    renderScheduler_ = scheduler;
}

void RenderRecord::SetDeathRecipient(const sptr<AppDeathRecipient> recipient)
{
    deathRecipient_ = recipient;
}

void RenderRecord::RegisterDeathRecipient()
{
    if (renderScheduler_ && deathRecipient_) {
        auto obj = renderScheduler_->AsObject();
        if (!obj || !obj->AddDeathRecipient(deathRecipient_)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
    }
}

AppRunningRecord::AppRunningRecord(
    const std::shared_ptr<ApplicationInfo> &info, const int32_t recordId, const std::string &processName)
    : appRecordId_(recordId), processName_(processName)
{
    if (info) {
        appInfo_ = info;
        mainBundleName_ = info->bundleName;
        isLauncherApp_ = info->isLauncherApp;
        mainAppName_ = info->name;
    }

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    startTimeMillis_ = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
}

void AppRunningRecord::SetApplicationClient(const sptr<IAppScheduler> &thread)
{
    if (!appLifeCycleDeal_) {
        appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    }
    appLifeCycleDeal_->SetApplicationClient(thread);

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        HILOG_ERROR("moduleRecordList is empty");
        return;
    }
    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
    }
}

const std::string &AppRunningRecord::GetBundleName() const
{
    return mainBundleName_;
}

int32_t AppRunningRecord::GetCallerPid() const
{
    return callerPid_;
}

void AppRunningRecord::SetCallerPid(int32_t pid)
{
    callerPid_ = pid;
}

int32_t AppRunningRecord::GetCallerUid() const
{
    return callerUid_;
}

void AppRunningRecord::SetCallerUid(int32_t uid)
{
    callerUid_ = uid;
}

int32_t AppRunningRecord::GetCallerTokenId() const
{
    return callerTokenId_;
}

void AppRunningRecord::SetCallerTokenId(int32_t tokenId)
{
    callerTokenId_ = tokenId;
}

bool AppRunningRecord::IsLauncherApp() const
{
    return isLauncherApp_;
}

int32_t AppRunningRecord::GetRecordId() const
{
    return appRecordId_;
}

const std::string &AppRunningRecord::GetName() const
{
    return mainAppName_;
}

const std::string &AppRunningRecord::GetSignCode() const
{
    return signCode_;
}

void AppRunningRecord::SetSignCode(const std::string &signCode)
{
    signCode_ = signCode;
}

const std::string &AppRunningRecord::GetJointUserId() const
{
    return jointUserId_;
}

void AppRunningRecord::SetJointUserId(const std::string &jointUserId)
{
    jointUserId_ = jointUserId;
}

const std::string &AppRunningRecord::GetProcessName() const
{
    return processName_;
}

void AppRunningRecord::SetSpecifiedProcessFlag(const std::string &flag)
{
    specifiedProcessFlag_ = flag;
}

const std::string &AppRunningRecord::GetSpecifiedProcessFlag() const
{
    return specifiedProcessFlag_;
}

int32_t AppRunningRecord::GetUid() const
{
    return mainUid_;
}

void AppRunningRecord::SetUid(const int32_t uid)
{
    mainUid_ = uid;
}

ApplicationState AppRunningRecord::GetState() const
{
    return curState_;
}

void AppRunningRecord::SetState(const ApplicationState state)
{
    if (state >= ApplicationState::APP_STATE_END) {
        HILOG_ERROR("Invalid application state");
        return;
    }
    if (state == ApplicationState::APP_STATE_FOREGROUND || state == ApplicationState::APP_STATE_BACKGROUND) {
        restartResidentProcCount_ = MAX_RESTART_COUNT;
    }
    curState_ = state;
}

void AppRunningRecord::SetRestartTimeMillis(const int64_t restartTimeMillis)
{
    restartTimeMillis_ = restartTimeMillis;
}

const std::list<std::shared_ptr<ApplicationInfo>> AppRunningRecord::GetAppInfoList()
{
    std::list<std::shared_ptr<ApplicationInfo>> appInfoList;
    std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
    for (const auto &item : appInfos_) {
        appInfoList.push_back(item.second);
    }
    return appInfoList;
}

const std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> AppRunningRecord::GetAbilities()
{
    std::map<const sptr<IRemoteObject>, std::shared_ptr<AbilityRunningRecord>> abilitysMap;
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilities = moduleRecord->GetAbilities();
        abilitysMap.insert(abilities.begin(), abilities.end());
    }
    return abilitysMap;
}

sptr<IAppScheduler> AppRunningRecord::GetApplicationClient() const
{
    return (appLifeCycleDeal_ ? appLifeCycleDeal_->GetApplicationClient() : nullptr);
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecord(const int64_t eventId) const
{
    HILOG_INFO("Get ability running record by eventId.");
    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        auto abilityRecord = moduleRecord->GetAbilityRunningRecord(eventId);
        if (abilityRecord) {
            return abilityRecord;
        }
    }

    return nullptr;
}

void AppRunningRecord::RemoveModuleRecord(const std::shared_ptr<ModuleRunningRecord> &moduleRecord)
{
    HILOG_INFO("Remove module record.");

    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    for (auto &item : hapModules_) {
        auto iter = std::find_if(item.second.begin(),
            item.second.end(),
            [&moduleRecord](const std::shared_ptr<ModuleRunningRecord> &record) { return moduleRecord == record; });
        if (iter != item.second.end()) {
            HILOG_DEBUG("Removed a record.");
            iter = item.second.erase(iter);
            if (item.second.empty()) {
                {
                    std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
                    HILOG_DEBUG("Removed an appInfo.");
                    appInfos_.erase(item.first);
                }
                hapModules_.erase(item.first);
            }
            return;
        }
    }
}

void AppRunningRecord::ForceKillApp([[maybe_unused]] const std::string &reason) const
{}

void AppRunningRecord::ScheduleAppCrash([[maybe_unused]] const std::string &description) const
{}

void AppRunningRecord::LaunchApplication(const Configuration &config)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        HILOG_ERROR("appThread is null");
        return;
    }
    AppLaunchData launchData;
    {
        std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
        auto moduleRecords = appInfos_.find(mainBundleName_);
        if (moduleRecords != appInfos_.end()) {
            launchData.SetApplicationInfo(*(moduleRecords->second));
        }
    }
    ProcessInfo processInfo(processName_, GetPriorityObject()->GetPid());
    launchData.SetProcessInfo(processInfo);
    launchData.SetRecordId(appRecordId_);
    launchData.SetUId(mainUid_);
    launchData.SetUserTestInfo(userTestRecord_);
    launchData.SetAppIndex(appIndex_);
    launchData.SetDebugApp(isDebugApp_);
    launchData.SetPerfCmd(perfCmd_);
    HILOG_INFO("Schedule launch application, app is %{public}s.", GetName().c_str());
    appLifeCycleDeal_->LaunchApplication(launchData, config);
}

void AppRunningRecord::UpdateApplicationInfoInstalled(const ApplicationInfo &appInfo)
{
    if (!isStageBasedModel_) {
        HILOG_INFO("Current version than supports !");
        return;
    }

    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return;
    }
    appLifeCycleDeal_->UpdateApplicationInfoInstalled(appInfo);
}

void AppRunningRecord::AddAbilityStage()
{
    if (!isStageBasedModel_) {
        HILOG_INFO("Current version than supports !");
        return;
    }
    HapModuleInfo abilityStage;
    if (GetTheModuleInfoNeedToUpdated(mainBundleName_, abilityStage)) {
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG, AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT);
        HILOG_INFO("Current Informed module : [%{public}s] | bundle : [%{public}s]",
            abilityStage.moduleName.c_str(), mainBundleName_.c_str());
        if (appLifeCycleDeal_ == nullptr) {
            HILOG_WARN("appLifeCycleDeal_ is null");
            return;
        }
        appLifeCycleDeal_->AddAbilityStage(abilityStage);
    }
}

void AppRunningRecord::AddAbilityStageBySpecifiedAbility(const std::string &bundleName)
{
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    HapModuleInfo hapModuleInfo;
    if (GetTheModuleInfoNeedToUpdated(bundleName, hapModuleInfo)) {
        if (startProcessSpecifiedAbilityEventId_ == 0) {
            HILOG_INFO("%{public}s START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG is not exist.", __func__);
            SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG,
                AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT);
        }
        if (appLifeCycleDeal_ == nullptr) {
            HILOG_WARN("appLifeCycleDeal_ is null");
            return;
        }
        appLifeCycleDeal_->AddAbilityStage(hapModuleInfo);
    }
}

void AppRunningRecord::AddAbilityStageBySpecifiedProcess(const std::string &bundleName)
{
    HILOG_DEBUG("call.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    HapModuleInfo hapModuleInfo;
    if (GetTheModuleInfoNeedToUpdated(bundleName, hapModuleInfo)) {
        SendEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG,
            AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT);
        if (appLifeCycleDeal_ == nullptr) {
            HILOG_WARN("appLifeCycleDeal_ is null");
            return;
        }
        appLifeCycleDeal_->AddAbilityStage(hapModuleInfo);
    }
}

void AppRunningRecord::AddAbilityStageDone()
{
    HILOG_INFO("Add ability stage done. bundle %{public}s and eventId %{public}d", mainBundleName_.c_str(),
        static_cast<int>(eventId_));

    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    if (startProcessSpecifiedAbilityEventId_ != 0) {
        eventHandler_->RemoveEvent(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG,
            startProcessSpecifiedAbilityEventId_);
        startProcessSpecifiedAbilityEventId_ = 0;
    }
    if (addAbilityStageInfoEventId_ != 0) {
        eventHandler_->RemoveEvent(AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG,
            addAbilityStageInfoEventId_);
        addAbilityStageInfoEventId_ = 0;
    }
    // Should proceed to the next notification

    if (isSpecifiedAbility_) {
        ScheduleAcceptWant(moduleName_);
        return;
    }

    if (isNewProcessRequest_) {
        HILOG_DEBUG("ScheduleNewProcessRequest.");
        ScheduleNewProcessRequest(newProcessRequestWant_, moduleName_);
        return;
    }

    AddAbilityStage();
}

void AppRunningRecord::LaunchAbility(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return;
    }
    if (!ability || !ability->GetToken()) {
        HILOG_ERROR("abilityRecord or abilityToken is nullptr.");
        return;
    }

    auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
    if (!moduleRecord) {
        HILOG_ERROR("moduleRecord is nullptr");
        return;
    }

    moduleRecord->LaunchAbility(ability);
}

void AppRunningRecord::ScheduleTerminate()
{
    SendEvent(AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG, AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT);
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_WARN("appLifeCycleDeal_ is null");
        return;
    }
    appLifeCycleDeal_->ScheduleTerminate();
}

void AppRunningRecord::LaunchPendingAbilities()
{
    HILOG_DEBUG("Launch pending abilities.");

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        HILOG_ERROR("moduleRecordList is empty");
        return;
    }
    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
        moduleRecord->LaunchPendingAbilities();
    }
}
void AppRunningRecord::ScheduleForegroundRunning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleForegroundRunning();
    }
}

void AppRunningRecord::ScheduleBackgroundRunning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleBackgroundRunning();
    }
    isAbilityForegrounding_.store(false);
}

void AppRunningRecord::ScheduleProcessSecurityExit()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleProcessSecurityExit();
    }
}

void AppRunningRecord::ScheduleTrimMemory()
{
    if (appLifeCycleDeal_ && priorityObject_) {
        appLifeCycleDeal_->ScheduleTrimMemory(priorityObject_->GetTimeLevel());
    }
}

void AppRunningRecord::ScheduleMemoryLevel(int32_t level)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleMemoryLevel(level);
    }
}

void AppRunningRecord::ScheduleHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->ScheduleHeapMemory(pid, mallocInfo);
    }
}

void AppRunningRecord::LowMemoryWarning()
{
    if (appLifeCycleDeal_) {
        appLifeCycleDeal_->LowMemoryWarning();
    }
}

void AppRunningRecord::AddModules(
    const std::shared_ptr<ApplicationInfo> &appInfo, const std::vector<HapModuleInfo> &moduleInfos)
{
    HILOG_DEBUG("Add modules");

    if (moduleInfos.empty()) {
        HILOG_INFO("moduleInfos is empty.");
        return;
    }

    for (auto &iter : moduleInfos) {
        AddModule(appInfo, nullptr, nullptr, iter, nullptr);
    }
}

void AppRunningRecord::AddModule(const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const sptr<IRemoteObject> &token,
    const HapModuleInfo &hapModuleInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    HILOG_INFO("Add module.");

    if (!appInfo) {
        HILOG_ERROR("appInfo is null");
        return;
    }

    std::shared_ptr<ModuleRunningRecord> moduleRecord;

    auto initModuleRecord = [=](const std::shared_ptr<ModuleRunningRecord> &moduleRecord) {
        moduleRecord->Init(hapModuleInfo);
        moduleRecord->SetAppMgrServiceInner(appMgrServiceInner_);
        moduleRecord->SetApplicationClient(appLifeCycleDeal_);
    };

    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    const auto &iter = hapModules_.find(appInfo->bundleName);
    if (iter != hapModules_.end()) {
        moduleRecord = GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, eventHandler_);
            iter->second.push_back(moduleRecord);
            initModuleRecord(moduleRecord);
        }
    } else {
        moduleRecord = std::make_shared<ModuleRunningRecord>(appInfo, eventHandler_);
        std::vector<std::shared_ptr<ModuleRunningRecord>> moduleList;
        moduleList.push_back(moduleRecord);
        hapModules_.emplace(appInfo->bundleName, moduleList);
        {
            std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
            appInfos_.emplace(appInfo->bundleName, appInfo);
        }
        initModuleRecord(moduleRecord);
    }

    if (!abilityInfo || !token) {
        HILOG_ERROR("abilityInfo or token is nullptr");
        return;
    }
    moduleRecord->AddAbility(token, abilityInfo, want);

    return;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRecordByModuleName(
    const std::string bundleName, const std::string &moduleName)
{
    HILOG_INFO("Get module record by module name.");
    auto moduleRecords = hapModules_.find(bundleName);
    if (moduleRecords != hapModules_.end()) {
        for (auto &iter : moduleRecords->second) {
            if (iter->GetModuleName() == moduleName) {
                return iter;
            }
        }
    }

    return nullptr;
}

void AppRunningRecord::StateChangedNotifyObserver(
    const std::shared_ptr<AbilityRunningRecord> &ability,
    const int32_t state,
    bool isAbility,
    bool isFromWindowFocusChanged)
{
    if (!ability || ability->GetAbilityInfo() == nullptr) {
        HILOG_ERROR("ability is null");
        return;
    }
    AbilityStateData abilityStateData;
    abilityStateData.bundleName = ability->GetAbilityInfo()->applicationInfo.bundleName;
    abilityStateData.moduleName = ability->GetAbilityInfo()->moduleName;
    abilityStateData.abilityName = ability->GetName();
    abilityStateData.pid = GetPriorityObject()->GetPid();
    abilityStateData.abilityState = state;
    abilityStateData.uid = ability->GetAbilityInfo()->applicationInfo.uid;
    abilityStateData.token = ability->GetToken();
    abilityStateData.abilityType = static_cast<int32_t>(ability->GetAbilityInfo()->type);
    abilityStateData.isFocused = ability->GetFocusFlag();
    if (ability->GetWant() != nullptr) {
        abilityStateData.callerAbilityName = ability->GetWant()->GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME);
        abilityStateData.callerBundleName = ability->GetWant()->GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    }

    if (isAbility && ability->GetAbilityInfo()->type == AbilityType::EXTENSION) {
        HILOG_INFO("extension type, not notify any more.");
        return;
    }
    auto serviceInner = appMgrServiceInner_.lock();
    if (serviceInner) {
        serviceInner->StateChangedNotifyObserver(abilityStateData, isAbility, isFromWindowFocusChanged);
    }
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        HILOG_ERROR("token is null");
        return nullptr;
    }

    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->GetAbilityRunningRecordByToken(token)) {
            return moduleRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<ModuleRunningRecord> AppRunningRecord::GetModuleRunningRecordByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    if (!token) {
        HILOG_ERROR("token is null");
        return nullptr;
    }

    auto moduleRecordList = GetAllModuleRecord();
    for (const auto &moduleRecord : moduleRecordList) {
        if (moduleRecord && moduleRecord->GetAbilityByTerminateLists(token)) {
            return moduleRecord;
        }
    }

    return nullptr;
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityRunningRecordByToken(
    const sptr<IRemoteObject> &token) const
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        return nullptr;
    }
    return moduleRecord->GetAbilityRunningRecordByToken(token);
}

std::shared_ptr<AbilityRunningRecord> AppRunningRecord::GetAbilityByTerminateLists(
    const sptr<IRemoteObject> &token) const
{
    auto moduleRecord = GetModuleRunningRecordByTerminateLists(token);
    if (!moduleRecord) {
        return nullptr;
    }
    return moduleRecord->GetAbilityByTerminateLists(token);
}

bool AppRunningRecord::UpdateAbilityFocusState(const sptr<IRemoteObject> &token, bool isFocus)
{
    HILOG_INFO("focus state is :%{public}d", isFocus);
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("can not find ability record");
        return false;
    }

    bool lastFocusState = abilityRecord->GetFocusFlag();
    if (lastFocusState == isFocus) {
        HILOG_ERROR("focus state not change, no need update");
        return false;
    }

    if (isFocus) {
        return AbilityFocused(abilityRecord);
    } else {
        return AbilityUnfocused(abilityRecord);
    }
}

void AppRunningRecord::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    HILOG_INFO("state is :%{public}d", static_cast<int32_t>(state));
    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("can not find ability record");
        return;
    }
    if (state == AbilityState::ABILITY_STATE_CREATE) {
        StateChangedNotifyObserver(
            abilityRecord, static_cast<int32_t>(AbilityState::ABILITY_STATE_CREATE), true, false);
        return;
    }
    if (state == abilityRecord->GetState()) {
        HILOG_ERROR("current state is already, no need update");
        return;
    }

    if (state == AbilityState::ABILITY_STATE_FOREGROUND) {
        AbilityForeground(abilityRecord);
    } else if (state == AbilityState::ABILITY_STATE_BACKGROUND) {
        AbilityBackground(abilityRecord);
    } else {
        HILOG_WARN("wrong state");
    }
}

void AppRunningRecord::AbilityForeground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        HILOG_ERROR("ability is null");
        return;
    }
    AbilityState curAbilityState = ability->GetState();
    if (curAbilityState != AbilityState::ABILITY_STATE_READY &&
        curAbilityState != AbilityState::ABILITY_STATE_BACKGROUND) {
        HILOG_ERROR("ability state(%{public}d) error", static_cast<int32_t>(curAbilityState));
        return;
    }

    HILOG_INFO("appState: %{public}d, bundle: %{public}s, ability: %{public}s",
        curState_, mainBundleName_.c_str(), ability->GetName().c_str());
    // We need schedule application to foregrounded when current application state is ready or background running.
    if (curState_ == ApplicationState::APP_STATE_FOREGROUND
        && pendingState_ != ApplicationPendingState::BACKGROUNDING) {
        // Just change ability to foreground if current application state is foreground or focus.
        auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
        moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_FOREGROUND);
        StateChangedNotifyObserver(ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND), true, false);
        auto serviceInner = appMgrServiceInner_.lock();
        if (serviceInner) {
            serviceInner->OnAppStateChanged(shared_from_this(), curState_, false, false);
        }
        return;
    }
    if (curState_ == ApplicationState::APP_STATE_READY || curState_ == ApplicationState::APP_STATE_BACKGROUND
        || curState_ == ApplicationState::APP_STATE_FOREGROUND) {
        if (foregroundingAbilityTokens_.empty() || pendingState_ == ApplicationPendingState::BACKGROUNDING) {
            HILOG_INFO("application foregrounding.");
            SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
            ScheduleForegroundRunning();
        }
        foregroundingAbilityTokens_.insert(ability->GetToken());
        HILOG_INFO("foregroundingAbility size: %{public}d", static_cast<int32_t>(foregroundingAbilityTokens_.size()));
        if (curState_ == ApplicationState::APP_STATE_BACKGROUND) {
            SendAppStartupTypeEvent(ability, AppStartType::HOT);
        }
    } else {
        HILOG_WARN("wrong application state");
    }
}

void AppRunningRecord::AbilityBackground(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        HILOG_ERROR("ability is null");
        return;
    }
    HILOG_INFO("ability is %{public}s", mainBundleName_.c_str());
    if (ability->GetState() != AbilityState::ABILITY_STATE_FOREGROUND &&
        ability->GetState() != AbilityState::ABILITY_STATE_READY) {
        HILOG_ERROR("ability state is not foreground or focus");
        return;
    }

    // First change ability to background.
    auto moduleRecord = GetModuleRunningRecordByToken(ability->GetToken());
    moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_BACKGROUND);
    StateChangedNotifyObserver(ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_BACKGROUND), true, false);
    if (curState_ == ApplicationState::APP_STATE_FOREGROUND) {
        int32_t foregroundSize = 0;
        auto abilitiesMap = GetAbilities();
        for (const auto &item : abilitiesMap) {
            const auto &abilityRecord = item.second;
            if (abilityRecord && abilityRecord->GetState() == AbilityState::ABILITY_STATE_FOREGROUND &&
                abilityRecord->GetAbilityInfo() &&
                (abilityRecord->GetAbilityInfo()->type == AppExecFwk::AbilityType::PAGE
                || AAFwk::UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo()->extensionAbilityType))) {
                foregroundSize++;
                break;
            }
        }



        // Then schedule application background when all ability is not foreground.
        if (foregroundSize == 0 && mainBundleName_ != LAUNCHER_NAME && windowIds_.empty()) {
            SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
            ScheduleBackgroundRunning();
        }
    } else {
        HILOG_WARN("wrong application state");
    }
}

bool AppRunningRecord::AbilityFocused(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        HILOG_ERROR("ability is null");
        return false;
    }
    ability->UpdateFocusState(true);

    // update ability state
    int32_t abilityState = static_cast<int32_t>(ability->GetState());
    bool isAbility = true;
    if (ability->GetAbilityInfo() != nullptr && ability->GetAbilityInfo()->type == AbilityType::EXTENSION) {
        isAbility = false;
    }
    StateChangedNotifyObserver(ability, abilityState, isAbility, true);

    if (isFocused_) {
        // process state is already focused, no need update process state.
        return false;
    }

    // update process focus state to true.
    isFocused_ = true;
    return true;
}

bool AppRunningRecord::AbilityUnfocused(const std::shared_ptr<AbilityRunningRecord> &ability)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!ability) {
        HILOG_ERROR("ability is null");
        return false;
    }
    ability->UpdateFocusState(false);

    // update ability state to unfocused.
    int32_t abilityState = static_cast<int32_t>(ability->GetState());
    bool isAbility = true;
    if (ability->GetAbilityInfo() != nullptr && ability->GetAbilityInfo()->type == AbilityType::EXTENSION) {
        isAbility = false;
    }
    StateChangedNotifyObserver(ability, abilityState, isAbility, true);

    if (!isFocused_) {
        return false; // invalid process focus state, already unfocused, process state not change.
    }

    bool changeProcessToUnfocused = true;
    auto abilitysMap = GetAbilities();
    for (const auto &item : abilitysMap) {
        const auto &abilityRecord = item.second;
        if (abilityRecord && abilityRecord->GetFocusFlag()) {
            changeProcessToUnfocused = false;
            break;
        }
    }

    if (changeProcessToUnfocused) {
        isFocused_ = false; // process focus state : from focus to unfocus.
    }
    return changeProcessToUnfocused;
}

void AppRunningRecord::PopForegroundingAbilityTokens()
{
    HILOG_INFO("foregroundingAbility size: %{public}d", static_cast<int32_t>(foregroundingAbilityTokens_.size()));
    for (auto iter = foregroundingAbilityTokens_.begin(); iter != foregroundingAbilityTokens_.end();) {
        auto ability = GetAbilityRunningRecordByToken(*iter);
        auto moduleRecord = GetModuleRunningRecordByToken(*iter);
        moduleRecord->OnAbilityStateChanged(ability, AbilityState::ABILITY_STATE_FOREGROUND);
        StateChangedNotifyObserver(ability, static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND), true, false);
        iter = foregroundingAbilityTokens_.erase(iter);
    }
}

void AppRunningRecord::TerminateAbility(const sptr<IRemoteObject> &token, const bool isForce)
{
    HILOG_INFO("Terminate ability, isForce: %{public}d", static_cast<int>(isForce));

    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        HILOG_ERROR("can not find module record");
        return;
    }

    auto abilityRecord = GetAbilityRunningRecordByToken(token);
    StateChangedNotifyObserver(
        abilityRecord, static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true, false);
    moduleRecord->TerminateAbility(shared_from_this(), token, isForce);
}

void AppRunningRecord::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("AbilityTerminated come.");
    auto moduleRecord = GetModuleRunningRecordByTerminateLists(token);
    if (!moduleRecord) {
        HILOG_ERROR("AbilityTerminated error, can not find module record");
        return;
    }

    bool isExtensionDebug = false;
    auto abilityRecord = moduleRecord->GetAbilityByTerminateLists(token);
    if (abilityRecord != nullptr && abilityRecord->GetAbilityInfo() != nullptr) {
        isExtensionDebug = (abilityRecord->GetAbilityInfo()->type == AppExecFwk::AbilityType::EXTENSION) &&
                           (isAttachDebug_ || isDebugApp_);
    }
    HILOG_DEBUG("Extension debug is [%{public}s]", isExtensionDebug ? "true" : "false");

    moduleRecord->AbilityTerminated(token);

    if (moduleRecord->GetAbilities().empty() && (!IsKeepAliveApp()
        || AAFwk::UIExtensionUtils::IsUIExtension(GetExtensionType()))) {
        RemoveModuleRecord(moduleRecord);
    }

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty() && (!IsKeepAliveApp()
        || AAFwk::UIExtensionUtils::IsUIExtension(GetExtensionType())) && !isExtensionDebug) {
        ScheduleTerminate();
    }
}

std::list<std::shared_ptr<ModuleRunningRecord>> AppRunningRecord::GetAllModuleRecord() const
{
    std::list<std::shared_ptr<ModuleRunningRecord>> moduleRecordList;
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    HILOG_DEBUG("Begin.");
    for (const auto &item : hapModules_) {
        for (const auto &list : item.second) {
            moduleRecordList.push_back(list);
        }
    }
    HILOG_DEBUG("End.");
    return moduleRecordList;
}

void AppRunningRecord::RegisterAppDeathRecipient() const
{
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        HILOG_ERROR("appThread is nullptr");
        return;
    }
    auto object = appLifeCycleDeal_->GetApplicationClient()->AsObject();
    if (!object || !object->AddDeathRecipient(appDeathRecipient_)) {
        HILOG_ERROR("AddDeathRecipient failed.");
    }
}

void AppRunningRecord::RemoveAppDeathRecipient() const
{
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return;
    }
    if (!appLifeCycleDeal_->GetApplicationClient()) {
        HILOG_ERROR("appThread is nullptr.");
        return;
    }
    auto object = appLifeCycleDeal_->GetApplicationClient()->AsObject();
    if (object) {
        object->RemoveDeathRecipient(appDeathRecipient_);
    }
}

void AppRunningRecord::SetAppMgrServiceInner(const std::weak_ptr<AppMgrServiceInner> &inner)
{
    appMgrServiceInner_ = inner;

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.empty()) {
        HILOG_ERROR("moduleRecordList is empty");
        return;
    }

    for (const auto &moduleRecord : moduleRecordList) {
        moduleRecord->SetAppMgrServiceInner(appMgrServiceInner_);
    }
}

void AppRunningRecord::SetAppDeathRecipient(const sptr<AppDeathRecipient> &appDeathRecipient)
{
    appDeathRecipient_ = appDeathRecipient;
}

std::shared_ptr<PriorityObject> AppRunningRecord::GetPriorityObject()
{
    if (!priorityObject_) {
        priorityObject_ = std::make_shared<PriorityObject>();
    }

    return priorityObject_;
}

void AppRunningRecord::SendEventForSpecifiedAbility(uint32_t msg, int64_t timeOut)
{
    SendEvent(msg, timeOut);
}

void AppRunningRecord::SendAppStartupTypeEvent(const std::shared_ptr<AbilityRunningRecord> &ability,
    const AppStartType startType)
{
    if (!ability) {
        HILOG_ERROR("AbilityRunningRecord is nullptr");
        return;
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = GetApplicationInfo();
    if (!applicationInfo) {
        HILOG_ERROR("applicationInfo is nullptr, can not get app information");
    } else {
        eventInfo.bundleName = applicationInfo->name;
        eventInfo.versionName = applicationInfo->versionName;
        eventInfo.versionCode = applicationInfo->versionCode;
    }

    auto abilityInfo = ability->GetAbilityInfo();
    if (!abilityInfo) {
        HILOG_ERROR("abilityInfo is nullptr, can not get ability information");
    } else {
        eventInfo.abilityName = abilityInfo->name;
    }
    if (GetPriorityObject() == nullptr) {
        HILOG_ERROR("appRecord's priorityObject is null");
    } else {
        eventInfo.pid = GetPriorityObject()->GetPid();
    }
    eventInfo.startType = static_cast<int32_t>(startType);
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_STARTUP_TYPE, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppRunningRecord::SendEvent(uint32_t msg, int64_t timeOut)
{
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    if (isDebugApp_ || isNativeDebug_ || isAttachDebug_) {
        HILOG_INFO("Is debug mode, no need to handle time out.");
        return;
    }

    appEventId_++;
    eventId_ = appEventId_;
    if (msg == AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG) {
        startProcessSpecifiedAbilityEventId_ = eventId_;
    }
    if (msg == AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG) {
        addAbilityStageInfoEventId_ = eventId_;
    }

    HILOG_INFO("eventId %{public}d", static_cast<int>(eventId_));
    eventHandler_->SendEvent(AAFwk::EventWrap(msg, eventId_), timeOut, false);
    SendClearTask(msg, timeOut);
}

void AppRunningRecord::SendClearTask(uint32_t msg, int64_t timeOut)
{
    if (!taskHandler_) {
        HILOG_ERROR("taskHandler_ is nullptr");
        return;
    }
    int64_t* eventId = nullptr;
    if (msg == AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG) {
        eventId = &startProcessSpecifiedAbilityEventId_;
    } else if (msg == AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG) {
        eventId = &addAbilityStageInfoEventId_;
    } else {
        HILOG_INFO("Other msg: %{public}d", msg);
        return;
    }
    taskHandler_->SubmitTask([wthis = weak_from_this(), eventId]() {
        auto pthis = wthis.lock();
        if (pthis) {
            *eventId = 0;
        }
        }, timeOut);
}

void AppRunningRecord::PostTask(std::string msg, int64_t timeOut, const Closure &task)
{
    if (!taskHandler_) {
        HILOG_ERROR("taskHandler_ is nullptr");
        return;
    }
    taskHandler_->SubmitTask(task, msg, timeOut);
}

int64_t AppRunningRecord::GetEventId() const
{
    return eventId_;
}

void AppRunningRecord::SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler)
{
    taskHandler_ = taskHandler;
}

void AppRunningRecord::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

bool AppRunningRecord::IsLastAbilityRecord(const sptr<IRemoteObject> &token)
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        HILOG_ERROR("can not find module record");
        return false;
    }

    auto moduleRecordList = GetAllModuleRecord();
    if (moduleRecordList.size() == 1) {
        return moduleRecord->IsLastAbilityRecord(token);
    }

    return false;
}

bool AppRunningRecord::IsLastPageAbilityRecord(const sptr<IRemoteObject> &token)
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        HILOG_ERROR("can not find module record");
        return false;
    }

    int32_t pageAbilitySize = 0;
    auto moduleRecordList = GetAllModuleRecord();
    for (auto moduleRecord : moduleRecordList) {
        pageAbilitySize += moduleRecord->GetPageAbilitySize() ;
        if (pageAbilitySize > 1) {
            return false;
        }
    }

    return pageAbilitySize == 1;
}

void AppRunningRecord::SetTerminating()
{
    isTerminating = true;
}

bool AppRunningRecord::IsTerminating()
{
    return isTerminating;
}

bool AppRunningRecord::IsKeepAliveApp() const
{
    return isKeepAliveApp_;
}

bool AppRunningRecord::IsEmptyKeepAliveApp() const
{
    return isEmptyKeepAliveApp_;
}

void AppRunningRecord::SetKeepAliveAppState(bool isKeepAlive, bool isEmptyKeepAliveApp)
{
    isKeepAliveApp_ = isKeepAlive;
    isEmptyKeepAliveApp_ = isEmptyKeepAliveApp;
}

void AppRunningRecord::SetStageModelState(bool isStageBasedModel)
{
    isStageBasedModel_ = isStageBasedModel;
}

bool AppRunningRecord::GetTheModuleInfoNeedToUpdated(const std::string bundleName, HapModuleInfo &info)
{
    bool result = false;
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    auto moduleInfoVectorIter = hapModules_.find(bundleName);
    if (moduleInfoVectorIter == hapModules_.end() || moduleInfoVectorIter->second.empty()) {
        return result;
    }
    std::string moduleName = moduleName_;
    auto findCondition = [moduleName](const std::shared_ptr<ModuleRunningRecord> &record) {
        if (record) {
            return (moduleName.empty() || (moduleName == record->GetModuleName())) &&
                (record->GetModuleRecordState() == ModuleRecordState::INITIALIZED_STATE);
        }
        return false;
    };
    auto moduleRecordIter =
        std::find_if(moduleInfoVectorIter->second.begin(), moduleInfoVectorIter->second.end(), findCondition);
    if (moduleRecordIter != moduleInfoVectorIter->second.end()) {
        (*moduleRecordIter)->GetHapModuleInfo(info);
        (*moduleRecordIter)->SetModuleRecordState(ModuleRecordState::RUNNING_STATE);
        result = true;
    }

    return result;
}

void AppRunningRecord::SetRestartResidentProcCount(int count)
{
    restartResidentProcCount_ = count;
}

void AppRunningRecord::DecRestartResidentProcCount()
{
    restartResidentProcCount_--;
}

int AppRunningRecord::GetRestartResidentProcCount() const
{
    return restartResidentProcCount_;
}

bool AppRunningRecord::CanRestartResidentProc()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t systemTimeMillis = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    if ((restartResidentProcCount_ >= 0) || ((systemTimeMillis - restartTimeMillis_) > RESTART_INTERVAL_TIME)) {
        return true;
    }
    return false;
}

void AppRunningRecord::GetBundleNames(std::vector<std::string> &bundleNames)
{
    std::lock_guard<ffrt::mutex> appInfosLock(appInfosLock_);
    HILOG_DEBUG("Begin.");
    for (auto &app : appInfos_) {
        bundleNames.emplace_back(app.first);
    }
    HILOG_DEBUG("End.");
}

void AppRunningRecord::SetUserTestInfo(const std::shared_ptr<UserTestRecord> &record)
{
    userTestRecord_ = record;
}

std::shared_ptr<UserTestRecord> AppRunningRecord::GetUserTestInfo()
{
    return userTestRecord_;
}

void AppRunningRecord::SetProcessAndExtensionType(const std::shared_ptr<AbilityInfo> &abilityInfo)
{
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }
    extensionType_ = abilityInfo->extensionAbilityType;
    if (extensionType_ == ExtensionAbilityType::UNSPECIFIED) {
        processType_ = ProcessType::NORMAL;
        return;
    }
    processType_ = ProcessType::EXTENSION;
    return;
}

void AppRunningRecord::SetSpecifiedAbilityFlagAndWant(
    const bool flag, const AAFwk::Want &want, const std::string &moduleName)
{
    isSpecifiedAbility_ = flag;
    SpecifiedWant_ = want;
    moduleName_ = moduleName;
}

void AppRunningRecord::SetScheduleNewProcessRequestState(
    const bool isNewProcessRequest, const AAFwk::Want &want, const std::string &moduleName)
{
    isNewProcessRequest_ = isNewProcessRequest;
    newProcessRequestWant_ = want;
    moduleName_ = moduleName;
}

bool AppRunningRecord::IsNewProcessRequest() const
{
    return isNewProcessRequest_;
}

bool AppRunningRecord::IsStartSpecifiedAbility() const
{
    return isSpecifiedAbility_;
}

void AppRunningRecord::ScheduleAcceptWant(const std::string &moduleName)
{
    SendEvent(
        AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG, AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT);
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_WARN("appLifeCycleDeal_ is null");
        return;
    }
    appLifeCycleDeal_->ScheduleAcceptWant(SpecifiedWant_, moduleName);
}

void AppRunningRecord::ScheduleAcceptWantDone()
{
    HILOG_INFO("Schedule accept want done. bundle %{public}s and eventId %{public}d", mainBundleName_.c_str(),
        static_cast<int>(eventId_));

    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    eventHandler_->RemoveEvent(AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG, eventId_);
}

void AppRunningRecord::ScheduleNewProcessRequest(const AAFwk::Want &want, const std::string &moduleName)
{
    SendEvent(
        AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG, AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT);
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_WARN("appLifeCycleDeal_ is null");
        return;
    }
    appLifeCycleDeal_->ScheduleNewProcessRequest(want, moduleName);
}

void AppRunningRecord::ScheduleNewProcessRequestDone()
{
    HILOG_INFO("ScheduleNewProcessRequestDone. bundle %{public}s and eventId %{public}d",
        mainBundleName_.c_str(), static_cast<int>(eventId_));

    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    eventHandler_->RemoveEvent(AMSEventHandler::START_SPECIFIED_PROCESS_TIMEOUT_MSG, eventId_);
}

void AppRunningRecord::ApplicationTerminated()
{
    HILOG_DEBUG("Application terminated bundle %{public}s and eventId %{public}d", mainBundleName_.c_str(),
        static_cast<int>(eventId_));

    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    eventHandler_->RemoveEvent(AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG, eventId_);
}

const AAFwk::Want &AppRunningRecord::GetSpecifiedWant() const
{
    return SpecifiedWant_;
}

const AAFwk::Want &AppRunningRecord::GetNewProcessRequestWant() const
{
    return newProcessRequestWant_;
}

int32_t AppRunningRecord::UpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("call %{public}s", __func__);
    if (!appLifeCycleDeal_) {
        HILOG_INFO("appLifeCycleDeal_ is null");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->UpdateConfiguration(config);
}

void AppRunningRecord::AddRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
    if (!record) {
        HILOG_DEBUG("AddRenderRecord: record is null");
        return;
    }
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    renderRecordMap_.emplace(record->GetUid(), record);
}

void AppRunningRecord::RemoveRenderRecord(const std::shared_ptr<RenderRecord> &record)
{
    if (!record) {
        HILOG_DEBUG("RemoveRenderRecord: record is null");
        return;
    }
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    renderRecordMap_.erase(record->GetUid());
}

std::shared_ptr<RenderRecord> AppRunningRecord::GetRenderRecordByPid(const pid_t pid)
{
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    if (renderRecordMap_.empty()) {
        return nullptr;
    }
    for (auto iter : renderRecordMap_) {
        auto renderRecord = iter.second;
        if (renderRecord && renderRecord->GetPid() == pid) {
            return renderRecord;
        }
    }
    return nullptr;
}

std::map<int32_t, std::shared_ptr<RenderRecord>> AppRunningRecord::GetRenderRecordMap()
{
    std::lock_guard renderRecordMapLock(renderRecordMapLock_);
    return renderRecordMap_;
}

void AppRunningRecord::SetStartMsg(const AppSpawnStartMsg &msg)
{
    startMsg_ = msg;
}

AppSpawnStartMsg AppRunningRecord::GetStartMsg()
{
    return startMsg_;
}

void AppRunningRecord::SetDebugApp(bool isDebugApp)
{
    HILOG_INFO("SetDebugApp come, value is %{public}d", isDebugApp);
    isDebugApp_ = isDebugApp;
}

bool AppRunningRecord::IsDebugApp()
{
    return isDebugApp_;
}

void AppRunningRecord::SetNativeDebug(bool isNativeDebug)
{
    HILOG_DEBUG("SetNativeDebug, value is %{public}d", isNativeDebug);
    isNativeDebug_ = isNativeDebug;
}

void AppRunningRecord::SetPerfCmd(const std::string &perfCmd)
{
    perfCmd_ = perfCmd;
}

void AppRunningRecord::SetAppIndex(const int32_t appIndex)
{
    appIndex_ = appIndex;
}

void AppRunningRecord::GetSplitModeAndFloatingMode(bool &isSplitScreenMode, bool &isFloatingWindowMode)
{
    auto abilitiesMap = GetAbilities();
    isSplitScreenMode = false;
    isFloatingWindowMode = false;
    for (const auto &item : abilitiesMap) {
        const auto &abilityRecord = item.second;
        if (abilityRecord == nullptr) {
            continue;
        }
        const auto &abilityWant = abilityRecord->GetWant();
        if (abilityWant != nullptr) {
            int windowMode = abilityWant->GetIntParam(Want::PARAM_RESV_WINDOW_MODE, -1);
            if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_FLOATING) {
                isFloatingWindowMode = true;
            }
            if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
                windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
                isSplitScreenMode = true;
            }
        }
        if (isFloatingWindowMode && isSplitScreenMode) {
            break;
        }
    }
}

int32_t AppRunningRecord::GetAppIndex() const
{
    return appIndex_;
}

void AppRunningRecord::SetSecurityFlag(bool securityFlag)
{
    securityFlag_ = securityFlag;
}

bool AppRunningRecord::GetSecurityFlag() const
{
    return securityFlag_;
}

void AppRunningRecord::SetKilling()
{
    isKilling_ = true;
}

bool AppRunningRecord::IsKilling() const
{
    return isKilling_;
}

void AppRunningRecord::RemoveTerminateAbilityTimeoutTask(const sptr<IRemoteObject>& token) const
{
    auto moduleRecord = GetModuleRunningRecordByToken(token);
    if (!moduleRecord) {
        HILOG_ERROR("can not find module record");
        return;
    }
    (void)moduleRecord->RemoveTerminateAbilityTimeoutTask(token);
}

int32_t AppRunningRecord::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback,
    const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appLifeCycleDeal_) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppRunningRecord::NotifyHotReloadPage(const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appLifeCycleDeal_) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyHotReloadPage(callback, recordId);
}

int32_t AppRunningRecord::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appLifeCycleDeal_) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyUnLoadRepairPatch(bundleName, callback, recordId);
}

int32_t AppRunningRecord::NotifyAppFault(const FaultData &faultData)
{
    HILOG_DEBUG("called.");
    if (!appLifeCycleDeal_) {
        HILOG_ERROR("appLifeCycleDeal_ is null");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->NotifyAppFault(faultData);
}

bool AppRunningRecord::IsAbilitytiesBackground()
{
    std::lock_guard<ffrt::mutex> hapModulesLock(hapModulesLock_);
    for (const auto &iter : hapModules_) {
        for (const auto &moduleRecord : iter.second) {
            if (moduleRecord == nullptr) {
                HILOG_ERROR("Module record is nullptr.");
                continue;
            }
            if (!moduleRecord->IsAbilitiesBackgrounded()) {
                return false;
            }
        }
    }
    return true;
}

void AppRunningRecord::OnWindowVisibilityChanged(
    const std::vector<sptr<OHOS::Rosen::WindowVisibilityInfo>> &windowVisibilityInfos)
{
    HILOG_DEBUG("Called.");
    if (windowVisibilityInfos.empty()) {
        HILOG_WARN("Window visibility info is empty.");
        return;
    }

    for (const auto &info : windowVisibilityInfos) {
        if (info == nullptr) {
            HILOG_ERROR("Window visibility info is nullptr.");
            continue;
        }
        if (info->pid_ != GetPriorityObject()->GetPid()) {
            continue;
        }
        auto iter = windowIds_.find(info->windowId_);
        if (iter != windowIds_.end() &&
            info->visibilityState_ == OHOS::Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            windowIds_.erase(iter);
            continue;
        }
        if (iter == windowIds_.end() &&
            info->visibilityState_ < OHOS::Rosen::WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION) {
            windowIds_.emplace(info->windowId_);
        }
    }

    if (!windowIds_.empty() && curState_ != ApplicationState::APP_STATE_FOREGROUND) {
        SetUpdateStateFromService(true);
        ScheduleForegroundRunning();
        return;
    }

    if (windowIds_.empty() && IsAbilitytiesBackground() && curState_ == ApplicationState::APP_STATE_FOREGROUND) {
        SetUpdateStateFromService(true);
        ScheduleBackgroundRunning();
    }
}

bool AppRunningRecord::IsContinuousTask()
{
    return isContinuousTask_;
}

void AppRunningRecord::SetContinuousTaskAppState(bool isContinuousTask)
{
    isContinuousTask_ = isContinuousTask;
}

bool AppRunningRecord::GetFocusFlag() const
{
    return isFocused_;
}

int64_t AppRunningRecord::GetAppStartTime() const
{
    return startTimeMillis_;
}

void AppRunningRecord::SetRequestProcCode(int32_t requestProcCode)
{
    requestProcCode_ = requestProcCode;
}

int32_t AppRunningRecord::GetRequestProcCode() const
{
    return requestProcCode_;
}

void AppRunningRecord::SetProcessChangeReason(ProcessChangeReason reason)
{
    processChangeReason_ = reason;
}

ProcessChangeReason AppRunningRecord::GetProcessChangeReason() const
{
    return processChangeReason_;
}

bool AppRunningRecord::IsUpdateStateFromService()
{
    return isUpdateStateFromService_;
}

void AppRunningRecord::SetUpdateStateFromService(bool isUpdateStateFromService)
{
    isUpdateStateFromService_ = isUpdateStateFromService;
}

ExtensionAbilityType AppRunningRecord::GetExtensionType() const
{
    return extensionType_;
}

ProcessType AppRunningRecord::GetProcessType() const
{
    return processType_;
}

int32_t AppRunningRecord::ChangeAppGcState(const int32_t state)
{
    HILOG_DEBUG("called.");
    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is nullptr.");
        return ERR_INVALID_VALUE;
    }
    return appLifeCycleDeal_->ChangeAppGcState(state);
}

void AppRunningRecord::SetAttachDebug(const bool &isAttachDebug)
{
    HILOG_DEBUG("Called.");
    isAttachDebug_ = isAttachDebug;

    if (appLifeCycleDeal_ == nullptr) {
        HILOG_ERROR("appLifeCycleDeal_ is nullptr.");
        return;
    }
    isAttachDebug_ ? appLifeCycleDeal_->AttachAppDebug() : appLifeCycleDeal_->DetachAppDebug();
}

bool AppRunningRecord::isAttachDebug() const
{
    return isAttachDebug_;
}

void AppRunningRecord::SetApplicationPendingState(ApplicationPendingState pendingState)
{
    pendingState_ = pendingState;
}

ApplicationPendingState AppRunningRecord::GetApplicationPendingState() const
{
    return pendingState_;
}

void AppRunningRecord::AddChildProcessRecord(pid_t pid, const std::shared_ptr<ChildProcessRecord> record)
{
    if (!record) {
        HILOG_ERROR("record is null.");
        return;
    }
    if (pid <= 0) {
        HILOG_ERROR("pid <= 0.");
        return;
    }
    std::lock_guard lock(childProcessRecordMapLock_);
    childProcessRecordMap_.emplace(pid, record);
}

void AppRunningRecord::RemoveChildProcessRecord(const std::shared_ptr<ChildProcessRecord> record)
{
    HILOG_INFO("Removing child process record, pid: %{public}d", record->GetPid());
    if (!record) {
        HILOG_ERROR("record is null.");
        return;
    }
    auto pid = record->GetPid();
    if (pid <= 0) {
        HILOG_ERROR("record.pid <= 0.");
        return;
    }
    std::lock_guard lock(childProcessRecordMapLock_);
    childProcessRecordMap_.erase(pid);
}

std::shared_ptr<ChildProcessRecord> AppRunningRecord::GetChildProcessRecordByPid(const pid_t pid)
{
    std::lock_guard lock(childProcessRecordMapLock_);
    auto iter = childProcessRecordMap_.find(pid);
    if (iter == childProcessRecordMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

std::map<int32_t, std::shared_ptr<ChildProcessRecord>> AppRunningRecord::GetChildProcessRecordMap()
{
    std::lock_guard lock(childProcessRecordMapLock_);
    return childProcessRecordMap_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
