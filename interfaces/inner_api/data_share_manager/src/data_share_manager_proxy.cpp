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

#include "data_share_manager_proxy.h"

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
bool DataShareManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(DataShareManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

int DataShareManagerProxy::ConnectAbilityCommon(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken, AppExecFwk::ExtensionAbilityType extensionType, int32_t userId)
{
    if (connect == nullptr) {
        HILOG_ERROR("connect is nullptr");
        return ERR_INVALID_VALUE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("want write failed.");
        return ERR_INVALID_VALUE;
    }
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
    if (callerToken) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(callerToken)) {
            HILOG_ERROR("flag and callerToken write failed.");
            return ERR_INVALID_VALUE;
        }
    } else {
        if (!data.WriteBool(false)) {
            HILOG_ERROR("flag write failed.");
            return ERR_INVALID_VALUE;
        }
    }
    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("userId write failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(static_cast<int32_t>(extensionType))) {
        HILOG_ERROR("extensionType write failed.");
        return INNER_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int error = SendRequest(AbilityManagerInterfaceCode::CONNECT_ABILITY_WITH_TYPE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}

ErrCode DataShareManagerProxy::SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data,
    MessageParcel &reply, MessageOption& option)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return INNER_ERR;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}
} // namespace AAFwk
} // namespace OHOS
