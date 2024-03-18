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

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int MAX_URI_COUNT = 500;
}
int UriPermissionManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IUriPermissionManager::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "InterfaceToken not equal IUriPermissionManager's descriptor.");
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
        case UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION_FOR_2_IN_1 : {
            return HandleBatchGrantUriPermissionFor2In1(data, reply);
        }
        case UriPermMgrCmd::ON_IS_Authorization_URI_ALLOWED : {
            return HandleIsAuthorizationUriAllowed(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return errCode;
}

int UriPermissionManagerStub::HandleRevokeUriPermission(MessageParcel &data, MessageParcel &reply)
{
    auto tokenId = data.ReadInt32();
    RevokeUriPermission(tokenId);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleRevokeAllUriPermission(MessageParcel &data, MessageParcel &reply)
{
    auto tokenId = data.ReadInt32();
    int result = RevokeAllUriPermissions(tokenId);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleGrantUriPermission(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "To read uri failed.");
        return ERR_DEAD_OBJECT;
    }
    auto flag = data.ReadInt32();
    auto targetBundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    auto initiatorTokenId = data.ReadUint32();
    int result = GrantUriPermission(*uri, flag, targetBundleName, appIndex, initiatorTokenId);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleBatchGrantUriPermission(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadUint32();
    if (size <= 0 || size > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "size is invalid.");
        return ERR_DEAD_OBJECT;
    }
    std::vector<Uri> uriVec;
    for (uint32_t i = 0; i < size; i++) {
        std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
        if (!uri) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "To read uri failed.");
            return ERR_DEAD_OBJECT;
        }
        uriVec.emplace_back(*uri);
    }
    auto flag = data.ReadInt32();
    auto targetBundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    auto initiatorTokenId = data.ReadUint32();
    int result = GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleRevokeUriPermissionManually(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "To read uri failed.");
        return ERR_DEAD_OBJECT;
    }
    auto bundleName = data.ReadString();
    int result = RevokeUriPermissionManually(*uri, bundleName);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleVerifyUriPermission(MessageParcel &data, MessageParcel &reply)
{
    std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
    if (!uri) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "To read uri failed.");
        return ERR_DEAD_OBJECT;
    }
    auto flag = data.ReadInt32();
    auto tokenId = data.ReadInt32();
    bool result = VerifyUriPermission(*uri, flag, tokenId);
    reply.WriteBool(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleBatchGrantUriPermissionFor2In1(MessageParcel &data, MessageParcel &reply)
{
    auto size = data.ReadUint32();
    if (size == 0 || size > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "size is invalid.");
        return ERR_DEAD_OBJECT;
    }
    std::vector<Uri> uriVec;
    for (uint32_t i = 0; i < size; i++) {
        std::unique_ptr<Uri> uri(data.ReadParcelable<Uri>());
        if (uri == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "To read uri failed.");
            return ERR_DEAD_OBJECT;
        }
        uriVec.emplace_back(*uri);
    }
    auto flag = data.ReadInt32();
    auto targetBundleName = data.ReadString();
    auto appIndex = data.ReadInt32();
    auto isSystemAppCall = data.ReadBool();
    int result = GrantUriPermissionFor2In1(uriVec, flag, targetBundleName, appIndex, isSystemAppCall);
    reply.WriteInt32(result);
    return ERR_OK;
}

int UriPermissionManagerStub::HandleIsAuthorizationUriAllowed(MessageParcel &data, MessageParcel &reply)
{
    auto fromTokenId = data.ReadInt32();
    bool result = IsAuthorizationUriAllowed(fromTokenId);
    reply.WriteBool(result);
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
