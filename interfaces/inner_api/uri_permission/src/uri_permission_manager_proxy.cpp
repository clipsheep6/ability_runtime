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
#include "hilog_tag_wrapper.h"
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
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::GrantUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write initiatorTokenId failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_GRANT_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::GrantUriPermission is called.");
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec is empty or exceed maximum size %{public}d.", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(uriVec.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write size of uriVec failed.");
        return INNER_ERR;
    }
    for (const auto &uri : uriVec) {
        if (!data.WriteParcelable(&uri)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(initiatorTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write initiatorTokenId failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int32_t UriPermissionManagerProxy::GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::GrantUriPermissionPrivileged is called.");
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec is empty or exceed maximum size %{public}d.", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(uriVec.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write size of uriVec failed.");
        return INNER_ERR;
    }
    for (const auto &uri : uriVec) {
        if (!data.WriteParcelable(&uri)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_GRANT_URI_PERMISSION_PRIVILEGED, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Called.");
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec is empty or exceed maximum size %{public}d.", MAX_URI_COUNT);
        return ERR_URI_LIST_OUT_OF_RANGE;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(uriVec.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write size of uriVec failed.");
        return INNER_ERR;
    }
    for (const auto &uri : uriVec) {
        if (!data.WriteParcelable(&uri)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
            return INNER_ERR;
        }
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(targetBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write targetBundleName failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write appIndex failed.");
        return INNER_ERR;
    }
    if (!data.WriteBool(isSystemAppCall)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write isSystemAppCall failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_BATCH_GRANT_URI_PERMISSION_FOR_2_IN_1, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fial, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

void UriPermissionManagerProxy::RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::RevokeUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write AccessTokenID failed.");
        return;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error: %{public}d", error);
    }
}

int UriPermissionManagerProxy::RevokeAllUriPermissions(const Security::AccessToken::AccessTokenID tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::RevokeAllUriPermissions is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write AccessTokenID failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_ALL_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

int UriPermissionManagerProxy::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::RevokeUriPermissionManually is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
        return INNER_ERR;
    }
    if (!data.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write bundleName failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION_MANUALLY, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}

bool UriPermissionManagerProxy::VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::VerifyUriPermission is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
        return false;
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed.");
        return false;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write tokenId failed.");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_VERIFY_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

std::vector<bool> UriPermissionManagerProxy::CheckUriAuthorization(const std::vector<std::string> &uriVec,
    uint32_t flag, uint32_t tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::CheckUriAuthorization is called.");
    std::vector<bool> result(uriVec.size(), false);
    if (uriVec.empty() || uriVec.size() > MAX_URI_COUNT) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec is empty or exceed maximum size %{public}d.", MAX_URI_COUNT);
        return result;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return result;
    }
    if (!data.WriteUint32(uriVec.size())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write size of uriVec failed.");
        return result;
    }
    for (const auto &uri : uriVec) {
        if (!data.WriteString(uri)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uri failed.");
            return result;
        }
    }
    if (!data.WriteUint32(flag)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write flag failed.");
        return result;
    }
    if (!data.WriteUint32(tokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write tokenId failed.");
        return result;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_CHECK_URI_AUTHORIZATION, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fial, error: %{public}d", error);
        return result;
    }
    auto size = reply.ReadUint32();
    for (auto i = 0; i < size; i++) {
        result[i] = reply.ReadBool();
    }
    return result;
}

bool UriPermissionManagerProxy::IsAuthorizationUriAllowed(uint32_t fromTokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "UriPermissionManagerProxy::IsAuthorizationUriAllowed is called.");
    MessageParcel data;
    if (!data.WriteInterfaceToken(IUriPermissionManager::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write interface token failed.");
        return false;
    }
    if (!data.WriteUint32(fromTokenId)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Write fromTokenId failed.");
        return false;
    }
    MessageParcel reply;
    MessageOption option;
    int error = SendTransactCmd(UriPermMgrCmd::ON_IS_Authorization_URI_ALLOWED, data, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest fail, error: %{public}d", error);
        return false;
    }
    return reply.ReadBool();
}

int32_t UriPermissionManagerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "remote object is nullptr.");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return ret;
    }
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
