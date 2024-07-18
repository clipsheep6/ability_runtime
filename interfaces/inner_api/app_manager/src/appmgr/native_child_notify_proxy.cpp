/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "native_child_notify_proxy.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {

NativeChildNotifyProxy::NativeChildNotifyProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<INativeChildNotify>(impl)
{
}

bool NativeChildNotifyProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NativeChildNotifyProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "NativeChildNotifyProxy write interface token failed");
        return false;
    }

    return true;
}

int32_t NativeChildNotifyProxy::SendRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption& option)
{
    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        TAG_LOGE(AAFwkTag::APPMGR, "NativeChildNotifyProxy get remote object failed");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "NativeChildNotifyProxy SendRequest failed(%{public}d)", ret);
        return ret;
    }

    return NO_ERROR;
}

void NativeChildNotifyProxy::OnNativeChildStarted(const sptr<IRemoteObject> &nativeChild)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeChildNotifyProxy OnNativeChildStarted");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteRemoteObject(nativeChild)) {
        TAG_LOGE(AAFwkTag::APPMGR, "NativeChildNotifyProxy write native child ipc object failed.");
        return;
    }

    SendRequest(INativeChildNotify::IPC_ID_ON_NATIVE_CHILD_STARTED, data, reply, option);
}

void NativeChildNotifyProxy::OnError(int32_t errCode)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeChildNotifyProxy OnError(%{public}d)", errCode);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }

    if (!data.WriteInt32(errCode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "NativeChildNotifyProxy write error code failed.");
        return;
    }

    SendRequest(INativeChildNotify::IPC_ID_ON_ERROR, data, reply, option);
}

} // OHOS
} // AppExecFwk