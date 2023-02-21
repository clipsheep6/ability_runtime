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

#include "accesstoken_kit.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
const int32_t DEFAULT_USER_ID = 0;
using TokenId = Security::AccessToken::AccessTokenID;

bool UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const TokenId fromTokenId, const TokenId targetTokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(IPCSkeleton::GetCallingTokenID());
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        HILOG_DEBUG("caller tokenType is not native, verify failure.");
        return false;
    }

    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: The param flag is invalid.");
        return false;
    }
    unsigned int tmpFlag = 0;
    if (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
        tmpFlag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    } else {
        tmpFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    }

    auto storageMgrProxy = ConnectStorageManager();
    if (storageMgrProxy == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return false;
    }
    auto uriStr = uri.ToString();
    auto ret = storageMgrProxy->CreateShareFile(uriStr, targetTokenId, tmpFlag);
    if (ret != 0 && ret != -EEXIST) {
        HILOG_ERROR("storageMgrProxy failed to CreateShareFile.");
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    if (search == uriMap_.end()) {
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uriStr, infoList);
        return true;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            if ((tmpFlag & item.flag) == 0) {
                HILOG_INFO("Update uri r/w permission.");
                item.flag = tmpFlag;
            }
            HILOG_INFO("uri permission has granted, not to grant again.");
            return true;
        }
    }
    infoList.emplace_back(info);
    return true;
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID tokenId)
{
    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl:::VerifyUriPermission: The param flag is invalid.");
        return false;
    }

    auto bms = ConnectBundleManager();
    auto uriStr = uri.ToString();
    if (bms) {
        AppExecFwk::ExtensionAbilityInfo info;
        if (!IN_PROCESS_CALL(bms->QueryExtensionAbilityInfoByUri(uriStr, GetCurrentAccountId(), info))) {
            HILOG_DEBUG("%{public}s, Fail to get extension info from bundle manager.", __func__);
            return false;
        }
        if (info.type != AppExecFwk::ExtensionAbilityType::FILESHARE) {
            HILOG_DEBUG("%{public}s, The upms only open to FILESHARE. The type is %{public}u.", __func__, info.type);
            return false;
        }

        if (tokenId == info.applicationInfo.accessTokenId) {
            HILOG_DEBUG("The uri belongs to this application.");
            return true;
        }
    }

    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    if (search == uriMap_.end()) {
        HILOG_DEBUG("This tokenID does not have permission for this uri.");
        return false;
    }

    unsigned int tmpFlag = 0;
    if (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
        tmpFlag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    } else {
        tmpFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    }

    for (const auto& item : search->second) {
        if (item.targetTokenId == tokenId &&
            (item.flag == Want::FLAG_AUTH_WRITE_URI_PERMISSION || item.flag == tmpFlag)) {
            HILOG_DEBUG("This tokenID have permission for this uri.");
            return true;
        }
    }

    HILOG_DEBUG("The application does not have permission for this URI.");
    return false;
}

void UriPermissionManagerStubImpl::RemoveUriPermission(const TokenId tokenId)
{
    HILOG_DEBUG("Start to remove uri permission.");
    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end(); it++) {
                if (it->targetTokenId == tokenId) {
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
        sptr<BMSOrSMDeathRecipient> recipient(new BMSOrSMDeathRecipient(onClearProxyCallback));
        bundleManager_->AsObject()->AddDeathRecipient(recipient);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return bundleManager_;
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
        sptr<BMSOrSMDeathRecipient> recipient(new BMSOrSMDeathRecipient(onClearProxyCallback));
        storageManager_->AsObject()->AddDeathRecipient(recipient);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return storageManager_;
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

void UriPermissionManagerStubImpl::BMSOrSMDeathRecipient::OnRemoteDied(
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
