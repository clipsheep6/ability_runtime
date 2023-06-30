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

#include "uri_permission_manager_proxy.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
UriPermissionManagerProxy::UriPermissionManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IUriPermissionManager>(impl) {}

int UriPermissionManagerProxy::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int autoremove, int32_t appIndex)
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
    if (!data.WriteInt32(autoremove)) {
        HILOG_ERROR("Write autoremove failed.");
        return INNER_ERR;
    }
    if (!data.WriteInt32(appIndex)) {
        HILOG_ERROR("Write appIndex failed.");
        return INNER_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(UriPermMgrCmd::ON_GRANT_URI_PERMISSION, data, reply, option);
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
    int error = Remote()->SendRequest(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
    }
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
    int error = Remote()->SendRequest(UriPermMgrCmd::ON_REVOKE_URI_PERMISSION_MANUALLY, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return INNER_ERR;
    }
    return reply.ReadInt32();
}
}  // namespace AAFwk
}  // namespace OHOS
