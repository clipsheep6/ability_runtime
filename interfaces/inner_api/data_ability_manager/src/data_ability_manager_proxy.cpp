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

#include "data_ability_manager_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk { 

bool DataAbilityManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(DataAbilityManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

ErrCode DataAbilityManagerProxy::SendRequest(AbilityManagerInterfaceCode code, MessageParcel &data,
    MessageParcel &reply, MessageOption& option)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote() is NULL");
        return INNER_ERR;
    }

    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

sptr<IAbilityScheduler> DataAbilityManagerProxy::AcquireDataAbility(
    const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!callerToken) {
        HILOG_ERROR("invalid parameters for acquire data ability.");
        return nullptr;
    }
    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }
    if (!data.WriteString(uri.ToString()) || !data.WriteBool(tryBind) || !data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("data write failed.");
        return nullptr;
    }

    error = SendRequest(AbilityManagerInterfaceCode::ACQUIRE_DATA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return nullptr;
    }

    return iface_cast<IAbilityScheduler>(reply.ReadRemoteObject());
}

int DataAbilityManagerProxy::ReleaseDataAbility(
    sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken)
{
    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!dataAbilityScheduler || !callerToken) {
        return ERR_INVALID_VALUE;
    }
    if (!WriteInterfaceToken(data)) {
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(dataAbilityScheduler->AsObject()) || !data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("data write failed.");
        return INNER_ERR;
    }

    error = SendRequest(AbilityManagerInterfaceCode::RELEASE_DATA_ABILITY, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
}  // namespace AAFwk
}  // namespace OHOS