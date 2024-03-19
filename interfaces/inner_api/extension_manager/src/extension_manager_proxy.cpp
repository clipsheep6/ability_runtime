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

#include "extension_manager_proxy.h"

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS::AAFwk {
bool ExtensionManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ExtensionManagerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::EXT, "write interface token failed.");
        return false;
    }
    return true;
}

int ExtensionManagerProxy::ConnectAbilityCommon(const Want &want, const sptr<IRemoteObject> &connect,
    const sptr<IRemoteObject> &callerToken, AppExecFwk::ExtensionAbilityType extensionType, int32_t userId,
    bool isQueryExtensionOnly)
{
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "connect is nullptr");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::EXT, "want write failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteBool(true) || !data.WriteRemoteObject(connect)) {
        TAG_LOGE(AAFwkTag::EXT, "flag and connect write failed.");
        return ERR_INVALID_VALUE;
    }
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            TAG_LOGE(AAFwkTag::EXT, "flag and callerToken write failed.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::EXT, "flag write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    if (!data.WriteInt32(userId)) {
        TAG_LOGE(AAFwkTag::EXT, "%{public}s, userId write failed.", __func__);
        return INNER_ERR;
    }
    if (!data.WriteInt32(static_cast<int32_t>(extensionType))) {
        TAG_LOGE(AAFwkTag::EXT, "%{public}s, extensionType write failed.", __func__);
        return INNER_ERR;
    }
    if (!data.WriteBool(isQueryExtensionOnly)) {
        TAG_LOGE(AAFwkTag::EXT, "isQueryExtensionOnly write failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = SendRequest(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "%{public}s, Send request error: %{public}d", __func__, error);
        return error;
    }
    return reply.ReadInt32();
}

int ExtensionManagerProxy::DisconnectAbility(const sptr<IRemoteObject> &connect)
{
    if (connect == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "disconnect ability fail, connect is nullptr");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(connect)) {
        TAG_LOGE(AAFwkTag::EXT, "connect write failed.");
        return ERR_INVALID_VALUE;
    }

    MessageParcel reply;
    MessageOption option;
    auto error = SendRequest(AbilityManagerInterfaceCode::DISCONNECT_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::EXT, "Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

ErrCode ExtensionManagerProxy::SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data,
    MessageParcel &reply, MessageOption& option)
{
    auto remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "Remote() is NULL");
        return INNER_ERR;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}
}  // namespace OHOS::AAFwk
