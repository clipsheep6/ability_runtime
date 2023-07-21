/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_manager_collaborator_proxy.h"
#include "errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
int32_t AbilityManagerCollaboratorProxy::NotifyStartAbility(
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t userId, Want &want, uint64_t accessTokenIDEx)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        HILOG_ERROR("abilityInfo write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint64(accessTokenIDEx)) {
        HILOG_ERROR("accessTokenIDEx write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(IAbilityManagerCollaborator::NOTIFY_START_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        HILOG_ERROR("notify start ability failed");
        return ERR_INVALID_OPERATION;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        HILOG_ERROR("readParcelableInfo failed");
        return ERR_INVALID_OPERATION;
    }
    want = *wantInfo;
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMissionCreated(int32_t missionId, const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(IAbilityManagerCollaborator::NOTIFY_MISSION_CREATED, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyLoadAbility(
    const AppExecFwk::AbilityInfo &abilityInfo, int32_t missionId, const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        HILOG_ERROR("abilityInfo write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(IAbilityManagerCollaborator::NOTIFY_LOAD_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMoveMissionToBackground(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_BACKGROUND, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMoveMissionToForeground(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_FOREGROUND, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyTerminateMission(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_TERMINATE_MISSION, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyClearMission(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        HILOG_ERROR("missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_CLEAR_MISSION, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(pid)) {
        HILOG_ERROR("pid write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(type)) {
        HILOG_ERROR("type write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString16(Str8ToStr16(reason))) {
        HILOG_ERROR("reason write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        HILOG_ERROR("remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_REMOVE_SHELL_PROCESS, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}
}   // namespace AAFwk
}   // namespace OHOS
