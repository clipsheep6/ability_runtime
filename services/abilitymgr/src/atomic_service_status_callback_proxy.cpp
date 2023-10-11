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

#include "atomic_service_status_callback_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
using OHOS::AAFwk::IAtomicServiceStatusCallback;

AtomicServiceStatusCallbackProxy::AtomicServiceStatusCallbackProxy(const sptr<IRemoteObject>& impl)
    : IRemoteProxy<IAtomicServiceStatusCallback>(impl)
{
}
void AtomicServiceStatusCallbackProxy::OnInstallFinished(int resultCode, const Want &want, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IAtomicServiceStatusCallback::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("Write resultCode failed.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write want failed.");
        return;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("Write userId failed.");
        return;
    }

    int32_t error = SendTransactCmd(IAtomicServiceStatusCallbackCmd::ON_FREE_INSTALL_DONE, data,
        reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnFinished fail, error: %{public}d", error);
        return;
    }
}

void AtomicServiceStatusCallbackProxy::OnRemoteInstallFinished(int resultCode, const Want &want, int32_t userId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IAtomicServiceStatusCallback::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteInt32(resultCode)) {
        HILOG_ERROR("Write resultCode error.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write want error.");
        return;
    }

    if (!data.WriteInt32(userId)) {
        HILOG_ERROR("Write userId error.");
        return;
    }

    int32_t error = SendTransactCmd(ON_REMOTE_FREE_INSTALL_DONE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnFinished fail, error: %{public}d", error);
        return;
    }
}

void AtomicServiceStatusCallbackProxy::OnRemoveTimeoutTask(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(IAtomicServiceStatusCallback::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&want)) {
        HILOG_ERROR("Write want error.");
        return;
    }

    int32_t error = SendTransactCmd(ON_REMOVE_TIMEOUT_TASK, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("OnFinished fail, error: %{public}d", error);
        return;
    }
}

int32_t AtomicServiceStatusCallbackProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
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
