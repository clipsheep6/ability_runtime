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

#include "uri_permission_manager_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int MAX_URI_COUNT = 500;
}
int UriPermissionManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IUriPermissionManager::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "InterfaceToken invalid");
        return ERR_INVALID_VALUE;
    }
    ErrCode errCode = ERR_OK;
    switch (code) {
        case UriPermMgrCmd::ON_GRANT_URI_PERMISSION : {
            return HandleGrantUriPermission(data, reply);
        }
        case UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION : {
            return HandleBatchGrantUriPermission(data, reply);
        }
        case UriPermMgrCmd::ON_GRANT_URI_PERMISSION_PRIVILEGED : {
            return HandleGrantUriPermissionPrivileged(data, reply);
        }
        case UriPermMgrCmd::ON_REVOKE_URI_PERMISSION : {
            return HandleRevokeUriPermission(data, reply);
        }
        case UriPermMgrCmd::ON_REVOKE_ALL_URI_PERMISSION : {
            return HandleRevokeAllUriPermission(data, reply);
        }
        case UriPermMgrCmd::ON_REVOKE_URI_PERMISSION_MANUALLY : {
            return HandleRevokeUriPermissionManually(data, reply);
        }
        case UriPermMgrCmd::ON_VERIFY_URI_PERMISSION : {
            return HandleVerifyUriPermission(data, reply);
        }
        case UriPermMgrCmd::ON_CHECK_URI_AUTHORIZATION : {
            return HandleCheckUriAuthorization(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return errCode;
}

int UriPermissionManagerStub::HandleRevokeUriPermission(MessageParcel &data, MessageParcel &reply)
{
    auto tokenId = data.ReadUint32();
    auto abilityId = data.ReadInt32();
    RevokeUriPermission(tokenId, abilityId);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleRevokeAllUriPermission(MessageParcel &data, MessageParcel &reply)
{
    auto tokenId = data.ReadUint32();
    int result = RevokeAllUriPermissions(tokenId);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleGrantUriPermission(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "read uri failed");
        return ERR_DEAD_OBJECT;
    }
    auto flag = data.ReadUint32();
    auto targetBundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    auto initiatorTokenId = data.ReadUint32();
    auto abilityId = data.ReadInt32();
    int result = GrantUriPermission(*uri, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleBatchGrantUriPermission(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    std::vector<Uri> uriVec;
    for (uint32_t i = 0; i < size; i++) {
        std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
        if (!uri) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "read uri failed");
            return ERR_DEAD_OBJECT;
        }
        uriVec.emplace_back(*uri);
    }
    auto flag = data.ReadUint32();
    auto targetBundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    auto initiatorTokenId = data.ReadUint32();
    auto abilityId = data.ReadInt32();
    int result = GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
    reply.WriteInt32(result);
    return ERR_OK;
}

int32_t UriPermissionManagerStub::HandleGrantUriPermissionPrivileged(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    std::vector<Uri> uriVec;
    for (uint32_t i = 0; i < size; i++) {
        std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
        if (!uri) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "read uri failed");
            return ERR_DEAD_OBJECT;
        }
        uriVec.emplace_back(*uri);
    }
    auto flag = data.ReadUint32();
    auto targetBundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    int result = GrantUriPermissionPrivileged(uriVec, flag, targetBundleName, appIndex);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleRevokeUriPermissionManually(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "read uri failed");
        return ERR_DEAD_OBJECT;
    }
    auto bundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    int result = RevokeUriPermissionManually(*uri, bundleName, appIndex);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleVerifyUriPermission(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "read uri failed");
        return ERR_DEAD_OBJECT;
    }
    auto flag = data.ReadUint32();
    auto tokenId = data.ReadUint32();
    bool result = VerifyUriPermission(*uri, flag, tokenId);
    reply.WriteBool(result);
    return ERR_OK;
}

int32_t UriPermissionManagerStub::HandleCheckUriAuthorization(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec empty or exceed maxSize %{public}d", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    std::vector<std::string> uriVec;
    for (uint32_t i = 0; i < size; i++) {
        auto uri = data.ReadString();
        uriVec.emplace_back(uri);
    }
    auto flag = data.ReadUint32();
    auto tokenId = data.ReadUint32();
    auto result = CheckUriAuthorization(uriVec, flag, tokenId);
    if (!reply.WriteUint32(result.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uriVec size failed");
        return ERR_DEAD_OBJECT;
    }
    for (auto res: result) {
        if (!reply.WriteBool(res)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write res failed");
            return ERR_DEAD_OBJECT;
        }
    }
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
