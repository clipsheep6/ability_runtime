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

#include "uri_permission_manager_stub_impl.h"

#include <unordered_map>

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "hilog_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "os_account_manager_wrapper.h"
#include "parameter.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "proxy_authorization_uri_config.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "want.h"

#define READ_MODE (1<<0)
#define WRITE_MODE (1<<1)
#define IS_POLICY_ALLOWED_TO_BE_PRESISTED (1<<0)

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_USER_ID = 0;
constexpr int32_t ERR_OK = 0;
}

void UriPermissionManagerStubImpl::Init()
{
    DelayedSingleton<ProxyAuthorizationUriConfig>::GetInstance()->LoadConfiguration();
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
{
    // verify if tokenId have uri permission record
    auto uriStr = uri.ToString();
    HILOG_INFO("uri is %{private}s, flag is %{public}u, tokenId is %{public}u", uriStr.c_str(), flag, tokenId);
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    if (search != uriMap_.end()) {
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | Want::FLAG_AUTH_READ_URI_PERMISSION) & flag) != 0) {
                HILOG_INFO("have uri permission.");
                return true;
            }
        }
    }
    HILOG_INFO("have no uri permission");
    return false;
}

bool UriPermissionManagerStubImpl::IsAuthorizationUriAllowed(uint32_t fromTokenId)
{
    return DelayedSingleton<ProxyAuthorizationUriConfig>::GetInstance()->IsAuthorizationUriAllowed(fromTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("CALL: appIndex is %{public}d.", appIndex);
    std::vector<Uri> uriVec = { uri };
    return GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("CALL: appIndex is %{public}d, uriVec size is %{public}zu", appIndex, uriVec.size());
    if (AppUtils::GetInstance().IsGrantPersistUriPermission()) {
        bool isSystemAppCall = PermissionVerification::GetInstance()->IsSystemAppCall();
        // IsSACall function used for visibility checks.
        if (!isSystemAppCall && !PermissionVerification::GetInstance()->IsSACall()) {
            HILOG_ERROR("Not system application or SA call.");
            return INNER_ERR;
        }
        return GrantUriPermissionFor2In1Inner(
            uriVec, flag, targetBundleName, appIndex, isSystemAppCall, initiatorTokenId);
    }
    return GrantUriPermissionInner(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermissionInner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("Called.");
    auto checkResult = CheckRule(flag);
    if (checkResult != ERR_OK) {
        return checkResult;
    }
    flag &= (Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION);
    auto targetTokenId = GetTokenIdByBundleName(targetBundleName, appIndex);
    // autoRemove will be set to 1 if the process name is foundation.
    uint32_t autoRemove = 0;
    uint32_t appTokenId = IPCSkeleton::GetCallingTokenID();
    if (IsFoundationCall()) {
        autoRemove = 1;
        appTokenId = initiatorTokenId;
    }
    if (uriVec.size() == 1) {
        return GrantSingleUriPermission(uriVec[0], flag, appTokenId, targetTokenId, autoRemove);
    }
    return GrantBatchUriPermission(uriVec, flag, appTokenId, targetTokenId, autoRemove);
}

int checkPersistPermission(uint64_t tokenId, const std::vector<PolicyInfo> &policy, std::vector<bool> &result)
{
    for (size_t i = 0; i < policy.size(); i++) {
        result.emplace_back(true);
    }
    HILOG_INFO("Called, result size is %{public}zu", result.size());
    return 0;
}

int32_t setPolicy(uint64_t tokenId, const std::vector<PolicyInfo> &policy, uint64_t policyFlag)
{
    HILOG_INFO("Called, policy size is %{public}zu", policy.size());
    return 0;
}

int persistPermission(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result)
{
    for (size_t i = 0; i < policy.size(); i++) {
        result.emplace_back(0);
    }
    HILOG_INFO("Called, result size is %{public}zu", result.size());
    return 0;
}

int UriPermissionManagerStubImpl::CheckRule(unsigned int flag)
{
    // reject sandbox to grant uri permission
    ConnectManager(appMgr_, APP_MGR_SERVICE_ID);
    if (appMgr_ == nullptr) {
        HILOG_ERROR("Get BundleManager failed!");
        return INNER_ERR;
    }
    auto callerPid = IPCSkeleton::GetCallingPid();
    bool isSandbox = false;
    if (appMgr_->JudgeSandboxByPid(callerPid, isSandbox) != ERR_OK) {
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
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
{
    HILOG_DEBUG("Called.");
    if (!IsFoundationCall()) {
        HILOG_ERROR("Not foundation call.");
        return INNER_ERR;
    }
    return GrantUriPermissionFor2In1Inner(uriVec, flag, targetBundleName, appIndex, isSystemAppCall);
}

int UriPermissionManagerStubImpl::AddTempUriPermission(const std::string &uri, unsigned int flag,
    TokenId fromTokenId, TokenId targetTokenId, uint32_t autoRemove)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    GrantInfo info = { flag, fromTokenId, targetTokenId, autoRemove };
    if (search == uriMap_.end()) {
        HILOG_INFO("Insert an uri r/w permission.");
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uri, infoList);
        return ERR_OK;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            HILOG_DEBUG("Item: flag = %{public}i, fromTokenId = %{public}i, targetTokenId = %{public}i,"
                "autoRemove = %{public}i", item.flag, item.fromTokenId, item.targetTokenId, item.autoRemove);
            if ((flag & (item.flag | Want::FLAG_AUTH_READ_URI_PERMISSION)) == 0) {
                HILOG_INFO("Update uri r/w permission.");
                item.flag = flag;
            } else {
                HILOG_INFO("uri permission has granted, not to grant again.");
            }
            return ERR_OK;
        }
    }
    HILOG_INFO("Insert an new uri permission record.");
    infoList.emplace_back(info);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
    TokenId callerTokenId, TokenId targetTokenId, uint32_t autoRemove)
{
    HILOG_INFO("uri = %{private}s, flag = %{public}i, callerTokenId = %{public}i, targetTokenId = %{public}i,"
        "autoRemove = %{public}i", uri.ToString().c_str(), flag, callerTokenId, targetTokenId, autoRemove);
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectManager failed");
        return INNER_ERR;
    }
    auto uriStr = uri.ToString();
    std::vector<std::string> uriVec = { uriStr };
    auto resVec = storageManager_->CreateShareFile(uriVec, targetTokenId, flag);
    if (resVec.size() == 0) {
        HILOG_ERROR("storageManager resVec is empty.");
        return INNER_ERR;
    }
    if (resVec[0] != 0 && resVec[0] != -EEXIST) {
        HILOG_ERROR("failed to CreateShareFile.");
        return INNER_ERR;
    }
    AddTempUriPermission(uriStr, flag, callerTokenId, targetTokenId, autoRemove);
    SendEvent(callerTokenId, targetTokenId, uriStr);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantSingleUriPermission(const Uri &uri, unsigned int flag, uint32_t callerTokenId,
    uint32_t targetTokenId, uint32_t autoRemove)
{
    if (!CheckUriTypeIsValid(uri)) {
        HILOG_ERROR("Check uri type failed, uri is %{private}s", uri.ToString().c_str());
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto permission = IsAuthorizationUriAllowed(callerTokenId);
    if (!permission && !CheckUriPermission(uri, flag, callerTokenId)) {
        HILOG_WARN("No permission, uri is %{private}s, callerTokenId is %{public}u",
            uri.ToString().c_str(), callerTokenId);
        return CHECK_PERMISSION_FAILED;
    }
    return GrantUriPermissionImpl(uri, flag, callerTokenId, targetTokenId, autoRemove);
}

int UriPermissionManagerStubImpl::GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec,
    unsigned int flag, TokenId callerTokenId, TokenId targetTokenId, uint32_t autoRemove)
{
    HILOG_INFO("callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}i, list size is %{public}zu",
        callerTokenId, targetTokenId, flag, uriVec.size());
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectManager failed.");
        return INNER_ERR;
    }
    auto resVec = storageManager_->CreateShareFile(uriVec, targetTokenId, flag);
    if (resVec.size() == 0) {
        HILOG_ERROR("Failed to createShareFile, storageManager resVec is empty.");
        return INNER_ERR;
    }
    if (resVec.size() != uriVec.size()) {
        HILOG_ERROR("Failed to createShareFile, ret is %{public}u", resVec[0]);
        return resVec[0];
    }
    EventInfo eventInfo;
    bool needSendEvent = CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);
    int successCount = 0;
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto ret = resVec[i];
        if (ret != 0 && ret != -EEXIST) {
            HILOG_ERROR("failed to CreateShareFile.");
            continue;
        }
        AddTempUriPermission(uriVec[i], flag, callerTokenId, targetTokenId, autoRemove);
        if (needSendEvent) {
            eventInfo.uri = uriVec[i];
            EventReport::SendKeyEvent(EventName::GRANT_URI_PERMISSION, HiSysEventType::BEHAVIOR, eventInfo);
        }
        successCount++;
    }
    HILOG_INFO("total %{public}d uri permissions added.", successCount);
    if (successCount == 0) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantBatchUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    uint32_t callerTokenId, uint32_t targetTokenId, uint32_t autoRemove)
{
    std::vector<std::string> uriStrVec = {};
    auto permission = IsAuthorizationUriAllowed(callerTokenId);
    for (const auto &uri : uriVec) {
        if (!CheckUriTypeIsValid(uri)) {
            HILOG_WARN("Check uri type failed, ");
            continue;
        }
        if (!permission && !CheckUriPermission(uri, flag, callerTokenId)) {
            HILOG_WARN("No permission, uri is %{private}s, callerTokenId is %{public}u",
                uri.ToString().c_str(), callerTokenId);
            continue;
        }
        uriStrVec.emplace_back(uri.ToString());
    }
    return GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, autoRemove);
}

void UriPermissionManagerStubImpl::RevokeUriPermission(const TokenId tokenId)
{
    HILOG_INFO("Start to remove uri permission.");
    if (!IsFoundationCall()) {
        HILOG_ERROR("RevokeUriPermission can only be called by foundation");
        return;
    }
    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end(); it++) {
                if (it->targetTokenId == tokenId && it->autoRemove) {
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
    if (!IsFoundationCall()) {
        HILOG_ERROR("RevokeAllUriPermissions can only be called by foundation");
        return CHECK_PERMISSION_FAILED;
    }
    std::map<unsigned int, std::vector<std::string>> uriLists;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        // delete temporary uri permission
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
        HILOG_ERROR("ConnectStorageManager failed.");
        return INNER_ERR;
    }
    if (!uriLists.empty()) {
        for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
            storageManager_->DeleteShareFile(iter->first, iter->second);
        }
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    HILOG_INFO("Start to remove uri permission manually.");
    Uri uri_inner = uri;
    auto uriStr = uri.ToString();
    auto&& authority = uri_inner.GetAuthority();
    auto&& scheme = uri_inner.GetScheme();
    if (scheme != "file" && scheme != "content") {
        HILOG_WARN("only support file uri.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto tokenId = GetTokenIdByBundleName(bundleName, 0);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    HILOG_DEBUG("callerTokenId is %{public}u, targetTokenId is %{public}u", callerTokenId, tokenId);
    if (tokenId == callerTokenId) {
        return DeleteTempUriPermission(uriStr, 0, tokenId);
    }
    return DeleteTempUriPermission(uriStr, callerTokenId, tokenId);
}

int UriPermissionManagerStubImpl::DeleteTempUriPermission(const std::string &uri, uint32_t fromTokenId,
    uint32_t targetTokenId)
{
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectStorageManager failed");
        return INNER_ERR;
    }

    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    if (search == uriMap_.end()) {
        HILOG_INFO("URI does not exist on uri map.");
        return ERR_OK;
    }
    auto& list = search->second;
    for (auto it = list.begin(); it != list.end(); it++) {
        if ((it->fromTokenId == fromTokenId || fromTokenId == 0) && it->targetTokenId == targetTokenId) {
            HILOG_INFO("Notify storageMGR to delete shareFile.");
            std::vector<std::string> uriList;
            uriList.emplace_back(search->first);
            auto procedureRet = storageManager_->DeleteShareFile(targetTokenId, uriList);
            if (procedureRet != ERR_OK) {
                HILOG_ERROR("DeleteShareFile failed");
                return procedureRet;
            }
            list.erase(it);
            break;
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

std::shared_ptr<AppExecFwk::BundleMgrHelper> UriPermissionManagerStubImpl::ConnectManagerHelper()
{
    if (bundleMgrHelper_ == nullptr) {
        bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    }
    return bundleMgrHelper_;
}

uint32_t UriPermissionManagerStubImpl::GetTokenIdByBundleName(const std::string bundleName, int32_t appIndex)
{
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_WARN("The bundleMgrHelper is nullptr.");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    AppExecFwk::BundleInfo bundleInfo;
    auto userId = GetCurrentAccountId();
    if (appIndex == 0) {
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName, bundleFlag, bundleInfo, userId))) {
            HILOG_WARN("Failed to get bundle info according to uri.");
            return GET_BUNDLE_INFO_FAILED;
        }
    } else {
        if (IN_PROCESS_CALL(bundleMgrHelper->GetSandboxBundleInfo(
            bundleName, appIndex, userId, bundleInfo) != ERR_OK)) {
            HILOG_WARN("Failed to get bundle info according to appIndex.");
            return GET_BUNDLE_INFO_FAILED;
        }
    }
    return bundleInfo.applicationInfo.accessTokenId;
}

void UriPermissionManagerStubImpl::ProxyDeathRecipient::OnRemoteDied([[maybe_unused]]
    const wptr<IRemoteObject>& remote)
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
        HILOG_ERROR("QueryActiveOsAccountIds error.");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        HILOG_ERROR("%{public}s, the QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1Inner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall, uint32_t initiatorTokenId)
{
    HILOG_DEBUG("Called, uriVec size is %{public}zu", uriVec.size());
    auto checkResult = CheckRule(flag);
    if (checkResult != ERR_OK) {
        return checkResult;
    }
    std::vector<PolicyInfo> docsVec;
    std::vector<Uri> otherVec;
    for (const auto &uri : uriVec) {
        Uri uri_inner = uri;
        auto &&scheme = uri_inner.GetScheme();
        if (scheme != "file") {
            HILOG_WARN("Only support file uri.");
            continue;
        }
        auto &&authority = uri_inner.GetAuthority();
        HILOG_DEBUG("The authority is %{public}s", authority.c_str());
        PolicyInfo policyInfo;
        policyInfo.path = uri_inner.ToString();
        if ((flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
            policyInfo.mode |= WRITE_MODE;
        } else {
            policyInfo.mode |= READ_MODE;
        }
        if (authority == "docs") {
            docsVec.emplace_back(policyInfo);
        } else {
            otherVec.emplace_back(uri_inner);
        }
    }
    uint32_t tokenId = GetTokenIdByBundleName(targetBundleName, appIndex);
    HILOG_DEBUG("The tokenId is %{public}u", tokenId);
    HandleUriPermission(tokenId, flag, docsVec, isSystemAppCall);
    if (!otherVec.empty()) {
        return GrantUriPermissionInner(otherVec, flag, targetBundleName, appIndex, initiatorTokenId);
    }
    return ERR_OK;
}

void UriPermissionManagerStubImpl::HandleUriPermission(
    uint64_t tokenId, unsigned int flag, std::vector<PolicyInfo> &docsVec, bool isSystemAppCall)
{
    uint32_t policyFlag = 0;
    if ((flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION) != 0) {
        policyFlag |= IS_POLICY_ALLOWED_TO_BE_PRESISTED;
    }
    // Handle docs type URI permission
    if (!docsVec.empty()) {
        std::vector<bool> result;
        checkPersistPermission(tokenId, docsVec, result);
        if (docsVec.size() != result.size()) {
            HILOG_ERROR("Check persist permission failed.");
            return;
        }
        std::vector<PolicyInfo> policyVec;
        auto docsItem = docsVec.begin();
        for (auto resultItem = result.begin(); resultItem != result.end();) {
            if (*resultItem == true) {
                policyVec.emplace_back(*docsItem);
            }
            resultItem++;
            docsItem++;
        }
        if (!policyVec.empty()) {
            setPolicy(tokenId, policyVec, policyFlag);
            // The current processing starts from API 11 and maintains 5 versions.
            if (((policyFlag & IS_POLICY_ALLOWED_TO_BE_PRESISTED) != 0) && isSystemAppCall) {
                std::vector<uint32_t> persistResult;
                persistPermission(policyVec, persistResult);
            }
        }
    }
}

bool UriPermissionManagerStubImpl::IsFoundationCall()
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    std::string processName;
    Security::AccessToken::AccessTokenKit::GetNativeTokenName(callerTokenId, processName);
    HILOG_DEBUG("Caller process name : %{public}s", processName.c_str());
    if (nativeInfo.processName == "foundation") {
        return true;
    }
    return false;
}

bool UriPermissionManagerStubImpl::SendEvent(uint32_t callerTokenId, uint32_t targetTokenId, std::string &uri)
{
    EventInfo eventInfo;
    eventInfo.uri = uri;
    if (CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo)) {
        EventReport::SendKeyEvent(EventName::GRANT_URI_PERMISSION, HiSysEventType::BEHAVIOR, eventInfo);
        return true;
    }
    return false;
}

bool UriPermissionManagerStubImpl::CheckAndCreateEventInfo(uint32_t callerTokenId, uint32_t targetTokenId,
    EventInfo &eventInfo)
{
    std::string callerBundleName = GetBundleNameByTokenId(callerTokenId);
    std::string targetBundleName = GetBundleNameByTokenId(targetTokenId);
    if (callerBundleName.empty() || targetBundleName.empty()) {
        HILOG_ERROR("Caller bundle name is empty or target bundle name is empty.");
        return false;
    }
    auto isSystemAppCall = CheckIsSystemAppByBundleName(callerBundleName);
    auto targetIsSystemApp = CheckIsSystemAppByBundleName(targetBundleName);
    if (!isSystemAppCall || targetIsSystemApp) {
        HILOG_DEBUG("Caller is not system app or callee is system app.");
        return false;
    }
    HILOG_INFO("Send Grant_Uri_Permission event.");
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    return true;
}

std::string UriPermissionManagerStubImpl::GetBundleNameByTokenId(uint32_t tokenId)
{
    Security::AccessToken::HapTokenInfo hapInfo;
    auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
    if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        HILOG_ERROR("GetHapTokenInfo failed, ret is %{public}i", ret);
        return "";
    }
    return hapInfo.bundleName;
}

bool UriPermissionManagerStubImpl::CheckIsSystemAppByBundleName(std::string &bundleName)
{
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_WARN("The bundleMgrHelper is nullptr.");
        return false;
    }
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetCurrentAccountId(), appInfo))) {
        HILOG_WARN("Get application info failed.");
        return false;
    }
    auto isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(appInfo.accessTokenIdEx);
    HILOG_DEBUG("BundleName is %{public}s, isSystemApp = %{public}i", bundleName.c_str(),
        static_cast<int32_t>(isSystemApp));
    return isSystemApp;
}

bool UriPermissionManagerStubImpl::CheckUriPermission(const Uri &uri, unsigned int flag, uint32_t callerTokenId)
{
    // Check if caller have permission to grant uri with type of bundle name.
    auto uriInner = uri;
    auto &&authority = uriInner.GetAuthority();
    auto authorityTokenId = GetTokenIdByBundleName(authority, 0);
    if (authority == "docs" || authority == "media") {
        HILOG_ERROR("Authotity is media or docs, do not have permission.");
        return false;
    }
    if (authorityTokenId != callerTokenId) {
        HILOG_ERROR("The uri does not belong to caller, have not permission");
        return false;
    }
    return true;
}

bool UriPermissionManagerStubImpl::CheckUriTypeIsValid(const Uri &uri)
{
    auto innerUri = uri;
    auto &&scheme = innerUri.GetScheme();
    if (scheme != "file" && scheme != "content") {
        HILOG_ERROR("Only support file or content uri, scheme is %{public}s", scheme.c_str());
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS