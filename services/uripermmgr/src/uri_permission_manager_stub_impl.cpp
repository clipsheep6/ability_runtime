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
#include "singleton.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
const int32_t DEFAULT_USER_ID = 0;
const int32_t ERR_OK = 0;
using TokenId = Security::AccessToken::AccessTokenID;

int UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int autoremove)
{
    HILOG_DEBUG("UriPermissionManagerStubImpl::GrantUriPermission is called.");
    // reject sandbox to grant uri permission
    auto appMgrProxy = ConnectAppMgr();
    if (appMgrProxy == nullptr) {
        HILOG_ERROR("ConnectAppMgr failed");
        return INNER_ERR;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    bool isSandbox = false;
    auto ret = appMgrProxy->JudgeSandboxByPid(callerPid, isSandbox);
    if (ret != ERR_OK) {
        HILOG_ERROR("JudgeSandboxByPid failed.");
        return INNER_ERR;
    }
    if (isSandbox) {
        HILOG_ERROR("Sandbox can not grant uri permission.");
        return CHECK_PERMISSION_FAILED;
    }

    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: The param flag is invalid.");
        return ERR_CODE_INVALID_URI_FLAG;
    }
    Uri uri_inner = uri;
    auto&& authority = uri_inner.GetAuthority();
    Security::AccessToken::AccessTokenID fromTokenId = GetTokenIdByBundleName(authority);
    Security::AccessToken::AccessTokenID targetTokenId = GetTokenIdByBundleName(targetBundleName);
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
    Security::AccessToken::AccessTokenID fromTokenId,
    Security::AccessToken::AccessTokenID targetTokenId, int autoremove)
{
    auto storageMgrProxy = ConnectStorageManager();
    if (storageMgrProxy == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return INNER_ERR;
    }
    auto uriStr = uri.ToString();
    auto ret = storageMgrProxy->CreateShareFile(uriStr, targetTokenId, flag);
    if (ret != 0 && ret != -EEXIST) {
        HILOG_ERROR("storageMgrProxy failed to CreateShareFile.");
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

    auto storageMgrProxy = ConnectStorageManager();
    if (storageMgrProxy == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return;
    }

    if (!uriList.empty()) {
        storageMgrProxy->DeleteShareFile(tokenId, uriList);
    }
}

void UriPermissionManagerStubImpl::RevokeAllUriPermissions(int tokenId)
{
    HILOG_DEBUG("Start to remove all uri permission for uninstalled app.");
    std::map<unsigned int, std::vector<std::string>> uriLists;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end();) {
                if (it->targetTokenId == tokenId || it->fromTokenId == tokenId) {
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

    auto storageMgrProxy = ConnectStorageManager();
    if (storageMgrProxy == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return;
    }

    if (!uriLists.empty()) {
        for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
            storageMgrProxy->DeleteShareFile(iter->first, iter->second);
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
    Security::AccessToken::AccessTokenID uriTokenId = GetTokenIdByBundleName(authority);
    Security::AccessToken::AccessTokenID tokenId = GetTokenIdByBundleName(bundleName);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto permission = PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    if (!permission && (uriTokenId != callerTokenId) && (tokenId != callerTokenId)) {
        HILOG_WARN("UriPermissionManagerStubImpl::RevokeUriPermission: No permission for revoke uri.");
        return CHECK_PERMISSION_FAILED;
    }

    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);

        auto uriStr = uri.ToString();
        auto search = uriMap_.find(uriStr);
        if (search == uriMap_.end()) {
            HILOG_INFO("URI does not exist on uri map.");
            return ERR_OK;
        }
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if (it->targetTokenId == tokenId) {
                HILOG_INFO("Erase an info form list.");
                auto storageMgrProxy = ConnectStorageManager();
                if (storageMgrProxy == nullptr) {
                    HILOG_ERROR("ConnectStorageManager failed");
                    return INNER_ERR;
                }
                uriList.emplace_back(search->first);
                if (storageMgrProxy->DeleteShareFile(tokenId, uriList) == ERR_OK) {
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
    }
    return ERR_OK;
}

sptr<AppExecFwk::IAppMgr> UriPermissionManagerStubImpl::ConnectAppMgr()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(appMgrMutex_);
    if (appMgr_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityMgr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(APP_MGR_SERVICE_ID);
        if (!remoteObj || (appMgr_ = iface_cast<AppExecFwk::IAppMgr>(remoteObj)) == nullptr) {
            HILOG_ERROR("Failed to get AppMgrService.");
            return nullptr;
        }
        auto self = weak_from_this();
        const auto& onClearProxyCallback = [self](const wptr<IRemoteObject>& remote) {
            auto impl = self.lock();
            if (impl && impl->appMgr_ == remote) {
                impl->ClearAppMgrProxy();
            }
        };
        sptr<ProxyDeathRecipient> recipient(new ProxyDeathRecipient(onClearProxyCallback));
        if (!appMgr_->AsObject()->AddDeathRecipient(recipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return appMgr_;
}

sptr<AppExecFwk::IBundleMgr> UriPermissionManagerStubImpl::ConnectBundleManager()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(bmsMutex_);
    if (bundleManager_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityMgr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
        if (!remoteObj || (bundleManager_ = iface_cast<AppExecFwk::IBundleMgr>(remoteObj)) == nullptr) {
            HILOG_ERROR("Failed to get bms.");
            return nullptr;
        }
        auto self = weak_from_this();
        const auto& onClearProxyCallback = [self](const wptr<IRemoteObject>& remote) {
            auto impl = self.lock();
            if (impl && impl->bundleManager_ == remote) {
                impl->ClearBMSProxy();
            }
        };
        sptr<ProxyDeathRecipient> recipient(new ProxyDeathRecipient(onClearProxyCallback));
        if (!bundleManager_->AsObject()->AddDeathRecipient(recipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return bundleManager_;
}

Security::AccessToken::AccessTokenID UriPermissionManagerStubImpl::GetTokenIdByBundleName(const std::string bundleName)
{
    auto bms = ConnectBundleManager();
    if (bms == nullptr) {
        HILOG_WARN("Failed to get bms.");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    AppExecFwk::BundleInfo bundleInfo;
    if (!IN_PROCESS_CALL(bms->GetBundleInfo(bundleName, bundleFlag, bundleInfo, GetCurrentAccountId()))) {
        HILOG_WARN("To fail to get bundle info according to uri.");
        return GET_BUNDLE_INFO_FAILED;
    }
    return bundleInfo.applicationInfo.accessTokenId;
}

sptr<StorageManager::IStorageManager> UriPermissionManagerStubImpl::ConnectStorageManager()
{
    std::lock_guard<std::mutex> lock(storageMutex_);
    if (storageManager_ == nullptr) {
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityMgr) {
            HILOG_ERROR("Failed to get SystemAbilityManager.");
            return nullptr;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (!remoteObj || (storageManager_ = iface_cast<StorageManager::IStorageManager>(remoteObj)) == nullptr) {
            HILOG_ERROR("Failed to get storage manager.");
            return nullptr;
        }
        auto self = weak_from_this();
        const auto& onClearProxyCallback = [self](const wptr<IRemoteObject>& remote) {
            auto impl = self.lock();
            if (impl && impl->storageManager_ == remote) {
                impl->ClearSMProxy();
            }
        };
        sptr<ProxyDeathRecipient> recipient(new ProxyDeathRecipient(onClearProxyCallback));
        if (!storageManager_->AsObject()->AddDeathRecipient(recipient)) {
            HILOG_ERROR("AddDeathRecipient failed.");
        }
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return storageManager_;
}

void UriPermissionManagerStubImpl::ClearAppMgrProxy()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(appMgrMutex_);
    appMgr_ = nullptr;
}

void UriPermissionManagerStubImpl::ClearBMSProxy()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(bmsMutex_);
    bundleManager_ = nullptr;
}

void UriPermissionManagerStubImpl::ClearSMProxy()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(bmsMutex_);
    storageManager_ = nullptr;
}

void UriPermissionManagerStubImpl::ProxyDeathRecipient::OnRemoteDied(
    [[maybe_unused]] const wptr<IRemoteObject>& remote)
{
    if (proxy_) {
        HILOG_DEBUG("%{public}s, bms stub died.", __func__);
        proxy_(remote);
    }
}

int UriPermissionManagerStubImpl::GetCurrentAccountId()
{
    std::vector<int32_t> osActiveAccountIds;
    ErrCode ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
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