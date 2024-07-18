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
#include "connect_callback_proxy.h"

#include "extra_params.h"
#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Remote device sends connection request.
 * @param deviceId indicators id of connection device.
 * @param deviceType indicators type of connection device.
 * @return none
 */
void ConnectCallbackProxy::Connect(const string &deviceId, const string &deviceType)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(IConnectCallback::GetDescriptor()) || !data.WriteString(deviceId) ||
        !data.WriteString(deviceType)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "params is wrong");
        return;
    }
    RemoteRequest(data, COMMAND_CONNECT);
}
/**
 * @brief Remote device sends disconnection request.
 * @param deviceId indicators id of disconnection device.
 * @return none
 */
void ConnectCallbackProxy::Disconnect(const string &deviceId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(IConnectCallback::GetDescriptor()) || !data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "params is wrong");
        return;
    }
    RemoteRequest(data, COMMAND_DISCONNECT);
}
void ConnectCallbackProxy::RemoteRequest(MessageParcel &data, int commandDisconnect)
{
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "object is nullptr");
        return;
    }
    remote->SendRequest(commandDisconnect, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
