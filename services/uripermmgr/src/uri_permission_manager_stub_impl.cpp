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

#include <unordered_map>

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "event_report.h"
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
const char* GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
constexpr int32_t GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_SIZE = 6;
}

void UriPermissionManagerStubImpl::Init()
{
    InitPersistableUriPermissionConfig();
    if (isGrantPersistableUriPermissionEnable_) {
        HILOG_INFO("Init uri permission database manager.");
        uriPermissionRdb_ = std::make_shared<UriPermissionRdb>();
    }
    DelayedSingleton<ProxyAuthorizationUriConfig>::GetInstance()->LoadConfiguration();
}

bool UriPermissionManagerStubImpl::CheckPersistableUriPermissionProxy(const Uri& uri, uint32_t flag, uint32_t tokenId)
{
    // check if caller can grant persistable uri permission
    auto uriStr = uri.ToString();
    if (uriPermissionRdb_ == nullptr) {
        HILOG_WARN("rbd manager is null.");
        return false;
    }
    return uriPermissionRdb_->CheckPersistableUriPermissionProxy(uriStr, flag, tokenId);
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
{
    // verify if tokenId have uri permission of flag, including temporary permission and persistable permission
    HILOG_DEBUG("VerifyUriPermission called: flag = %{public}i", static_cast<int>(flag));
    auto uriStr = uri.ToString();
    bool tempPermission = false;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto search = uriMap_.find(uriStr);
        if (search != uriMap_.end()) {
            auto& list = search->second;
            for (auto it = list.begin(); it != list.end(); it++) {
                bool condition = (it->targetTokenId == tokenId) &&
                    ((it->flag | Want::FLAG_AUTH_READ_URI_PERMISSION) & flag) != 0;
                if (condition) {
                    tempPermission = true;
                    break;
                }
            }
        }
    }
    if (tempPermission) {
        HILOG_DEBUG("temporary uri permission exists");
        return true;
    }
    if (isGrantPersistableUriPermissionEnable_) {
        if (uriPermissionRdb_ == nullptr) {
            HILOG_WARN("rbd manager is null.");
            return false;
        }
        if (uriPermissionRdb_->CheckPersistableUriPermissionProxy(uriStr, flag, tokenId)) {
            HILOG_DEBUG("persistable uri permission exists");
            return true;
        }
    }
    HILOG_DEBUG("uri permission not exists");
    return false;
}

bool UriPermissionManagerStubImpl::IsAuthorizationUriAllowed(uint32_t fromTokenId)
{
    return DelayedSingleton<ProxyAuthorizationUriConfig>::GetInstance()->IsAuthorizationUriAllowed(fromTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex)
{
    HILOG_DEBUG("CALL: appIndex is %{public}d.", appIndex);
    std::vector<Uri> uriVec = { uri };
    return GrantUriPermission(uriVec, flag, targetBundleName, appIndex);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex)
{
    HILOG_DEBUG("CALL: appIndex is %{public}d, uriVec size is %{public}zu", appIndex, uriVec.size());
    auto checkResult = CheckRule(flag);
    if (checkResult != ERR_OK) {
        return checkResult;
    }
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto targetTokenId = GetTokenIdByBundleName(targetBundleName, appIndex);
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    // autoremove will be set to 1 if the process name is foundation.
    HILOG_DEBUG("callerprocessName : %{public}s", nativeInfo.processName.c_str());
    int autoremove = 0;
    if (nativeInfo.processName == "foundation") {
        autoremove = 1;
    }
    // reserve origin process
    if (uriVec.size() == 1) {
        auto singleRet =  GrantSingleUriPermission(uriVec[0], flag, targetBundleName, autoremove, appIndex);
        if (singleRet == ERR_OK) {
            SendEvent(uriVec[0], targetBundleName, targetTokenId);
        }
        return singleRet;
    }
    std::unordered_map<uint32_t, std::vector<std::string>> uriVecMap;
    std::unordered_map<uint32_t, std::vector<uint32_t>> fromTokenIdVecMap;
    GetUriPermissionBatchFlag(uriVec, flag, targetTokenId, uriVecMap, fromTokenIdVecMap);
    if (uriVecMap.empty()) {
        HILOG_DEBUG("uriVecMap is empty");
        return INNER_ERR;
    }
    int ret = INNER_ERR;
    for (const auto &item : uriVecMap) {
        auto tempRet = GrantBatchUriPermissionImpl(item.second, item.first, fromTokenIdVecMap[item.first],
            targetTokenId, autoremove);
        if (tempRet == ERR_OK) {
            ret = ERR_OK;
            SendEvent(uriVec[0], targetBundleName, targetTokenId, item.second);
        }
    }
    return ret;
}

int checkPersistPermission(uint64_t tokenId, const std::vector<PolicyInfo> &policy, std::vector<bool> &result)
{
    for (auto i = 0; i < policy.size(); i++) {
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
    for (auto i = 0; i < policy.size(); i++) {
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

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1(
    const Uri &uri, unsigned int flag, const std::string &targetBundleName, int32_t appIndex)
{
    HILOG_DEBUG("Called.");
    if (!PermissionVerification::GetInstance()->IsSystemAppCall()) {
        HILOG_ERROR("Not system application call.");
        return ERR_NOT_SYSTEM_APP;
    }
    std::vector<Uri> uriVec = { uri };
    return GrantUriPermissionFor2In1Inner(uriVec, flag, targetBundleName, appIndex, true);
}

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
{
    if (!IsFoundationCall()) {
        HILOG_ERROR("Not foundation call.");
        return INNER_ERR;
    }
    return GrantUriPermissionFor2In1Inner(uriVec, flag, targetBundleName, appIndex, isSystemAppCall);
}

int UriPermissionManagerStubImpl::GetUriPermissionFlag(const Uri &uri, unsigned int flag,
    uint32_t fromTokenId, uint32_t targetTokenId, unsigned int &newFlag)
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Uri uri_inner = uri;
    auto&& authority = uri_inner.GetAuthority();
    auto permission = IsAuthorizationUriAllowed(callerTokenId);
    if ((flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
        newFlag |= Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    } else {
        newFlag |= Want::FLAG_AUTH_READ_URI_PERMISSION;
    }
    if (authority != "docs" || !isGrantPersistableUriPermissionEnable_) {
        if (!permission && (fromTokenId != callerTokenId)) {
            HILOG_WARN("No permission for proxy authorization uri.");
            return CHECK_PERMISSION_FAILED;
        }
        return ERR_OK;
    }
    // docs uri for 2in1
    auto persistableFlag = flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    if (persistableFlag == 0 && CheckPersistableUriPermissionProxy(uri, flag, targetTokenId)) {
        HILOG_INFO("persistable uri permission has been granted");
        newFlag = 0;
        return ERR_OK;
    }

    if (permission) {
        newFlag |= persistableFlag;
        return ERR_OK;
    }

    if (!CheckPersistableUriPermissionProxy(uri, flag, callerTokenId)) {
        HILOG_ERROR("Do not have persistable uri permission proxy.");
        return CHECK_PERMISSION_FAILED;
    }

    newFlag |= Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
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
            if ((flag & (item.flag | Want::FLAG_AUTH_READ_URI_PERMISSION)) == 0) {
                HILOG_INFO("Update uri r/w permission.");
                item.flag = flag;
            } else {
                HILOG_INFO("uri permission has granted, not to grant again.");
            }
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
    auto compareFunc = [flag, targetTokenId] (GrantInfo &grantInfo) -> bool {
        return (grantInfo.targetTokenId == targetTokenId) && (grantInfo.flag & flag) != 0;
    };
    auto iter = find_if(list.begin(), list.end(), compareFunc);
    if (iter != list.end()) {
        HILOG_DEBUG("delete temporary uri permission.");
        ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
        if (storageManager_ == nullptr) {
            HILOG_ERROR("ConnectManager failed");
            return INNER_ERR;
        }
        std::vector<std::string> uriList = { uri };
        if (storageManager_->DeleteShareFile(targetTokenId, uriList) == ERR_OK) {
            list.erase(iter);
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
    std::vector<std::string> uriVec = { uriStr };
    auto resVec = storageManager_->CreateShareFile(uriVec, targetTokenId, flag);
    if (resVec.size() == 0) {
        HILOG_ERROR("storageManager resVec is empty.");
        return INNER_ERR;
    }
    auto createFileRet = resVec[0];
    if (createFileRet != 0 && createFileRet != -EEXIST) {
        HILOG_ERROR("failed to CreateShareFile.");
        return INNER_ERR;
    }
    // grant persistable uri permission
    if ((flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION) != 0) {
        if (uriPermissionRdb_ == nullptr) {
            HILOG_WARN("rbd manager is null.");
            return INNER_ERR;
        }
        auto addInfoRet = uriPermissionRdb_->AddGrantInfo(uriStr, flag, fromTokenId, targetTokenId);
        if (addInfoRet == ERR_OK) {
            // delete temporary uri permission
            return DeletTempUriPermission(uriStr, flag, targetTokenId);
        }
        return addInfoRet;
    }
    // grant temporary uri permission
    return AddTempUriPermission(uriStr, flag, fromTokenId, targetTokenId, autoremove);
}

int UriPermissionManagerStubImpl::GrantSingleUriPermission(const Uri &uri, unsigned int flag,
    const std::string &targetBundleName, int autoremove, int32_t appIndex)
{
    Uri uri_inner = uri;
    auto&& scheme = uri_inner.GetScheme();
    if (scheme != "file" && scheme != "content") {
        HILOG_WARN("only support file uri.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto&& authority = uri_inner.GetAuthority();
    auto fromTokenId = GetTokenIdByBundleName(authority, 0);
    auto targetTokenId = GetTokenIdByBundleName(targetBundleName, appIndex);
    unsigned int tmpFlag = 0;
    auto ret = GetUriPermissionFlag(uri, flag, fromTokenId, targetTokenId, tmpFlag);
    if (ret != ERR_OK || tmpFlag == 0) {
        return ret;
    }
    return GrantUriPermissionImpl(uri, tmpFlag, fromTokenId, targetTokenId, autoremove);
}

void UriPermissionManagerStubImpl::GetUriPermissionBatchFlag(const std::vector<Uri> &uriVec,
    unsigned int flag, uint32_t targetTokenId,
    std::unordered_map<uint32_t, std::vector<std::string>> &uriVecMap,
    std::unordered_map<uint32_t, std::vector<uint32_t>> &fromTokenIdVecMap)
{
    HILOG_DEBUG("GetUriPermissionBatchFlag Call, flag is %{public}i", flag);
    for (const auto &uri : uriVec) {
        Uri uri_inner = uri;
        auto&& scheme = uri_inner.GetScheme();
        if (scheme != "file" && scheme != "content") {
            HILOG_WARN("only support file uri.");
            continue;
        }
        auto&& authority = uri_inner.GetAuthority();
        auto fromTokenId = GetTokenIdByBundleName(authority, 0);
        unsigned int tmpFlag = 0;
        auto ret = GetUriPermissionFlag(uri, flag, fromTokenId, targetTokenId, tmpFlag);
        if (ret != ERR_OK || tmpFlag == 0) {
            continue;
        }
        if (uriVecMap.find(tmpFlag) == uriVecMap.end()) {
            HILOG_DEBUG("find tempFlag = %{public}i", tmpFlag);
            std::vector<std::string> uriVec;
            uriVecMap.emplace(tmpFlag, uriVec);
            std::vector<uint32_t> tokenIdVec;
            fromTokenIdVecMap.emplace(tmpFlag, tokenIdVec);
        }
        uriVecMap[tmpFlag].emplace_back(uri.ToString());
        fromTokenIdVecMap[tmpFlag].emplace_back(fromTokenId);
    }
}

int UriPermissionManagerStubImpl::GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec,
    unsigned int flag, std::vector<uint32_t> &fromTokenIdVec, TokenId targetTokenId, int autoremove)
{
    HILOG_DEBUG("CALL: targetTokenId is %{public}d, flag is %{public}i, uriVec size is %{public}zu",
        targetTokenId, flag, uriVec.size());
    bool persistableFlag = (flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION);
    if (persistableFlag != 0 && uriPermissionRdb_ == nullptr) {
        HILOG_ERROR("rbd manager is null.");
        return INNER_ERR;
    }
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        HILOG_ERROR("ConnectManager failed");
        return INNER_ERR;
    }
    auto resVec = storageManager_->CreateShareFile(uriVec, targetTokenId, flag);
    if (resVec.size() == 0) {
        HILOG_ERROR("storageManager resVec is empty.");
        return INNER_ERR;
    }
    if (resVec.size() > 0 && resVec.size() != uriVec.size()) {
        HILOG_ERROR("Failed to createShareFile.");
        return resVec[0];
    }
    int successCount = 0;
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto ret = resVec[i];
        if (ret != 0 && ret != -EEXIST) {
            HILOG_ERROR("failed to CreateShareFile.");
            continue;
        }
        auto uriStr = uriVec[i];
        auto fromTokenId = fromTokenIdVec[i];
        if (persistableFlag == 0) {
            auto addTempInfoRet = AddTempUriPermission(uriStr, flag, fromTokenId, targetTokenId, autoremove);
            successCount += (addTempInfoRet == ERR_OK ? 1 : 0);
            continue;
        }
        // grant persistable uri permission
        auto addInfoRet = uriPermissionRdb_->AddGrantInfo(uriStr, flag, fromTokenId, targetTokenId);
        if (addInfoRet == ERR_OK) {
            successCount++;
            DeletTempUriPermission(uriStr, flag, targetTokenId);
        }
    }
    HILOG_DEBUG("total %{public}i uri permissions added.", successCount);
    if (successCount == 0) {
        return INNER_ERR;
    }
    return ERR_OK;
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
        HILOG_ERROR("ConnectStorageManager failed.");
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
    if (uriPermissionRdb_ == nullptr) {
        HILOG_ERROR("rdb manager is nullptr");
        return INNER_ERR;
    }
    return uriPermissionRdb_->RemoveGrantInfo(tokenId, storageManager_);
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
    auto uriTokenId = GetTokenIdByBundleName(authority, 0);
    auto tokenId = GetTokenIdByBundleName(bundleName, 0);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto permission = IsAuthorizationUriAllowed(callerTokenId);
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
        if (uriPermissionRdb_ == nullptr) {
            HILOG_ERROR("rdb manager is nullptr");
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
        HILOG_ERROR("QueryActiveOsAccountIds error.");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        HILOG_ERROR("%{public}s, the QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}

void UriPermissionManagerStubImpl::InitPersistableUriPermissionConfig()
{
    char value[GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_SIZE] = "false";
    int retSysParam = GetParameter(GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_PARAMETER, "false", value,
        GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_SIZE);
    HILOG_INFO("GrantPersistableUriPermissionEnable, %{public}s value is %{public}s",
        GRANT_PERSISTABLE_URI_PERMISSION_ENABLE_PARAMETER, value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        isGrantPersistableUriPermissionEnable_ = true;
    }
}

void UriPermissionManagerStubImpl::SendEvent(const Uri &uri, const std::string &targetBundleName,
    uint32_t targetTokenId, const std::vector<std::string> &uriVec)
{
    auto isSystemAppCall = PermissionVerification::GetInstance()->IsSystemAppCall();
    auto targetIsSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(targetTokenId);
    if (isSystemAppCall && !targetIsSystemApp) {
        EventInfo eventInfo;
        Uri uri_inner = uri;
        eventInfo.bundleName = targetBundleName;
        eventInfo.callerBundleName = uri_inner.GetAuthority();
        if (uriVec.size() != 0) {
            for (const auto &item : uriVec) {
                eventInfo.uri = item;
                EventReport::SendKeyEvent(EventName::GRANT_URI_PERMISSION, HiSysEventType::BEHAVIOR, eventInfo);
            }
        } else {
            eventInfo.uri = uri_inner.ToString();
            EventReport::SendKeyEvent(EventName::GRANT_URI_PERMISSION, HiSysEventType::BEHAVIOR, eventInfo);
        }
    } else {
        HILOG_INFO("caller is not SA or callee is SA");
    }
}

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1Inner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
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
        return GrantUriPermission(otherVec, flag, targetBundleName, appIndex);
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
        }
        // The current processing starts from API 11 and maintains 5 versions.
        if (((policyFlag & IS_POLICY_ALLOWED_TO_BE_PRESISTED) != 0) && isSystemAppCall) {
            std::vector<uint32_t> persistResult;
            persistPermission(policyVec, persistResult);
        }
    }
}

bool UriPermissionManagerStubImpl::IsFoundationCall()
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::NativeTokenInfo nativeInfo;
    Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    HILOG_DEBUG("Caller process name : %{public}s", nativeInfo.processName.c_str());
    if (nativeInfo.processName == "foundation") {
        return true;
    }
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS