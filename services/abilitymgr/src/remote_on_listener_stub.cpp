
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

#include "remote_on_listener_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
RemoteOnListenerStub::RemoteOnListenerStub()
{}

RemoteOnListenerStub::~RemoteOnListenerStub()
{}

int RemoteOnListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = RemoteOnListenerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "RemoteOnListenerStub Local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case IRemoteOnListener::ON_CALLBACK: {
            return OnCallbackInner(data, reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t RemoteOnListenerStub::OnCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t continueState = data.ReadUint32();
    std::string srcDeviceId = data.ReadString();
    std::string bundleName = data.ReadString();
    std::string continueType = data.ReadString();
    std::string srcBundleName = data.ReadString();
    OnCallback(continueState, srcDeviceId, bundleName, continueType, srcBundleName);
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
