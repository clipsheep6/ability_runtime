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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H

#include <functional>

#include "uri.h"
#include "uri_permission_manager_interface.h"

namespace OHOS {
namespace AAFwk {
using ClearProxyCallback = std::function<void()>;
class UriPermissionManagerClient {
public:
    static UriPermissionManagerClient& GetInstance();
    ~UriPermissionManagerClient() = default;

    /**
     * @brief Authorize the uri permission of to targetBundleName.
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     */
    int GrantUriPermission(const Uri &uri, unsigned int flag, const std::string targetBundleName, int32_t appIndex = 0,
        uint32_t initiatorTokenId = 0);

    /**
     * @brief Authorize the uri permission of to targetBundleName.
     *
     * @param uriVec The file uri list.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     */
    int GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0);

    /**
     * @brief Authorize the uri permission to targetBundleName for 2in1, only supports AbilityManagerService calls.
     *
     * @param uriVec The file urilist.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param targetBundleName The user of uri.
     * @param appIndex The index of application in sandbox.
     * @param isSystemAppCall The flag of system application called.
     * @return Returns true if the authorization is successful, otherwise returns false.
     */
    int GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string &targetBundleName, int32_t appIndex = 0, bool isSystemAppCall = false);

    /**
     * @brief Clear user's uri authorization record with auto remove flag.
     *
     * @param tokenId A tokenId of an application.
     */
    void RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId);

    /**
     * @brief Clear user's all uri authorization record with auto remove flag.
     *
     * @param tokenId A tokenId of an application.
     */
    int RevokeAllUriPermissions(const Security::AccessToken::AccessTokenID tokenId);

    /**
     * @brief Clear user's uri authorization record.
     *
     * @param uri The file uri.
     * @param BundleName A BundleName of an application.
     */
    int RevokeUriPermissionManually(const Uri &uri, const std::string &bundleName);

    /**
     * @brief Clear user's uri authorization record, size of uri list must be no more than 500.
     *
     *
     * @param uriVec The file uri list.
     * @param BundleName A BundleName of an application.
     * @return Returns ERR_OK if at least one uri permission record is revoked.
     */
    int32_t RevokeUriPermissionManually(const std::vector<Uri> &uriVec, const std::string &bundleName);

    /**
     * @brief verify if tokenId have uri permission of flag, including temporary permission and persistable permission
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId);

    bool IsAuthorizationUriAllowed(uint32_t fromTokenId);

    void OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void OnLoadSystemAbilityFail();
private:
    UriPermissionManagerClient() = default;
    sptr<IUriPermissionManager> ConnectUriPermService();
    void ClearProxy();
    bool LoadUriPermService();
    void SetUriPermMgr(const sptr<IRemoteObject> &remoteObject);
    sptr<IUriPermissionManager> GetUriPermMgr();
    DISALLOW_COPY_AND_MOVE(UriPermissionManagerClient);

    class UpmsDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit UpmsDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        ~UpmsDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    std::mutex saLoadMutex_;
    std::condition_variable loadSaVariable_;
    bool saLoadFinished_ = false;
    sptr<IUriPermissionManager> uriPermMgr_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
