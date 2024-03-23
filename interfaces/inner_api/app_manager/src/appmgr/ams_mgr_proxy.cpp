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
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AmsMgrProxy::AmsMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAmsMgr>(impl)
{}

bool AmsMgrProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AmsMgrProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed");
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
    HILOG_DEBUG("start");
    if (!abilityInfo || !appInfo) {
        HILOG_ERROR("param error");
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
        HILOG_ERROR("Write data want failed.");
        return;
    }
    if (!data.WriteInt32(abilityRecordId)) {
        HILOG_ERROR("Write data abilityRecordId failed.");
        return;
    }

    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::LOAD_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    HILOG_DEBUG("start");
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
    if (!data.WriteBool(clearMissionFlag)) {
        HILOG_ERROR("Failed to write clearMissionFlag");
        return;
    }
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state)
{
    HILOG_DEBUG("start");
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
    data.WriteInt32(static_cast<int32_t>(state));
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_ABILITY_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::UpdateExtensionState(const sptr<IRemoteObject> &token, const ExtensionState state)
{
    HILOG_DEBUG("UpdateExtensionState begin");
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
    data.WriteInt32(static_cast<int32_t>(state));
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_EXTENSION_STATE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::RegisterAppStateCallback(const sptr<IAppStateCallback> &callback)
{
    HILOG_DEBUG("begin");
    if (!callback) {
        HILOG_ERROR("callback is nullptr");
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
            HILOG_ERROR("Failed to write flag and callback");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return;
        }
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_STATE_CALLBACK), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::AbilityBehaviorAnalysis(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &preToken,
    const int32_t visibility, const int32_t perceptibility, const int32_t connectionState)
{
    HILOG_DEBUG("start");
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

    if (preToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(preToken.GetRefPtr())) {
            HILOG_ERROR("Failed to write flag and preToken");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return;
        }
    }

    data.WriteInt32(static_cast<int32_t>(visibility));
    data.WriteInt32(static_cast<int32_t>(perceptibility));
    data.WriteInt32(static_cast<int32_t>(connectionState));
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ABILITY_BEHAVIOR_ANALYSIS), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::KillProcessByAbilityToken(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("start");
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
    int32_t ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::KillProcessesByUserId(int32_t userId)
{
    HILOG_DEBUG("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("parcel WriteInt32 failed");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("ending");
}

int32_t AmsMgrProxy::KillProcessWithAccount(const std::string &bundleName, const int accountId)
{
    HILOG_DEBUG("start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }

    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteInt32(accountId)) {
        HILOG_ERROR("parcel WriteInt32 failed");
        return ERR_FLATTEN_OBJECT;
    }

    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }

    HILOG_DEBUG("end");

    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplication(const std::string &bundleName)
{
    HILOG_DEBUG("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("parcel WriteString failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid)
{
    HILOG_DEBUG("start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        HILOG_ERROR("uid write failed.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UPDATE_APPLICATION_INFO_INSTALLED),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::KillApplicationByUid(const std::string &bundleName, const int uid)
{
    HILOG_DEBUG("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("parcel WriteString failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(uid)) {
        HILOG_ERROR("Failed to write uid.");
        return ERR_FLATTEN_OBJECT;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_BYUID), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
        HILOG_ERROR("SendRequest is wrong, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

void AmsMgrProxy::AbilityAttachTimeOut(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("beginning");
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
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ABILITY_ATTACH_TIMEOUT), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
}

void AmsMgrProxy::PrepareTerminate(const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("start");
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
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::PREPARE_TERMINATE_ABILITY),
            data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
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
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return;
    }

    std::unique_ptr<AppExecFwk::RunningProcessInfo> processInfo(reply.ReadParcelable<AppExecFwk::RunningProcessInfo>());
    if (processInfo == nullptr) {
        HILOG_ERROR("recv process info faild");
        return;
    }

    info = *processInfo;
}

void AmsMgrProxy::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info)
{
    HILOG_DEBUG("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        HILOG_ERROR("parcel WriteInt32 failed.");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_PID), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
        return;
    }

    std::unique_ptr<AppExecFwk::RunningProcessInfo> processInfo(reply.ReadParcelable<AppExecFwk::RunningProcessInfo>());
    if (processInfo == nullptr) {
        HILOG_ERROR("recv process info failded");
        return;
    }

    info = *processInfo;
}

void AmsMgrProxy::SetAbilityForegroundingFlagToAppRecord(const pid_t pid)
{
    HILOG_DEBUG("calling");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(pid))) {
        HILOG_ERROR("parcel WriteInt32 failed");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::SET_ABILITY_FOREGROUNDING_FLAG), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
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
        HILOG_ERROR("Write data failed.");
        return;
    }

    auto ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_ABILITY), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AmsMgrProxy::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo)

{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write data failed.");
        return;
    }

    if (!data.WriteParcelable(&want) || !data.WriteParcelable(&abilityInfo)) {
        HILOG_ERROR("Write data failed.");
        return;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return;
    }
    auto ret = remote->SendRequest(
        static_cast<uint32_t>(IAmsMgr::Message::START_SPECIFIED_PROCESS), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

void AmsMgrProxy::RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response)
{
    HILOG_DEBUG("Register multi instances response by proxy.");
    if (!response) {
        HILOG_ERROR("response is null");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(response->AsObject())) {
        HILOG_ERROR("Failed to write remote object.");
        return;
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_START_SPECIFIED_ABILITY_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

int AmsMgrProxy::GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("token write error.");
        return ERR_FLATTEN_OBJECT;
    }
    data.WriteInt32(pid);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::GET_APPLICATION_INFO_BY_PROCESS_ID), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("send request fail");
        return ret;
    }
    auto result = reply.ReadInt32();
    if (result != NO_ERROR) {
        HILOG_ERROR("reply result false");
        return result;
    }
    std::unique_ptr<AppExecFwk::ApplicationInfo> info(reply.ReadParcelable<AppExecFwk::ApplicationInfo>());
    if (!info) {
        HILOG_ERROR("readParcelableInfo failed");
        return ERR_NAME_NOT_FOUND;
    }
    application = *info;
    debug = reply.ReadBool();
    HILOG_DEBUG("get parcelable info success");
    return NO_ERROR;
}

void AmsMgrProxy::SetCurrentUserId(const int32_t userId)
{
    HILOG_DEBUG("start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("Failed to write userId");
        return;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_CURRENT_USER_ID),
            data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    HILOG_DEBUG("end");
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
        HILOG_ERROR("Failed to write pid");
        return ERR_INVALID_DATA;
    }
    int32_t ret =
        SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::Get_BUNDLE_NAME_BY_PID),
            data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
    bundleName = reply.ReadString();
    uid = reply.ReadInt32();
    return NO_ERROR;
}

int32_t AmsMgrProxy::RegisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("Write listener failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::REGISTER_APP_DEBUG_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (listener == nullptr || !data.WriteRemoteObject(listener->AsObject())) {
        HILOG_ERROR("Write listener failed.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::UNREGISTER_APP_DEBUG_LISTENER),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::AttachAppDebug(const std::string &bundleName)
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

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::ATTACH_APP_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::DetachAppDebug(const std::string &bundleName)
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

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::DETACH_APP_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t AmsMgrProxy::RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_DATA;
    }

    if (response == nullptr || !data.WriteRemoteObject(response->AsObject())) {
        HILOG_ERROR("Failed to write remote object.");
        return ERR_INVALID_DATA;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IAmsMgr::Message::REGISTER_ABILITY_DEBUG_RESPONSE), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest is failed, error code: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

bool AmsMgrProxy::IsAttachDebug(const std::string &bundleName)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }

    if (bundleName.empty() || !data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundle name fail.");
        return false;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::IS_ATTACH_DEBUG),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", ret);
        return false;
    }
    return reply.ReadBool();
}

void AmsMgrProxy::SetAppAssertionPauseState(int32_t pid, bool flag)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("Write pid fail.");
        return;
    }

    if (!data.WriteBool(flag)) {
        HILOG_ERROR("Write flag fail.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::SET_APP_ASSERT_PAUSE_STATE),
        data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request failed, err: %{public}d", ret);
    }
}

void AmsMgrProxy::ClearProcessByToken(sptr<IRemoteObject> token)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        HILOG_ERROR("Failed to write interface token.");
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        HILOG_ERROR("Failed to write token");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    auto ret = SendTransactCmd(static_cast<uint32_t>(IAmsMgr::Message::CLEAR_PROCESS_BY_TOKEN), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_WARN("SendRequest is failed, error code: %{public}d", ret);
    }
}

int32_t AmsMgrProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest error. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return ret;
}
} // namespace AppExecFwk
} // namespace OHOS
