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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_PROXY_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_PROXY_H

#include<vector>
#include "iremote_proxy.h"
#include "uri_permission_manager_interface.h"

namespace OHOS {
namespace AAFwk {
class UriPermissionManagerProxy : public IRemoteProxy<IUriPermissionManager> {
public:
    explicit UriPermissionManagerProxy(const sptr<IRemoteObject> &impl);
    virtual ~UriPermissionManagerProxy() = default;

    virtual int GrantUriPermission(const Uri &uri, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t abilityId = -1) override;

    virtual int GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
        const std::string targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t abilityId = -1) override;

    virtual int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0) override;
    virtual void RevokeUriPermission(const uint32_t tokenId, int32_t abilityId) override;
    virtual int RevokeAllUriPermissions(const uint32_t tokenId) override;
    virtual int RevokeUriPermissionManually(const Uri &uri, const std::string bundleName,
        int32_t appIndex = 0) override;
    virtual bool VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId) override;
    virtual std::vector<bool> CheckUriAuthorization(const std::vector<std::string> &uriVec,
        uint32_t flag, uint32_t tokenId) override;

private:
    static inline BrokerDelegator<UriPermissionManagerProxy> delegator_;
    int32_t SendTransactCmd(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_PROXY_H
