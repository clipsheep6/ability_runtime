/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mission_list_delegator_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t CYCLE_LIMIT = 1000;

template <typename T>
int32_t GetParcelableInfos(MessageParcel &reply, std::vector<T> &parcelableInfos)
{
    int32_t infoSize = reply.ReadInt32();
    if (infoSize > CYCLE_LIMIT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "infoSize is too large");
        return ERR_INVALID_VALUE;
    }

    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(reply.ReadParcelable<T>());
        if (!info) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read Parcelable infos failed.");
            return ERR_INVALID_VALUE;
        }
        parcelableInfos.emplace_back(*info);
    }
    return NO_ERROR;
}
} // namespace
bool MissionListDelegatorProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(MissionListDelegatorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return false;
    }
    return true;
}

int32_t MissionListDelegatorProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote object is nullptr.");
        return -1;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendTransactCmd failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return 0;
}

int32_t MissionListDelegatorProxy::LockMissionForCleanup(int32_t missionId)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "lock mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }

    error = SendTransactCmd(LOCK_MISSION_FOR_CLEANUP, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "lock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::UnlockMissionForCleanup(int32_t missionId)
{
    int32_t error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unlock mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }

    error = SendTransactCmd(UNLOCK_MISSION_FOR_CLEANUP, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unlock mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::RegisterMissionListener(sptr<IMissionListener> listener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!listener) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register mission listener, listener is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write mission listener failed when register mission listener.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendTransactCmd(REGISTER_MISSION_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::UnRegisterMissionListener(sptr<IMissionListener> listener)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!listener) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "unregister mission listener, listener is nullptr");
        return ERR_INVALID_VALUE;
    }

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write mission listener failed when unregister mission listener.");
        return ERR_INVALID_VALUE;
    }

    int32_t error = SendTransactCmd(UNREGISTER_MISSION_LISTENER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write deviceId failed when GetMissionInfos.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(numMax)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfos numMax write failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t error = SendTransactCmd(GET_MISSION_INFOS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfos Send request error: %{public}d", error);
        return error;
    }
    error = GetParcelableInfos<MissionInfo>(reply, missionInfos);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfos error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString16(Str8ToStr16(deviceId))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write deviceId failed when GetMissionInfo.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfo write missionId failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t error = SendTransactCmd(GET_MISSION_INFO_BY_ID, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfo Send request error: %{public}d", error);
        return error;
    }

    std::unique_ptr<MissionInfo> info(reply.ReadParcelable<MissionInfo>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read missioninfo failed.");
        return ERR_UNKNOWN_OBJECT;
    }
    missionInfo = *info;
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::CleanMission(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "clean mission by id , WriteInt32 fail.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendTransactCmd(CLEAN_MISSION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "clean mission by id , error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::CleanAllMissions()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    auto error = SendTransactCmd(CLEAN_ALL_MISSIONS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "lock mission by id ,SendTransactCmd error: %d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::RegisterSnapshotHandler(sptr<ISnapshotHandler> handler)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(handler->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: handler write failed.");
        return INNER_ERR;
    }
    auto error = SendTransactCmd(REGISTER_SNAPSHOT_HANDLER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
#ifdef SUPPORT_SCREEN
int32_t MissionListDelegatorProxy::RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler)
{
    if (!handler) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: handler is nullptr.", __func__);
        return INNER_ERR;
    }
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: write interface token failed.", __func__);
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(handler->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: handler write failed.", __func__);
        return INNER_ERR;
    }
    MessageOption option;
    MessageParcel reply;
    auto error = SendTransactCmd(REGISTER_WMS_HANDLER, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s: send request error: %{public}d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

void MissionListDelegatorProxy::UpdateMissionSnapShot(sptr<IRemoteObject> token,
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write token failed.");
        return;
    }
    if (!data.WriteParcelable(pixelMap.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write pixelMap failed.");
        return;
    }
    auto error = SendTransactCmd(UPDATE_MISSION_SNAPSHOT_FROM_WMS, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
    }
}

int32_t MissionListDelegatorProxy::SetMissionLabel(sptr<IRemoteObject> token, const std::string &label)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel write token failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString16(Str8ToStr16(label))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel write label failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendTransactCmd(SET_MISSION_LABEL, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::SetMissionIcon(sptr<IRemoteObject> token,
    std::shared_ptr<Media::PixelMap> icon)
{
    if (!token || !icon) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon abilitytoken or icon is invalid.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon write token failed.");
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteParcelable(icon.get())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon write icon failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendTransactCmd(SET_MISSION_ICON, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
#endif
int32_t MissionListDelegatorProxy::MoveMissionsToForeground(const std::vector<int32_t>& missionIds,
    int32_t topMissionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "mission id write failed.");
        return INNER_ERR;
    }

    if (!data.WriteInt32(topMissionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "top mission id write failed.");
        return INNER_ERR;
    }

    auto error = SendTransactCmd(MOVE_MISSIONS_TO_FOREGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query front missionInfo failed: send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteInt32Vector(missionIds)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "mission id write failed.");
        return INNER_ERR;
    }

    auto error = SendTransactCmd(MOVE_MISSIONS_TO_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query front missionInfo failed: send request error: %{public}d", error);
        return error;
    }

    if (!reply.ReadInt32Vector(&result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read result failed");
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::GetTopAbility(sptr<IRemoteObject> &token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    auto error = SendTransactCmd(GET_TOP_ABILITY_TOKEN, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }

    token = sptr<IRemoteObject>(reply.ReadRemoteObject());
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read IRemoteObject failed.");
        return ERR_UNKNOWN_OBJECT;
    }

    return reply.ReadInt32();
}

AppExecFwk::ElementName MissionListDelegatorProxy::GetTopAbility(bool isNeedLocalDeviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return {};
    }
    if (!data.WriteBool(isNeedLocalDeviceId)) {
        return {};
    }

    int error = SendTransactCmd(GET_TOP_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return {};
    }
    std::unique_ptr<AppExecFwk::ElementName> name(reply.ReadParcelable<AppExecFwk::ElementName>());
    if (!name) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read info failed.");
        return {};
    }
    AppExecFwk::ElementName result = *name;
    return result;
}

int32_t MissionListDelegatorProxy::GetMissionIdByToken(sptr<IRemoteObject> token)
{
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token is nullptr.");
        return -1;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data interface token failed.");
        return -1;
    }

    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data write failed.");
        return -1;
    }

    auto error = SendTransactCmd(GET_MISSION_ID_BY_ABILITY_TOKEN,
        data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return -1;
    }

    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::MoveAbilityToBackground(sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (token) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(token)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and token write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write failed.");
            return INNER_ERR;
        }
    }
    auto error = SendTransactCmd(MOVE_ABILITY_TO_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d.", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "deviceId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteBool(isLowResolution)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "isLowResolution write failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendTransactCmd(GET_MISSION_SNAPSHOT_INFO, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }
    std::unique_ptr<MissionSnapshot> info(reply.ReadParcelable<MissionSnapshot>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelableInfo failed.");
        auto errorCode = reply.ReadInt32();
        return errorCode ? errorCode : ERR_UNKNOWN_OBJECT;
    }
    snapshot = *info;
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::DelegatorDoAbilityForeground(sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendTransactCmd(DELEGATOR_DO_ABILITY_FOREGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::DelegatorDoAbilityBackground(sptr<IRemoteObject> token)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data write failed.");
        return ERR_INVALID_VALUE;
    }

    auto error = SendTransactCmd(DELEGATOR_DO_ABILITY_BACKGROUND, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }

    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::SetMissionContinueState(sptr<IRemoteObject> token, ContinueState state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState write token failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(static_cast<int32_t>(state))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState write state failed.");
        return ERR_INVALID_VALUE;
    }
    auto error = SendTransactCmd(SET_MISSION_CONTINUE_STATE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int32_t MissionListDelegatorProxy::IsValidMissionIds(
    const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed.");
        return INNER_ERR;
    }

    constexpr int32_t MAX_COUNT = 20;
    int32_t num = static_cast<int32_t>(missionIds.size() > MAX_COUNT ? MAX_COUNT : missionIds.size());
    data.WriteInt32(num);
    for (auto i = 0; i < num; ++i) {
        data.WriteInt32(missionIds.at(i));
    }

    auto error = SendTransactCmd(QUERY_MISSION_VALID, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", error);
        return error;
    }

    auto resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request reply error: %{public}d", resultCode);
        return resultCode;
    }

    auto infoSize = reply.ReadInt32();
    for (auto i = 0; i < infoSize && i < MAX_COUNT; ++i) {
        std::unique_ptr<MissionValidResult> info(reply.ReadParcelable<MissionValidResult>());
        if (!info) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read Parcelable result infos failed.");
            return INNER_ERR;
        }
        results.emplace_back(*info);
    }

    return resultCode;
}
} // AAFwk
} // OHOS