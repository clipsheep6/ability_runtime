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

#include "app_mgr_stub.h"

#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "ability_info.h"
#include "app_mgr_proxy.h"
#include "app_scheduler_interface.h"
#include "appexecfwk_errors.h"
#include "hitrace_meter.h"
#include "hilog_wrapper.h"
#include "iapp_state_callback.h"
#include "want.h"
#include "bundle_info.h"
#include "app_malloc_info.h"

namespace OHOS {
namespace AppExecFwk {
constexpr int32_t CYCLE_LIMIT = 1000;
AppMgrStub::AppMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_ATTACH_APPLICATION)] =
        &AppMgrStub::HandleAttachApplication;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_FOREGROUNDED)] =
        &AppMgrStub::HandleApplicationForegrounded;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_BACKGROUNDED)] =
        &AppMgrStub::HandleApplicationBackgrounded;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED)] =
        &AppMgrStub::HandleApplicationTerminated;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_CHECK_PERMISSION)] =
        &AppMgrStub::HandleCheckPermission;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_ABILITY_CLEANED)] =
        &AppMgrStub::HandleAbilityCleaned;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_MGR_INSTANCE)] = &AppMgrStub::HandleGetAmsMgr;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA)] =
        &AppMgrStub::HandleClearUpApplicationData;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RUNNING_PROCESSES)] =
        &AppMgrStub::HandleGetAllRunningProcesses;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_MEMORY_LEVEL)] =
        &AppMgrStub::HandleNotifyMemoryLevel;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID)] =
        &AppMgrStub::HandleGetProcessRunningInfosByUserId;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_ADD_ABILITY_STAGE_INFO_DONE)] =
        &AppMgrStub::HandleAddAbilityStageDone;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::STARTUP_RESIDENT_PROCESS)] =
        &AppMgrStub::HandleStartupResidentProcess;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER)] =
        &AppMgrStub::HandleRegisterApplicationStateObserver;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APPLICATION_STATE_OBSERVER)] =
        &AppMgrStub::HandleUnregisterApplicationStateObserver;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::GET_FOREGROUND_APPLICATIONS)] =
        &AppMgrStub::HandleGetForegroundApplications;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::START_USER_TEST_PROCESS)] =
        &AppMgrStub::HandleStartUserTestProcess;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::FINISH_USER_TEST)] =
        &AppMgrStub::HandleFinishUserTest;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_ACCEPT_WANT_DONE)] =
        &AppMgrStub::HandleScheduleAcceptWantDone;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID)] =
        &AppMgrStub::HandleGetAbilityRecordsByProcessID;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS)] =
        &AppMgrStub::HandlePreStartNWebSpawnProcess;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::START_RENDER_PROCESS)] =
        &AppMgrStub::HandleStartRenderProcess;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::ATTACH_RENDER_PROCESS)] =
        &AppMgrStub::HandleAttachRenderProcess;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::GET_RENDER_PROCESS_TERMINATION_STATUS)] =
        &AppMgrStub::HandleGetRenderProcessTerminationStatus;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::GET_CONFIGURATION)] =
        &AppMgrStub::HandleGetConfiguration;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION)] =
        &AppMgrStub::HandleUpdateConfiguration;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_CONFIGURATION_OBSERVER)] =
        &AppMgrStub::HandleRegisterConfigurationObserver;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_CONFIGURATION_OBSERVER)] =
        &AppMgrStub::HandleUnregisterConfigurationObserver;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_PROCESS_RUNNING_INFORMATION)] =
        &AppMgrStub::HandleGetProcessRunningInformation;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_HEAP_MEMORY_PROCESS)] =
        &AppMgrStub::HandleDumpHeapMemory;
#ifdef ABILITY_COMMAND_FOR_TEST
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::BLOCK_APP_SERVICE)] =
        &AppMgrStub::HandleBlockAppServiceDone;
#endif
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_STATE)] =
        &AppMgrStub::HandleGetAppRunningStateByBundleName;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_LOAD_REPAIR_PATCH)] =
        &AppMgrStub::HandleNotifyLoadRepairPatch;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_HOT_RELOAD_PAGE)] =
        &AppMgrStub::HandleNotifyHotReloadPage;
#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS)] =
        &AppMgrStub::HandleSetContinuousTaskProcess;
#endif
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_UNLOAD_REPAIR_PATCH)] =
        &AppMgrStub::HandleNotifyUnLoadRepairPatch;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::IS_SHARED_BUNDLE_RUNNING)] =
        &AppMgrStub::HandleIsSharedBundleRunning;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::START_NATIVE_PROCESS_FOR_DEBUGGER)] =
        &AppMgrStub::HandleStartNativeProcessForDebugger;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT)] =
        &AppMgrStub::HandleNotifyFault;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA)] =
        &AppMgrStub::HandleNotifyFaultBySA;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::JUDGE_SANDBOX_BY_PID)] =
        &AppMgrStub::HandleJudgeSandboxByPid;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::GET_BUNDLE_NAME_BY_PID)] =
        &AppMgrStub::HandleGetBundleNameByPid;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RENDER_PROCESSES)] =
        &AppMgrStub::HandleGetAllRenderProcesses;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::GET_PROCESS_MEMORY_BY_PID)] =
        &AppMgrStub::HandleGetProcessMemoryByPid;
    memberFuncMap_[static_cast<uint32_t>(IAppMgr::Message::GET_PIDS_BY_BUNDLENAME)] =
        &AppMgrStub::HandleGetRunningProcessInformation;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PAGE_SHOW)] =
        &AppMgrStub::HandleNotifyPageShow;
    memberFuncMap_[static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_PAGE_HIDE)] =
        &AppMgrStub::HandleNotifyPageHide;
}

AppMgrStub::~AppMgrStub()
{
    memberFuncMap_.clear();
}

int AppMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOG_DEBUG("AppMgrStub::OnReceived, code = %{public}u, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = AppMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AppMgrStub::HandleAttachApplication(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> client = data.ReadRemoteObject();
    AttachApplication(client);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationForegrounded(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ApplicationForegrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationBackgrounded(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ApplicationBackgrounded(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleApplicationTerminated(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    ApplicationTerminated(data.ReadInt32());
    return NO_ERROR;
}

int32_t AppMgrStub::HandleCheckPermission(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t recordId = data.ReadInt32();
    std::string permission = data.ReadString();
    int32_t result = CheckPermission(recordId, permission);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAbilityCleaned(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    AbilityCleaned(token);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAmsMgr(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t result = NO_ERROR;
    sptr<IAmsMgr> amsMgr = GetAmsMgr();
    if (!amsMgr) {
        HILOG_ERROR("abilitymgr instance is nullptr");
        result = ERR_NO_INIT;
    } else {
        if (!reply.WriteRemoteObject(amsMgr->AsObject())) {
            HILOG_ERROR("failed to reply abilitymgr instance to client, for write parcel error");
            result = ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleClearUpApplicationData(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t result = ClearUpApplicationData(bundleName);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRunningProcesses(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<RunningProcessInfo> info;
    auto result = GetAllRunningProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessRunningInfosByUserId(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t userId = data.ReadInt32();
    std::vector<RunningProcessInfo> info;
    auto result = GetProcessRunningInfosByUserId(info, userId);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAllRenderProcesses(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<RenderProcessInfo> info;
    auto result = GetAllRenderProcesses(info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleJudgeSandboxByPid(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    bool isSandbox = false;
    auto result = JudgeSandboxByPid(pid, isSandbox);
    if (!reply.WriteBool(isSandbox)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessRunningInformation(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    RunningProcessInfo info;
    auto result = GetProcessRunningInformation(info);
    if (!reply.WriteParcelable(&info)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleAddAbilityStageDone(MessageParcel &data, MessageParcel &reply)
{
    int32_t recordId = data.ReadInt32();
    AddAbilityStageDone(recordId);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyMemoryLevel(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t level = data.ReadInt32();
    auto result = NotifyMemoryLevel(level);
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleDumpHeapMemory(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("AppMgrStub::HandleDumpHeapMemory.");
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    struct OHOS::AppExecFwk::MallocInfo mallocInfo;
    auto result = DumpHeapMemory(pid, mallocInfo);
    if (result != NO_ERROR) {
        return result;
    }
    reply.WriteParcelable(&mallocInfo);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartupResidentProcess(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    int32_t infoSize = data.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppExecFwk::BundleInfo> bundleInfo(data.ReadParcelable<AppExecFwk::BundleInfo>());
        if (!bundleInfo) {
            HILOG_ERROR("Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        bundleInfos.emplace_back(*bundleInfo);
    }
    StartupResidentProcess(bundleInfos);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> bundleNameList;
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadRemoteObject());
    data.ReadStringVector(&bundleNameList);
    int32_t result = RegisterApplicationStateObserver(callback, bundleNameList);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterApplicationStateObserver(MessageParcel &data, MessageParcel &reply)
{
    auto callback = iface_cast<AppExecFwk::IApplicationStateObserver>(data.ReadRemoteObject());
    int32_t result = UnregisterApplicationStateObserver(callback);
    reply.WriteInt32(result);
    return NO_ERROR;
}


int32_t AppMgrStub::HandleGetForegroundApplications(MessageParcel &data, MessageParcel &reply)
{
    std::vector<AppStateData> appStateDatas;
    int32_t result = GetForegroundApplications(appStateDatas);
    reply.WriteInt32(appStateDatas.size());
    for (auto &it : appStateDatas) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleStartUserTestProcess(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    std::unique_ptr<BundleInfo> bundleInfo(data.ReadParcelable<BundleInfo>());
    if (bundleInfo == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto observer = data.ReadRemoteObject();
    int32_t userId = data.ReadInt32();
    int32_t result = StartUserTestProcess(*want, observer, *bundleInfo, userId);
    reply.WriteInt32(result);
    return result;
}

int32_t AppMgrStub::HandleFinishUserTest(MessageParcel &data, MessageParcel &reply)
{
    std::string msg = data.ReadString();
    int64_t resultCode = data.ReadInt64();
    std::string bundleName = data.ReadString();
    int32_t result = FinishUserTest(msg, resultCode, bundleName);
    reply.WriteInt32(result);
    return result;
}

int32_t AppMgrStub::RegisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer,
    const std::vector<std::string> &bundleNameList)
{
    return NO_ERROR;
}

int32_t AppMgrStub::UnregisterApplicationStateObserver(const sptr<IApplicationStateObserver> &observer)
{
    return NO_ERROR;
}

int32_t AppMgrStub::GetForegroundApplications(std::vector<AppStateData> &list)
{
    return NO_ERROR;
}

int32_t AppMgrStub::HandleScheduleAcceptWantDone(MessageParcel &data, MessageParcel &reply)
{
    auto recordId = data.ReadInt32();
    AAFwk::Want *want = data.ReadParcelable<AAFwk::Want>();
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto flag = data.ReadString();

    ScheduleAcceptWantDone(recordId, *want, flag);
    delete want;
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetAbilityRecordsByProcessID(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    int32_t pid = data.ReadInt32();
    std::vector<sptr<IRemoteObject>> tokens;
    auto result = GetAbilityRecordsByProcessID(pid, tokens);
    reply.WriteInt32(tokens.size());
    for (auto &it : tokens) {
        if (!reply.WriteRemoteObject(it)) {
            HILOG_ERROR("failed to write query result.");
            return ERR_FLATTEN_OBJECT;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandlePreStartNWebSpawnProcess(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("HandlePreNWebSpawnProcess");
    int32_t result = PreStartNWebSpawnProcess();
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleStartRenderProcess(MessageParcel &data, MessageParcel &reply)
{
    std::string renderParam = data.ReadString();
    int32_t ipcFd = data.ReadFileDescriptor();
    int32_t sharedFd = data.ReadFileDescriptor();
    int32_t crashFd = data.ReadFileDescriptor();
    int32_t renderPid = 0;
    int32_t result =
        StartRenderProcess(renderParam, ipcFd, sharedFd, crashFd, renderPid);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(renderPid)) {
        HILOG_ERROR("write renderPid error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleAttachRenderProcess(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> scheduler = data.ReadRemoteObject();
    AttachRenderProcess(scheduler);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRenderProcessTerminationStatus(MessageParcel &data, MessageParcel &reply)
{
    int32_t renderPid = data.ReadInt32();
    if (renderPid <= 0) {
        HILOG_ERROR("invalid renderPid value");
        return ERR_INVALID_VALUE;
    }
    int status = 0;
    int32_t result = GetRenderProcessTerminationStatus(renderPid, status);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("write result error.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(status)) {
        HILOG_ERROR("write status error.");
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t AppMgrStub::HandleGetConfiguration(MessageParcel &data, MessageParcel &reply)
{
    Configuration config;
    int ret = GetConfiguration(config);
    if (ret != ERR_OK) {
        HILOG_ERROR("AppMgrStub GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteParcelable(&config)) {
        HILOG_ERROR("AppMgrStub GetConfiguration error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUpdateConfiguration(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Configuration> config(data.ReadParcelable<Configuration>());
    if (!config) {
        HILOG_ERROR("AppMgrStub read configuration error");
        return ERR_INVALID_VALUE;
    }
    int32_t ret = UpdateConfiguration(*config);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleRegisterConfigurationObserver(MessageParcel &data, MessageParcel &reply)
{
    auto observer = iface_cast<AppExecFwk::IConfigurationObserver>(data.ReadRemoteObject());
    int32_t result = RegisterConfigurationObserver(observer);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t AppMgrStub::HandleUnregisterConfigurationObserver(MessageParcel &data, MessageParcel &reply)
{
    auto observer = iface_cast<AppExecFwk::IConfigurationObserver>(data.ReadRemoteObject());
    int32_t result = UnregisterConfigurationObserver(observer);
    reply.WriteInt32(result);
    return NO_ERROR;
}

#ifdef ABILITY_COMMAND_FOR_TEST
int32_t AppMgrStub::HandleBlockAppServiceDone(MessageParcel &data, MessageParcel &reply)
{
    HILOG_INFO("%{public}s", __func__);
    int32_t result = BlockAppService();
    reply.WriteInt32(result);
    return result;
}
#endif

int32_t AppMgrStub::HandleGetAppRunningStateByBundleName(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::string bundleName = data.ReadString();
    auto ret = GetAppRunningStateByBundleName(bundleName);
    if (!reply.WriteBool(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyLoadRepairPatch(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    auto ret = NotifyLoadRepairPatch(bundleName, callback);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyHotReloadPage(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    auto ret = NotifyHotReloadPage(bundleName, callback);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrStub::HandleSetContinuousTaskProcess(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    int32_t pid = data.ReadInt32();
    bool isContinuousTask = data.ReadBool();
    auto ret = SetContinuousTaskProcess(pid, isContinuousTask);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
#endif

int32_t AppMgrStub::HandleNotifyUnLoadRepairPatch(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::string bundleName = data.ReadString();
    auto callback = iface_cast<IQuickFixCallback>(data.ReadRemoteObject());
    auto ret = NotifyUnLoadRepairPatch(bundleName, callback);
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleIsSharedBundleRunning(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::string bundleName = data.ReadString();
    uint32_t versionCode = data.ReadUint32();
    bool result = IsSharedBundleRunning(bundleName, versionCode);
    if (!reply.WriteBool(result)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleStartNativeProcessForDebugger(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    std::unique_ptr<AAFwk::Want> want(data.ReadParcelable<AAFwk::Want>());
    if (want == nullptr) {
        HILOG_ERROR("want is nullptr");
        return ERR_INVALID_VALUE;
    }
    auto result = StartNativeProcessForDebugger(*want);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetBundleNameByPid(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = data.ReadInt32();
    std::string bundleName;
    int32_t uid;
    auto result = GetBundleNameByPid(pid, bundleName, uid);
    if (result != ERR_OK) {
        return result;
    }

    if (!reply.WriteString(bundleName)) {
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(uid)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyFault(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<FaultData> faultData(data.ReadParcelable<FaultData>());
    if (faultData == nullptr) {
        HILOG_ERROR("ReadParcelable<FaultData> failed");
        return ERR_INVALID_VALUE;
    }

    int32_t result = NotifyAppFault(*faultData);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyFaultBySA(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<AppFaultDataBySA> faultData(data.ReadParcelable<AppFaultDataBySA>());
    if (faultData == nullptr) {
        HILOG_ERROR("ReadParcelable<AppFaultDataBySA> failed");
        return ERR_INVALID_VALUE;
    }

    int32_t result = NotifyAppFaultBySA(*faultData);
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("reply write failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetProcessMemoryByPid(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int32_t pid = data.ReadInt32();
    int32_t memorySize = 0;
    auto result = GetProcessMemoryByPid(pid, memorySize);
    if (!reply.WriteInt32(memorySize)) {
        HILOG_ERROR("Memory size write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleGetRunningProcessInformation(MessageParcel &data, MessageParcel &reply)
{
    HITRACE_METER(HITRACE_TAG_APP);
    std::string bundleName = data.ReadString();
    int32_t userId = data.ReadInt32();
    std::vector<RunningProcessInfo> info;
    auto result = GetRunningProcessInformation(bundleName, userId, info);
    reply.WriteInt32(info.size());
    for (auto &it : info) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        HILOG_ERROR("fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyPageShow(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    std::string moduleName = data.ReadString();
    std::string abilityName = data.ReadString();
    std::string pageName = data.ReadString();

    auto result = NotifyPageShow(bundleName, moduleName, abilityName, pageName);
    if(!reply.WriteInt32(result)) {
        HILOG_ERROR("fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t AppMgrStub::HandleNotifyPageHide(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    std::string moduleName = data.ReadString();
    std::string abilityName = data.ReadString();
    std::string pageName = data.ReadString();

    auto result = NotifyPageHide(bundleName, moduleName, abilityName, pageName);
    if(!reply.WriteInt32(result)) {
        HILOG_ERROR("fail to write result.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
