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

#include "uri_permission_manager_client.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "uri_permission_load_callback.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int LOAD_SA_TIMEOUT_MS = 4 * 1000;
const int MAX_URI_COUNT = 500;
} // namespace
UriPermissionManagerClient& UriPermissionManagerClient::GetInstance()
{
    static UriPermissionManagerClient instance;
    return instance;
}

int UriPermissionManagerClient::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("targetBundleName :%{public}s", targetBundleName.c_str());
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->GrantUriPermission(uri, flag, targetBundleName, appIndex, initiatorTokenId);
    }
    return INNER_ERR;
}

int UriPermissionManagerClient::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("targetBundleName: %{public}s, uriVec size: %{public}zu", targetBundleName.c_str(), uriVec.size());
    if (uriVec.size() == 0 || uriVec.size() > MAX_URI_COUNT) {
        HILOG_ERROR("The size of uriVec should be between 1 and %{public}i.", MAX_URI_COUNT);
        return INNER_ERR;
    }
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);
    }
    return INNER_ERR;
}

int UriPermissionManagerClient::GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
{
    HILOG_DEBUG("targetBundleName: %{public}s, uriVec size: %{public}zu", targetBundleName.c_str(), uriVec.size());
    if (uriVec.size() == 0 || uriVec.size() > MAX_URI_COUNT) {
        HILOG_ERROR("The size of uriVec should be between 1 and %{public}i.", MAX_URI_COUNT);
        return INNER_ERR;
    }
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr != nullptr) {
        return uriPermMgr->GrantUriPermissionFor2In1(uriVec, flag, targetBundleName, appIndex, isSystemAppCall);
    }
    return INNER_ERR;
}

void UriPermissionManagerClient::RevokeUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("UriPermissionManagerClient::RevokeUriPermission is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeUriPermission(tokenId);
    }
}

int UriPermissionManagerClient::RevokeAllUriPermissions(const Security::AccessToken::AccessTokenID tokenId)
{
    HILOG_DEBUG("UriPermissionManagerClient::RevokeAllUriPermissions is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeAllUriPermissions(tokenId);
    }
    return INNER_ERR;
}

int UriPermissionManagerClient::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    HILOG_DEBUG("UriPermissionManagerClient::RevokeUriPermissionManually is called.");
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->RevokeUriPermissionManually(uri, bundleName);
    }
    return INNER_ERR;
}

bool UriPermissionManagerClient::VerifyUriPermission(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->VerifyUriPermission(uri, flag, tokenId);
    }
    return false;
}

bool UriPermissionManagerClient::IsAuthorizationUriAllowed(uint32_t fromTokenId)
{
    auto uriPermMgr = ConnectUriPermService();
    if (uriPermMgr) {
        return uriPermMgr->IsAuthorizationUriAllowed(fromTokenId);
    }
    return false;
}

sptr<IUriPermissionManager> UriPermissionManagerClient::ConnectUriPermService()
{
    HILOG_DEBUG("UriPermissionManagerClient::ConnectUriPermService is called.");
    auto uriPermMgr = GetUriPermMgr();
    if (uriPermMgr == nullptr) {
        if (!LoadUriPermService()) {
            HILOG_ERROR("Load uri permission manager service failed.");
            return nullptr;
        }
        uriPermMgr = GetUriPermMgr();
        if (uriPermMgr == nullptr || uriPermMgr->AsObject() == nullptr) {
            HILOG_ERROR("Failed to get uri permission manager.");
            return nullptr;
        }
        const auto& onClearProxyCallback = [] {
            UriPermissionManagerClient::GetInstance().ClearProxy();
        };
        sptr<UpmsDeathRecipient> recipient(new UpmsDeathRecipient(onClearProxyCallback));
        uriPermMgr->AsObject()->AddDeathRecipient(recipient);
    }
    HILOG_DEBUG("End UriPermissionManagerClient::ConnectUriPermService.");
    return uriPermMgr;
}

bool UriPermissionManagerClient::LoadUriPermService()
{
    HILOG_DEBUG("UriPermissionManagerClient::LoadUriPermService is called.");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        HILOG_ERROR("Failed to get SystemAbilityManager.");
        return false;
    }

    sptr<UriPermissionLoadCallback> loadCallback = new (std::nothrow) UriPermissionLoadCallback();
    if (loadCallback == nullptr) {
        HILOG_ERROR("Create load callback failed.");
        return false;
    }

    auto ret = systemAbilityMgr->LoadSystemAbility(URI_PERMISSION_MGR_SERVICE_ID, loadCallback);
    if (ret != 0) {
        HILOG_ERROR("Load system ability %{public}d failed with %{public}d.", URI_PERMISSION_MGR_SERVICE_ID, ret);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(saLoadMutex_);
        auto waitStatus = loadSaVariable_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return saLoadFinished_;
            });
        if (!waitStatus) {
            HILOG_ERROR("Wait for load sa timeout.");
            return false;
        }
    }
    return true;
}

sptr<IUriPermissionManager> UriPermissionManagerClient::GetUriPermMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return uriPermMgr_;
}

void UriPermissionManagerClient::SetUriPermMgr(const sptr<IRemoteObject> &remoteObject)
{
    HILOG_DEBUG("UriPermissionManagerClient::SetUriPermMgr is called.");
    std::lock_guard<std::mutex> lock(mutex_);
    uriPermMgr_ = iface_cast<IUriPermissionManager>(remoteObject);
}

void UriPermissionManagerClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    HILOG_DEBUG("UriPermissionManagerClient::OnLoadSystemAbilitySuccess is called.");
    SetUriPermMgr(remoteObject);
    std::unique_lock<std::mutex> lock(saLoadMutex_);
    saLoadFinished_ = true;
    loadSaVariable_.notify_one();
}

void UriPermissionManagerClient::OnLoadSystemAbilityFail()
{
    HILOG_DEBUG("UriPermissionManagerClient::OnLoadSystemAbilityFail is called.");
    SetUriPermMgr(nullptr);
    std::unique_lock<std::mutex> lock(saLoadMutex_);
    saLoadFinished_ = true;
    loadSaVariable_.notify_one();
}

void UriPermissionManagerClient::ClearProxy()
{
    HILOG_DEBUG("UriPermissionManagerClient::ClearProxy is called.");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        uriPermMgr_ = nullptr;
    }
    std::unique_lock<std::mutex> lock(saLoadMutex_);
    saLoadFinished_ = false;
}

void UriPermissionManagerClient::UpmsDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    HILOG_ERROR("upms stub died.");
    proxy_();
}
}  // namespace AAFwk
}  // namespace OHOS
