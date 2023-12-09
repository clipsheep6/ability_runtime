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

#include "app_mgr_service.h"

#include <chrono>
#include <nlohmann/json.hpp>
#include <sys/types.h>
#include <thread>

#include "app_death_recipient.h"
#include "app_mgr_constants.h"
#include "datetime_ex.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "perf_profile.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
using namespace std::chrono_literals;
#ifdef ABILITY_COMMAND_FOR_TEST
static const int APP_MS_BLOCK = 65;
#endif
const std::string TASK_INIT_APPMGRSERVICEINNER = "InitAppMgrServiceInnerTask";
const std::string TASK_ATTACH_APPLICATION = "AttachApplicationTask";
const std::string TASK_APPLICATION_FOREGROUNDED = "ApplicationForegroundedTask";
const std::string TASK_APPLICATION_BACKGROUNDED = "ApplicationBackgroundedTask";
const std::string TASK_APPLICATION_TERMINATED = "ApplicationTerminatedTask";
const std::string TASK_ABILITY_CLEANED = "AbilityCleanedTask";
const std::string TASK_ADD_APP_DEATH_RECIPIENT = "AddAppRecipientTask";
const std::string TASK_CLEAR_UP_APPLICATION_DATA = "ClearUpApplicationDataTask";
const std::string TASK_STARTUP_RESIDENT_PROCESS = "StartupResidentProcess";
const std::string TASK_ADD_ABILITY_STAGE_DONE = "AddAbilityStageDone";
const std::string TASK_START_USER_TEST_PROCESS = "StartUserTestProcess";
const std::string TASK_FINISH_USER_TEST = "FinishUserTest";
const std::string TASK_ATTACH_RENDER_PROCESS = "AttachRenderTask";
const std::string TASK_ATTACH_CHILD_PROCESS = "AttachChildProcessTask";
const std::string TASK_EXIT_CHILD_PROCESS_SAFELY = "ExitChildProcessSafelyTask";
}  // namespace

REGISTER_SYSTEM_ABILITY_BY_ID(AppMgrService, APP_MGR_SERVICE_ID, true);

AppMgrService::AppMgrService()
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    HILOG_INFO("instance created with no para");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::AppMgrService(const int32_t serviceId, bool runOnCreate) : SystemAbility(serviceId, runOnCreate)
{
    appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    HILOG_INFO("instance created");
    PerfProfile::GetInstance().SetAmsLoadStartTime(GetTickCount());
}

AppMgrService::~AppMgrService()
{
    HILOG_INFO("instance destroyed");
}

void AppMgrService::OnStart()
{
    HILOG_INFO("ready to start service");
    if (appMgrServiceState_.serviceRunningState == ServiceRunningState::STATE_RUNNING) {
        HILOG_WARN("failed to start service since it's already running");
        return;
    }

    ErrCode errCode = Init();
    if (FAILED(errCode)) {
        HILOG_ERROR("init failed, errCode: %{public}08x", errCode);
        return;
    }
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_RUNNING;
    AddSystemAbilityListener(WINDOW_MANAGER_SERVICE_ID);
    HILOG_INFO("start service success");
    PerfProfile::GetInstance().SetAmsLoadEndTime(GetTickCount());
    PerfProfile::GetInstance().Dump();
}

void AppMgrService::OnStop()
{
    HILOG_INFO("ready to stop service");
    appMgrServiceState_.serviceRunningState = ServiceRunningState::STATE_NOT_START;
    eventHandler_.reset();
    taskHandler_.reset();
    if (appMgrServiceInner_) {
        appMgrServiceInner_->OnStop();
    }
    HILOG_INFO("stop service success");
}

void AppMgrService::SetInnerService(const std::shared_ptr<AppMgrServiceInner> &innerService)
{
    appMgrServiceInner_ = innerService;
}

AppMgrServiceState AppMgrService::QueryServiceState()
{
    if (appMgrServiceInner_) {
        appMgrServiceState_.connectionState = appMgrServiceInner_->QueryAppSpawnConnectionState();
    }
    return appMgrServiceState_;
}

ErrCode AppMgrService::Init()
{
    HILOG_INFO("ready to init");
    if (!appMgrServiceInner_) {
        HILOG_ERROR("init failed without inner service");
        return ERR_INVALID_OPERATION;
    }

    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("app_mgr_task_queue");
    eventHandler_ = std::make_shared<AMSEventHandler>(taskHandler_, appMgrServiceInner_);
    appMgrServiceInner_->SetTaskHandler(taskHandler_);
    appMgrServiceInner_->SetEventHandler(eventHandler_);
    std::function<void()> initAppMgrServiceInnerTask =
        std::bind(&AppMgrServiceInner::Init, appMgrServiceInner_);
    taskHandler_->SubmitTask(initAppMgrServiceInnerTask, TASK_INIT_APPMGRSERVICEINNER);

    ErrCode openErr = appMgrServiceInner_->OpenAppSpawnConnection();
    if (FAILED(openErr)) {
        HILOG_WARN("failed to connect to AppSpawnDaemon! errCode: %{public}08x", openErr);
    }
    if (!Publish(this)) {
        HILOG_ERROR("failed to publish app mgr service to systemAbilityMgr");
        return ERR_APPEXECFWK_SERVICE_NOT_CONNECTED;
    }
    amsMgrScheduler_ = new (std::nothrow) AmsMgrScheduler(appMgrServiceInner_, taskHandler_);
    if (!amsMgrScheduler_) {
        HILOG_ERROR("init failed without ams scheduler");
        return ERR_INVALID_OPERATION;
    }
    HILOG_INFO("init success");
    return ERR_OK;
}

int32_t AppMgrService::CheckPermission(
    [[maybe_unused]] const int32_t recordId, [[maybe_unused]] const std::string &permission)
{
    HILOG_INFO("check application's permission");

    return ERR_OK;
}

void AppMgrService::AttachApplication(const sptr<IRemoteObject> &app)
{
    HILOG_INFO("LoadLifecycle: appMGR receives a binding request.");
    if (!IsReady()) {
        HILOG_ERROR("AttachApplication failed, not ready.");
        return;
    }

    pid_t pid = IPCSkeleton::GetCallingPid();
    AddAppDeathRecipient(pid);
    std::function<void()> attachApplicationFunc =
        std::bind(&AppMgrServiceInner::AttachApplication, appMgrServiceInner_, pid, iface_cast<IAppScheduler>(app));
    taskHandler_->SubmitTask(attachApplicationFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_ATTACH_APPLICATION,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ApplicationForegrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeSelfCalledByRecordId(recordId)) {
        return;
    }
    std::function<void()> applicationForegroundedFunc =
        std::bind(&AppMgrServiceInner::ApplicationForegrounded, appMgrServiceInner_, recordId);
    taskHandler_->SubmitTask(applicationForegroundedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_APPLICATION_FOREGROUNDED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ApplicationBackgrounded(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeSelfCalledByRecordId(recordId)) {
        return;
    }
    std::function<void()> applicationBackgroundedFunc =
        std::bind(&AppMgrServiceInner::ApplicationBackgrounded, appMgrServiceInner_, recordId);
    taskHandler_->SubmitTask(applicationBackgroundedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_APPLICATION_BACKGROUNDED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ApplicationTerminated(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeSelfCalledByRecordId(recordId)) {
        return;
    }
    std::function<void()> applicationTerminatedFunc =
        std::bind(&AppMgrServiceInner::ApplicationTerminated, appMgrServiceInner_, recordId);
    taskHandler_->SubmitTask(applicationTerminatedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_APPLICATION_TERMINATED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    if (!IsReady()) {
        return;
    }

    auto callerUid = IPCSkeleton::GetCallingUid();
    auto appRecord = appMgrServiceInner_->GetTerminatingAppRunningRecord(token);
    if (!appRecord || appRecord->GetUid() != callerUid) {
        HILOG_ERROR("Permission verification failed.");
        return;
    }

    std::function<void()> abilityCleanedFunc =
        std::bind(&AppMgrServiceInner::AbilityTerminated, appMgrServiceInner_, token);
    taskHandler_->SubmitTask(abilityCleanedFunc, AAFwk::TaskAttribute{
        .taskName_ = TASK_ABILITY_CLEANED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

bool AppMgrService::IsReady() const
{
    if (appMgrServiceInner_ && taskHandler_ && eventHandler_) {
        return true;
    }

    HILOG_WARN("Not ready");
    return false;
}

void AppMgrService::AddAppDeathRecipient(const pid_t pid) const
{
    if (!IsReady()) {
        return;
    }
    sptr<AppDeathRecipient> appDeathRecipient = new AppDeathRecipient();
    appDeathRecipient->SetTaskHandler(taskHandler_);
    appDeathRecipient->SetAppMgrServiceInner(appMgrServiceInner_);
    std::function<void()> addAppRecipientFunc =
        std::bind(&AppMgrServiceInner::AddAppDeathRecipient, appMgrServiceInner_, pid, appDeathRecipient);
    taskHandler_->SubmitTask(addAppRecipientFunc, TASK_ADD_APP_DEATH_RECIPIENT);
}

void AppMgrService::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    if (!IsReady()) {
        return;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    pid_t pid = getpid();
    if (callingPid != pid) {
        HILOG_ERROR("Not this process call.");
        return;
    }
    HILOG_INFO("Notify start resident process");
    std::function <void()> startupResidentProcess =
        std::bind(&AppMgrServiceInner::LoadResidentProcess, appMgrServiceInner_, bundleInfos);
    taskHandler_->SubmitTask(startupResidentProcess, AAFwk::TaskAttribute{
        .taskName_ = TASK_STARTUP_RESIDENT_PROCESS,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

sptr<IAmsMgr> AppMgrService::GetAmsMgr()
{
    return amsMgrScheduler_;
}

int32_t AppMgrService::ClearUpApplicationData(const std::string &bundleName, const int32_t userId)
{
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    if (remoteClientManager == nullptr) {
        HILOG_ERROR("The remoteClientManager is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != 0 || userId < 0) {
        std::string callerBundleName;
        auto result = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, callerBundleName));
        if (result != ERR_OK) {
            HILOG_ERROR("GetBundleName failed: %{public}d.", result);
            return ERR_INVALID_OPERATION;
        }
        auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
        if (!isSaCall && bundleName != callerBundleName) {
            auto isCallingPerm = AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                AAFwk::PermissionConstants::PERMISSION_CLEAN_APPLICATION_DATA);
            if (!isCallingPerm) {
                HILOG_ERROR("Permission verification failed");
                return ERR_PERMISSION_DENIED;
            }
        }
    }

    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t uid = IPCSkeleton::GetCallingUid();
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> clearUpApplicationDataFunc =
        std::bind(&AppMgrServiceInner::ClearUpApplicationData, appMgrServiceInner_, bundleName, uid, pid, userId);
    taskHandler_->SubmitTask(clearUpApplicationDataFunc, TASK_CLEAR_UP_APPLICATION_DATA);
    return ERR_OK;
}

int32_t AppMgrService::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRunningProcesses(info);
}

int32_t AppMgrService::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAllRenderProcesses(info);
}

int32_t AppMgrService::JudgeSandboxByPid(pid_t pid, bool &isSandbox)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    auto appRunningRecord = appMgrServiceInner_->GetAppRunningRecordByPid(pid);
    if (appRunningRecord && appRunningRecord->GetAppIndex() > 0) {
        isSandbox = true;
        HILOG_DEBUG("current app is a sandbox.");
        return ERR_OK;
    }
    HILOG_DEBUG("current app is not a sandbox.");
    return ERR_OK;
}

int32_t AppMgrService::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetProcessRunningInfosByUserId(info, userId);
}

int32_t AppMgrService::GetProcessRunningInformation(RunningProcessInfo &info)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetProcessRunningInformation(info);
}

int32_t AppMgrService::NotifyMemoryLevel(int32_t level)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyMemoryLevel(level);
}

int32_t AppMgrService::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->DumpHeapMemory(pid, mallocInfo);
}

void AppMgrService::AddAbilityStageDone(const int32_t recordId)
{
    if (!IsReady()) {
        return;
    }
    if (!JudgeSelfCalledByRecordId(recordId)) {
        return;
    }
    std::function <void()> addAbilityStageDone =
        std::bind(&AppMgrServiceInner::AddAbilityStageDone, appMgrServiceInner_, recordId);
    taskHandler_->SubmitTask(addAbilityStageDone, AAFwk::TaskAttribute{
        .taskName_ = TASK_ADD_ABILITY_STAGE_DONE,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

int32_t AppMgrService::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
    const std::vector<std::string> &bundleNameList)
{
    HILOG_DEBUG("begin");
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterApplicationStateObserver(observer, bundleNameList);
}

int32_t AppMgrService::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    HILOG_DEBUG("begin");
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterApplicationStateObserver(observer);
}

int32_t AppMgrService::RegisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("Not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrService::UnregisterAbilityForegroundStateObserver(const sptr<IAbilityForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("Not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterAbilityForegroundStateObserver(observer);
}

int32_t AppMgrService::GetForegroundApplications(std::vector<AppStateData> &list)
{
    HILOG_DEBUG("begin");
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetForegroundApplications(list);
}

int AppMgrService::StartUserTestProcess(const AAFwk::Want &want, const sptr<IRemoteObject> &observer,
    const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return ERR_INVALID_OPERATION;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        HILOG_ERROR("StartUserTestProcess is not shell call.");
        return ERR_INVALID_OPERATION;
    }
    std::function<void()> startUserTestProcessFunc =
        std::bind(&AppMgrServiceInner::StartUserTestProcess, appMgrServiceInner_, want, observer, bundleInfo, userId);
    taskHandler_->SubmitTask(startUserTestProcessFunc, TASK_START_USER_TEST_PROCESS);
    return ERR_OK;
}

int AppMgrService::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    if (!IsReady()) {
        HILOG_ERROR("Not ready");
        return ERR_INVALID_OPERATION;
    }
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    if (remoteClientManager == nullptr) {
        HILOG_ERROR("The remoteClientManager is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return ERR_INVALID_OPERATION;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    auto result = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callingUid, callerBundleName));
    if (result == ERR_OK) {
        HILOG_INFO("The callingPid_ is %{public}s.", callerBundleName.c_str());
        if (bundleName != callerBundleName) {
            HILOG_ERROR("Not this process call.");
            return ERR_INVALID_OPERATION;
        }
    } else {
        HILOG_ERROR("GetBundleName failed: %{public}d.", result);
        return ERR_INVALID_OPERATION;
    }
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    std::function<void()> finishUserTestProcessFunc =
        std::bind(&AppMgrServiceInner::FinishUserTest, appMgrServiceInner_, msg, resultCode, bundleName, callingPid);
    taskHandler_->SubmitTask(finishUserTestProcessFunc, TASK_FINISH_USER_TEST);
    return ERR_OK;
}

int AppMgrService::Dump(int fd, const std::vector<std::u16string>& args)
{
    if (!IsReady()) {
        HILOG_ERROR("not ready.");
        return ERR_APPEXECFWK_HIDUMP_ERROR;
    }

    std::string result;
    Dump(args, result);
    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        HILOG_ERROR("dprintf error.");
        return ERR_APPEXECFWK_HIDUMP_ERROR;
    }
    return ERR_OK;
}

void AppMgrService::Dump(const std::vector<std::u16string>& args, std::string& result) const
{
    auto size = args.size();
    if (size == 0) {
        ShowHelp(result);
        return;
    }

    std::string optionKey = Str16ToStr8(args[0]);
    if (optionKey != "-h") {
        result.append("error: unkown option.\n");
    }
    ShowHelp(result);
}

void AppMgrService::ShowHelp(std::string& result) const
{
    result.append("Usage:\n")
        .append("-h                          ")
        .append("help text for the tool\n");
}

void AppMgrService::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return;
    }
    if (!JudgeSelfCalledByRecordId(recordId)) {
        return;
    }
    auto task = [=]() { appMgrServiceInner_->ScheduleAcceptWantDone(recordId, want, flag); };
    taskHandler_->SubmitTask(task);
}

void AppMgrService::ScheduleNewProcessRequestDone(const int32_t recordId, const AAFwk::Want &want,
    const std::string &flag)
{
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return;
    }
    if (!JudgeSelfCalledByRecordId(recordId)) {
        return;
    }
    auto task = [=]() { appMgrServiceInner_->ScheduleNewProcessRequestDone(recordId, want, flag); };
    taskHandler_->SubmitTask(task, AAFwk::TaskQoS::USER_INTERACTIVE);
}

int AppMgrService::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    if (!IsReady()) {
        HILOG_ERROR("not ready");
        return ERR_INVALID_OPERATION;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        HILOG_ERROR("Not SA call.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetAbilityRecordsByProcessID(pid, tokens);
}

int32_t AppMgrService::PreStartNWebSpawnProcess()
{
    HILOG_INFO("PreStartNWebSpawnProcess");
    if (!IsReady()) {
        HILOG_ERROR("PreStartNWebSpawnProcess failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->PreStartNWebSpawnProcess(IPCSkeleton::GetCallingPid());
}

int32_t AppMgrService::StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, int32_t crashFd, pid_t &renderPid)
{
    if (!IsReady()) {
        HILOG_ERROR("StartRenderProcess failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->StartRenderProcess(IPCSkeleton::GetCallingPid(),
        renderParam, ipcFd, sharedFd, crashFd, renderPid);
}

void AppMgrService::AttachRenderProcess(const sptr<IRemoteObject> &scheduler)
{
    HILOG_DEBUG("AttachRenderProcess called.");
    if (!IsReady()) {
        HILOG_ERROR("AttachRenderProcess failed, not ready.");
        return;
    }

    auto pid = IPCSkeleton::GetCallingPid();
    auto fun = std::bind(&AppMgrServiceInner::AttachRenderProcess,
        appMgrServiceInner_, pid, iface_cast<IRenderScheduler>(scheduler));
    taskHandler_->SubmitTask(fun, AAFwk::TaskAttribute{
        .taskName_ = TASK_ATTACH_RENDER_PROCESS,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

int32_t AppMgrService::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    if (!IsReady()) {
        HILOG_ERROR("GetRenderProcessTerminationStatus failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetRenderProcessTerminationStatus(renderPid, status);
}

int32_t AppMgrService::GetConfiguration(Configuration& config)
{
    if (!IsReady()) {
        HILOG_ERROR("GetConfiguration failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }
    config = *(appMgrServiceInner_->GetConfiguration());
    return ERR_OK;
}

int32_t AppMgrService::UpdateConfiguration(const Configuration& config)
{
    if (!IsReady()) {
        HILOG_ERROR("UpdateConfiguration failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UpdateConfiguration(config);
}

int32_t AppMgrService::RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    if (!IsReady()) {
        HILOG_ERROR("RegisterConfigurationObserver failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterConfigurationObserver(observer);
}

int32_t AppMgrService::UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    if (!IsReady()) {
        HILOG_ERROR("UnregisterConfigurationObserver failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterConfigurationObserver(observer);
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AppMgrService::BlockAppService()
{
    HILOG_DEBUG("begin");
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    auto task = [=]() {
        while (1) {
            HILOG_DEBUG("begin block app service");
            std::this_thread::sleep_for(APP_MS_BLOCK*1s);
        }
    };
    taskHandler_->SubmitTask(task);
    return ERR_OK;
}
#endif

bool AppMgrService::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return false;
    }

    return appMgrServiceInner_->GetAppRunningStateByBundleName(bundleName);
}

int32_t AppMgrService::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyLoadRepairPatch(bundleName, callback);
}

int32_t AppMgrService::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyHotReloadPage(bundleName, callback);
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrService::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->SetContinuousTaskProcess(pid, isContinuousTask);
}
#endif

int32_t AppMgrService::NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (!isSaCall) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyUnLoadRepairPatch(bundleName, callback);
}

bool AppMgrService::JudgeSelfCalledByRecordId(int32_t recordId)
{
    auto isSaCall = AAFwk::PermissionVerification::GetInstance()->IsSACall();
    if (isSaCall) {
        return true;
    }

    if (appMgrServiceInner_ == nullptr) {
        return false;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner_->GetAppRunningRecordByAppRecordId(recordId);
    if (appRecord == nullptr || ((appRecord->GetApplicationInfo())->accessTokenId) != callingTokenId) {
        HILOG_ERROR("Is not self, not enabled");
        return false;
    }

    return true;
}

bool AppMgrService::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsSharedBundleRunning(bundleName, versionCode);
}

int32_t AppMgrService::StartNativeProcessForDebugger(const AAFwk::Want &want)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    auto isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    if (!isShellCall) {
        HILOG_ERROR("permission denied, only called by shell.");
        return ERR_INVALID_OPERATION;
    }
    auto ret = appMgrServiceInner_->StartNativeProcessForDebugger(want);
    if (ret != ERR_OK) {
        HILOG_ERROR("debuggablePipe fail to start native process.");
    }
    return ret;
}

int32_t AppMgrService::GetBundleNameByPid(const int32_t pid, std::string &bundleName, int32_t &uid)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetBundleNameByPid(pid, bundleName, uid);
}

int32_t AppMgrService::NotifyAppFault(const FaultData &faultData)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }

    auto ret = appMgrServiceInner_->NotifyAppFault(faultData);
    if (ret != ERR_OK) {
        HILOG_ERROR("Notify fault data fail.");
    }
    return ret;
}

int32_t AppMgrService::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }

    auto ret = appMgrServiceInner_->NotifyAppFaultBySA(faultData);
    if (ret != ERR_OK) {
        HILOG_ERROR("Notify fault data fail.");
    }
    return ret;
}

int32_t AppMgrService::GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetProcessMemoryByPid(pid, memorySize);
}

int32_t AppMgrService::GetRunningProcessInformation(const std::string &bundleName, int32_t userId,
    std::vector<RunningProcessInfo> &info)
{
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }

    return appMgrServiceInner_->GetRunningProcessInformation(bundleName, userId, info);
}

void AppMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOG_INFO("systemAbilityId: %{public}d add", systemAbilityId);
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return;
    }

    if (systemAbilityId != WINDOW_MANAGER_SERVICE_ID) {
        return;
    }

    appMgrServiceInner_->InitFocusListener();
    appMgrServiceInner_->InitWindowVisibilityChangedListener();
}

void AppMgrService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOG_INFO("systemAbilityId: %{public}d remove", systemAbilityId);
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return;
    }

    if (systemAbilityId != WINDOW_MANAGER_SERVICE_ID) {
        return;
    }

    appMgrServiceInner_->FreeFocusListener();
    appMgrServiceInner_->FreeWindowVisibilityChangedListener();
}

int32_t AppMgrService::ChangeAppGcState(pid_t pid, int32_t state)
{
    HILOG_DEBUG("called.");
    if (!appMgrServiceInner_) {
        return ERR_INVALID_VALUE;
    }
    return appMgrServiceInner_->ChangeAppGcState(pid, state);
}

int32_t AppMgrService::NotifyPageShow(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    HILOG_DEBUG("bundleName: %{public}s, moduelName: %{public}s, abilityName: %{public}s, pageName: %{public}s",
        pageStateData.bundleName.c_str(), pageStateData.moduleName.c_str(), pageStateData.abilityName.c_str(),
        pageStateData.pageName.c_str());
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyPageShow(token, pageStateData);
}

int32_t AppMgrService::NotifyPageHide(const sptr<IRemoteObject> &token, const PageStateData &pageStateData)
{
    HILOG_DEBUG("bundleName: %{public}s, moduelName: %{public}s, abilityName: %{public}s, pageName: %{public}s",
        pageStateData.bundleName.c_str(), pageStateData.moduleName.c_str(), pageStateData.abilityName.c_str(),
        pageStateData.pageName.c_str());
    if (!IsReady()) {
        HILOG_ERROR("AppMgrService is not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->NotifyPageHide(token, pageStateData);
}

int32_t AppMgrService::RegisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("Not ready");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterAppRunningStatusListener(listener);
}

int32_t AppMgrService::UnregisterAppRunningStatusListener(const sptr<IRemoteObject> &listener)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("Not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterAppRunningStatusListener(listener);
}

int32_t AppMgrService::RegisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("Not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->RegisterAppForegroundStateObserver(observer);
}

int32_t AppMgrService::UnregisterAppForegroundStateObserver(const sptr<IAppForegroundStateObserver> &observer)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("Not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->UnregisterAppForegroundStateObserver(observer);
}

int32_t AppMgrService::IsApplicationRunning(const std::string &bundleName, bool &isRunning)
{
    if (!IsReady()) {
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->IsApplicationRunning(bundleName, isRunning);
}

int32_t AppMgrService::StartChildProcess(const std::string &srcEntry, pid_t &childPid)
{
    HILOG_DEBUG("Called.");
    if (!IsReady()) {
        HILOG_ERROR("StartChildProcess failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->StartChildProcess(IPCSkeleton::GetCallingPid(), srcEntry, childPid);
}

int32_t AppMgrService::GetChildProcessInfoForSelf(ChildProcessInfo &info)
{
    if (!IsReady()) {
        HILOG_ERROR("StartChildProcess failed, AppMgrService not ready.");
        return ERR_INVALID_OPERATION;
    }
    return appMgrServiceInner_->GetChildProcessInfoForSelf(info);
}

void AppMgrService::AttachChildProcess(const sptr<IRemoteObject> &childScheduler)
{
    HILOG_DEBUG("AttachChildProcess.");
    if (!IsReady()) {
        HILOG_ERROR("AttachChildProcess failed, not ready.");
        return;
    }
    if (!taskHandler_) {
        HILOG_ERROR("taskHandler_ is null.");
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> task = std::bind(&AppMgrServiceInner::AttachChildProcess,
        appMgrServiceInner_, pid, iface_cast<IChildScheduler>(childScheduler));
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = TASK_ATTACH_CHILD_PROCESS,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}

void AppMgrService::ExitChildProcessSafely()
{
    if (!IsReady()) {
        HILOG_ERROR("ExitChildProcessSafely failed, AppMgrService not ready.");
        return;
    }
    if (!taskHandler_) {
        HILOG_ERROR("taskHandler_ is null.");
        return;
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    std::function<void()> task = std::bind(&AppMgrServiceInner::ExitChildProcessSafelyByChildPid,
        appMgrServiceInner_, pid);
    taskHandler_->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = TASK_EXIT_CHILD_PROCESS_SAFELY,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}
}  // namespace AppExecFwk
}  // namespace OHOS
