
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

#include "remote_on_listener_proxy.h"

#include "hilog_wrapper.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
void RemoteOnListenerProxy::OnCallback(const uint32_t continueState, const std::string &srcDeviceId, const std::string &bundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteOnListenerProxy::GetDescriptor())) {
        HILOG_ERROR("NotifyMissionsChanged Write interface token failed.");
        return;
    }
    if (!data.WriteUint32(continueState)) {
        HILOG_ERROR("NotifyOnsChanged Write ContinueState failed.");
        return;
    }
    if (!data.WriteString(srcDeviceId)) {
        HILOG_ERROR("NotifyOnsChanged Write srcDeviceId failed.");
        return;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("NotifyOnsChanged Write bundleName failed.");
        return;
    }
    int result = Remote()->SendRequest(IRemoteOnListener::ON_CALLBACK, data, reply, option);
    if (result != NO_ERROR) {
        HILOG_ERROR("NotifyMissionsChanged SendRequest fail, error: %{public}d", result);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
