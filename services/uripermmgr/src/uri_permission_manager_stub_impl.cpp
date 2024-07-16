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

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "file_permission_manager.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "media_permission_manager.h"
#include "parameter.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "uri_permission_utils.h"
#include "want.h"

#define IS_POLICY_ALLOWED_TO_BE_PRESISTED (1<<0)

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr uint32_t FLAG_READ_WRITE_URI = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr uint32_t FLAG_WRITE_URI = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
constexpr uint32_t FLAG_READ_URI = Want::FLAG_AUTH_READ_URI_PERMISSION;
constexpr uint32_t FLAG_PERSIST_URI = Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
constexpr uint32_t API10 = 10;
constexpr const char* CLOUND_DOCS_URI_MARK = "?networkid=";
}

bool UriPermissionManagerStubImpl::VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "uri is %{private}s, flag is %{public}u, tokenId is %{public}u",
        uri.ToString().c_str(), flag, tokenId);
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return false;
    }
    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid.");
        return false;
    }
    auto uriInner = uri;
    if (!UPMSUtils::CheckUriTypeIsValid(uriInner)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "type of uri is invalid.");
        return false;
    }
    std::vector<Uri> uriVec = { uriInner };
    if (uriInner.GetScheme() == "content" || uriInner.GetAuthority() == "media") {
        auto result = VerifyUriPermissionByMap(uriVec, flag, tokenId);
        if (!result[0]) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "uri permission not exists.");
        }
        return result[0];
    }
    auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriInner, flag);
    std::vector<PolicyInfo> policys = { policyInfo };
    auto result = VerifyUriPermissionByPolicy(policys, flag, tokenId);
    if (!result[0]) {
        TAG_LOGI(AAFwkTag::URIPERMMGR,
            "verify uri permission by policy failed, path is %{private}s.", policyInfo.path.c_str());
    }
    return result[0];
}

std::vector<bool> UriPermissionManagerStubImpl::VerifyUriPermissionByPolicy(std::vector<PolicyInfo> &policys,
    uint32_t flag, uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy called, %{public}zu policys", policys.size());
    std::vector<bool> result;
    auto ret = SandboxManagerKit::CheckPolicy(tokenId, policys, result);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Check policy failed: %{public}d.", ret);
        result = std::vector<bool>(policys.size(), false);
        return result;
    }
    for (size_t i = 0; i < result.size(); i++) {
        if (!result[i]) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "no policy record.");
            break;
        }
    }
    return result;
}

std::vector<bool> UriPermissionManagerStubImpl::VerifyUriPermissionByMap(std::vector<Uri> &uriVec,
    uint32_t flag, uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    uint32_t newFlag = FLAG_READ_URI;
    if ((flag & FLAG_WRITE_URI) != 0) {
        newFlag = FLAG_WRITE_URI;
    }
    std::vector<bool> result(uriVec.size(), false);
    std::lock_guard<std::mutex> guard(mutex_);
    for (size_t i = 0; i < uriVec.size(); i++) {
        result[i] = VerifySingleUriPermissionByMap(uriVec[i].ToString(), newFlag, tokenId);
    }
    return result;
}

bool UriPermissionManagerStubImpl::VerifySingleUriPermissionByMap(const std::string &uri,
    uint32_t flag, uint32_t tokenId)
{
    auto search = uriMap_.find(uri);
    if (search != uriMap_.end()) {
        auto& list = search->second;
        for (auto it = list.begin(); it != list.end(); it++) {
            if ((it->targetTokenId == tokenId) && ((it->flag | FLAG_READ_URI) & flag) != 0) {
                TAG_LOGD(AAFwkTag::URIPERMMGR, "have uri permission.");
                return true;
            }
        }
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Uri permission not exists.");
        return false;
    }
    return VerifySubDirUriPermission(uri, flag, tokenId);
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
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "BundleName is %{public}s, appIndex is %{public}d, flag is %{public}u, size of uri is %{public}zu",
        targetBundleName.c_str(), appIndex, flag, uriVec.size());
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
    flag = (flag | FLAG_READ_URI) & (FLAG_READ_WRITE_URI | FLAG_PERSIST_URI);
    uint32_t targetTokenId = 0;
    auto ret = UPMSUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId of target bundle name failed.");
        return ret;
    }
    uint32_t callerTokenId = initiatorTokenId;
    if (!UPMSUtils::IsFoundationCall()) {
        // recordId will be set default id if the process name is not foundation.
        abilityId = DEFAULT_ABILITY_ID;
        callerTokenId = IPCSkeleton::GetCallingTokenID();
    }
    std::string callerBundleName;
    if (!UPMSUtils::GetBundleNameByTokenId(callerTokenId, callerBundleName)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get caller name failed: %{public}d.", callerTokenId);
    }
    // split uris by uri authority
    BatchUri batchUri;
    auto mode = (flag & FLAG_READ_WRITE_URI);
    if (batchUri.Init(uriVec, mode, callerBundleName, targetBundleName) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "All uri is invalid.");
        return ERR_CODE_INVALID_URI_TYPE;
    }
    auto checkRet = CheckUriPermission(batchUri, flag, callerTokenId, targetTokenId);
    if (checkRet != ERR_OK) {
        return checkRet;
    }
    auto grantRet = GrantUriPermissionInner(batchUri, uriVec, flag, callerTokenId, targetTokenId, abilityId);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Grant UriPermission finished.");
    return grantRet;
}

int UriPermissionManagerStubImpl::GrantUriPermissionInner(BatchUri &batchUri, const std::vector<Uri> &uriVec,
    uint32_t flag, uint32_t callerTokenId, uint32_t targetTokenId, int32_t abilityId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "callerTokenId is %{public}u, targetTokenId is %{public}u, abilityId is %{public}d",
        callerTokenId, targetTokenId, abilityId);
    int32_t grantRet = INNER_ERR;
    if (batchUri.targetBundleUriCount > 0) {
        grantRet = ERR_OK;
    }
    // media, content
    std::vector<std::string> uriStrVec;
    if (batchUri.GetUriToGrantByMap(uriStrVec) > 0) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "size of media and content uri is %{public}zu.", uriStrVec.size());
        if (GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, abilityId) == ERR_OK) {
            grantRet = ERR_OK;
        }
    }
    std::vector<PolicyInfo> docsPolicyInfoVec;
    std::vector<PolicyInfo> bundlePolicyInfoVec;
    if (batchUri.GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec) == 0) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "size of other policy info is 0.");
        return grantRet;
    }
    bool isSystemAppCall = UPMSUtils::IsSystemAppCall();
    // bundle
    TAG_LOGD(AAFwkTag::URIPERMMGR, "size of bundle policy info is %{public}zu.", bundlePolicyInfoVec.size());
    if (GrantBatchUriPermissionImplByPolicy(bundlePolicyInfoVec, flag, callerTokenId, targetTokenId,
        isSystemAppCall) == ERR_OK) {
        grantRet = ERR_OK;
    }
    // docs
    TAG_LOGD(AAFwkTag::URIPERMMGR, "size of docs policy info is %{public}zu.", docsPolicyInfoVec.size());
    bool autoPersist = CheckNeedAutoPersist(batchUri.targetAppName, flag);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "autoPersist is %{public}d.", autoPersist);
    if (GrantBatchUriPermissionImplByPolicy(docsPolicyInfoVec, flag, callerTokenId, targetTokenId,
        isSystemAppCall, autoPersist) == ERR_OK) {
        grantRet = ERR_OK;
    }

    UPMSUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, batchUri.result);
    return grantRet;
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionImplByPolicy(const std::vector<PolicyInfo> &policyInfoVec,
    uint32_t flag, uint32_t callerTokenId, uint32_t targetTokenId, bool isSystemAppCall, bool autoPersist)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (policyInfoVec.empty()) {
        return INNER_ERR;
    }
    std::vector<uint32_t> result;
    uint32_t policyFlag = (flag & FLAG_PERSIST_URI) == 0 ? 0 : IS_POLICY_ALLOWED_TO_BE_PRESISTED;
    auto ret = IN_PROCESS_CALL(SandboxManagerKit::SetPolicy(targetTokenId, policyInfoVec, policyFlag, result));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "SetPolicy failed, ret is %{public}d", ret);
        return ret;
    }
    int successCount = 0;
    for (size_t i = 0; i < result.size(); i++) {
        if (result[i] != ERR_OK) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "Failed to set policy, ret is %{public}d.", result[i]);
            continue;
        }
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Insert an uri policy info, path is %{private}s",
            policyInfoVec[i].path.c_str());
        if (isSystemAppCall) {
            AddPolicyRecordCache(callerTokenId, targetTokenId, policyInfoVec[i].path);
        }
        successCount++;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "Total %{public}d path policy added.", successCount);
    if (successCount == 0) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Grant uri policy failed.");
        return INNER_ERR;
    }
    // The current processing starts from API 11 and maintains 5 versions.
    if (autoPersist) {
        PersistPolicyAutoly(policyInfoVec, targetTokenId);
    }
    return ERR_OK;
}

void UriPermissionManagerStubImpl::PersistPolicyAutoly(const std::vector<PolicyInfo> &policyInfoVec,
    uint32_t targetTokenId)
{
    std::vector<uint32_t> persistResult;
    auto ret = IN_PROCESS_CALL(SandboxManagerKit::PersistPolicy(targetTokenId, policyInfoVec, persistResult));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "PersistPolicy failed, ret is %{public}d.", ret);
        return;
    }
    for (size_t i = 0; i < persistResult.size(); i++) {
        if (!persistResult[i]) {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "PersistPolicy failed, path is %{private}s.",
                policyInfoVec[i].path.c_str());
        }
    }
}

void UriPermissionManagerStubImpl::AddPolicyRecordCache(uint32_t callerTokenId, uint32_t targetTokenId,
    const std::string &path)
{
    GrantPolicyInfo grantPolicyInfo = { callerTokenId, targetTokenId };
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    auto it = policyMap_.find(path);
    if (it == policyMap_.end()) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "add a new policy record, path is %{private}s", path.c_str());
        std::list<GrantPolicyInfo> grantPolicyInfoList = { grantPolicyInfo };
        policyMap_.emplace(path, grantPolicyInfoList);
        return;
    }
    auto grantPolicyInfoList = it->second;
    for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end(); it++) {
        if (it->Equal(callerTokenId, targetTokenId)) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "policy record exists.");
            return;
        }
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "add a policy record.");
    grantPolicyInfoList.emplace_back(grantPolicyInfo);
    return;
}

int32_t UriPermissionManagerStubImpl::GrantBatchUriPermissionImpl(const std::vector<std::string> &uriVec,
    uint32_t flag, TokenId callerTokenId, TokenId targetTokenId, int32_t abilityId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "size of privileged uri is %{public}zu, abilityId is %{public}d",
        uriVec.size(), abilityId);
    if (uriVec.empty()) {
        return INNER_ERR;
    }
    flag &= FLAG_READ_WRITE_URI;
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
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to createShareFile, error code is %{public}d.", resVec[0]);
        return resVec[0];
    }
    int successCount = 0;
    for (size_t i = 0; i < uriVec.size(); i++) {
        auto ret = resVec[i];
        if (ret != 0 && ret != -EEXIST) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Failed to CreateShareFile, ret is %{public}d", ret);
            continue;
        }
        AddTempUriPermission(uriVec[i], flag, callerTokenId, targetTokenId, abilityId);
        successCount++;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "total %{public}d uri permissions added.", successCount);
    if (successCount == 0) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Grant uri permission failed.");
        return INNER_ERR;
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

int32_t UriPermissionManagerStubImpl::GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
    const std::string &targetBundleName, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "BundleName is %{public}s, appIndex is %{public}d, flag is %{public}u.",
        targetBundleName.c_str(), appIndex, flag);

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
    flag = (flag | FLAG_READ_URI) & (FLAG_READ_WRITE_URI | FLAG_PERSIST_URI);

    uint32_t targetTokenId = 0;
    auto ret = UPMSUtils::GetTokenIdByBundleName(targetBundleName, appIndex, targetTokenId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get tokenId failed, bundlename is %{public}s.", targetBundleName.c_str());
        return ret;
    }
    ret = GrantUriPermissionPrivilegedInner(uriVec, flag, callerTokenId, targetTokenId, targetBundleName);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged finished.");
    return ret;
}

int32_t UriPermissionManagerStubImpl::GrantUriPermissionPrivilegedInner(const std::vector<Uri> &uriVec, uint32_t flag,
    uint32_t callerTokenId, uint32_t targetTokenId, const std::string &targetBundleName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "callerTokenId is %{public}u, targetTokenId is %{public}u", callerTokenId, targetTokenId);
    std::vector<std::string> uriStrVec;
    std::vector<PolicyInfo> policys;
    int32_t validUriCount = 0;
    int32_t targetBundleUriCount = 0;
    for (auto &uri : uriVec) {
        auto uriInner = uri;
        if (!UPMSUtils::CheckUriTypeIsValid(uriInner)) {
            continue;
        }
        validUriCount++;
        if (uriInner.GetScheme() == "content") {
            uriStrVec.emplace_back(uriInner.ToString());
            continue;
        }
        auto &&authority = uriInner.GetAuthority();
        if (authority == "media") {
            uriStrVec.emplace_back(uriInner.ToString());
            continue;
        }
        if (authority == targetBundleName) {
            targetBundleUriCount++;
            continue;
        }
        auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriInner, flag);
        policys.emplace_back(policyInfo);
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "size of valid uris is %{public}d.", validUriCount);
    if (validUriCount == 0) {
        return ERR_CODE_INVALID_URI_TYPE;
    }
    int32_t grantRet = INNER_ERR;
    if (targetBundleUriCount > 0) {
        grantRet = ERR_OK;
    }
    if (GrantBatchUriPermissionImpl(uriStrVec, flag, callerTokenId, targetTokenId, DEFAULT_ABILITY_ID) == ERR_OK) {
        grantRet = ERR_OK;
    }
    if (GrantBatchUriPermissionImplByPolicy(policys, flag, callerTokenId, targetTokenId, true) == ERR_OK) {
        grantRet = ERR_OK;
    }
    return grantRet;
}

std::vector<bool> UriPermissionManagerStubImpl::CheckUriAuthorization(const std::vector<std::string> &uriStrVec,
    uint32_t flag, uint32_t tokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string tokenBundleName;
    UPMSUtils::GetBundleNameByTokenId(tokenId, tokenBundleName);
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "tokenId is %{public}u, tokenBundleName is %{public}s, flag is %{public}u, size of uris is %{public}zu",
        tokenId, tokenBundleName.c_str(), flag, uriStrVec.size());
    std::vector<bool> result(uriStrVec.size(), false);
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return result;
    }

    if ((flag & FLAG_READ_WRITE_URI) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Flag is invalid.");
        return result;
    }
    std::vector<Uri> uriVec;
    for (auto &uriStr : uriStrVec) {
        uriVec.emplace_back(uriStr);
    }
    // split uri by uri authority
    BatchUri batchUri;
    if (batchUri.Init(uriVec, 0, tokenBundleName) == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "All uri is invalid.");
        return result;
    }
    CheckUriPermission(batchUri, flag, tokenId);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization finished.");
    return batchUri.result;
}

int32_t UriPermissionManagerStubImpl::CheckUriPermission(BatchUri &batchUri, uint32_t flag,
    uint32_t callerTokenId, uint32_t targetTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "callerTokenId is %{public}u, flag is %{public}u", callerTokenId, flag);
    if (!batchUri.contentUris.empty()) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "content uri: %{public}zu", batchUri.contentUris.size());
        auto contentUriResult = AccessContentUriPermission(batchUri.contentUris);
        batchUri.SetContentUriCheckResult(contentUriResult);
    }
    if (!batchUri.mediaUris.empty()) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "media uri: %{public}zu", batchUri.mediaUris.size());
        auto mediaUriResult = MediaPermissionManager::CheckUriPermission(batchUri.mediaUris, callerTokenId, flag);
        batchUri.SetMediaUriCheckResult(mediaUriResult);
    }
    if (!batchUri.otherUris.empty()) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "other uri: %{public}zu", batchUri.otherUris.size());
        auto otherUriResult = FilePermissionManager::CheckUriPersistentPermission(batchUri.otherUris, callerTokenId,
            flag, batchUri.otherPolicyInfos);
        TAG_LOGI(AAFwkTag::URIPERMMGR, "size of otherPolicyInfos is %{public}zu.", batchUri.otherPolicyInfos.size());
        batchUri.SetOtherUriCheckResult(otherUriResult);
    }

    auto permissionedUriCount = batchUri.GetPermissionedUriCount();
    if (permissionedUriCount != batchUri.validUriCount) {
        TAG_LOGI(AAFwkTag::URIPERMMGR,
            "Need check proxy uri permission, permissioned uri count is %{public}d", permissionedUriCount);
        CheckProxyUriPermission(batchUri, callerTokenId, flag);
    }
    permissionedUriCount = batchUri.GetPermissionedUriCount();
    if (targetTokenId != 0 && permissionedUriCount != batchUri.totalUriCount) {
        UPMSUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);
    }
    if (permissionedUriCount == 0) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri permission failed.");
        return CHECK_PERMISSION_FAILED;
    }
    return ERR_OK;
}

std::vector<bool> UriPermissionManagerStubImpl::AccessContentUriPermission(std::vector<Uri> &uriVec)
{
    std::vector<bool> result;
    bool isFoundationCall = UPMSUtils::IsFoundationCall();
    for (size_t i = 0; i < uriVec.size(); i++) {
        result.emplace_back(isFoundationCall);
    }
    return result;
}

int32_t UriPermissionManagerStubImpl::CheckProxyUriPermission(BatchUri &batchUri, uint32_t callerTokenId,
    uint32_t flag)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TokenIdPermission tokenIdPermission(callerTokenId);
    if (!tokenIdPermission.VerifyProxyAuthorizationUriPermission()) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "No proxy authorization uri permission.");
        return CHECK_PERMISSION_FAILED;
    }
    std::vector<PolicyInfo> proxyUrisByPolicy;
    std::vector<Uri> proxyUrisByMap;
    
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy, proxyUrisByMap);
    if (!proxyUrisByPolicy.empty()) {
        auto proxyResultByPolicy = VerifyUriPermissionByPolicy(proxyUrisByPolicy, flag, callerTokenId);
        batchUri.SetCheckProxyByPolicyResult(proxyResultByPolicy);
    }
    if (!proxyUrisByMap.empty()) {
        auto proxyResultByMap = VerifyUriPermissionByMap(proxyUrisByMap, flag, callerTokenId);
        batchUri.SetCheckProxyByMapResult(proxyResultByMap);
    }
    return ERR_OK;
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
    return RevokeAllUriPermissionsByPolicy(tokenId);
}

int32_t UriPermissionManagerStubImpl::RevokeAllUriPermissionsByPolicy(uint32_t tokenId)
{
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    for (auto iter = policyMap_.begin(); iter != policyMap_.end();) {
        auto& grantPolicyInfoList = iter->second;
        for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end();) {
            if (it->callerTokenId == tokenId || it->targetTokenId == tokenId) {
                TAG_LOGI(AAFwkTag::URIPERMMGR, "Erase an uri policy record.");
                grantPolicyInfoList.erase(it++);
                continue;
            }
            it++;
        }
        if (grantPolicyInfoList.empty()) {
            policyMap_.erase(iter++);
            continue;
        }
        iter++;
    }
    return ERR_OK;
}

int UriPermissionManagerStubImpl::RevokeUriPermissionManually(const Uri &uri, const std::string bundleName,
    int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "Revoke uri permission manually, uri is %{private}s, bundleName is %{public}s, appIndex is %{public}d",
        uri.ToString().c_str(), bundleName.c_str(), appIndex);
    if (!UPMSUtils::IsSAOrSystemAppCall()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Only support SA and SystemApp called.");
        return CHECK_PERMISSION_FAILED;
    }
    auto uriInner = uri;
    if (!UPMSUtils::CheckUriTypeIsValid(uriInner)) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Check uri type failed, uri is %{private}s.", uri.ToString().c_str());
        return ERR_CODE_INVALID_URI_TYPE;
    }
    uint32_t targetTokenId = 0;
    if (UPMSUtils::GetTokenIdByBundleName(bundleName, appIndex, targetTokenId) != ERR_OK) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "get tokenId by bundle name failed.");
        return INNER_ERR;
    }
    return RevokeUriPermissionManuallyInnner(uriInner, targetTokenId);
}

int32_t UriPermissionManagerStubImpl::RevokeUriPermissionManuallyInnner(Uri &uri, uint32_t targetTokenId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    TAG_LOGI(AAFwkTag::URIPERMMGR, "callerTokenId is %{public}u, targetTokenId is %{public}u.",
        callerTokenId, targetTokenId);
    auto scheme = uri.GetScheme();
    auto &&authority = uri.GetAuthority();
    if (scheme != "content" && authority != "media") {
        return RevokeUriPermissionByPolicy(callerTokenId, targetTokenId, uri);
    }
    auto uriStr = uri.ToString();
    uint32_t authorityTokenId = 0;
    UPMSUtils::GetTokenIdByBundleName(authority, 0, authorityTokenId);
    // uri belong to caller or caller is target.
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

int32_t UriPermissionManagerStubImpl::RevokeUriPermissionByPolicy(uint32_t callerTokenId, uint32_t targetTokenId,
    Uri &uri)
{
    auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, FLAG_READ_WRITE_URI);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "path is %{private}s.", policyInfo.path.c_str());
    std::lock_guard<std::mutex> guard(policyMapMutex_);
    auto searchPathIter = policyMap_.find(policyInfo.path);
    if (searchPathIter == policyMap_.end()) {
        TAG_LOGW(AAFwkTag::URIPERMMGR, "Do not found policy info record by path.");
        return ERR_OK;
    }
    bool isPolicySetByCaller = false;
    auto &grantPolicyInfoList = searchPathIter->second;
    for (auto it = grantPolicyInfoList.begin(); it != grantPolicyInfoList.end();) {
        if ((targetTokenId == it->targetTokenId) &&
            (callerTokenId == it->targetTokenId || callerTokenId == it->callerTokenId)) {
            grantPolicyInfoList.erase(it++);
            isPolicySetByCaller = true;
            continue;
        }
    }
    if (grantPolicyInfoList.empty()) {
        policyMap_.erase(searchPathIter);
    }
    if (isPolicySetByCaller) {
        TAG_LOGI(AAFwkTag::URIPERMMGR, "Start to unSetPolicy, path is %{private}s.", policyInfo.path.c_str());
        auto ret = IN_PROCESS_CALL(SandboxManagerKit::UnSetPolicy(targetTokenId, policyInfo));
        if (ret != ERR_OK) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "UnSetPolicy failed, ret is %{public}d", ret);
        }
        return ret;
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "No grant uri permission record.");
    return ERR_OK;
}

int32_t UriPermissionManagerStubImpl::DeleteShareFile(uint32_t targetTokenId, const std::vector<std::string> &uriVec)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
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

int32_t UriPermissionManagerStubImpl::CheckCalledBySandBox()
{
    // reject sandbox to grant uri permission
    ConnectManager(appMgr_, APP_MGR_SERVICE_ID);
    if (appMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "Get AppMgr failed!");
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

bool UriPermissionManagerStubImpl::CheckNeedAutoPersist(const std::string &bundleName, uint32_t flag)
{
    if ((flag & FLAG_PERSIST_URI) == 0) {
        return false;
    }
    if (!AppUtils::GetInstance().IsGrantPersistUriPermission()) {
        return false;
    }
    int32_t targetApiVersion = 0;
    if (!UPMSUtils::GetBundleApiTargetVersion(bundleName, targetApiVersion)) {
        return false;
    }
    return targetApiVersion == API10;
}
}  // namespace AAFwk
}  // namespace OHOS
