/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "connection_observer_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
void ConnectionObserverProxy::OnExtensionConnected(const ConnectionData& connectionData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    HILOG_INFO("ConnectionObserverProxy OnExtensionConnected.");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&connectionData)) {
        HILOG_ERROR("Write ConnectionData error.");
        return;
    }

    if (!SendRequest(IConnectionObserver::ON_EXTENSION_CONNECTED, data, reply, option)) {
        HILOG_ERROR("OnExtensionConnected sned request fail");
        return;
    }
}

void ConnectionObserverProxy::OnExtensionDisconnected(const ConnectionData& connectionData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    HILOG_INFO("ConnectionObserverProxy OnExtensionDisconnected.");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&connectionData)) {
        HILOG_ERROR("Write ConnectionData error.");
        return;
    }

    if (!SendRequest(IConnectionObserver::ON_EXTENSION_DISCONNECTED, data, reply, option)) {
        HILOG_ERROR("OnExtensionDisconnected send request fail");
        return;
    }
}

void ConnectionObserverProxy::OnDlpAbilityOpened(const DlpStateData& dlpData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    HILOG_INFO("ConnectionObserverProxy OnDlpAbilityOpened.");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&dlpData)) {
        HILOG_ERROR("Write DlpStateData error.");
        return;
    }

    if (!SendRequest(IConnectionObserver::ON_DLP_ABILITY_OPENED, data, reply, option)) {
        HILOG_ERROR("OnDlpAbilityOpened send request fail");
        return;
    }
}

void ConnectionObserverProxy::OnDlpAbilityClosed(const DlpStateData& dlpData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    HILOG_INFO("ConnectionObserverProxy OnDlpAbilityClosed.");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&dlpData)) {
        HILOG_ERROR("Write DlpStateData error.");
        return;
    }

    if (!SendRequest(IConnectionObserver::ON_DLP_ABILITY_CLOSED, data, reply, option)) {
        HILOG_ERROR("OnDlpAbilityClosed send request fail");
        return;
    }
}

bool ConnectionObserverProxy::SendRequest(uint32_t code, MessageParcel &data,
                                          MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
