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

#include "batch_uri.h"

#include "file_permission_manager.h"
#include "hilog_tag_wrapper.h"
#include "uri_permission_utils.h"

namespace OHOS {
namespace AAFwk {

int32_t BatchUri::Init(const std::vector<Uri> &uriVec, uint32_t mode, const std::string &callerBundleName,
    const std::string &targetBundleName)
{
    if (uriVec.empty()) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "uriVec is empty.");
        return 0;
    }
    targetAppName = targetBundleName;
    totalUriCount = uriVec.size();
    validUriCount = 0;
    result = std::vector<bool>(totalUriCount, false);
    isDocsUriVec = std::vector<bool>(totalUriCount, false);
    isTargetBundleUri = std::vector<bool>(totalUriCount, false);
    for (size_t index = 0; index < uriVec.size(); index++) {
        auto uriInner = uriVec[index];
        auto &&scheme = uriInner.GetScheme();
        if (scheme != "content" && scheme != "file") {
            TAG_LOGW(AAFwkTag::URIPERMMGR, "uri is invalid: %{private}s.", uriInner.ToString().c_str());
            continue;
        }
        validUriCount++;
        // content uri
        if (scheme == "content") {
            contentUris.emplace_back(uriInner);
            contentIndexs.emplace_back(index);
            continue;
        }
        InitFileUriInfo(uriInner, index, mode, callerBundleName, targetBundleName);
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "count of uri is %{public}d, count of valid uri is %{public}d.",
        totalUriCount, validUriCount);
    return validUriCount;
}

void BatchUri::InitFileUriInfo(Uri &uriInner, uint32_t index, const uint32_t mode,
    const std::string &callerBundleName, const std::string &targetBundleName)
{
    auto &&authority = uriInner.GetAuthority();
    TAG_LOGD(AAFwkTag::URIPERMMGR, "Authority of uri is %{public}s.", authority.c_str());
    // media uri
    if (authority == "media") {
        mediaUris.emplace_back(uriInner);
        mediaIndexs.emplace_back(index);
        return;
    }
    // bundle uri
    isTargetBundleUri[index] = (!targetBundleName.empty() && authority == targetBundleName);
    if (!callerBundleName.empty() && authority == callerBundleName) {
        result[index] = true;
        if (isTargetBundleUri[index]) {
            TAG_LOGI(AAFwkTag::URIPERMMGR, "uri belong to targetBundle.");
            targetBundleUriCount++;
            return;
        }
        if (mode > 0) {
            // need set policy
            auto policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriInner, mode);
            selfBundlePolicyInfos.emplace_back(policyInfo);
        }
        return;
    }
    if (authority == "docs") {
        isDocsUriVec[index] = true;
    }
    // docs and bundle uri, need to check uri pemission
    otherUris.emplace_back(uriInner);
    otherIndexs.emplace_back(index);
}

void BatchUri::SetContentUriCheckResult(const std::vector<bool> &contentUriResult)
{
    for (size_t i = 0; i < contentUriResult.size(); i++) {
        auto index = contentIndexs[i];
        result[index] = contentUriResult[i];
    }
}

void BatchUri::SetMediaUriCheckResult(const std::vector<bool> &mediaUriResult)
{
    for (size_t i = 0; i < mediaUriResult.size(); i++) {
        auto index = mediaIndexs[i];
        result[index] = mediaUriResult[i];
    }
}

void BatchUri::SetOtherUriCheckResult(const std::vector<bool> &otherUriResult)
{
    for (size_t i = 0; i < otherUriResult.size(); i++) {
        auto index = otherIndexs[i];
        result[index] = otherUriResult[i];
        if (result[index] && isTargetBundleUri[index]) {
            targetBundleUriCount++;
        }
    }
}

void BatchUri::GetNeedCheckProxyPermissionURI(std::vector<PolicyInfo> &proxyUrisByPolicy,
    std::vector<Uri> &proxyUrisByMap)
{
    // docs uri and bundle uri
    for (size_t i = 0; i < otherIndexs.size(); i++) {
        auto index = otherIndexs[i];
        if (!result[index]) {
            proxyIndexsByPolicy.emplace_back(index);
            proxyUrisByPolicy.emplace_back(otherPolicyInfos[i]);
        }
    }
    // media uri
    for (size_t i = 0; i < mediaIndexs.size(); i++) {
        auto index = mediaIndexs[i];
        if (!result[index]) {
            proxyIndexsByMap.emplace_back(index);
            proxyUrisByMap.emplace_back(mediaUris[i]);
        }
    }
}

void BatchUri::SetCheckProxyByMapResult(std::vector<bool> &proxyResultByMap)
{
    for (size_t i = 0; i < proxyResultByMap.size(); i++) {
        auto index = proxyIndexsByMap[i];
        result[index] = proxyResultByMap[i];
    }
    proxyIndexsByMap.clear();
}

void BatchUri::SetCheckProxyByPolicyResult(std::vector<bool> &proxyResultByPolicy)
{
    for (size_t i = 0; i < proxyResultByPolicy.size(); i++) {
        auto index = proxyIndexsByPolicy[i];
        result[index] = proxyResultByPolicy[i];
    }
    proxyIndexsByPolicy.clear();
}

int32_t BatchUri::GetUriToGrantByMap(std::vector<std::string> &uriVec)
{
    // content
    SelectPermissionedUri(contentUris, contentIndexs, uriVec);
    // media
    SelectPermissionedUri(mediaUris, mediaIndexs, uriVec);
    return uriVec.size();
}

void BatchUri::SelectPermissionedUri(std::vector<Uri> &uris, std::vector<int32_t> &indexs,
    std::vector<std::string> &uriVec)
{
    for (size_t i = 0; i < indexs.size(); i++) {
        if (result[indexs[i]]) {
            uriVec.emplace_back(uris[i].ToString());
        }
    }
}

int32_t BatchUri::GetUriToGrantByPolicy(std::vector<PolicyInfo> &docsPolicyInfoVec,
    std::vector<PolicyInfo> &bundlePolicyInfoVec)
{
    // bundleName + docs
    int32_t count = 0;
    for (auto &selfBundleUriPolicy : selfBundlePolicyInfos) {
        bundlePolicyInfoVec.emplace_back(selfBundleUriPolicy);
        count++;
    }
    for (size_t i = 0; i < otherPolicyInfos.size(); i++) {
        auto index = otherIndexs[i];
        if (!result[index]) {
            continue;
        }
        // the uri belong to target app.
        if (isTargetBundleUri[index]) {
            continue;
        }
        TAG_LOGD(AAFwkTag::URIPERMMGR, "Add policy: path is %{private}s, mode is %{public}u.",
            otherPolicyInfos[i].path.c_str(), static_cast<uint32_t>(otherPolicyInfos[i].mode));
        if (isDocsUriVec[index]) {
            docsPolicyInfoVec.emplace_back(otherPolicyInfos[i]);
        } else {
            bundlePolicyInfoVec.emplace_back(otherPolicyInfos[i]);
        }
        count++;
    }
    return count;
}

int32_t BatchUri::GetPermissionedUriCount()
{
    int32_t permissionedUriCount = 0;
    for (auto checkRes: result) {
        if (checkRes) {
            permissionedUriCount++;
        }
    }
    return permissionedUriCount;
}
} // OHOS
} // AAFwk