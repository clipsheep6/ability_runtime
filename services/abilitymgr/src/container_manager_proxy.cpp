/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "container_manager_proxy.h"

#include "errors.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool ContainerManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(ContainerManagerProxy::GetDescriptor())) {
        HILOG_ERROR("write interface token failed.");
        return false;
    }
    return true;
}

int ContainerManagerProxy::NotifyBootComplete(int32_t state)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return -1;
    }

    if (!data.WriteInt32(state)) {
        HILOG_ERROR("state write failed.");
        return -1;
    }

    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        HILOG_ERROR("ContainerManagerProxy::NotifyBootComplete, Remote() is nullptr");
        return -1;
    }
    int error = remoteObject->SendRequest(IContainerManager::NOTIFY_BOOT_COMPLETE, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
}  // namespace AAFwk
}  // namespace OHOS