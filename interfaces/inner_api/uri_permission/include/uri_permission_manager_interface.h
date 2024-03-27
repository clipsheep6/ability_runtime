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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_INTERFACE_H
#include <vector>
#include "base/security/access_token/interfaces/innerkits/accesstoken/include/access_token.h"
#include "iremote_broker.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
class IUriPermissionManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.ability.UriPermissionManager");

    /**
     * @brief Authorize the uri permission to targetBundleName.
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     * @return Returns true if the authorization is successful, otherwise returns false.
     */
    virtual int GrantUriPermission(const Uri &uri, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0) = 0;

    /**
     * @brief Authorize the uri permission to targetBundleName.
     *
     * @param uriVec The file urilist.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     * @return Returns true if the authorization is successful, otherwise returns false.
     */
    virtual int GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0) = 0;

    /**
     * @brief Authorize the uri permission to targetBundleName for 2in1, only supports foundation process calls.
     *
     * @param uriVec The file urilist.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     * @param appIndex The index of application in sandbox.
     * @param isSystemAppCall The flag of system application called.
     * @return Returns true if the authorization is successful, otherwise returns false.
     */
    virtual int GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string &targetBundleName, int32_t appIndex = 0, bool isSystemAppCall = false) = 0;

    /**
     * @brief Clear user's uri authorization record with autoremove flag.
     *
     * @param tokenId A tokenId of an application.
     * @return Returns true if the remove is successful, otherwise returns false.
     */
    virtual void RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId) = 0;

    /**
     * @brief Clear user's all uri authorization record with autoremove flag.
     *
     * @param tokenId A tokenId of an application.
     * @return Returns true if the remove is successful, otherwise returns false.
     */
    virtual int RevokeAllUriPermissions(const Security::AccessToken::AccessTokenID tokenId) = 0;

    /**
     * @brief Clear user's uri authorization record.
     *
     * @param uri The file uri.
     * @param bundleName bundleName of an application.
     * @return Returns true if the remove is successful, otherwise returns false.
     */
    virtual int RevokeUriPermissionManually(const Uri &uri, const std::string &bundleName) = 0;

    /**
     * @brief Clear user's uri authorization record, size of uri list must be no more than 500.
     *
     *
     * @param uriVec The file uri list.
     * @param BundleName A BundleName of an application.
     * @return Returns ERR_OK if at least one uri permission record is revoked.
     */
    virtual int32_t RevokeUriPermissionManually(const std::vector<Uri> &uriVec, const std::string &bundleName) = 0;

    /**
     * @brief verify if tokenId have uri permission of flag, including temporary permission and persistable permission
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    virtual bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId) = 0;

    virtual bool IsAuthorizationUriAllowed(uint32_t fromTokenId) = 0;

    enum UriPermMgrCmd {
        // ipc id for GrantUriPermission
        ON_GRANT_URI_PERMISSION = 0,

        // ipc id for RevokeUriPermission
        ON_REVOKE_URI_PERMISSION,

        // ipc id for RevokeAllUriPermission
        ON_REVOKE_ALL_URI_PERMISSION,

        ON_REVOKE_URI_PERMISSION_MANUALLY,

        // ipc id for VerifyUriPermission
        ON_VERIFY_URI_PERMISSION,

        // ipc id for BatchGrantUriPermission
        ON_BATCH_GRANT_URI_PERMISSION,

        // ipc id for BatchGrantUriPermissionFor2In1
        ON_BATCH_GRANT_URI_PERMISSION_FOR_2_IN_1,

        //ipc id for IsAuthorizationUriAllowed
        ON_IS_Authorization_URI_ALLOWED,
        
        // ipc id for Batch RevokeUriPermissionManually
        ON_BATCH_REVOKE_URI_PERMISSION_MANUALLY
    };
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_INTERFACE_H
