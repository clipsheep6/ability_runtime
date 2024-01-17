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

    HILOG_DEBUG("called");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&connectionData)) {
        HILOG_ERROR("Write ConnectionData error.");
        return;
    }

    int error = SendTransactCmd(IConnectionObserver::ON_EXTENSION_CONNECTED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnExtensionConnected sned request fail, error: %{public}d", error);
        return;
    }
}

void ConnectionObserverProxy::OnExtensionDisconnected(const ConnectionData& connectionData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    HILOG_DEBUG("called.");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&connectionData)) {
        HILOG_ERROR("Write ConnectionData error.");
        return;
    }

    int error = SendTransactCmd(IConnectionObserver::ON_EXTENSION_DISCONNECTED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnExtensionDisconnected send request fail, error: %{public}d", error);
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

    int error = SendTransactCmd(IConnectionObserver::ON_DLP_ABILITY_OPENED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnDlpAbilityOpened send request fail, error: %{public}d", error);
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

    int error = SendTransactCmd(IConnectionObserver::ON_DLP_ABILITY_CLOSED, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnDlpAbilityClosed send request fail, error: %{public}d", error);
        return;
    }
}

int32_t ConnectionObserverProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
