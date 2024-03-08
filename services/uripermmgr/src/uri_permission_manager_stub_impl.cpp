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
#include "parameter.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t ERR_OK = 0;
const char* GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
constexpr int32_t GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_SIZE = 6;
}

void UriPermissionManagerStubImpl::Init()
{
    uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    InitPersistableUriPermissionConfig();
}

bool UriPermissionManagerStubImpl::CheckPersistableUriPermissionProxy(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    // check if caller can grant persistable uri permission
    auto uriStr = uri.ToString();
    return uriPermissionRdb_->CheckPersistableUriPermissionProxy(uriStr, flag, tokenId);
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
{
    // verify if tokenId have uri permission of flag, including temporary permission and persistable permission
    HILOG_DEBUG("VerifyUriPermission called: flag = %{public}i", static_cast<int>(flag));
    auto uriStr = uri.ToString();
    bool tempPermission = false;
    bool perPermission = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto search = uriMap_.find(uriStr);
        if (search != uriMap_.end()) {
            auto& list = search->second;
            for (auto it = list.begin(); it != list.end(); it++) {
                bool condition = (it->targetTokenId == tokenId) &&
                    ((it->flag | Want::FLAG_AUTH_READ_URI_PERMISSION) & flag) != 0;
                if (condition) {
                    HILOG_DEBUG("temporary uri permission exists");
                    tempPermission = true;
                    break;
                }
            }
        }
    }
    if (uriPermissionRdb_->CheckPersistableUriPermissionProxy(uriStr, flag, tokenId)) {
        HILOG_DEBUG("persistable uri permission exists");
        tempPermission = true;
    }
    if (!tempPermission && !perPermission) {
        HILOG_DEBUG("uri permission not exists");
        return false;
    }
    return true;
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
    auto&& scheme = uri_inner.GetScheme();
    if (scheme != "file") {
        HILOG_WARN("only support file uri.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto&& authority = uri_inner.GetAuthority();
    auto fromTokenId = GetTokenIdByBundleName(authority, 0);
    auto targetTokenId = GetTokenIdByBundleName(targetBundleName, appIndex);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    unsigned int tmpFlag = 0;
    ret = GetUriPermissionFlag(uri, flag, fromTokenId, targetTokenId, tmpFlag);
    if (ret != ERR_OK || tmpFlag == 0) {
        return ret;
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

int UriPermissionManagerStubImpl::GetUriPermissionFlag(const Uri &uri, unsigned int flag,
    uint32_t fromTokenId, uint32_t targetTokenId, unsigned int &newFlag)
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Uri uri_inner = uri;
    auto&& authority = uri_inner.GetAuthority();
    bool authorityFlag = authority == "media" || authority == "docs";
    auto permission = PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    newFlag = flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    if ((flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
        newFlag |= Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    } else {
        newFlag |= Want::FLAG_AUTH_READ_URI_PERMISSION;
    }
    if (!authorityFlag && !permission && (fromTokenId != callerTokenId)) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: No permission for proxy authorization uri.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!authorityFlag) {
        // ignore persistable uri permission flag.
        newFlag &= (~Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
        return ERR_OK;
    }

    if (!isGrantPersistableUriPermissionEnable_) {
        if (!permission) {
            HILOG_WARN("Do not have persistable uri permission proxy.");
            return CHECK_PERMISSION_FAILED;
        }
        newFlag &= (~Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
        return ERR_OK;
    }

    if ((newFlag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION) == 0 && CheckPersistableUriPermissionProxy(uri,
        flag, targetTokenId)) {
        newFlag = 0;
        HILOG_DEBUG("persistable uri permission has been granted");
        return ERR_OK;
    }
    if (!permission) {
        if (!CheckPersistableUriPermissionProxy(uri, flag, callerTokenId)) {
            HILOG_WARN("Do not have persistable uri permission proxy.");
            return CHECK_PERMISSION_FAILED;
        }
        newFlag |= Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::AddTempUriPermission(const std::string &uri, unsigned int flag,
    TokenId fromTokenId, TokenId targetTokenId, int autoremove)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    GrantInfo info = { flag, fromTokenId, targetTokenId, autoremove };
    if (search == uriMap_.end()) {
        HILOG_INFO("Insert an uri r/w permission.");
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uri, infoList);
        return ERR_OK;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            HILOG_DEBUG("Item: flag = %{public}i, fromTokenId = %{public}i, targetTokenId = %{public}i,\
                autoremove = %{public}i", item.flag, item.fromTokenId, item.targetTokenId, item.autoremove);
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

int UriPermissionManagerStubImpl::DeletTempUriPermission(const std::string &uri, uint32_t flag,
    uint32_t targetTokenId)
{
    if ((flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
        flag |= Want::FLAG_AUTH_READ_URI_PERMISSION;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    if (search == uriMap_.end()) {
        HILOG_DEBUG("uri do not in uri map.");
        return ERR_OK;
    }
    auto& list = search->second;
    for (auto it = list.begin(); it != list.end(); it++) {
        if (it->targetTokenId == targetTokenId && (it->flag & flag) != 0) {
            HILOG_DEBUG("delet the temporary uri permission in uri map.");
            list.erase(it);
            break;
        }
    }
    if (list.size() == 0) {
        uriMap_.erase(search);
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
    TokenId fromTokenId, TokenId targetTokenId, int autoremove)
{
    HILOG_INFO("uri = %{private}s, flag = %{public}i, fromTokenId = %{public}i, targetTokenId = %{public}i,\
        autoremove = %{public}i", uri.ToString().c_str(), flag, fromTokenId, targetTokenId, autoremove);
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
    // grant persistable uri permission
    if ((flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION) != 0) {
        ret = uriPermissionRdb_->AddGrantInfo(uriStr, flag, fromTokenId, targetTokenId);
        if (ret == ERR_OK) {
            // delete temporary uri permission
            ret = DeletTempUriPermission(uriStr, flag, targetTokenId);
        }
        return ret;
    }
    // grant temporary uri permission
    ret = AddTempUriPermission(uriStr, flag, fromTokenId, targetTokenId, autoremove);
    return ret;
}

void UriPermissionManagerStubImpl::RevokeUriPermission(const TokenId tokenId)
{
    HILOG_INFO("Start to remove uri permission.");
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

int UriPermissionManagerStubImpl::RevokeAllUriPermissions(uint32_t tokenId)
{
    HILOG_INFO("Start to remove all uri permission for uninstalled app or clear app data.");
        auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (nativeInfo.processName != "foundation") {
        HILOG_ERROR("RevokeAllUriPermission can only be called by foundation");
        return CHECK_PERMISSION_FAILED;
    }
    std::map<unsigned int, std::vector<std::string>> uriLists;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        // delte temporary uri permission
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
        return INNER_ERR;
    }

    if (!uriLists.empty()) {
        for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
            storageManager_->DeleteShareFile(iter->first, iter->second);
        }
    }

    if (!isGrantPersistableUriPermissionEnable_) {
        return ERR_OK;
    }
    // delete persistable uri permission
    auto ret = uriPermissionRdb_->RemoveGrantInfo(tokenId, storageManager_);
    return ret;
}

int UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    HILOG_INFO("Start to remove uri permission manually.");
    Uri uri_inner = uri;
    auto uriStr = uri.ToString();
    auto&& authority = uri_inner.GetAuthority();
    auto&& scheme = uri_inner.GetScheme();
    if (scheme != "file") {
        HILOG_WARN("only support file uri.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto uriTokenId = GetTokenIdByBundleName(authority, 0);
    auto tokenId = GetTokenIdByBundleName(bundleName, 0);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    VerifyUriPermission(uri, Want::FLAG_AUTH_READ_URI_PERMISSION, tokenId);
    VerifyUriPermission(uri, Want::FLAG_AUTH_WRITE_URI_PERMISSION, tokenId);
    auto permission = PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    bool authorityFlag = authority == "media" || authority == "docs";

    if (!authorityFlag && (uriTokenId != callerTokenId) && (tokenId != callerTokenId)) {
        HILOG_WARN("UriPermissionManagerStubImpl::RevokeUriPermission: No permission for revoke uri.");
        return CHECK_PERMISSION_FAILED;
    }

    if (authorityFlag && !permission && tokenId != callerTokenId) {
        HILOG_WARN("UriPermissionManagerStubImpl::RevokeUriPermission: No permission for revoke uri.");
        return CHECK_PERMISSION_FAILED;
    }

    if (authorityFlag && isGrantPersistableUriPermissionEnable_) {
        // delete persistable grant info
        ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
        if (storageManager_ == nullptr) {
            HILOG_ERROR("ConnectStorageManager failed");
            return INNER_ERR;
        }
        auto ret = uriPermissionRdb_->RemoveGrantInfo(uriStr, tokenId, storageManager_);
        if (ret != ERR_OK) {
            HILOG_ERROR("remove persistable uri permission failed.");
            return INNER_ERR;
        }
    }
    // delete temporary grant info
    return DeletTempUriPermissionAndShareFile(uriStr, tokenId);
}

int UriPermissionManagerStubImpl::DeletTempUriPermissionAndShareFile(const std::string &uri, uint32_t targetTokenId)
{
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return INNER_ERR;
    }
    std::vector<std::string> uriList;
    std::lock_guard<std::mutex> guard(mutex_);

    auto search = uriMap_.find(uri);
    if (search == uriMap_.end()) {
        HILOG_INFO("URI does not exist on uri map.");
        return ERR_OK;
    }
    auto& list = search->second;
    for (auto it = list.begin(); it != list.end(); it++) {
        if (it->targetTokenId == targetTokenId) {
            HILOG_INFO("Erase an info form list.");
            uriList.emplace_back(search->first);
            if (storageManager_->DeleteShareFile(targetTokenId, uriList) == ERR_OK) {
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

void UriPermissionManagerStubImpl::InitPersistableUriPermissionConfig()
{
    char value[GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_SIZE] = "false";
    int retSysParam = GetParameter(GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_PARAMETER, "false", value,
        GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_SIZE);
    HILOG_INFO("GrantPersistableUriPermissionEnable, %{public}s value is %{public}s.",
        GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_PARAMETER, value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        isGrantPersistableUriPermissionEnable_ = true;
    }
}
}  // namespace AAFwk
}  // namespace OHOS