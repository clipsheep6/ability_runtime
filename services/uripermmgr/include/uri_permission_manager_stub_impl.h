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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H

#include <functional>
#include <map>

#include "app_mgr_interface.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "storage_manager_proxy.h"
#include "istorage_manager.h"
#include "uri.h"
#include "uri_permission_manager_stub.h"

namespace OHOS {
namespace AAFwk {
using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;

struct GrantInfo {
    unsigned int flag;
    const unsigned int fromTokenId;
    const unsigned int targetTokenId;
    int autoremove;
};
class UriPermissionManagerStubImpl : public UriPermissionManagerStub,
                                     public std::enable_shared_from_this<UriPermissionManagerStubImpl> {
public:
    UriPermissionManagerStubImpl() = default;
    virtual ~UriPermissionManagerStubImpl() = default;

    int GrantUriPermission(const Uri &uri, unsigned int flag,
        const std::string targetBundleName, int autoremove) override;

    void RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId) override;
    void RevokeAllUriPermissions(int tokenId);
    int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName) override;
    sptr<AppExecFwk::IBundleMgr> ConnectBundleManager();

private:
    sptr<AppExecFwk::IAppMgr> ConnectAppMgr();
    sptr<StorageManager::IStorageManager> ConnectStorageManager();
    int GetCurrentAccountId();
    void ClearAppMgrProxy();
    void ClearBMSProxy();
    void ClearSMProxy();
    int GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
        Security::AccessToken::AccessTokenID fromTokenId,
        Security::AccessToken::AccessTokenID targetTokenId, int autoremove);
    Security::AccessToken::AccessTokenID GetTokenIdByBundleName(const std::string bundleName);

    class ProxyDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit ProxyDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        ~ProxyDeathRecipient() = default;
        virtual void OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::map<std::string, std::list<GrantInfo>> uriMap_;
    std::mutex mutex_;
    std::mutex appMgrMutex_;
    std::mutex bmsMutex_;
    std::mutex storageMutex_;
    sptr<AppExecFwk::IAppMgr> appMgr_ = nullptr;
    sptr<AppExecFwk::IBundleMgr> bundleManager_ = nullptr;
    sptr<StorageManager::IStorageManager> storageManager_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_STUB_IMPL_H
