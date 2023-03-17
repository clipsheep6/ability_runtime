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

#include "app_mgr_service_inner.h"

#include <cinttypes>
#include <csignal>
#include <securec.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_mem_info.h"
#include "app_mgr_service.h"
#include "app_process_data.h"
#include "app_state_observer_manager.h"
#include "application_state_observer_stub.h"
#include "bundle_constants.h"
#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "datetime_ex.h"
#include "event_report.h"
#include "hilog_wrapper.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "iservice_registry.h"
#include "itest_observer.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#endif
#include "parameter.h"
#include "parameters.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "uri_permission_manager_client.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::Rosen;
using namespace OHOS::Security;

namespace {
#define CHECK_CALLER_IS_SYSTEM_APP                                                             \
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) { \
        HILOG_ERROR("The caller is not system-app, can not use system-api");                   \
        return AAFwk::ERR_NOT_SYSTEM_APP;                                                             \
    }

// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 milli second
constexpr int64_t MICROSECONDS = 1000000;
// Kill process timeout setting
constexpr int KILL_PROCESS_TIMEOUT_MICRO_SECONDS = 1000;
// Kill process delay time setting
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 200;
// delay register focus listener to wms
constexpr int REGISTER_FOCUS_DELAY = 5000;
const std::string CLASS_NAME = "ohos.app.MainThread";
const std::string FUNC_NAME = "main";
const std::string SO_PATH = "system/lib64/libmapleappkit.z.so";
const std::string RENDER_PARAM = "invalidparam";
const std::string COLD_START = "coldStart";
const std::string DLP_PARAMS_INDEX = "ohos.dlp.params.index";
const std::string PERMISSION_INTERNET = "ohos.permission.INTERNET";
const std::string PERMISSION_ACCESS_BUNDLE_DIR = "ohos.permission.ACCESS_BUNDLE_DIR";
const std::string DLP_PARAMS_SECURITY_FLAG = "ohos.dlp.params.securityFlag";
const int32_t SIGNAL_KILL = 9;
constexpr int32_t USER_SCALE = 200000;
#define ENUM_TO_STRING(s) #s
#define APP_ACCESS_BUNDLE_DIR 0x20

constexpr int32_t BASE_USER_RANGE = 200000;

constexpr int32_t MAX_RESTART_COUNT = 3;
constexpr int32_t RESTART_INTERVAL_TIME = 120000;

constexpr ErrCode APPMGR_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, 0x01);
constexpr ErrCode ERR_ALREADY_EXIST_RENDER = APPMGR_ERR_OFFSET + 100; // error code for already exist render.
const std::string EVENT_NAME_LIFECYCLE_TIMEOUT = "APP_LIFECYCLE_TIMEOUT";
constexpr char EVENT_KEY_UID[] = "UID";
constexpr char EVENT_KEY_PID[] = "PID";
constexpr char EVENT_KEY_PACKAGE_NAME[] = "PACKAGE_NAME";
constexpr char EVENT_KEY_PROCESS_NAME[] = "PROCESS_NAME";
constexpr char EVENT_KEY_MESSAGE[] = "MSG";

// Msg length is less than 48 characters
const std::string EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT = "Terminate Ability TimeOut!";
const std::string EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT = "Terminate Application TimeOut!";
const std::string EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT = "Add Ability Stage TimeOut!";
const std::string EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT = "Start Specified Ability TimeOut!";
const std::string EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT = "Start Process Specified Ability TimeOut!";
const std::string EVENT_MESSAGE_DEFAULT = "AppMgrServiceInner HandleTimeOut!";

const std::string SYSTEM_BASIC = "system_basic";
const std::string SYSTEM_CORE = "system_core";
const std::string ABILITY_OWNER_USERID = "AbilityMS_Owner_UserId";
const std::string PROCESS_EXIT_EVENT_TASK = "Send Process Exit Event Task";

constexpr int32_t ROOT_UID = 0;
constexpr int32_t FOUNDATION_UID = 5523;

int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}
}  // namespace

using OHOS::AppExecFwk::Constants::PERMISSION_GRANTED;
using OHOS::AppExecFwk::Constants::PERMISSION_NOT_GRANTED;

AppMgrServiceInner::AppMgrServiceInner()
    : appProcessManager_(std::make_shared<AppProcessManager>()),
      remoteClientManager_(std::make_shared<RemoteClientManager>()),
      appRunningManager_(std::make_shared<AppRunningManager>()),
      configuration_(std::make_shared<Configuration>())
{}

void AppMgrServiceInner::Init()
{
    InitGlobalConfiguration();
    AddWatchParameter();
    DelayedSingleton<AppStateObserverManager>::GetInstance()->Init();
    InitFocusListener();
}

AppMgrServiceInner::~AppMgrServiceInner()
{}

void AppMgrServiceInner::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AAFwk::Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!CheckLoadAbilityConditions(token, abilityInfo, appInfo)) {
        HILOG_ERROR("CheckLoadAbilityConditions failed");
        return;
    }
    HILOG_INFO("AppMgrService start loading ability, name is %{public}s.", abilityInfo->name.c_str());

    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = (want == nullptr) ? 0 : want->GetIntParam(DLP_PARAMS_INDEX, 0);
    if (!GetBundleAndHapInfo(*abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        HILOG_ERROR("GetBundleAndHapInfo failed");
        return;
    }

    std::string processName;
    MakeProcessName(abilityInfo, appInfo, hapModuleInfo, appIndex, processName);

    auto appRecord =
        appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (!appRecord) {
        appRecord = CreateAppRunningRecord(token, preToken, appInfo, abilityInfo,
            processName, bundleInfo, hapModuleInfo, want);
        if (!appRecord) {
            HILOG_ERROR("CreateAppRunningRecord failed, appRecord is nullptr");
            return;
        }
        uint32_t startFlags = (want == nullptr) ? 0 : BuildStartFlags(*want, *abilityInfo);
        int32_t bundleIndex = (want == nullptr) ? 0 : want->GetIntParam(DLP_PARAMS_INDEX, 0);
        StartProcess(abilityInfo->applicationName, processName, startFlags, appRecord,
            appInfo->uid, appInfo->bundleName, bundleIndex);
    } else {
        int32_t requestProcCode = (want == nullptr) ? 0 : want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0);
        if (requestProcCode != 0 && appRecord->GetRequestProcCode() == 0) {
            appRecord->SetRequestProcCode(requestProcCode);
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessReused(appRecord);
        }
        StartAbility(token, preToken, abilityInfo, appRecord, hapModuleInfo, want);
    }
    PerfProfile::GetInstance().SetAbilityLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
    PerfProfile::GetInstance().Reset();
}

bool AppMgrServiceInner::CheckLoadAbilityConditions(const sptr<IRemoteObject> &token,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo)
{
    if (!token || !abilityInfo || !appInfo) {
        HILOG_ERROR("param error");
        return false;
    }
    if (abilityInfo->name.empty() || appInfo->name.empty()) {
        HILOG_ERROR("error abilityInfo or appInfo");
        return false;
    }
    if (abilityInfo->applicationName != appInfo->name) {
        HILOG_ERROR("abilityInfo and appInfo have different appName, don't load for it");
        return false;
    }

    return true;
}

void AppMgrServiceInner::MakeProcessName(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, int32_t appIndex,
    std::string &processName)
{
    if (!abilityInfo || !appInfo) {
        HILOG_ERROR("param error");
        return;
    }
    if (!abilityInfo->process.empty()) {
        processName = abilityInfo->process;
        return;
    }
    MakeProcessName(appInfo, hapModuleInfo, processName);
    if (appIndex != 0) {
        processName += std::to_string(appIndex);
    }
}

void AppMgrServiceInner::MakeProcessName(
    const std::shared_ptr<ApplicationInfo> &appInfo, const HapModuleInfo &hapModuleInfo, std::string &processName)
{
    if (!appInfo) {
        return;
    }
    // check after abilityInfo, because abilityInfo contains extension process.
    if (hapModuleInfo.isStageBasedModel && !hapModuleInfo.process.empty()) {
        processName = hapModuleInfo.process;
        HILOG_INFO("Stage mode, Make processName:%{public}s", processName.c_str());
        return;
    }
    if (!appInfo->process.empty()) {
        processName = appInfo->process;
        return;
    }
    processName = appInfo->bundleName;
}

bool AppMgrServiceInner::GetBundleAndHapInfo(const AbilityInfo &abilityInfo,
    const std::shared_ptr<ApplicationInfo> &appInfo, BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo,
    int32_t appIndex)
{
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return false;
    }

    auto userId = GetUserIdByUid(appInfo->uid);
    HILOG_INFO("GetBundleAndHapInfo come, call bms GetBundleInfo and GetHapModuleInfo, userId is %{public}d", userId);
    bool bundleMgrResult;
    if (appIndex == 0) {
        bundleMgrResult = IN_PROCESS_CALL(bundleMgr_->GetBundleInfo(appInfo->bundleName,
            BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId));
    } else {
        bundleMgrResult = (IN_PROCESS_CALL(bundleMgr_->GetSandboxBundleInfo(appInfo->bundleName,
            appIndex, userId, bundleInfo)) == 0);
    }

    if (!bundleMgrResult) {
        HILOG_ERROR("GetBundleInfo is fail");
        return false;
    }
    if (appIndex == 0) {
        bundleMgrResult = bundleMgr_->GetHapModuleInfo(abilityInfo, userId, hapModuleInfo);
    } else {
        bundleMgrResult = (bundleMgr_->GetSandboxHapModuleInfo(abilityInfo, appIndex, userId, hapModuleInfo) == 0);
    }
    if (!bundleMgrResult) {
        HILOG_ERROR("GetHapModuleInfo is fail");
        return false;
    }

    return true;
}

void AppMgrServiceInner::AttachApplication(const pid_t pid, const sptr<IAppScheduler> &appScheduler)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (pid <= 0) {
        HILOG_ERROR("invalid pid:%{public}d", pid);
        return;
    }
    if (!appScheduler) {
        HILOG_ERROR("app client is null");
        return;
    }
    HILOG_INFO("AppMgrService attach application come, pid is %{public}d.", pid);
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord");
        return;
    }
    appRecord->SetApplicationClient(appScheduler);
    appRecord->RegisterAppDeathRecipient();
    if (appRecord->GetState() == ApplicationState::APP_STATE_CREATE) {
        LaunchApplication(appRecord);
    }
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.bundleName = applicationInfo->name;
    eventInfo.versionName = applicationInfo->versionName;
    eventInfo.versionCode = applicationInfo->versionCode;
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_ATTACH, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::LaunchApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        HILOG_ERROR("appRecord is null");
        return;
    }

    if (!configuration_) {
        HILOG_ERROR("configuration_ is null");
        return;
    }

    if (appRecord->GetState() != ApplicationState::APP_STATE_CREATE) {
        HILOG_ERROR("wrong app state:%{public}d", appRecord->GetState());
        return;
    }

    appRecord->LaunchApplication(*configuration_);
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    int restartResidentProcCount = MAX_RESTART_COUNT;
    appRecord->SetRestartResidentProcCount(restartResidentProcCount);

    // There is no ability when the empty resident process starts
    // The status of all resident processes is ready
    // There is no process of switching the foreground, waiting for his first ability to start
    if (appRecord->IsEmptyKeepAliveApp()) {
        appRecord->AddAbilityStage();
        return;
    }

    if (appRecord->IsStartSpecifiedAbility()) {
        appRecord->AddAbilityStageBySpecifiedAbility(appRecord->GetBundleName());
        return;
    }
    appRecord->LaunchPendingAbilities();
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.bundleName = applicationInfo->name;
    eventInfo.versionName = applicationInfo->versionName;
    eventInfo.versionCode = applicationInfo->versionCode;
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_LAUNCH, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::AddAbilityStageDone(const int32_t recordId)
{
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        HILOG_ERROR("get app record failed");
        return;
    }
    appRecord->AddAbilityStageDone();
}

void AppMgrServiceInner::ApplicationForegrounded(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        HILOG_ERROR("get app record failed");
        return;
    }
    appRecord->PopForegroundingAbilityTokens();
    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_READY || appState == ApplicationState::APP_STATE_BACKGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
        bool needNotifyApp = appRunningManager_->IsApplicationFirstForeground(*appRecord);
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND, needNotifyApp);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
    } else {
        HILOG_WARN("app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(), static_cast<ApplicationState>(appState));
    }

    // push the foregrounded app front of RecentAppList.
    PushAppFront(recordId);
    HILOG_INFO("application is foregrounded");
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.bundleName = applicationInfo->name;
    eventInfo.versionName = applicationInfo->versionName;
    eventInfo.versionCode = applicationInfo->versionCode;
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_FOREGROUND, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::ApplicationBackgrounded(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        HILOG_ERROR("get app record failed");
        return;
    }
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {
        appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
        bool needNotifyApp = appRunningManager_->IsApplicationBackground(appRecord->GetBundleName());
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_BACKGROUND, needNotifyApp);
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
    } else {
        HILOG_WARN("app name(%{public}s), app state(%{public}d)!",
            appRecord->GetName().c_str(), static_cast<ApplicationState>(appRecord->GetState()));
    }

    HILOG_INFO("application is backgrounded");
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.bundleName = applicationInfo->name;
    eventInfo.versionName = applicationInfo->versionName;
    eventInfo.versionCode = applicationInfo->versionCode;
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_BACKGROUND, HiSysEventType::BEHAVIOR, eventInfo);
}

void AppMgrServiceInner::ApplicationTerminated(const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        HILOG_ERROR("get app record failed");
        return;
    }
    appRecord->ApplicationTerminated();
    // Maybe can't get in here
    if (appRecord->IsKeepAliveApp()) {
        return;
    }
    if (appRecord->GetState() != ApplicationState::APP_STATE_BACKGROUND) {
        HILOG_ERROR("current state is not background");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_APP_TERMINATED);
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED, false);
    appRunningManager_->RemoveAppRunningRecordById(recordId);
    RemoveAppFromRecentListById(recordId);
    AAFwk::EventInfo eventInfo;
    auto applicationInfo = appRecord->GetApplicationInfo();
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.bundleName = applicationInfo->name;
    eventInfo.versionName = applicationInfo->versionName;
    eventInfo.versionCode = applicationInfo->versionCode;
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }

    HILOG_INFO("application is terminated");
    SendProcessExitEvent(appRecord->GetPriorityObject()->GetPid());
}

int32_t AppMgrServiceInner::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    int32_t result = VerifyProcessPermission();
    if (result != ERR_OK) {
        HILOG_ERROR("Permission verification failed");
        return result;
    }

    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }

    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return ERR_NO_INIT;
    }
    auto userId = GetUserIdByUid(uid);
    ApplicationInfo appInfo;
    HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetApplicationInfo");
    bool bundleMgrResult = bundleMgr_->GetApplicationInfo(bundleName,
        ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo);
    if (!bundleMgrResult) {
        HILOG_ERROR("GetApplicationInfo is fail");
        return ERR_INVALID_OPERATION;
    }

    HILOG_DEBUG("uid value is %{public}d", uid);
    result = appRunningManager_->ProcessUpdateApplicationInfoInstalled(appInfo);
    if (result != ERR_OK) {
        HILOG_INFO("The process corresponding to the package name did not start");
    }

    return result;
}

int32_t AppMgrServiceInner::KillApplication(const std::string &bundleName)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    auto result = VerifyProcessPermission(bundleName);
    if (result != ERR_OK) {
        HILOG_ERROR("Permission verification failed.");
        return result;
    }

    return KillApplicationByBundleName(bundleName);
}

int32_t AppMgrServiceInner::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    auto result = VerifyProcessPermission(bundleName);
    if (result != ERR_OK) {
        HILOG_ERROR("Permission verification failed.");
        return result;
    }

    result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return ERR_NO_INIT;
    }
    HILOG_INFO("uid value is %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids)) {
        HILOG_INFO("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        HILOG_INFO("The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            HILOG_ERROR("KillApplication failed for bundleName:%{public}s pid:%{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

void AppMgrServiceInner::SendProcessExitEventTask(pid_t pid, time_t exitTime, int32_t count)
{
    auto exitResult = !ProcessExist(pid);
    constexpr int32_t EXIT_SUCESS = 0;
    constexpr int32_t EXIT_FAILED = -1;

    if (exitResult) {
        AAFwk::EventInfo eventInfo;
        eventInfo.time = exitTime;
        eventInfo.exitResult = EXIT_SUCESS;
        eventInfo.pid = pid;
        AAFwk::EventReport::SendAppEvent(AAFwk::EventName::PROCESS_EXIT, HiSysEventType::BEHAVIOR, eventInfo);
        HILOG_INFO("%{public}s. time : %{public}" PRId64 ", exitResult : %{public}d, pid : %{public}d",
            __func__, eventInfo.time, eventInfo.exitResult, eventInfo.pid);
        return;
    }

    if (--count <= 0) {
        AAFwk::EventInfo eventInfo;
        eventInfo.time = exitTime;
        eventInfo.exitResult = EXIT_FAILED;
        eventInfo.pid = pid;
        AAFwk::EventReport::SendAppEvent(AAFwk::EventName::PROCESS_EXIT, HiSysEventType::BEHAVIOR, eventInfo);
        HILOG_INFO("%{public}s. time : %{public}" PRId64 ", exitResult : %{public}d, pid : %{public}d",
            __func__, eventInfo.time, eventInfo.exitResult, eventInfo.pid);
        return;
    }

    auto sendEventTask = [inner = shared_from_this(), pid, exitTime, count] () {
        inner->SendProcessExitEventTask(pid, exitTime, count);
    };
    eventHandler_->PostTask(sendEventTask, PROCESS_EXIT_EVENT_TASK, KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
}

void AppMgrServiceInner::SendProcessExitEvent(pid_t pid)
{
    HILOG_INFO("%{public}s called.", __func__);
    time_t currentTime;
    time(&currentTime);
    constexpr int32_t RETRY_COUNT = 5;
    SendProcessExitEventTask(pid, currentTime, RETRY_COUNT);
    return;
}

int32_t AppMgrServiceInner::KillApplicationSelf()
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    auto bundleName = appRecord->GetBundleName();
    return KillApplicationByBundleName(bundleName);
}

int32_t AppMgrServiceInner::KillApplicationByBundleName(const std::string &bundleName)
{
    int result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;

    if (!appRunningManager_->ProcessExitByBundleName(bundleName, pids)) {
        HILOG_ERROR("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        HILOG_DEBUG("The remote process exited successfully ");
        NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            HILOG_ERROR("KillApplicationSelf is failed for bundleName:%{public}s, pid: %{public}d",
                bundleName.c_str(), *iter);
            return result;
        }
    }
    NotifyAppStatus(bundleName, EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_RESTARTED);
    return result;
}

int32_t AppMgrServiceInner::KillApplicationByUserId(const std::string &bundleName, const int userId)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    CHECK_CALLER_IS_SYSTEM_APP;
    if (VerifyAccountPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES, userId) == ERR_PERMISSION_DENIED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr = remoteClientManager_->GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return ERR_NO_INIT;
    }

    return KillApplicationByUserIdLocked(bundleName, userId);
}

int32_t AppMgrServiceInner::KillApplicationByUserIdLocked(const std::string &bundleName, const int userId)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }

    int result = ERR_OK;
    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ fail");
        return ERR_NO_INIT;
    }
    auto bundleMgr = remoteClientManager_->GetBundleManager();
    if (bundleMgr == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return ERR_NO_INIT;
    }

    HILOG_INFO("userId value is %{public}d", userId);
    int uid = IN_PROCESS_CALL(bundleMgr->GetUidByBundleName(bundleName, userId));
    HILOG_INFO("uid value is %{public}d", uid);
    if (!appRunningManager_->ProcessExitByBundleNameAndUid(bundleName, uid, pids)) {
        HILOG_INFO("The process corresponding to the package name did not start");
        return result;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        HILOG_INFO("The remote process exited successfully ");
        return result;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        result = KillProcessByPid(*iter);
        if (result < 0) {
            HILOG_ERROR("KillApplication is fail bundleName: %{public}s pid: %{public}d", bundleName.c_str(), *iter);
            return result;
        }
    }
    return result;
}

void AppMgrServiceInner::ClearUpApplicationData(const std::string &bundleName, int32_t callerUid, pid_t callerPid)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    auto userId = GetUserIdByUid(callerUid);
    HILOG_INFO("userId:%{public}d", userId);
    ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, userId);
}

void AppMgrServiceInner::ClearUpApplicationDataByUserId(
    const std::string &bundleName, int32_t callerUid, pid_t callerPid, const int userId)
{
    if (callerPid <= 0) {
        HILOG_ERROR("invalid callerPid:%{public}d", callerPid);
        return;
    }
    if (callerUid <= 0) {
        HILOG_ERROR("invalid callerUid:%{public}d", callerUid);
        return;
    }
    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return;
    }

    // request to clear user information permission.
    auto tokenId = AccessToken::AccessTokenKit::GetHapTokenID(userId, bundleName, 0);
    int32_t result = AccessToken::AccessTokenKit::ClearUserGrantedPermissionState(tokenId);
    if (result) {
        HILOG_ERROR("ClearUserGrantedPermissionState failed, ret:%{public}d", result);
        return;
    }
    // 2.delete bundle side user data
    if (!IN_PROCESS_CALL(bundleMgr_->CleanBundleDataFiles(bundleName, userId))) {
        HILOG_ERROR("Delete bundle side user data is fail");
        return;
    }
    // 3.kill application
    // 4.revoke user rights
    result = KillApplicationByUserId(bundleName, userId);
    if (result < 0) {
        HILOG_ERROR("Kill Application by bundle name is fail");
        return;
    }
    NotifyAppStatusByCallerUid(bundleName, userId, callerUid,
        EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_DATA_CLEARED);
}

int32_t AppMgrServiceInner::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    // check permission
    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        if (isPerm) {
            GetRunningProcesses(appRecord, info);
        } else {
            auto applicationInfo = appRecord->GetApplicationInfo();
            if (!applicationInfo) {
                continue;
            }
            auto callingTokenId = IPCSkeleton::GetCallingTokenID();
            auto tokenId = applicationInfo->accessTokenId;
            if (callingTokenId == tokenId) {
                GetRunningProcesses(appRecord, info);
            }
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (VerifyAccountPermission(AAFwk::PermissionConstants::PERMISSION_GET_RUNNING_INFO, userId) ==
        ERR_PERMISSION_DENIED) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    for (const auto &item : appRunningManager_->GetAppRunningRecordMap()) {
        const auto &appRecord = item.second;
        int32_t userIdTemp = static_cast<int32_t>(appRecord->GetUid() / USER_SCALE);
        if (userIdTemp == userId) {
            GetRunningProcesses(appRecord, info);
        }
    }
    return ERR_OK;
}

int32_t AppMgrServiceInner::GetProcessRunningInformation(RunningProcessInfo &info)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_NO_INIT;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord, callerPid:%{public}d", callerPid);
        return ERR_INVALID_VALUE;
    }
    GetRunningProcess(appRecord, info);
    return ERR_OK;
}

int32_t AppMgrServiceInner::NotifyMemoryLevel(int32_t level)
{
    HILOG_INFO("AppMgrServiceInner start");

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("callerToken not SA %{public}s", __func__);
        return ERR_INVALID_VALUE;
    }
    if (!(level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_MODERATE ||
        level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_CRITICAL ||
        level == OHOS::AppExecFwk::MemoryLevel::MEMORY_LEVEL_LOW)) {
        HILOG_ERROR("Level value error!");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager nullptr!");
        return ERR_INVALID_VALUE;
    }

    return appRunningManager_->NotifyMemoryLevel(level);
}

int32_t AppMgrServiceInner::DumpHeapMemory(std::vector<int32_t> &mallinfo)
{
    HILOG_INFO("AppMgrServiceInner DumpHeapMemory start");
    if (mallinfo.empty()) {
        HILOG_ERROR("mallinfo's pid nullptr or empty!");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager nullptr!");
        return ERR_INVALID_VALUE;
    }
    return appRunningManager_->DumpHeapMemory(mallinfo);
}

void AppMgrServiceInner::GetRunningProcesses(const std::shared_ptr<AppRunningRecord> &appRecord,
    std::vector<RunningProcessInfo> &info)
{
    RunningProcessInfo runningProcessInfo;
    GetRunningProcess(appRecord, runningProcessInfo);
    info.emplace_back(runningProcessInfo);
}

void AppMgrServiceInner::GetRunningProcess(const std::shared_ptr<AppRunningRecord> &appRecord,
    RunningProcessInfo &info)
{
    info.processName_ = appRecord->GetProcessName();
    info.pid_ = appRecord->GetPriorityObject()->GetPid();
    info.uid_ = appRecord->GetUid();
    info.state_ = static_cast<AppProcessState>(appRecord->GetState());
    info.isContinuousTask = appRecord->IsContinuousTask();
    info.isKeepAlive = appRecord->IsKeepAliveApp();
    info.isFocused = appRecord->GetFocusFlag();
    info.startTimeMillis_ = appRecord->GetAppStartTime();
    appRecord->GetBundleNames(info.bundleNames);
}

int32_t AppMgrServiceInner::KillProcessByPid(const pid_t pid) const
{
    int32_t ret = -1;
    if (pid > 0) {
        HILOG_INFO("kill pid %{public}d", pid);
        ret = kill(pid, SIGNAL_KILL);
    }
    AAFwk::EventInfo eventInfo;
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        return ret;
    }
    auto applicationInfo = appRecord->GetApplicationInfo();
    eventInfo.pid = appRecord->GetPriorityObject()->GetPid();
    eventInfo.bundleName = applicationInfo->name;
    eventInfo.versionName = applicationInfo->versionName;
    eventInfo.versionCode = applicationInfo->versionCode;
    eventInfo.processName = appRecord->GetProcessName();
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::APP_TERMINATE, HiSysEventType::BEHAVIOR, eventInfo);
    return ret;
}

bool AppMgrServiceInner::WaitForRemoteProcessExit(std::list<pid_t> &pids, const int64_t startTime)
{
    int64_t delayTime = SystemTimeMillisecond() - startTime;
    while (delayTime < KILL_PROCESS_TIMEOUT_MICRO_SECONDS) {
        if (CheckAllProcessExist(pids)) {
            return true;
        }
        usleep(KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
        delayTime = SystemTimeMillisecond() - startTime;
    }
    return false;
}

bool AppMgrServiceInner::GetAllPids(std::list<pid_t> &pids)
{
    for (const auto &appTaskInfo : appProcessManager_->GetRecentAppList()) {
        if (appTaskInfo) {
            auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
            if (appRecord) {
                pids.push_back(appTaskInfo->GetPid());
                appRecord->ScheduleProcessSecurityExit();
            }
        }
    }
    return (pids.empty() ? false : true);
}

bool AppMgrServiceInner::ProcessExist(pid_t &pid)
{
    char pid_path[128] = {0};
    struct stat stat_buf;
    if (!pid) {
        return false;
    }
    if (snprintf_s(pid_path, sizeof(pid_path), sizeof(pid_path) - 1, "/proc/%d/status", pid) < 0) {
        return false;
    }
    if (stat(pid_path, &stat_buf) == 0) {
        return true;
    }
    return false;
}

bool AppMgrServiceInner::CheckAllProcessExist(std::list<pid_t> &pids)
{
    for (auto iter = pids.begin(); iter != pids.end();) {
        if (!ProcessExist(*iter)) {
            iter = pids.erase(iter);
        } else {
            iter++;
        }
    }
    if (pids.empty()) {
        return true;
    }
    return false;
}

int64_t AppMgrServiceInner::SystemTimeMillisecond()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByPid(const pid_t pid) const
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager nullptr!");
        return nullptr;
    }
    return appRunningManager_->GetAppRunningRecordByPid(pid);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::CreateAppRunningRecord(const sptr<IRemoteObject> &token,
    const sptr<IRemoteObject> &preToken, const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::string &processName, const BundleInfo &bundleInfo,
    const HapModuleInfo &hapModuleInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager nullptr!");
        return nullptr;
    }
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo);
    if (!appRecord) {
        HILOG_ERROR("get app record failed");
        return nullptr;
    }

    bool isKeepAlive = bundleInfo.isKeepAlive && bundleInfo.singleton;
    appRecord->SetKeepAliveAppState(isKeepAlive, false);
    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want);
    if (want) {
        appRecord->SetDebugApp(want->GetBoolParam("debugApp", false));
        if (want->GetBoolParam(COLD_START, false)) {
            appRecord->SetDebugApp(true);
        }
        appRecord->SetAppIndex(want->GetIntParam(DLP_PARAMS_INDEX, 0));
        appRecord->SetSecurityFlag(want->GetBoolParam(DLP_PARAMS_SECURITY_FLAG, false));
        appRecord->SetRequestProcCode(want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0));
        appRecord->SetCallerPid(want->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1));
        appRecord->SetCallerUid(want->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1));
        appRecord->SetCallerTokenId(want->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, -1));
    }

    if (preToken) {
        auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
        if (abilityRecord) {
            abilityRecord->SetPreToken(preToken);
        }
    }

    return appRecord;
}

void AppMgrServiceInner::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Terminate ability come.");
    if (!token) {
        HILOG_ERROR("AppMgrServiceInner::TerminateAbility token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("AppMgrServiceInner::TerminateAbility app is not exist!");
        return;
    }

    if (appRunningManager_) {
        std::shared_ptr<AppMgrServiceInner> appMgrServiceInner = shared_from_this();
        appRunningManager_->TerminateAbility(token, clearMissionFlag, appMgrServiceInner);
    }
}

void AppMgrServiceInner::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("AppMgrService start to update the ability to state %{public}d.", static_cast<int32_t>(state));
    if (!token) {
        HILOG_ERROR("token is null!");
        return;
    }

    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("can not find ability record!");
        return;
    }
    if (state == abilityRecord->GetState()) {
        HILOG_ERROR("current state is already, no need update!");
        return;
    }
    if (abilityRecord->GetAbilityInfo() == nullptr) {
        HILOG_ERROR("ability info nullptr!");
        return;
    }
    auto type = abilityRecord->GetAbilityInfo()->type;
    if (type == AppExecFwk::AbilityType::SERVICE &&
        (state == AbilityState::ABILITY_STATE_CREATE ||
        state == AbilityState::ABILITY_STATE_TERMINATED ||
        state == AbilityState::ABILITY_STATE_CONNECTED ||
        state == AbilityState::ABILITY_STATE_DISCONNECTED)) {
        HILOG_INFO("StateChangedNotifyObserver service type, state:%{public}d", static_cast<int32_t>(state));
        appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), true);
        return;
    }
    if (state > AbilityState::ABILITY_STATE_BACKGROUND || state < AbilityState::ABILITY_STATE_FOREGROUND) {
        HILOG_ERROR("state is not foreground or background!");
        return;
    }

    appRecord->UpdateAbilityState(token, state);
}

void AppMgrServiceInner::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    if (!token) {
        HILOG_ERROR("token is null!");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("app is not exist!");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("can not find ability record!");
        return;
    }
    appRecord->StateChangedNotifyObserver(abilityRecord, static_cast<int32_t>(state), false);
}

void AppMgrServiceInner::OnStop()
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager nullptr!");
        return;
    }

    appRunningManager_->ClearAppRunningRecordMap();
    CloseAppSpawnConnection();
}

ErrCode AppMgrServiceInner::OpenAppSpawnConnection()
{
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ is null");
        return ERR_INVALID_VALUE;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->OpenConnection();
    }
    return ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT;
}

void AppMgrServiceInner::CloseAppSpawnConnection() const
{
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ is null");
        return;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        remoteClientManager_->GetSpawnClient()->CloseConnection();
    }
}

SpawnConnectionState AppMgrServiceInner::QueryAppSpawnConnectionState() const
{
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ is null");
        return SpawnConnectionState::STATE_NOT_CONNECT;
    }

    if (remoteClientManager_->GetSpawnClient()) {
        return remoteClientManager_->GetSpawnClient()->QueryConnectionState();
    }
    return SpawnConnectionState::STATE_NOT_CONNECT;
}

void AppMgrServiceInner::SetAppSpawnClient(std::shared_ptr<AppSpawnClient> spawnClient)
{
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ is null");
        return;
    }

    remoteClientManager_->SetSpawnClient(std::move(spawnClient));
}

void AppMgrServiceInner::SetBundleManager(sptr<IBundleMgr> bundleManager)
{
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ is null");
        return;
    }

    remoteClientManager_->SetBundleManager(bundleManager);
}

void AppMgrServiceInner::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        HILOG_ERROR("%{public}s: Not abilityMgr call.", __func__);
        return;
    }
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (callback != nullptr) {
        appStateCallbacks_.push_back(callback);
    }
}

void AppMgrServiceInner::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility,       // 0:false,1:true
    const int32_t perceptibility,   // 0:false,1:true
    const int32_t connectionState)  // 0:false,1:true
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        HILOG_ERROR("token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("app record is not exist for ability token");
        return;
    }
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    if (!abilityRecord) {
        HILOG_ERROR("ability record is not exist for ability previous token");
        return;
    }
    if (preToken) {
        abilityRecord->SetPreToken(preToken);
    }
    abilityRecord->SetVisibility(visibility);
    abilityRecord->SetPerceptibility(perceptibility);
    abilityRecord->SetConnectionState(connectionState);
}

void AppMgrServiceInner::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!token) {
        HILOG_ERROR("token is null");
        return;
    }
    auto appRecord = GetAppRunningRecordByAbilityToken(token);
    if (!appRecord) {
        HILOG_ERROR("app record is not exist for ability token");
        return;
    }

    // before exec ScheduleProcessSecurityExit return
    // The resident process won't let him die
    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        std::list<pid_t> pids;
        pids.push_back(pid);
        appRecord->ScheduleProcessSecurityExit();
        if (!WaitForRemoteProcessExit(pids, SystemTimeMillisecond())) {
            int32_t result = KillProcessByPid(pid);
            if (result < 0) {
                HILOG_ERROR("KillProcessByAbilityToken kill process is fail");
                return;
            }
        }
    }
}

void AppMgrServiceInner::KillProcessesByUserId(int32_t userId)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }

    int64_t startTime = SystemTimeMillisecond();
    std::list<pid_t> pids;
    if (!appRunningManager_->GetPidsByUserId(userId, pids)) {
        HILOG_INFO("The process corresponding to the userId did not start");
        return;
    }
    if (WaitForRemoteProcessExit(pids, startTime)) {
        HILOG_INFO("The remote process exited successfully ");
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        auto result = KillProcessByPid(*iter);
        if (result < 0) {
            HILOG_ERROR("KillProcessByPid is failed. pid: %{public}d", *iter);
            return;
        }
    }
}

void AppMgrServiceInner::StartAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<AppRunningRecord> &appRecord,
    const HapModuleInfo &hapModuleInfo, const std::shared_ptr<AAFwk::Want> &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("already create appRecord, just start ability");
    if (!appRecord) {
        HILOG_ERROR("appRecord is null");
        return;
    }

    if (want) {
        want->SetParam(DLP_PARAMS_SECURITY_FLAG, appRecord->GetSecurityFlag());
    }

    auto ability = appRecord->GetAbilityRunningRecordByToken(token);
    if (abilityInfo->launchMode == LaunchMode::SINGLETON && ability != nullptr) {
        HILOG_WARN("same ability info in singleton launch mode, will not add ability");
        return;
    }

    if (ability && preToken) {
        HILOG_ERROR("Ability is already started");
        ability->SetPreToken(preToken);
        return;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo->applicationInfo);
    appRecord->AddModule(appInfo, abilityInfo, token, hapModuleInfo, want);
    auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
    if (!moduleRecord) {
        HILOG_ERROR("add moduleRecord failed");
        return;
    }

    ability = moduleRecord->GetAbilityRunningRecordByToken(token);
    if (!ability) {
        HILOG_ERROR("add ability failed");
        return;
    }

    if (preToken != nullptr) {
        ability->SetPreToken(preToken);
    }

    ApplicationState appState = appRecord->GetState();
    if (appState == ApplicationState::APP_STATE_CREATE) {
        HILOG_ERROR("in create state, don't launch ability");
        return;
    }
    appRecord->LaunchAbility(ability);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAbilityToken(
    const sptr<IRemoteObject> &abilityToken) const
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return nullptr;
    }

    return appRunningManager_->GetAppRunningRecordByAbilityToken(abilityToken);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetTerminatingAppRunningRecord(
    const sptr<IRemoteObject> &token) const
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr.");
        return nullptr;
    }
    return appRunningManager_->GetTerminatingAppRunningRecord(token);
}

void AppMgrServiceInner::AbilityTerminated(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_DEBUG("Terminate ability come.");
    if (!token) {
        HILOG_ERROR("Terminate ability error, token is null!");
        return;
    }

    auto appRecord = appRunningManager_->GetTerminatingAppRunningRecord(token);
    if (!appRecord) {
        HILOG_ERROR("Terminate ability error, appRecord is not exist!");
        return;
    }

    appRecord->AbilityTerminated(token);
}

std::shared_ptr<AppRunningRecord> AppMgrServiceInner::GetAppRunningRecordByAppRecordId(const int32_t recordId) const
{
    if (appRunningManager_ == nullptr) {
        HILOG_ERROR("appRunningManager is nullptr");
        return nullptr;
    }
    const auto&& appRunningRecordMap = appRunningManager_->GetAppRunningRecordMap();
    const auto& iter = appRunningRecordMap.find(recordId);
    return iter != appRunningRecordMap.end() ? iter->second : nullptr;
}

void AppMgrServiceInner::OnAppStateChanged(
    const std::shared_ptr<AppRunningRecord> &appRecord, const ApplicationState state, bool needNotifyApp)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!appRecord) {
        HILOG_ERROR("OnAppStateChanged come, app record is null");
        return;
    }

    HILOG_DEBUG("OnAppStateChanged begin, bundleName is %{public}s, state:%{public}d",
        appRecord->GetBundleName().c_str(), static_cast<int32_t>(state));
    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAppStateChanged(WrapAppProcessData(appRecord, state));
        }
    }

    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnAppStateChanged(appRecord, state, needNotifyApp);
}

AppProcessData AppMgrServiceInner::WrapAppProcessData(const std::shared_ptr<AppRunningRecord> &appRecord,
    const ApplicationState state)
{
    AppProcessData processData;
    auto appInfoList = appRecord->GetAppInfoList();
    for (const auto &list : appInfoList) {
        AppData data;
        data.appName = list->name;
        data.uid = list->uid;
        processData.appDatas.push_back(data);
    }
    processData.processName = appRecord->GetProcessName();
    processData.pid = appRecord->GetPriorityObject()->GetPid();
    processData.appState = state;
    processData.isFocused = appRecord->GetFocusFlag();
    return processData;
}

void AppMgrServiceInner::OnAbilityStateChanged(
    const std::shared_ptr<AbilityRunningRecord> &ability, const AbilityState state)
{
    if (!ability) {
        HILOG_ERROR("ability is null");
        return;
    }
    for (const auto &callback : appStateCallbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityRequestDone(ability->GetToken(), state);
        }
    }
}

void AppMgrServiceInner::StateChangedNotifyObserver(const AbilityStateData abilityStateData, bool isAbility)
{
    DelayedSingleton<AppStateObserverManager>::GetInstance()->StateChangedNotifyObserver(abilityStateData, isAbility);
}

void AppMgrServiceInner::StartProcess(const std::string &appName, const std::string &processName, uint32_t startFlags,
                                      const std::shared_ptr<AppRunningRecord> &appRecord, const int uid,
                                      const std::string &bundleName, const int32_t bundleIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (!remoteClientManager_->GetSpawnClient() || !appRecord) {
        HILOG_ERROR("appSpawnClient or appRecord is null");
        return;
    }

    auto bundleMgr_ = remoteClientManager_->GetBundleManager();
    if (bundleMgr_ == nullptr) {
        HILOG_ERROR("GetBundleManager fail");
        return;
    }

    auto userId = GetUserIdByUid(uid);
    BundleInfo bundleInfo;
    bool bundleMgrResult;
    if (bundleIndex == 0) {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetBundleInfo");
        bundleMgrResult = IN_PROCESS_CALL(bundleMgr_->GetBundleInfo(bundleName,
            BundleFlag::GET_BUNDLE_WITH_REQUESTED_PERMISSION, bundleInfo, userId));
    } else {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "BMS->GetSandboxBundleInfo");
        bundleMgrResult = (IN_PROCESS_CALL(bundleMgr_->GetSandboxBundleInfo(bundleName,
            bundleIndex, userId, bundleInfo)) == 0);
    }

    if (!bundleMgrResult) {
        HILOG_ERROR("GetBundleInfo is fail");
        return;
    }

    HspList hspList;
    ErrCode ret = bundleMgr_->GetBaseSharedBundleInfos(bundleName, hspList);
    if (ret != ERR_OK) {
        HILOG_ERROR("GetBaseSharedBundleInfos failed: %d", ret);
        return;
    }

    bool hasAccessBundleDirReq = std::any_of(bundleInfo.reqPermissions.begin(), bundleInfo.reqPermissions.end(),
        [] (const auto &reqPermission) {
            if (PERMISSION_ACCESS_BUNDLE_DIR == reqPermission) {
                return true;
            }

            return false;
        });
    uint8_t setAllowInternet = 0;
    uint8_t allowInternet = 1;
    auto token = bundleInfo.applicationInfo.accessTokenId;
    {
        // Add TRACE
        HITRACE_METER_NAME(HITRACE_TAG_APP, "AccessTokenKit::VerifyAccessToken");
        int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_INTERNET);
        if (result != Security::AccessToken::PERMISSION_GRANTED) {
            setAllowInternet = 1;
            allowInternet = 0;
        }

        if (hasAccessBundleDirReq) {
            int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(token, PERMISSION_ACCESS_BUNDLE_DIR);
            if (result != Security::AccessToken::PERMISSION_GRANTED) {
                HILOG_ERROR("StartProcess PERMISSION_ACCESS_BUNDLE_DIR NOT GRANTED");
                hasAccessBundleDirReq = false;
            }
        }
    }

    AppSpawnStartMsg startMsg;
    startMsg.uid = bundleInfo.uid;
    startMsg.gid = bundleInfo.gid;
    startMsg.accessTokenId = bundleInfo.applicationInfo.accessTokenId;
    startMsg.apl = bundleInfo.applicationInfo.appPrivilegeLevel;
    startMsg.bundleName = bundleName;
    startMsg.renderParam = RENDER_PARAM;
    startMsg.flags = startFlags;
    startMsg.bundleIndex = bundleIndex;
    startMsg.setAllowInternet = setAllowInternet;
    startMsg.allowInternet = allowInternet;
    startMsg.hspList = hspList;
    if (hasAccessBundleDirReq) {
        startMsg.flags = startMsg.flags | APP_ACCESS_BUNDLE_DIR;
    }

    HILOG_DEBUG("Start process, apl is %{public}s, bundleName is %{public}s, startFlags is %{public}d.",
        startMsg.apl.c_str(), bundleName.c_str(), startFlags);

    bundleMgrResult = IN_PROCESS_CALL(bundleMgr_->GetBundleGidsByUid(bundleName, uid, startMsg.gids));
    if (!bundleMgrResult) {
        HILOG_ERROR("GetBundleGids is fail");
        return;
    }

    startMsg.procName = processName;
    startMsg.soPath = SO_PATH;
    startMsg.accessTokenIdEx = bundleInfo.applicationInfo.accessTokenIdEx;

    PerfProfile::GetInstance().SetAppForkStartTime(GetTickCount());
    pid_t pid = 0;
    ErrCode errCode = remoteClientManager_->GetSpawnClient()->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        HILOG_ERROR("failed to spawn new app process, errCode %{public}08x", errCode);
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
        return;
    }
    HILOG_INFO("Start process success, pid is %{public}d, processName is %{public}s.", pid, processName.c_str());
    SetRunningSharedBundleList(bundleName, hspList);
    appRecord->GetPriorityObject()->SetPid(pid);
    appRecord->SetUid(startMsg.uid);
    appRecord->SetStartMsg(startMsg);
    appRecord->SetAppMgrServiceInner(weak_from_this());
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_CREATE, false);
    AddAppToRecentList(appName, appRecord->GetProcessName(), pid, appRecord->GetRecordId());
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessCreated(appRecord);
    PerfProfile::GetInstance().SetAppForkEndTime(GetTickCount());
    SendProcessStartEvent(appRecord);
}

void AppMgrServiceInner::UpDateStartupType(
    const std::shared_ptr<AbilityInfo> &info, int32_t &abilityType, int32_t &extensionType)
{
    if (info == nullptr) {
        return;
    }

    HILOG_DEBUG("bundleName:%{public}s, abilityName:%{public}s", info->bundleName.c_str(), info->name.c_str());
    abilityType = static_cast<int32_t>(info->type);
    if (info->type != AbilityType::EXTENSION) {
        return;
    }

    extensionType = static_cast<int32_t>(info->extensionAbilityType);
}

bool AppMgrServiceInner::SendProcessStartEvent(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return false;
    }

    AAFwk::EventInfo eventInfo;
    time_t currentTime;
    time(&currentTime);
    eventInfo.time = currentTime;
    eventInfo.callerUid = appRecord->GetCallerUid() == -1 ? IPCSkeleton::GetCallingUid() : appRecord->GetCallerUid();
    if (!appRecord->GetAbilities().empty()) {
        auto abilityinfo = appRecord->GetAbilities().begin()->second->GetAbilityInfo();
        UpDateStartupType(abilityinfo, eventInfo.abilityType, eventInfo.extensionType);
    } else {
        HILOG_INFO("Abilities nullptr!");
    }

    auto callerAppRecord = GetAppRunningRecordByPid(appRecord->GetCallerPid());
    if (callerAppRecord == nullptr) {
        Security::AccessToken::NativeTokenInfo nativeTokenInfo = {};
        auto token = appRecord->GetCallerTokenId() == -1 ?
            static_cast<int>(IPCSkeleton::GetCallingTokenID()) : appRecord->GetCallerTokenId();
        Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(token, nativeTokenInfo);
        eventInfo.bundleName = "";
        eventInfo.processName = nativeTokenInfo.processName;
    } else {
        if (callerAppRecord->GetBundleName().empty()) {
            eventInfo.bundleName = callerAppRecord->GetName();
        } else {
            eventInfo.bundleName = callerAppRecord->GetBundleName();
        }
        eventInfo.processName = callerAppRecord->GetProcessName();
    }
    AAFwk::EventReport::SendAppEvent(AAFwk::EventName::PROCESS_START, HiSysEventType::BEHAVIOR, eventInfo);
    HILOG_INFO("%{public}s. time : %{public}" PRId64 ", abilityType : %{public}d, bundle : %{public}s,\
        uid : %{public}d, process : %{public}s",
        __func__, eventInfo.time, eventInfo.abilityType, eventInfo.bundleName.c_str(), eventInfo.callerUid,
        eventInfo.processName.c_str());

    return true;
}

void AppMgrServiceInner::RemoveAppFromRecentList(const std::string &appName, const std::string &processName)
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    auto appTaskInfo = appProcessManager_->GetAppTaskInfoByProcessName(appName, processName);
    if (!appTaskInfo) {
        return;
    }
    auto appRecord = GetAppRunningRecordByPid(appTaskInfo->GetPid());
    if (!appRecord) {
        appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
        return;
    }

    // Do not delete resident processes, before exec ScheduleProcessSecurityExit
    if (appRecord->IsKeepAliveApp()) {
        return;
    }

    startTime = SystemTimeMillisecond();
    pids.push_back(appTaskInfo->GetPid());
    appRecord->ScheduleProcessSecurityExit();
    if (!WaitForRemoteProcessExit(pids, startTime)) {
        int32_t result = KillProcessByPid(appTaskInfo->GetPid());
        if (result < 0) {
            HILOG_ERROR("RemoveAppFromRecentList kill process is fail");
            return;
        }
    }
    appProcessManager_->RemoveAppFromRecentList(appTaskInfo);
}

const std::list<const std::shared_ptr<AppTaskInfo>> &AppMgrServiceInner::GetRecentAppList() const
{
    return appProcessManager_->GetRecentAppList();
}

void AppMgrServiceInner::ClearRecentAppList()
{
    int64_t startTime = 0;
    std::list<pid_t> pids;
    if (GetAllPids(pids)) {
        return;
    }

    startTime = SystemTimeMillisecond();
    if (WaitForRemoteProcessExit(pids, startTime)) {
        appProcessManager_->ClearRecentAppList();
        return;
    }
    for (auto iter = pids.begin(); iter != pids.end(); ++iter) {
        int32_t result = KillProcessByPid(*iter);
        if (result < 0) {
            HILOG_ERROR("ClearRecentAppList kill process is fail");
            return;
        }
    }
    appProcessManager_->ClearRecentAppList();
}

void AppMgrServiceInner::OnRemoteDied(const wptr<IRemoteObject> &remote, bool isRenderProcess)
{
    HILOG_ERROR("On remote died.");
    if (isRenderProcess) {
        OnRenderRemoteDied(remote);
        return;
    }

    auto appRecord = appRunningManager_->OnRemoteDied(remote);
    if (!appRecord) {
        return;
    }

    ClearAppRunningData(appRecord, false);
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }
}

void AppMgrServiceInner::ClearAppRunningData(const std::shared_ptr<AppRunningRecord> &appRecord, bool containsApp)
{
    if (!appRecord) {
        return;
    }

    if (containsApp) {
        appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    }

    FinishUserTestLocked("App died", -1, appRecord);
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_REMOTE_DIED);

    for (const auto &item : appRecord->GetAbilities()) {
        const auto &abilityRecord = item.second;
        appRecord->StateChangedNotifyObserver(abilityRecord,
            static_cast<int32_t>(AbilityState::ABILITY_STATE_TERMINATED), true);
    }
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);

    // kill render if exist.
    auto renderRecord = appRecord->GetRenderRecord();
    if (renderRecord && renderRecord->GetPid() > 0) {
        HILOG_DEBUG("Kill render process when host died.");
        KillProcessByPid(renderRecord->GetPid());
        DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
    }

    if (appRecord->GetPriorityObject() != nullptr) {
        SendProcessExitEvent(appRecord->GetPriorityObject()->GetPid());
    }

    if (appRecord->IsKeepAliveApp()) {
        auto restartProcess = [appRecord, innerService = shared_from_this()]() {
            innerService->RestartResidentProcess(appRecord);
        };
        if (appRecord->CanRestartResidentProc()) {
            if (!eventHandler_) {
                HILOG_ERROR("eventHandler_ is nullptr");
                return;
            }
            eventHandler_->PostTask(restartProcess, "RestartResidentProcess");
        } else {
            auto findRestartResidentTask = [appRecord](const std::shared_ptr<AppRunningRecord> &appRunningRecord) {
                return (appRecord != nullptr && appRecord->GetBundleName() == appRunningRecord->GetBundleName());
            };
            auto findIter = find_if(restartResedentTaskList_.begin(), restartResedentTaskList_.end(),
                findRestartResidentTask);
            if (findIter != restartResedentTaskList_.end()) {
                HILOG_WARN("The restart app task has been registered.");
                return;
            }
            restartResedentTaskList_.emplace_back(appRecord);
            HILOG_INFO("PostRestartResidentProcessDelayTask.");
            eventHandler_->PostTask(restartProcess, "RestartResidentProcessDelayTask", RESTART_INTERVAL_TIME);
        }
    }
}

void AppMgrServiceInner::PushAppFront(const int32_t recordId)
{
    appProcessManager_->PushAppFront(recordId);
}

void AppMgrServiceInner::RemoveAppFromRecentListById(const int32_t recordId)
{
    appProcessManager_->RemoveAppFromRecentListById(recordId);
}

void AppMgrServiceInner::AddAppToRecentList(
    const std::string &appName, const std::string &processName, const pid_t pid, const int32_t recordId)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    appProcessManager_->AddAppToRecentList(appName, processName, pid, recordId);
}

const std::shared_ptr<AppTaskInfo> AppMgrServiceInner::GetAppTaskInfoById(const int32_t recordId) const
{
    return appProcessManager_->GetAppTaskInfoById(recordId);
}

void AppMgrServiceInner::AddAppDeathRecipient(const pid_t pid, const sptr<AppDeathRecipient> &appDeathRecipient) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::shared_ptr<AppRunningRecord> appRecord = GetAppRunningRecordByPid(pid);
    if (appRecord) {
        appRecord->SetAppDeathRecipient(appDeathRecipient);
    }
}

void AppMgrServiceInner::HandleTimeOut(const InnerEvent::Pointer &event)
{
    HILOG_INFO("handle time out");
    if (!appRunningManager_ || event == nullptr) {
        HILOG_ERROR("appRunningManager or event is nullptr");
        return;
    }

    // check libc.hook_mode
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        HILOG_DEBUG("HandleTimeOut, Hook_mode: no handle time out");
        return;
    }

    switch (event->GetInnerEventId()) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            appRunningManager_->HandleTerminateTimeOut(event->GetParam());
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            SendHiSysEvent(event->GetInnerEventId(), event->GetParam());
            HandleTerminateApplicationTimeOut(event->GetParam());
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            SendHiSysEvent(event->GetInnerEventId(), event->GetParam());
            HandleAddAbilityStageTimeOut(event->GetParam());
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            SendHiSysEvent(event->GetInnerEventId(), event->GetParam());
            HandleStartSpecifiedAbilityTimeOut(event->GetParam());
            break;
        default:
            break;
    }
}

void AppMgrServiceInner::SetEventHandler(const std::shared_ptr<AMSEventHandler> &handler)
{
    eventHandler_ = handler;
}

void AppMgrServiceInner::HandleAbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s called", __func__);
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->HandleAbilityAttachTimeOut(token);
}

void AppMgrServiceInner::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    HILOG_INFO("AppMgrService prepare to terminate the ability.");
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }
    appRunningManager_->PrepareTerminate(token);
}

void AppMgrServiceInner::HandleTerminateApplicationTimeOut(const int64_t eventId)
{
    HILOG_INFO("handle terminate application time out");
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    TerminateApplication(appRecord);
}

void AppMgrServiceInner::TerminateApplication(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appRecord->RemoveAppDeathRecipient();
    appRecord->SetProcessChangeReason(ProcessChangeReason::REASON_APP_TERMINATED_TIMEOUT);
    OnAppStateChanged(appRecord, ApplicationState::APP_STATE_TERMINATED, false);
    pid_t pid = appRecord->GetPriorityObject()->GetPid();
    if (pid > 0) {
        auto timeoutTask = [pid, innerService = shared_from_this()]() {
            HILOG_INFO("KillProcessByPid %{public}d", pid);
            int32_t result = innerService->KillProcessByPid(pid);
            innerService->SendProcessExitEvent(pid);
            if (result < 0) {
                HILOG_ERROR("KillProcessByPid kill process is fail");
                return;
            }
        };
        if (!eventHandler_) {
            HILOG_ERROR("eventHandler_ is nullptr");
            return;
        }
        eventHandler_->PostTask(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
    }
    appRunningManager_->RemoveAppRunningRecordById(appRecord->GetRecordId());
    RemoveAppFromRecentListById(appRecord->GetRecordId());
    if (!GetAppRunningStateByBundleName(appRecord->GetBundleName())) {
        RemoveRunningSharedBundleList(appRecord->GetBundleName());
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessDied(appRecord);
}

void AppMgrServiceInner::HandleAddAbilityStageTimeOut(const int64_t eventId)
{
    HILOG_INFO("called add ability stage info time out!");
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }
    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant());
    }

    KillApplicationByRecord(appRecord);
}

void AppMgrServiceInner::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    HILOG_INFO("%{public}s called", __func__);
    if (!CheckGetRunningInfoPermission()) {
        return;
    }

    appRunningManager_->GetRunningProcessInfoByToken(token, info);
}

void AppMgrServiceInner::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    HILOG_INFO("%{public}s called", __func__);
    if (!CheckGetRunningInfoPermission()) {
        return;
    }

    appRunningManager_->GetRunningProcessInfoByPid(pid, info);
}

bool AppMgrServiceInner::CheckGetRunningInfoPermission() const
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return false;
    }

    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return false;
    }

    return true;
}

void AppMgrServiceInner::LoadResidentProcess(const std::vector<AppExecFwk::BundleInfo> &infos)
{
    HILOG_INFO("%{public}s called", __func__);

    HILOG_INFO("bundle info size: [%{public}zu]", infos.size());
    StartResidentProcess(infos, -1, true);
}

void AppMgrServiceInner::StartResidentProcess(const std::vector<BundleInfo> &infos, int restartCount,
    bool isEmptyKeepAliveApp)
{
    HILOG_INFO("start resident process");
    if (infos.empty()) {
        HILOG_ERROR("infos is empty!");
        return;
    }

    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }

    for (auto &bundle : infos) {
        HILOG_INFO("processName = [%{public}s]", bundle.applicationInfo.process.c_str());
        if (bundle.applicationInfo.process.empty()) {
            continue;
        }
        auto processName = bundle.applicationInfo.process;
        // Inspection records
        auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
            bundle.applicationInfo.name, processName, bundle.applicationInfo.uid, bundle);
        if (appRecord) {
            HILOG_INFO("processName [%{public}s] Already exists ", processName.c_str());
            continue;
        }
        HILOG_INFO("Start empty resident process, processName = [%{public}s]", processName.c_str());
        StartEmptyResidentProcess(bundle, processName, restartCount, isEmptyKeepAliveApp);
    }
}

void AppMgrServiceInner::StartEmptyResidentProcess(
    const BundleInfo &info, const std::string &processName, int restartCount, bool isEmptyKeepAliveApp)
{
    HILOG_INFO("start bundle [%{public}s | processName [%{public}s]]", info.name.c_str(), processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        HILOG_INFO("Failed to start resident process!");
        return;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info);
    if (!appRecord) {
        HILOG_ERROR("start process [%{public}s] failed!", processName.c_str());
        return;
    }

    StartProcess(appInfo->name, processName, 0, appRecord, appInfo->uid, appInfo->bundleName, 0);

    // If it is empty, the startup failed
    if (!appRecord) {
        HILOG_ERROR("start process [%{public}s] failed!", processName.c_str());
        return;
    }

    appRecord->SetKeepAliveAppState(true, isEmptyKeepAliveApp);

    if (restartCount > 0) {
        HILOG_INFO("StartEmptyResidentProcess restartCount : [%{public}d], ", restartCount);
        appRecord->SetRestartResidentProcCount(restartCount);
    }

    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    HILOG_INFO("StartEmptyResidentProcess of pid : [%{public}d], ", appRecord->GetPriorityObject()->GetPid());
}

bool AppMgrServiceInner::CheckRemoteClient()
{
    if (!remoteClientManager_) {
        HILOG_ERROR("remoteClientManager_ is null");
        return false;
    }

    if (!remoteClientManager_->GetSpawnClient()) {
        HILOG_ERROR("appSpawnClient is null");
        return false;
    }

    if (!remoteClientManager_->GetBundleManager()) {
        HILOG_ERROR("GetBundleManager fail");
        return false;
    }
    return true;
}

void AppMgrServiceInner::RestartResidentProcess(std::shared_ptr<AppRunningRecord> appRecord)
{
    if (appRecord == nullptr) {
        HILOG_ERROR("Restart resident process failed, the appRecord is nullptr.");
        return;
    }
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    appRecord->SetRestartTimeMillis(static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS));
    appRecord->DecRestartResidentProcCount();

    auto findRestartResidentTask = [appRecord](const std::shared_ptr<AppRunningRecord> &appRunningRecord) {
        return (appRecord != nullptr && appRecord->GetBundleName() == appRunningRecord->GetBundleName());
    };
    auto findIter = find_if(restartResedentTaskList_.begin(), restartResedentTaskList_.end(), findRestartResidentTask);
    if (findIter != restartResedentTaskList_.end()) {
        restartResedentTaskList_.erase(findIter);
    }

    if (!CheckRemoteClient() || !appRecord || !appRunningManager_) {
        HILOG_ERROR("restart resident process failed!");
        return;
    }

    auto bundleMgr = remoteClientManager_->GetBundleManager();
    BundleInfo bundleInfo;
    auto callerUid = IPCSkeleton::GetCallingUid();
    auto userId = GetUserIdByUid(callerUid);
    if (!IN_PROCESS_CALL(
        bundleMgr->GetBundleInfo(appRecord->GetBundleName(), BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        HILOG_ERROR("GetBundleInfo fail");
        return;
    }
    std::vector<BundleInfo> infos;
    infos.emplace_back(bundleInfo);
    HILOG_INFO("the resident process [%{public}s] remaining restarts num is [%{public}d]",
        appRecord->GetProcessName().c_str(), (int)appRecord->GetRestartResidentProcCount());
    StartResidentProcess(infos, appRecord->GetRestartResidentProcCount(), appRecord->IsEmptyKeepAliveApp());
}

void AppMgrServiceInner::NotifyAppStatus(const std::string &bundleName, const std::string &eventData)
{
    HILOG_INFO("%{public}s called, bundle name is %{public}s, event is %{public}s",
        __func__, bundleName.c_str(), eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, 0);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void AppMgrServiceInner::NotifyAppStatusByCallerUid(const std::string &bundleName, const int32_t userId,
    const int32_t callerUid, const std::string &eventData)
{
    HILOG_INFO("%{public}s called, bundle name is %{public}s, , userId is %{public}d, event is %{public}s",
        __func__, bundleName.c_str(), userId, eventData.c_str());
    Want want;
    want.SetAction(eventData);
    ElementName element;
    element.SetBundleName(bundleName);
    want.SetElement(element);
    want.SetParam(Constants::USER_ID, userId);
    want.SetParam(Constants::UID, callerUid);
    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

int32_t AppMgrServiceInner::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->RegisterApplicationStateObserver(
        observer, bundleNameList);
}

int32_t AppMgrServiceInner::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    CHECK_CALLER_IS_SYSTEM_APP;
    return DelayedSingleton<AppStateObserverManager>::GetInstance()->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrServiceInner::GetForegroundApplications(std::vector<AppStateData> &list)
{
    HILOG_INFO("%{public}s, begin.", __func__);
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isPerm = AAFwk::PermissionVerification::GetInstance()->VerifyRunningInfoPerm();
    if (!isPerm) {
        HILOG_ERROR("%{public}s: Permission verification failed", __func__);
        return ERR_PERMISSION_DENIED;
    }

    appRunningManager_->GetForegroundApplications(list);
    return ERR_OK;
}

int AppMgrServiceInner::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    HILOG_INFO("Enter");
    if (!observer) {
        HILOG_ERROR("observer nullptr.");
        return ERR_INVALID_VALUE;
    }
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return ERR_INVALID_VALUE;
    }

    std::string bundleName = want.GetStringParam("-b");
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name");
        return ERR_INVALID_VALUE;
    }

    if (KillApplicationByUserIdLocked(bundleName, userId)) {
        HILOG_ERROR("Failed to kill the application");
        return ERR_INVALID_VALUE;
    }

    HapModuleInfo hapModuleInfo;
    if (GetHapModuleInfoForTestRunner(want, observer, bundleInfo, hapModuleInfo)) {
        HILOG_ERROR("Failed to get HapModuleInfo for TestRunner");
        return ERR_INVALID_VALUE;
    }

    std::string processName;
    MakeProcessName(std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo), hapModuleInfo, processName);
    HILOG_INFO("processName = [%{public}s]", processName.c_str());

    // Inspection records
    auto appRecord = appRunningManager_->CheckAppRunningRecordIsExist(
        bundleInfo.applicationInfo.name, processName, bundleInfo.applicationInfo.uid, bundleInfo);
    if (appRecord) {
        HILOG_INFO("processName [%{public}s] Already exists ", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    return StartEmptyProcess(want, observer, bundleInfo, processName, userId);
}

int AppMgrServiceInner::GetHapModuleInfoForTestRunner(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &bundleInfo, HapModuleInfo &hapModuleInfo)
{
    HILOG_INFO("Enter");
    if (!observer) {
        HILOG_ERROR("observer nullptr.");
        return ERR_INVALID_VALUE;
    }

    bool moduleJson = false;
    if (!bundleInfo.hapModuleInfos.empty()) {
        moduleJson = bundleInfo.hapModuleInfos.back().isModuleJson;
    }
    if (moduleJson) {
        std::string moduleName = want.GetStringParam("-m");
        if (moduleName.empty()) {
            UserTestAbnormalFinish(observer, "No module name is specified.");
            return ERR_INVALID_VALUE;
        }

        bool found = false;
        for (auto item : bundleInfo.hapModuleInfos) {
            if (item.moduleName == moduleName) {
                hapModuleInfo = item;
                found = true;
                break;
            }
        }
        if (!found) {
            UserTestAbnormalFinish(observer, "The specified module name is not found.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int AppMgrServiceInner::UserTestAbnormalFinish(const sptr<IRemoteObject> &observer, const std::string &msg)
{
    sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(observer);
    if (!observerProxy) {
        HILOG_ERROR("Failed to get ITestObserver proxy");
        return ERR_INVALID_VALUE;
    }
    observerProxy->TestFinished(msg, -1);
    return ERR_OK;
}

int AppMgrServiceInner::StartEmptyProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const BundleInfo &info, const std::string &processName, const int userId)
{
    HILOG_INFO("enter bundle [%{public}s | processName [%{public}s]]", info.name.c_str(), processName.c_str());
    if (!CheckRemoteClient() || !appRunningManager_) {
        HILOG_ERROR("Failed to start the process being tested!");
        return ERR_INVALID_VALUE;
    }

    auto appInfo = std::make_shared<ApplicationInfo>(info.applicationInfo);
    auto appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, info);
    if (!appRecord) {
        HILOG_ERROR("Failed to start process [%{public}s]!", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    auto isDebug = want.GetBoolParam("debugApp", false);
    HILOG_INFO("Set Debug : %{public}s", (isDebug ? "true" : "false"));
    appRecord->SetDebugApp(isDebug);
    if (want.GetBoolParam(COLD_START, false)) {
        appRecord->SetDebugApp(true);
    }

    std::shared_ptr<UserTestRecord> testRecord = std::make_shared<UserTestRecord>();
    if (!testRecord) {
        HILOG_ERROR("Failed to make UserTestRecord!");
        return ERR_INVALID_VALUE;
    }
    testRecord->want = want;
    testRecord->observer = observer;
    testRecord->isFinished = false;
    testRecord->userId = userId;
    appRecord->SetUserTestInfo(testRecord);

    int32_t bundleIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
    StartProcess(appInfo->name, processName, 0, appRecord, appInfo->uid, appInfo->bundleName, bundleIndex);

    // If it is empty, the startup failed
    if (!appRecord) {
        HILOG_ERROR("Failed to start process [%{public}s]!", processName.c_str());
        return ERR_INVALID_VALUE;
    }

    appRecord->SetEventHandler(eventHandler_);
    appRecord->AddModules(appInfo, info.hapModuleInfos);
    HILOG_INFO("StartEmptyProcess OK pid : [%{public}d]", appRecord->GetPriorityObject()->GetPid());

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTest(
    const std::string &msg, const int64_t &resultCode, const std::string &bundleName, const pid_t &pid)
{
    HILOG_INFO("Enter");
    if (bundleName.empty()) {
        HILOG_ERROR("Invalid bundle name.");
        return ERR_INVALID_VALUE;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord");
        return ERR_INVALID_VALUE;
    }

    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        HILOG_ERROR("unstart user test");
        return ERR_INVALID_VALUE;
    }

    FinishUserTestLocked(msg, resultCode, appRecord);

    int ret = KillApplicationByUserIdLocked(bundleName, userTestRecord->userId);
    if (ret) {
        HILOG_ERROR("Failed to kill process.");
        return ret;
    }

    return ERR_OK;
}

int AppMgrServiceInner::FinishUserTestLocked(
    const std::string &msg, const int64_t &resultCode, const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HILOG_INFO("Enter");
    if (!appRecord) {
        HILOG_ERROR("Invalid appRecord");
        return ERR_INVALID_VALUE;
    }

    std::unique_lock<std::mutex> lock(userTestLock_);
    auto userTestRecord = appRecord->GetUserTestInfo();
    if (!userTestRecord) {
        HILOG_WARN("not start user test");
        return ERR_INVALID_VALUE;
    }
    if (!userTestRecord->isFinished) {
        sptr<AAFwk::ITestObserver> observerProxy = iface_cast<AAFwk::ITestObserver>(userTestRecord->observer);
        if (!observerProxy) {
            HILOG_ERROR("Failed to get ITestObserver proxy");
            return ERR_INVALID_VALUE;
        }
        observerProxy->TestFinished(msg, resultCode);

        userTestRecord->isFinished = true;
    }

    return ERR_OK;
}

void AppMgrServiceInner::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)
{
    HILOG_DEBUG("Start specified ability.");
    if (!CheckRemoteClient()) {
        return;
    }

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto appInfo = std::make_shared<ApplicationInfo>(abilityInfo.applicationInfo);

    int32_t appIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
    if (!GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex)) {
        return;
    }

    std::string processName;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    MakeProcessName(abilityInfoPtr, appInfo, hapModuleInfo, appIndex, processName);

    std::vector<HapModuleInfo> hapModules;
    hapModules.emplace_back(hapModuleInfo);

    std::shared_ptr<AppRunningRecord> appRecord;
    appRecord = appRunningManager_->CheckAppRunningRecordIsExist(appInfo->name, processName, appInfo->uid, bundleInfo);
    if (!appRecord) {
        // new app record
        appRecord = appRunningManager_->CreateAppRunningRecord(appInfo, processName, bundleInfo);
        if (!appRecord) {
            HILOG_ERROR("start process [%{public}s] failed!", processName.c_str());
            return;
        }
        auto wantPtr = std::make_shared<AAFwk::Want>(want);
        if (wantPtr != nullptr) {
            appRecord->SetCallerPid(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_PID, -1));
            appRecord->SetCallerUid(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_UID, -1));
            appRecord->SetCallerTokenId(wantPtr->GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, -1));
        }
        appRecord->SetEventHandler(eventHandler_);
        appRecord->SendEventForSpecifiedAbility(AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG,
            AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT);
        uint32_t startFlags = BuildStartFlags(want, abilityInfo);
        int32_t bundleIndex = want.GetIntParam(DLP_PARAMS_INDEX, 0);
        StartProcess(appInfo->name, processName, startFlags, appRecord, appInfo->uid, appInfo->bundleName,
            bundleIndex);

        appRecord->SetSpecifiedAbilityFlagAndWant(true, want, hapModuleInfo.moduleName);
        appRecord->AddModules(appInfo, hapModules);
    } else {
        HILOG_DEBUG("process is exist");
        appRecord->SetSpecifiedAbilityFlagAndWant(true, want, hapModuleInfo.moduleName);
        auto moduleRecord = appRecord->GetModuleRecordByModuleName(appInfo->bundleName, hapModuleInfo.moduleName);
        if (!moduleRecord) {
            HILOG_DEBUG("module record is nullptr, add modules");
            appRecord->AddModules(appInfo, hapModules);
            appRecord->AddAbilityStageBySpecifiedAbility(appInfo->bundleName);
        } else {
            HILOG_DEBUG("schedule accept want");
            appRecord->ScheduleAcceptWant(hapModuleInfo.moduleName);
        }
    }
}

void AppMgrServiceInner::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    if (!response) {
        HILOG_ERROR("response is nullptr, register failed.");
        return;
    }

    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        HILOG_ERROR("%{public}s: Not abilityMgr call.", __func__);
        return;
    }

    startSpecifiedAbilityResponse_ = response;
}

void AppMgrServiceInner::ScheduleAcceptWantDone(
    const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    HILOG_DEBUG("Schedule accept want done, flag: %{public}s", flag.c_str());

    auto appRecord = GetAppRunningRecordByAppRecordId(recordId);
    if (!appRecord) {
        HILOG_ERROR("Get app record failed.");
        return;
    }
    appRecord->ScheduleAcceptWantDone();

    if (startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnAcceptWantResponse(want, flag);
    }
}

void AppMgrServiceInner::HandleStartSpecifiedAbilityTimeOut(const int64_t eventId)
{
    HILOG_DEBUG("called start specified ability time out!");
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }

    if (appRecord->IsStartSpecifiedAbility() && startSpecifiedAbilityResponse_) {
        startSpecifiedAbilityResponse_->OnTimeoutResponse(appRecord->GetSpecifiedWant());
    }

    KillApplicationByRecord(appRecord);
}

int32_t AppMgrServiceInner::UpdateConfiguration(const Configuration &config)
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is null");
        return ERR_INVALID_VALUE;
    }
    CHECK_CALLER_IS_SYSTEM_APP;

    auto ret = AAFwk::PermissionVerification::GetInstance()->VerifyUpdateConfigurationPerm();
    if (ret != ERR_OK) {
        return ret;
    }

    std::vector<std::string> changeKeyV;
    configuration_->CompareDifferent(changeKeyV, config);
    HILOG_INFO("changeKeyV size :%{public}zu", changeKeyV.size());
    if (changeKeyV.empty()) {
        HILOG_ERROR("changeKeyV is empty");
        return ERR_INVALID_VALUE;
    }
    configuration_->Merge(changeKeyV, config);
    // all app
    int32_t result = appRunningManager_->UpdateConfiguration(config);
    if (result != ERR_OK) {
        HILOG_ERROR("update error, not notify");
        return result;
    }
    // notify
    std::lock_guard<std::recursive_mutex> notifyLock(configurationObserverLock_);
    for (auto &observer : configurationObservers_) {
        if (observer != nullptr) {
            observer->OnConfigurationUpdated(config);
        }
    }
    return result;
}

int32_t AppMgrServiceInner::RegisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
{
    HILOG_INFO("AppMgrServiceInner::RegisterConfigurationObserver: called");

    if (observer == nullptr) {
        HILOG_ERROR("AppMgrServiceInner::Register error: observer is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::recursive_mutex> registerLock(configurationObserverLock_);
    auto it = std::find_if(configurationObservers_.begin(), configurationObservers_.end(),
        [&observer](const sptr<IConfigurationObserver> &item) {
            return (item && item->AsObject() == observer->AsObject());
        }
    );
    if (it != configurationObservers_.end()) {
        HILOG_ERROR("AppMgrServiceInner::Register error: observer exist");
        return ERR_INVALID_VALUE;
    }
    configurationObservers_.push_back(observer);
    return NO_ERROR;
}

int32_t AppMgrServiceInner::UnregisterConfigurationObserver(const sptr<IConfigurationObserver>& observer)
{
    HILOG_INFO("AppMgrServiceInner::UnregisterConfigurationObserver: called");
    if (observer == nullptr) {
        HILOG_ERROR("AppMgrServiceInner::Register error: observer is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard<std::recursive_mutex> unregisterLock(configurationObserverLock_);
    auto it = std::find_if(configurationObservers_.begin(), configurationObservers_.end(),
        [&observer](const sptr<IConfigurationObserver> &item) {
            return (item && item->AsObject() == observer->AsObject());
        }
    );
    if (it != configurationObservers_.end()) {
        configurationObservers_.erase(it);
        return NO_ERROR;
    }
    HILOG_INFO("AppMgrServiceInner ConfigurationObserver not register");
    return ERR_INVALID_VALUE;
}

void AppMgrServiceInner::InitGlobalConfiguration()
{
    if (!configuration_) {
        HILOG_ERROR("configuration_ is null");
        return;
    }

#ifdef SUPPORT_GRAPHICS
    // Currently only this interface is known
    auto language = OHOS::Global::I18n::LocaleConfig::GetSystemLanguage();
    HILOG_INFO("current global language is : %{public}s", language.c_str());
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, language);
#endif

    // Assign to default colorMode "light"
    HILOG_INFO("current global colorMode is : %{public}s", ConfigurationInner::COLOR_MODE_LIGHT);
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);

    // Get input pointer device
    std::string hasPointerDevice = system::GetParameter(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "false");
    HILOG_INFO("current hasPointerDevice is %{public}s", hasPointerDevice.c_str());
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, hasPointerDevice);

    // Get DeviceType
    std::string deviceType = system::GetParameter(AAFwk::GlobalConfigurationKey::DEVICE_TYPE,
        ConfigurationInner::DEVICE_TYPE_DEFAULT);
    HILOG_INFO("current deviceType is %{public}s", deviceType.c_str());
    configuration_->AddItem(AAFwk::GlobalConfigurationKey::DEVICE_TYPE, deviceType);
}

std::shared_ptr<AppExecFwk::Configuration> AppMgrServiceInner::GetConfiguration()
{
    return configuration_;
}

void AppMgrServiceInner::KillApplicationByRecord(const std::shared_ptr<AppRunningRecord> &appRecord)
{
    HILOG_DEBUG("Kill application by appRecord.");
    if (!appRecord || !eventHandler_) {
        HILOG_WARN("appRecord or eventHandler_ is nullptr.");
        return;
    }

    auto pid = appRecord->GetPriorityObject()->GetPid();
    appRecord->SetTerminating();
    appRecord->ScheduleProcessSecurityExit();

    auto startTime = SystemTimeMillisecond();
    std::list<pid_t> pids = {pid};
    if (WaitForRemoteProcessExit(pids, startTime)) {
        HILOG_INFO("The remote process exited successfully");
        return;
    }

    auto timeoutTask = [pid, innerService = shared_from_this()]() {
        HILOG_INFO("KillProcessByPid %{public}d", pid);
        int32_t result = innerService->KillProcessByPid(pid);
        if (result < 0) {
            HILOG_ERROR("Kill application by app record failed, pid: %{public}d", pid);
            return;
        }
    };
    eventHandler_->PostTask(timeoutTask, "DelayKillProcess", AMSEventHandler::KILL_PROCESS_TIMEOUT);
}

void AppMgrServiceInner::SendHiSysEvent(const int32_t innerEventId, const int64_t eventId)
{
    HILOG_DEBUG("called AppMgrServiceInner SendHiSysEvent!");
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecord(eventId);
    if (!appRecord) {
        HILOG_ERROR("appRecord is nullptr");
        return;
    }
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *hook_mode = "startup:";
    int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
    if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) {
        HILOG_DEBUG("SendHiSysEvent, Hook_mode: no handle time out");
        return;
    }

    std::string eventName = EVENT_NAME_LIFECYCLE_TIMEOUT;
    int32_t pid = appRecord->GetPriorityObject()->GetPid();
    int32_t uid = appRecord->GetUid();
    std::string packageName = appRecord->GetBundleName();
    std::string processName = appRecord->GetProcessName();
    std::string msg;
    switch (innerEventId) {
        case AMSEventHandler::TERMINATE_ABILITY_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_TERMINATE_ABILITY_TIMEOUT;
            break;
        case AMSEventHandler::TERMINATE_APPLICATION_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_TERMINATE_APPLICATION_TIMEOUT;
            break;
        case AMSEventHandler::ADD_ABILITY_STAGE_INFO_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_ADD_ABILITY_STAGE_INFO_TIMEOUT;
            break;
        case AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_START_PROCESS_SPECIFIED_ABILITY_TIMEOUT;
            break;
        case AMSEventHandler::START_SPECIFIED_ABILITY_TIMEOUT_MSG:
            msg = EVENT_MESSAGE_START_SPECIFIED_ABILITY_TIMEOUT;
            break;
        default:
            msg = EVENT_MESSAGE_DEFAULT;
            break;
    }

    HILOG_DEBUG("SendHiSysEvent, eventName = %{public}s, uid = %{public}d, pid = %{public}d, \
        packageName = %{public}s, processName = %{public}s, msg = %{public}s",
        eventName.c_str(), uid, pid, packageName.c_str(), processName.c_str(), msg.c_str());

    HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::AAFWK,
        eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        EVENT_KEY_PID, pid,
        EVENT_KEY_UID, uid,
        EVENT_KEY_PACKAGE_NAME, packageName,
        EVENT_KEY_PROCESS_NAME, processName,
        EVENT_KEY_MESSAGE, msg);
}

int AppMgrServiceInner::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord");
        return ERR_NAME_NOT_FOUND;
    }

    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (!isSaCall && callingPid != pid) {
        HILOG_ERROR("Permission verify failed.");
        return ERR_PERMISSION_DENIED;
    }
    for (auto &item : appRecord->GetAbilities()) {
        tokens.emplace_back(item.first);
    }
    return ERR_OK;
}

int AppMgrServiceInner::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application,
    bool &debug)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isSaCall && !isShellCall) {
        HILOG_ERROR("no permissions.");
        return ERR_PERMISSION_DENIED;
    }
    auto appRecord = GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord for PID:%{public}d", pid);
        return ERR_NAME_NOT_FOUND;
    }

    auto info = appRecord->GetApplicationInfo();
    if (info == nullptr) {
        HILOG_ERROR("ApplicationInfo is nullptr !");
        return ERR_NO_INIT;
    }
    application = *info;
    debug = appRecord->IsDebugApp();
    return ERR_OK;
}

int AppMgrServiceInner::VerifyProcessPermission() const
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    return isCallingPerm ? ERR_OK : ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::VerifyProcessPermission(const std::string &bundleName) const
{
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (isSaCall || isShellCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        if (AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
            return ERR_OK;
        }
        auto callerPid = IPCSkeleton::GetCallingPid();
        auto appRecord = GetAppRunningRecordByPid(callerPid);
        if (!appRecord || appRecord->GetBundleName() != bundleName) {
            HILOG_ERROR("Permission verification failed.");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        HILOG_ERROR("Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    
    return ERR_OK;
}

int AppMgrServiceInner::VerifyProcessPermission(const sptr<IRemoteObject> &token) const
{
    CHECK_CALLER_IS_SYSTEM_APP;
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    if (VerifyAPL()) {
        return ERR_OK;
    }

    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CLEAN_BACKGROUND_PROCESSES);
    if (isCallingPerm) {
        if (AAFwk::PermissionVerification::GetInstance()->IsGatewayCall()) {
            return ERR_OK;
        }
        auto callerUid = IPCSkeleton::GetCallingUid();
        auto appRecord = GetAppRunningRecordByAbilityToken(token);
        if (!appRecord || appRecord->GetUid() != callerUid) {
            HILOG_ERROR("Permission verification failed.");
            return ERR_PERMISSION_DENIED;
        }
    } else {
        HILOG_ERROR("Permission verification failed.");
        return ERR_PERMISSION_DENIED;
    }
    
    return ERR_OK;
}

bool AppMgrServiceInner::VerifyAPL() const
{
    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr");
        return false;
    }

    auto callerPid = IPCSkeleton::GetCallingPid();
    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(callerPid);
    if (!appRecord) {
        HILOG_ERROR("Get app running record by calling pid failed. callingPId: %{public}d", callerPid);
        return false;
    }

    auto applicationInfo = appRecord->GetApplicationInfo();
    if (!applicationInfo) {
        HILOG_ERROR("Get application info failed.");
        return false;
    }

    auto apl = applicationInfo->appPrivilegeLevel;
    if (apl != SYSTEM_BASIC && apl != SYSTEM_CORE) {
        HILOG_ERROR("caller is not system_basic or system_core.");
        return false;
    }
    return true;
}

int AppMgrServiceInner::VerifyAccountPermission(const std::string &permissionName, const int userId) const
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return ERR_OK;
    }

    const int currentUserId = (int)(getuid() / Constants::BASE_USER_RANGE);
    if (userId != currentUserId) {
        auto isCallingPermAccount = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
            AAFwk::PermissionConstants::PERMISSION_INTERACT_ACROSS_LOCAL_ACCOUNTS);
        if (!isCallingPermAccount) {
            HILOG_ERROR("%{public}s: Permission accounts verification failed", __func__);
            return ERR_PERMISSION_DENIED;
        }
    }
    auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(permissionName);
    return isCallingPerm ? ERR_OK : ERR_PERMISSION_DENIED;
}

int AppMgrServiceInner::VerifyRequestPermission() const
{
    auto callerUid = IPCSkeleton::GetCallingUid();

    if (callerUid == ROOT_UID || callerUid == FOUNDATION_UID) {
        return ERR_OK;
    } else {
        HILOG_ERROR("Permission verification failed, callerUid: %{public}d", callerUid);
        return ERR_PERMISSION_DENIED;
    }
}

int AppMgrServiceInner::PreStartNWebSpawnProcess(const pid_t hostPid)
{
    HILOG_INFO("AppMgrServiceInner::PreStartNWebSpawnProcess");
    if (hostPid <= 0) {
        HILOG_ERROR("invalid param, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        HILOG_ERROR("nwebSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        HILOG_ERROR("no such app Record, pid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    ErrCode errCode = nwebSpawnClient->PreStartNWebSpawnProcess();
    if (FAILED(errCode)) {
        HILOG_ERROR("failed to spawn new render process, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }

    return 0;
}

int AppMgrServiceInner::StartRenderProcess(const pid_t hostPid, const std::string &renderParam,
    int32_t ipcFd, int32_t sharedFd, pid_t &renderPid)
{
    HILOG_INFO("start render process, hostPid:%{public}d", hostPid);
    if (hostPid <= 0 || renderParam.empty() || ipcFd <= 0 || sharedFd <= 0) {
        HILOG_ERROR("invalid param: hostPid:%{public}d renderParam:%{private}s ipcFd:%{public}d sharedFd:%{public}d",
            hostPid, renderParam.c_str(), ipcFd, sharedFd);
        return ERR_INVALID_VALUE;
    }

    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is nullptr, not start render process");
        return ERR_INVALID_VALUE;
    }

    auto appRecord = GetAppRunningRecordByPid(hostPid);
    if (!appRecord) {
        HILOG_ERROR("no such appRecord, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    auto renderRecord = appRecord->GetRenderRecord();
    if (renderRecord) {
        HILOG_WARN("already exit render process,do not request again, renderPid:%{public}d", renderRecord->GetPid());
        renderPid = renderRecord->GetPid();
        return ERR_ALREADY_EXIST_RENDER;
    }

    renderRecord = RenderRecord::CreateRenderRecord(hostPid, renderParam, ipcFd, sharedFd, appRecord);
    if (!renderRecord) {
        HILOG_ERROR("create render record failed, hostPid:%{public}d", hostPid);
        return ERR_INVALID_VALUE;
    }

    return StartRenderProcessImpl(renderRecord, appRecord, renderPid);
}

void AppMgrServiceInner::AttachRenderProcess(const pid_t pid, const sptr<IRenderScheduler> &scheduler)
{
    HILOG_DEBUG("attach render process start");
    if (pid <= 0) {
        HILOG_ERROR("invalid render process pid:%{public}d", pid);
        return;
    }
    if (!scheduler) {
        HILOG_ERROR("render scheduler is null");
        return;
    }

    if (!appRunningManager_) {
        HILOG_ERROR("appRunningManager_ is null");
        return;
    }

    HILOG_INFO("attach render process pid:%{public}d", pid);
    auto appRecord = appRunningManager_->GetAppRunningRecordByRenderPid(pid);
    if (!appRecord) {
        HILOG_ERROR("no such app Record, pid:%{public}d", pid);
        return;
    }

    auto renderRecord = appRecord->GetRenderRecord();
    if (!renderRecord) {
        HILOG_ERROR("no such render Record, pid:%{public}d", pid);
        return;
    }

    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetEventHandler(eventHandler_);
    appDeathRecipient->SetAppMgrServiceInner(shared_from_this());
    appDeathRecipient->SetIsRenderProcess(true);
    renderRecord->SetScheduler(scheduler);
    renderRecord->SetDeathRecipient(appDeathRecipient);
    renderRecord->RegisterDeathRecipient();

    // notify fd to render process
    scheduler->NotifyBrowserFd(renderRecord->GetIpcFd(), renderRecord->GetSharedFd());
}

int AppMgrServiceInner::StartRenderProcessImpl(const std::shared_ptr<RenderRecord> &renderRecord,
    const std::shared_ptr<AppRunningRecord> appRecord, pid_t &renderPid)
{
    if (!renderRecord || !appRecord) {
        HILOG_ERROR("renderRecord or appRecord is nullptr.");
        return ERR_INVALID_VALUE;
    }

    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        HILOG_ERROR("nwebSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    AppSpawnStartMsg startMsg = appRecord->GetStartMsg();
    startMsg.renderParam = renderRecord->GetRenderParam();
    startMsg.code = 0; // 0: DEFAULT
    pid_t pid = 0;
    ErrCode errCode = nwebSpawnClient->StartProcess(startMsg, pid);
    if (FAILED(errCode)) {
        HILOG_ERROR("failed to spawn new render process, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }
    renderPid = pid;
    appRecord->SetRenderRecord(renderRecord);
    renderRecord->SetPid(pid);
    HILOG_INFO("start render process success, hostPid:%{public}d, pid:%{public}d uid:%{public}d",
        renderRecord->GetHostPid(), pid, startMsg.uid);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessCreated(renderRecord);
    return 0;
}

int AppMgrServiceInner::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    if (remoteClientManager_ == nullptr) {
        HILOG_ERROR("remoteClientManager_ is null");
        return ERR_INVALID_VALUE;
    }
    auto nwebSpawnClient = remoteClientManager_->GetNWebSpawnClient();
    if (!nwebSpawnClient) {
        HILOG_ERROR("nwebSpawnClient is null");
        return ERR_INVALID_VALUE;
    }

    AppSpawnStartMsg startMsg;
    startMsg.pid = renderPid;
    startMsg.code = 1; // 1: GET_RENDER_TERMINATION_STATUS
    ErrCode errCode = nwebSpawnClient->GetRenderProcessTerminationStatus(startMsg, status);
    if (FAILED(errCode)) {
        HILOG_ERROR("failed to get render process termination status, errCode %{public}08x", errCode);
        return ERR_INVALID_VALUE;
    }
    HILOG_INFO("Get render process termination status success, renderPid:%{public}d, status:%{public}d",
        renderPid, status);
    return 0;
}

void AppMgrServiceInner::OnRenderRemoteDied(const wptr<IRemoteObject> &remote)
{
    HILOG_ERROR("On render remote died.");
    if (appRunningManager_) {
        auto renderRecord = appRunningManager_->OnRemoteRenderDied(remote);
        if (renderRecord) {
            DelayedSingleton<AppStateObserverManager>::GetInstance()->OnRenderProcessDied(renderRecord);
        }
    }
}

uint32_t AppMgrServiceInner::BuildStartFlags(const AAFwk::Want &want, const AbilityInfo &abilityInfo)
{
    uint32_t startFlags = 0x0;
    if (want.GetBoolParam("coldStart", false)) {
        startFlags = startFlags | (AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT << StartFlags::COLD_START);
    }

    if (want.GetIntParam(DLP_PARAMS_INDEX, 0) != 0) {
        startFlags = startFlags | (AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT << StartFlags::DLP_MANAGER);
    }

    if (abilityInfo.extensionAbilityType == ExtensionAbilityType::BACKUP) {
        startFlags = startFlags | (AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT << StartFlags::BACKUP_EXTENSION);
    }

    if (abilityInfo.applicationInfo.debug) {
        startFlags = startFlags | (AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT << StartFlags::DEBUGGABLE);
    }
    if (abilityInfo.applicationInfo.asanEnabled) {
	    startFlags = startFlags | (AppSpawn::ClientSocket::APPSPAWN_COLD_BOOT << StartFlags::ASANENABLED);
    }

    return startFlags;
}

void AppMgrServiceInner::AddWatchParameter()
{
    HILOG_INFO("%{public}s called.", __func__);
    auto context = new (std::nothrow) std::weak_ptr<AppMgrServiceInner>(shared_from_this());
    int ret = WatchParameter(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, PointerDeviceEventCallback,
        context);
    if (ret != 0) {
        HILOG_ERROR("watch parameter %{public}s failed with %{public}d.",
            AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, ret);
    }
}

void AppMgrServiceInner::InitFocusListener()
{
    HILOG_INFO("begin initFocus listener.");
    if (focusListener_) {
        return;
    }

    focusListener_ = new WindowFocusChangedListener(shared_from_this(), eventHandler_);
    auto registerTask = [innerService = shared_from_this()]() {
        if (innerService) {
            HILOG_INFO("RegisterFocusListener task");
            innerService->RegisterFocusListener();
        }
    };
    if (eventHandler_) {
        eventHandler_->PostTask(registerTask, "RegisterFocusListenerTask", REGISTER_FOCUS_DELAY);
    }
}

void AppMgrServiceInner::RegisterFocusListener()
{
    HILOG_INFO("RegisterFocusListener begin");
    if (!focusListener_) {
        HILOG_ERROR("no focusListener_");
        return;
    }
    WindowManager::GetInstance().RegisterFocusChangedListener(focusListener_);
    HILOG_INFO("RegisterFocusListener end");
}

void AppMgrServiceInner::HandleFocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        HILOG_WARN("focused, invalid focusChangeInfo");
        return;
    }
    HILOG_DEBUG("focused, uid:%{public}d, pid:%{public}d", focusChangeInfo->uid_, focusChangeInfo->pid_);

    if (focusChangeInfo->pid_ <= 0) {
        HILOG_ERROR("invalid pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) {
        HILOG_ERROR("focused, no such appRecord, pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, true)) {
        HILOG_DEBUG("only change ability focus state, do not change process or application focus state.");
        return;
    }

    bool needNotifyApp = appRunningManager_->IsApplicationFirstFocused(*appRecord);
    OnAppStateChanged(appRecord, appRecord->GetState(), needNotifyApp);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
}

void AppMgrServiceInner::HandleUnfocused(const sptr<OHOS::Rosen::FocusChangeInfo> &focusChangeInfo)
{
    if (!focusChangeInfo) {
        HILOG_WARN("unfocused, invalid focusChangeInfo");
        return;
    }
    HILOG_DEBUG("unfocused, uid:%{public}d, pid:%{public}d", focusChangeInfo->uid_, focusChangeInfo->pid_);

    if (focusChangeInfo->pid_ <= 0) {
        HILOG_ERROR("invalid pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) {
        HILOG_ERROR("unfocused, no such appRecord, pid:%{public}d", focusChangeInfo->pid_);
        return;
    }

    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, false)) {
        HILOG_DEBUG("only change ability from focus to unfocus, do not change process or application focus state.");
        return;
    }

    bool needNotifyApp = appRunningManager_->IsApplicationUnfocused(appRecord->GetBundleName());
    OnAppStateChanged(appRecord, appRecord->GetState(), needNotifyApp);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);
}

void AppMgrServiceInner::PointerDeviceEventCallback(const char *key, const char *value, void *context)
{
    HILOG_INFO("%{public}s called.", __func__);
    auto weak = static_cast<std::weak_ptr<AppMgrServiceInner>*>(context);
    if (weak == nullptr) {
        HILOG_ERROR("context is nullptr.");
        return;
    }

    auto appMgrServiceInner = weak->lock();
    if (appMgrServiceInner == nullptr) {
        HILOG_ERROR("app manager service inner is nullptr.");
        return;
    }

    if ((strcmp(key, AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE) != 0) ||
        ((strcmp(value, "true") != 0) && (strcmp(value, "false") != 0))) {
        HILOG_ERROR("key %{public}s or value %{public}s mismatch.", key, value);
        return;
    }

    Configuration changeConfig;
    if (!changeConfig.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, value)) {
        HILOG_ERROR("add %{public}s item to configuration failed.", key);
        return;
    }

    HILOG_DEBUG("update config %{public}s to %{public}s", key, value);
    auto result = appMgrServiceInner->UpdateConfiguration(changeConfig);
    if (result != 0) {
        HILOG_ERROR("update config failed with %{public}d, key: %{public}s, value: %{public}s.", result, key, value);
        return;
    }
}

bool AppMgrServiceInner::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appRunningManager_) {
        HILOG_ERROR("app running manager is nullptr.");
        return false;
    }

    return appRunningManager_->GetAppRunningStateByBundleName(bundleName);
}

int32_t AppMgrServiceInner::NotifyLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appRunningManager_) {
        HILOG_ERROR("app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    return appRunningManager_->NotifyLoadRepairPatch(bundleName, callback);
}

int32_t AppMgrServiceInner::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appRunningManager_) {
        HILOG_ERROR("app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    return appRunningManager_->NotifyHotReloadPage(bundleName, callback);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrServiceInner::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("callerToken not SA %{public}s", __func__);
        return ERR_INVALID_VALUE;
    }

    if (!appRunningManager_) {
        HILOG_ERROR("app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    auto appRecord = appRunningManager_->GetAppRunningRecordByPid(pid);
    if (!appRecord) {
        HILOG_ERROR("Get app running record by pid failed. pid: %{public}d", pid);
        return false;
    }
    appRecord->SetContinuousTaskAppState(isContinuousTask);
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord);

    return ERR_OK;
}
#endif

int32_t AppMgrServiceInner::NotifyUnLoadRepairPatch(const std::string &bundleName,
    const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    if (!appRunningManager_) {
        HILOG_ERROR("app running manager is nullptr.");
        return ERR_INVALID_OPERATION;
    }

    return appRunningManager_->NotifyUnLoadRepairPatch(bundleName, callback);
}

bool AppMgrServiceInner::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    if (!CheckGetRunningInfoPermission()) {
        return false;
    }
    for (const auto &it : runningSharedBundleList_) {
        for (const auto &item : it.second) {
            if (item.bundleName == bundleName && item.versionCode == versionCode) {
                return true;
            }
        }
    }
    return false;
}

void AppMgrServiceInner::SetRunningSharedBundleList(const std::string &bundleName,
    const std::vector<BaseSharedBundleInfo> baseSharedBundleInfoList)
{
    runningSharedBundleList_.try_emplace(bundleName, baseSharedBundleInfoList);
}

void AppMgrServiceInner::RemoveRunningSharedBundleList(const std::string &bundleName)
{
    auto iterator = runningSharedBundleList_.find(bundleName);
    if (iterator == runningSharedBundleList_.end()) {
        return;
    }
    runningSharedBundleList_.erase(iterator);
}
}  // namespace AppExecFwk
}  // namespace OHOS
