/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "remote_mission_listener_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void RemoteMissionListenerProxy::NotifyMissionsChanged(const std::string& deviceId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteMissionListenerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyMissionsChanged Write interface token failed.");
        return;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyMissionsChanged Write deviceId failed.");
        return;
    }
    int32_t result = SendTransactCmd(IRemoteMissionListener::NOTIFY_MISSION_CHANGED, data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyMissionsChanged SendRequest fail, error: %{public}d", result);
        return;
    }
}

void RemoteMissionListenerProxy::NotifySnapshot(const std::string& deviceId, int32_t missionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteMissionListenerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifySnapshot Write interface token failed.");
        return;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifySnapshot Write deviceId failed.");
        return;
    }
    if (!data.WriteInt32(missionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifySnapshot Write missionId failed.");
        return;
    }
    int32_t result = SendTransactCmd(IRemoteMissionListener::NOTIFY_SNAPSHOT, data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifySnapshot SendRequest fail, error: %{public}d", result);
        return;
    }
}

void RemoteMissionListenerProxy::NotifyNetDisconnect(const std::string& deviceId, int32_t state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteMissionListenerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyNetDisconnect Write interface token failed.");
        return;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyNetDisconnect Write deviceId failed.");
        return;
    }
    if (!data.WriteInt32(state)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyNetDisconnect Write missionId failed.");
        return;
    }
    int32_t result = SendTransactCmd(IRemoteMissionListener::NOTIFY_NET_DISCONNECT, data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyNetDisconnect SendRequest fail, error: %{public}d", result);
        return;
    }
}

int32_t RemoteMissionListenerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
