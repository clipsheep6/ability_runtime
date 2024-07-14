
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

#ifndef OHOS_ABILITY_RUNTIME_BATCH_URI_H
#define OHOS_ABILITY_RUNTIME_BATCH_URI_H

#include <sys/types.h>
#include <vector>

#include "policy_info.h"
#include "uri.h"

namespace OHOS {
namespace AAFwk {
namespace {
using PolicyInfo = AccessControl::SandboxManager::PolicyInfo;
}

class BatchUri {
public:
    BatchUri() {}

    int32_t Init(const std::vector<Uri> &uriVec, uint32_t mode = 0, const std::string &callerBundleName = "",
        const std::string &targetBundleName = "");
    
    void InitFileUriInfo(Uri &uriInner, uint32_t index, const uint32_t mode = 0,
        const std::string &callerBundleName = "", const std::string &targetBundleName = "");

    void SetContentUriCheckResult(const std::vector<bool> &contentUriResult);
    
    void SetMediaUriCheckResult(const std::vector<bool> &mediaUriResult);

    void SetOtherUriCheckResult(const std::vector<bool> &otherUriResult);

    void GetNeedCheckProxyPermissionURI(std::vector<PolicyInfo> &proxyUrisByPolicy, std::vector<Uri> &proxyUrisByMap);

    void SetCheckProxyByMapResult(std::vector<bool> &proxyResultByMap);

    void SetCheckProxyByPolicyResult(std::vector<bool> &proxyResultByPolicy);

    int32_t GetUriToGrantByMap(std::vector<std::string> &uriVec);
    
    void SelectPermissionedUri(std::vector<Uri> &uris, std::vector<int32_t> &indexs, std::vector<std::string> &uriVec);

    int32_t GetUriToGrantByPolicy(std::vector<PolicyInfo> &docsPolicyInfoVec,
        std::vector<PolicyInfo> &bundlePolicyInfoVec);

    int32_t GetPermissionedUriCount();
    
    std::string targetAppName;

    // media uri
    std::vector<Uri> mediaUris;
    std::vector<int32_t> mediaIndexs;
    
    // content uri
    std::vector<Uri> contentUris;
    std::vector<int32_t> contentIndexs;
    
    // docs and bundle uri
    std::vector<Uri> otherUris;
    std::vector<int32_t> otherIndexs;
    std::vector<PolicyInfo> otherPolicyInfos;

    // caller's uri
    std::vector<PolicyInfo> selfBundlePolicyInfos;

    // for check proxy uri permission
    std::vector<int32_t> proxyIndexsByMap;
    std::vector<int32_t> proxyIndexsByPolicy;
    
    // result of CheckUriPermission
    std::vector<bool> result;
    std::vector<bool> isDocsUriVec;

    // target's uri
    int32_t targetBundleUriCount = 0;
    std::vector<bool> isTargetBundleUri;

    int32_t validUriCount = 0;
    int32_t totalUriCount = 0;
};
} // OHOS
} // AAFwk
#endif // OHOS_ABILITY_RUNTIME_BATCH_URI_H