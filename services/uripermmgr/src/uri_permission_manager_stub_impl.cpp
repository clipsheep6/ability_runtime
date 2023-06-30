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

#include "uri_permission_manager_stub_impl.h"

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t ERR_OK = 0;
}

void UriPermissionManagerStubImpl::Init()
{
    // Register UriBundleEventCallback to receive hap updates
    HILOG_INFO("Register UriBundleEventCallback to receive hap updates.");
    ConnectManager(bundleManager_, BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleManager_ ==  nullptr) {
        HILOG_ERROR("Get BundleManager failed!");
        return;
    }
    uriBundleEventCallback_ = new UriBundleEventCallback(this);
    auto ret = bundleManager_->RegisterBundleEventCallback(uriBundleEventCallback_);
    if (!ret) {
        HILOG_ERROR("RegisterBundleEventCallback failed!");
    }
}

void UriPermissionManagerStubImpl::Stop() const
{
    if (uriBundleEventCallback_ != nullptr) {
        auto ret = bundleManager_->UnregisterBundleEventCallback(uriBundleEventCallback_);
        if (!ret) {
            HILOG_ERROR("UnregisterBundleEventCallback failed!");
        }
    }
}

int UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int autoremove, int32_t appIndex)
{
    HILOG_DEBUG("CALL: appIndex is %{public}d.", appIndex);
    // reject sandbox to grant uri permission
    ConnectManager(appMgr_, APP_MGR_SERVICE_ID);
    if (appMgr_ == nullptr) {
        HILOG_ERROR("Get BundleManager failed!");
        return INNER_ERR;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    bool isSandbox = false;
    auto ret = appMgr_->JudgeSandboxByPid(callerPid, isSandbox);
    if (ret != ERR_OK) {
        HILOG_ERROR("JudgeSandboxByPid failed.");
        return INNER_ERR;
    }
    if (isSandbox) {
        HILOG_ERROR("Sandbox application can not grant URI permission.");
        return ERR_CODE_GRANT_URI_PERMISSION;
    }

    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: The param flag is invalid.");
        return ERR_CODE_INVALID_URI_FLAG;
    }
    Uri uri_inner = uri;
    auto&& authority = uri_inner.GetAuthority();
    auto fromTokenId = GetTokenIdByBundleName(authority, 0);
    auto targetTokenId = GetTokenIdByBundleName(targetBundleName, appIndex);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto permission = PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    if (!permission && (fromTokenId != callerTokenId)) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: No permission for proxy authorization uri.");
        return CHECK_PERMISSION_FAILED;
    }
    unsigned int tmpFlag = 0;
    if (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
        tmpFlag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    } else {
        tmpFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    }
    auto&& scheme = uri_inner.GetScheme();
    if (scheme != "file") {
        HILOG_WARN("only support file uri.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    // auto remove URI permission for clipboard
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    HILOG_DEBUG("callerprocessName : %{public}s", nativeInfo.processName.c_str());
    if (nativeInfo.processName == "pasteboard_serv") {
        autoremove = 1;
    }
    return GrantUriPermissionImpl(uri, tmpFlag, fromTokenId, targetTokenId, autoremove);
}

int UriPermissionManagerStubImpl::GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
    TokenId fromTokenId, TokenId targetTokenId, int autoremove)
{
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectManager failed");
        return INNER_ERR;
    }
    auto uriStr = uri.ToString();
    auto ret = storageManager_->CreateShareFile(uriStr, targetTokenId, flag);
    if (ret != 0 && ret != -EEXIST) {
        HILOG_ERROR("failed to CreateShareFile.");
        return INNER_ERR;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    GrantInfo info = { flag, fromTokenId, targetTokenId, autoremove };
    if (search == uriMap_.end()) {
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uriStr, infoList);
        return ERR_OK;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            if ((flag & item.flag) == 0) {
                HILOG_INFO("Update uri r/w permission.");
                item.flag = flag;
            }
            HILOG_INFO("uri permission has granted, not to grant again.");
            return ERR_OK;
        }
    }
    infoList.emplace_back(info);
    return ERR_OK;
}

void UriPermissionManagerStubImpl::RevokeUriPermission(const TokenId tokenId)
{
    HILOG_DEBUG("Start to remove uri permission.");
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    HILOG_DEBUG("callerprocessName : %{public}s", nativeInfo.processName.c_str());
    if (nativeInfo.processName != "foundation") {
        HILOG_ERROR("RevokeUriPermission can only be called by foundation");
        return;
    }
    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end(); it++) {
                if (it->targetTokenId == tokenId && it->autoremove) {
                    HILOG_INFO("Erase an info form list.");
                    list.erase(it);
                    uriList.emplace_back(iter->first);
                    break;
                }
            }
            if (list.size() == 0) {
                uriMap_.erase(iter++);
            } else {
                iter++;
            }
        }
    }

    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectManager failed");
        return;
    }

    if (!uriList.empty()) {
        storageManager_->DeleteShareFile(tokenId, uriList);
    }
}

void UriPermissionManagerStubImpl::RevokeAllUriPermissions(uint32_t tokenId)
{
    HILOG_DEBUG("Start to remove all uri permission for uninstalled app.");
    std::map<unsigned int, std::vector<std::string>> uriLists;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end();) {
                if (it->targetTokenId == static_cast<uint32_t>(tokenId) ||
                    it->fromTokenId == static_cast<uint32_t>(tokenId)) {
                        HILOG_INFO("Erase an info form list.");
                        uriLists[it->targetTokenId].emplace_back(iter->first);
                        list.erase(it++);
                } else {
                    it++;
                }
            }
            if (list.size() == 0) {
                uriMap_.erase(iter++);
            } else {
                iter++;
            }
        }
    }

    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return;
    }

    if (!uriLists.empty()) {
        for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
            storageManager_->DeleteShareFile(iter->first, iter->second);
        }
    }
}

int UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    HILOG_DEBUG("Start to remove uri permission manually.");
    Uri uri_inner = uri;
    auto&& authority = uri_inner.GetAuthority();
    auto&& scheme = uri_inner.GetScheme();
    if (scheme != "file") {
        HILOG_WARN("only support file uri.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto uriTokenId = GetTokenIdByBundleName(authority, 0);
    auto tokenId = GetTokenIdByBundleName(bundleName, 0);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto permission = PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    if (!permission && (uriTokenId != callerTokenId) && (tokenId != callerTokenId)) {
        HILOG_WARN("UriPermissionManagerStubImpl::RevokeUriPermission: No permission for revoke uri.");
        return CHECK_PERMISSION_FAILED;
    }

    std::vector<std::string> uriList;
    auto uriStr = uri.ToString();
    std::lock_guard<std::mutex> guard(mutex_);

    auto search = uriMap_.find(uriStr);
    if (search == uriMap_.end()) {
        HILOG_INFO("URI does not exist on uri map.");
        return ERR_OK;
    }
    auto& list = search->second;
    for (auto it = list.begin(); it != list.end(); it++) {
        if (it->targetTokenId == tokenId) {
            HILOG_INFO("Erase an info form list.");
            ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
            if (storageManager_ == nullptr) {
                HILOG_ERROR("ConnectStorageManager failed");
                return INNER_ERR;
            }
            uriList.emplace_back(search->first);
            if (storageManager_->DeleteShareFile(tokenId, uriList) == ERR_OK) {
                list.erase(it);
                break;
            } else {
                HILOG_ERROR("DeleteShareFile failed");
                return INNER_ERR;
            }
        }
    }
    if (list.size() == 0) {
        uriMap_.erase(search);
    }
    return ERR_OK;
}

template<typename T>
void UriPermissionManagerStubImpl::ConnectManager(sptr<T> &mgr, int32_t serviceId)
{
    HILOG_DEBUG("Call.");
    std::lock_guard<std::mutex> lock(mgrMutex_);
    if (mgr == nullptr) {
        HILOG_ERROR("mgr is nullptr.");
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(serviceId);
        if (remoteObj == nullptr) {
            HILOG_ERROR("Failed to get mgr.");
            return;
        }
        HILOG_ERROR("to cast.");
        mgr = iface_cast<T>(remoteObj);
        if (mgr == nullptr) {
            HILOG_ERROR("Failed to cast.");
            return;
        }
        wptr<T> manager = mgr;
        auto self = weak_from_this();
        auto onClearProxyCallback = [manager, self](const auto& remote) {
            auto mgrSptr = manager.promote();
            auto impl = self.lock();
            if (impl && mgrSptr && mgrSptr->AsObject() == remote.promote()) {
                std::lock_guard<std::mutex> lock(impl->mgrMutex_);
                mgrSptr.clear();
            }
        };
        sptr<ProxyDeathRecipient> recipient(new ProxyDeathRecipient(std::move(onClearProxyCallback)));
        if (!mgr->AsObject()->AddDeathRecipient(recipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
    }
}

uint32_t UriPermissionManagerStubImpl::GetTokenIdByBundleName(const std::string bundleName, int32_t appIndex)
{
    ConnectManager(bundleManager_, BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleManager_ == nullptr) {
        HILOG_WARN("Failed to get bms.");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    AppExecFwk::BundleInfo bundleInfo;
    auto userId = GetCurrentAccountId();
    if (appIndex == 0) {
        if (!IN_PROCESS_CALL(bundleManager_->GetBundleInfo(bundleName, bundleFlag, bundleInfo, userId))) {
            HILOG_WARN("Failed to get bundle info according to uri.");
            return GET_BUNDLE_INFO_FAILED;
        }
    } else {
        if (IN_PROCESS_CALL(bundleManager_->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo) != ERR_OK)) {
            HILOG_WARN("Failed to get bundle info according to appIndex.");
            return GET_BUNDLE_INFO_FAILED;
        }
    }
    return bundleInfo.applicationInfo.accessTokenId;
}

void UriPermissionManagerStubImpl::ProxyDeathRecipient::OnRemoteDied(
    [[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    if (proxy_) {
        HILOG_DEBUG("mgr stub died.");
        proxy_(remote);
    }
}

int32_t UriPermissionManagerStubImpl::GetCurrentAccountId() const
{
    std::vector<int32_t> osActiveAccountIds;
    auto ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        HILOG_ERROR("QueryActiveOsAccountIds failed.");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        HILOG_ERROR("%{public}s, QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}
}  // namespace AAFwk
}  // namespace OHOS