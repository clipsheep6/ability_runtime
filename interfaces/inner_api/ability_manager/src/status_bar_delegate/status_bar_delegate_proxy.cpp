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

#include "status_bar_delegate_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {

StatusBarDelegateProxy::StatusBarDelegateProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStatusBarDelegate>(impl) {}

int32_t StatusBarDelegateProxy::CheckIfStatusBarItemExists(uint32_t accessTokenId, bool& isExist)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(IStatusBarDelegate::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    if (!data.WriteUint32(accessTokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "accessTokenId write failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    auto ret = SendRequest(StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d.", ret);
        return ret;
    }
    ret = reply.ReadInt32();
    isExist = reply.ReadBool();
    return ret;
}

int32_t StatusBarDelegateProxy::AttachPidToStatusBarItem(uint32_t accessTokenId, int32_t pid)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(IStatusBarDelegate::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    if (!data.WriteUint32(accessTokenId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write accessTokenId failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    if (!data.WriteInt32(pid)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write pid failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    auto ret = SendRequest(StatusBarDelegateCmd::ATTACH_PID_TO_STATUS_BAR_ITEM, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send request error: %{public}d.", ret);
        return ret;
    }
    return reply.ReadInt32();
}

int32_t StatusBarDelegateProxy::SendRequest(
    StatusBarDelegateCmd code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }
    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}
} // namespace AbilityRuntime
} // namespace OHOS