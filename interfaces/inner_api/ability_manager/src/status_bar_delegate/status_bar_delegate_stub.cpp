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

#include "status_bar_delegate_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AbilityRuntime {
StatusBarDelegateStub::StatusBarDelegateStub()
{
    vecMemberFunc_.resize(static_cast<size_t>(StatusBarDelegateCmd::END));
    vecMemberFunc_[static_cast<uint32_t>(StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS)] =
        &StatusBarDelegateStub::HandleCheckIfStatusBarItemExists;
    vecMemberFunc_[static_cast<uint32_t>(StatusBarDelegateCmd::ATTACH_PID_TO_STATUS_BAR_ITEM)] =
        &StatusBarDelegateStub::HandleAttachPidToStatusBarItem;
}

int32_t StatusBarDelegateStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = StatusBarDelegateStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code < static_cast<uint32_t>(StatusBarDelegateCmd::END)) {
        auto memberFunc = vecMemberFunc_[code];
        return (this->*memberFunc)(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t StatusBarDelegateStub::HandleCheckIfStatusBarItemExists(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    uint32_t accessTokenId = data.ReadUint32();
    bool isExist = false;
    auto result = CheckIfStatusBarItemExists(accessTokenId, isExist);
    if (!reply.WriteBool(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    if (!reply.WriteBool(isExist)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write isExist failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    return NO_ERROR;
}

int32_t StatusBarDelegateStub::HandleAttachPidToStatusBarItem(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    uint32_t accessTokenId = data.ReadUint32();
    int32_t pid = data.ReadInt32();
    auto result = AttachPidToStatusBarItem(accessTokenId, pid);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write result failed.");
        return AAFwk::ERR_NATIVE_IPC_PARCEL_FAILED;
    }
    return NO_ERROR;
}
} // namespace AbilityRuntime
} // namespace OHOS