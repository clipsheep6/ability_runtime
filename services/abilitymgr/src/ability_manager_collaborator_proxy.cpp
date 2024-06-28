/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "configuration.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityInfo write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "userId write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteUint64(accessTokenIDEx)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "accessTokenIDEx write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_START_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "notify start ability failed");
        return ERR_INVALID_OPERATION;
    }
    std::unique_ptr<Want> wantInfo(reply.ReadParcelable<Want>());
    if (!wantInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelableInfo failed");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_MISSION_CREATED, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMissionCreated(const sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and sessionInfo write failed.");
            return ERR_INVALID_OPERATION;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write failed.");
            return ERR_INVALID_OPERATION;
        }
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_MISSION_CREATED_BY_SCB, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityInfo write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_LOAD_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyLoadAbility(
    const AppExecFwk::AbilityInfo &abilityInfo, const sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteParcelable(&abilityInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityInfo write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and sessionInfo write failed.");
            return ERR_INVALID_OPERATION;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write failed.");
            return ERR_INVALID_OPERATION;
        }
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_LOAD_ABILITY_BY_SCB, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMoveMissionToBackground(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_BACKGROUND, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyPreloadAbility(const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName write failed.");
        return ERR_INVALID_OPERATION;
    }
    auto remote = Remote();
    if (!remote) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote is nullptr");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = remote->SendRequest(
        IAbilityManagerCollaborator::NOTIFY_PRELOAD_ABILITY, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyMoveMissionToForeground(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_MOVE_MISSION_TO_FOREGROUND, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyTerminateMission(int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_TERMINATE_MISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "missionId write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_CLEAR_MISSION, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

int32_t AbilityManagerCollaboratorProxy::NotifyRemoveShellProcess(int32_t pid, int32_t type, const std::string &reason)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "pid write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteInt32(type)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "type write failed.");
        return ERR_INVALID_OPERATION;
    }
    if (!data.WriteString16(Str8ToStr16(reason))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "reason write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(
        IAbilityManagerCollaborator::NOTIFY_REMOVE_SHELL_PROCESS, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return NO_ERROR;
}

void AbilityManagerCollaboratorProxy::UpdateMissionInfo(InnerMissionInfoDto &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&info)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write mission info failed.");
        return;
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_MISSION_INFO, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return;
    }

    std::unique_ptr<InnerMissionInfoDto> innerInfo(reply.ReadParcelable<InnerMissionInfoDto>());
    if (!innerInfo) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get InnerMissionInfoDto error.");
        return;
    }
    info = *innerInfo;
    return;
}

void AbilityManagerCollaboratorProxy::UpdateMissionInfo(sptr<SessionInfo> &sessionInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return;
    }

    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag and sessionInfo write failed.");
            return;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "flag write failed.");
            return;
        }
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_MISSION_INFO_BY_SCB, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return;
    }

    sessionInfo = reply.ReadParcelable<SessionInfo>();
    return;
}

int32_t AbilityManagerCollaboratorProxy::CheckCallAbilityPermission(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return ERR_INVALID_OPERATION;
    }

    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "want write failed.");
        return ERR_INVALID_OPERATION;
    }
    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::CHECK_CALL_ABILITY_PERMISSION,
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return ret;
    }
    return reply.ReadInt32();
}

bool AbilityManagerCollaboratorProxy::UpdateConfiguration(const AppExecFwk::Configuration &config, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteParcelable(&config)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write config failed.");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write usr failed.");
        return false;
    }
    auto error = SendTransactCmd(IAbilityManagerCollaborator::UPDATE_CONFIGURATION, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send config error: %{public}d", error);
        return true;
    }
    return true;
}

int32_t AbilityManagerCollaboratorProxy::OpenFile(const Uri& uri, uint32_t flag)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write uri failed.");
        return false;
    }
    if (!data.WriteInt32(flag)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write flag failed.");
        return false;
    }

    int32_t ret = SendTransactCmd(IAbilityManagerCollaborator::OPEN_FILE, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d", ret);
        return -1;
    }
    return reply.ReadFileDescriptor();
}

void AbilityManagerCollaboratorProxy::NotifyMissionBindPid(int32_t missionId, int32_t pid)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(AbilityManagerCollaboratorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write missionId failed.");
        return;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write pid failed.");
        return;
    }
    auto error = SendTransactCmd(IAbilityManagerCollaborator::NOTIFY_MISSION_BIND_PID, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send config error: %{public}d", error);
    }
}

int32_t AbilityManagerCollaboratorProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
}   // namespace AAFwk
}   // namespace OHOS
