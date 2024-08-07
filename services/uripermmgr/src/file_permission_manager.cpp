/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "file_permission_manager.h"

#include "accesstoken_kit.h"
#include "file_uri.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "tokenid_kit.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
constexpr const uint32_t SANDBOX_MANAGER_OK = 0;
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string DOWNLOAD_PATH = "/storage/Users/currentUser/Download";
const std::string DESKTOP_PATH = "/storage/Users/currentUser/Desktop";
const std::string DOCUMENTS_PATH = "/storage/Users/currentUser/Documents";
const std::string CURRENTUSER = "currentUser";

static bool CheckPermission(uint64_t tokenCaller, const std::string &permission)
{
    return PermissionVerification::GetInstance()->VerifyPermissionByTokenId(tokenCaller, permission);
}

static bool CheckFileManagerUriPermission(uint64_t providerTokenId, std::string &path)
{
    if (path.find(DOWNLOAD_PATH) == 0) {
        return CheckPermission(providerTokenId, PermissionConstants::PERMISSION_READ_WRITE_DOWNLOAD);
    }
    if (path.find(DESKTOP_PATH) == 0) {
        return CheckPermission(providerTokenId, PermissionConstants::PERMISSION_READ_WRITE_DESKTON);
    }
    if (path.find(DOCUMENTS_PATH) == 0) {
        return CheckPermission(providerTokenId, PermissionConstants::PERMISSION_READ_WRITE_DOCUMENTS);
    }
    return false;
}

PolicyInfo FilePermissionManager::GetPathPolicyInfoFromUri(Uri &uri, uint32_t flag, const std::string &bundleName)
{
    AppFileService::ModuleFileUri::FileUri fileUri(uri.ToString());
    std::string path = fileUri.GetRealPathBySA(bundleName);
    PolicyInfo policyInfo;
    policyInfo.path = path;
    policyInfo.mode = (flag & (OperationMode::READ_MODE | OperationMode::WRITE_MODE));
    return policyInfo;
}

std::vector<bool> FilePermissionManager::CheckUriPersistentPermission(std::vector<Uri> &uriVec,
    uint32_t callerTokenId, uint32_t flag, std::vector<PolicyInfo> &pathPolicies)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR,
        "call, uri size:%{public}zu", uriVec.size());
    std::vector<bool> resultCodes(uriVec.size(), false);
    pathPolicies.clear();
    if (CheckPermission(callerTokenId, PermissionConstants::PERMISSION_FILE_ACCESS_MANAGER)) {
        for (size_t i = 0; i < uriVec.size(); i++) {
            resultCodes[i] = true;
            PolicyInfo policyInfo = GetPathPolicyInfoFromUri(uriVec[i], flag);
            pathPolicies.emplace_back(policyInfo);
        }
        return resultCodes;
    }
    std::vector<int32_t> resultIndex;
    std::vector<PolicyInfo> persistPolicys;
    for (size_t i = 0; i < uriVec.size(); i++) {
        PolicyInfo policyInfo = GetPathPolicyInfoFromUri(uriVec[i], flag);
        pathPolicies.emplace_back(policyInfo);
        if (uriVec[i].GetAuthority() == FILE_MANAGER_AUTHORITY &&
            CheckFileManagerUriPermission(callerTokenId, policyInfo.path)) {
            resultCodes[i] = true;
            continue;
        }
        resultIndex.emplace_back(i);
        persistPolicys.emplace_back(policyInfo);
    }
    
    std::vector<bool> persistResultCodes;
    int32_t ret = SandboxManagerKit::CheckPersistPolicy(callerTokenId, persistPolicys, persistResultCodes);
    if (ret == SANDBOX_MANAGER_OK && persistResultCodes.size() == resultIndex.size()) {
        for (size_t i = 0; i < persistResultCodes.size(); i++) {
            auto index = resultIndex[i];
            resultCodes[index] = persistResultCodes[i];
        }
    }
    return resultCodes;
}
}
}
