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

#include "uri_permission_manager_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int MAX_URI_COUNT = 500;
}
UriPermissionManagerProxy::UriPermissionManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IUriPermissionManager>(impl) {}

int UriPermissionManagerProxy::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("UriPermissionManagerProxy::GrantUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("Write uri failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(flag)) {
        HILOG_ERROR("Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        HILOG_ERROR("Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        HILOG_ERROR("Write appIndex failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        HILOG_ERROR("Write initiatorTokenId failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_GRANT_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("UriPermissionManagerProxy::GrantUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(uriVec.size())) {
        HILOG_ERROR("Write size of uriVec failed.");
        return INNER_ERR;
    }
    for (const auto &uri : uriVec) {
        if (!data.WriteParcelable(&uri)) {
            HILOG_ERROR("Write uri failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(flag)) {
        HILOG_ERROR("Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        HILOG_ERROR("Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        HILOG_ERROR("Write appIndex failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        HILOG_ERROR("Write initiatorTokenId failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
{
    HILOG_DEBUG("Called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (uriVec.size() > MAX_URI_COUNT) {
        HILOG_ERROR("Exceeded maximum uri count.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(uriVec.size())) {
        HILOG_ERROR("Write size of uriVec failed.");
        return INNER_ERR;
    }
    for (const auto &uri : uriVec) {
        if (!data.WriteParcelable(&uri)) {
            HILOG_ERROR("Write uri failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteInt32(flag)) {
        HILOG_ERROR("Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        HILOG_ERROR("Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        HILOG_ERROR("Write appIndex failed.");
        return INNER_ERR;
    }
    if (!data.WriteBool(isSystemAppCall)) {
        HILOG_ERROR("Write isSystemAppCall failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION_FOR_2_IN_1, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

void UriPermissionManagerProxy::RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("UriPermissionManagerProxy::RevokeUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }
    if (!data.WriteInt32(tokenId)) {
        HILOG_ERROR("Write AccessTokenID failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
}

int UriPermissionManagerProxy::RevokeAllUriPermissions(const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("UriPermissionManagerProxy::RevokeAllUriPermissions is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(tokenId)) {
        HILOG_ERROR("Write AccessTokenID failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_ALL_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return INNER_ERR;
    }
    return ERR_OK;
}

int UriPermissionManagerProxy::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    HILOG_DEBUG("UriPermissionManagerProxy::RevokeUriPermissionManually is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("Write uri failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("Write bundleName failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION_MANUALLY, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

bool UriPermissionManagerProxy::VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    HILOG_DEBUG("UriPermissionManagerProxy::VerifyUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }
    if (!data.WriteParcelable(&uri)) {
        HILOG_ERROR("Write uri failed.");
        return false;
    }
    if (!data.WriteInt32(flag)) {
        HILOG_ERROR("Write flag failed.");
        return false;
    }
    if (!data.WriteInt32(tokenId)) {
        HILOG_ERROR("Write tokenId failed.");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_VERIFY_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

bool UriPermissionManagerProxy::IsAuthorizationUriAllowed(uint32_t fromTokenId)
{
    HILOG_DEBUG("UriPermissionManagerProxy::IsAuthorizationUriAllowed is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return false;
    }
    if (!data.WriteInt32(fromTokenId)) {
        HILOG_ERROR("Write fromTokenId failed.");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_IS_Authorization_URI_ALLOWED, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t UriPermissionManagerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
