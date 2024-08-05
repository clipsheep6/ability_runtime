/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "want_sender_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool WantSenderProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(WantSenderProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "write interface token failed");
        return false;
    }
    return true;
}

void WantSenderProxy::Send(SenderInfo &senderInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&senderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "fail to WriteParcelable value");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "remote is NULL");
        return;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IWantSender::WANT_SENDER_SEND), data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "error code: %{public}d", ret);
    }
}
}  // namespace AAFwk
}  // namespace OHOS
