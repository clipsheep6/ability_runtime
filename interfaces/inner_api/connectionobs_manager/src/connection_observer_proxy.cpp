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

#include "connection_observer_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
void ConnectionObserverProxy::OnExtensionConnected(const ConnectionData& connectionData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    TAG_LOGD(AAFwkTag::CONNECTION, "called");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write token failed");
        return;
    }

    if (!data.WriteParcelable(&connectionData)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write ConnectionData error");
        return;
    }

    int error = SendTransactCmd(IConnectionObserver::ON_EXTENSION_CONNECTED, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "send request error: %{public}d", error);
        return;
    }
}

void ConnectionObserverProxy::OnExtensionDisconnected(const ConnectionData& connectionData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    TAG_LOGD(AAFwkTag::CONNECTION, "called");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write token failed");
        return;
    }

    if (!data.WriteParcelable(&connectionData)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write ConnectionData error");
        return;
    }

    int error = SendTransactCmd(IConnectionObserver::ON_EXTENSION_DISCONNECTED, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "send request error: %{public}d", error);
        return;
    }
}

#ifdef WITH_DLP
void ConnectionObserverProxy::OnDlpAbilityOpened(const DlpStateData& dlpData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    TAG_LOGI(AAFwkTag::CONNECTION, "called");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write token failed");
        return;
    }

    if (!data.WriteParcelable(&dlpData)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write DlpStateData error");
        return;
    }

    int error = SendTransactCmd(IConnectionObserver::ON_DLP_ABILITY_OPENED, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "send request error: %{public}d", error);
        return;
    }
}

void ConnectionObserverProxy::OnDlpAbilityClosed(const DlpStateData& dlpData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    TAG_LOGI(AAFwkTag::CONNECTION, "called");
    if (!data.WriteInterfaceToken(IConnectionObserver::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write token failed");
        return;
    }

    if (!data.WriteParcelable(&dlpData)) {
        TAG_LOGE(AAFwkTag::CONNECTION, "Write DlpStateData error");
        return;
    }

    int error = SendTransactCmd(IConnectionObserver::ON_DLP_ABILITY_CLOSED, data, reply, option);
    if (error != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "send request error: %{public}d", error);
        return;
    }
}
#endif // WITH_DLP

int32_t ConnectionObserverProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::CONNECTION, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::CONNECTION, "SendRequest failed. code: %{public}d, ret: %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
