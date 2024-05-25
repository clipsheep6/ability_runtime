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

#include "native_child_notify_stub.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {

int NativeChildNotifyStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::APPMGR, "NativeChildNotifyStub::OnRemoteRequest, code=%{public}u, flags=%{public}d.",
        code, option.GetFlags());
    std::u16string descriptor = NativeChildNotifyStub::GetDescriptor();
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (descriptor != remoteDesc) {
        TAG_LOGE(AAFwkTag::APPMGR, "A local descriptor is not equivalent to a remote");
        return ERR_INVALID_STATE;
    }

    int32_t ret;
    switch (code) {
        case INativeChildNotify::IPC_ID_ON_NATIVE_CHILD_STARTED:
            ret = HandleOnNativeChildStarted(data, reply);
            break;

        case INativeChildNotify::IPC_ID_ON_ERROR:
            ret = HandleOnError(data, reply);
            break;
        
        default:
            TAG_LOGW(AAFwkTag::APPMGR, "NativeChildNotifyStub Unknow ipc call(%{public}u)", code);
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }

    TAG_LOGD(AAFwkTag::APPMGR, "NativeChildNotifyStub::OnRemoteRequest end");
    return ret;
}

int32_t NativeChildNotifyStub::HandleOnNativeChildStarted(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> cb = data.ReadRemoteObject();
    OnNativeChildStarted(cb);
    return ERR_NONE;
}

int32_t NativeChildNotifyStub::HandleOnError(MessageParcel &data, MessageParcel &reply)
{
    int32_t err = data.ReadInt32();
    OnError(err);
    return ERR_NONE;
}

} // OHOS
} // AppExecFwk
