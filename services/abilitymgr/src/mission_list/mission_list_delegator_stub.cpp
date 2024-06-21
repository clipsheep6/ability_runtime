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

#include "mission_list_delegator_stub.h"

#include "hilog_tag_wrapper.h"
#include "mission_list_wrapper.h"

namespace OHOS {
namespace AAFwk {
int32_t MissionListDelegatorStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = MissionListDelegatorStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case LOCK_MISSION_FOR_CLEANUP: return LockMissionForCleanupInner(data, reply);
        case UNLOCK_MISSION_FOR_CLEANUP: return UnlockMissionForCleanupInner(data, reply);
        case REGISTER_MISSION_LISTENER: return RegisterMissionListenerInner(data, reply);
        case UNREGISTER_MISSION_LISTENER: return UnRegisterMissionListenerInner(data, reply);
        case GET_MISSION_INFOS: return GetMissionInfosInner(data, reply);
        case GET_MISSION_INFO_BY_ID: return GetMissionInfoInner(data, reply);
        case CLEAN_MISSION: return CleanMissionInner(data, reply);
        case CLEAN_ALL_MISSIONS: return CleanAllMissionsInner(data, reply);
        case REGISTER_SNAPSHOT_HANDLER: return RegisterSnapshotHandlerInner(data, reply);
        case REGISTER_WMS_HANDLER: return RegisterWindowManagerServiceHandlerInner(data, reply);
        case SET_MISSION_LABEL: return SetMissionLabelInner(data, reply);
        case SET_MISSION_ICON: return SetMissionIconInner(data, reply);
        case MOVE_MISSIONS_TO_FOREGROUND: return MoveMissionsToForegroundInner(data, reply);
        case MOVE_MISSIONS_TO_BACKGROUND: return MoveMissionsToBackgroundInner(data, reply);
        case GET_TOP_ABILITY_TOKEN: return GetTopAbilityTokenInner(data, reply);
        case GET_TOP_ABILITY: return GetTopAbilityInner(data, reply);
        case GET_MISSION_ID_BY_ABILITY_TOKEN: return GetMissionIdByTokenInner(data, reply);
        case MOVE_ABILITY_TO_BACKGROUND: return MoveAbilityToBackgroundInner(data, reply);
        case GET_MISSION_SNAPSHOT_INFO: return GetMissionSnapshotInner(data, reply);
        case DELEGATOR_DO_ABILITY_FOREGROUND: return DelegatorDoAbilityForegroundInner(data, reply);
        case DELEGATOR_DO_ABILITY_BACKGROUND: return DelegatorDoAbilityBackgroundInner(data, reply);
        case SET_MISSION_CONTINUE_STATE: return SetMissionContinueStateInner(data, reply);
        case QUERY_MISSION_VALID: return IsValidMissionIdsInner(data, reply);
        default: break;
    }

    TAG_LOGW(AAFwkTag::ABILITYMGR, "MissionListDelegatorStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t MissionListDelegatorStub::LockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t result = LockMissionForCleanup(missionId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::LockMissionForCleanup(int32_t missionId)
{
    return MissionListWrapper::GetInstance().SetMissionLockedState(missionId, true);
}

int32_t MissionListDelegatorStub::UnlockMissionForCleanupInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t result = UnlockMissionForCleanup(missionId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "lock mission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::UnlockMissionForCleanup(int32_t missionId)
{
    return MissionListWrapper::GetInstance().SetMissionLockedState(missionId, false);
}

int32_t MissionListDelegatorStub::RegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "stub register mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = RegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::RegisterMissionListener(sptr<IMissionListener> listener)
{
    return MissionListWrapper::GetInstance().RegisterMissionListener(listener);
}

int32_t MissionListDelegatorStub::UnRegisterMissionListenerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IMissionListener> listener = iface_cast<IMissionListener>(data.ReadRemoteObject());
    if (listener == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "stub unregister mission listener, listener is nullptr.");
        return ERR_INVALID_VALUE;
    }

    int32_t result = UnRegisterMissionListener(listener);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::UnRegisterMissionListener(sptr<IMissionListener> listener)
{
    return MissionListWrapper::GetInstance().UnRegisterMissionListener(listener);
}

int32_t MissionListDelegatorStub::GetMissionInfosInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t numMax = data.ReadInt32();
    std::vector<MissionInfo> missionInfos;
    int32_t result = GetMissionInfos(deviceId, numMax, missionInfos);
    reply.WriteInt32(missionInfos.size());
    for (auto &it : missionInfos) {
        if (!reply.WriteParcelable(&it)) {
            return ERR_INVALID_VALUE;
        }
    }
    if (!reply.WriteInt32(result)) {
        return ERR_INVALID_VALUE;
    }
    return result;
}

int32_t MissionListDelegatorStub::GetMissionInfos(const std::string& deviceId, int32_t numMax,
    std::vector<MissionInfo> &missionInfos)
{
    return MissionListWrapper::GetInstance().GetMissionInfos(deviceId, numMax, missionInfos);
}

int32_t MissionListDelegatorStub::GetMissionInfoInner(MessageParcel &data, MessageParcel &reply)
{
    MissionInfo info;
    std::string deviceId = Str16ToStr8(data.ReadString16());
    int32_t missionId = data.ReadInt32();
    int32_t result = GetMissionInfo(deviceId, missionId, info);
    if (!reply.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfo error");
        return ERR_INVALID_VALUE;
    }

    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionInfo result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::GetMissionInfo(const std::string& deviceId, int32_t missionId,
    MissionInfo &missionInfo)
{
    return MissionListWrapper::GetInstance().GetMissionInfo(deviceId, missionId, missionInfo);
}

int32_t MissionListDelegatorStub::CleanMissionInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t missionId = data.ReadInt32();
    int32_t result = CleanMission(missionId);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CleanMission failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::CleanMission(int32_t missionId)
{
    return MissionListWrapper::GetInstance().CleanMission(missionId);
}

int32_t MissionListDelegatorStub::CleanAllMissionsInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = CleanAllMissions();
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CleanAllMissions failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::CleanAllMissions()
{
    return MissionListWrapper::GetInstance().CleanAllMissions();
}

int32_t MissionListDelegatorStub::RegisterSnapshotHandlerInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<ISnapshotHandler> handler = iface_cast<ISnapshotHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerStub read snapshot handler failed!");
        return ERR_NULL_OBJECT;
    }
    int32_t result = RegisterSnapshotHandler(handler);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerStub register snapshot handler result = %{public}d",
        result);
    return result;
}

int32_t MissionListDelegatorStub::RegisterSnapshotHandler(sptr<ISnapshotHandler> handler)
{
    return MissionListWrapper::GetInstance().RegisterSnapshotHandler(handler);
}
#ifdef SUPPORT_SCREEN
int32_t MissionListDelegatorStub::RegisterWindowManagerServiceHandler(sptr<IWindowManagerServiceHandler> handler)
{
    return MissionListWrapper::GetInstance().RegisterWindowManagerServiceHandler(handler);
}
void MissionListDelegatorStub::UpdateMissionSnapShot(sptr<IRemoteObject> token,
    std::shared_ptr<Media::PixelMap> pixelMap)
{
    MissionListWrapper::GetInstance().UpdateMissionSnapShot(token, pixelMap);
}
int32_t MissionListDelegatorStub::SetMissionLabel(sptr<IRemoteObject> token, const std::string &label)
{
    return MissionListWrapper::GetInstance().SetMissionLabel(token, label);
}
int32_t MissionListDelegatorStub::SetMissionIcon(sptr<IRemoteObject> abilityToken,
    std::shared_ptr<Media::PixelMap> icon)
{
    return MissionListWrapper::GetInstance().SetMissionIcon(abilityToken, icon);
}
#endif // SUPPORT_SCREEN
int32_t MissionListDelegatorStub::RegisterWindowManagerServiceHandlerInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef SUPPORT_SCREEN
    sptr<IWindowManagerServiceHandler> handler = iface_cast<IWindowManagerServiceHandler>(data.ReadRemoteObject());
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s read WMS handler failed!", __func__);
        return ERR_NULL_OBJECT;
    }
    return RegisterWindowManagerServiceHandler(handler);
#else
    return NO_ERROR;
#endif // SUPPORT_SCREEN
}

int32_t MissionListDelegatorStub::UpdateMissionSnapShotInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef SUPPORT_SCREEN
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::shared_ptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read pixelMap failed.");
        return ERR_NULL_OBJECT;
    }
    UpdateMissionSnapShot(token, pixelMap);
#endif // SUPPORT_SCREEN
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::SetMissionLabelInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef SUPPORT_SCREEN
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabelInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::string label = Str16ToStr8(data.ReadString16());
    int32_t result = SetMissionLabel(token, label);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionLabel failed.");
        return ERR_INVALID_VALUE;
    }
#endif
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::SetMissionIconInner(MessageParcel &data, MessageParcel &reply)
{
#ifdef SUPPORT_SCREEN
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIconInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    std::shared_ptr<Media::PixelMap> icon(data.ReadParcelable<Media::PixelMap>());
    if (!icon) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIconInner read icon failed.");
        return ERR_NULL_OBJECT;
    }

    int32_t result = SetMissionIcon(token, icon);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionIcon failed.");
        return ERR_INVALID_VALUE;
    }
#endif
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::MoveMissionsToForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is called.", __func__);
    std::vector<int32_t> missionIds;
    data.ReadInt32Vector(&missionIds);
    int32_t topMissionId = data.ReadInt32();
    int32_t errCode = MoveMissionsToForeground(missionIds, topMissionId);
    if (!reply.WriteInt32(errCode)) {
        return ERR_INVALID_VALUE;
    }
    return errCode;
}

int32_t MissionListDelegatorStub::MoveMissionsToForeground(const std::vector<int32_t>& missionIds,
    int32_t topMissionId)
{
    return MissionListWrapper::GetInstance().MoveMissionsToForeground(missionIds, topMissionId);
}

int32_t MissionListDelegatorStub::MoveMissionsToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is called.", __func__);
    std::vector<int32_t> missionIds;
    std::vector<int32_t> result;

    data.ReadInt32Vector(&missionIds);
    int32_t errCode = MoveMissionsToBackground(missionIds, result);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is called. resultSize: %{public}zu", __func__, result.size());
    if (!reply.WriteInt32Vector(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s is called. WriteInt32Vector Failed", __func__);
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(errCode)) {
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::MoveMissionsToBackground(const std::vector<int32_t>& missionIds,
    std::vector<int32_t>& result)
{
    return MissionListWrapper::GetInstance().MoveMissionsToBackground(missionIds, result);
}

int32_t MissionListDelegatorStub::GetTopAbilityTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token;
    auto result = GetTopAbility(token);
    if (!reply.WriteRemoteObject(token)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data write failed.");
        return ERR_INVALID_VALUE;
    }
    reply.WriteInt32(result);

    return NO_ERROR;
}

int32_t MissionListDelegatorStub::GetTopAbility(sptr<IRemoteObject> &token)
{
    return MissionListWrapper::GetInstance().GetTopAbility(token);
}

int32_t MissionListDelegatorStub::GetTopAbilityInner(MessageParcel &data, MessageParcel &reply)
{
    bool isNeedLocalDeviceId = data.ReadBool();
    AppExecFwk::ElementName result = GetTopAbility(isNeedLocalDeviceId);
    if (result.GetDeviceID().empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetTopAbilityInner is nullptr");
    }
    reply.WriteParcelable(&result);
    return NO_ERROR;
}

AppExecFwk::ElementName MissionListDelegatorStub::GetTopAbility(bool isNeedLocalDeviceId)
{
    return MissionListWrapper::GetInstance().GetTopAbility(isNeedLocalDeviceId);
}

int32_t MissionListDelegatorStub::GetMissionIdByTokenInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    int32_t missionId = GetMissionIdByToken(token);
    if (!reply.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionIdByToken write missionId failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::GetMissionIdByToken(sptr<IRemoteObject> token)
{
    return MissionListWrapper::GetInstance().GetMissionIdByToken(token);
}

int32_t MissionListDelegatorStub::MoveAbilityToBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = nullptr;
    if (data.ReadBool()) {
        token = data.ReadRemoteObject();
    }
    int32_t result = MoveAbilityToBackground(token);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::MoveAbilityToBackground(sptr<IRemoteObject> token)
{
    return MissionListWrapper::GetInstance().MoveAbilityToBackground(token);
}

int32_t MissionListDelegatorStub::GetMissionSnapshotInner(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t missionId = data.ReadInt32();
    bool isLowResolution = data.ReadBool();
    MissionSnapshot missionSnapshot;
    int32_t result = GetMissionSnapshot(deviceId, missionId, missionSnapshot, isLowResolution);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "snapshot: AbilityManagerStub get snapshot result = %{public}d", result);
    if (!reply.WriteParcelable(&missionSnapshot)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionSnapshot error");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetMissionSnapshot result error");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
    MissionSnapshot& snapshot, bool isLowResolution)
{
    return MissionListWrapper::GetInstance().GetMissionSnapshot(deviceId, missionId, snapshot, isLowResolution);
}

int32_t MissionListDelegatorStub::DelegatorDoAbilityForegroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityForeground(token);
    reply.WriteInt32(result);

    return NO_ERROR;
}

int32_t MissionListDelegatorStub::DelegatorDoAbilityForeground(sptr<IRemoteObject> token)
{
    return MissionListWrapper::GetInstance().DelegatorDoAbilityForeground(token);
}

int32_t MissionListDelegatorStub::DelegatorDoAbilityBackgroundInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    auto result = DelegatorDoAbilityBackground(token);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::DelegatorDoAbilityBackground(sptr<IRemoteObject> token)
{
    return MissionListWrapper::GetInstance().DelegatorDoAbilityBackground(token);
}

int32_t MissionListDelegatorStub::SetMissionContinueStateInner(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    if (!token) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueStateInner read ability token failed.");
        return ERR_NULL_OBJECT;
    }

    int32_t state = data.ReadInt32();
    int32_t result = SetMissionContinueState(token, static_cast<ContinueState>(state));
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SetMissionContinueState failed.");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::SetMissionContinueState(sptr<IRemoteObject> token, ContinueState state)
{
    return MissionListWrapper::GetInstance().SetMissionContinueState(token, state);
}

int32_t MissionListDelegatorStub::IsValidMissionIdsInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s is called.", __func__);
    std::vector<int32_t> missionIds;
    std::vector<MissionValidResult> results;

    data.ReadInt32Vector(&missionIds);
    auto err = IsValidMissionIds(missionIds, results);
    if (err != ERR_OK) {
        results.clear();
    }

    if (!reply.WriteInt32(err)) {
        return ERR_INVALID_VALUE;
    }

    reply.WriteInt32(static_cast<int32_t>(results.size()));
    for (auto &item : results) {
        if (!reply.WriteParcelable(&item)) {
            return ERR_INVALID_VALUE;
        }
    }
    return NO_ERROR;
}

int32_t MissionListDelegatorStub::IsValidMissionIds(const std::vector<int32_t> &missionIds,
    std::vector<MissionValidResult> &results)
{
    return MissionListWrapper::GetInstance().IsValidMissionIds(missionIds, results);
}
} // AAFwk
} // OHOS