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

#include "bundle_manager_utils.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t BASE_USER_RANGE = 200000;
int32_t GetUserIdByUid(int32_t uid)
{
    return uid / BASE_USER_RANGE;
}
BundleManagerUtils::BundleManagerUtils() {}

BundleManagerUtils::~BundleManagerUtils() {}

bool BundleManagerUtils::GetBundleAndHapInfo(const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &appInfo, int32_t appIndex,
    AppExecFwk::BundleInfo &bundleInfo, AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    auto userId = GetUserIdByUid(appInfo.uid);
    HILOG_INFO("GetBundleAndHapInfo come, call bms GetSandboxBundleInfo, userId is %{public}d", userId);
    int32_t result = IN_PROCESS_CALL(bundleClient_.GetSandboxBundleInfo(appInfo.bundleName,
        appIndex, userId, bundleInfo));
    if (result != 0) {
        HILOG_ERROR("GetBundleAndHapInfo fail");
        return false;
    }
    for (AppExecFwk::HapModuleInfo &hap : bundleInfo.hapModuleInfos) {
        if (abilityInfo.moduleName == hap.moduleName) {
            hapModuleInfo = hap;
            return true;
        }
    }
    return false;
}

bool BundleManagerUtils::GetBundleInfos(const std::string &bundleName,
    int32_t appIndex, int32_t userId, std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    HILOG_INFO("GetBundleInfos come, call bms GetSandboxBundleInfo, userId is %{public}d", userId);
    AppExecFwk::BundleInfo bundleInfo;
    int32_t result = IN_PROCESS_CALL(bundleClient_.GetSandboxBundleInfo(bundleName, appIndex, userId, bundleInfo));
    if (result != 0) {
        HILOG_ERROR("GetBundleInfos fail");
        return false;
    }
    bundleInfos.emplace_back(bundleInfo);
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
