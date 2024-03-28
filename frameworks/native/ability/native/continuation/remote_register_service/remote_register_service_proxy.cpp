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
#include "remote_register_service_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * register to controlcenter continuation register service.
 *
 * @param bundleName bundlename of ability.
 * @param extras filter with supported device list.
 * @param callback callback for device connect and disconnect.
 */
int RemoteRegisterServiceProxy::Register(const std::string &bundleName, const sptr<IRemoteObject> &token,
    const ExtraParams &extras, const sptr<IConnectCallback> &callback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called", __func__);

    if (bundleName.empty() || token == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s param invalid", __func__);
        return ERR_INVALID_DATA;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteString(bundleName) ||
        !data.WriteRemoteObject(token) || !data.WriteInt32(1) || !extras.Marshalling(data) ||
        !data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s Failed to write transfer data.", __func__);
        return IPC_INVOKER_WRITE_TRANS_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int result = SendTransactCmd(COMMAND_REGISTER, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s SendRequest ok", __func__);
        return reply.ReadInt32();
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return IPC_INVOKER_TRANSLATE_ERR;
    }
}

/**
 * unregister to controlcenter continuation register service.
 *
 * @param registerToken token from register return value.
 */
bool RemoteRegisterServiceProxy::Unregister(int registerToken)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called", __func__);

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteInt32(registerToken)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s Failed to write transfer data.", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = SendTransactCmd(COMMAND_UNREGISTER, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s SendRequest ok", __func__);
        return reply.ReadInt32() == ERR_NONE;
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

/**
 * notify continuation status to controlcenter continuation register service.
 *
 * @param registerToken token from register.
 * @param deviceId deviceid.
 * @param status device status.
 */
bool RemoteRegisterServiceProxy::UpdateConnectStatus(int registerToken, const std::string &deviceId, int status)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called", __func__);

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteInt32(registerToken) ||
        !data.WriteString(deviceId) || !data.WriteInt32(status)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s Failed to write transfer data.", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = SendTransactCmd(COMMAND_UPDATE_CONNECT_STATUS, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s SendRequest ok", __func__);
        return reply.ReadInt32() == ERR_NONE;
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

/**
 * notify controlcenter continuation register service to show device list.
 *
 * @param registerToken token from register
 * @param extras filter with supported device list.
 */
bool RemoteRegisterServiceProxy::ShowDeviceList(int registerToken, const ExtraParams &extras)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called", __func__);

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteInt32(registerToken) ||
        !data.WriteInt32(1) || !extras.Marshalling(data)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s Failed to write transfer data.", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = SendTransactCmd(COMMAND_SHOW_DEVICE_LIST, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s SendRequest ok", __func__);
        return reply.ReadInt32() == ERR_NONE;
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

int32_t RemoteRegisterServiceProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
