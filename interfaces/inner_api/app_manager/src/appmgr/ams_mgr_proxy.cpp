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

#include "ams_mgr_proxy.h"
#include "ipc_types.h"
#include "iremote_object.h"
#include "string_ex.h"

#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t MAX_APP_DEBUG_COUNT = 100;
}
AmsMgrProxy::AmsMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAmsMgr>(impl)
{}

bool AmsMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AmsMgrProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}
namespace {
bool WriteTokenObject(MessageParcel &data, sptr<IRemoteObject> token)
{
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            HILOG_ERROR("Failed to write flag or token");
            return false;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return false;
        }
    }
    return true;
}
}

void AmsMgrProxy::LoadAbility(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::shared_ptr<ApplicationInfo> &appInfo,
    const std::shared_ptr<AAFwk::Want> &want, int32_t abilityRecordId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    if (!abilityInfo || !appInfo) {
        TAG_LOGE(AAFwkTag::APPMGR, "param error");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!WriteTokenObject(data, token)) {
        return;
    }
    if (!WriteTokenObject(data, preToken)) {
        return;
    }

    data.WriteParcelable(abilityInfo.get());
    data.WriteParcelable(appInfo.get());
    if (!data.WriteParcelable(want.get())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data want failed.");
        return;
    }
    if (!data.WriteInt32(abilityRecordId)) {
        HILOG_ERROR("Write data abilityRecordId failed.");
        return;
    }

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteBool(clearMissionFlag)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write clearMissionFlag");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(state));
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    TAG_LOGD(AAFwkTag::APPMGR, "UpdateExtensionState begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    data.WriteInt32(static_cast<int32_t>(state));
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin");
    if (!callback) {
        TAG_LOGE(AAFwkTag::APPMGR, "callback is nullptr");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (callback->AsObject()) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callback->AsObject())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and callback");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility, const int32_t perceptibility, const int32_t connectionState)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }

    if (preToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(preToken.GetRefPtr())) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag and preToken");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to write flag");
            return;
        }
    }

    data.WriteInt32(static_cast<int32_t>(visibility));
    data.WriteInt32(static_cast<int32_t>(perceptibility));
    data.WriteInt32(static_cast<int32_t>(connectionState));
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ABILITY_BEHAVIOR_ANALYSIS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::KillProcessesByUserId(int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "ending");
}

void AmsMgrProxy::KillProcessesByPids(std::vector<int32_t> &pids)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteUint32(pids.size())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write size failed.");
        return;
    }
    for (const auto &pid: pids) {
        if (!data.WriteInt32(pid)) {
            TAG_LOGE(AAFwkTag::APPMGR, "Write pid failed.");
            return;
        }
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_PIDS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::AttachPidToParent(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callerToken)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    if (!data.WriteRemoteObject(callerToken)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write callerToken");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ATTACH_PID_TO_PARENT),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

int32_t AmsMgrProxy::KillProcessWithAccount(const std::string &bundleName, const int accountId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(accountId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "end");

    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplication(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteString failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "uid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_APPLICATION_INFO_INSTALLED),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write uid.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplicationSelf()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_SELF), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is wrong, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "beginning");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

void AmsMgrProxy::GetRunningProcessInfoByToken(
    const sptr<IRemoteObject> &token, AppExecFwk::RunningProcessInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteRemoteObject(token.GetRefPtr())) {
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }

    std::unique_ptr<AppExecFwk::RunningProcessInfo> processInfo(reply.ReadParcelable<AppExecFwk::RunningProcessInfo>());
    if (processInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "recv process info faild");
        return;
    }

    info = *processInfo;
}

void AmsMgrProxy::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed.");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_PID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }

    std::unique_ptr<AppExecFwk::RunningProcessInfo> processInfo(reply.ReadParcelable<AppExecFwk::RunningProcessInfo>());
    if (processInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "recv process info failded");
        return;
    }

    info = *processInfo;
}

void AmsMgrProxy::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    TAG_LOGD(AAFwkTag::APPMGR, "calling");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        TAG_LOGE(AAFwkTag::APPMGR, "parcel WriteInt32 failed");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::SET_ABILITY_FOREGROUNDING_FLAG), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AmsMgrProxy::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)

{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AmsMgrProxy::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)

{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote is nullptr.");
        return;
    }
    auto ret = remote->SendRequest(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

void AmsMgrProxy::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Register multi instances response by proxy.");
    if (!response) {
        TAG_LOGE(AAFwkTag::APPMGR, "response is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(response->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write remote object.");
        return;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

int AmsMgrProxy::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "token write error.");
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(pid);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_APPLICATION_INFO_BY_PROCESS_ID), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "send request fail");
        return ret;
    }
    auto result = reply.ReadInt32();
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "reply result false");
        return result;
    }
    std::unique_ptr<AppExecFwk::ApplicationInfo> info(reply.ReadParcelable<AppExecFwk::ApplicationInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::APPMGR, "readParcelableInfo failed");
        return ERR_NAME_NOT_FOUND;
    }
    application = *info;
    debug = reply.ReadBool();
    TAG_LOGD(AAFwkTag::APPMGR, "get parcelable info success");
    return NO_ERROR;
}

void AmsMgrProxy::SetCurrentUserId(const int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write userId");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_CURRENT_USER_ID),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::APPMGR, "end");
}

int32_t AmsMgrProxy::GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write pid");
        return ERR_INVALID_DATA;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID),
            data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
    bundleName = reply.ReadString();
    uid = reply.ReadInt32();
    return NO_ERROR;
}

int32_t AmsMgrProxy::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write listener failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write listener failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::AttachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::DetachAppDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::SetAppWaitingDebug(const std::string &bundleName, bool isPersist)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundle name failed.");
        return ERR_INVALID_DATA;
    }

    if (!data.WriteBool(isPersist)) {
        HILOG_ERROR("Write persist flag failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_APP_WAITING_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, error code is %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::CancelAppWaitingDebug()
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::CANCEL_APP_WAITING_DEBUG), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, error code is %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::GetWaitingDebugApp(std::vector<std::string> &debugInfoList)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::GET_WAITING_DEBUG_APP), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, error code is %{public}d.", ret);
        return ret;
    }

    auto resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        HILOG_ERROR("Reply error is %{public}d.", resultCode);
        return resultCode;
    }

    auto infoSize = reply.ReadInt32();
    if (infoSize > MAX_APP_DEBUG_COUNT) {
        HILOG_ERROR("Max app debug count is %{public}d.", infoSize);
        return ERR_INVALID_DATA;
    }

    if (!reply.ReadStringVector(&debugInfoList)) {
        HILOG_ERROR("Fail to read string vector debug info list.");
        return ERR_INVALID_DATA;
    }

    return NO_ERROR;
}

bool AmsMgrProxy::IsWaitingDebugApp(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundle name failed.");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_WAITING_DEBUG_APP), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, error code is %{public}d.", ret);
        return false;
    }
    return reply.ReadBool();
}

void AmsMgrProxy::ClearNonPersistWaitingDebugFlag()
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::CLEAR_NON_PERSIST_WAITING_DEBUG_FLAG), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("Send request is failed, error code is %{public}d.", ret);
    }
}

int32_t AmsMgrProxy::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (response == nullptr || !data.WriteRemoteObject(response->AsObject())) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write remote object.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

bool AmsMgrProxy::IsAttachDebug(const std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return false;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write bundle name fail.");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_ATTACH_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d", ret);
        return false;
    }
    return reply.ReadBool();
}

void AmsMgrProxy::SetAppAssertionPauseState(int32_t pid, bool flag)
{
    TAG_LOGD(AAFwkTag::APPMGR, "Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write pid fail.");
        return;
    }

    if (!data.WriteBool(flag)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Write flag fail.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_APP_ASSERT_PAUSE_STATE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "Send request failed, err: %{public}d", ret);
    }
}

void AmsMgrProxy::ClearProcessByToken(sptr<IRemoteObject> token)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write interface token.");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to write token");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::CLEAR_PROCESS_BY_TOKEN), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGW(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d", ret);
    }
}

bool AmsMgrProxy::IsMemorySizeSufficent()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return true;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_MEMORY_SIZE_SUFFICIENT), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, error code is %{public}d.", ret);
        return true;
    }
    return reply.ReadBool();
}

int32_t AmsMgrProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest error. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return ret;
}
} // namespace AppExecFwk
} // namespace OHOS
