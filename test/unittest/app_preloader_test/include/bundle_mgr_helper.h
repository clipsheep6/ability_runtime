/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include "bundle_mgr_interface.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    BundleMgrHelper() = default;;
    ~BundleMgrHelper() = default;

    ErrCode GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId)
    {
        return ERR_OK;
    }

    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
    {
        abilityInfo.type = AppExecFwk::AbilityType::PAGE;
        abilityInfo.isStageBasedModel = true;
        abilityInfo.name = "MainAbility";
        abilityInfo.applicationName = "com.acts.preloadtest";
        abilityInfo.applicationInfo.name = abilityInfo.applicationName;
        return true;
    }

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
    {
        return true;
    }

    bool GetHapModuleInfo(const AbilityInfo &abilityInfo, int32_t userId, HapModuleInfo &hapModuleInfo)
    {
        return true;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H