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

#include "app_mgr_proxy.h"

#include "appexecfwk_errors.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
constexpr int32_t CYCLE_LIMIT = 1000;
AppMgrProxy::AppMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAppMgr>(impl)
{}

bool AppMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AppMgrProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
        return false;
    }
    return true;
}

void AppMgrProxy::AttachApplication(const sptr<IRemoteObject> &obj)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(obj.GetRefPtr())) {
        HILOG_ERROR("Failed to write remote object");
        return;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ATTACH_APPLICATION);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
}

void AppMgrProxy::ApplicationForegrounded(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_FOREGROUNDED);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
}

void AppMgrProxy::ApplicationBackgrounded(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_BACKGROUNDED);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
}

void AppMgrProxy::ApplicationTerminated(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteInt32(recordId);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
}

int32_t AppMgrProxy::CheckPermission(const int32_t recordId, const std::string &permission)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_PERMISSION_DENIED;
    }
    data.WriteInt32(recordId);
    data.WriteString(permission);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_CHECK_PERMISSION);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ERR_PERMISSION_DENIED;
    }
    return reply.ReadInt32();
}

void AppMgrProxy::AbilityCleaned(const sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        HILOG_ERROR("Failed to write token");
        return;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ABILITY_CLEANED);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
}

sptr<IAmsMgr> AppMgrProxy::GetAmsMgr()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_MGR_INSTANCE);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return nullptr;
    }
    sptr<IRemoteObject> object = reply.ReadRemoteObject();
    sptr<IAmsMgr> amsMgr = iface_cast<IAmsMgr>(object);
    if (!amsMgr) {
        HILOG_ERROR("ams instance is nullptr");
        return nullptr;
    }
    return amsMgr;
}

int32_t AppMgrProxy::ClearUpApplicationData(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetAllRunningProcesses(std::vector<RunningProcessInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RUNNING_PROCESSES);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        return ERR_NULL_OBJECT;
    }
    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetAllRenderProcesses(std::vector<RenderProcessInfo> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ALL_RENDER_PROCESSES);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    ret = GetParcelableInfos<RenderProcessInfo>(reply, info);
    if (ret != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    return ret;
}

int32_t AppMgrProxy::JudgeSandboxByPid(pid_t pid, bool &isSandbox)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("Pid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::JUDGE_SANDBOX_BY_PID);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    isSandbox = reply.ReadBool();
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    data.WriteInt32(userId);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_RUNNING_PROCESSES_BY_USER_ID);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    ret = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (ret != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    return ret;
}

int32_t AppMgrProxy::GetProcessRunningInformation(RunningProcessInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_PROCESS_RUNNING_INFORMATION);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    std::unique_ptr<RunningProcessInfo> infoReply(reply.ReadParcelable<RunningProcessInfo>());
    info = *infoReply;
    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyMemoryLevel(int32_t level)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(level);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_MEMORY_LEVEL);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    int result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::DumpHeapMemory(const int32_t pid, OHOS::AppExecFwk::MallocInfo &mallocInfo)
{
    HILOG_DEBUG("AppMgrProxy::DumpHeapMemory.");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(pid);

    MessageOption option(MessageOption::TF_SYNC);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::DUMP_HEAP_MEMORY_PROCESS);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    std::unique_ptr<MallocInfo> info(reply.ReadParcelable<MallocInfo>());
    if (info == nullptr) {
        HILOG_ERROR("MallocInfo ReadParcelable nullptr");
        return ERR_NULL_OBJECT;
    }
    mallocInfo = *info;
    return ret;
}

int32_t AppMgrProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
                                     MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }
    int32_t result = remote->SendRequest(code, data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, result);
        return result;
    }
    return NO_ERROR;
}

void AppMgrProxy::AddAbilityStageDone(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId)) {
        HILOG_ERROR("want write failed.");
        return;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ADD_ABILITY_STAGE_INFO_DONE);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
    return;
}

void AppMgrProxy::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(bundleInfos.size())) {
        HILOG_ERROR("write bundle info size failed.");
        return;
    }

    for (auto &bundleInfo : bundleInfos) {
        if (!data.WriteParcelable(&bundleInfo)) {
            HILOG_ERROR("write bundle info failed");
            return;
        }
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::STARTUP_RESIDENT_PROCESS);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
    return;
}

template<typename T>
int AppMgrProxy::GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            HILOG_ERROR("Read Parcelable infos failed");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    HILOG_DEBUG("get parcelable infos success");
    return NO_ERROR;
}

int AppMgrProxy::RegisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer, const std::vector<std::string> &bundleNameList)
{
    if (!observer) {
        HILOG_ERROR("observer null");
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("RegisterApplicationStateObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteStringVector(bundleNameList)) {
        HILOG_ERROR("bundleNameList write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_APPLICATION_STATE_OBSERVER);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret ;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::UnregisterApplicationStateObserver(
    const sptr<IApplicationStateObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("observer null");
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("UnregisterApplicationStateObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_APPLICATION_STATE_OBSERVER);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::GetForegroundApplications(std::vector<AppStateData> &list)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::GET_FOREGROUND_APPLICATIONS);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<AppStateData> info(reply.ReadParcelable<AppStateData>());
        if (!info) {
            HILOG_ERROR("Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        list.emplace_back(*info);
    }
    return reply.ReadInt32();
}

int AppMgrProxy::StartUserTestProcess(
    const AAFwk::Want &want, const sptr<IRemoteObject> &observer, const BundleInfo &bundleInfo, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(observer)) {
        HILOG_ERROR("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&bundleInfo)) {
        HILOG_ERROR("bundleInfo write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::START_USER_TEST_PROCESS);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

int AppMgrProxy::FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(msg)) {
        HILOG_ERROR("msg write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt64(resultCode)) {
        HILOG_ERROR("resultCode:WriteInt32 fail.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("bundleName write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::FINISH_USER_TEST);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

void AppMgrProxy::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId) || !data.WriteParcelable(&want) || !data.WriteString(flag)) {
        HILOG_ERROR("want write failed.");
        return;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_ACCEPT_WANT_DONE);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
}

int AppMgrProxy::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(pid);
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        HILOG_ERROR("infoSize is too large");
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < infoSize; i++) {
        auto iRemote = reply.ReadRemoteObject();
        tokens.emplace_back(iRemote);
    }
    return reply.ReadInt32();
}

int AppMgrProxy::PreStartNWebSpawnProcess()
{
    HILOG_INFO("PreStartNWebSpawnProcess");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::PRE_START_NWEBSPAWN_PROCESS);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        HILOG_ERROR("PreStartNWebSpawnProcess failed, result: %{public}d", ret);
        return ret;
    }
    return 0;
}

int AppMgrProxy::StartRenderProcess(const std::string &renderParam,
                                    int32_t ipcFd, int32_t sharedFd,
                                    int32_t crashFd, pid_t &renderPid)
{
    if (renderParam.empty() || ipcFd <= 0 || sharedFd <= 0 || crashFd <= 0) {
        HILOG_ERROR("Invalid params, renderParam:%{private}s, ipcFd:%{public}d, "
                    "sharedFd:%{public}d, crashFd:%{public}d",
                    renderParam.c_str(), ipcFd, sharedFd, crashFd);
        return -1;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(renderParam)) {
        HILOG_ERROR("want paramSize failed.");
        return -1;
    }

    if (!data.WriteFileDescriptor(ipcFd) || !data.WriteFileDescriptor(sharedFd) ||
        !data.WriteFileDescriptor(crashFd)) {
        HILOG_ERROR("want fd failed, ipcFd:%{public}d, sharedFd:%{public}d, "
                    "crashFd:%{public}d",
                    ipcFd, sharedFd, crashFd);
        return -1;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::START_RENDER_PROCESS);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    auto result = reply.ReadInt32();
    renderPid = reply.ReadInt32();
    if (result != 0) {
        HILOG_WARN("StartRenderProcess failed, result: %{public}d", result);
        return result;
    }
    return 0;
}

void AppMgrProxy::AttachRenderProcess(const sptr<IRemoteObject> &renderScheduler)
{
    if (!renderScheduler) {
        HILOG_ERROR("renderScheduler is null");
        return;
    }

    HILOG_DEBUG("AttachRenderProcess start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(renderScheduler)) {
        HILOG_ERROR("renderScheduler write failed.");
        return;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::ATTACH_RENDER_PROCESS);
    if (SendTransactCmd(code, data, reply, option) != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd ATTACH_RENDER_PROCESS failed");
        return;
    }
}

int AppMgrProxy::GetRenderProcessTerminationStatus(pid_t renderPid, int &status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(renderPid)) {
        HILOG_ERROR("write renderPid failed.");
        return -1;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::GET_RENDER_PROCESS_TERMINATION_STATUS);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        HILOG_ERROR("GetRenderProcessTerminationStatus failed, result: %{public}d", result);
        return result;
    }
    status = reply.ReadInt32();
    return 0;
}

int32_t AppMgrProxy::UpdateConfiguration(const Configuration &config)
{
    HILOG_INFO("AppMgrProxy UpdateConfiguration");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&config)) {
        HILOG_ERROR("parcel config failed");
        return ERR_INVALID_DATA;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetConfiguration(Configuration &config)
{
    HILOG_INFO("AppMgrProxy GetConfiguration");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("parcel data failed");
        return ERR_INVALID_DATA;
    }
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::GET_CONFIGURATION);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    std::unique_ptr<Configuration> info(reply.ReadParcelable<Configuration>());
    if (!info) {
        HILOG_ERROR("read configuration failed.");
        return ERR_UNKNOWN_OBJECT;
    }
    config = *info;
    return reply.ReadInt32();
}

int32_t AppMgrProxy::RegisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    if (!observer) {
        HILOG_ERROR("observer null");
        return ERR_INVALID_VALUE;
    }
    HILOG_DEBUG("RegisterConfigurationObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::REGISTER_CONFIGURATION_OBSERVER);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AppMgrProxy::UnregisterConfigurationObserver(const sptr<IConfigurationObserver> &observer)
{
    HILOG_DEBUG("UnregisterConfigurationObserver start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteRemoteObject(observer->AsObject())) {
        HILOG_ERROR("observer write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::UNREGISTER_CONFIGURATION_OBSERVER);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}

#ifdef ABILITY_COMMAND_FOR_TEST
int AppMgrProxy::BlockAppService()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::BLOCK_APP_SERVICE);
    int32_t ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    return reply.ReadInt32();
}
#endif

bool AppMgrProxy::GetAppRunningStateByBundleName(const std::string &bundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundle name failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::GET_APP_RUNNING_STATE);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("SendTransactCmd failed");
        return false;
    }

    return reply.ReadBool();
}

int32_t AppMgrProxy::NotifyLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("NotifyLoadRepairPatch, function called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("NotifyLoadRepairPatch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("NotifyLoadRepairPatch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("Write callback failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_LOAD_REPAIR_PATCH);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyHotReloadPage(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("Write callback failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_HOT_RELOAD_PAGE);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}

#ifdef BGTASKMGR_CONTINUOUS_TASK_ENABLE
int32_t AppMgrProxy::SetContinuousTaskProcess(int32_t pid, bool isContinuousTask)
{
    HILOG_DEBUG("SetContinuousTaskProcess start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("uid write failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteBool(isContinuousTask)) {
        HILOG_ERROR("isContinuousTask write failed.");
        return ERR_INVALID_DATA;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::SET_CONTINUOUSTASK_PROCESS);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}
#endif

int32_t AppMgrProxy::NotifyUnLoadRepairPatch(const std::string &bundleName, const sptr<IQuickFixCallback> &callback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Notify unload patch, Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Notify unload patch, Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (callback == nullptr || !data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("Write callback failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_UNLOAD_REPAIR_PATCH);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != 0) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}

bool AppMgrProxy::IsSharedBundleRunning(const std::string &bundleName, uint32_t versionCode)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }
    if (!data.WriteString(bundleName) || !data.WriteUint32(versionCode)) {
        HILOG_ERROR("Write bundle name or version code failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::IS_SHARED_BUNDLE_RUNNING);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return false;
    }

    return reply.ReadBool();
}

int32_t AppMgrProxy::StartNativeProcessForDebugger(const AAFwk::Want &want)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::START_NATIVE_PROCESS_FOR_DEBUGGER);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("pid write failed.");
        return ERR_INVALID_DATA;
    }

    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::GET_BUNDLE_NAME_BY_PID);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    bundleName = reply.ReadString();
    uid = reply.ReadInt32();
    return ERR_NONE;
}

int32_t AppMgrProxy::NotifyAppFault(const FaultData &faultData)
{
    HILOG_DEBUG("called.");
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteParcelable(&faultData)) {
        HILOG_ERROR("Write FaultData error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::NotifyAppFaultBySA(const AppFaultDataBySA &faultData)
{
    HILOG_DEBUG("called.");
    MessageParcel data;

    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteParcelable(&faultData)) {
        HILOG_ERROR("Write FaultDataBySA error.");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(AppMgrInterfaceCode::NOTIFY_APP_FAULT_BY_SA);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    return reply.ReadInt32();
}

int32_t AppMgrProxy::GetProcessMemoryByPid(const int32_t pid, int32_t &memorySize)
{
    HILOG_DEBUG("GetProcessMemoryByPid start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("write pid failed.");
        return ERR_INVALID_DATA;
    }

    uint32_t code = static_cast<uint32_t>(IAppMgr::Message::GET_PROCESS_MEMORY_BY_PID);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }
    memorySize = reply.ReadInt32();
    auto result = reply.ReadInt32();
    return result;
}

int32_t AppMgrProxy::GetRunningProcessInformation(
    const std::string &bundleName, int32_t userId, std::vector<RunningProcessInfo> &info)
{
    HILOG_DEBUG("GetRunningProcessInformation start");
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("write bundleName failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("write userId failed.");
        return ERR_INVALID_DATA;
    }
    MessageOption option(MessageOption::TF_SYNC);
    uint32_t code = static_cast<uint32_t>(IAppMgr::Message::GET_PIDS_BY_BUNDLENAME);
    auto ret = SendTransactCmd(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendTransactCmd failed");
        return ret;
    }

    auto error = GetParcelableInfos<RunningProcessInfo>(reply, info);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetParcelableInfos fail, error: %{public}d", error);
        return error;
    }
    int result = reply.ReadInt32();
    return result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
