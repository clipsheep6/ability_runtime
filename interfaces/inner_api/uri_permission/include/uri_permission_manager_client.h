/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
     * @param autoremove the uri is temperarily or not
     */
    int GrantUriPermission(const Uri &uri, unsigned int flag,
        const std::string targetBundleName, int autoremove, int32_t appIndex = 0);

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
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName);

    /**
     * @brief check if caller can grant persistable uri permission
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    bool CheckPersistableUriPermissionProxy(const Uri& uri, uint32_t flag, uint32_t tokenId);

    /**
     * @brief verify if tokenId have uri permission of flag, including temporary permission and persistable permission
     *
     * @param uri The file uri.
     * @param flag Want::FLAG_AUTH_READ_URI_PERMISSION or Want::FLAG_AUTH_WRITE_URI_PERMISSION.
     * @param tokenId A tokenId of an application.
     */
    bool VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId);

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
