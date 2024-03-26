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

#include "mock_bundle_mgr_helper.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper()
{}

void BundleMgrHelper::OnDeath()
{}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(
    const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info)
{
    HILOG_INFO("DSZ:: GetSandboxBundleInfo By MockBunleMgrHelper");
    uint32_t tokenId = std::stoi(bundleName.substr(bundleName.size() - 4, 4));
    info.applicationInfo.accessTokenId = tokenId;
    HILOG_INFO("DSZ:: tokenId = %{public}u", tokenId);
    return ERR_OK;
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, const BundleFlag flags,
    BundleInfo &bundleInfo, int32_t userId)
{
    HILOG_INFO("DSZ:: GetBundleInfo By MockBunleMgrHelper");
    uint32_t tokenId = std::stoi(bundleName.substr(bundleName.size() - 4, 4));
    bundleInfo.applicationInfo.accessTokenId = tokenId;
    HILOG_INFO("DSZ:: tokenId = %{public}u", tokenId);
    return true;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string &appName, uint32_t flags,
    int32_t userId, ApplicationInfo &appInfo)
{
    HILOG_INFO("DSZ:: GetApplicationInfo By MockBunleMgrHelper");
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS