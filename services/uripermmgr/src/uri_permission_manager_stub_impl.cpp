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
#include "hilog_tag_wrapper.h"
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
constexpr uint32_t FLAG_READ_WRITE_URI = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr const char* CLOUND_DOCS_URI_MARK = "?networkid=";
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";
constexpr const char* LINUX_FUSION_SERVICE = "linux_fusion_service";
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
{
    // verify if tokenId have uri permission record
    auto uriStr = uri.ToString();
    TAG_LOGI(AAFwkTag::URIPERMMGR, "uri is %{private}s, flag is %{public}u, tokenId is %{public}u",
        uriStr.c_str(), flag, tokenId);
    if (!IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return false;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    if (search != uriMap_.end()) {
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | Want::FLAG_AUTH_READ_URI_PERMISSION) & flag) != 0) {
                TAG_LOGI(AAFwkTag::URIPERMMGR, "have uri permission.");
                return true;
            }
        }
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri permission not exists.");
    return false;
}

bool UriPermissionManagerStubImpl::IsAuthorizationUriAllowed(uint32_t fromTokenId)
{
    if (!IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return false;
    }
    return DelayedSingleton<ProxyAuthorizationUriConfig>::GetInstance()->IsAuthorizationUriAllowed(fromTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri is %{private}s.", uri.ToString().c_str());
    if (!IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return CHECK_PERMISSION_FAILED;
    }
    std::vector<Uri> uriVec = { uri };
    return GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d, size of uriVec is %{public}zu.",
        targetBundleName.c_str(), appIndex, uriVec.size());
    if (!IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return CHECK_PERMISSION_FAILED;
    }
    auto checkResult = CheckCalledBySandBox();
    if (checkResult != ERR_OK) {
        return checkResult;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid, value is %{public}u.", flag);
        return ERR_CODE_INVALID_URI_FLAG;
    }
    if (AppUtils::GetInstance().IsGrantPersistUriPermission()) {
        bool isSystemAppCall = PermissionVerification::GetInstance()->IsSystemAppCall();
        if (IsFoundationCall()) {
            isSystemAppCall = CheckIsSystemAppByTokenId(initiatorTokenId);
        }
        return GrantUriPermissionFor2In1Inner(
            uriVec, flag, targetBundleName, appIndex, isSystemAppCall, initiatorTokenId);
    }
    return GrantUriPermissionInner(uriVec, flag, targetBundleName, appIndex, initiatorTokenId);
}

int32_t UriPermissionManagerStubImpl::GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d, size of uriVec is %{public}zu.",
        targetBundleName.c_str(), appIndex, uriVec.size());
    uint32_t callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto callerName = GetTokenName(callerTokenId);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "callerTokenId is %{public}u, callerName is %{public}s",
        callerTokenId, callerName.c_str());
    auto permissionName = PermissionConstants::PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED;
    if (!PermissionVerification::GetInstance()->VerifyPermissionByTokenId(callerTokenId, permissionName) &&
        !IsLinuxFusionCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to call.");
        return CHECK_PERMISSION_FAILED;
    }

    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid, value is %{public}u.", flag);
        return ERR_CODE_INVALID_URI_FLAG;
    }
    uint32_t targetTokenId = 0;
    auto ret = GetTokenIdByBundleName(targetBundleName, 0, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get tokenId failed, bundlename is %{public}s.", targetBundleName.c_str());
        return ret;
    }

    uint32_t autoRemove = IsFoundationCall() ? 1 : 0;
    if (AppUtils::GetInstance().IsGrantPersistUriPermission()) {
        return GrantBatchUriPermissionFor2In1Privileged(uriVec, flag, callerTokenId, targetTokenId, autoRemove);
    }
    return GrantBatchUriPermissionPrivileged(uriVec, flag, callerTokenId, targetTokenId, autoRemove);
}

int UriPermissionManagerStubImpl::GrantUriPermissionInner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Called.");
    flag &= FLAG_READ_WRITE_URI;
    uint32_t targetTokenId = 0;
    auto ret = GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId of target bundle name failed.");
        return ret;
    }
    // autoRemove will be set to 1 if the process name is foundation.
    uint32_t autoRemove = 0;
    uint32_t appTokenId = IPCSkeleton::GetCallingTokenID();
    if (IsFoundationCall()) {
        autoRemove = 1;
        appTokenId = initiatorTokenId;
        auto callerName = GetTokenName(appTokenId);
        TAG_LOGI(AAFwkTag::URIPERMMGR, "RealTokenId is %{public}u, RealCallerName is %{public}s.",
            appTokenId, callerName.c_str());
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
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Called, result size is %{public}zu", result.size());
    return 0;
}

int32_t setPolicy(uint64_t tokenId, const std::vector<PolicyInfo> &policy, uint64_t policyFlag)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Called, policy size is %{public}zu", policy.size());
    return 0;
}

int persistPermission(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result)
{
    for (size_t i = 0; i < policy.size(); i++) {
        result.emplace_back(0);
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Called, result size is %{public}zu", result.size());
    return 0;
}

int32_t UriPermissionManagerStubImpl::CheckCalledBySandBox()
{
    // reject sandbox to grant uri permission
    ConnectManager(appMgr_, APP_MGR_SERVICE_ID);
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get BundleManager failed!");
        return INNER_ERR;
    }
    auto callerPid = IPCSkeleton::GetCallingRealPid();
    bool isSandbox = false;
    if (appMgr_->JudgeSandboxByPid(callerPid, isSandbox) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "JudgeSandboxByPid failed.");
        return INNER_ERR;
    }
    if (isSandbox) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Sandbox application can not grant URI permission.");
        return ERR_CODE_GRANT_URI_PERMISSION;
    }
    return ERR_OK;
}

// To be deleted.
int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Called.");
    if (!IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Not foundation call.");
        return CHECK_PERMISSION_FAILED;
    }
    auto checkResult = CheckCalledBySandBox();
    if (checkResult != ERR_OK) {
        return checkResult;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is %{public}u, which is invalid.", flag);
        return ERR_CODE_INVALID_URI_FLAG;
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
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Insert an uri r/w permission.");
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uri, infoList);
        return ERR_OK;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            TAG_LOGD(AAFwkTag::URIPERMMGR, "Item: flag is %{public}u, autoRemove is %{public}u.",
                item.flag, item.autoRemove);
            if (item.autoRemove == 1 && info.autoRemove == 0) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "Update autoRemove.");
                item.autoRemove = 1;
            }
            if ((flag & (item.flag | Want::FLAG_AUTH_READ_URI_PERMISSION)) == 0) {
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Update uri r/w permission.");
                item.flag = flag;
            } else {
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri has been granted, not to grant again.");
            }
            return ERR_OK;
        }
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Insert an new uri permission record.");
    infoList.emplace_back(info);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
    TokenId callerTokenId, TokenId targetTokenId, uint32_t autoRemove)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "uri = %{private}s, flag = %{public}i, callerTokenId = %{public}i,"
        "targetTokenId = %{public}i, autoRemove = %{public}i", uri.ToString().c_str(), flag, callerTokenId,
        targetTokenId, autoRemove);
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "ConnectManager failed");
        return INNER_ERR;
    }
    auto uriStr = uri.ToString();
    std::vector<std::string> uriVec = { uriStr };
    auto resVec = storageManager_->CreateShareFile(uriVec, targetTokenId, flag);
    if (resVec.size() == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "storageManager resVec is empty.");
        return INNER_ERR;
    }
    if (resVec[0] != 0 && resVec[0] != -EEXIST) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "failed to CreateShareFile.");
        return INNER_ERR;
    }
    AddTempUriPermission(uriStr, flag, callerTokenId, targetTokenId, autoRemove);
    SendEvent(callerTokenId, targetTokenId, uriStr);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantSingleUriPermission(const Uri &uri, unsigned int flag, uint32_t callerTokenId,
    uint32_t targetTokenId, uint32_t autoRemove)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "uri is %{private}s, callerTokenId is %{public}u, targetTokenId is %{public}u, autoRemove is %{public}u",
        uri.ToString().c_str(), callerTokenId, targetTokenId, autoRemove);
    if (!CheckUriTypeIsValid(uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s", uri.ToString().c_str());
        return ERR_CODE_INVALID_URI_TYPE;
    }
    TokenIdPermission tokenIdPermission(callerTokenId);
    if (!CheckUriPermission(uri, flag, tokenIdPermission)) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "No permission, uri is %{private}s, callerTokenId is %{public}u",
            uri.ToString().c_str(), callerTokenId);
        return CHECK_PERMISSION_FAILED;
    }
    return GrantUriPermissionImpl(uri, flag, callerTokenId, targetTokenId, autoRemove);
}

int UriPermissionManagerStubImpl::GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec,
    unsigned int flag, TokenId callerTokenId, TokenId targetTokenId, uint32_t autoRemove)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}i,"
        "list size is %{public}zu", callerTokenId, targetTokenId, flag, uriVec.size());
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "ConnectManager failed.");
        return INNER_ERR;
    }
    auto resVec = storageManager_->CreateShareFile(uriVec, targetTokenId, flag);
    if (resVec.size() == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to createShareFile, storageManager resVec is empty.");
        return INNER_ERR;
    }
    if (resVec.size() != uriVec.size()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to createShareFile, ret is %{public}u", resVec[0]);
        return resVec[0];
    }
    EventInfo eventInfo;
    bool needSendEvent = CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);
    int successCount = 0;
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto ret = resVec[i];
        if (ret != 0 && ret != -EEXIST) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "failed to CreateShareFile.");
            continue;
        }
        AddTempUriPermission(uriVec[i], flag, callerTokenId, targetTokenId, autoRemove);
        if (needSendEvent) {
            eventInfo.uri = uriVec[i];
            EventReport::SendKeyEvent(EventName::GRANT_URI_PERMISSION, HiSysEventType::BEHAVIOR, eventInfo);
        }
        successCount++;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "total %{public}d uri permissions added.", successCount);
    if (successCount == 0) {
        return INNER_ERR;
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantBatchUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    uint32_t callerTokenId, uint32_t targetTokenId, uint32_t autoRemove)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}u, autoRemove is %{public}u.",
        callerTokenId, targetTokenId, flag, autoRemove);
    TokenIdPermission tokenIdPermission(callerTokenId);
    std::vector<std::string> uriStrVec = {};
    for (const auto &uri : uriVec) {
        if (!CheckUriTypeIsValid(uri)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s", uri.ToString().c_str());
            continue;
        }
        if (!CheckUriPermission(uri, flag, tokenIdPermission)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "No permission, uri is %{private}s.", uri.ToString().c_str());
            continue;
        }
        uriStrVec.emplace_back(uri.ToString());
    }
    if (uriStrVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Valid uri list is empty.");
        return INNER_ERR;
    }
    return GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, autoRemove);
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    uint32_t callerTokenId, uint32_t targetTokenId, uint32_t autoRemove)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}u.",
        callerTokenId, targetTokenId, flag);
    std::vector<std::string> uriStrVec = {};
    for (const auto &uri : uriVec) {
        if (!CheckUriTypeIsValid(uri)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s.", uri.ToString().c_str());
            continue;
        }
        uriStrVec.emplace_back(uri.ToString());
    }
    if (uriStrVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Valid uri list is empty.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    return GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, autoRemove);
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionFor2In1Privileged(const std::vector<Uri> &uriVec,
    uint32_t flag, uint32_t callerTokenId, uint32_t targetTokenId, uint32_t autoRemove)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}u.",
        callerTokenId, targetTokenId, flag);
    std::vector<std::string> uriStrVec = {};
    std::vector<PolicyInfo> docsVec = {};
    for (const auto &uri : uriVec) {
        auto uriInner = uri;
        auto uriStr = uriInner.ToString();
        if (!CheckUriTypeIsValid(uri)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s.", uriStr.c_str());
            continue;
        }
        auto &&authority = uriInner.GetAuthority();
        if (authority != "docs" || uriStr.find(CLOUND_DOCS_URI_MARK) == std::string::npos) {
            uriStrVec.emplace_back(uriStr);
            continue;
        }
        PolicyInfo policyInfo;
        policyInfo.path = uriStr;
        policyInfo.mode = (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) == 0 ? READ_MODE : WRITE_MODE;
        docsVec.emplace_back(policyInfo);
    }

    if (uriStrVec.empty() && docsVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Valid uri list is empty.");
        return ERR_CODE_INVALID_URI_TYPE;
    }

    if (!uriStrVec.empty()) {
        auto ret = GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, autoRemove);
        if (docsVec.empty()) {
            return ret;
        }
    }

    bool isSystemAppCall = PermissionVerification::GetInstance()->IsSystemAppCall();
    HandleUriPermission(targetTokenId, flag, docsVec, isSystemAppCall);
    return ERR_OK;
}

void UriPermissionManagerStubImpl::RevokeUriPermission(const TokenId tokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Start to remove uri permission, tokenId is %{public}u", tokenId);
    if (!IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to revoke uri permission.");
        return;
    }
    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end(); it++) {
                if (it->targetTokenId == tokenId && it->autoRemove) {
                    TAG_LOGI(AAFwkTag::URIPERMMGR, "Erase an info form list.");
                    list.erase(it);
                    uriList.emplace_back(iter->first);
                    break;
                }
            }
            if (list.empty()) {
                uriMap_.erase(iter++);
                continue;
            }
            iter++;
        }
    }
    if (!uriList.empty()) {
        DeleteShareFile(tokenId, uriList);
    }
}

int UriPermissionManagerStubImpl::RevokeAllUriPermissions(uint32_t tokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Start to revoke all uri permission, tokenId is %{public}u.", tokenId);
    if (!IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to revoke all uri permission.");
        return CHECK_PERMISSION_FAILED;
    }
    std::map<uint32_t, std::vector<std::string>> uriLists;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
            uint32_t authorityTokenId = 0;
            auto authority = Uri(iter->first).GetAuthority();
            // uri belong to target tokenId.
            auto ret = GetTokenIdByBundleName(authority, 0, authorityTokenId);
            if (ret == ERR_OK && authorityTokenId == tokenId) {
                for (const auto &record : iter->second) {
                    uriLists[record.targetTokenId].emplace_back(iter->first);
                }
                uriMap_.erase(iter++);
                continue;
            }
            auto& list = iter->second;
            for (auto it = list.begin(); it != list.end();) {
                if (it->targetTokenId == tokenId || it->fromTokenId == tokenId) {
                    TAG_LOGI(AAFwkTag::URIPERMMGR, "Erase an uri permission record.");
                    uriLists[it->targetTokenId].emplace_back(iter->first);
                    list.erase(it++);
                    continue;
                }
                it++;
            }
            if (list.empty()) {
                uriMap_.erase(iter++);
                continue;
            }
            iter++;
        }
    }

    for (auto iter = uriLists.begin(); iter != uriLists.end(); iter++) {
        if (DeleteShareFile(iter->first, iter->second) != ERR_OK) {
            return INNER_ERR;
        }
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Revoke uri permission manually, uri is %{private}s, bundleName is %{public}s",
        uri.ToString().c_str(), bundleName.c_str());
    if (!IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!CheckUriTypeIsValid(uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s.", uri.ToString().c_str());
        return ERR_CODE_INVALID_URI_TYPE;
    }
    uint32_t targetTokenId = 0;
    auto ret = GetTokenIdByBundleName(bundleName, 0, targetTokenId);
    if (ret != ERR_OK) {
        return ret;
    }

    auto uriStr = uri.ToString();
    auto uriInner = uri;
    uint32_t authorityTokenId = 0;
    GetTokenIdByBundleName(uriInner.GetAuthority(), 0, authorityTokenId);
    // uri belong to caller or caller is target.
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    bool isRevokeSelfUri = (callerTokenId == targetTokenId || callerTokenId == authorityTokenId);
    std::vector<std::string> uriList;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        auto search = uriMap_.find(uriStr);
        if (search == uriMap_.end()) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "URI does not exist on uri map.");
            return ERR_OK;
        }
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if (it->targetTokenId == targetTokenId && (callerTokenId == it->fromTokenId || isRevokeSelfUri)) {
                uriList.emplace_back(search->first);
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Revoke an uri permission record.");
                list.erase(it);
                break;
            }
        }
        if (list.empty()) {
            uriMap_.erase(search);
        }
    }
    return DeleteShareFile(targetTokenId, uriList);
}

int32_t UriPermissionManagerStubImpl::DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uriVec)
{
    ConnectManager(storageManager_, STORAGE_MANAGER_MANAGER_ID);
    if (storageManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Connect StorageManager failed.");
        return INNER_ERR;
    }
    auto ret = storageManager_->DeleteShareFile(targetTokenId, uriVec);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "DeleteShareFile failed, errorCode is %{public}d.", ret);
    }
    return ret;
}

std::vector<bool> UriPermissionManagerStubImpl::CheckUriAuthorization(const std::vector<std::string> &uriVec,
    uint32_t flag, uint32_t tokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "tokenId is %{public}u, tokenName is %{public}s, flag is %{public}u, size of uris is %{public}zu",
        tokenId, GetTokenName(tokenId).c_str(), flag, uriVec.size());
    std::vector<bool> result(uriVec.size(), false);
    if (!IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return result;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid.");
        return result;
    }

    TokenIdPermission tokenIdPermission(tokenId);
    for (size_t i = 0; i < uriVec.size(); i++) {
        Uri uri(uriVec[i]);
        if (!CheckUriTypeIsValid(uri)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "uri is invalid, uri is %{private}s.", uriVec[i].c_str());
            continue;
        }
        result[i] = CheckUriPermission(uri, flag, tokenIdPermission);
        if (!result[i]) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Check uri permission failed, uri is %{private}s.", uriVec[i].c_str());
        }
    }
    return result;
}

template<typename T>
void UriPermissionManagerStubImpl::ConnectManager(sptr<T> &mgr, int32_t serviceId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Call.");
    std::lock_guard<std::mutex> lock(mgrMutex_);
    if (mgr == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "mgr is nullptr.");
        auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemAbilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to get SystemAbilityManager.");
            return;
        }

        auto remoteObj = systemAbilityMgr->GetSystemAbility(serviceId);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to get mgr.");
            return;
        }
        TAG_LOGE(AAFwkTag::URIPERMMGR, "to cast.");
        mgr = iface_cast<T>(remoteObj);
        if (mgr == nullptr) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to cast.");
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
            TAG_LOGE(AAFwkTag::URIPERMMGR, "AddDeathRecipient failed.");
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

int32_t UriPermissionManagerStubImpl::GetTokenIdByBundleName(const std::string &bundleName, int32_t appIndex,
    uint32_t &tokenId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d.", bundleName.c_str(), appIndex);
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "The bundleMgrHelper is nullptr.");
        return GET_BUNDLE_MANAGER_SERVICE_FAILED;
    }
    auto bundleFlag = AppExecFwk::BundleFlag::GET_BUNDLE_WITH_EXTENSION_INFO;
    AppExecFwk::BundleInfo bundleInfo;
    auto userId = GetCurrentAccountId();
    if (appIndex == 0) {
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName, bundleFlag, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to get bundle info according to uri.");
            return GET_BUNDLE_INFO_FAILED;
        }
        tokenId = bundleInfo.applicationInfo.accessTokenId;
        return ERR_OK;
    }
    if (IN_PROCESS_CALL(bundleMgrHelper->GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo) != ERR_OK)) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to get sandbox bundle info according to appIndex.");
        return GET_BUNDLE_INFO_FAILED;
    }
    tokenId = bundleInfo.applicationInfo.accessTokenId;
    return ERR_OK;
}

void UriPermissionManagerStubImpl::ProxyDeathRecipient::OnRemoteDied([[maybe_unused]]
    const wptr<IRemoteObject>& remote)
{
    if (proxy_) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "mgr stub died.");
        proxy_(remote);
    }
}

int32_t UriPermissionManagerStubImpl::GetCurrentAccountId() const
{
    std::vector<int32_t> osActiveAccountIds;
    auto ret = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()->
        QueryActiveOsAccountIds(osActiveAccountIds);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "QueryActiveOsAccountIds error.");
        return DEFAULT_USER_ID;
    }
    if (osActiveAccountIds.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "%{public}s, the QueryActiveOsAccountIds is empty, no accounts.", __func__);
        return DEFAULT_USER_ID;
    }

    return osActiveAccountIds.front();
}

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1Inner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall, uint32_t initiatorTokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriVec size is %{public}zu, targetBundleName is %{public}s",
        uriVec.size(), targetBundleName.c_str());
    std::vector<PolicyInfo> docsVec;
    std::vector<Uri> otherVec;
    for (const auto &uri : uriVec) {
        Uri uri_inner = uri;
        auto &&scheme = uri_inner.GetScheme();
        if (scheme != "file") {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Only support file uri.");
            continue;
        }
        auto &&authority = uri_inner.GetAuthority();
        TAG_LOGD(AAFwkTag::URIPERMMGR, "The authority is %{public}s", authority.c_str());
        PolicyInfo policyInfo;
        policyInfo.path = uri_inner.ToString();
        if ((flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0) {
            policyInfo.mode |= WRITE_MODE;
        } else {
            policyInfo.mode |= READ_MODE;
        }
        if (authority == "docs" && uri.ToString().find(CLOUND_DOCS_URI_MARK) == std::string::npos) {
            docsVec.emplace_back(policyInfo);
        } else {
            otherVec.emplace_back(uri_inner);
        }
    }
    if (!otherVec.empty()) {
        auto ret = GrantUriPermissionInner(otherVec, flag, targetBundleName, appIndex, initiatorTokenId);
        if (docsVec.empty()) {
            return ret;
        }
    }
    uint32_t tokenId = 0;
    auto ret = GetTokenIdByBundleName(targetBundleName, appIndex, tokenId);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "The tokenId is %{public}u", tokenId);
    HandleUriPermission(tokenId, flag, docsVec, isSystemAppCall);
    return ERR_OK;
}

void UriPermissionManagerStubImpl::HandleUriPermission(
    uint64_t tokenId, unsigned int flag, std::vector<PolicyInfo> &docsVec, bool isSystemAppCall)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "HandleUriPermission called.");
    uint32_t policyFlag = 0;
    if ((flag & Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION) != 0) {
        policyFlag |= IS_POLICY_ALLOWED_TO_BE_PRESISTED;
    }
    // Handle docs type URI permission
    if (!docsVec.empty()) {
        std::vector<bool> result;
        checkPersistPermission(tokenId, docsVec, result);
        if (docsVec.size() != result.size()) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Check persist permission failed.");
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerTokenId is %{public}u", callerTokenId);
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Is not native call");
        return false;
    }
    Security::AccessToken::NativeTokenInfo nativeInfo;
    auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GetNativeTokenInfo failed, callerTokenId is %{public}u.", callerTokenId);
        return false;
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Caller process name : %{public}s", nativeInfo.processName.c_str());
    return nativeInfo.processName == FOUNDATION_PROCESS_NAME;
}

bool UriPermissionManagerStubImpl::IsLinuxFusionCall()
{
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callerTokenId is %{public}u", callerTokenId);
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Is not native call");
        return false;
    }
    Security::AccessToken::NativeTokenInfo nativeInfo;
    auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GetNativeTokenInfo failed, callerTokenId is %{public}u.", callerTokenId);
        return false;
    }
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Caller process name : %{public}s", nativeInfo.processName.c_str());
    return nativeInfo.processName == LINUX_FUSION_SERVICE;
}

std::string UriPermissionManagerStubImpl::GetTokenName(uint32_t callerTokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerTokenId);
    if (tokenType == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        Security::AccessToken::NativeTokenInfo nativeInfo;
        auto result = Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(callerTokenId, nativeInfo);
        if (result != ERR_OK) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "GetNativeTokenInfo failed, callerTokenId is %{public}u.", callerTokenId);
            return "";
        }
        return nativeInfo.processName;
    }
    return GetBundleNameByTokenId(callerTokenId);
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
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Caller bundle name is empty or target bundle name is empty.");
        return false;
    }
    auto isSystemAppCall = CheckIsSystemAppByBundleName(callerBundleName);
    auto targetIsSystemApp = CheckIsSystemAppByBundleName(targetBundleName);
    if (!isSystemAppCall || targetIsSystemApp) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "Caller is not system app or callee is system app.");
        return false;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Send Grant_Uri_Permission event.");
    eventInfo.callerBundleName = callerBundleName;
    eventInfo.bundleName = targetBundleName;
    return true;
}

std::string UriPermissionManagerStubImpl::GetBundleNameByTokenId(uint32_t tokenId)
{
    Security::AccessToken::HapTokenInfo hapInfo;
    auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
    if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "GetHapTokenInfo failed, ret is %{public}i.", ret);
        return "";
    }
    return hapInfo.bundleName;
}

bool UriPermissionManagerStubImpl::CheckIsSystemAppByBundleName(std::string &bundleName)
{
    auto bundleMgrHelper = ConnectManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "The bundleMgrHelper is nullptr.");
        return false;
    }
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, GetCurrentAccountId(), appInfo))) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Get application info failed.");
        return false;
    }
    auto isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(appInfo.accessTokenIdEx);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, isSystemApp = %{public}i", bundleName.c_str(),
        static_cast<int32_t>(isSystemApp));
    return isSystemApp;
}

bool UriPermissionManagerStubImpl::CheckIsSystemAppByTokenId(uint32_t tokenId)
{
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "tokenType is %{public}d.", tokenType);
        return false;
    }
    auto bundleName = GetBundleNameByTokenId(tokenId);
    if (!bundleName.empty()) {
        return CheckIsSystemAppByBundleName(bundleName);
    }
    return false;
}

bool UriPermissionManagerStubImpl::CheckUriPermission(Uri uri, uint32_t flag, TokenIdPermission &tokenIdPermission)
{
    auto &&authority = uri.GetAuthority();
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Authority of uri is %{public}s", authority.c_str());
    if (IsLinuxFusionCall()) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Caller is linux_fusion_service.");
        return true;
    }
    if (authority == "docs") {
        return AccessDocsUriPermission(tokenIdPermission, uri, flag);
    }
    if (authority == "media") {
        return AccessMediaUriPermission(tokenIdPermission, uri, flag);
    }
    uint32_t authorityTokenId = 0;
    if (GetTokenIdByBundleName(authority, 0, authorityTokenId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get tokenId of %{public}s failed.", authority.c_str());
        return false;
    }
    if (tokenIdPermission.GetTokenId() == authorityTokenId) {
        return true;
    }
    return CheckProxyUriPermission(tokenIdPermission, uri, flag);
}

bool UriPermissionManagerStubImpl::AccessMediaUriPermission(TokenIdPermission &tokenIdPermission,
    const Uri &uri, uint32_t flag)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Call AccessMediaUriPermission.");
    bool isWriteFlag = (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) != 0;
    auto innerUri = uri;
    auto path = innerUri.GetPath();
    if (path.rfind("/Photo/", 0) == 0) {
        if (tokenIdPermission.VerifyWriteImageVideoPermission()) {
            return true;
        }
        if (!isWriteFlag && tokenIdPermission.VerifyReadImageVideoPermission()) {
            return true;
        }
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Do not have IMAGEVIDEO Permission.");
        return CheckProxyUriPermission(tokenIdPermission, uri, flag);
    }
    if (path.rfind("/Audio/", 0) == 0) {
        if (tokenIdPermission.VerifyWriteAudioPermission()) {
            return true;
        }
        if (!isWriteFlag && tokenIdPermission.VerifyReadAudioPermission()) {
            return true;
        }
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Do not have AUDIO Permission.");
        return CheckProxyUriPermission(tokenIdPermission, uri, flag);
    }
    TAG_LOGE(AAFwkTag::URIPERMMGR, "Media uri is invalid, path is %{public}s", path.c_str());
    return false;
}

bool UriPermissionManagerStubImpl::AccessDocsUriPermission(TokenIdPermission &tokenIdPermission,
    const Uri &uri, uint32_t flag)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Call AccessDocsUriPermission.");
    if (tokenIdPermission.VerifyFileAccessManagerPermission()) {
        return true;
    }
    TAG_LOGW(AAFwkTag::URIPERMMGR, "Do not have FILE_ACCESS_MANAGER Permission.");
    return CheckProxyUriPermission(tokenIdPermission, uri, flag);
}

int32_t UriPermissionManagerStubImpl::CheckProxyUriPermission(TokenIdPermission &tokenIdPermission,
    const Uri &uri, uint32_t flag)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Call CheckProxyUriPermission.");
    auto tokenId = tokenIdPermission.GetTokenId();
    if (tokenIdPermission.VerifyProxyAuthorizationUriPermission() && VerifyUriPermission(uri, flag, tokenId)) {
        return true;
    }
    TAG_LOGW(AAFwkTag::URIPERMMGR, "Check proxy uri permission failed.");
    return false;
}

bool UriPermissionManagerStubImpl::CheckUriTypeIsValid(Uri uri)
{
    auto &&scheme = uri.GetScheme();
    if (scheme != "file" && scheme != "content") {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Type of uri is invalid, Scheme is %{public}s", scheme.c_str());
        return false;
    }
    return true;
}

bool UriPermissionManagerStubImpl::IsSAOrSystemAppCall()
{
    return PermissionVerification::GetInstance()->IsSystemAppCall() ||
        PermissionVerification::GetInstance()->IsSACall();
}
}  // namespace AAFwk
}  // namespace OHOS