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
#include "permission_constants.h"
#include "permission_verification.h"
#include "singleton.h"
#include "system_ability_definition.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
const int32_t DEFAULT_USER_ID = 0;

void UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID fromTokenId, const Security::AccessToken::AccessTokenID targetTokenId)
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerTokenId : %{public}u", callerTokenId); 

    // only uri with proxy authorization permission or from process itself can be granted
    auto permission = PermissionVerification::GetInstance()->VerifyCallingPermission(  
        AAFwk::PermissionConstants::PERMISSION_PROXY_AUTHORIZATION_URI);
    if (!permission && (fromTokenId != callerTokenId)) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: No permission for proxy authorization uri.");
        return;
    }

    if ((flag & (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION)) == 0) {
        HILOG_WARN("UriPermissionManagerStubImpl::GrantUriPermission: The param flag is invalid.");
        return;
    }
    unsigned int tmpFlag = 0;
    if (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) {
        tmpFlag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    } else {
        tmpFlag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    }

    auto bms = ConnectBundleManager();
    Uri uri_inner = uri;
    auto&& authority = uri_inner.GetAuthority();
    HILOG_INFO("uri authority is %{public}s.", authority.c_str());
    AppExecFwk::BundleInfo uriBundleInfo;
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    if (!IN_PROCESS_CALL(bms->GetBundleInfo(authority, bundleFlag, uriBundleInfo, GetCurrentAccountId()))) {
        HILOG_WARN("To fail to get bundle info according to uri.");
        return;
    }
    if (uriBundleInfo.applicationInfo.accessTokenId != callerTokenId) {
        HILOG_ERROR("the uri does not belong to caller.");
        return;
    }
    auto&& scheme = uri_inner.GetScheme();
    HILOG_INFO("uri scheme is %{public}s.", scheme.c_str());
    // only support file or dataShare scheme
    if (scheme != "file" && scheme != "dataShare") {
        HILOG_WARN("only support file or dataShare uri.");
        return;
    }

    auto uriStr = uri.ToString();
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    unsigned int autoremove = 0;
    // auto remove URI permission for clipboard
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(fromTokenId, nativeInfo);
    HILOG_DEBUG("callerprocessName : %{public}s", nativeInfo.processName.c_str());
    if (nativeInfo.processName == "pasteboard_serv") {
        autoremove = 1;
    }
    GrantInfo info = { tmpFlag, callerTokenId, targetTokenId, autoremove };
    if (search == uriMap_.end()) {
        HILOG_INFO("uri is not exist, add uri and GrantInfo to map.");
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uriStr, infoList);
        return;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            if ((tmpFlag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
                item.flag = tmpFlag;
            }
            HILOG_INFO("uri permission has granted, not to grant again.");
            return;
        }
    }
    HILOG_DEBUG("uri is exist, add GrantInfo to list.");
    infoList.emplace_back(info);
}

void UriPermissionManagerStubImpl::GrantUriPermissionFromSelf(const Uri &uri, unsigned int flag,
    const Security::AccessToken::AccessTokenID targetTokenId)
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerTokenId : %{public}u", callerTokenId);
    
    GrantUriPermission(uri, flag, callerTokenId, targetTokenId);
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

void UriPermissionManagerStubImpl::RemoveUriPermission(const Security::AccessToken::AccessTokenID tokenId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
        auto& list = iter->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if (it->targetTokenId == tokenId && it->autoremove) {
                HILOG_INFO("Erase an info form list.");
                list.erase(it);
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
                impl->ClearProxy();
            }
        };
        sptr<BMSDeathRecipient> recipient(new BMSDeathRecipient(onClearProxyCallback));
        bundleManager_->AsObject()->AddDeathRecipient(recipient);
    }
    HILOG_DEBUG("%{public}s end.", __func__);
    return bundleManager_;
}

void UriPermissionManagerStubImpl::ClearProxy()
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    std::lock_guard<std::mutex> lock(bmsMutex_);
    bundleManager_ = nullptr;
}

void UriPermissionManagerStubImpl::BMSDeathRecipient::OnRemoteDied([[maybe_unused]] const wptr<IRemoteObject>& remote)
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