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

#include "connection_observer_stub.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
ConnectionObserverStub::ConnectionObserverStub() {}

int ConnectionObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = ConnectionObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::CONNECTION, "invalid descriptor");
        return ERR_INVALID_STATE;
    }
    if (code < IConnectionObserver::CMD_MAX && code >= 0) {
        switch (code) {
            case ON_EXTENSION_CONNECTED:
                return OnExtensionConnectedInner(data, reply);
            case ON_EXTENSION_DISCONNECTED:
                return OnExtensionDisconnectedInner(data, reply);
#ifdef WITH_DLP
            case ON_DLP_ABILITY_OPENED:
                return OnDlpAbilityOpenedInner(data, reply);
            case ON_DLP_ABILITY_CLOSED:
                return OnDlpAbilityClosedInner(data, reply);
#endif // WITH_DLP
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int ConnectionObserverStub::OnExtensionConnectedInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ConnectionData> connectionData(data.ReadParcelable<ConnectionData>());
    if (!connectionData) {
        TAG_LOGE(AAFwkTag::CONNECTION, "error connectionData");
        return ERR_INVALID_VALUE;
    }

    OnExtensionConnected(*connectionData);
    return NO_ERROR;
}

int ConnectionObserverStub::OnExtensionDisconnectedInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<ConnectionData> connectionData(data.ReadParcelable<ConnectionData>());
    if (!connectionData) {
        TAG_LOGE(AAFwkTag::CONNECTION, "error connectionData");
        return ERR_INVALID_VALUE;
    }

    OnExtensionDisconnected(*connectionData);
    return NO_ERROR;
}

#ifdef WITH_DLP
int ConnectionObserverStub::OnDlpAbilityOpenedInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<DlpStateData> dlpData(data.ReadParcelable<DlpStateData>());
    if (!dlpData) {
        TAG_LOGE(AAFwkTag::CONNECTION, "error dlpData");
        return ERR_INVALID_VALUE;
    }

    OnDlpAbilityOpened(*dlpData);
    return NO_ERROR;
}

int ConnectionObserverStub::OnDlpAbilityClosedInner(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<DlpStateData> dlpData(data.ReadParcelable<DlpStateData>());
    if (!dlpData) {
        TAG_LOGE(AAFwkTag::CONNECTION, "error dlpData");
        return ERR_INVALID_VALUE;
    }

    OnDlpAbilityClosed(*dlpData);
    return NO_ERROR;
}
#endif // WITH_DLP
}  // namespace AbilityRuntime
}  // namespace OHOS
