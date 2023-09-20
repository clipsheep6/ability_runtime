/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
    HILOG_INFO("%{public}s called", __func__);

    if (bundleName.empty() || token == nullptr || callback == nullptr) {
        HILOG_ERROR("%{public}s param invalid", __func__);
        return ERR_INVALID_DATA;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteString(bundleName) ||
        !data.WriteRemoteObject(token) || !data.WriteInt32(1) || !extras.Marshalling(data) ||
        !data.WriteRemoteObject(callback->AsObject())) {
        HILOG_ERROR("%{public}s Failed to write transfer data.", __func__);
        return IPC_INVOKER_WRITE_TRANS_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    if (SendRequest(COMMAND_REGISTER, data, reply, option)) {
        HILOG_INFO("SendRequest success");
        return reply.ReadInt32();
    } else {
        HILOG_ERROR("SendRequest failed" );
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
    HILOG_INFO("%{public}s called", __func__);

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteInt32(registerToken)) {
        HILOG_ERROR("%{public}s Failed to write transfer data.", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    if (SendRequest(COMMAND_UNREGISTER, data, reply, option)) {
        HILOG_INFO("SendRequest success");
        return reply.ReadInt32() == ERR_NONE;
    } else {
        HILOG_INFO("SendRequest failed");
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
    HILOG_INFO("%{public}s called", __func__);

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteInt32(registerToken) ||
        !data.WriteString(deviceId) || !data.WriteInt32(status)) {
        HILOG_ERROR("%{public}s Failed to write transfer data.", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    if (SendRequest(COMMAND_UPDATE_CONNECT_STATUS, data, reply, option)) {
        HILOG_INFO("SendRequest success");
        return reply.ReadInt32() == ERR_NONE;
    } else {
        HILOG_INFO("SendRequest failed");
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
    HILOG_INFO("%{public}s called", __func__);

    MessageParcel data;
    if (!data.WriteInterfaceToken(IRemoteRegisterService::GetDescriptor()) || !data.WriteInt32(registerToken) ||
        !data.WriteInt32(1) || !extras.Marshalling(data)) {
        HILOG_ERROR("%{public}s Failed to write transfer data.", __func__);
        return false;
    }

    MessageParcel reply;
    MessageOption option;
    if (SendRequest(COMMAND_SHOW_DEVICE_LIST, data, reply, option)) {
        HILOG_INFO("SendRequest success");
        return reply.ReadInt32() == ERR_NONE;
    } else {
        HILOG_INFO("SendRequest failed");
        return false;
    }
}

bool RemoteRegisterServiceProxy::SendRequest(uint32_t code, MessageParcel &data,
                                             MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t ret = remote->SendRequest(static_cast<int32_t>(code), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
