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
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "parameter.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "uri_permission_utils.h"
#include "want.h"

#define READ_MODE (1<<0)
#define WRITE_MODE (1<<1)
#define IS_POLICY_ALLOWED_TO_BE_PRESISTED (1<<0)

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr uint32_t FLAG_READ_WRITE_URI = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr uint32_t FLAG_WRITE_URI = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr uint32_t FLAG_READ_URI = Want::FLAG_AUTH_READ_URI_PERMISSION;
constexpr const char* CLOUND_DOCS_URI_MARK = "?networkid=";
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
{
    // verify if tokenId have uri permission record
    auto uriStr = uri.ToString();
    TAG_LOGD(AAFwkTag::URIPERMMGR, "uri is %{private}s, flag is %{public}u, tokenId is %{public}u",
        uriStr.c_str(), flag, tokenId);
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return false;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid.");
        return false;
    }
    uint32_t newFlag = FLAG_READ_URI;
    if ((flag & FLAG_WRITE_URI) != 0) {
        newFlag = FLAG_WRITE_URI;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uriStr);
    if (search != uriMap_.end()) {
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | FLAG_READ_URI) & newFlag) != 0) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "have uri permission.");
                return true;
            }
        }
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri permission not exists.");
        return false;
    }
    return VerifySubDirUriPermission(uriStr, newFlag, tokenId);
}

bool UriPermissionManagerStubImpl::VerifySubDirUriPermission(const std::string &uriStr,
                                                             uint32_t newFlag, uint32_t tokenId)
{
    auto iPos = uriStr.find(CLOUND_DOCS_URI_MARK);
    if (iPos == std::string::npos) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Local uri not support to verify sub directory uri permission.");
        return false;
    }

    for (auto search = uriMap_.rbegin(); search != uriMap_.rend(); ++search) {
        if (!IsDistributedSubDirUri(uriStr, search->first)) {
            continue;
        }
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | FLAG_READ_URI) & newFlag) != 0) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "have uri permission.");
                return true;
            }
        }
        break;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri permission not exists.");
    return false;
}

bool UriPermissionManagerStubImpl::IsDistributedSubDirUri(const std::string &inputUri, const std::string &cachedUri)
{
    auto iPos = inputUri.find(CLOUND_DOCS_URI_MARK);
    auto cPos = cachedUri.find(CLOUND_DOCS_URI_MARK);
    if ((iPos == std::string::npos) || (cPos == std::string::npos)) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "The uri is not distributed file uri.");
        return false;
    }
    std::string iTempUri = inputUri.substr(0, iPos);
    std::string cTempUri = cachedUri.substr(0, cPos);
    return iTempUri.find(cTempUri + "/") == 0;
}

int UriPermissionManagerStubImpl::GrantUriPermission(const Uri &uri, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t abilityId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri is %{private}s.", uri.ToString().c_str());
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return CHECK_PERMISSION_FAILED;
    }
    std::vector<Uri> uriVec = { uri };
    return GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
}

int UriPermissionManagerStubImpl::GrantUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t abilityId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d, size of uriVec is %{public}zu.",
        targetBundleName.c_str(), appIndex, uriVec.size());
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
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
        bool isSystemAppCall = UPMSUtils::IsSystemAppCall(initiatorTokenId);
        return GrantUriPermissionFor2In1Inner(
            uriVec, flag, targetBundleName, appIndex, isSystemAppCall, initiatorTokenId, abilityId);
    }
    return GrantUriPermissionInner(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
}

int32_t UriPermissionManagerStubImpl::GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d, size of uriVec is %{public}zu.",
        targetBundleName.c_str(), appIndex, uriVec.size());

    uint32_t callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto callerName = UPMSUtils::GetCallerNameByTokenId(callerTokenId);
    TAG_LOGD(AAFwkTag::URIPERMMGR, "callerTokenId is %{public}u, callerName is %{public}s",
        callerTokenId, callerName.c_str());

    auto permissionName = PermissionConstants::PERMISSION_GRANT_URI_PERMISSION_PRIVILEGED;
    if (!PermissionVerification::GetInstance()->VerifyPermissionByTokenId(callerTokenId, permissionName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to call.");
        return CHECK_PERMISSION_FAILED;
    }

    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid, value is %{public}u.", flag);
        return ERR_CODE_INVALID_URI_FLAG;
    }
    flag &= FLAG_READ_WRITE_URI;
    uint32_t targetTokenId = 0;
    auto ret = UPMSUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get tokenId failed, bundlename is %{public}s.", targetBundleName.c_str());
        return ret;
    }

    if (AppUtils::GetInstance().IsGrantPersistUriPermission()) {
        return GrantBatchUriPermissionFor2In1Privileged(uriVec, flag, callerTokenId, targetTokenId);
    }
    return GrantBatchUriPermissionPrivileged(uriVec, flag, callerTokenId, targetTokenId);
}

int UriPermissionManagerStubImpl::GrantUriPermissionInner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string targetBundleName, int32_t appIndex, uint32_t initiatorTokenId, int32_t abilityId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Called.");
    flag &= FLAG_READ_WRITE_URI;
    uint32_t targetTokenId = 0;
    auto ret = UPMSUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId of target bundle name failed.");
        return ret;
    }
    // recordId will be set default id if the process name is not foundation.
    int32_t recordId = -1;
    uint32_t appTokenId = IPCSkeleton::GetCallingTokenID();
    if (UPMSUtils::IsFoundationCall()) {
        recordId = abilityId;
        appTokenId = initiatorTokenId;
        auto callerName = UPMSUtils::GetCallerNameByTokenId(appTokenId);
    }
    if (uriVec.size() == 1) {
        return GrantSingleUriPermission(uriVec[0], flag, appTokenId, targetTokenId, recordId);
    }
    return GrantBatchUriPermission(uriVec, flag, appTokenId, targetTokenId, recordId);
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
    auto callerPid = IPCSkeleton::GetCallingPid();
    bool isSandbox = false;
    if (IN_PROCESS_CALL(appMgr_->JudgeSandboxByPid(callerPid, isSandbox)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "JudgeSandboxByPid failed.");
        return INNER_ERR;
    }
    if (isSandbox) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Sandbox application can not grant URI permission.");
        return ERR_CODE_GRANT_URI_PERMISSION;
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::AddTempUriPermission(const std::string &uri, unsigned int flag,
    TokenId fromTokenId, TokenId targetTokenId, int32_t abilityId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto search = uriMap_.find(uri);
    bool autoRemove = (abilityId != DEFAULT_ABILITY_ID);
    GrantInfo info = { flag, fromTokenId, targetTokenId, autoRemove, {} };
    info.AddAbilityId(abilityId);
    if (search == uriMap_.end()) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Insert an uri r/w permission.");
        std::list<GrantInfo> infoList = { info };
        uriMap_.emplace(uri, infoList);
        return ERR_OK;
    }
    auto& infoList = search->second;
    for (auto& item : infoList) {
        if (item.fromTokenId == fromTokenId && item.targetTokenId == targetTokenId) {
            TAG_LOGI(AAFwkTag::URIPERMMGR,
                "Item: flag is %{public}u, autoRemove is %{public}u, ability size is %{public}zu.",
                item.flag, item.autoRemove, item.abilityIds.size());
            item.AddAbilityId(abilityId);
            // r-w
            if ((item.flag & FLAG_WRITE_URI) == 0 && (flag & FLAG_WRITE_URI) != 0) {
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Update uri r/w permission.");
                item.autoRemove = autoRemove;
                item.flag |= FLAG_WRITE_URI;
                return ERR_OK;
            }
            // w-r
            TAG_LOGD(AAFwkTag::URIPERMMGR, "Uri has been granted, not to grant again.");
            if ((item.flag & FLAG_WRITE_URI) != 0 && (flag & FLAG_WRITE_URI) == 0) {
                return ERR_OK;
            }
            // other
            if (!autoRemove) {
                item.autoRemove = autoRemove;
            }
            return ERR_OK;
        }
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Insert a new uri permission record.");
    infoList.emplace_back(info);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantUriPermissionImpl(const Uri &uri, unsigned int flag,
    TokenId callerTokenId, TokenId targetTokenId, int32_t abilityId)
{
    TAG_LOGD(AAFwkTag::URIPERMMGR, "uri = %{private}s, flag = %{public}u, callerTokenId = %{public}u,"
        "targetTokenId = %{public}u, abilityId = %{public}d", uri.ToString().c_str(), flag, callerTokenId,
        targetTokenId, abilityId);
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
    AddTempUriPermission(uriStr, flag, callerTokenId, targetTokenId, abilityId);
    UPMSUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, resVec);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantSingleUriPermission(const Uri &uri, unsigned int flag, uint32_t callerTokenId,
    uint32_t targetTokenId, int32_t abilityId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "uri is %{private}s, callerTokenId is %{public}u, targetTokenId is %{public}u, abilityId is %{public}d",
        uri.ToString().c_str(), callerTokenId, targetTokenId, abilityId);
    if (!CheckUriTypeIsValid(uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s", uri.ToString().c_str());
        return ERR_CODE_INVALID_URI_TYPE;
    }
    TokenIdPermission tokenIdPermission(callerTokenId);
    if (!CheckUriPermission(uri, flag, tokenIdPermission)) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "No permission, uri is %{private}s, callerTokenId is %{public}u",
            uri.ToString().c_str(), callerTokenId);
        UPMSUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);
        return CHECK_PERMISSION_FAILED;
    }
    return GrantUriPermissionImpl(uri, flag, callerTokenId, targetTokenId, abilityId);
}

int UriPermissionManagerStubImpl::GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec,
    unsigned int flag, TokenId callerTokenId, TokenId targetTokenId, int32_t abilityId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,"callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}u,"
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
    int successCount = 0;
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto ret = resVec[i];
        if (ret != 0 && ret != -EEXIST) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "failed to CreateShareFile.");
            continue;
        }
        AddTempUriPermission(uriVec[i], flag, callerTokenId, targetTokenId, abilityId);
        successCount++;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "total %{public}d uri permissions added.", successCount);
    if (successCount == 0) {
        return INNER_ERR;
    }
    UPMSUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, resVec);
    return ERR_OK;
}

int UriPermissionManagerStubImpl::GrantBatchUriPermission(const std::vector<Uri> &uriVec, unsigned int flag,
    uint32_t callerTokenId, uint32_t targetTokenId, int32_t abilityId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "callerTokenId is %{public}u, targetTokenId is %{public}u, flag is %{public}u, abilityId is %{public}u.",
        callerTokenId, targetTokenId, flag, abilityId);
    TokenIdPermission tokenIdPermission(callerTokenId);
    std::vector<std::string> uriStrVec = {};
    bool checkUriPermissionFailedFlag = false;
    for (const auto &uri : uriVec) {
        if (!CheckUriTypeIsValid(uri)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s", uri.ToString().c_str());
            continue;
        }
        if (!CheckUriPermission(uri, flag, tokenIdPermission)) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "No permission, uri is %{private}s.", uri.ToString().c_str());
            checkUriPermissionFailedFlag = true;
            continue;
        }
        uriStrVec.emplace_back(uri.ToString());
    }
    if (checkUriPermissionFailedFlag) {
        UPMSUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);
    }
    if (uriStrVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Valid uri list is empty.");
        return INNER_ERR;
    }
    return GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, abilityId);
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    uint32_t callerTokenId, uint32_t targetTokenId, int32_t abilityId)
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
    return GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, abilityId);
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionFor2In1Privileged(const std::vector<Uri> &uriVec,
    uint32_t flag, uint32_t callerTokenId, uint32_t targetTokenId, int32_t abilityId)
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
        if (authority != "docs" || uriStr.find(CLOUND_DOCS_URI_MARK) != std::string::npos) {
            uriStrVec.emplace_back(uriStr);
            continue;
        }
        PolicyInfo policyInfo;
        policyInfo.path = uriStr;
        policyInfo.mode = (flag & Want::FLAG_AUTH_WRITE_URI_PERMISSION) == 0 ? READ_MODE : WRITE_MODE;
        docsVec.emplace_back(policyInfo);
    }

    TAG_LOGI(AAFwkTag::URIPERMMGR, "docsUri size is %{public}zu, otherUri size is %{public}zu",
        docsVec.size(), uriStrVec.size());

    if (uriStrVec.empty() && docsVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Valid uri list is empty.");
        return ERR_CODE_INVALID_URI_TYPE;
    }

    if (!uriStrVec.empty()) {
        auto ret = GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, abilityId);
        if (docsVec.empty()) {
            return ret;
        }
    }

    bool isSystemAppCall = PermissionVerification::GetInstance()->IsSystemAppCall();
    HandleUriPermission(targetTokenId, flag, docsVec, isSystemAppCall);
    return ERR_OK;
}

void UriPermissionManagerStubImpl::RemoveUriRecord(std::vector<std::string> &uriList, const TokenId tokenId,
    int32_t abilityId)
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto iter = uriMap_.begin(); iter != uriMap_.end();) {
        auto& list = iter->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if (it->targetTokenId != tokenId || !it->RemoveAbilityId(abilityId) || !it->autoRemove) {
                continue;
            }
            if (!it->IsEmptyAbilityId()) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "Remove an abilityId.");
                break;
            }
            TAG_LOGI(AAFwkTag::URIPERMMGR, "Erase an info form list.");
            list.erase(it);
            uriList.emplace_back(iter->first);
            break;
        }
        if (list.empty()) {
            uriMap_.erase(iter++);
            continue;
        }
        ++iter;
    }
}

void UriPermissionManagerStubImpl::RevokeUriPermission(const TokenId tokenId, int32_t abilityId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "Start to remove uri permission, tokenId is %{public}u, abilityId is %{public}d", tokenId, abilityId);
    if (!UPMSUtils::IsFoundationCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "No permission to revoke uri permission.");
        return;
    }
    std::vector<std::string> uriList;
    RemoveUriRecord(uriList, tokenId, abilityId);
    if (!uriList.empty()) {
        DeleteShareFile(tokenId, uriList);
    }
}

int UriPermissionManagerStubImpl::RevokeAllUriPermissions(uint32_t tokenId)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Start to revoke all uri permission, tokenId is %{public}u.", tokenId);
    if (!UPMSUtils::IsFoundationCall()) {
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
            auto ret = UPMSUtils::GetTokenIdByBundleName(authority, 0, authorityTokenId);
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

int UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName,
    int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "Revoke uri permission manually, uri is %{private}s, bundleName is %{public}s, appIndex is %{public}d",
        uri.ToString().c_str(), bundleName.c_str(), appIndex);
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return CHECK_PERMISSION_FAILED;
    }
    if (!CheckUriTypeIsValid(uri)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s.", uri.ToString().c_str());
        return ERR_CODE_INVALID_URI_TYPE;
    }
    uint32_t targetTokenId = 0;
    if (UPMSUtils::GetTokenIdByBundleName(bundleName, appIndex, targetTokenId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId by bundle name failed.");
        return INNER_ERR;
    }

    auto uriStr = uri.ToString();
    auto uriInner = uri;
    uint32_t authorityTokenId = 0;
    UPMSUtils::GetTokenIdByBundleName(uriInner.GetAuthority(), 0, authorityTokenId);
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
        tokenId, UPMSUtils::GetCallerNameByTokenId(tokenId).c_str(), flag, uriVec.size());
    std::vector<bool> result(uriVec.size(), false);
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
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

void UriPermissionManagerStubImpl::ProxyDeathRecipient::OnRemoteDied([[maybe_unused]]
    const wptr<IRemoteObject>& remote)
{
    if (proxy_) {
        TAG_LOGD(AAFwkTag::URIPERMMGR, "mgr stub died.");
        proxy_(remote);
    }
}

int UriPermissionManagerStubImpl::GrantUriPermissionFor2In1Inner(const std::vector<Uri> &uriVec, unsigned int flag,
    const std::string &targetBundleName, int32_t appIndex, bool isSystemAppCall, uint32_t initiatorTokenId,
    int32_t abilityId)
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
        auto ret = GrantUriPermissionInner(otherVec, flag, targetBundleName, appIndex, initiatorTokenId, abilityId);
        if (docsVec.empty()) {
            return ret;
        }
    }
    uint32_t tokenId = 0;
    auto ret = UPMSUtils::GetTokenIdByBundleName(targetBundleName, appIndex, tokenId);
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

bool UriPermissionManagerStubImpl::CheckUriPermission(Uri uri, uint32_t flag, TokenIdPermission &tokenIdPermission)
{
    auto &&authority = uri.GetAuthority();
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Authority of uri is %{public}s", authority.c_str());
    if (uri.GetScheme() == "content") {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "uri is content type.");
        return UPMSUtils::IsFoundationCall();
    }
    if (authority == "docs") {
        return AccessDocsUriPermission(tokenIdPermission, uri, flag);
    }
    if (authority == "media") {
        return AccessMediaUriPermission(tokenIdPermission, uri, flag);
    }
    uint32_t authorityTokenId = 0;
    if (UPMSUtils::GetTokenIdByBundleName(authority, 0, authorityTokenId) != ERR_OK) {
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
}  // namespace AAFwk
}  // namespace OHOS