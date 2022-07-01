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

#ifndef OHOS_AAFWK_BUNDLE_MANAGER_UTILS_H
#define OHOS_AAFWK_BUNDLE_MANAGER_UTILS_H

#include <string>

#include "bundle_mgr_client.h"
#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class BundleManagerUtils : public std::enable_shared_from_this<BundleManagerUtils> {
    DECLARE_DELAYED_SINGLETON(BundleManagerUtils)
public:
    bool GetBundleAndHapInfo(const AppExecFwk::AbilityInfo &abilityInfo, const AppExecFwk::ApplicationInfo &appInfo,
        int32_t appIndex, AppExecFwk::BundleInfo &bundleInfo, AppExecFwk::HapModuleInfo &hapModuleInfo);
    bool GetBundleInfos(const std::string &bundleName,
        int32_t appIndex, int32_t userId, std::vector<AppExecFwk::BundleInfo> &bundleInfos);
    void QueryAbilityInfo(const Want &want, int32_t appIndex, int32_t flags, int32_t userId,
        AppExecFwk::AbilityInfo &abilityInfo);
    void QueryExtensionAbilityInfos(const Want &want, int32_t appIndex, const int32_t &flags,
        const int32_t &userId, std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos);

private:
    AppExecFwk::BundleMgrClient bundleClient_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_BUNDLE_MANAGER_UTILS_H