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

#include "uiextension_manager_proxy.h"

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS::AAFwk {

#define CHECK_POINTER_AND_RETURN_LOG(object, value, log) \
    if (!object) {                                       \
        HILOG_ERROR("%{public}s:", log);                 \
        return value;                                    \
    }

bool UIExtensionManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(UIExtensionManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

ErrCode UIExtensionManagerProxy::SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data,
    MessageParcel &reply, MessageOption& option)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return INNER_ERR;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

int UIExtensionManagerProxy::StartUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int32_t userId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    CHECK_POINTER_AND_RETURN_LOG(extensionSessionInfo, ERR_INVALID_VALUE,
        "connect ability fail, extensionSessionInfo is nullptr");
    if (extensionSessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(extensionSessionInfo)) {
            HILOG_ERROR("flag and extensionSessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("StartExtensionAbility, userId write failed.");
        return INNER_ERR;
    }

    int error;
    MessageParcel reply;
    MessageOption option;
    error = SendRequest(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("StartExtensionAbility, Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int UIExtensionManagerProxy::TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo, int resultCode,
    const Want *resultWant)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    CHECK_POINTER_AND_RETURN_LOG(extensionSessionInfo, ERR_INVALID_VALUE,
        "connect ability fail, extensionSessionInfo is nullptr");
    if (extensionSessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(extensionSessionInfo)) {
            HILOG_ERROR("flag and extensionSessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }

    if (!data.WriteInt32(resultCode) || !data.WriteParcelable(resultWant)) {
        HILOG_ERROR("data write failed.");
        return INNER_ERR;
    }

    int error;
    MessageParcel reply;
    MessageOption option;
    error = SendRequest(AbilityManagerInterfaceCode::TERMINATE_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int UIExtensionManagerProxy::MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
    bool fromUser)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    CHECK_POINTER_AND_RETURN_LOG(extensionSessionInfo, ERR_INVALID_VALUE,
        "connect ability fail, extensionSessionInfo is nullptr");
    if (extensionSessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(extensionSessionInfo)) {
            HILOG_ERROR("flag and extensionSessionInfo write failed.");
            return INNER_ERR;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteBool(fromUser)) {
        HILOG_ERROR("data write failed.");
        return ERR_INVALID_VALUE;
    }

    int error;
    MessageParcel reply;
    MessageOption option;
    error = SendRequest(AbilityManagerInterfaceCode::MINIMIZE_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int UIExtensionManagerProxy::ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<SessionInfo> &sessionInfo, int32_t userId)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_VALUE;
    }
    CHECK_POINTER_AND_RETURN_LOG(connect, ERR_INVALID_VALUE, "connect ability fail, connect is nullptr");
    if (connect->AsObject()) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(connect->AsObject())) {
            HILOG_ERROR("flag and connect write failed.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    CHECK_POINTER_AND_RETURN_LOG(sessionInfo, ERR_INVALID_VALUE, "connect ability fail, sessionInfo is nullptr");
    if (sessionInfo) {
        if (!data.WriteBool(true) || !data.WriteParcelable(sessionInfo)) {
            HILOG_ERROR("flag and sessionInfo write failed.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("UserId write failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendRequest(AbilityManagerInterfaceCode::CONNECT_UI_EXTENSION_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

int UIExtensionManagerProxy::CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }

    if (!data.WriteUint32(uiExtensionTokenId)) {
        HILOG_ERROR("uiExtensionTokenId write failed.");
        return ERR_INVALID_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    auto error = remote->SendRequest(IAbilityManager::CHECK_UI_EXTENSION_IS_FOCUSED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }

    isFocused = reply.ReadBool();
    return NO_ERROR;
}
} // namespace OHOS::AAFwk
