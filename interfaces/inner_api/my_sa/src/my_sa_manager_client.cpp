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

#include "my_sa_manager_client.h"

#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
void MySaManagerClient::GrantUriPermission(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID fromTokenId, const Security::AccessToken::AccessTokenID targetTokenId)
{
    HILOG_DEBUG("MySaManagerClient::GrantUriPermission is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        uriPermMgr->GrantUriPermission(uri, flag, fromTokenId, targetTokenId);
    }
}

bool MySaManagerClient::VerifyUriPermission(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("MySaManagerClient::VerifyUriPermission is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->VerifyUriPermission(uri, flag, tokenId);
    }
    return false;
}

void MySaManagerClient::RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("MySaManagerClient::RemoveUriPermission is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        uriPermMgr->RemoveUriPermission(tokenId);
    }
}

sptr<IMySaManager> MySaManagerClient::ConnectUriPermService()
{
    HILOG_DEBUG("MySaManagerClient::ConnectUriPermService is called.");
    std::lock_guard<std::mutex> lock(mutex_);
    if (uriPermMgr_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityMgr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(URI_PERMISSION_MGR_SERVICE_ID);
        if (!remoteObj || (uriPermMgr_ = iface_cast<IMySaManager>(remoteObj)) == nullptr) {
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
    HILOG_DEBUG("End MySaManagerClient::ConnectUriPermService.");
    return uriPermMgr_;
}

void MySaManagerClient::ClearProxy()
{
    HILOG_DEBUG("MySaManagerClient::ClearProxy is called.");
    std::lock_guard<std::mutex> lock(mutex_);
    uriPermMgr_ = nullptr;
}

void MySaManagerClient::UpmsDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    HILOG_ERROR("upms stub died.");
    proxy_();
}
}  // namespace AAFwk
}  // namespace OHOS
