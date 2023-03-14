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

#include "uri_permission_manager_client.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
const int32_t INTERNAL_ERROR = 16000050;
int UriPermissionManagerClient::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int autoremove)
{
    HILOG_DEBUG("UriPermissionManagerClient::GrantUriPermission is called.");
    HILOG_DEBUG("targetBundleName :%{public}s", targetBundleName.c_str());
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->GrantUriPermission(uri, flag, targetBundleName, autoremove);
    }
    return INTERNAL_ERROR;
}

void UriPermissionManagerClient::RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("UriPermissionManagerClient::RevokeUriPermission is called.");
    auto uriPermMgr = ConnectUriPermService();
    uriPermMgr->RevokeUriPermission(tokenId);
    return;
}

int UriPermissionManagerClient::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    HILOG_DEBUG("UriPermissionManagerClient::RevokeUriPermissionManually is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeUriPermissionManually(uri, bundleName);
    }
    return INTERNAL_ERROR;
}

sptr<IUriPermissionManager> UriPermissionManagerClient::ConnectUriPermService()
{
    HILOG_DEBUG("UriPermissionManagerClient::ConnectUriPermService is called.");
    std::lock_guard<std::mutex> lock(mutex_);
    if (uriPermMgr_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityMgr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(URI_PERMISSION_MGR_SERVICE_ID);
        if (!remoteObj || (uriPermMgr_ = iface_cast<IUriPermissionManager>(remoteObj)) == nullptr) {
            HILOG_ERROR("Failed to get UriPermService.");
            return nullptr;
        }
        auto self = shared_from_this();
        const auto& onClearProxyCallback = [self] {
            if (self) {
                self->ClearProxy();
            }
        };
        sptr<UpmsDeathRecipient> recipient(new UpmsDeathRecipient(onClearProxyCallback));
        uriPermMgr_->AsObject()->AddDeathRecipient(recipient);
    }
    HILOG_DEBUG("End UriPermissionManagerClient::ConnectUriPermService.");
    return uriPermMgr_;
}

void UriPermissionManagerClient::ClearProxy()
{
    HILOG_DEBUG("UriPermissionManagerClient::ClearProxy is called.");
    std::lock_guard<std::mutex> lock(mutex_);
    uriPermMgr_ = nullptr;
}

void UriPermissionManagerClient::UpmsDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    HILOG_ERROR("upms stub died.");
    proxy_();
}
}  // namespace AAFwk
}  // namespace OHOS
