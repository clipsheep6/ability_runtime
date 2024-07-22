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
#include "remote_register_service_stub.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
RemoteRegisterServiceStub::RemoteRegisterServiceStub() {}

RemoteRegisterServiceStub::~RemoteRegisterServiceStub() {}

int RemoteRegisterServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGI(AAFwkTag::CONTINUATION,
        "%{public}s called, cmd=%{public}d, flags=%{public}d", __func__, code, option.GetFlags());

    std::u16string descriptor = IRemoteRegisterService::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s local descriptor is not equal to remote", __func__);
        return ERR_INVALID_STATE;
    }
    switch (code) {
        case COMMAND_REGISTER:
            return RegisterInner(data, reply);
        case COMMAND_UNREGISTER:
            return UnregisterInner(data, reply);
        case COMMAND_UPDATE_CONNECT_STATUS:
            return UpdateConnectStatusInner(data, reply);
        case COMMAND_SHOW_DEVICE_LIST:
            return ShowDeviceListInner(data, reply);
    }
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s Not found cmd, need check.", __func__);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int RemoteRegisterServiceStub::RegisterInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    std::string bundleName = data.ReadString();
    sptr<IRemoteObject> token = data.ReadRemoteObject();
    ExtraParams *pExtras = nullptr;
    int32_t extraId = data.ReadInt32();
    if (extraId != 0) {
        pExtras = ExtraParams::Unmarshalling(data);
    }
    if (pExtras == nullptr) {
        reply.WriteInt32(ERR_INVALID_DATA);
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s error to read ExtraParams.", __func__);
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> object = data.ReadRemoteObject();
    if (object == nullptr) {
        delete pExtras;
        pExtras = nullptr;
        reply.WriteInt32(ERR_NULL_OBJECT);
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s Failed to read IConnectCallback.", __func__);
        return ERR_NULL_OBJECT;
    }

    sptr<IConnectCallback> callback = iface_cast<IConnectCallback>(object);
    int result = Register(bundleName, token, *pExtras, callback);
    delete pExtras;
    pExtras = nullptr;
    reply.WriteInt32(result);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return NO_ERROR;
}

int RemoteRegisterServiceStub::UnregisterInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    int registerToken = data.ReadInt32();
    bool result = Unregister(registerToken);
    reply.WriteInt32(result ? ERR_NONE : IPC_STUB_ERR);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return NO_ERROR;
}

int RemoteRegisterServiceStub::UpdateConnectStatusInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    int registerToken = data.ReadInt32(registerToken);
    std::string deviceId = data.ReadString();
    int status = data.ReadInt32();
    bool result = UpdateConnectStatus(registerToken, deviceId, status);
    reply.WriteInt32(result ? ERR_NONE : IPC_STUB_ERR);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return NO_ERROR;
}

int RemoteRegisterServiceStub::ShowDeviceListInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called begin", __func__);
    int registerToken = data.ReadInt32();
    ExtraParams *pExtras = nullptr;
    int32_t extraId = data.ReadInt32();
    if (extraId != 0) {
        pExtras = ExtraParams::Unmarshalling(data);
    }
    if (pExtras == nullptr) {
        reply.WriteInt32(ERR_INVALID_DATA);
        TAG_LOGE(AAFwkTag::CONTINUATION, "%{public}s Failed to read ExtraParams", __func__);
        return ERR_INVALID_DATA;
    }

    bool result = ShowDeviceList(registerToken, *pExtras);
    delete pExtras;
    pExtras = nullptr;
    reply.WriteInt32(result ? ERR_NONE : IPC_STUB_ERR);
    TAG_LOGI(AAFwkTag::CONTINUATION, "%{public}s called end", __func__);
    return NO_ERROR;
}
}  // namespace AppExecFwk
}  // namespace OHOS
