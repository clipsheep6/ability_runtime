
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

#include "remote_on_listener_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
void RemoteOnListenerProxy::OnCallback(const uint32_t continueState, const std::string &srcDeviceId,
    const std::string &bundleName, const std::string &continueType, const std::string &srcBundleName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(RemoteOnListenerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyMissionsChanged Write interface token failed.");
        return;
    }
    if (!data.WriteUint32(continueState)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyOnsChanged Write ContinueState failed.");
        return;
    }
    if (!data.WriteString(srcDeviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyOnsChanged Write srcDeviceId failed.");
        return;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyOnsChanged Write bundleName failed.");
        return;
    }
    if (!data.WriteString(continueType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyOnsChanged Write continueType failed.");
        return;
    }
    if (!data.WriteString(srcBundleName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyOnsChanged Write srcBundleName failed.");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "remote object is nullptr.");
        return;
    }
    int result = remote->SendRequest(IRemoteOnListener::ON_CALLBACK, data, reply, option);
    if (result != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyMissionsChanged SendRequest fail, error: %{public}d", result);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
