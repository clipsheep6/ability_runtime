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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::APP_ATTACH_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_FOREGROUNDED), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_BACKGROUNDED), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_APPLICATION_TERMINATED), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_PERMISSION_DENIED;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::APP_CHECK_PERMISSION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::APP_ABILITY_CLEANED), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

sptr<IAmsMgr> AppMgrProxy::GetAmsMgr()
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_MGR_INSTANCE, data, reply)) {
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::APP_CLEAR_UP_APPLICATION_DATA), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_ALL_RUNNING_PROCESSES, data, reply)) {
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

int32_t AppMgrProxy::GetProcessRunningInfosByUserId(std::vector<RunningProcessInfo> &info, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(userId);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_RUNNING_PROCESSES_BY_USER_ID, data, reply)) {
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

int32_t AppMgrProxy::GetProcessRunningInformation(RunningProcessInfo &info)
{
    MessageParcel data;
    MessageParcel reply;

    if (!WriteInterfaceToken(data)) {
        return ERR_FLATTEN_OBJECT;
    }
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_PROCESS_RUNNING_INFORMATION, data, reply)) {
        return ERR_NULL_OBJECT;
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }
    int32_t ret =
        remote->SendRequest(
            static_cast<uint32_t>(IAppMgr::Message::APP_NOTIFY_MEMORY_LEVEL), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    int result = reply.ReadInt32();
    return result;
}

bool AppMgrProxy::SendTransactCmd(IAppMgr::Message code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);
    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        HILOG_ERROR("fail to send transact cmd %{public}d due to remote object", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}

void AppMgrProxy::AddAbilityStageDone(const int32_t recordId)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId)) {
        HILOG_ERROR("want write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::APP_ADD_ABILITY_STAGE_INFO_DONE, data, reply)) {
        HILOG_ERROR("SendTransactCmd failed");
        return;
    }
    return;
}

void AppMgrProxy::StartupResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    MessageParcel data;
    MessageParcel reply;
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

    if (!SendTransactCmd(IAppMgr::Message::STARTUP_RESIDENT_PROCESS, data, reply)) {
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

    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::REGISTER_APPLICATION_STATE_OBSERVER),
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
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

    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::UNREGISTER_APPLICATION_STATE_OBSERVER),
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
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
    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::GET_FOREGROUND_APPLICATIONS),
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("GetForegroundApplications fail, error: %{public}d", error);
        return error;
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
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::START_USER_TEST_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::FINISH_USER_TEST), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AppMgrProxy::ScheduleAcceptWantDone(const int32_t recordId, const AAFwk::Want &want, const std::string &flag)
{
    MessageParcel data;
    MessageParcel reply;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("WriteInterfaceToken failed");
        return;
    }

    if (!data.WriteInt32(recordId) || !data.WriteParcelable(&want) || !data.WriteString(flag)) {
        HILOG_ERROR("want write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::SCHEDULE_ACCEPT_WANT_DONE, data, reply)) {
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
    if (!SendTransactCmd(IAppMgr::Message::APP_GET_ABILITY_RECORDS_BY_PROCESS_ID, data, reply)) {
        return ERR_NULL_OBJECT;
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

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::PRE_START_NWEBSPAWN_PROCESS),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("PreStartNWebSpawnProcess failed, result: %{public}d", ret);
        return ret;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        HILOG_WARN("PreStartNWebSpawnProcess failed, result: %{public}d", ret);
        return ret;
    }
    return 0;
}

int AppMgrProxy::StartRenderProcess(const std::string &renderParam, int32_t ipcFd,
    int32_t sharedFd, pid_t &renderPid)
{
    if (renderParam.empty() || ipcFd <= 0 || sharedFd <= 0) {
        HILOG_ERROR("Invalid params, renderParam:%{private}s, ipcFd:%{public}d, sharedFd:%{public}d",
            renderParam.c_str(), ipcFd, sharedFd);
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

    if (!data.WriteFileDescriptor(ipcFd) || !data.WriteFileDescriptor(sharedFd)) {
        HILOG_ERROR("want fd failed, ipcFd:%{public}d, sharedFd:%{public}d", ipcFd, sharedFd);
        return -1;
    }

    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::START_RENDER_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("StartRenderProcess SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    auto result = reply.ReadInt32();
    renderPid = reply.ReadInt32();
    if (result != 0) {
        HILOG_WARN("StartRenderProcess failed, result: %{public}d", ret);
        return ret;
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
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(renderScheduler)) {
        HILOG_ERROR("renderScheduler write failed.");
        return;
    }

    if (!SendTransactCmd(IAppMgr::Message::ATTACH_RENDER_PROCESS, data, reply)) {
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

    int32_t ret = Remote()->SendRequest(
        static_cast<uint32_t>(IAppMgr::Message::GET_RENDER_PROCESS_TERMINATION_STATUS), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("GetRenderProcessTerminationStatus SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    auto result = reply.ReadInt32();
    if (result != 0) {
        HILOG_WARN("GetRenderProcessTerminationStatus failed, result: %{public}d", result);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_INVALID_DATA;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::UPDATE_CONFIGURATION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_INVALID_DATA;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::GET_CONFIGURATION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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

    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::REGISTER_CONFIGURATION_OBSERVER),
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
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

    auto error = Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::UNREGISTER_CONFIGURATION_OBSERVER),
        data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
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

    int32_t ret =
        Remote()->SendRequest(static_cast<uint32_t>(IAppMgr::Message::BLOCK_APP_SERVICE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::GET_APP_RUNNING_STATE),
        data, reply, option);
    if (ret != 0) {
        HILOG_WARN("Send request failed with error code %{public}d.", ret);
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

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("NotifyLoadRepairPatch, Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::NOTIFY_LOAD_REPAIR_PATCH),
        data, reply, option);
    if (ret != 0) {
        HILOG_WARN("NotifyLoadRepairPatch, Send request failed with error code %{public}d.", ret);
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

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::NOTIFY_HOT_RELOAD_PAGE),
        data, reply, option);
    if (ret != 0) {
        HILOG_WARN("Send request failed with error code %{public}d.", ret);
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

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    auto ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::SET_CONTINUOUSTASK_PROCESS),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("Send request failed with error code %{public}d.", ret);
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

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Notify unload patch, Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::NOTIFY_UNLOAD_REPAIR_PATCH),
        data, reply, option);
    if (ret != 0) {
        HILOG_WARN("Notify unload patch, Send request failed with error code %{public}d.", ret);
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
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = remote->SendRequest(static_cast<uint32_t>(IAppMgr::Message::IS_SHARED_BUNDLE_RUNNING),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return false;
    }

    return reply.ReadBool();
}
}  // namespace AppExecFwk
}  // namespace OHOS
